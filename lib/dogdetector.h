/*
 * DogDetector.h
 *
 *  Created on: Feb 18, 2011
 *      Author: gabriele
 */

#ifndef DOGDETECTOR_H_
#define DOGDETECTOR_H_


#include "cv.h"
#include "highgui.h"
#include "cxcore.h"
#include "cvaux.h"

#include "module.h"
#include "inputconnector.h"
#include "outputconnector.h"
#include "cvimage.h"
#include "CSCEllipticRegions.h"

//Include files for SIFT feature extraction
#include "utils.h"
#include "imgfeatures.h"
#include "sift.h"

#include <iostream>
#include <unistd.h>



using namespace std;



class DogDetector : public Module{
public:


	DogDetector(std::string name="DogDetector");
	~DogDetector();

	void setParam(unsigned int cornerCount);

	void execute();
	void executeR();

	bool debug;
	bool m_fixBlobSize;


	InputConnector<CVImage*> cvGrayImageIn;
	InputConnector<CvRect*> roiIn;
	OutputConnector<CSCEllipticRegions*> regionsOut;

private:


	CSCEllipticRegions* mp_regions;
	float m_fixed_blob_size;
	unsigned int m_cornerCount; //maximum number of corners returned


};

#endif /* DOGDETECTOR_H_ */
