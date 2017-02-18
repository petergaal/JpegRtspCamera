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

#include "JPEGVideoStreamFramer.hh"
#include "JPEGVideoStreamParser.hh"


#include <string.h>
#include <GroupsockHelper.hh>


JPEGVideoStreamFramer* JPEGVideoStreamFramer::createNew(
                                                         UsageEnvironment& env,
                                                         FramedSource* inputSource)
{
   JPEGVideoStreamFramer* fr;
   fr = new JPEGVideoStreamFramer(env, inputSource);
   return fr;
}

u_int8_t JPEGVideoStreamFramer::type()
{
	return 1;
}
u_int8_t JPEGVideoStreamFramer::qFactor()
{
	return 70;
}
u_int8_t JPEGVideoStreamFramer::width()
{
	return 80;
}
u_int8_t JPEGVideoStreamFramer::height() {
	return 60;
}

u_int8_t const* JPEGVideoStreamFramer::quantizationTables(u_int8_t& precision,
					     u_int16_t& length)
{
	return NULL;
}

JPEGVideoStreamFramer::JPEGVideoStreamFramer(
                              UsageEnvironment& env,
                              FramedSource* inputSource,
                              Boolean createParser, double frameRate)
							  : JPEGVideoSource((env))
{
   // Use the current wallclock time as the base 'presentation time':
   gettimeofday(&fPresentationTimeBase, NULL);
   fParser = createParser ? new JPEGVideoStreamParser(this, inputSource) : NULL;
}

JPEGVideoStreamFramer::~JPEGVideoStreamFramer()
{
   delete   fParser;
}


void JPEGVideoStreamFramer::doGetNextFrame()
{
  fParser->registerReadInterest(fTo, fMaxSize);
  continueReadProcessing();
}


Boolean JPEGVideoStreamFramer::isJPEGVideoSource() const
{
  return True;
}

void JPEGVideoStreamFramer::continueReadProcessing(
                                   void* clientData,
                                   unsigned char* /*ptr*/, unsigned /*size*/,
                                   struct timeval /*presentationTime*/)
{
   JPEGVideoStreamFramer* framer = (JPEGVideoStreamFramer*)clientData;
   framer->continueReadProcessing();
}

void JPEGVideoStreamFramer::continueReadProcessing()
{
   unsigned acquiredFrameSize;

   u_int64_t frameDuration;  // in ms

   acquiredFrameSize = fParser->parse(frameDuration);
// Calculate some average bitrate information (to be adapted)
//	avgBitrate = (totalBytes * 8 * H263_TIMESCALE) / totalDuration;

   if (acquiredFrameSize > 0) {
      // We were able to acquire a frame from the input.
      // It has already been copied to the reader's space.
      fFrameSize = acquiredFrameSize;
//    fNumTruncatedBytes = fParser->numTruncatedBytes(); // not needed so far

      fFrameRate = frameDuration == 0 ? 0.0 : 1000./(long)frameDuration;

      // Compute "fPresentationTime"
      if (acquiredFrameSize == 5) // first frame
         fPresentationTime = fPresentationTimeBase;
      else
         fPresentationTime.tv_usec += (long) frameDuration*1000;

      while (fPresentationTime.tv_usec >= 1000000) {
         fPresentationTime.tv_usec -= 1000000;
         ++fPresentationTime.tv_sec;
      }

      // Compute "fDurationInMicroseconds"
      fDurationInMicroseconds = (unsigned int) frameDuration*1000;;

      // Call our own 'after getting' function.  Because we're not a 'leaf'
      // source, we can call this directly, without risking infinite recursion.
      afterGetting(this);
   } else {
      // We were unable to parse a complete frame from the input, because:
      // - we had to read more data from the source stream, or
      // - the source stream has ended.
   }
}
