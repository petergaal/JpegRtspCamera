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


//
// Class: MJPEGVideoSource
#ifndef _MJPEGVIDEOSOURCE_H_
#define _MJPEGVIDEOSOURCE_H_

// livemedia
#include <GroupsockHelper.hh>
#include <liveMedia.hh>
#include <JPEGVideoSource.hh>

// opencv
//#include <cv.h>
//#include <highgui.h>

#include <time.h>

//#include "OStreamConfig.hh"
//#include "BaseInputVideo.hh"
//#include "OStreamVideoEncoder.hh"

/**
 * Jpeg used to stream with liveMedia should have this default header size.
 * Other formats, even if conforming JPEG/JFIF standards, will not be used.
 * For example ImageMagick library uses "wrong" [and dynamic] header size. This 
 * requirement is done because the JPEGVideoRTPSink reconstructor needs
 * an header like this.
 */
#define JPEG_HEADER_MAX_SIZE 623

/**
 * Set a default maximum jpeg size of 500Kb bytes. Should fit greatly
 * a 640*480 stream. If you need more increase this.
 */
#define OUTPUT_PACKET_BUFFER_MAXSIZE	512000

/**
 * MJPEG video encapsulator.
 * This class will take IplImages from a video source and then will encode them into
 * JPEG images. Concatenating them will result in a Motion-JPEG stream. 
 * This class will subclass the event-driven library LiveMedia.
 * Derives JPEGVideoSource.
 */
class MJPEGVideoSource : public JPEGVideoSource
{
public:
	/**
	 * Static method.
	 * @param env UsageEnvironment, see LiveMedia doc. here
	 *        http://www.live555.com/liveMedia/public/doxygen/html
	 * @param input_video A BaseInputVideo class, can be a MultiVideoSource or a 
	 *        SingleVideoSource kind of class.
	 * @param stream_id Stream id that will identify this MJPEGVideoSource 
	 *        incapsulator class. It defaults to 0.
	 * @param frame_rate Frames Per Second value. Number of frames per time unit.
	 * @param encoder_internal_buf_size Encoder internal buffer size. Memory to alloc
	 *        for encoding purposes. Defaults to 1000000.
	 * @return New instance of MJPEGVideoSource.
	 * @see H263plusVideoSource
	 */
	static MJPEGVideoSource*
		createNew( UsageEnvironment& env, /*BaseInputVideo* input_video, */
				  int stream_id = 0, int frame_rate = 25, 
				  int encoder_internal_buf_size = 1000000 );

	virtual ~MJPEGVideoSource();
	
protected:
	/**
	 * Constructor.
	 * @see createNew
	 */
	MJPEGVideoSource( UsageEnvironment& env, /*BaseInputVideo* input_video, */
					  int stream_id, int frame_rate, int encoder_internal_buf_size);
	
private:		// functions
	/**
 	 * Inherited function.\  Used to process next frame to send.
 	 */
	void doGetNextFrame();
	
	/** Provides RTP encapsulation. */	
	void deliverFrame();
	
	/** Captures a IplImage and encode it to a JPEG memory buffer. */	
	void  startCapture();

	/**
	 * @return Type of the image. Usually it's 1 ( = JPEG)
	 */
	u_int8_t type();
	
	/**
	 * Quality factor of JPEG image.
	 * @return ffmpeg needs 195 to be returned.
	 */
	u_int8_t qFactor();
	
	/**
	 * Width of image.
	 * @return Width of image.
	 */
	u_int8_t width();			// # pixels/8 (or 0 for 2048 pixels)
	
	/**
	 * Height of image.
	 * @return Height of image.
	 */
	u_int8_t height();			// # pixels/8 (or 0 for 2048 pixels)

	/**
	 * If "qFactor()" returns a value >= 128, then this function is called
	 * to tell us the quantization tables that are being used.
	 * (The default implementation of this function just returns NULL.) 
	 * "precision" and "length" are as defined in RFC 2435, section 3.1.8.
	 * @return Quantization tables that are being used
	 */ 
	u_int8_t const* quantizationTables( u_int8_t& precision, u_int16_t& length );
	
	
	/**
	 * Try to find width and height markers and grab their values.
	 * @param fJPEGHeader JPEG memory buffer. Obviously the image header must be inside
	 *        that buffer.
	 * @return -1 on error, >= 0 if went fine.
	 */	
	int setParamsFromHeader( const unsigned char* fJPEGHeader );
	
	/** redefined virtual functions */
	Boolean isJPEGVideoSource() const;
	
private:		// data	
	/** Specify whether the class is initialized or not. */	
	bool _initialized;

	/* Time per frame */
	unsigned fTimePerFrame;
	
	/** Preferred size of current frame to send */
	unsigned int fPreferredFrameSize;

	/** 
	 * It will just hold the pointer of the internal_buffer of OStreamVideoEncoder.
	 * In this particular MJPEG case it will contain jpeg header+data before it's sending
	 */
	//const unsigned char* _encoded_frame;
	unsigned char _encoded_frame[256*1024];
	
	/** Contains the last capture time. Useful for internal packet ordering. */ 
	struct timeval fLastCaptureTime;
	
	/** Frames per second. */		
	int _fps;
	
	/** Equals at 1000/frame_rate */		
	int _frame_rate_millis;
	
	/** Quality factor, width and height of the jpeg image */
	u_int8_t fLastQFactor, fLastWidth, fLastHeight;
	
	/** Quantization tables */
	u_int8_t *qTable;

	/** Size of the internal buffer used for encoding. */
	int _encoder_internal_buf_size;
	
	/** Stream id. */	
	int _stream_id;
	
	/** BaseInputVideo class instance pointer. */	
//	BaseInputVideo *_input_video;
	
	/** OStreamVideoEncoder class instance pointer. */	
//	OStreamVideoEncoder *_video_encoder;	   
	
#ifdef __LINUX__
	struct timezone Idunno;	
#else
	int Idunno;
#endif
	
	
};


#endif	//_MJPEGVIDEOSOURCE_H_

