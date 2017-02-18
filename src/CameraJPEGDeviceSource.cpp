/*
 *  Copyright (C) Peter Gaal
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

// JPEG camera input device


#include "CameraJPEGDeviceSource.hh"
#include "JpegFrameParser.hh"
#include <GroupsockHelper.hh> // for "gettimeofday()"
#include "CameraThread.hh"
//#include <sys/ioctl.h>
//#include "asm/c313a.h"

//#define USE_SAMPLE

//char InputFileName[100];



CameraJPEGDeviceSource* CameraJPEGDeviceSource::createNew(UsageEnvironment& env, unsigned clientSessionId,
unsigned timePerFrame) {


	FILE* fid = NULL;
#ifndef USE_SAMPLE

	//fid = fopen("c:\\Temp\\temp.jpg", "rb");
/*	fid = fopen(InputFileName, "rb");
	if (fid == NULL) {
		env.setResultErrMsg("Failed to open input file");
		return NULL;
	}
	fclose(fid);
*/
#endif  

	//return 
	ourDevice = new CameraJPEGDeviceSource(env, fid, timePerFrame);
	return ourDevice;
}

CameraJPEGDeviceSource* CameraJPEGDeviceSource::createNew(UsageEnvironment& env, unsigned clientSessionId) {


	return createNew(env, NULL, 200000);
}


EventTriggerId CameraJPEGDeviceSource::eventTriggerId = 0;
CameraJPEGDeviceSource* CameraJPEGDeviceSource::ourDevice = NULL;


CameraJPEGDeviceSource
::CameraJPEGDeviceSource(UsageEnvironment& env, FILE* fid,
unsigned timePerFrame)
: JPEGVideoSource(env),
fFid(fid), fTimePerFrame(timePerFrame), fNeedAFrame(False), fEnv(env) {

	fJpegFrameParser = new JpegFrameParser();

	// Ask to be notified when data becomes available on the camera's socket:
	int fd = 0; //fFid->_fileno;
	/*  envir().taskScheduler().
		turnOnBackgroundReadHandling(fd,
		(TaskScheduler::BackgroundHandlerProc*)&newFrameHandler, this);
		*/
	// Start getting frames:

	if (eventTriggerId == 0) {
		eventTriggerId = envir().taskScheduler().createEventTrigger(deliverFrame0);
	}

	startCapture();
}

CameraJPEGDeviceSource::~CameraJPEGDeviceSource()
{
//	fclose(fFid);
	envir().taskScheduler().deleteEventTrigger(eventTriggerId);
	eventTriggerId = 0;

	delete fJpegFrameParser;
}

static int Idunno;


void CameraJPEGDeviceSource::deliverFrame0(void* clientData)
{

	//	((JPEGDeviceSource*)clientData)->deliverFrame();
	((CameraJPEGDeviceSource*)clientData)->deliverFrame();
}


void CameraJPEGDeviceSource::deliverFrame() {
	// This function is called when new frame data is available from the device.
	// We deliver this data by copying it to the 'downstream' object, using the following parameters (class members):
	// 'in' parameters (these should *not* be modified by this function):
	//     fTo: The frame data is copied to this address.
	//         (Note that the variable "fTo" is *not* modified.  Instead,
	//          the frame data is copied to the address pointed to by "fTo".)
	//     fMaxSize: This is the maximum number of bytes that can be copied
	//         (If the actual frame is larger than this, then it should
	//          be truncated, and "fNumTruncatedBytes" set accordingly.)
	// 'out' parameters (these are modified by this function):
	//     fFrameSize: Should be set to the delivered frame size (<= fMaxSize).
	//     fNumTruncatedBytes: Should be set iff the delivered frame would have been
	//         bigger than "fMaxSize", in which case it's set to the number of bytes
	//         that have been omitted.
	//     fPresentationTime: Should be set to the frame's presentation time
	//         (seconds, microseconds).  This time must be aligned with 'wall-clock time' - i.e., the time that you would get
	//         by calling "gettimeofday()".
	//     fDurationInMicroseconds: Should be set to the frame's duration, if known.
	//         If, however, the device is a 'live source' (e.g., encoded from a camera or microphone), then we probably don't need
	//         to set this variable, because - in this case - data will never arrive 'early'.
	// Note the code below.

	if (!isCurrentlyAwaitingData())
		return; // we're not ready for the data yet
	
	const u_int8_t* newFrameDataStart; // = (u_int8_t*)0xDEADBEEF; //%%% TO BE WRITTEN %%%
	unsigned newFrameSize = 0; //%%% TO BE WRITTEN %%%
	


	//FILE* fid = fopen(InputFileName, "rb");
	/*
	if (fid == NULL) {
		printf("Failed to open input file");
	}
	*/

	int size, parser_ret;
	unsigned int jpeg_length;
#define MAX_SIZE 256*1024
	unsigned char buf[MAX_SIZE];
	unsigned char const *scan_data;
	//size = fread(buf, 1, MAX_SIZE, fFid);

	{
		std::unique_lock<std::mutex> locker(lock_jpegbuffer);
		memcpy(buf, CamThreadFrameBuffer, CamThreadFrameBufferSize);
		size = CamThreadFrameBufferSize;
	}

	parser_ret = fJpegFrameParser->parse(buf, size);
	scan_data = fJpegFrameParser->scandata(jpeg_length);
	fLastQFactor = fJpegFrameParser->qFactor();
	//fLastQFactor = 195;
	fLastWidth = fJpegFrameParser->width();
	fLastHeight = fJpegFrameParser->height();
	fType = fJpegFrameParser->type();
	//memcpy(fTo, scan_data, jpeg_length);
	fFrameSize = jpeg_length;

	//fclose(fid);

	newFrameDataStart = scan_data;
	newFrameSize = size;


	// Deliver the data here:
	if (newFrameSize > fMaxSize) {
		fFrameSize = fMaxSize;
		fNumTruncatedBytes = newFrameSize - fMaxSize;
	}
	else {
		fFrameSize = newFrameSize;
	}
	gettimeofday(&fPresentationTime, NULL); // If you have a more accurate time - e.g., from an encoder - then use that instead.
	// If the device is *not* a 'live source' (e.g., it comes instead from a file or buffer), then set "fDurationInMicroseconds" here.
	memmove(fTo, newFrameDataStart, fFrameSize);

	// After delivering the data, inform the reader that it is now available:
	FramedSource::afterGetting(this);
}


