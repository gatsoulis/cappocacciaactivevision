#include "cv.h"
#include "highgui.h"

#include "CSCData.h"
#include "CSCEllipticRegions.h"
#include "CSCMatrix2D.h"
#include "gabordescriptor.h"
#include "gaborfilter.h"
#include "harrisdetector.h"
#include "inputconnector.h"
#include "module.h"
#include "outputconnector.h"
#include "serializer.h"
#include "cvimage.h"
#include "resize.h"
#include "convertflt.h"
#include "CSCVector.h"
#include "convertcvimagegray.h"
#include "displaycvimage.h"
#include "drawcross.h"
#include "csctype.h"
#include "loadcvimage.h"
#include "camera.h"
#include "detectobject.h"
#include "drawrect.h"
#include "cuetemplate.h"
#include "cueprediction.h"
#include "cuemotion.h"
#include "cuecolorkernel.h"
#include "cueintegration.h"
#include "convert32FTo8U.h"
//#include "cuecontrastkernel.h"
#include "box.h"
#include "drawdisparity.h"
#include "attention.h"
#include "attention2.h"
#include "torichard.h"


#include <iostream>
#include <vector>

using namespace std; 
#define CUEINTEGRATION 0

//Pramod added starts
#define ONLINE_RECOGNITION 1
#if ONLINE_RECOGNITION
#include "detectsavedobject.h"
#include "objectComparison.h";
#define DETECTION_BLOB 1
#endif
//Pramod added ends

