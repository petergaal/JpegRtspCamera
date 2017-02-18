/*
 *  Copyright (C) Peter Gaal
 *  this code is derived from work of Massimo Cora' 2006 <maxcvs@email.it>
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

 // JPEG (MJPEG) Video Source

#include "MJPEGVideoSource.hh"

// livemedia
#include <GroupsockHelper.hh>

#include <MediaSink.hh>

// opencv
//#include <cv.h>
//#include <highgui.h>



//--------------------------------------------------------------------------
// constructor

MJPEGVideoSource::MJPEGVideoSource( UsageEnvironment& env, /*BaseInputVideo* input_video, */
								   int stream_id,  int frame_rate, int encoder_buf_size ) 
						: JPEGVideoSource( env ), /* _input_video( input_video ), */
						_encoder_internal_buf_size( encoder_buf_size ), 
						_fps( frame_rate ),
						_stream_id( stream_id )
{
	_initialized = false;
	
	// set the max output packet buffer size
	OutPacketBuffer::maxSize = OUTPUT_PACKET_BUFFER_MAXSIZE;
//	_video_encoder = NULL;

	qTable = NULL;
	
	_frame_rate_millis = 1000/frame_rate;

	// initialize device
//	if ( _input_video->init_device () == true )
		_initialized = true;		
}

//--------------------------------------------------------------------------
//

MJPEGVideoSource* MJPEGVideoSource::createNew( UsageEnvironment &env, /*BaseInputVideo* input_video, */
											   int stream_id /* = 0 */, int frame_rate /*= 25*/, 
											   int encoder_buf_size /* = 1000000*/ ) {
	return new MJPEGVideoSource( env, /*input_video, */stream_id, frame_rate, encoder_buf_size );
}

//--------------------------------------------------------------------------
//

void  MJPEGVideoSource::startCapture() 
{
/*	IplImage* current_frame = NULL;
	fPreferredFrameSize = 0;

	if ( _input_video->is_multi_stream () ) {
		if ( (current_frame = _input_video->get_next_frame_by_stream_id ( _stream_id )) == NULL ) {
			DEBUG_PRINT ("id #%d :_input_video->get_next_frame_by_stream_id = NULL\n", _stream_id );
		}
	}
	else if ( (current_frame = _input_video->get_next_frame ()) == NULL ) {
		DEBUG_PRINT ("_input_video->get_next_frame = NULL\n");
	}

	// we cannot proceed with encoding or something else. Just exit here
	if ( current_frame == NULL ) {
		fFrameSize = fPreferredFrameSize = 0;
		_encoded_frame = NULL;
		return;
	}  */
	gettimeofday( &fLastCaptureTime, &Idunno );
	/*
	// proceed with encoding
	if ( _video_encoder == NULL ) {
		// encoder is NULL, create it.
		_video_encoder = new OStreamVideoEncoder( 1000000, OSTREAM_ENCODING_MJPEG,
							 	current_frame->width, current_frame->height, _fps );
	}
	*/
	int size_taken;
	/*
	_encoded_frame = _video_encoder->encode_video_frame( current_frame, &size_taken );

	if ( size_taken > _encoder_internal_buf_size || _encoded_frame == NULL ) {
		DEBUG_PRINT ("Error: the jpeg frame to stream is too big.\n");
		fPreferredFrameSize = 0;
		_encoded_frame = NULL;
		return;
	}

		*/

	FILE* fin = fopen("c:\\Temp\\temp.jpg", "rb");
	if (fin == NULL) {
		printf("Failed to open input file");
	}
	size_taken = fread(_encoded_frame, 1, 256*1024, fin);
	fclose(fin);

	int jpeg_buf_size = size_taken;

	/*
    if ( jpeg_buf_size <= 0 || (unsigned int)jpeg_buf_size > fMaxSize ) {
		DEBUG_PRINT ("Error: the jpeg frame to stream is too big.\n");
		fPreferredFrameSize = 0;
		_encoded_frame = NULL;
		return;
	}*/

	fPreferredFrameSize = jpeg_buf_size;
//	cvReleaseImage( &current_frame );
}


MJPEGVideoSource::~MJPEGVideoSource() {

	// we don't need the encoder anymore
/*	if ( _video_encoder )
		delete _video_encoder;
	_video_encoder = NULL;
	
	// unuseful but anyway..
	_encoded_frame = NULL;	
	*/
	if ( qTable != NULL )
		free( qTable );

	_initialized = false;
}

u_int8_t const* MJPEGVideoSource::quantizationTables(u_int8_t& precision,
													 u_int16_t& length) 
{
	length = 0;
	precision = 0;
	
	if ( qTable == NULL )
		return NULL;
	
	precision = 8;
	length = 64 * 2; //Q-table is 64-bytes.
	
	return qTable;
}

