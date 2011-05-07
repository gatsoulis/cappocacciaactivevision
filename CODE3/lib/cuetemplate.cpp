#include "cuetemplate.h"



CueTemplate::CueTemplate(std::string name) : Cue(name) {
	std::cout << "\nConstructing " << getName() << "...";
	
	mp_templateimg = NULL;
	mp_newtemplateimg = NULL;
	mp_origtemplateimg = NULL;
	mp_temptemplateimg = NULL;
	mp_boundedoutputimg = NULL;

	mp_cvtemplateimg = NULL;
	mp_cvoutputimg = NULL;
	m_noise = cvScalarAll(0.1);
	m_back = 10.0;
	m_threshold = 0.6;

	m_init = false;
	debug = false;

	this->setParam();
	
	trackOut.setBuffer(NULL);

	std::cout << "finished.\n";
}

CueTemplate::~CueTemplate() {                                                 
	std::cout << "Destructing " << getName() << "...";
	
	//if (mp_templateimg) cvReleaseImage(&mp_templateimg);
	if (mp_newtemplateimg) cvReleaseImage(&mp_newtemplateimg);
	if (mp_origtemplateimg) cvReleaseImage(&mp_origtemplateimg);
	if (mp_temptemplateimg) cvReleaseImage(&mp_temptemplateimg);
	//if (mp_boundedoutputImg) cvReleaseImage(&mp_boundedoutputImg);

	if (mp_cvtemplateimg) delete mp_cvtemplateimg;
	if (mp_cvoutputimg) delete mp_cvoutputimg;

	std::cout << "finished.\n";
}

void CueTemplate::setParam(int templateSize, int method) {
	m_templatesizex = templateSize;
	m_templatesizey = templateSize;
	m_method = method;

	m_halftemplatesizex = cvRound(floor((double)m_templatesizex/2.0));
	m_halftemplatesizey = cvRound(floor((double)m_templatesizey/2.0));

//	m_part = 0.001;
//	m_maxPrev = 0.01;
//	m_thresholdhold = 0.6;
}

void CueTemplate::findwinner() {

	if(!m_init) {
		trackOut.setBuffer(NULL);
		trackOut.out();
		return;
	}

	double min, max;
	CvPoint minloc, maxloc;
	cvMinMaxLoc(mp_cvoutputimg->ipl, &min, &max, &minloc, &maxloc, NULL);

	if(max < m_threshold) {
		trackOut.setBuffer(NULL);
		trackOut.out();
		return;
	}

	m_track.winnerPos = maxloc;
	m_track.reliability = max;

//	m_track.imageSize.width = mp_cvoutputimg->width;
//	m_track.imageSize.height = mp_cvoutputimg->height;
	
//	m_track.winnerSize.width = 100;
//	m_track.winnerSize.height = 100;

//	m_track.winnerRect.x = m_track.winnerPos.x - cvRound((float)m_track.winnerSize.width/2.0);
//	m_track.winnerRect.y = m_track.winnerPos.y - cvRound((float)m_track.winnerSize.height/2.0);
//	m_track.winnerRect.width = m_track.winnerSize.width;
//	m_track.winnerRect.height = m_track.winnerSize.height;

	trackOut.setBuffer(&m_track);
	trackOut.out();

}