int main()
{


	char key;

	bool select = true;

	std::string object;
	std::string go;

	int delay = 2;

	bool g_quit = true;

	int objcount= 0;

	int savcount = 0;

	//////////////////////////////////
	//								//
	//	Creating Modules			//
	//								//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	camera cam("Camera");

	//////////////////////////////////
	//								//
	//	Gabor Matching				//
	//								//	DECLARATION
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	DrawCross drawLeft("DrawLeft");
	DrawCross drawRight("DrawRight");

	DrawRect DrawRectLeft("DrawRectLeft");
	DrawRect DrawRectRight("DrawRectRight");

	DisplayCVImage dispLeft("Left", false);
	DisplayCVImage dispRight("Right", false);

	DisplayCVImage dispOrLeft("Or_Left",false);
	DisplayCVImage dispOrRight("Or_Right",false);

	DetectObject detect("DetectObject");
	detect.debug = true;

	Box DrawBox("DrawBox");

	ToRichard ToRichardL("RichardLeft");
	ToRichard ToRichardR("RichardRight");

	//Pramod added starts
#if ONLINE_RECOGNITION
	DetectSavedObject detectsavedLeft("DetectSavedObjectLeft");
	DetectSavedObject detectsavedRight("DetectSavedObjectRight");

	objectCompare ObjectComparisonL("comparisonLeft");
	objectCompare ObjectComparisonR("comparisonRight");
#endif
	//Pramod added ends

	//////////////////////////////////
	//								//
	//	Gabor Matching				//
	//								//	CONNECTION
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	cam.cvLeftImage2Out.connectTo(&(drawLeft.cvImageIn), false);
	cam.cvRightImage2Out.connectTo(&(drawRight.cvImageIn), false);

	// image from cam to module that draws the blue bounding box, the red cross (center of the object) and the green crosses
	cam.cvLeftImage2Out.connectTo(&(DrawRectLeft.cvImageIn), false);
	cam.cvRightImage2Out.connectTo(&(DrawRectRight.cvImageIn), false);

	cam.cvLeftGrayImage2Out.connectTo(&(detect.cvLeftGrayImageIn), false);
	cam.cvRightGrayImage2Out.connectTo(&(detect.cvRightGrayImageIn), false);

	detect.regionsLeftOut.connectTo(&(drawLeft.regionsIn), false);
	detect.regionsRightOut.connectTo(&(drawRight.regionsIn), false);

	// keypoints from the detect module to the module that draws them on the image containing the boundind box
	detect.regionsLeftOut.connectTo(&(DrawRectLeft.regionsIn), false);
	detect.regionsRightOut.connectTo(&(DrawRectRight.regionsIn), false);

	drawLeft.cvImageOut.connectTo(&(dispLeft.cvImageIn), false);
	drawRight.cvImageOut.connectTo(&(dispRight.cvImageIn), false);

	// structure containing the center of the object, the dimension of the bounding box to the module that draws the bounding box
	detect.trackOutL.connectTo(&(DrawRectLeft.trackIn));
	detect.trackOutR.connectTo(&(DrawRectRight.trackIn));

	// image from the module that draws the bounding box to the module that dispalys the image
	DrawRectLeft.cvImageOut.connectTo(&(dispOrLeft.cvImageIn), false);
	DrawRectRight.cvImageOut.connectTo(&(dispOrRight.cvImageIn), false);

	detect.regionsLeftOut.connectTo(&(ToRichardL.regionsIn), false);
	detect.regionsRightOut.connectTo(&(ToRichardR.regionsIn), false);

	//Pramod added starts
#if ONLINE_RECOGNITION
	//Left image in objectdetect.cpp is from camera
	cam.cvLeftGrayImage2Out.connectTo(&(detectsavedLeft.cvCameraGrayImageIn), false);
	cam.cvRightGrayImage2Out.connectTo(&(detectsavedRight.cvCameraGrayImageIn), false);
	//Right image in objectdetect.cpp is from the saved images one by one
	//ObjectComparisonL contains the left image of saved set and similarly for R
	ObjectComparisonL.cvsavedImageOut.connectTo(&(detectsavedLeft.cvDatabaseGrayImageIn), false);
	ObjectComparisonR.cvsavedImageOut.connectTo(&(detectsavedRight.cvDatabaseGrayImageIn), false);
	//Regions from only right detectsaved module is sent because it contains
	//the gabor matches on the saved image which will be counted to decide the presence or
	//absence of the object
	detectsavedLeft.regionsRightOut.connectTo(&(ObjectComparisonL.regionsInDatabase),false);
	detectsavedRight.regionsRightOut.connectTo(&(ObjectComparisonR.regionsInDatabase),false);

	detectsavedLeft.regionsLeftOut.connectTo(&(ObjectComparisonL.regionsInCamera),false);
	detectsavedRight.regionsLeftOut.connectTo(&(ObjectComparisonR.regionsInCamera),false);

	//detectsavedLeft.regionsLeftOut.connectTo(&(ObjectComparisonL.regionsIn), false);
	//detectsavedRight.regionsRightOut.connectTo(&(ObjectComparisonR.regionsIn), false);

	cam.cvLeftGrayImage2Out.connectTo(&(ObjectComparisonL.cvGrayImageIn), false);
	cam.cvRightGrayImage2Out.connectTo(&(ObjectComparisonR.cvGrayImageIn), false);

	//detect.regionsLeftOut.connectTo(&(ObjectComparisonL.regionsIn), false);
	//detect.regionsRightOut.connectTo(&(ObjectComparisonR.regionsIn), false);
#endif
	//Pramod added ends

	//////////////////////////////////
	//								//
	//	Draw Box					//
	//								//	CONNECTION
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	// image from camera to module that draws the green crosses (matching Gabor features)
	cam.cvLeftImage2Out.connectTo(&(DrawBox.cvImageInLeft), false);
	cam.cvRightImage2Out.connectTo(&(DrawBox.cvImageInRight), false);

	detect.regionsLeftOut.connectTo(&(DrawBox.regionsInLeft), false);
	detect.regionsRightOut.connectTo(&(DrawBox.regionsInRight), false);


	//////////////////////////////////
	//								//
	//	Gabor Matching				//
	//								//	INITIALIZE
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	TrackData trackL;
	TrackData trackR;

	TrackData* trackptrL;
	TrackData* trackptrR;

	CvPoint pos;

	pos = cvPoint(160, 120);

	drawLeft.pointIn.in(&pos);
	drawRight.pointIn.in(&pos);

	DrawBox.memoryall();

	//////////////////////////////////
	//								//	LOOP FOR CALCULATING THE CENTER AND THE SIZE OF THE OBJECT, IT SENDS THE CENTER OF
	//	Gabor Matching				//	THE OBJECT AFTER THAT AT LEAST 10 MATCHING FEATURES FOR 5 FRAMES ARE DETECTED, WHEN   		//				//	YOU ARE READY PRESS "Q" TO JUMP TO THE NEXT SECTION. (CENTER AND STDV ARE DISPLAYED)
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		DrawBox.createwin();
		dispLeft.createwin();
		dispRight.createwin();
		dispOrLeft.createwin();
		dispOrRight.createwin();
		DrawBox.moveWindow(350, 350);
		dispLeft.moveWindow(50, 50);
		dispRight.moveWindow(650, 50);
		dispOrLeft.moveWindow(50,650);
		dispOrRight.moveWindow(650,650);

		trackL.imageSize.width = 320;
		trackL.imageSize.height = 240;
		trackR.imageSize.width = 320;
		trackR.imageSize.height = 240;

		trackptrL = NULL;
		trackptrR = NULL;

		g_quit = false;

	//Pramod added starts
#if ONLINE_RECOGNITION
		int save;int count=0;
#if DETECTION_BLOB
		IplImage* blobObjectL;IplImage* blobObjectR;
		cvNamedWindow("objectBlobsL");
		cvNamedWindow("objectBlobsR");
#endif
#endif
	//Pramod added ends

		while(!g_quit)
		{
			cam.execute();
			cam.num1++;
			detect.execute();

			ToRichardL.execute();
			ToRichardR.execute();

			drawLeft.execute();
			dispLeft.execute();

			drawRight.execute();
			dispRight.execute();

			DrawRectLeft.execute();
			dispOrLeft.execute();

			DrawRectRight.execute();
			dispOrRight.execute();

			DrawBox.execute();

			//Pramod added starts
#if ONLINE_RECOGNITION

			printf("want to save? press 1 to save, any other key otherwise\n");
			//std::cin>>save;
			//if(save==1){//if(save)
			if(count==0||count==1||count==2)
			{
				ObjectComparisonL.saveImg();
				ObjectComparisonR.saveImg();
				//CvRect savrect1;savrect1.x=110;savrect1.y=70;savrect1.width=100;savrect1.height=100;
				//CvRect savrect2;savrect2.x=110;savrect2.y=70;savrect2.width=100;savrect2.height=100;
				CvRect savrect1;savrect1.x=140;savrect1.y=85;savrect1.width=40;savrect1.height=70;
				CvRect savrect2;savrect2.x=140;savrect2.y=85;savrect2.width=40;savrect2.height=70;
				ObjectComparisonL.saveRect(savrect1);
				ObjectComparisonR.saveRect(savrect2);
			}
#if DETECTION_BLOB
			IplImage* blobsAllObjectsL = cvCreateImage(cvSize(320,240),IPL_DEPTH_8U,3);
			cvSetZero(blobsAllObjectsL);
			IplImage* blobsAllObjectsR = cvCreateImage(cvSize(320,240),IPL_DEPTH_8U,3);
			cvSetZero(blobsAllObjectsR);
#endif
			if(count!=0&&count!=1&&count!=2){
			for(int i=0;i<ObjectComparisonL.count;i++)
			{
				ObjectComparisonL.readImg(i);
				ObjectComparisonR.readImg(i);
				detectsavedLeft.execute();
				detectsavedRight.execute();
				ObjectComparisonL.execute();
				ObjectComparisonR.execute();
				if(ObjectComparisonL.countMatching>5)
				{
					std::cout << " Object " << i << " is present at location  x " << ObjectComparisonL.objLocX<<"and y "<<ObjectComparisonL.objLocY<<" on left image \n";
#if DETECTION_BLOB
					blobObjectL = cvCloneImage(ObjectComparisonL.salimg);
					cvAdd(blobObjectL,blobsAllObjectsL,blobsAllObjectsL);
#endif
				}
				if(ObjectComparisonR.countMatching>5)
				{
					std::cout << " Object " << i << " is present at location  x " << ObjectComparisonR.objLocX<<"and y "<<ObjectComparisonR.objLocY<<" on right image \n";
#if DETECTION_BLOB
					blobObjectR = cvCloneImage(ObjectComparisonR.salimg);
					cvAdd(blobObjectR,blobsAllObjectsR,blobsAllObjectsR);
#endif
				}

			}
			}
#if DETECTION_BLOB
			cvMoveWindow("objectBlobsL",700,0);
			cvShowImage("objectBlobsL",blobsAllObjectsL);
			cvReleaseImage(&blobsAllObjectsL);
			cvMoveWindow("objectBlobsR",700,350);
			cvShowImage("objectBlobsR",blobsAllObjectsR);
			//cvWaitKey(0);
			cvReleaseImage(&blobsAllObjectsR);
#endif
			count++;

#endif
	//Pramod added ends


			trackptrL = detect.trackOutL.getBuffer();
			trackptrR = detect.trackOutR.getBuffer();

			if(trackptrL && trackptrR) {

				trackL.winnerPos = trackptrL->winnerPos;
				trackL.winnerSize.width = 50;
				trackL.winnerSize.height = 50;
				trackL.winnerRect.x = (trackptrL->winnerPos.x - 50);
				trackL.winnerRect.y = (trackptrL->winnerPos.y - 50);
				trackL.winnerRect.width = 50;
				trackL.winnerRect.height = 50;
				trackL.reliability = 1;

				trackR.winnerPos = trackptrR->winnerPos;
				trackR.winnerSize.width = 50;
				trackR.winnerSize.height = 50;
				trackR.winnerRect.x = (trackptrR->winnerPos.x - 50);
				trackR.winnerRect.y = (trackptrR->winnerPos.y - 50);
				trackR.winnerRect.width = 50;
				trackR.winnerRect.height = 50;
				trackR.reliability = 1;

				std::cout << "\nDetected::winnerRect Left = [" << trackL.winnerRect.x << " " << trackL.winnerRect.y << " "<<  				trackL.winnerRect.width << " " << trackL.winnerRect.height << "]";
				std::cout << "\nDetected::winnerRect Right = [" << trackR.winnerRect.x << " " << trackR.winnerRect.y << " "<<  				trackR.winnerRect.width << " " << trackR.winnerRect.height << "]";

			}
			ToRichardL.release();
			ToRichardR.release();
			key = cvWaitKey(delay);
			if (key == 'q')
				g_quit = true;


			char c = cvWaitKey(33);
			if(c==27) break;

		}

		dispLeft.destroywin();
		dispRight.destroywin();
		dispOrLeft.destroywin();
		dispOrRight.destroywin();
		DrawBox.destroywin();
		return 0;
}
