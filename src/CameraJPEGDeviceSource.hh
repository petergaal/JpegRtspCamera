/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**********/

// JPEG camera input device
// C++ header

#ifndef JPEG_DEVICE_SOURCE_HH
#define JPEG_DEVICE_SOURCE_HH

//#ifndef _JPEG_VIDEO_SOURCE_HH
#include "JPEGVideoSource.hh"
//#endif

#include "JpegFrameParser.hh"

//#define JPEG_HEADER_SIZE 623
//#define JPEG_HEADER_SIZE 623
#define JPEG_HEADER_SIZE 0x299+12

//extern char InputFileName[100];

class CameraJPEGDeviceSource : public JPEGVideoSource {
public:
	static CameraJPEGDeviceSource* createNew(UsageEnvironment& env, unsigned clientSessionId);
	static CameraJPEGDeviceSource* createNew(UsageEnvironment& env, unsigned clientSessionId,
		unsigned timePerFrame);
	// "timePerFrame" is in microseconds

public:
	static EventTriggerId eventTriggerId;
	static CameraJPEGDeviceSource* ourDevice;
	// Note that this is defined here to be a static class variable, because this code is intended to illustrate how to
	// encapsulate a *single* device - not a set of devices.
	// You can, however, redefine this to be a non-static member variable.

protected:
	CameraJPEGDeviceSource(UsageEnvironment& env,
		FILE* fid, unsigned timePerFrame);
	// called only by createNew()
	virtual ~CameraJPEGDeviceSource();

private:
	// redefined virtual functions:
	virtual void doGetNextFrame();
	virtual u_int8_t type();
	virtual u_int8_t qFactor();
	virtual u_int8_t width();
	virtual u_int8_t height();
	virtual u_int8_t const* quantizationTables(u_int8_t& precision,
		u_int16_t& length);

private:
	UsageEnvironment& fEnv;
	static void deliverFrame0(void* clientData);
	void deliverFrame();

	static void newFrameHandler(CameraJPEGDeviceSource* source, int mask);
	void newFrameHandler1();
	void deliverFrameToClient();
	void startCapture();
	void setParamsFromHeader();

private:

	FILE* fFid;
	JpegFrameParser *fJpegFrameParser;
	unsigned fTimePerFrame;
	struct timeval fLastCaptureTime;
	u_int8_t fType, fLastQFactor, fLastWidth, fLastHeight;
	Boolean fNeedAFrame;
	unsigned char fJPEGHeader[JPEG_HEADER_SIZE];

};

#endif
