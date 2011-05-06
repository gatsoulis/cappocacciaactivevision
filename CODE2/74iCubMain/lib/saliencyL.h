/*
 * saliencyL.h
 *
 *  Created on: May 02, 2011
 *      Author: gabriele
 */

#ifndef SALIENCYL_H_
#define SALIENCYL_H_

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

class saliencyL : public Module  {

public:

  saliencyL(std::string name = "Saliency");
  ~saliencyL();
  void execute();
  void release();

  InputConnector<CVImage*> cvImageInL;

  InputConnector<int*> arrayInL;
  InputConnector<int*> sizearrayInL;

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

  int* keypointsL;



};

#endif /* SALIENCYL_H_ */
