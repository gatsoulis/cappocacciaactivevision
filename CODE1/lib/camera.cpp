/*
 * camera.cpp
 *
 *  Created on: Dec 14, 2010
 *      Author: gabriele
 */

#include "camera.h"

#include <iostream>
#include <sys/stat.h>
using namespace std;


camera::camera(std::string name) : Module(name) {

	std::cout << "\nConstructing " << getName() << "...";

//	captureL = cvCreateFileCapture("/media/Dati/Testing/Video/left.avi");
//	captureR = cvCreateFileCapture("/media/Dati/Testing/Video/right.avi");

	imageL = NULL;
	imageR = NULL;

	mp_leftrgbimage = NULL;
	mp_rightrgbimage = NULL;


	mp_leftgrayimage = NULL;
	mp_rightgrayimage = NULL;

	mp_cvimgL = NULL;
	mp_cvimgR = NULL;

	mp_cvgrayimgL = NULL;
	mp_cvgrayimgR = NULL;

	m_checkfileexistence = false;

	calibKleft = cvCreateMat(3, 3, CV_32FC1);
	calibKright = cvCreateMat(3, 3, CV_32FC1);

	calibDleft = cvCreateMat(5, 1, CV_32FC1);
	calibDright = cvCreateMat(5, 1, CV_32FC1);

	cvSetIdentity(calibKleft);
	cvmSet(calibKleft,0,0,215.4238);
	cvmSet(calibKleft,0,2,170.7683);
	cvmSet(calibKleft,1,1,215.9923);
	cvmSet(calibKleft,1,2,130.5201);

	cvSetIdentity(calibKright);
	cvmSet(calibKright,0,0,210.8715);
	cvmSet(calibKright,0,2,158.7346);
	cvmSet(calibKright,1,1,211.5428);
	cvmSet(calibKright,1,2,122.6226);

	cvSetZero(calibDleft);
	cvmSet(calibDleft,0,0,-0.40254);
	cvmSet(calibDleft,0,1,0.147592);
	cvmSet(calibDleft,0,2,-0.00201);
	cvmSet(calibDleft,0,3,-0.00034);

	cvSetZero(calibDright);
	cvmSet(calibDright,0,0,-0.42333);
	cvmSet(calibDright,0,1,0.188585);
	cvmSet(calibDright,0,2,-0.00415);
	cvmSet(calibDright,0,3,0.007222);

	std::cout << "finished.\n";
}

camera::~camera() {

	std::cout << "\nDestructing " << getName() << "...";

	if(mp_cvimgL) delete mp_cvimgL;
	if(mp_cvimgR) delete mp_cvimgR;

	if (mp_cvgrayimgL) delete mp_cvgrayimgL;
	if (mp_cvgrayimgR) delete mp_cvgrayimgR;

	cvReleaseMat(&calibKleft);
	cvReleaseMat(&calibKright);
	cvReleaseMat(&calibDleft);
	cvReleaseMat(&calibDright);

	/*
	if(mp_leftrgbimage) delete mp_leftrgbimage;
	if(mp_rightrgbimage) delete mp_rightrgbimage;

	if(mp_leftgrayimage) delete mp_leftgrayimage;
	if(mp_rightgrayimage) delete mp_rightgrayimage;
	*/

//	cvReleaseCapture(&captureL);
//	cvReleaseCapture(&captureR);

	std::cout << "finished.\n";
}

void camera::execute(){

	cvCvtColor(imageL,imageL, CV_RGB2BGR);
	cvCvtColor(imageR,imageR, CV_RGB2BGR);

//	imageL = cvQueryFrame( captureL );
//	imageR = cvQueryFrame( captureR );

	//LOAD FROM VIDEO LEFT
//	imageL = cvQueryFrame( captureL);
	if (!imageL) std::cout<< getName() << "::execute()::Error in loading imageL" ;

	//LOAD FROM VIDEO RIGHT
//	imageR = cvQueryFrame( captureR);
	if (!imageR) std::cout<< getName() << "::execute()::Error in loading imgageR" ;

	//CONVERT TO CVIMAGE LEFT
	mp_leftrgbimage = convert2CVImageLeft(imageL);

	//CONVERT TO IMAGE RGB RIGHT
	mp_rightrgbimage = convert2CVImageRight(imageR);

	//SEND CVIMAGE RGB LEFT
	cvLeftImage2Out.setBuffer(mp_leftrgbimage);
	cvLeftImage2Out.out();

	//SEND CVIMAGE RGB RIGHT
	cvRightImage2Out.setBuffer(mp_rightrgbimage);
	cvRightImage2Out.out();

	//CONVERT TO GRAY LEFT
	mp_leftgrayimage  = convert2grayLeft(mp_leftrgbimage);

	//SEND GRAY IMAGE LEFT
	cvLeftGrayImage2Out.setBuffer(mp_leftgrayimage);
	cvLeftGrayImage2Out.out();

	//CONVERT TO GRAY RIGHT
	mp_rightgrayimage = convert2grayRight(mp_rightrgbimage);

	//SEND GRAY IMAGE LEFT
	cvRightGrayImage2Out.setBuffer(mp_rightgrayimage);
	cvRightGrayImage2Out.out();

}

