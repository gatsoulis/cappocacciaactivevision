#include "cropresize.h"


CropResize::CropResize(std::string name) : Module(name){
//	std::cout<< "Constructing " << getName() << "...";

	cvImageIn.setModule(this);
//	rectIn.setModule(this);
//	sizeIn.setModule(this);

	mp_cvimage = NULL;
	m_outputsize.width = -1;
	m_outputsize.height = -1;

	m_interpolation = CV_INTER_NN;

	debug = false;

//	std::cout<< "finished.\n";
}

CropResize::~CropResize(){
//	std::cout<< "Destructing " << getName() << "...";
	if(mp_cvimage != NULL) delete mp_cvimage;
//	std::cout<< "finished.\n";
}

void CropResize::execute() {

	CVImage* cvimg = cvImageIn.getBuffer();
	if(!cvimg) { std::cerr << getName() << "::execute()::ERROR::cvImageIn is NULL!...\n"; return; }

	CvRect* inputrect = rectIn.getBuffer();
	if(inputrect) {
		m_inputrect = *inputrect;
	}
	else{
		m_inputrect.x = 0;
		m_inputrect.y = 0;
		m_inputrect.width = cvimg->width;
		m_inputrect.height = cvimg->height;
	}

	if(m_inputrect.x < 0) m_inputrect.x = 0;
	if(m_inputrect.y < 0) m_inputrect.y = 0;
	if((m_inputrect.x+m_inputrect.width) >= cvimg->width) m_inputrect.width = cvimg->width - m_inputrect.x - 1;
	if((m_inputrect.y+m_inputrect.height) >= cvimg->height) m_inputrect.height = cvimg->height - m_inputrect.y - 1;

	CvSize* outputsize = sizeIn.getBuffer();
	if(outputsize) { m_outputsize = *outputsize; }

	if(mp_cvimage) {
		if(mp_cvimage->width != outputsize->width || mp_cvimage->height != outputsize->height) {
			delete mp_cvimage;
			mp_cvimage = NULL;
		}
	}

	if(!mp_cvimage) {
		mp_cvimage = new CVImage(m_outputsize, cvimg->cvMatType, 0);
		cvImageOut.setBuffer(mp_cvimage);
	}
	//std::cout << getName() << "::execute()::mp_cvimage->width = " << mp_cvimage->width << ", mp_cvimage->height = " << mp_cvimage->height << "\n";

//	cvSetZero(mp_cvimage->ipl);

	CvRect prevrect = cvGetImageROI(cvimg->ipl);
	cvSetImageROI(cvimg->ipl, m_inputrect);
	cvResize(cvimg->ipl, mp_cvimage->ipl, m_interpolation);
	cvSetImageROI(cvimg->ipl, prevrect);

	cvImageOut.out();
}

