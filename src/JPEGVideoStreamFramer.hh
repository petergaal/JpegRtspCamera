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

// JPEG VIDEOstreamFramer
//

#ifndef _JPEG_VIDEO_STREAM_FRAMER_HH
#define _JPEG_VIDEO_STREAM_FRAMER_HH

#include "JPEGVideoSource.hh"


class JPEGVideoStreamFramer: public JPEGVideoSource {
public:

  static JPEGVideoStreamFramer* createNew(UsageEnvironment& env, FramedSource* inputSource);

  Boolean& pictureEndMarker() { return fPictureEndMarker; }    // a hack for implementing the RTP 'M' bit

protected:
  // Constructor called only by createNew(), or by subclass constructors
  JPEGVideoStreamFramer(UsageEnvironment& env,
			                FramedSource* inputSource,
			                Boolean createParser = True, double frameRate = 5.0f);
  virtual ~JPEGVideoStreamFramer();


public:
  static void continueReadProcessing(void* clientData,
				     unsigned char* ptr, unsigned size,
				     struct timeval presentationTime);
  void continueReadProcessing();

  virtual u_int8_t type();
  virtual u_int8_t qFactor();
  virtual u_int8_t width(); // # pixels/8 (or 0 for 2048 pixels)
  virtual u_int8_t height(); // # pixels/8 (or 0 for 2048 pixels)

  virtual u_int8_t const* quantizationTables(u_int8_t& precision,
					     u_int16_t& length);

private:
  virtual void doGetNextFrame();
  virtual Boolean isJPEGVideoSource() const;

protected:
  double   fFrameRate;
  unsigned fPictureCount; // hack used to implement doGetNextFrame() ??
  Boolean  fPictureEndMarker;

private:
  class JPEGVideoStreamParser* fParser;
  struct timeval fPresentationTimeBase;
};

#endif
