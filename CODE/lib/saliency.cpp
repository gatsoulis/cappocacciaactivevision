#include <iostream>
#include <sys/stat.h>
#include "saliency.h"

using namespace std;


saliency::saliency(std::string name) : Module(name) {

	std::cout << "\nConstructing " << getName() << "...";

	ipl_inputL = 0;
	ipl_inputR = 0;
	ipl_outputL = 0;
	ipl_outputR = 0;
	desired_numsalwinnersL = 1;
	desired_numsalwinnersR = 1;
	results = NULL;
	//resultsR = NULL;
	cvImageInL.setModule(this);
	cvImageInR.setModule(this);
	saliencystrL = "Saliency_Left";
	saliencystrR = "Saliency_Right";

	cvNamedWindow((saliencystrL).c_str(), CV_WINDOW_AUTOSIZE);
	cvNamedWindow((saliencystrR).c_str(), CV_WINDOW_AUTOSIZE);
	
	move = false;

	debug = false;

	std::cout << "finished.\n";
}

saliency::~saliency() {

	std::cout << "\nDestructing " << getName() << "...";

	cvReleaseImage(&ipl_inputL);
	cvReleaseImage(&ipl_inputR);
	cvReleaseImage(&ipl_outputL);
	cvReleaseImage(&ipl_outputR);

	std::cout << "finished.\n";
}

void saliency::createwind(){
	
	cvNamedWindow((saliencystrL).c_str(), CV_WINDOW_AUTOSIZE);
	cvNamedWindow((saliencystrR).c_str(), CV_WINDOW_AUTOSIZE);
}

