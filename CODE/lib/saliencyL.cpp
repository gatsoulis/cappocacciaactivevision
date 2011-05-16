#include <iostream>
#include <sys/stat.h>
#include "saliencyL.h"

using namespace std;


saliencyL::saliencyL(std::string name) : Module(name) {

	std::cout << "\nConstructing " << getName() << "...";

/*	ipl_input = 0;
	ipl_output = 0;
	desired_numsalwinners = 1;
	results = NULL;
	cvImageInL.setModule(this);
	saliencystr = getName();

	cvNamedWindow((saliencystr).c_str(), CV_WINDOW_AUTOSIZE);
	
	sizearrayL=0;
*/
	debug = false;

	std::cout << "finished.\n";
}

saliencyL::~saliencyL() {

	std::cout << "\nDestructing " << getName() << "...";
/*
	cvReleaseImage(&ipl_input);
	cvReleaseImage(&ipl_output);
	cvDestroyWindow((saliencystr).c_str());
*/

	std::cout << "finished.\n";
}


void saliencyL::execute(){


  
//	cvimg = cvImageInL.getBuffer();
	int sizearrayL = (*sizearrayInL.getBuffer());
	keypointsL = (int*)malloc(2*sizeof(int)*sizearrayL);
	keypointsL = arrayInL.getBuffer();
	
	if(debug)
	{
		printf("\nsizearray left:%d\n",sizearrayL*2);
		cvWaitKey(0);
		for(int i = 0; i<2*sizearrayL;i++)
		{
			std::cout<<getName()<<"::keypointsL["<<i<<"]="<<keypointsL[i]<<"\n";

		}

	}
	free(keypointsL);
/*  ipl_input = cvCloneImage(cvimg->ipl);
  int img_w = ipl_input->width;
  int img_h = ipl_input->height;
  cout << "Size: " << img_w << " " << img_h << endl;
  
  //REV: note this will also init the long-term saliency map, which you guys don't seem to want.
  //envision_init(img_w, img_h);
  

  //REV: just cheat, load the other one to get the init function called from opencv haha
  //REV: we're going to write over it anyways
  ipl_output = cvCloneImage(cvimg->ipl);
  
  
  
  //1d int array of x1, y1, x2, y2, etc.
 	 results = envision_nextpic_frommemL(ipl_input, ipl_output, desired_numsalwinners);
  
//  results = envision_nextpic_frommem(ipl_input, ipl_output, desired_numsalwinners);

  if(debug)  
  {
  		cout << "LEFT\nRESULT X, Y: (" << results[0] << ", " << results[1] << ")" << endl;
  
  }

  output_filename = "result.ppm";
  cvSaveImage(output_filename.c_str(), ipl_output);

  pointO = cvPoint(results[0],results[1]);

  //pointO = cvPoint(120,90);

  cvShowImage((saliencystr).c_str(), ipl_output);
 
  pointOut.setBuffer(&pointO);
  pointOut.out();

	//REV: need to release the output image to free memory
  cvReleaseImage(&ipl_output);
  cvReleaseImage(&ipl_input);
*/
//if(keypointsL)
//	free(keypointsL);
}

//void saliencyL::release(){
//	free(keypointsL);
//}


