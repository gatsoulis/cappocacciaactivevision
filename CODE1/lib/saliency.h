/*
 * saliency.h
 *
 *  Created on: May 02, 2011
 *      Author: gabriele
 */

#ifndef SALIENCY_H_
#define SALIENCY_H_

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

class saliency : public Module  {

public:

  saliency(std::string name = "Saliency");
  ~saliency();
  void execute();

  InputConnector<CVImage*> cvImageIn;
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


};

#endif /* SALIENCY_H_ */