CVImage* camera::convert2CVImageLeft(IplImage* InputImage) {
	if(m_checkfileexistence) {
		if(!InputImage) { std::cout<< getName() << "::convert2CVImage::ERROR::File does not exist!\n"; return NULL; }
		//else if(debug) { std::cout << "convert2CVImage::load()::file [" << filename << "] exists!\n"; }
	}

	IplImage* tempimg = cvCloneImage(InputImage);

	if(!tempimg) { std::cerr<< getName() << "::convert2CVImage::ERROR opening file\n"; return NULL; }

	if(mp_cvimgL) {
		if(mp_cvimgL->width != tempimg->width || mp_cvimgL->height != tempimg->height || mp_cvimgL->ipl->nChannels != tempimg->nChannels || mp_cvimgL->ipl->depth != tempimg->depth) {
			delete mp_cvimgL;
			mp_cvimgL = NULL;
		}
	}

	if(!mp_cvimgL) {

		int cvMatType;
		if(tempimg->nChannels == 3){
			cvMatType = CV_8UC3;
		}
		else if(tempimg->nChannels == 1) {
			cvMatType = CV_8UC1;
		}

		mp_cvimgL = new CVImage(cvSize(tempimg->width, tempimg->height), cvMatType, 0);
		if(!mp_cvimgL) { std::cout<< getName() << "::convert2CVImage::ERROR::Failed to create new CVImage\n"; return NULL; }
	}

	int datasize = (tempimg->widthStep)*(tempimg->height);
	memcpy(mp_cvimgL->ipl->imageData, tempimg->imageData, datasize );

	cvReleaseImage(&tempimg);
	if(tempimg) { std::cout<< getName() << "::convert2CVImage::ERROR::Failed to release IplImage\n"; return NULL; }

	//if(debug) { std::cout << "LoadCVImage::load()::finished\n"; }
	return mp_cvimgL;
}



CVImage* camera::convert2grayLeft(CVImage* input) {
	if(!input) { std::cout<< getName() << "::convert2gray::ERROR::Input is NULL\n"; return NULL; }

	if(input->cvMatType == CV_8UC1) {
		return input;
	}

	if(mp_cvgrayimgL) {
		if(mp_cvgrayimgL->width != input->width || mp_cvgrayimgL->height != input->height) {
			delete mp_cvgrayimgL;
			mp_cvgrayimgL = NULL;
		}
	}

	if(!mp_cvgrayimgL) {
		mp_cvgrayimgL = new CVImage(cvSize(input->width, input->height), CV_8UC1, 0);
		if(!mp_cvgrayimgL) { std::cout<< getName() << "::convert2gray::ERROR::Failed to create new CVImage\n"; return NULL; }
	}

	cvCvtColor(input->ipl, mp_cvgrayimgL->ipl, CV_BGR2GRAY);

	return mp_cvgrayimgL;
}

CVImage* camera::convert2CVImageRight(IplImage* InputImage) {
	if(m_checkfileexistence) {
		if(!InputImage) { std::cout<< getName() << "::convert2CVImage::ERROR::File does not exist!\n"; return NULL; }
		//else if(debug) { std::cout << "convert2CVImage::load()::file [" << filename << "] exists!\n"; }
	}

	IplImage* tempimg = cvCloneImage(InputImage);
	if(!tempimg) { std::cerr<< getName() << "::convert2CVImage::ERROR opening file\n"; return NULL; }

	if(mp_cvimgR) {
		if(mp_cvimgR->width != tempimg->width || mp_cvimgR->height != tempimg->height || mp_cvimgR->ipl->nChannels != tempimg->nChannels || mp_cvimgR->ipl->depth != tempimg->depth) {
			delete mp_cvimgR;
			mp_cvimgR = NULL;
		}
	}

	if(!mp_cvimgR) {

		int cvMatType;
		if(tempimg->nChannels == 3){
			cvMatType = CV_8UC3;
		}
		else if(tempimg->nChannels == 1) {
			cvMatType = CV_8UC1;
		}

		mp_cvimgR = new CVImage(cvSize(tempimg->width, tempimg->height), cvMatType, 0);
		if(!mp_cvimgR) { std::cout<< getName() << "::convert2CVImage::ERROR::Failed to create new CVImage\n"; return NULL; }
	}

	int datasize = (tempimg->widthStep)*(tempimg->height);
	memcpy(mp_cvimgR->ipl->imageData, tempimg->imageData, datasize );

	cvReleaseImage(&tempimg);
	if(tempimg) { std::cout<< getName() << "::convert2CVImage::ERROR::Failed to release IplImage\n"; return NULL; }

	//if(debug) { std::cout << "LoadCVImage::load()::finished\n"; }
	return mp_cvimgR;
}

CVImage* camera::convert2grayRight(CVImage* input) {
	if(!input) { std::cout<< getName() << "::convert2gray::ERROR::Input is NULL\n"; return NULL; }

	if(input->cvMatType == CV_8UC1) {
		return input;
	}

	if(mp_cvgrayimgR) {
		if(mp_cvgrayimgR->width != input->width || mp_cvgrayimgR->height != input->height) {
			delete mp_cvgrayimgR;
			mp_cvgrayimgR = NULL;
		}
	}

	if(!mp_cvgrayimgR) {
		mp_cvgrayimgR = new CVImage(cvSize(input->width, input->height), CV_8UC1, 0);
		if(!mp_cvgrayimgR) { std::cout<< getName() << "::convert2gray::ERROR::Failed to create new CVImage\n"; return NULL; }
	}

	cvCvtColor(input->ipl, mp_cvgrayimgR->ipl, CV_BGR2GRAY);

	return mp_cvgrayimgR;
}
