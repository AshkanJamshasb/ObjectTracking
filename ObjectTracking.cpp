/*
 * ObjectTracking.cpp
 *
 *  Created on: Mar 17, 2017
 *      Author: ash
 */

#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv/cv.h"

using namespace cv;
using namespace std;

//These values will get changed with trackbars
int hMin4HSV = 0, sMin4HSV = 0, lMin4HSV = 0, hMax4HSV = 256, sMax4HSV = 256, lMax4HSV = 256;
int hMin4HSL = 0, sMin4HSL = 0, lMin4HSL = 0, hMax4HSL = 256, sMax4HSL = 256, lMax4HSL = 256;
int blurVal = 1;

int x = 0, y = 0;

int const MAX_HSV = 255;
int const MAX_HSL = 255;
int const MAX_BLUR = 31;


bool bothDisplays = false;

const string OG_IMAGE = "Original Image", HSL_IMAGE = "HSL", HSV_IMAGE = "HSV", THRESHOLD_HSL = "Threshold HSL", THRESHOLD_HSV = "Threshold HSV";

//HSV
void lowHSlider4HSV(int, void*);
void highHSlider4HSV(int, void*);
void lowSSlider4HSV(int, void*);
void highSSlider4HSV(int, void*);
void lowVSlider4HSV(int, void*);
void highVSlider4HSV(int, void*);


//HSL
void lowHSlider4HSL(int, void*);
void highHSlider4HSL(int, void*);
void lowSSlider4HSL(int, void*);
void highSSlider4HSL(int, void*);
void lowVSlider4HSL(int, void*);
void highVSlider4HSL(int, void*);

//Blur
void blurImage(int, void*);

//Tracking
void trackFilteredObject(int &x, int &y, Mat threshold, Mat &cam);

//Line
void drawCenterScreen(Mat);

int main (int argc, char* argv[]) {

	//Creating windows
	Mat cam, hsl, hsv, thresholdHSL, thresholdHSV;

	namedWindow(OG_IMAGE, WINDOW_AUTOSIZE);
	namedWindow(HSL_IMAGE, WINDOW_AUTOSIZE);
	namedWindow(THRESHOLD_HSL, WINDOW_AUTOSIZE);

	if(bothDisplays) {
	namedWindow(THRESHOLD_HSV, WINDOW_AUTOSIZE);
	namedWindow(HSV_IMAGE, WINDOW_AUTOSIZE);
	}

	//Create video capture
	VideoCapture capture;
	capture.open(0);

	while((char)waitKey(1)!='q') {

		//Decode and return video frame
		capture.read(cam);


		if(bothDisplays) {
 		//Maintain trackbars
		createTrackbar("H Low", THRESHOLD_HSV, &hMin4HSV, MAX_HSV, lowHSlider4HSV);
		createTrackbar("H High", THRESHOLD_HSV, &hMax4HSV, MAX_HSV, highHSlider4HSV);
		createTrackbar("S Low", THRESHOLD_HSV, &sMin4HSV, MAX_HSV, lowSSlider4HSV);
		createTrackbar("S High", THRESHOLD_HSV, &sMax4HSV, MAX_HSV, highSSlider4HSV);
		createTrackbar("V Low", THRESHOLD_HSV, &lMin4HSV, MAX_HSV, lowVSlider4HSV);
		createTrackbar("V High", THRESHOLD_HSV, &lMax4HSV, MAX_HSV, highVSlider4HSV);
		}

		createTrackbar("H Low", THRESHOLD_HSL, &hMin4HSL, MAX_HSL, lowHSlider4HSL);
		createTrackbar("H High", THRESHOLD_HSL, &hMax4HSL, MAX_HSL, highHSlider4HSL);
		createTrackbar("S Low", THRESHOLD_HSL, &sMin4HSL, MAX_HSL, lowSSlider4HSL);
		createTrackbar("S High", THRESHOLD_HSL, &sMax4HSL, MAX_HSL, highSSlider4HSL);
		createTrackbar("L Low", THRESHOLD_HSL, &lMin4HSL, MAX_HSL, lowVSlider4HSL);
		createTrackbar("L High", THRESHOLD_HSL, &lMax4HSL, MAX_HSL, highVSlider4HSL);

		createTrackbar("Blur Value", OG_IMAGE, &blurVal, MAX_BLUR, blurImage);

		//Conversions for images

		blur(cam, cam, Size(blurVal, blurVal), Point(-1, -1));


		cvtColor(cam, hsl, COLOR_RGB2HLS);

		if(bothDisplays)
		cvtColor(cam, hsv, COLOR_RGB2HSV);



		inRange(hsl, Scalar(hMin4HSL, sMin4HSL, lMin4HSL), Scalar(hMax4HSL, sMax4HSL, lMax4HSL), thresholdHSL);
//		inRange(hsl, Scalar(0, 84, 75), Scalar(57, 172, 168), thresholdHSL);

		if(bothDisplays)
		inRange(hsv, Scalar(hMin4HSV, sMin4HSV, lMin4HSV), Scalar(hMax4HSV, sMax4HSV, lMax4HSV), thresholdHSV);

		trackFilteredObject(x, y, thresholdHSL, cam);

		drawCenterScreen(cam);

		imshow(OG_IMAGE, cam);
		imshow(HSL_IMAGE, hsl);

		if(bothDisplays)
		imshow(HSV_IMAGE, hsv);

		imshow(THRESHOLD_HSL, thresholdHSL);

		if(bothDisplays)
		imshow(THRESHOLD_HSV, thresholdHSV);


	}

	return 0;
}