void CueTemplate::execute() {

	CVImage* cvgrayimg = cvGrayImageIn.getBuffer();
	if(!cvgrayimg) { std::cerr<< getName() << "::ERROR::execute()::cvGrayImageIn is NULL!...\n"; cvSalImageOut.setBuffer(NULL); cvSalImageOut.out(); return; }
	IplImage* grayimg = cvgrayimg->ipl;

	if (!mp_cvoutputimg) {
//		std::cout << getName() << "Line 2\n";
		mp_cvoutputimg = new CVImage( cvSize( grayimg->width, grayimg->height), CV_32FC1, 0);
		cvSetZero(mp_cvoutputimg->ipl);
		cvSalImageOut.setBuffer(mp_cvoutputimg);

		mp_boundedoutputimg = mp_cvoutputimg->ipl;

	}
	
	cvSetZero(mp_boundedoutputimg);

	if(!m_init) {
//		cvSalImageOut.out();
//		return; 
		initialize();
		return;
		//std::cerr<< getName() << "::ERROR::execute()::Call initialize() first!...\n"; cvSalImageOut.setBuffer(NULL); cvSalImageOut.out(); return;
	}

	CvRect* roi = roiIn.getBuffer();
	if(roi){
		if(debug) std::cout << getName() << "::execute()::roiIn not NULL!\n";
		cvSetImageROI( grayimg, (*roi) );
		m_boundaryRect.x = (*roi).x + m_halftemplatesizex;
		m_boundaryRect.y = (*roi).y + m_halftemplatesizey;
		m_boundaryRect.width = (*roi).width - m_templatesizex-1;
		m_boundaryRect.height = (*roi).height - m_templatesizey-1;
	}
	
	if(debug) std::cout << std::endl << getName() << "::execute()";
	cvSetImageROI( mp_boundedoutputimg, m_boundaryRect);
	if(m_method == CV_TM_CCOEFF_NORMED | m_method == CV_TM_CCOEFF | m_method == CV_TM_CCORR_NORMED | m_method == CV_TM_CCORR){
		// Global max position is the target
		if(debug) {
			std::cout << std::endl << getName() << "::execute()::cvMatchTemplate";
			std::cout << "\nW = " << grayimg->width << ", H = " << grayimg->height;
			std::cout << "\nw = " << mp_templateimg->width << ", h = " << mp_templateimg->height;
			std::cout << "\n(W-w+1) = " << m_boundaryRect.width << ", (H-h+1) = " << m_boundaryRect.height << "\n";
		}
		cvMatchTemplate( grayimg, mp_templateimg, mp_boundedoutputimg, m_method);
		if(debug) std::cout << getName() << "::execute()::cvMinMaxLoc\n";
		double min, max;
		//CvPoint minLoc, maxLoc;
		cvMinMaxLoc( mp_boundedoutputimg, &min, &max, NULL, NULL, NULL);
		if(debug) std::cout << getName() << "::execute()::max(winner) = " << max << ", min = " << min << "\n";
		if(max < m_threshold) {
			//m_init = false;
			if(debug) std::cout << getName() << "::execute()::Tracking lost\n";
			cvSetZero(mp_boundedoutputimg);
		}
		else{
			if(max > min) {
				cvConvertScale( mp_boundedoutputimg, mp_boundedoutputimg, (1.0/(max)), 0.0);
				// dst = src if src > thres, ow 0
				cvThreshold( mp_boundedoutputimg, mp_boundedoutputimg, 0.5, 0, CV_THRESH_TOZERO);
			}
			else{
				cvSet(mp_boundedoutputimg, m_noise);
			}
		}
	}
	else if( m_method == CV_TM_SQDIFF_NORMED | m_method == CV_TM_SQDIFF ){
		// Global min position is the target
		cvMatchTemplate( grayimg, mp_templateimg, mp_boundedoutputimg, m_method);
		cvConvertScale( mp_boundedoutputimg, mp_boundedoutputimg, -1, 0);   
		double min, max;
		//CvPoint minLoc, maxLoc;
		cvMinMaxLoc( mp_boundedoutputimg, &min, &max, NULL, NULL, NULL);
		if(debug) std::cout << getName() << "::execute()::min(winner) = " << min << ", max = " << max << "\n";
		if(min > m_threshold) {
			m_init = false;
			if(debug) std::cout << getName() << "::execute()::Tracking lost\n";
		}
		else{
			if(max > min) {
				cvConvertScale( mp_boundedoutputimg, mp_boundedoutputimg, (1.0/(max)), 0.0);
				// dst = src if src > thres, 0 otherwise
				cvThreshold( mp_boundedoutputimg, mp_boundedoutputimg, 0, 0, CV_THRESH_TOZERO);
				// dst = 0 if src > thres, = src otherwise
				cvThreshold( mp_boundedoutputimg, mp_boundedoutputimg, 1, 1, CV_THRESH_TOZERO_INV);
			}
			else{
				cvSet(mp_boundedoutputimg, m_noise);
			}
		}
	}
	cvResetImageROI( mp_boundedoutputimg );

	if(roi){
		cvResetImageROI( grayimg );
	}

	cvSalImageOut.out();	
	if(debug) std::cout << getName() << "::execute() complete\n";

}


