/*
 * DogDetector.cpp
 *
 *  Created on: Feb 18, 2011
 *      Author: gabriele
 */

#include "dogdetector.h"


using namespace std;


/******************************** Globals ************************************/

char* pname;
char* img_file_name;
char* out_file_name = NULL;
char* out_img_name = NULL;
int intvls = SIFT_INTVLS;
double sigma = SIFT_SIGMA;
double contr_thr = SIFT_CONTR_THR;
int curv_thr = SIFT_CURV_THR;
int img_dbl = SIFT_IMG_DBL;
int descr_width = SIFT_DESCR_WIDTH;
int descr_hist_bins = SIFT_DESCR_HIST_BINS;
int display = 1;

#define OPTIONS ":o:m:i:s:c:r:n:b:dxh"

DogDetector::DogDetector(std::string name) : Module(name) {
	std::cout << "Constructing " << getName() << "...";

	cvGrayImageIn.setModule(this);
	mp_regions = NULL;
	debug = false;

	m_fixBlobSize = true;
	m_fixed_blob_size = 27.5;

	std::cout << "finished.\n";

}

DogDetector::~DogDetector() {
	std::cout << "Destructing " << getName() << "...";

	if(mp_regions) delete mp_regions;

	std::cout << "finished.\n";
}

void DogDetector::setParam(unsigned int cornerCount) {

	m_cornerCount = cornerCount; //maximum number of corners returned

	if(mp_regions) {
			if(mp_regions->getHeight() != m_cornerCount) {
				delete mp_regions;
				mp_regions = NULL;
			}
		}
		if(!mp_regions) mp_regions = new CSCEllipticRegions(m_cornerCount, 5);
		regionsOut.setBuffer(mp_regions);

	}

void DogDetector::execute() {

	CVImage* cvimg = cvGrayImageIn.getBuffer();
	IplImage* img1;
	IplImage* img2;
	IplImage* img3;
	cvNamedWindow("cName",1);
	struct feature* features;


	if(!cvimg) { std::cerr << getName() << "::execute()::ERROR::cvGrayImageIn is NULL!\n"; return; }
	if(cvimg->nChannels != 1) { std::cerr << getName() << "::execute()::ERROR::cvGrayImageIn must be single-channel 8-bit image!\n"; return; }

	img1 = cvimg->ipl;
	img2 = cvCloneImage(img1);
	img3 = cvCloneImage(img1);


	std::cout << getName() << "::execute()::Finding SIFT features\n";
	int n = 0;

	CvRect* newroi = roiIn.getBuffer();
	if(newroi) {
		//if(debug) {
		//	std::cout << getName() << "::execute()::roi = [" << newroi->x << "," << newroi->y << "," << newroi->width << "," << newroi->height << "]\n";
		//}
		cvSetImageROI(img3, *newroi);
	}

	if( ! img3 )
		fatal_error( "unable to load image from %s", img_file_name );
		n = _sift_features( img3, &features, intvls, sigma, contr_thr, curv_thr,
			  img_dbl, descr_width, descr_hist_bins );
		fprintf( stderr, "Found %d features.\n", n );

	 if( display )
	 {
		 draw_features( img2, features, n );
		 cvShowImage("cName",img2);
		 cvWaitKey(33);

	}

		if( out_file_name != NULL )
			export_features( out_file_name, features, n );

		if( out_img_name != NULL )
			cvSaveImage( out_img_name, img2, NULL );
	//int f=export_features( (char*)"SIFTfeature.txt", features, n);

	if(newroi) {
		cvResetImageROI(img3);
	}


	if(n <= 0) {
		mp_regions->setCurrentSize(0);
		regionsOut.out();
		return;
	}

	float* mat = mp_regions->getData();
	unsigned int width = mp_regions->getWidth();
	for(int j = 0; j < n; j++) {
		unsigned int temp = j*width;
		if(newroi) {
			mat[temp] = cvRound( (features + j)->x ) + newroi->x;
			mat[temp + 1] = cvRound( (features + j)->y) + newroi->y;
		}
		else{
			mat[temp] =   cvRound( (features + j)->x );//DOGfeat->data.db[j*5];//((double*)(DOGfeat->data.ptr + DOGfeat->step*j))[0];;
			mat[temp + 1] = cvRound( (features + j)->y );//DOGfeat->data.db[j*5+1];((double*)(DOGfeat->data.ptr + DOGfeat->step*j))[1];
			if(debug){
				printf("feature x = %f \n",mat[temp]);
				printf("feature y = %f \n",mat[temp + 1]);
				//cvWaitKey(0);
			}
		}

		if(m_fixBlobSize) {
			mat[temp + 2] = m_fixed_blob_size;
			mat[temp + 3] = m_fixed_blob_size;
		}
		else {
			//std::cout << getName() << "::execute()::Currently HarrisDetector only detects fixed size blobs\n";
			mat[temp + 2] = m_fixed_blob_size;
			mat[temp + 3] = m_fixed_blob_size;
		}
		mat[temp + 4] = 0.0f;
	}
	mp_regions->setCurrentSize(n);
	regionsOut.out();
	if (features) delete features;
	//cvReleaseImage(&img1);
	cvReleaseImage(&img2);
	cvReleaseImage(&img3);
	//cvDestroyWindow("cName");



}