void drawCenterScreen(Mat cam) {
	line(cam, Point(320, 480), Point(320, 0), Scalar(0,0,255), 2);
}

void drawObject(int &x, int &y, Mat cam) {
	circle(cam,Point(x,y),20,Scalar(0,255,0),2);
    if(y-25>0)
    line(cam,Point(x,y),Point(x,y-25),Scalar(0,255,0),2);
    else line(cam,Point(x,y),Point(x,0),Scalar(0,255,0),2);
    if(y+25<480)
    line(cam,Point(x,y),Point(x,y+25),Scalar(0,255,0),2);
    else line(cam,Point(x,y),Point(x,480),Scalar(0,255,0),2);
    if(x-25>0)
    line(cam,Point(x,y),Point(x-25,y),Scalar(0,255,0),2);
    else line(cam,Point(x,y),Point(0,y),Scalar(0,255,0),2);
    if(x+25<640)
    line(cam,Point(x,y),Point(x+25,y),Scalar(0,255,0),2);
    else line(cam,Point(x,y),Point(640,y),Scalar(0,255,0),2);
}

void trackFilteredObject(int &x, int &y, Mat threshold, Mat &cam) {

	Mat temp;
	threshold.copyTo(temp);

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	findContours(temp, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE);


	double refArea = 0;
	bool objectFound = false;

	if(hierarchy.size() > 0) {
		int numObject = hierarchy.size();

		if(numObject<100) {
			for(int i = 0; i >= 0; i = hierarchy[i][0]){

				Moments moment = moments((cv::Mat)contours[i]);
				double area = moment.m00;

				if(area>1000 && area < 150000) {
//More about moments
// http://docs.opencv.org/2.4/modules/imgproc/doc/structural_analysis_and_shape_descriptors.html?highlight=moments#moments
					x = moment.m10/area;
					circle(cam, Point(moment.m10, moment.m01), 20, Scalar(0,255,0), 2);

					y = moment.m01/area;
					objectFound = true;
					refArea = area;
				} else  {
					objectFound = false;
				}
				if (objectFound) {

				    std::ostringstream ss;
				    ss << area;
				    drawObject(x,y,cam);
				 	drawContours(cam, contours, -1, Scalar(0,255,0), 3);
					putText(cam, ss.str(), Point(0,50),2,1,Scalar(0,225,0),2);
				} else {
	//				putText(cam, "No Target Lock", Point(0,50),2,1,Scalar(0,225,0),2);
				}
			}
		}
	}
}





//Update sliders for hsl
void lowHSlider4HSL(int, void*) {
	hMin4HSL = min(hMax4HSL-1, hMin4HSL);
	setTrackbarPos("H Low", THRESHOLD_HSL, hMin4HSL);
}
void highHSlider4HSL(int, void*) {
	hMax4HSL = max(hMax4HSL, hMin4HSL+1);
	setTrackbarPos("H High", THRESHOLD_HSL, hMax4HSL);
}
void lowSSlider4HSL(int, void*) {
	sMin4HSL = min(sMax4HSL-1, sMin4HSL);
	setTrackbarPos("S Low", THRESHOLD_HSL, sMin4HSL);
}
void highSSlider4HSL(int, void*) {
	sMax4HSL = max(sMax4HSL, sMin4HSL+1);
	setTrackbarPos("S High", THRESHOLD_HSL, sMax4HSL);
}
void lowVSlider4HSL(int, void*) {
	lMin4HSL = min(lMax4HSL-1, lMin4HSL);
	setTrackbarPos("L Low", THRESHOLD_HSL, lMin4HSL);
}
void highVSlider4HSL(int, void*) {
	lMax4HSL = max(lMax4HSL, lMin4HSL+1);
	setTrackbarPos("L Max", THRESHOLD_HSL, lMax4HSL);
}

//HSV values
void lowHSlider4HSV(int, void*) {
	hMin4HSV = min(hMax4HSV-1, hMin4HSV);
	setTrackbarPos("H Low", THRESHOLD_HSV, hMin4HSV);
}
void highHSlider4HSV(int, void*) {
	hMax4HSV = max(hMax4HSV, hMin4HSV+1);
	setTrackbarPos("H High", THRESHOLD_HSV, hMax4HSV);
}
void lowSSlider4HSV(int, void*) {
	sMin4HSV = min(sMax4HSV-1, sMin4HSV);
	setTrackbarPos("S Low", THRESHOLD_HSV, sMin4HSV);
}
void highSSlider4HSV(int, void*) {
	sMax4HSV = max(sMax4HSV, sMin4HSV+1);
	setTrackbarPos("S High", THRESHOLD_HSV, sMax4HSV);
}
void lowVSlider4HSV(int, void*) {
	lMin4HSV = min(lMax4HSV-1, lMin4HSV);
	setTrackbarPos("V Low", THRESHOLD_HSV, lMin4HSV);
}
void highVSlider4HSV(int, void*) {
	lMax4HSV = max(lMax4HSV, lMin4HSV+1);
	setTrackbarPos("V Max", THRESHOLD_HSV, lMax4HSV);
}

//Blur value
void blurImage(int, void*) {
	setTrackbarPos("Blur Value", OG_IMAGE, (blurVal<1)?blurVal=1:blurVal);
}