void CameraJPEGDeviceSource::doGetNextFrame() {
	/*
	  if (feof(fFid) || ferror(fFid)) {
	  handleClosure(this);
	  return;
	  }
	  */
	fNeedAFrame = True;
	int fd = 0; //fFid->_fileno;
	//ioctl (fd, _IO(CMOSCAM_IOCTYPE, IO_CCAM_JPEG), JPEG_CMD_CATCHUP);

	// If a new frame is already available for us, use it:
	//  if (lseek(fd, 0, SEEK_END) > 0) deliverFrameToClient();
	//deliverFrameToClient();
}

void CameraJPEGDeviceSource
::newFrameHandler(CameraJPEGDeviceSource* source, int mask) {
	source->newFrameHandler1();
}

void CameraJPEGDeviceSource::newFrameHandler1() {
	if (fNeedAFrame) deliverFrameToClient();
}

u_int8_t const* CameraJPEGDeviceSource::quantizationTables(u_int8_t& precision, u_int16_t& length)
{
	precision = fJpegFrameParser->precision();
	return fJpegFrameParser->quantizationTables(length);
}

void CameraJPEGDeviceSource::deliverFrameToClient() {
	static int tickcounter = 0;

	fNeedAFrame = False;
	//  fNeedAFrame = True;

	int tempcount = GetTickCount();
	if (tickcounter == 0)
		tickcounter = tempcount;


	tickcounter = tempcount;

	// Set the 'presentation time': the time that this frame was captured

	// Start capturing the next frame:
	startCapture();

	fPresentationTime = fLastCaptureTime;
	fDurationInMicroseconds = fTimePerFrame;

	// Now, read the previously captured frame:
	// Start with the JPEG header:
	//int fd = fFid->_fileno;
	//  lseek(fd, 0, SEEK_SET);

	printf("CameraJPEGDeviceSource::deliverFrameToClient(): %d\n", GetTickCount());
#ifndef USE_SAMPLE
	//  FILE* fid = fopen("c:\\Temp\\temp.jpg", "rb");
/*	FILE* fid = fopen(InputFileName, "rb");
	if (fid == NULL) {
		printf("Failed to open input file");
	}
*/
//	int size, parser_ret;
//	unsigned int jpeg_length;
#define MAX_SIZE 256*1024
//	unsigned char buf[MAX_SIZE];
//	unsigned char const *scan_data;
//	size = fread(buf, 1, MAX_SIZE, fFid);

/*	parser_ret = fJpegFrameParser->parse(buf, size);
	scan_data = fJpegFrameParser->scandata(jpeg_length);
	fLastQFactor = fJpegFrameParser->qFactor();
	//fLastQFactor = 195;
	fLastWidth = fJpegFrameParser->width();
	fLastHeight = fJpegFrameParser->height();
	fType = fJpegFrameParser->type();
	memcpy(fTo, scan_data, jpeg_length);
	fFrameSize = jpeg_length;
	*/
//	fclose(fid);
	/*


	fread(fJPEGHeader, 1, JPEG_HEADER_SIZE, fFid);
	setParamsFromHeader();
	fFrameSize = fread(fTo, 1, fMaxSize, fFid);
	if (fFrameSize == fMaxSize) {
	fprintf(stderr, "JPEGDeviceSource::doGetNextFrame(): read maximum buffer size: %d bytes.  Frame may be truncated\n", fMaxSize);
	}
	*/


#else


#define KJpegCh2ScanDataLen 56

	// RGB JPEG images as RTP payload - 64x48 pixel
	char JpegScanDataCh2A[KJpegCh2ScanDataLen] =
	{
		0xf8, 0xbe, 0x8a, 0x28, 0xaf, 0xe5, 0x33, 0xfd, 
		0xfc, 0x0a, 0x28, 0xa2, 0x80, 0x0a, 0x28, 0xa2, 
		0x80, 0x0a, 0x28, 0xa2, 0x80, 0x0a, 0x28, 0xa2, 
		0x80, 0x0a, 0x28, 0xa2, 0x80, 0x0a, 0x28, 0xa2, 
		0x80, 0x0a, 0x28, 0xa2, 0x80, 0x0a, 0x28, 0xa2, 
		0x80, 0x0a, 0x28, 0xa2, 0x80, 0x0a, 0x28, 0xa2, 
		0x80, 0x0a, 0x28, 0xa2, 0x80, 0x3f, 0xff, 0xd9            
	};
	char JpegScanDataCh2B[KJpegCh2ScanDataLen] =
	{
		0xf5, 0x8a, 0x28, 0xa2, 0xbf, 0xca, 0xf3, 0xfc, 
		0x53, 0x0a, 0x28, 0xa2, 0x80, 0x0a, 0x28, 0xa2, 
		0x80, 0x0a, 0x28, 0xa2, 0x80, 0x0a, 0x28, 0xa2, 
		0x80, 0x0a, 0x28, 0xa2, 0x80, 0x0a, 0x28, 0xa2, 
		0x80, 0x0a, 0x28, 0xa2, 0x80, 0x0a, 0x28, 0xa2, 
		0x80, 0x0a, 0x28, 0xa2, 0x80, 0x0a, 0x28, 0xa2, 
		0x80, 0x0a, 0x28, 0xa2, 0x80, 0x3f, 0xff, 0xd9            
	};

	char * JPEGs[2] = { JpegScanDataCh2A, JpegScanDataCh2B};
	static int jpegcounter=0;

	jpegcounter&=1;

	//c:\Temp\temp.jpg
	// Then, the JPEG payload:
	fFrameSize = KJpegCh2ScanDataLen;

	//memcpy(fTo, JpegScanDataCh2A, fFrameSize);
	memcpy(fTo, JPEGs[jpegcounter], fFrameSize);
	fLastWidth=8;
	fLastHeight=6;
	fLastQFactor= 0x5e;
	jpegcounter++;
#endif
	//  ioctl (fd, _IO(CMOSCAM_IOCTYPE, IO_CCAM_JPEG), JPEG_CMD_FORGET);
	//clearerr(fFid); // clears EOF flag (for next time)

	// Switch to another task, and inform the reader that he has data:
	nextTask() = envir().taskScheduler().scheduleDelayedTask(0,
		(TaskFunc*)FramedSource::afterGetting, this);

	/*
	  if (tempcount-tickcounter<100)
	  {
	  //	  fNeedAFrame = True;
	  nextTask() = envir().taskScheduler().scheduleDelayedTask(1000000,
	  (TaskFunc*)FramedSource::afterGetting, this);

	  return;
	  }
	  */
}

