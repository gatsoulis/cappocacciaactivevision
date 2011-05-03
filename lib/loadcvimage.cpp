#include "loadcvimage.h"
#include <sys/stat.h>


LoadCVImage::LoadCVImage() {
	mp_cvimg = NULL;
	m_checkfileexistence = false;
	//debug = true;
}

LoadCVImage::~LoadCVImage() {
	if(mp_cvimg) delete mp_cvimg;
}

///ORIGINAL
//CVImage* LoadCVImage::load(const std::string& filename) {
CVImage* LoadCVImage::load(const std::string& filename) {
	if(m_checkfileexistence) {
		struct stat buffer;
		if(stat(filename.c_str(), &buffer)) { std::cerr << "LoadCVImage::load()::ERROR::File [" << filename << "] does not exist!\n"; return NULL; }
		//else if(debug) { std::cout << "LoadCVImage::load()::file [" << filename << "] exists!\n"; }
	}

	IplImage* tempimg = cvLoadImage( filename.c_str(), -1);
	if(!tempimg) { std::cerr << "LoadCVImage::load()::ERROR::ERROR opening file [" << filename << "]\n"; return NULL; }

	if(mp_cvimg) {
		if(mp_cvimg->width != tempimg->width || mp_cvimg->height != tempimg->height || mp_cvimg->ipl->nChannels != tempimg->nChannels || mp_cvimg->ipl->depth != tempimg->depth) {
			delete mp_cvimg;
			mp_cvimg = NULL;
		}
	}

	if(!mp_cvimg) {

		int cvMatType;
		if(tempimg->nChannels == 3){
			cvMatType = CV_8UC3;
		}
		else if(tempimg->nChannels == 1) {
			cvMatType = CV_8UC1;
		}

		mp_cvimg = new CVImage(cvSize(tempimg->width, tempimg->height), cvMatType, 0);
		if(!mp_cvimg) { std::cerr << "LoadCVImage::load()::ERROR::Failed to create new CVImage\n"; return NULL; }
	}

	int datasize = (tempimg->widthStep)*(tempimg->height);
	memcpy(mp_cvimg->ipl->imageData, tempimg->imageData, datasize );

	cvReleaseImage(&tempimg);
	if(tempimg) { std::cerr << "LoadCVImage::load()::ERROR::Failed to release IplImage\n"; return NULL; }

	//if(debug) { std::cout << "LoadCVImage::load()::finished\n"; }
	return mp_cvimg;
}

CVImage* LoadCVImage::load(IplImage* InputImage) {
	if(m_checkfileexistence) {

		if(!InputImage) { std::cerr << "LoadCVImage::load()::ERROR::File does not exist!\n"; return NULL; }
		//else if(debug) { std::cout << "LoadCVImage::load()::file [" << filename << "] exists!\n"; }
	}	

	IplImage* tempimg = cvCloneImage(InputImage);
	if(!tempimg) { std::cerr << "LoadCVImage::load()::ERROR::ERROR opening file ]\n"; return NULL; }

	if(mp_cvimg) {
		if(mp_cvimg->width != tempimg->width || mp_cvimg->height != tempimg->height || mp_cvimg->ipl->nChannels != tempimg->nChannels || mp_cvimg->ipl->depth != tempimg->depth) {
			delete mp_cvimg;
			mp_cvimg = NULL;
		}
	}

	if(!mp_cvimg) {
	
		int cvMatType;
		if(tempimg->nChannels == 3){
			cvMatType = CV_8UC3;
		}
		else if(tempimg->nChannels == 1) {
			cvMatType = CV_8UC1;
		}
	
		mp_cvimg = new CVImage(cvSize(tempimg->width, tempimg->height), cvMatType, 0);
		if(!mp_cvimg) { std::cerr << "LoadCVImage::load()::ERROR::Failed to create new CVImage\n"; return NULL; }
	}

	int datasize = (tempimg->widthStep)*(tempimg->height);
	memcpy(mp_cvimg->ipl->imageData, tempimg->imageData, datasize );

	cvReleaseImage(&tempimg);
	if(tempimg) { std::cerr << "LoadCVImage::load()::ERROR::Failed to release IplImage\n"; return NULL; }

	//if(debug) { std::cout << "LoadCVImage::load()::finished\n"; }
	return mp_cvimg;
}

CVImage* LoadCVImage::loadfromcamera(IplImage* image){

	IplImage* tempimg = cvCloneImage(image);
	if(!tempimg) { std::cerr << "LoadCVImage::load()::ERROR::ERROR loading tempimg\n"; return NULL; }

	if(mp_cvimg) {
		if(mp_cvimg->width != tempimg->width || mp_cvimg->height != tempimg->height || mp_cvimg->ipl->nChannels != tempimg->nChannels || mp_cvimg->ipl->depth != tempimg->depth) {
			delete mp_cvimg;
			mp_cvimg = NULL;
		}
	}

	if(!mp_cvimg) {

		int cvMatType;
		if(tempimg->nChannels == 3){
			cvMatType = CV_8UC3;
		}
		else if(tempimg->nChannels == 1) {
			cvMatType = CV_8UC1;
		}

		mp_cvimg = new CVImage(cvSize(tempimg->width, tempimg->height), cvMatType, 0);
		if(!mp_cvimg) { std::cerr << "LoadCVImage::load()::ERROR::Failed to create new CVImage\n"; return NULL; }
	}

	int datasize = (tempimg->widthStep)*(tempimg->height);
	memcpy(mp_cvimg->ipl->imageData, tempimg->imageData, datasize );

	cvReleaseImage(&tempimg);
	cvReleaseImage(&image);
	if(tempimg) { std::cerr << "LoadCVImage::load()::ERROR::Failed to release tempimg\n"; return NULL; }
	if(image) { std::cerr << "LoadCVImage::load()::ERROR::Failed to release image\n"; return NULL; }

	//if(debug) { std::cout << "LoadCVImage::load()::finished\n"; }
	return mp_cvimg;

}