Boolean MJPEGVideoSource::isJPEGVideoSource() const 
{
  return true;
}



//--------------------------------------------------------------------------
// 

void MJPEGVideoSource::doGetNextFrame() {
	// let the cpu breath.
//	CV_WAIT_KEY( _frame_rate_millis );

	deliverFrame();
}

//--------------------------------------------------------------------------
//

void MJPEGVideoSource::deliverFrame() {
	//
	// This would be called when new frame data is available from the device.
	// This function should deliver the next frame of data from the device,
	// using the following parameters (class members):
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
	//         (seconds, microseconds).
	//     fDurationInMicroseconds: Should be set to the frame's duration, if known.

	// Start capturing the next frame:
	startCapture();

	// Set the 'presentation time': the time that this frame was captured
	fPresentationTime = fLastCaptureTime;

	// Now, read the previously captured frame:
	// Start with the JPEG header:
	int jpeg_header_size = setParamsFromHeader( _encoded_frame );

	// if the header size has missed go on with the next task
	if ( jpeg_header_size <= 0 || fPreferredFrameSize == 0 ) {
		nextTask()  = envir().taskScheduler().scheduleDelayedTask( 1000, (TaskFunc*)afterGetting,
						  											this);
		return;
	}
	
	// Then, the JPEG payload: fTo *do not have to* include jpeg header. 
	// It will automatically filled with the rtp header which is
	// smaller

	fFrameSize = fPreferredFrameSize - jpeg_header_size;
	memcpy( fTo, _encoded_frame + jpeg_header_size, fFrameSize );

	if ( fFrameSize == fMaxSize ) {
		
		printf( "MJPEGVideoSource::doGetNextFrame(): "
			"read maximum buffer size: %d bytes.  Frame may be truncated\n", fMaxSize);
		
	}

	// After delivering the data, switch to another task, and inform
	// the reader that he has data:
	nextTask()  = envir().taskScheduler().scheduleDelayedTask( 1000, (TaskFunc*)afterGetting,
						  										this);
}


//--------------------------------------------------------------------------
//

u_int8_t MJPEGVideoSource::type() {
	return 1;		// 1 is for jpeg type.
}

//--------------------------------------------------------------------------
//

u_int8_t MJPEGVideoSource::width() {
	return fLastWidth;
}

//--------------------------------------------------------------------------
//

u_int8_t MJPEGVideoSource::height() {
	return fLastHeight;
}

//--------------------------------------------------------------------------
//

u_int8_t MJPEGVideoSource::qFactor() {
	// ffmpeg needs this to be returned.
	return 255;
}


//--------------------------------------------------------------------------
// try to find width and height markers and grab their values.
// -1 on error

int MJPEGVideoSource::setParamsFromHeader( const unsigned char *fJPEGHeader ) {
	// Look for the "SOF0" marker (0xFF 0xC0), to get the frame
	// width and height:
	bool found_sofo = false;
	bool found_hdr_size = false;
	int jpeg_header_size;

	for (int i = 0; i < JPEG_HEADER_MAX_SIZE/*-8*/; ++i) {
		if ( !found_sofo && fJPEGHeader[i] == 0xFF && fJPEGHeader[i+1] == 0xC0 ) {
			fLastHeight = (fJPEGHeader[i+5]<<5)|(fJPEGHeader[i+6]>>3);
			fLastWidth = (fJPEGHeader[i+7]<<5)|(fJPEGHeader[i+8]>>3);
			found_sofo = true;
		}

		if (fJPEGHeader[i] == 0xFF && fJPEGHeader[i+1] == 0xDB) {
			if(qTable != NULL)
				free (qTable);
			
			qTable = (u_int8_t *) malloc(sizeof(u_int8_t) * 128);
			memcpy(qTable, fJPEGHeader + i + 5, 64);
			memcpy(qTable + 64, fJPEGHeader + i + 5, 64);
		}
		
		// we should look for the final byte of the 0xFF 0xDA marker,
		// the start of scan part.
		if ( fJPEGHeader[i] == 0x3F && fJPEGHeader[i+1] == 0x00 ) {
			found_hdr_size = true;
			jpeg_header_size = i+2;
			break;
		}
	}

	
	if ( !found_sofo )
		printf( "MJPEGVideoSource: Failed to find SOF0 marker in header!\n");
		
	if ( !found_hdr_size )	
		printf( "MJPEGVideoSource: Failed to find header size!\n");
		
	if ( found_hdr_size == false )
		return -1;

	return jpeg_header_size;
}

