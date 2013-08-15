#include "camera.h"
#include <iostream>
#include <stdio.h>
#include <string>
#include <sys/time.h>
#include <limits>
#include <libraw1394/raw1394.h>

static void resetBus() {
	//cleanup primitive resources. required if the last user abruptly
	//terminated. this method isn't ideal since it just resets the first bus,
	//even though there might be more (there aren't on our robot, though).
	raw1394handle_t handle = raw1394_new_handle();
	if(!handle) {
		CV_Error(CV_StsError, "unable to open raw1394 handle (is there a bus available on this computer?)");
	}
	const int numPorts = raw1394_get_port_info(handle, NULL, 0);
	if(numPorts < 1) {
		raw1394_destroy_handle(handle);
		CV_Error(CV_StsError, "no raw1394 ports");
	}
	raw1394_set_port(handle, 0);
	raw1394_reset_bus(handle);
	raw1394_destroy_handle(handle);
}

Camera::Camera()
:contours(0),adjustment(0),erosion(2),dilation(5),resWidth(1024),resHeight(768),redRange(70),greenRange(20),blueRange(70)
{
	resetBus();
	/* Setup camera capture, grab first frame, and create resultant image */
	storage = cvCreateMemStorage(0);
	capture = cvCaptureFromCAM(0);

	// 160 x 120 is the one that always works
	// 1024 x 768 is ideal

	// Set the resolution of the picture to be taken
	cvSetCaptureProperty(capture,CV_CAP_PROP_FRAME_WIDTH,resWidth);
	cvSetCaptureProperty(capture,CV_CAP_PROP_FRAME_HEIGHT,resHeight);

	//cvSetCaptureProperty(capture,CV_CAP_PROP_AUTO_EXPOSURE, (double)false);
	//std::cout << "auto exposure: " << cvGetCaptureProperty(capture,CV_CAP_PROP_AUTO_EXPOSURE) << "\n";

	//cvSetCaptureProperty(capture,CV_CAP_PROP_EXPOSURE, (double)-10);
	//std::cout << "Exposure value: " << cvGetCaptureProperty(capture,CV_CAP_PROP_EXPOSURE) << "\n";

//	std::cout << "Taking a "<< cvGetCaptureProperty(capture,CV_CAP_PROP_FRAME_WIDTH) << 
//		"x" << cvGetCaptureProperty(capture,CV_CAP_PROP_FRAME_HEIGHT) << " picture.\n";
//	std::cout << "Erodes set to " << erosion << " and dilation set to " << dilation << ".\n";

	// Attempt to take a picture
	frame = cvQueryFrame(capture);

	if(!frame) {
		CV_Error(CV_StsError, "failed to take initial picture");
	}

	// Create a result as well as contour image
	result	= cvCreateImage(cvGetSize(frame), 8, 1);
	contourimage = cvCreateImage(cvGetSize(frame), 8, 1);
}
Camera::Camera(int rr,int gr,int br,int erosion,int dilation)
:contours(0),adjustment(0),erosion(erosion),dilation(dilation),resWidth(1024),resHeight(768),redRange(rr),greenRange(gr),blueRange(br)
{
	resetBus();
	/* Setup camera capture, grab first frame, and create resultant image */
	storage = cvCreateMemStorage(0);
	capture = cvCaptureFromCAM(0);

	// 160 x 120 is the one that always works
	// 1024 x 768 is ideal

	// Set the resolution of the picture to be taken
	cvSetCaptureProperty(capture,CV_CAP_PROP_FRAME_WIDTH,resWidth);
	cvSetCaptureProperty(capture,CV_CAP_PROP_FRAME_HEIGHT,resHeight);

	//cvSetCaptureProperty(capture,CV_CAP_PROP_AUTO_EXPOSURE, (double)false);
	//std::cout << "auto exposure: " << cvGetCaptureProperty(capture,CV_CAP_PROP_AUTO_EXPOSURE) << "\n";

	//cvSetCaptureProperty(capture,CV_CAP_PROP_EXPOSURE, (double)-10);
	//std::cout << "Exposure value: " << cvGetCaptureProperty(capture,CV_CAP_PROP_EXPOSURE) << "\n";

//	std::cout << "Taking a "<< cvGetCaptureProperty(capture,CV_CAP_PROP_FRAME_WIDTH) << 
//		"x" << cvGetCaptureProperty(capture,CV_CAP_PROP_FRAME_HEIGHT) << " picture.\n";
//	std::cout << "Erodes set to " << erosion << " and dilation set to " << dilation << ".\n";

	// Attempt to take a picture
	frame = cvQueryFrame(capture);

	if(!frame) {
		CV_Error(CV_StsError, "failed to take initial picture");
	}

	// Create a result as well as contour image
	result	= cvCreateImage(cvGetSize(frame), 8, 1);
	contourimage = cvCreateImage(cvGetSize(frame), 8, 1);
}
// Primary function
double Camera::getCameraHeading(bool &coneExists)
{
	coneExists = 0;
	// Take a picture
	frame = cvQueryFrame(capture);


	//time_t timeval;


	// Set up
	data = (uchar *)frame->imageData;
	datar = (uchar *)result->imageData;

	// Save the initial picture
	//cvSaveImage("picture.jpeg",frame);
	// r 255
	// g 117
	// b 0

	int idealRed = 255;
	int idealGreen = 117;
	int idealBlue = 10;

	//int redRange = 150;
	//int greenRange = 20;
	//int blueRange = 60;	// need 100 for sun directly behind cone


	//  pixel must have a r value > idealRed - redRange
	//                  a g value < idealGreen + greenRange
	//		    a b value < idealBlue + blueRange


	// Iterate through every pixel looking for rgb values within each range
	for(int i = 0; i < (frame->height); i++) {
		for(int j = 0; j < (frame->width); j++) {
			if((data[i*frame->widthStep+j*frame->nChannels+2] > (idealRed-redRange)) && 		// red value > 255-125
			   (data[i*frame->widthStep+j*frame->nChannels+1] < (idealGreen+greenRange)) && 	// green value < 117+40
			   (data[i*frame->widthStep+j*frame->nChannels]   < (idealBlue+blueRange))) 		// blue value < 0 + 100
				datar[i*result->widthStep+j*result->nChannels] = 255;
			else
				datar[i*result->widthStep+j*result->nChannels] = 0;
		}
	}



	//std::cout << "Color change complete.\n";

	/* Apply erosion and dilation to eliminate some noise and even out blob */
	if(erosion >= 0) {
		cvErode(result,result,0,erosion);
	}
	if(dilation >= 0) {
		cvDilate(result,result,0,dilation);
	}

	//std::cout << "Erosion and dilation complete.\n";

	/* FindContours should not alter result (its const in the function declaration), but it does...
	This function looks for contours (edges of polygons) on the already monochrome image */
	cvFindContours(result,storage,&contours);

	/* Draw the contours on contourimage */
	if(contours) {
		cvDrawContours(contourimage,contours,cvScalarAll(255),cvScalarAll(255),100);
	}

	//std::cout << "Contour drawing complete.\n";

	//time(&timeval);
	//std::string filename("boxes.jpeg");
	//filename = filename + ctime(&timeval);
	//cvSaveImage(filename.c_str(),contourimage);
//	cvSaveImage("boxes.jpeg",contourimage);




	//std::cout << "Countour image saved.\n";

	/* Calculate the bounding rectangle */
	bound = cvBoundingRect(contourimage,0);

	//std::cout << "Bounding rectangle computed.\n";

	/* Reset the contourimage image (otherwise contourimage turns into an Etch A Sketch) */
	if(contours) {
		//delete contours;
		//contours = new CvSeq;
		//cvZero(contours);
		cvClearSeq(contours);
	}

	cvZero(contourimage);

	//std::cout << "Countour image zeroed.\n";

	/* Calculate the bounding rectangle's top-left and bottom-right vertex */
	p1.x = bound.x;
	p2.x = bound.x + bound.width;
	p1.y = bound.y;
	p2.y = bound.x + bound.height;

	//std::cout << "Bound calculations complete.\n";





	/* Check if there is a rectangle in frame */
	if (p1.x == 0 && p1.y == 0) {


		//cvSaveImage("picture.jpeg",frame);
		cvReleaseCapture(&capture);
//		adjustment = std::numeric_limits<double>::quiet_NaN();
		adjustment = 0;
		coneExists = 0;
		return adjustment;

	} else {


		// Draw the bounding rectangle on the original image
		cvRectangle(frame,p1,p2,CV_RGB(255,0,0),3,8,0);

		// Calculate where the center of the rectangle would be
		// Add half of the bounding rectangle's width to the top-left point's x-coordinate
		p1.x = bound.x + (bound.width/2);

		// Add half of the difference between top and bottom edge to the bottom edge
		p1.y = p2.y + ((p1.y - p2.y)/2);

		// Draw a small circle at the center of the bounding rectangle
		cvCircle(frame,p1,3,CV_RGB(0,0,255),1,8,0);



		/* Check if there is a rectangle in frame */
		double fieldDegrees = 43.3;
		double halfField = fieldDegrees/2;
		adjustment = (double)p1.x;
		adjustment = adjustment/frame->width;
		adjustment = adjustment*fieldDegrees;
		adjustment = adjustment-halfField;
		if(adjustment == -0)
				adjustment = 0;




		cvZero(result);

	//	cvSaveImage("picture.jpeg",frame);


		cvReleaseCapture(&capture);

		coneExists = 1;


		return adjustment;
	}
}
