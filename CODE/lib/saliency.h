/*
 * saliencyL.h
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
  void release();
  void createwind();
  void destroywind();

  InputConnector<CVImage*> cvImageInL;
  InputConnector<CVImage*> cvImageInR;

  InputConnector<int*> arrayInL;
  InputConnector<int*> arrayInR;
  InputConnector<int*> sizearrayInL;
  InputConnector<int*> sizearrayInR;
  OutputConnector<CvPoint*> pointOutL;
  OutputConnector<CvPoint*> pointOutR;

  CvPoint pointOL;
  CvPoint pointOR;

  bool move;

  bool debug;

private:

  IplImage* ipl_inputL;
  IplImage* ipl_inputR;
  IplImage* ipl_outputL;
  IplImage* ipl_outputR;
  int* results;
  int index;
//  int* resultsR;
  string output_filenameL;
  string output_filenameR;
  int desired_numsalwinnersL;
  int desired_numsalwinnersR;
  CVImage* cvimgL;
  CVImage* cvimgR;
  std::string saliencystrL;
  std::string saliencystrR;
  int* keypointsL;
  int* keypointsR;



};

#endif /* SALIENCY_H_ */
