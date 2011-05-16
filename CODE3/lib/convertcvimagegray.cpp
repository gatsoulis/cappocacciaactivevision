#include "convertcvimagegray.h"



ConvertCVImageGray::ConvertCVImageGray() {
	mp_cvgrayimg = NULL;
}

ConvertCVImageGray::~ConvertCVImageGray() {
	if (mp_cvgrayimg) delete mp_cvgrayimg;
}

CVImage* ConvertCVImageGray::convert(CVImage* input) {
	if(!input) { std::cerr << "ConvertCVImageGray::load()::ERROR::Input is NULL\n"; return NULL; }

	if(input->cvMatType == CV_8UC1) {
		return input;
	}

	if(mp_cvgrayimg) {
		if(mp_cvgrayimg->width != input->width || mp_cvgrayimg->height != input->height) {
			delete mp_cvgrayimg;
			mp_cvgrayimg = NULL;
		}
	}

	if(!mp_cvgrayimg) {
		mp_cvgrayimg = new CVImage(cvSize(input->width, input->height), CV_8UC1, 0);
		if(!mp_cvgrayimg) { std::cerr << "ConvertCVImageGray::load()::ERROR::Failed to create new CVImage\n"; return NULL; }
	}

	cvCvtColor(input->ipl, mp_cvgrayimg->ipl, CV_BGR2GRAY);

	return mp_cvgrayimg;
}