u_int8_t CameraJPEGDeviceSource::type() {
	return fType;
}
u_int8_t CameraJPEGDeviceSource::qFactor() {
	return fLastQFactor;
}
u_int8_t CameraJPEGDeviceSource::width() {
	return fLastWidth;
}
u_int8_t CameraJPEGDeviceSource::height() {
	return fLastHeight;
}

void CameraJPEGDeviceSource::startCapture() {
	// Arrange to get a new frame now:
	int fd = 0; //fFid->_fileno;
	//  ioctl (fd, _IO(CMOSCAM_IOCTYPE, IO_CCAM_JPEG), JPEG_CMD_ACQUIRE);
	// Consider the capture as having occurred now:
	gettimeofday(&fLastCaptureTime, &Idunno);
}

void CameraJPEGDeviceSource::setParamsFromHeader() {
	// Look for the "SOF0" marker (0xFF 0xC0), to get the frame
	// width and height:
	Boolean foundIt = False;
	for (int i = 0; i < JPEG_HEADER_SIZE - 8; ++i) {
		if (fJPEGHeader[i] == 0xFF && fJPEGHeader[i + 1] == 0xC0) {
			fLastHeight = (fJPEGHeader[i + 5] << 5) | (fJPEGHeader[i + 6] >> 3);
			fLastWidth = (fJPEGHeader[i + 7] << 5) | (fJPEGHeader[i + 8] >> 3);
			foundIt = True;
			break;
		}
	}
	if (!foundIt) fprintf(stderr, "JPEGDeviceSource: Failed to find SOF0 marker in header!\n");

	// The 'Q' factor is not in the header; do an ioctl() to get it:
	int fd = 0; // fFid->_fileno;
	//  fLastQFactor = ioctl(fd, _CCCMD(CCAM_RPARS, P_QUALITY), 0); 
	fLastQFactor = 70;
	//  fLastQFactor = 50;
}


