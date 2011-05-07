/*
 * saliencyR.h
 *
 *  Created on: May 02, 2011
 *      Author: gabriele
 */

#ifndef SALIENCYR_H_
#define SALIENCYR_H_

#include "cv.h"
#include "highgui.h"
#include "module.h"
#include "inputconnector.h"
#include "outputconnector.h"
#include "cvimage.h"

#include <iostream>
#include <fstream>
#include <vector>

#include "envision_lib.h"
#include <cstdlib>

#include <yarp/os/all.h>
#include <yarp/sig/all.h>

using namespace std;

class saliencyR : public Module  {

public:

  saliencyR(std::string name = "Saliency");
  ~saliencyR();
  void execute();
  void release();


  InputConnector<CVImage*> cvImageInR;

  InputConnector<int*> arrayInR;

  InputConnector<int*> sizearrayInR;
  OutputConnector<CvPoint*> pointOut;

  bool debug;

private:

  IplImage* ipl_input;
  IplImage* ipl_output;
  int* results;
  string output_filename;
  int desired_numsalwinners;
  CVImage* cvimg;
  CvPoint pointO;
  std::string saliencystr;
  int* keypointsR;







};

#endif /* SALIENCYR_H_ */
