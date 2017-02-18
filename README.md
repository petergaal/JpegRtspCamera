# JpegRtspCamera - RTSP server streaming MJPEG video from PC camera


###A server side demo project, using live555 and OpenCV library.

This project was originally written in 2015, making a basic framework for an intelligent camera running on Linux, developed and debugged first in Visual Studio 2013, then transferred to Linux. Haven't tested the Linux version yet but will test it later and make build files also for Linux. At least I updated the project files to Visual Studio 2015 and verified if everything works.


It is quite hard to start with any similar project as there are very few resources on the internet about it. So I decided to make this project available on GitHub to help other developers with quick start on a similar project.

You will need to download OpenCV library, preferably 2.4.13 and unpack it. The installer is available [here](https://sourceforge.net/projects/opencvlibrary/files/opencv-win/2.4.13/opencv-2.4.13.exe/download). On my installation I unpacked it into folder "D:\OpenCV2.4.13" so if you will unpack somewhere else you'll need to update the project settings in Visual Studio (include and library paths). The project should work with any 2.4 OpenCV version, at least I used it with 2.4.9, 2.4.10 and 2.4.10 in the past without problems. I am not sure if it will work with OpenCV 3.0.X version or newer, but you can try and tell me.

OpenCV is used here as an image processing library. It's possible to use something else in your project for grabbing a video stream, some USB (or other) cameras will support jpeg and H.264 directly so maybe you can grab it using DirectShow (on Windows) or V4L2 (on Linux). Here the goal of the project is different, you can grab a raw image, do some manipulation (transformation, put a text or logo, etc.) on it with OpenCV, then compress with image encoder and send this out to the RTSP part. But if you need just to broadcast image directly without modification then DirectShow/V4L2 is a good option which will save you CPU time.

Other library used here is live555, known library for RTSP. Live555 is also used in VLC. You don't need to download as this is part of the project directly with source code.

After installing OpenCV you can compile the project and everything should work. Make sure you have a working camera device on your system, otherwise the program will fail. The program takes on parameter where you specify the frame rate (in units of frames per second).

```
JpegRtspCamera.exe <fps>
```

Currently the resolution for video is 640x480 but you can change it to something else (at least from the values which are supported by the camera or library).If the frames are too big make sure you will allocate bigger buffer in the source code (CameraJPEGDeviceSource.cpp, MAX_SIZE, currently 256kB).

### Testing

Run the program with desired frame rate. After starting it will print the current RTSP url, for example:
```
rtsp://192.168.1.1/jpeg
```
. You can open this URL in VLC, using Media, Open Network Stream... from the main menu. Enter url, press play. You can open more VLC clients, I tested it with 5 player session and it worked without problem.
