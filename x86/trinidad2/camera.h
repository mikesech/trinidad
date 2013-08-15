#ifndef CAMERA_H
#define CAMERA_H

#include "cv.h"
#include "highgui.h"

class Camera {
	public:
		/**
		 * Constructor.
		 * Sets up the camera
		 */
		Camera();
		// Same thing but with different rgb ranges
		Camera(int rr,int gr, int br, int erosion, int dilation);
		/**
		 * .
		 *
		 */
		double getCameraHeading(bool &coneExists);
		int takePicture();
		CvCapture* capture;

		//void RGBtoHSV( float r, float g, float b, float *h, float *s, float *v );
	private:

		CvSeq* contours;
		double adjustment;


		int erosion;
		int dilation;

		int resWidth;
		int resHeight;


		int redRange;
		int greenRange;
		int blueRange;

		uchar *data,*datar;
		CvMemStorage* storage;
//		CvCapture* capture;
		CvRect bound;
		CvPoint p1,p2;
		IplImage* frame;
		IplImage* result;
		IplImage* contourimage;


};
#endif
