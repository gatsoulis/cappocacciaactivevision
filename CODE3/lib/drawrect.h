/*
 * drawrect.h
 *
 *  Created on: Jan 18, 2011
 *      Author: gabriele
 */

#ifndef DRAWRECT_H_
#define DRAWRECT_H_

#include "cv.h"

#include "module.h"
#include "inputconnector.h"
#include "outputconnector.h"
#include "cvimage.h"
#include "csctype.h"
#include "CSCEllipticRegions.h"

#include <iostream>

#define CROSS_HAIR_SIZE 3

class DrawRect : public Module{
public:
	DrawRect(std::string name="DrawRect", bool sameimage = false);

	~DrawRect();
	void execute();

	InputConnector<CVImage*> cvImageIn;
	InputConnector<CvPoint*> pointIn;
	InputConnector<CvPoint2D32f*> pointFltIn;
	InputConnector<CSCEllipticRegions*> regionsIn;
	InputConnector<TrackData*> trackIn;

//start adds gabriele
	InputConnector<CvPoint*> maxIn;
	InputConnector<CvPoint*> minIn;
//end adds gabriele



	OutputConnector<CVImage*> cvImageOut;

	void setColor(CvScalar color);

	bool debug;

private:

	CvScalar m_color;
	CVImage* mp_cvimg;

	int m_thickness;

	bool m_sameimage;

	CvScalar m_colorpt;
};

#endif /* DRAWRECT_H_ */