void CueTemplate::adapt() {
	if(!m_init) return;

	CVImage* cvgrayimg = cvGrayImageIn.getBuffer();
	if(!cvgrayimg) { std::cerr<< getName() << "::ERROR::execute()::cvGrayImageIn is NULL!...\n"; return; }
	IplImage* grayimg = cvgrayimg->ipl;

	TrackData* track = trackIn.getBuffer();
//	if(!track){ std::cerr<< getName() << "::ERROR::execute()::trackIn is NULL!...\n"; return; }	

	float rel;
	CvPoint winner;
	if(track) {
		rel = track->reliability;
		winner = track->winnerPos;
	}
	else{
		double min, max;
		CvPoint minLoc, maxLoc;
		cvMinMaxLoc(mp_boundedoutputimg, &min, &max, &minLoc, &maxLoc, NULL);
		rel = (float)max;
		winner = maxLoc;
	}

	if(rel > m_threshold){
		// adapt toward new template
		int x = winner.x;
		int y = winner.y;
		if(x < m_halftemplatesizex) x = m_halftemplatesizex;
		if(y < m_halftemplatesizey) y = m_halftemplatesizey;
		if(x >= grayimg->width - m_halftemplatesizex) x = grayimg->width - m_halftemplatesizex-1;
		if(y >= grayimg->height - m_halftemplatesizey) y = grayimg->height - m_halftemplatesizey-1;
		CvRect rect;
		rect.x = x - m_halftemplatesizex;
		rect.y = y - m_halftemplatesizey;
		rect.width = m_templatesizex;
		rect.height = m_templatesizey;
		cvSetImageROI(grayimg, rect );
		cvCopy( grayimg, mp_newtemplateimg );
		cvScale( mp_templateimg, mp_templateimg, 1.0 - m_tfacs);
		cvScale( mp_newtemplateimg, mp_newtemplateimg, m_tfacs);
		cvAdd( mp_newtemplateimg, mp_templateimg, mp_templateimg );
		cvResetImageROI( grayimg );
		cvTemplateImageOut.out();
	}
	else{
		// adapting back to the original template
		cvScale( mp_templateimg, mp_templateimg, 1.0 - (m_tfacs/m_back) );
		cvScale( mp_origtemplateimg, mp_temptemplateimg, (m_tfacs/m_back) );
		cvAdd( mp_temptemplateimg, mp_templateimg, mp_templateimg );
		cvTemplateImageOut.out();
	}
}

