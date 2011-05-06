/*
 * camera.h
 *
 *  Created on: Dec 14, 2010
 *      Author: gabriele
 */

#ifndef CAMERA_H_
#define CAMERA_H_


#include "cv.h"
#include "highgui.h"
#include "module.h"
#include "inputconnector.h"
#include "outputconnector.h"
#include "cvimage.h"

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

class camera : public Module  {
public:
	camera(std::string name = "Camera");
	~camera();
	void execute();
	CVImage* convert2CVImageLeft(IplImage* InputImage);
	CVImage* convert2grayLeft(CVImage* input);
	CVImage* convert2CVImageRight(IplImage* InputImage);
	CVImage* convert2grayRight(CVImage* input);





	OutputConnector<CVImage*> cvLeftImage2Out;
	OutputConnector<CVImage*> cvRightImage2Out;

	OutputConnector<CVImage*> cvLeftGrayImage2Out;
	OutputConnector<CVImage*> cvRightGrayImage2Out;

	IplImage *imageL;
	IplImage *imageR;



private:



//	CvCapture* captureL;
//	CvCapture* captureR;

	CVImage* mp_leftrgbimage;
	CVImage* mp_rightrgbimage;

	CVImage* mp_leftgrayimage ;
	CVImage* mp_rightgrayimage;

	CVImage* mp_cvimgL;
	CVImage* mp_cvimgR;

	CVImage* mp_cvgrayimgL;
	CVImage* mp_cvgrayimgR;


	bool m_checkfileexistence;
};

#endif /* CAMERA_H_ */