void DogDetector::executeR() {

	CVImage* cvimg = cvGrayImageIn.getBuffer();
	IplImage* img1;
	IplImage* img2;
	IplImage* img3;
	cvNamedWindow("cNameR",1);
	struct feature* features;


	if(!cvimg) { std::cerr << getName() << "::execute()::ERROR::cvGrayImageIn is NULL!\n"; return; }
	if(cvimg->nChannels != 1) { std::cerr << getName() << "::execute()::ERROR::cvGrayImageIn must be single-channel 8-bit image!\n"; return; }

	img1 = cvimg->ipl;
	img2 = cvCloneImage(img1);
	img3 = cvCloneImage(img1);


	std::cout << getName() << "::execute()::Finding SIFT features\n";
	int n = 0;

	CvRect* newroi = roiIn.getBuffer();
	if(newroi) {
		//if(debug) {
		//	std::cout << getName() << "::execute()::roi = [" << newroi->x << "," << newroi->y << "," << newroi->width << "," << newroi->height << "]\n";
		//}
		cvSetImageROI(img3, *newroi);
	}

	if( ! img3 )
		fatal_error( "unable to load image from %s", img_file_name );
		n = _sift_features( img3, &features, intvls, sigma, contr_thr, curv_thr,
			  img_dbl, descr_width, descr_hist_bins );
		fprintf( stderr, "Found %d features.\n", n );

	 if( display )
	 {
		 draw_features( img2, features, n );
		 cvShowImage("cNameR",img2);
		 cvWaitKey(33);

	}

		if( out_file_name != NULL )
			export_features( out_file_name, features, n );

		if( out_img_name != NULL )
			cvSaveImage( out_img_name, img2, NULL );
	//int f=export_features( (char*)"SIFTfeature.txt", features, n);

	if(newroi) {
		cvResetImageROI(img3);
	}


	if(n <= 0) {
		mp_regions->setCurrentSize(0);
		regionsOut.out();
		return;
	}

	float* mat = mp_regions->getData();
	unsigned int width = mp_regions->getWidth();
	for(int j = 0; j < n; j++) {
		unsigned int temp = j*width;
		if(newroi) {
			mat[temp] = cvRound( (features + j)->x ) + newroi->x;
			mat[temp + 1] = cvRound( (features + j)->y) + newroi->y;
		}
		else{
			mat[temp] =   cvRound( (features + j)->x );//DOGfeat->data.db[j*5];//((double*)(DOGfeat->data.ptr + DOGfeat->step*j))[0];;
			mat[temp + 1] = cvRound( (features + j)->y );//DOGfeat->data.db[j*5+1];((double*)(DOGfeat->data.ptr + DOGfeat->step*j))[1];
			if(debug){
				printf("feature x = %f \n",mat[temp]);
				printf("feature y = %f \n",mat[temp + 1]);
				//cvWaitKey(0);
			}
		}

		if(m_fixBlobSize) {
			mat[temp + 2] = m_fixed_blob_size;
			mat[temp + 3] = m_fixed_blob_size;
		}
		else {
			//std::cout << getName() << "::execute()::Currently HarrisDetector only detects fixed size blobs\n";
			mat[temp + 2] = m_fixed_blob_size;
			mat[temp + 3] = m_fixed_blob_size;
		}
		mat[temp + 4] = 0.0f;
	}
	mp_regions->setCurrentSize(n);
	regionsOut.out();
	if (features) delete features;
	//cvReleaseImage(&img1);
	cvReleaseImage(&img2);
	cvReleaseImage(&img3);
	//cvDestroyWindow("cNameR");



}