void CueTemplate::initialize() {
	TrackData* track = trackIn.getBuffer();
	if(!track) { return; }
	if(track->reliability < m_threshold) { 
		return; 
	}

	CVImage* cvgrayimg = cvGrayImageIn.getBuffer();
	if(!cvgrayimg) { std::cerr<< getName() << "::ERROR::execute()::cvGrayImageIn is NULL!...\n"; return; }
	IplImage* grayimg = cvgrayimg->ipl;

	m_track.winnerPos = track->winnerPos;
	m_track.winnerSize = track->winnerSize;
	m_track.winnerRect = track->winnerRect;
	m_track.reliability = track->reliability;

	if(debug) { std::cout << getName() << "::initialize()::Initializing rect [" << m_track.winnerRect.x << " " << m_track.winnerRect.y << " " << m_track.winnerRect.width << " " << m_track.winnerRect.height << "]\n"; }

	int width, height;
	width = grayimg->width;
	height = grayimg->height;
	m_track.imageSize.width = width;
	m_track.imageSize.height = height;
	
	m_templatesizex = m_track.winnerRect.width;
	m_templatesizey = m_track.winnerRect.height;
	
	m_halftemplatesizex = cvRound(floor((double)m_templatesizex/2.0));
	m_halftemplatesizey = cvRound(floor((double)m_templatesizey/2.0));

	m_boundaryRect.x = m_halftemplatesizex;
	m_boundaryRect.y = m_halftemplatesizey;
	m_boundaryRect.width = grayimg->width - m_templatesizex + 1; // W-w+1
	m_boundaryRect.height = grayimg->height - m_templatesizey + 1; // H-h+1
	if(debug) std::cout << "m_boundaryRect = [" << m_boundaryRect.x << " " << m_boundaryRect.y << " " << m_boundaryRect.width << " " << m_boundaryRect.height << "]\n";

	if (mp_cvtemplateimg) delete mp_cvtemplateimg;
	if (mp_newtemplateimg) cvReleaseImage(&mp_newtemplateimg);
	if (mp_origtemplateimg) cvReleaseImage(&mp_origtemplateimg);
	if (mp_temptemplateimg) cvReleaseImage(&mp_temptemplateimg);

	mp_cvtemplateimg = new CVImage( cvSize( m_templatesizex, m_templatesizey ), CV_8UC1, 0);
	mp_templateimg = mp_cvtemplateimg->ipl;
	mp_newtemplateimg = cvCreateImage( cvSize( m_templatesizex, m_templatesizey ), IPL_DEPTH_8U, 1);
	mp_origtemplateimg = cvCreateImage( cvSize( m_templatesizex, m_templatesizey ), IPL_DEPTH_8U, 1);
	mp_temptemplateimg = cvCreateImage( cvSize( m_templatesizex, m_templatesizey ), IPL_DEPTH_8U, 1);

	cvSetZero(mp_templateimg);
	cvTemplateImageOut.setBuffer(mp_cvtemplateimg);
	
	int x, y;
	x = track->winnerPos.x;
	y = track->winnerPos.y;
	if(x < m_halftemplatesizex) x = m_halftemplatesizex;
	if(y < m_halftemplatesizey) y = m_halftemplatesizey;
	if(x >= width - m_halftemplatesizex) x = width - m_halftemplatesizex - 1;
	if(y >= height - m_halftemplatesizey) y = height - m_halftemplatesizey - 1;

	CvRect rect;
	rect.x = x - m_halftemplatesizex;
	rect.y = y - m_halftemplatesizey;
	rect.width = m_templatesizex;
	rect.height = m_templatesizey;

	cvSetImageROI(grayimg, rect );
	cvCopy( grayimg, mp_templateimg );
	cvResetImageROI( grayimg );
	cvCopy( mp_templateimg, mp_origtemplateimg );

	cvTemplateImageOut.out();

	m_init = true;
	if(debug) { std::cout << getName() << "::initialize()::Initializing complete\n"; }
}

void CueTemplate::drawgaussianblob(const CvPoint& pos, const CvSize& size) {
	float temp = 1.0;
	temp = (mp_boundedoutputimg->width) / 160.0;
	float K = 2.0 * size.width * temp; // smaller K, smaller circle
	int windowSizeX = cvRound(size.width * temp);
	int windowSizeY = cvRound(size.height * temp);
	int posx = pos.x;
	int posy = pos.y;
	int width = mp_boundedoutputimg->width;
	int height = mp_boundedoutputimg->height;
	cvSetZero(mp_boundedoutputimg);
	for(int x = -windowSizeX; x <= windowSizeX; ++x) {
		for(int y = -windowSizeY; y <= windowSizeY; ++y) {
		
			int tempx = posx + x;
			int tempy = posy + y;

			if(tempx >= 0 && tempy >= 0 && tempx < width && tempy < height) {
				double val = exp(-0.5/K*((float)x*(float)x + (float)y*(float)y));
				cvSetReal2D(mp_boundedoutputimg, tempy, tempx, val);
			}
		
		}
	}
}
