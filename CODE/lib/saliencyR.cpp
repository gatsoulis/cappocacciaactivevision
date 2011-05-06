#include <iostream>
#include <sys/stat.h>
#include "saliencyR.h"

using namespace std;


saliencyR::saliencyR(std::string name) : Module(name) {

	std::cout << "\nConstructing " << getName() << "...";

/*	ipl_input = 0;
	ipl_output = 0;
	desired_numsalwinners = 1;
	results = NULL;
	cvImageInR.setModule(this);
	saliencystr = getName();

	cvNamedWindow((saliencystr).c_str(), CV_WINDOW_AUTOSIZE);
	

	sizearrayR=0;
*/
	debug = false;

	std::cout << "finished.\n";
}

saliencyR::~saliencyR() {

	std::cout << "\nDestructing " << getName() << "...";

/*	cvReleaseImage(&ipl_input);
	cvReleaseImage(&ipl_output);
	cvDestroyWindow((saliencystr).c_str());
*/

	std::cout << "finished.\n";
}


void saliencyR::execute(){


//	cvimg = cvImageInR.getBuffer();
	int sizearrayR = (*sizearrayInR.getBuffer());
	keypointsR = (int*)malloc(2*sizeof(int)*sizearrayR);
	keypointsR = arrayInR.getBuffer();
	
	if(debug)
	{
		printf("\nsizearray right:%d\n",sizearrayR*2);
		cvWaitKey(0);
		for(int i = 0; i<2*sizearrayR;i++)
		{
			std::cout<<getName()<<"::keypointsR["<<i<<"]="<<keypointsR[i]<<"\n";

		}

	}
	free(keypointsR);
/*
  ipl_input = cvCloneImage(cvimg->ipl);
  int img_w = ipl_input->width;
  int img_h = ipl_input->height;
  cout << "Size: " << img_w << " " << img_h << endl;
  
  //REV: note this will also init the long-term saliency map, which you guys don't seem to want.
  //envision_init(img_w, img_h);
  

  //REV: just cheat, load the other one to get the init function called from opencv haha
  //REV: we're going to write over it anyways
  ipl_output = cvCloneImage(cvimg->ipl);
  
  
 	 results = envision_nextpic_frommemR(ipl_input, ipl_output, desired_numsalwinners);
//  results = envision_nextpic_frommem(ipl_input, ipl_output, desired_numsalwinners);

  if(debug)  
  {

  		cout << "RIGHT\nRESULT X, Y: (" << results[0] << ", " << results[1] << ")" << endl;
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

//if(keypointsR)
//	free(keypointsR);

}

//void saliencyR::release(){
//	free(keypointsR);
//}
