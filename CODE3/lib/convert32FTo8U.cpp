#include "convert32FTo8U.h"



Convert32FTo8U::Convert32FTo8U(std::string name) : Module(name) {

	cvImageIn.setModule(this);

	//mp_cvimg32f = NULL;
	mp_cvimg8u = NULL;

	debug = false;

	//std::cout << "finished.\n";
}

Convert32FTo8U::~Convert32FTo8U() {
	//std::cout << "Destructing " << getName() << "...";
	//if(mp_cvimg32f) delete mp_cvimg32f;
	if(mp_cvimg8u) delete mp_cvimg8u;
	//std::cout << "finished.\n";
}

void Convert32FTo8U::execute() {
	CVImage* cvimg = cvImageIn.getBuffer();
	if(!cvimg) { if(debug) std::cerr << getName() << "::ERROR::cvImageIn is NULL!\n"; cvImageOut.setBuffer(NULL); cvImageOut.out(); return; }
	if(cvimg->cvMatType != CV_32FC1) { if(debug) std::cerr << getName() << "::ERROR::cvImageIn has incorrect type (must be CV_32FC1)!\n"; cvImageOut.setBuffer(NULL); cvImageOut.out(); return; }

	if(!mp_cvimg8u){
		//mp_cvimg32f = new CVImage(cvSize(cvimg->width, cvimg->height), CV_32FC1, 0);
		mp_cvimg8u = new CVImage(cvSize(cvimg->width, cvimg->height), CV_8UC1, 0);
	}

	IplImage* img = cvimg->ipl;

	double minval, maxval;
	cvMinMaxLoc(img, &minval, &maxval, NULL, NULL, NULL);

	double scale, shift;
	
	if(maxval == minval) { scale = 255.0; shift = 0.0; }
	else {
		scale = 255.0 / (maxval - minval);
		shift = - minval * scale;
	}
	
	cvConvertScale(img, mp_cvimg8u->ipl, scale, shift);
	
	cvImageOut.setBuffer(mp_cvimg8u);
	cvImageOut.out();
}

