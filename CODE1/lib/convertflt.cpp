#include "convertflt.h"

using namespace std;


ConvertFlt::ConvertFlt() {
	mp_cvoutputimg = NULL;
}

ConvertFlt::~ConvertFlt() {
	if(mp_cvoutputimg) delete mp_cvoutputimg;
}

CVImage* ConvertFlt::convert(CVImage* img) {

	if(!img) { std::cerr << "ConvertFlt::convert()::ERROR::img is NULL!\n"; return NULL; }

	if(img->cvMatType != CV_8UC1) { std::cerr << "ConvertFlt::convert()::ERROR::img type incorrect!\n"; return NULL; }

	if(mp_cvoutputimg) {
		if(img->width != mp_cvoutputimg->width || img->height != mp_cvoutputimg->height) {
			delete mp_cvoutputimg;
			mp_cvoutputimg = NULL;
		}
	}

	if(!mp_cvoutputimg) {
		mp_cvoutputimg = new CVImage(cvSize(img->width, img->height), CV_32FC1, 0);
	}

	cvConvert(img->ipl, mp_cvoutputimg->ipl);

	return mp_cvoutputimg;
}