void saliency::destroywind(){
	
	cvDestroyWindow((saliencystrL).c_str());
	cvDestroyWindow((saliencystrR).c_str());
}
void saliency::execute(){

	move=false;
  
	cvimgL = cvImageInL.getBuffer();
	cvimgR = cvImageInR.getBuffer();


	int sizearrayL = (*sizearrayInL.getBuffer());
	int sizearrayR = (*sizearrayInR.getBuffer());


	keypointsL = (int*)malloc(2*sizeof(int)*sizearrayL);
	keypointsR = (int*)malloc(2*sizeof(int)*sizearrayR);

	keypointsL = arrayInL.getBuffer();
	keypointsR = arrayInR.getBuffer();
	
	if(debug)
	{
		printf("\nsizearray left:%d\n",sizearrayL*2);
		cvWaitKey(0);
		for(int i = 0; i<2*sizearrayL;i++)
		{
			std::cout<<getName()<<"::keypointsL["<<i<<"]="<<keypointsL[i]<<"\n";

		}
		printf("\nsizearray right:%d\n",sizearrayR*2);
		cvWaitKey(0);
		for(int i = 0; i<2*sizearrayR;i++)
		{
			std::cout<<getName()<<"::keypointsR["<<i<<"]="<<keypointsR[i]<<"\n";

		}

	}



  ipl_inputL = cvCloneImage(cvimgL->ipl);
  ipl_inputR = cvCloneImage(cvimgR->ipl);

  int img_w = ipl_inputL->width;
  int img_h = ipl_inputR->height;
  cout << "Size: " << img_w << " " << img_h << endl;
  
  //REV: note this will also init the long-term saliency map, which you guys don't seem to want.
  //envision_init(img_w, img_h);
  

  //REV: just cheat, load the other one to get the init function called from opencv haha
  //REV: we're going to write over it anyways
  ipl_outputL = cvCloneImage(cvimgL->ipl);
  ipl_outputR = cvCloneImage(cvimgR->ipl);
  
  
  
  //1d int array of x1, y1, x2, y2, etc.
 	 envision_nextframe(ipl_inputL, ipl_outputL, 0);
 	 envision_nextframe(ipl_inputR, ipl_outputR, 1);
	 index = SC_subset_winner(keypointsL, (sizearrayL),keypointsR, (sizearrayR));
	 //SC_winners_update();	 
	results=SC_naive_competition(ipl_outputL, ipl_outputR);
//  results = envision_nextpic_frommem(ipl_input, ipl_output, desired_numsalwinners);

  if(debug)  
  {
		cout << "\n0=LEFT  1=RIGHT ? =>" << results[0]<< endl;
  		cout << "\nRESULT X, Y: (" << results[1] << ", " << results[2] << ")" << endl;
  	
  }

  output_filenameL = "resultL.ppm";
  output_filenameR = "resultR.ppm";
  cvSaveImage(output_filenameL.c_str(), ipl_outputL);
  cvSaveImage(output_filenameR.c_str(), ipl_outputR);


  int posxL = keypointsL[2*index];
  int posyL = keypointsL[2*index+1]; 
  int posxR = keypointsR[2*index]; 
  int posyR = keypointsR[2*index+1];

/*  int index = 0;
  int count = 0;
  if(results[0]==0)
  {
		
		printf("\nLEFT IMAGE IS WINNING");
		for(index = 0;index<2*sizearrayL;index++) {
			int posx = keypointsL[index*sizearrayL];
			int posy = keypointsL[index*sizearrayL+1];
			if(posx>results[1]-10 && posx<results[1]+10 && posy>results[1]-10 && posy<results[1]+10)
			{
				//index=i;
				printf("\nFOUND IN THE LEFT IM!");
				count++;
			}
	
		}
		if(true)
		{
			printf("\n\npointOL = [%d,%d]",pointOL.x,pointOL.y);
			printf("\npointOR = [%d,%d]\n",pointOR.x,pointOR.y);
			printf("\ncount = %d\n",count);
		}

		  pointOL = cvPoint(keypointsL[index],keypointsL[index+1]);
		  pointOR = cvPoint(keypointsR[index],keypointsR[index+1]);
cvWaitKey(0);
  }
  
  else if(results[0]==1)
  {
		printf("\nRIGHT IMAGE IS WINNING");
  		for(index = 0;index<2*sizearrayR;index++) {
			int posx = keypointsR[index*sizearrayR];
			int posy = keypointsR[index*sizearrayR+1];
			if(posx>results[1]-10 && posx<results[1]+10 && posy>results[1]-10 && posy<results[1]+10)
			{
				//index=i;
				printf("\nFOUND IN THE RIGHT IM!");
				printf("\ncount = %d\n",count);
			}
		
		}
		if(true)
		{
			printf("\n\npointOL = [%d,%d]",pointOL.x,pointOL.y);
			printf("\npointOR = [%d,%d]\n",pointOR.x,pointOR.y);
		}
		  pointOL = cvPoint(keypointsL[index],keypointsL[index+1]);
		  pointOR = cvPoint(keypointsR[index],keypointsR[index+1]);
cvWaitKey(0);
  }
  else
  {
		printf("\nNO WINNERS");
		if(true)
		{
			printf("\n\npointOL = [%d,%d]",pointOL.x,pointOL.y);
			printf("\npointOR = [%d,%d]\n",pointOR.x,pointOR.y);
		}
		  pointOL = cvPoint(160,120);
		  pointOR = cvPoint(160,120);
  }

*/


  //pointO = cvPoint(120,90);
/*  IplImage* imageL = cvCloneImage(ipl_outputL);
  cvThreshold(ipl_outputL, imageL, 230, 255.0, CV_THRESH_BINARY);
  cvShowImage((saliencystrL).c_str(), imageL);

  IplImage* imageR = cvCloneImage(ipl_outputR);
  cvThreshold(ipl_outputR, imageR, 230, 255.0, CV_THRESH_BINARY);
  cvShowImage((saliencystrR).c_str(), imageR);
*/

  if(results[0]==0 || results[0]==1)
	move = true;

  pointOL = cvPoint(posxL,posyL);
  pointOR = cvPoint(posxR,posyR);
  cvCircle(ipl_outputL, pointOL, 20, cvScalar(255,0,0), 1);
  cvCircle(ipl_outputR, pointOR, 20, cvScalar(255,0,0), 1);
  cvShowImage((saliencystrL).c_str(), ipl_outputL);
  cvShowImage((saliencystrR).c_str(), ipl_outputR);
 
  pointOutL.setBuffer(&pointOL);
  pointOutL.out();

  pointOutR.setBuffer(&pointOR);
  pointOutR.out();

	//REV: need to release the output image to free memory
  cvReleaseImage(&ipl_outputL);
  cvReleaseImage(&ipl_outputR);
  cvReleaseImage(&ipl_inputL);
  cvReleaseImage(&ipl_inputR);

}

void saliency::release(){
	free(keypointsL);
	free(keypointsR);
}


