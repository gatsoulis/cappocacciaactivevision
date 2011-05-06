#include "cuemotion.h"



CueMotion::CueMotion(std::string name) : Cue(name) {
	std::cout << "Constructing " << getName() << "...";

	mp_cvprevimg = NULL;
	mp_cvoutputimg = NULL;

	setParam();

	std::cout << "finished.\n";
}

CueMotion::~CueMotion(){
	std::cout<<"Destructing " << getName() << "...";

	if(mp_cvprevimg) delete mp_cvprevimg;
	if(mp_cvoutputimg) delete mp_cvoutputimg;

	std::cout << "finished.\n";
}

void CueMotion::execute() {

	CVImage* cvgrayimg = cvGrayImageIn.getBuffer();

	if(!cvgrayimg) { std::cerr<< getName() << "::ERROR::execute()::cvGrayImageIn is NULL!...\n"; cvSalImageOut.setBuffer(NULL); cvSalImageOut.out(); return; }

	IplImage* grayimg = cvgrayimg->ipl;

	if (!mp_cvprevimg) {
		m_track.imageSize.width = grayimg->width;
		m_track.imageSize.height = grayimg->height;
		mp_cvprevimg = new CVImage(cvSize(m_track.imageSize.width, m_track.imageSize.height), CV_8UC1, 0);
		mp_cvoutputimg = new CVImage(cvSize(m_track.imageSize.width, m_track.imageSize.height), CV_32FC1, 0);
		cvCopy(grayimg, mp_cvprevimg->ipl, NULL);
		cvSalImageOut.setBuffer(mp_cvoutputimg);
	}

	CvRect* roi = roiIn.getBuffer();

	IplImage* previmg = mp_cvprevimg->ipl;
	IplImage* outputimg = mp_cvoutputimg->ipl;

	if(roi){

		m_cvgrayimgroi = cvGetImageROI( grayimg );

		cvSetImageROI( grayimg, (*roi) );
		cvSetImageROI( previmg, (*roi) );
		cvSetImageROI( outputimg, (*roi) );
		
		// Computing absolute difference of current frame to old frame and store it in old frame
		cvAbsDiff(grayimg, previmg, previmg);
		// Binary thresholding
		cvThreshold(previmg, previmg, m_thresholdMotion, 1, CV_THRESH_BINARY);
		// Converting 8bit unsigned image to 32bit float image
		cvConvert(previmg, outputimg);
		
		cvSetImageROI( grayimg, m_cvgrayimgroi );
		cvSetImageROI( previmg, m_cvgrayimgroi );
		cvSetImageROI( outputimg, m_cvgrayimgroi );
	}
	else{			
		// Computing absolute difference of current frame to old frame and store it in old frame
		cvAbsDiff(grayimg, previmg, previmg);
		// Binary thresholding
		cvThreshold(previmg, previmg, m_thresholdMotion, 1, CV_THRESH_BINARY);
		// Converting 8bit unsigned image to 32bit float image
		cvConvert(previmg, outputimg);	
	}
	
	// Gaussian blurring
	if (m_kernelSize>0) cvSmooth(outputimg, outputimg, CV_GAUSSIAN, m_kernelSize, m_kernelSize);

	// Set the old frame to current frame
	cvCopy(grayimg, previmg, NULL);

	//salImageOut.out();
	cvSalImageOut.out();
}

void CueMotion::adapt() {
	// Do nothing
}

void CueMotion::initialize() {
	// Do nothing
}

void CueMotion::print(){
	std::cout << getName() << "::print()\n";
	std::cout << "tfacs = " << m_tfacs << ", thres = " << m_threshold << "\n";
	std::cout << "kernelSize = " << m_kernelSize << ", thresholdMotion = " << m_thresholdMotion << "\n";
	std::cout << "winnerPos = ( " << m_track.winnerPos.x << ", " << m_track.winnerPos.y << ")\n";
	std::cout << "winnerSize = ( " << m_track.winnerSize.width << ", " << m_track.winnerSize.height << ")\n";
	std::cout << "winnerRect = ( " << m_track.winnerRect.x << ", " << m_track.winnerRect.y << ", " << m_track.winnerRect.width << ", " << m_track.winnerRect.height << ")\n";
	std::cout << "reliability = " << m_track.reliability << "\n";
	std::cout << "imageSize = ( " << m_track.imageSize.width << ", " << m_track.imageSize.height << ")\n";
}
