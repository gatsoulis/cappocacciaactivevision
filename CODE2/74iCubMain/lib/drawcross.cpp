#include "drawcross.h"



DrawCross::DrawCross(std::string name, bool sameimage) : Module(name) {

	cvImageIn.setModule(this);
	pointIn.setModule(this);
	pointFltIn.setModule(this);
	regionsIn.setModule(this);

	mp_cvimg = NULL;
	m_sameimage = sameimage;

	m_color = CV_RGB(0, 255, 0);
	m_colorpt = CV_RGB(255, 0, 0);

	m_thickness = 1;

	debug = false;

	//std::cout << "finished.\n";
}

DrawCross::~DrawCross() {
	//std::cout << "Destructing " << getName() << "...";
	if(mp_cvimg && !m_sameimage) delete mp_cvimg;
	//std::cout << "finished.\n";
}

void DrawCross::execute() {
	CVImage* cvimg = cvImageIn.getBuffer();
	CvPoint* point = pointIn.getBuffer();
	TrackData* track = trackIn.getBuffer();
	CvPoint2D32f* pointflt = pointFltIn.getBuffer();
	CSCEllipticRegions* region = regionsIn.getBuffer();
	if(!cvimg) { std::cerr << getName() << "::ERROR::cvImageIn is NULL!\n"; return; }
	if(!point && !pointflt && !region) {
		cvImageOut.setBuffer(cvimg);
		cvImageOut.out();
		return;
	}

//	if(debug) {
//		std::cout << getName() << "::execute()::region.size = " << region->getCurrentSize() << "\n";
//	}

	if(!mp_cvimg && !m_sameimage){
		mp_cvimg = new CVImage(cvSize(cvimg->width, cvimg->height), CV_8UC3, 0);
	}

	if(!m_sameimage) {
//		cvCopy(cvimg->ipl, mp_cvimg->ipl);
		if(cvimg->cvMatType == mp_cvimg->cvMatType) {
			cvCopy(cvimg->ipl, mp_cvimg->ipl);
		}
		else{
			cvCvtColor(cvimg->ipl, mp_cvimg->ipl, CV_GRAY2BGR);
		}
	}
	else{
		mp_cvimg = cvimg;
	}

	IplImage* img = mp_cvimg->ipl;

	if(track) {
		point = &(track->winnerPos);
	}

	// Draw cross
	if(point) {

		cvLine( img, cvPoint(point->x, point->y-CROSS_HAIR_SIZE), cvPoint(point->x, point->y+CROSS_HAIR_SIZE), m_colorpt, m_thickness);
		cvLine( img, cvPoint(point->x-CROSS_HAIR_SIZE, point->y), cvPoint(point->x+CROSS_HAIR_SIZE, point->y), m_colorpt, m_thickness);

/*
		cvLine( img, cvPoint(point->x-1, point->y-CROSS_HAIR_SIZE), cvPoint(point->x-1, point->y+CROSS_HAIR_SIZE),color1lo, 1);
		cvLine( img, cvPoint(point->x+1, point->y-CROSS_HAIR_SIZE), cvPoint(point->x+1, point->y+CROSS_HAIR_SIZE),color1lo, 1);
		cvLine( img, cvPoint(point->x,   point->y-CROSS_HAIR_SIZE), cvPoint(point->x,   point->y+CROSS_HAIR_SIZE),color1hi, 1);
	
		cvLine( img, cvPoint(point->x-CROSS_HAIR_SIZE, point->y-1), cvPoint(point->x+CROSS_HAIR_SIZE, point->y-1),color1lo, 1);
		cvLine( img, cvPoint(point->x-CROSS_HAIR_SIZE, point->y+1), cvPoint(point->x+CROSS_HAIR_SIZE, point->y+1),color1lo, 1);
		cvLine( img, cvPoint(point->x-CROSS_HAIR_SIZE, point->y  ), cvPoint(point->x+CROSS_HAIR_SIZE, point->y  ),color1hi, 1);
*/		
	}

	if(pointflt) {
		cvLine( img, cvPoint(cvRound(pointflt->x), cvRound(pointflt->y-CROSS_HAIR_SIZE)), cvPoint(cvRound(pointflt->x), cvRound(pointflt->y+CROSS_HAIR_SIZE)), m_colorpt, m_thickness);
		cvLine( img, cvPoint(cvRound(pointflt->x-CROSS_HAIR_SIZE), cvRound(pointflt->y)), cvPoint(cvRound(pointflt->x+CROSS_HAIR_SIZE), cvRound(pointflt->y)), m_colorpt, m_thickness);
	}

	if(region) {
		unsigned int height = region->getCurrentSize();
		unsigned int width = region->getWidth();
		float* data = region->getData();
		for(unsigned int i = 0;i<height;i++) {
			float posx = data[i*width];
			float posy = data[i*width+1];
			cvLine( img, cvPoint(cvRound(posx), cvRound(posy-CROSS_HAIR_SIZE)), cvPoint(cvRound(posx), cvRound(posy+CROSS_HAIR_SIZE)), m_color, m_thickness);
			cvLine( img, cvPoint(cvRound(posx-CROSS_HAIR_SIZE), cvRound(posy)), cvPoint(cvRound(posx+CROSS_HAIR_SIZE), cvRound(posy)), m_color, m_thickness);
		}
	}

	cvImageOut.setBuffer(mp_cvimg);
	cvImageOut.out();
}

void DrawCross::setColor(CvScalar color) {
	m_color = color;
}

