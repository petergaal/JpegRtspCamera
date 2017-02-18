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

#include "CameraThread.hh"
#include "BasicUsageEnvironment.hh"
#include "CameraJPEGDeviceSource.hh"

#include <iostream>
#include <thread>
#include <chrono>
#include "opencv2\opencv.hpp"

std::mutex lock_jpegbuffer;

char CamThreadFrameBuffer[256 * 1024];
int CamThreadFrameBufferSize;

// The following code would be called to signal that a new frame of data has become available.
// This (unlike other "LIVE555 Streaming Media" library code) may be called from a separate thread.
// (Note, however, that "triggerEvent()" cannot be called with the same 'event trigger id' from different threads.
// Also, if you want to have multiple device threads, each one using a different 'event trigger id', then you will need
// to make "eventTriggerId" a non-static member variable of "DeviceSource".)
void signalNewFrameData(TaskScheduler* scheduler) {
	TaskScheduler* ourScheduler = scheduler; //%%% TO BE WRITTEN %%%
	//	DeviceSource* ourDevice = NULL; //%%% TO BE WRITTEN %%%
	CameraJPEGDeviceSource* ourDevice = CameraJPEGDeviceSource::ourDevice;

	if (ourScheduler != NULL) { // sanity check
		if (ourDevice != NULL) {
			ourScheduler->triggerEvent(CameraJPEGDeviceSource::eventTriggerId, ourDevice);
		}
		
	}
}


void cam_thread_main(TaskScheduler* env, int fps)
{
	std::cout << "cam_thread_main_start" << std::endl;
	std::vector<int> encoding_var;
	encoding_var.push_back(CV_IMWRITE_JPEG_QUALITY);
	encoding_var.push_back(70);
	std::vector<uchar> buffer;

	cv::VideoCapture cap(0);
//	cap.set(CV_CAP_PROP_FRAME_WIDTH, 1280); 
//	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 640);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
//	cap.set(CV_CAP_PROP_FRAME_WIDTH, 320);
//	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
	cv::Mat frame;
	cap >> frame;
	int delay_per_frame = 1000 / fps;
	DWORD OldTick, NewTick, StartTick;
	std::chrono::steady_clock::time_point tp;
	std::chrono::duration < int, std::milli> interval(delay_per_frame);
	//std::chrono::steady_clock::duration dur;
//	bool first_time = true;
	tp = std::chrono::steady_clock::now();
	OldTick = GetTickCount();
	StartTick = OldTick;
	while (true)
	{
		
		tp = tp + interval;
//		tp2 = std::chrono::steady_clock::now();
		//std::this_thread::sleep_for(std::chrono::milliseconds(50));
		std::this_thread::sleep_until(tp);


		//std::this_thread::sleep_for(std::chrono::milliseconds(50));

		cap >> frame;
		imencode(".jpg", frame, buffer, encoding_var);

//		std::cout << "Resolution: " << frame.cols << "x" << frame.rows << ", encoded frame size:" << buffer.size() << std::endl;
		{
			// lock the jpeg buffer
			std::unique_lock<std::mutex> locker(lock_jpegbuffer);
			CamThreadFrameBufferSize = buffer.size();
			memcpy(CamThreadFrameBuffer, (char*)&buffer[0], buffer.size());
		}  // unlock the buffer (locker goes out of scope here)
		
		NewTick = GetTickCount();
//		std::cout << "cam_thread_main: signal new data" << /*", gettickcount=" << GetTickCount() <<*/
//				", delta=" << NewTick - OldTick  <<
//				", fromstart=" << (NewTick - StartTick) / 1000.0f <<
//				std::endl;  
		OldTick = NewTick;
		signalNewFrameData(env);
	}

	std::cout << "cam_thread_main_end" << std::endl;
	return;
}
