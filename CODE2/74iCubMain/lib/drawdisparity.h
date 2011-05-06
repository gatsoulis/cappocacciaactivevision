/*
 * drawdisparity.h
 *
 *  Created on: Feb 25, 2011
 *      Author: gabriele
 */

#ifndef DRAWDISPARITY_H_
#define DRAWDISPARITY_H_

#include "cv.h"
#include "highgui.h"

#include "module.h"
#include "inputconnector.h"
#include "outputconnector.h"
#include "cvimage.h"
#include "csctype.h"
#include "CSCEllipticRegions.h"

#include <iostream>
#include <stdio.h>

class DrawDisp : public Module{
public:

	DrawDisp(std::string name="DrawDisp", bool sameimage = false);
	~DrawDisp();

	void execute();

	InputConnector<CVImage*> cvImageIn;

	InputConnector<CSCEllipticRegions*> regionsInLeft;
	InputConnector<CSCEllipticRegions*> regionsInRight;

	OutputConnector<CVImage*> cvImageOut;



	bool debug;

private:

	CVImage* mp_cvimg;

	bool m_sameimage;

};

#endif /* DRAWDISPARITY_H_ */
