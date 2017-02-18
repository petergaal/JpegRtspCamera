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

 // JPEG Video stream parser
 // this was modified from H263plusVideoStreamParser class, to handle JPEG frames
 
#include "JPEGVideoStreamParser.hh"
#include "JPEGVideoStreamFramer.hh"
//#include <string.h>
//#include "GroupsockHelper.hh"


JPEGVideoStreamParser::JPEGVideoStreamParser(
                              JPEGVideoStreamFramer* usingSource,
                              FramedSource* inputSource)
                              : StreamParser(inputSource,
                                   FramedSource::handleClosure,
                                   usingSource,
                                   &JPEGVideoStreamFramer::continueReadProcessing,
                                   usingSource)
{

}

JPEGVideoStreamParser::~JPEGVideoStreamParser()
{
}

void JPEGVideoStreamParser::restoreSavedParserState()
{
   StreamParser::restoreSavedParserState();
   fTo = fSavedTo;
   fNumTruncatedBytes = fSavedNumTruncatedBytes;
}

void JPEGVideoStreamParser::setParseState()
{
   fSavedTo = fTo;
   fSavedNumTruncatedBytes = fNumTruncatedBytes;
   saveParserState();  // Needed for the parsing process in StreamParser
}


void JPEGVideoStreamParser::registerReadInterest(
                                   unsigned char* to,
                                   unsigned maxSize)
{
   fStartOfFrame = fTo = fSavedTo = to;
   fLimit = to + maxSize;
   fMaxSize = maxSize;
   fNumTruncatedBytes = fSavedNumTruncatedBytes = 0;
}

unsigned JPEGVideoStreamParser::parse(u_int64_t & currentDuration)
{

   u_int32_t      frameSize;        // The current frame size

   try    // The get data routines of the class FramedFilter returns an error when
   {      // the buffer is empty. This occurs at the beginning and at the end of the file.
//      fCurrentInfo = fNextInfo;

      // Parse 1 frame
      // For the first time, only the first frame's header is returned.
      // The second time the full first frame is returned
      frameSize = parseJPEGFrame();

      currentDuration = 0;
      if ((frameSize > 0)){

	 setParseState(); // Needed for the parsing process in StreamParser
      }
   } catch (int /*e*/) {
#ifdef DEBUG
      fprintf(stderr, "JPEGVideoStreamParser::parse() EXCEPTION (This is normal behavior - *not* an error)\n");
#endif
  //    frameSize=0;
   }

   return frameSize;
}


int JPEGVideoStreamParser::parseJPEGFrame( )
{
   char     row = 0;
   u_int8_t * bufferIndex = fTo;
   // The buffer end which will allow the loop to leave place for
   // the additionalBytesNeeded
   u_int8_t * bufferEnd = fTo + fMaxSize - ADDITIONAL_BYTES_NEEDED - 1;

   memcpy(fTo, fNextHeader, JPEG_REQUIRE_HEADER_SIZE_BYTES);
   bufferIndex += JPEG_REQUIRE_HEADER_SIZE_BYTES;


   // The state table and the following loop implements a state machine enabling
   // us to read bytes from the file until (and inclusing) the requested
   // start code (00 00 8X) is found

   // Initialize the states array, if it hasn't been initialized yet...
   if (!fStates[0][0]) {
      // One 00 was read
      fStates[0][0] = 1;
      // Two sequential 0x00 ware read
      fStates[1][0] = fStates[2][0] = 2;
      // A full start code was read
      fStates[2][128] = fStates[2][129] = fStates[2][130] = fStates[2][131] = -1;
   }

   // Read data from file into the output buffer until either a start code
   // is found, or the end of file has been reached.
   do {
      *bufferIndex = get1Byte();
   } while ((bufferIndex < bufferEnd) &&                    // We have place in the buffer
            ((row = fStates[(unsigned char)row][*(bufferIndex++)]) != -1)); // Start code was not found

   if (row != -1) {
      fprintf(stderr, "%s: Buffer too small (%u)\n",
         "h263reader:", bufferEnd - fTo + ADDITIONAL_BYTES_NEEDED);
      return 0;
   }

   // Cool ... now we have a start code
   // Now we just have to read the additionalBytesNeeded
   getBytes(bufferIndex, ADDITIONAL_BYTES_NEEDED);
   memcpy(fNextHeader, bufferIndex - JPEG_STARTCODE_SIZE_BYTES, JPEG_REQUIRE_HEADER_SIZE_BYTES);

	int sz = bufferIndex - fTo - JPEG_STARTCODE_SIZE_BYTES;

   if (sz == 5) // first frame
      memcpy(fTo, fTo+JPEG_REQUIRE_HEADER_SIZE_BYTES, JPEG_REQUIRE_HEADER_SIZE_BYTES);

   return sz;
}

