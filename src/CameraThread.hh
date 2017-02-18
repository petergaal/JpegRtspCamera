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

// Camera Thread

#ifndef _CAMERA_THREAD_HH_INCLUDED
#define _CAMERA_THREAD_HH_INCLUDED

#include "BasicUsageEnvironment.hh"
#include <mutex>

extern std::mutex lock_jpegbuffer;
extern char CamThreadFrameBuffer[256 * 1024];
extern int CamThreadFrameBufferSize;
void cam_thread_main(TaskScheduler* env, int fps);


#endif /* _CAMERA_THREAD_HH_INCLUDED */
