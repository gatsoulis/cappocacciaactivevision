#ifndef CueTemplate_H
#define CueTemplate_H

#include "cue.h"
#include "csctype.h"
#include "inputconnector.h"
#include "outputconnector.h"
#include "cvimage.h"

#include <string>
#include <iostream>
//#include "prototypetemplate.h"


//! class CueTemplate  -- Template cue using OpenCV's function cvMatchTemplate().
/*! class CueTemplate  -- Template cue using OpenCV's function cvMatchTemplate().

	Parameter 
	int templateSize 		size of the template (width and height) [in pixels] for matching
	int method 				different matching methods supported (detailed explanation below)

	Different methods for cvMatchTemplate function (from OpenCV manual)
	==============================================
	method=CV_TM_SQDIFF:
	R(x,y)=sumx',y'[T(x',y')-I(x+x',y+y')]2

	method=CV_TM_SQDIFF_NORMED:
	R(x,y)=sumx',y'[T(x',y')-I(x+x',y+y')]2/sqrt[sumx',y'T(x',y')2•sumx',y'I(x+x',y+y')2]

	method=CV_TM_CCORR:
	R(x,y)=sumx',y'[T(x',y')•I(x+x',y+y')]

	method=CV_TM_CCORR_NORMED:
	R(x,y)=sumx',y'[T(x',y')•I(x+x',y+y')]/sqrt[sumx',y'T(x',y')2•sumx',y'I(x+x',y+y')2]

	method=CV_TM_CCOEFF:
	R(x,y)=sumx',y'[T'(x',y')•I'(x+x',y+y')],

	where T'(x',y')=T(x',y') - 1/(w•h)•sumx",y"T(x",y") (mean template brightness=>0)
    	  I'(x+x',y+y')=I(x+x',y+y') - 1/(w•h)•sumx",y"I(x+x",y+y") (mean patch brightness=>0)

	method=CV_TM_CCOEFF_NORMED: !!!
	R(x,y)=sumx',y'[T'(x',y')•I'(x+x',y+y')]/sqrt[sumx',y'T'(x',y')2•sumx',y'I'(x+x',y+y')2
	==============================================
		revised 03/2006 by Hyundo Kim
*/
	class CueTemplate : public Cue  {
	public: 
		CueTemplate(std::string name="CueTemplate");
		~CueTemplate();

		OutputConnector<CVImage*> cvTemplateImageOut;

		void execute();
		void adapt();
		void initialize();
		
		void findwinner();

		void setParam(int templateSize = 11, int method = CV_TM_CCOEFF_NORMED);
		bool debug;

	private:
		int m_templatesizex, m_templatesizey;
		int m_halftemplatesizex, m_halftemplatesizey;
		int m_method;

		CVImage* mp_cvtemplateimg;
		IplImage* mp_templateimg;
		IplImage* mp_newtemplateimg;
		IplImage* mp_origtemplateimg;
		IplImage* mp_temptemplateimg;
		IplImage* mp_boundedoutputimg;
		//CVImage* mp_cvoutputimg;

		//double m_min, m_max;
		//CvPoint m_minLoc, m_maxLoc;
		CvRect m_boundaryRect;
		CvScalar m_noise;
		double m_back;
		
		void drawgaussianblob(const CvPoint& pos, const CvSize& size);

/*
		float m_part;
		double m_maxPrev;
*/
	};

#endif
