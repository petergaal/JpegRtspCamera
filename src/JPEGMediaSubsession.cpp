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

 // JPEG Media subsession

#include "JPEGMediaSubsession.hh"
#include "FramedSource.hh"
#include "CameraJPEGDeviceSource.hh"
#include "JPEGVideoRTPSink.hh"

JPEGMediaSubsession::JPEGMediaSubsession(UsageEnvironment& env, char const* fileName,
			    Boolean reuseFirstSource)
  : OnDemandServerMediaSubsession(env, reuseFirstSource),
    fFileSize(0) {
  fFileName = strDup(fileName);
}

JPEGMediaSubsession::~JPEGMediaSubsession() {
  delete[] (char*)fFileName;
}


FramedSource* JPEGMediaSubsession::createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate)
{ 
	FramedSource* framedSource = CameraJPEGDeviceSource::createNew(envir(), clientSessionId);
  return framedSource;
}

RTPSink* JPEGMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource){

   return JPEGVideoRTPSink::createNew(envir(), rtpGroupsock);
}
