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
#include "detectobject_disp.h"
#include "cuetemplate.h"
#include "cueprediction.h"
#include "cuemotion.h"
#include "cuecolorkernel.h"
#include "cueintegration.h"
#include "convert32FTo8U.h"
#include "cueintegrationR.h"
#include "cuecontrastkernel.h"
#include "drawrect.h"
#include "box.h"
#include "drawdisparity.h"
#include "attention.h"
//#include "sendinput.h"
#include "attention2.h"
//#include "moveneck.h"
#include "manageobjlist.h"
#include "cscsave.h"
#include "saliency.h"
#include "torichard.h"



//Pramod added starts
#define ONLINE_RECOGNITION 1
#if ONLINE_RECOGNITION
#include "detectsavedobject.h"
#include "objectComparison.h";
#define DETECTION_BLOB 1
#endif
//Pramod added ends




#include <yarp/os/all.h>
#include <yarp/sig/all.h>


#include <iostream>
#include <vector>

using namespace std; 

using namespace yarp::os;
using namespace yarp::sig;


int main(int argc, char* argv[])
{


	//////////////////////////////////
	//				//
	//	Setting iCub		//
	//				//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	BufferedPort<ImageOf<PixelRgb> > imagePortLeft;  // make a port for reading images
	BufferedPort<ImageOf<PixelRgb> > imagePortRight;  // make a port for reading images

	BufferedPort<Bottle> SavePort;

	Network yarp; // set up yarp

	imagePortLeft.open("/optical/image/in/left");  // give the port a name
	imagePortRight.open("/optical/image/in/right");  // give the port a name

	SavePort.open("/optical/save/out");

	Network::connect("/icub/cam/left","/optical/image/in/left");
	Network::connect("/icub/cam/right","/optical/image/in/right");


	//////////////////////////////////
	//				//
	//	Inizializing		//
	//				//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	ImageOf<PixelRgb> *imageLeft = NULL;
	ImageOf<PixelRgb> *imageRight = NULL;

	char key;

	bool select = true;

	std::string object;
	std::string go;

	int delay = 2;

	bool g_quit = true;

	int objcount= 0;

	bool debug = false;


	//////////////////////////////////
	//				//
	//	Creating Modules	//
	//				//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	camera cam("Camera");


	//////////////////////////////////
	//				//
	//	Attention Mechanism	//	
	//				//	DECLARATION
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	saliency sal("Saliency");

	Att AttentionL("Attention_Left");
	Att AttentionR("Attention_Right");
	
	int connection;
	int centering;
	double x;
	double y;



	AttentionL.memoryall();
	AttentionR.memoryall();


	//////////////////////////////////
	//				//
	//	Attention Mechanism	//	
	//				//	CONNECTION
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cam.cvLeftImage2Out.connectTo(&(sal.cvImageInL), false);
	cam.cvRightImage2Out.connectTo(&(sal.cvImageInR), false);


	//////////////////////////////////
	//				//
	//	Gabor Matching		//
	//				//	DECLARATION
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	DrawCross drawLeft("DrawLeft");
	DrawCross drawRight("DrawRight");

	DrawCross drawLeftdisp("DrawLeftDisp");
	DrawCross drawRightdisp("DrawRightDisp");

	DisplayCVImage dispLeft("Left", false);
	DisplayCVImage dispRight("Right", false);

	DisplayCVImage dispLeftdisp("LeftDisp", false);
	DisplayCVImage dispRightdisp("RightDisp", false);

	DetectObject detect("DetectObject");
	detect.debug = true;

	DetectObjectDisp detectdisp("DetectObjectDisp");
	detectdisp.debug = true;

	Box DrawBox("DrawBox");

	Box DrawBoxdisp("DrawBoxDisp");

	ToRichard ToRichardL("RichardLeft");
	ToRichard ToRichardR("RichardRight");

	ToRichard ToRichardLdisp("RichardLeftDisp");
	ToRichard ToRichardRdisp("RichardRightDisp");




	//////////////////////////////////
	//				//
	//	Recognition		//
	//				//	DEFINITION
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	//Pramod added starts
#if ONLINE_RECOGNITION
	DetectSavedObject detectsavedLeft("DetectSavedObjectLeft");
	DetectSavedObject detectsavedRight("DetectSavedObjectRight");

	objectCompare ObjectComparisonL("comparisonLeft");
	objectCompare ObjectComparisonR("comparisonRight");
#endif
	//Pramod added ends






	//////////////////////////////////
	//				//
	//	Gabor Matching		//
	//				//	CONNECTION
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	cam.cvLeftImage2Out.connectTo(&(drawLeft.cvImageIn), false);
	cam.cvRightImage2Out.connectTo(&(drawRight.cvImageIn), false);

	cam.cvLeftImage2Out.connectTo(&(drawLeftdisp.cvImageIn), false);
	cam.cvRightImage2Out.connectTo(&(drawRightdisp.cvImageIn), false);

	cam.cvLeftGrayImage2Out.connectTo(&(detect.cvLeftGrayImageIn), false);
	cam.cvRightGrayImage2Out.connectTo(&(detect.cvRightGrayImageIn), false);

	cam.cvLeftGrayImage2Out.connectTo(&(detectdisp.cvLeftGrayImageIn), false);
	cam.cvRightGrayImage2Out.connectTo(&(detectdisp.cvRightGrayImageIn), false);

	detect.regionsLeftOut.connectTo(&(drawLeft.regionsIn), false);
	detect.regionsRightOut.connectTo(&(drawRight.regionsIn), false);

	detectdisp.regionsLeftOut.connectTo(&(drawLeftdisp.regionsIn), false);
	detectdisp.regionsRightOut.connectTo(&(drawRightdisp.regionsIn), false);

	drawLeft.cvImageOut.connectTo(&(dispLeft.cvImageIn), false);
	drawRight.cvImageOut.connectTo(&(dispRight.cvImageIn), false);

	drawLeftdisp.cvImageOut.connectTo(&(dispLeftdisp.cvImageIn), false);
	drawRightdisp.cvImageOut.connectTo(&(dispRightdisp.cvImageIn), false);

	detect.regionsLeftOut.connectTo(&(ToRichardL.regionsIn), false);
	detect.regionsRightOut.connectTo(&(ToRichardR.regionsIn), false);

	detectdisp.regionsLeftOut.connectTo(&(ToRichardLdisp.regionsIn), false);
	detectdisp.regionsRightOut.connectTo(&(ToRichardRdisp.regionsIn), false);

	ToRichardL.arrayOut.connectTo(&(sal.arrayInL),false);
	ToRichardR.arrayOut.connectTo(&(sal.arrayInR),false);

	ToRichardL.sizearrayOut.connectTo(&(sal.sizearrayInL),false);
	ToRichardL.sizearrayOut.connectTo(&(sal.sizearrayInR),false);

	ToRichardLdisp.sizearrayOut.connectTo(&(sal.sizearraydispInL),false);
	ToRichardLdisp.sizearrayOut.connectTo(&(sal.sizearraydispInR),false);


	//////////////////////////////////
	//				//
	//	Draw Box		//
	//				//	CONNECTION
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	// image from camera to module that draws the green crosses (matching Gabor features)
	cam.cvLeftImage2Out.connectTo(&(DrawBox.cvImageInLeft), false);
	cam.cvRightImage2Out.connectTo(&(DrawBox.cvImageInRight), false);

	cam.cvLeftImage2Out.connectTo(&(DrawBoxdisp.cvImageInLeft), false);
	cam.cvRightImage2Out.connectTo(&(DrawBoxdisp.cvImageInRight), false);

	detect.regionsLeftOut.connectTo(&(DrawBox.regionsInLeft), false);
	detect.regionsRightOut.connectTo(&(DrawBox.regionsInRight), false);

	detectdisp.regionsLeftOut.connectTo(&(DrawBoxdisp.regionsInLeft), false);
	detectdisp.regionsRightOut.connectTo(&(DrawBoxdisp.regionsInRight), false);



	//////////////////////////////////
	//				//
	//	Recognition		//
	//				//	CONNECTION
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


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




	//////////////////////////////////
	//				//
	//	Gabor Matching		//
	//				//	INITIALIZE
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	
	TrackData trackL;
	TrackData trackR;

	TrackData* trackptrL;
	TrackData* trackptrR;

	CvPoint pos;

	pos = cvPoint(160, 120);

	drawLeft.pointIn.in(&pos);
	drawRight.pointIn.in(&pos);

	drawLeftdisp.pointIn.in(&pos);
	drawRightdisp.pointIn.in(&pos);

	DrawBox.memoryall();

	DrawBoxdisp.memoryall();



	//////////////////////////////////
	//				//
	//	Gabor Matching		//
	//				//	INITIALIZE
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	//Pramod added starts
#if ONLINE_RECOGNITION
		int save;int count=0;
#if DETECTION_BLOB
		IplImage* blobObjectL;
		IplImage* blobObjectR;
		cvNamedWindow("objectBlobsL");
		cvNamedWindow("objectBlobsR");
#endif
#endif
	//Pramod added ends


	if(debug)
	{
		printf("\n1\n");
		cvWaitKey(0);
	}




	std::cout << "\nOBJECT NUMBER "<<objcount<<"\nPress any key to go to the next step: \n";
	std::cin >> go;
	std::cout << "\nOk...let's go!";



	while(select==true)//chiudere
	{
		sleep(1);


		//////////////////////////////////
		//				//
		//	Cue Integration		//
		//				//	CREATING MODULES FOR CUE INTEGRATION
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	//	SendInput SendFromCue("SendFromCue");	

		CueTemplate cuetemplateL("CueTemplate Left");
		//CuePrediction cuepredictionL("CuePrediction Left");
		//CueMotion cuemotionL("CueMotion Left");
		//CueColorKernel cuecolorkernelL("CueColorKernel Left");
		//CueContrastKernel cuecontrastkernelL("CueContrastKernel Left");

		CueTemplate cuetemplateR("CueTemplate Right");
		//CuePrediction cuepredictionR("CuePrediction Right");
		//CueMotion cuemotionR("CueMotion Right");
		CueColorKernel cuecolorkernelR("CueColorKernel Right");
		CueContrastKernel cuecontrastkernelR("CueContrastKernel Right");

		CueIntegration cueintegrationL("CueIntegration left");
		CueIntegrationR cueintegrationR("CueIntegrationR right");

		cueintegrationL.setthres(0.2);
		cueintegrationL.addCue(&cuetemplateL, 1);
		//cueintegrationL.addCue(&cuepredictionL, 0.2);
		//cueintegrationL.addCue(&cuemotionL, 0.1);
		//cueintegrationL.addCue(&cuecolorkernelL, 0.3);
		//cueintegrationL.addCue(&cuecontrastkernelL, 0.3);//0.3

		cueintegrationR.setthres(0.2);
		cueintegrationR.addCue(&cuetemplateR, 1);
		//cueintegrationR.addCue(&cuepredictionR, 0.2);
		//cueintegrationR.addCue(&cuemotionR, 0.1);
		//cueintegrationR.addCue(&cuecolorkernelR, 0.3);
		//cueintegrationR.addCue(&cuecontrastkernelR, 0.3);//0.3

		Convert32FTo8U convertintegrationL("Convert32FTo8U CueIntegration Left");
		Convert32FTo8U convertintegrationR("Convert32FTo8U CueIntegration Right");

		DisplayCVImage dispRealL("Real Left");
		DisplayCVImage dispRealR("Real Right");

		cvWaitKey(33);

		DisplayCVImage dispSalL("Sal Left");
		DisplayCVImage dispSalR("Sal Right");

		cvWaitKey(33);

		DisplayCVImage dispTempL("Template Left");
		DisplayCVImage dispTempR("Template Right");

		cvWaitKey(33);

		//////////////////////////////////
		//				//
		//	Cue Integration		//
		//				//	CONNECTION
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		cam.cvLeftImage2Out.connectTo(&(cueintegrationL.cvImageIn), false);
		cam.cvRightImage2Out.connectTo(&(cueintegrationR.cvImageIn), false);

		cam.cvLeftGrayImage2Out.connectTo(&(cueintegrationL.cvGrayImageIn), false);
		cam.cvRightGrayImage2Out.connectTo(&(cueintegrationR.cvGrayImageIn), false);

		cueintegrationL.cvSalImageOut.connectTo(&(convertintegrationL.cvImageIn), false);
		cueintegrationR.cvSalImageOut.connectTo(&(convertintegrationR.cvImageIn), false);

		convertintegrationL.cvImageOut.connectTo(&(dispSalL.cvImageIn), false);
		convertintegrationR.cvImageOut.connectTo(&(dispSalR.cvImageIn), false);

		cuetemplateL.cvTemplateImageOut.connectTo(&(dispTempL.cvImageIn), false);
		cuetemplateR.cvTemplateImageOut.connectTo(&(dispTempR.cvImageIn), false);

		//cueintegrationL.cvpointOutL.connectTo(&(SendFromCue.pointInLeft));
		//cueintegrationR.cvpointOutR.connectTo(&(SendFromCue.pointInRight));

		cam.cvLeftImage2Out.connectTo(&(dispRealL.cvImageIn), false);
		cam.cvRightImage2Out.connectTo(&(dispRealR.cvImageIn), false);


		//////////////////////////////////
		//				//
		//	Attention Mechanism	//	
		//				//	DRAW A GREEN BOX AROUND THE SALIENT OBJECT/S
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		if(debug)
		{
			objcount++;
			std::cout << "\nOBJECT NUMBER "<<objcount<<"\nPress any key to go to the next step: \n";
			std::cin >> go;
			std::cout << "\nOk...let's go!";
		}

		//////////////////////////////////
		//				//	LOOP FOR CALCULATING THE CENTER AND THE SIZE OF THE OBJECT, IT SENDS THE CENTER OF 
		//	Gabor Matching		//	THE OBJECT AFTER THAT AT LEAST 10 MATCHING FEATURES FOR 5 FRAMES ARE DETECTED, WHEN   		//				//	YOU ARE READY PRESS "Q" TO JUMP TO THE NEXT SECTION. (CENTER AND STDV ARE DISPLAYED)
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		DrawBox.createwin();
		dispLeft.createwin();
		dispRight.createwin();

		DrawBox.moveWindow(350, 350);
		dispLeft.moveWindow(50, 50);
		dispRight.moveWindow(650, 50);

		trackL.imageSize.width = 320;
		trackL.imageSize.height = 240;
		trackR.imageSize.width = 320;
		trackR.imageSize.height = 240;

		trackptrL = NULL;
		trackptrR = NULL;

		sal.createwind();

		CvPoint resultsL;
		CvPoint resultsR;
		
		int wait = 0;
		
		//MOVE
		/*
		cvNamedWindow( "ORIGINAL_LEFT", 1);
		IplImage* OriginalL;
		cvNamedWindow( "ORIGINAL_RIGHT", 1);
		IplImage* OriginalR;
		*/
		//MOVE

		g_quit = false;
		while(!g_quit)
		{

			imageLeft = imagePortLeft.read();	// read an image
			imageRight = imagePortRight.read();	// read an image

			cam.imageL = (IplImage*)imageLeft->getIplImage();
			cam.imageR = (IplImage*)imageRight->getIplImage();
	
			if (!cam.imageL || !cam.imageR) break;

			cam.execute();

			detect.execute();

			drawLeft.execute();
			dispLeft.execute();

			drawRight.execute();
			dispRight.execute();

			DrawBox.execute();

			ToRichardL.execute();

			ToRichardR.execute();

			sal.execute();
			
			resultsL = sal.pointOL;
			resultsR = sal.pointOR;
			
			double distlx = resultsL.x-160;
			double distly = resultsL.y-120;			
			double distrx = resultsR.x-160;
			double distry = resultsR.y-120;			

			//double rdistL = sqrt(distlx*distlx+distly*distly);
			//double rdistR = sqrt(distrx*distrx+distry*distry);

			/*
			OriginalL=(IplImage*)imageLeft->getIplImage();
			OriginalR=(IplImage*)imageRight->getIplImage();
			cvCircle(OriginalL, resultsL, 20, cvScalar(0,255,0), 1);
			cvCircle(OriginalR, resultsR, 20, cvScalar(0,255,0), 1);
			cvShowImage("ORIGINAL_LEFT",OriginalL);
			cvShowImage("ORIGINAL_RIGHT",OriginalR);
			*/
		
			ToRichardL.release();
			ToRichardR.release();

			key = cvWaitKey(delay);

			if(sal.move==true && fabs(distlx)<110 && fabs(distly)<70 && fabs(distrx)<110 && fabs(distry)<70)
			{
				g_quit = true;


				trackL.winnerPos = sal.pointOL;
				trackL.winnerSize.width = 50;
				trackL.winnerSize.height = 50;
				trackL.winnerRect.x = (sal.pointOL.x - 50);
				trackL.winnerRect.y = (sal.pointOL.y - 50);
				trackL.winnerRect.width = 50;
				trackL.winnerRect.height = 50;
				trackL.reliability = 1;

				trackR.winnerPos = sal.pointOR;
				trackR.winnerSize.width = 50;
				trackR.winnerSize.height = 50;
				trackR.winnerRect.x = (sal.pointOR.x - 50);
				trackR.winnerRect.y = (sal.pointOR.y - 50);
				trackR.winnerRect.width = 50;
				trackR.winnerRect.height = 50;
				trackR.reliability = 1;

				if(debug)
				{
					std::cout << "\nDetected::winnerRect Left = [" << trackL.winnerRect.x << " " << trackL.winnerRect.y << " "<<  						trackL.winnerRect.width << " " << trackL.winnerRect.height << "]";
					std::cout << "\nDetected::winnerRect Right = [" << trackR.winnerRect.x << " " << trackR.winnerRect.y << " "<<  						trackR.winnerRect.width << " " << trackR.winnerRect.height << "]";
					std::cout << "\nPress any key to go to the next step: ";
					std::cin >> go;
					std::cout << "\nOk...let's go!";
				}

				key = cvWaitKey(33);
				if (key == 'e') 
					g_quit = true;

				wait++;
			}



		}

		/*	cvReleaseImage(&OriginalL);
			cvReleaseImage(&OriginalR);
			cvDestroyWindow("ORIGINAL_LEFT");
			cvDestroyWindow("ORIGINAL_RIGHT");
		*/

		dispLeft.destroywin();
		dispRight.destroywin();
		DrawBox.destroywin();
		sal.destroywind();

		sleep(1);

		//////////////////////////////////
		//				//
		//	Cue Integration		//
		//				//	EXECUTE
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		std::cout << "\n\nmain()::initializing";

		dispRealL.createwin();
		dispRealL.moveWindow(50, 50);
		dispRealR.createwin();
		dispRealR.moveWindow(450, 50);
		dispSalL.createwin();	
		dispSalL.moveWindow(50, 380);
		dispSalR.createwin();
		dispSalR.moveWindow(380, 380);
		dispTempL.createwin();
		dispTempL.moveWindow(50, 650);
		dispTempR.createwin();
		dispTempR.moveWindow(650, 650);

		imageLeft = imagePortLeft.read();
		imageRight = imagePortRight.read();

		cam.imageL = (IplImage*)imageLeft->getIplImage();
		cam.imageR = (IplImage*)imageRight->getIplImage();

		cam.execute();

		cueintegrationL.trackIn.setBuffer(&trackL);
		cueintegrationR.trackIn.setBuffer(&trackR);

		cueintegrationL.connect();
		cueintegrationR.connect();

		cueintegrationL.initialize();
		cueintegrationR.initialize();

		cueintegrationL.trackIn.setBuffer(NULL);
		std::cout << "\nmain()::initializing complete";
		cueintegrationR.trackIn.setBuffer(NULL);
		std::cout << "\nmain()::initializing complete";

		centering = 0;
	
		if(debug)
		{
			printf("\n2\n");
			cvWaitKey(0);
		}

		g_quit = false;
		while(!g_quit) {

			//std::cout << "main()::tracking\n";

			imageLeft = imagePortLeft.read();
			imageRight = imagePortRight.read();

			cam.imageL = (IplImage*)imageLeft->getIplImage();
			cam.imageR = (IplImage*)imageRight->getIplImage();

			cam.execute();

			cueintegrationL.execute();
			cueintegrationR.execute();

	//		SendFromCue.execute();

			convertintegrationL.execute();
			convertintegrationR.execute();

			dispRealL.execute();
			dispRealR.execute();

			dispTempL.execute();
			dispTempR.execute();

			dispSalL.execute();
			dispSalR.execute();

			key = cvWaitKey(33);

			if(fabs(cueintegrationL.distxL)<5.1 && fabs(cueintegrationR.distxR)<5.1 )
			{

				printf("\nSTATE_TRACK distxL = %d, distxR = %d\n", cueintegrationL.distxL, cueintegrationR.distxR);
				centering ++;	
				if(centering>20)			
					break;

			}
		

			if(key == 'q') 
				g_quit = true;
			

		}


		dispRealL.destroywin();
		dispRealR.destroywin();
		dispSalL.destroywin();
		dispSalR.destroywin();
		dispTempL.destroywin();
		dispTempR.destroywin();

		sleep(1);

		if(debug)
		{
/////////////////////////////////////
			std::cout << "\nPress a key : ";
			std::cin >> go;
/////////////////////////////////////
		}

		cueintegrationL.clean();
		cueintegrationR.clean();

		sleep (1);

		//////////////////////////////////
		//				//	LOOP FOR CALCULATING THE CENTER AND THE SIZE OF THE OBJECT, IT SENDS THE CENTER OF 
		//	Gabor Matching		//	THE OBJECT AFTER THAT AT LEAST 10 MATCHING FEATURES FOR 5 FRAMES ARE DETECTED, WHEN   			//				//	YOU ARE READY PRESS "Q" TO JUMP TO THE NEXT SECTION. (CENTER AND STDV ARE DISPLAYED)
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		DrawBoxdisp.createwin();
		dispLeftdisp.createwin();
		dispRightdisp.createwin();

		DrawBoxdisp.moveWindow(350, 350);
		dispLeftdisp.moveWindow(50, 50);
		dispRightdisp.moveWindow(650, 50);

		int count=0;
		while(count<1)
		{

			imageLeft = imagePortLeft.read();	// read an image
			imageRight = imagePortRight.read();	// read an image

			cam.imageL = (IplImage*)imageLeft->getIplImage();
			cam.imageR = (IplImage*)imageRight->getIplImage();
	
			if (!cam.imageL || !cam.imageR) break;

			cam.execute();

			detectdisp.execute();

			drawLeftdisp.execute();
			dispLeftdisp.execute();

			drawRightdisp.execute();
			dispRightdisp.execute();

			DrawBoxdisp.execute();

			key = cvWaitKey(33);
			if (key == 'e') 
				g_quit = true;
			count++;








			//Pramod added starts
#if ONLINE_RECOGNITION

			//std::cin>>save;
			//if(save==1){//if(save)
			//if(count==0||count==1||count==2)
			//{
				ObjectComparisonL.saveImg();
				ObjectComparisonR.saveImg();
				CvRect savrect1;
				savrect1.x=110;	
				savrect1.y=70;
				savrect1.width=100;
				savrect1.height=100;
				CvRect savrect2;
				savrect2.x=110;
				savrect2.y=70;
				savrect2.width=100;
				savrect2.height=100;
				//CvRect savrect1;savrect1.x=140;savrect1.y=85;savrect1.width=40;savrect1.height=70;
				//CvRect savrect2;savrect2.x=140;savrect2.y=85;savrect2.width=40;savrect2.height=70;
				ObjectComparisonL.saveRect(savrect1);
				ObjectComparisonR.saveRect(savrect2);
			//}
#endif

			//Pramod added ends



		}

		dispLeftdisp.destroywin();
		dispRightdisp.destroywin();
		DrawBoxdisp.destroywin();

		sleep(1);

/////////////////////////////////////
		if(debug)
		{
			g_quit=false;
			while (!g_quit)
			{
				std::cout << "\nAgain? [Y/N] : ";
				std::cin >> go;
				if (go == "n" || go == "N"){

					std::cout << "You chose not to look at another object..\nSee you man\n";
					select = false;
					g_quit=true;
				}
				else if (go == "y" || go == "Y"){

					std::cout << "E che palle..\niCub è stressato...\n";
					g_quit=true;
				}
				else {
					std::cout << "Incorrect input!\n";
				}
			}
		}
/////////////////////////////////////


		g_quit = false;
		while(!g_quit)
		{
			//Pramod start adds
#if ONLINE_RECOGNITION
#if DETECTION_BLOB
			IplImage* blobsAllObjectsL = cvCreateImage(cvSize(320,240),IPL_DEPTH_8U,3);
			cvSetZero(blobsAllObjectsL);
			IplImage* blobsAllObjectsR = cvCreateImage(cvSize(320,240),IPL_DEPTH_8U,3);
			cvSetZero(blobsAllObjectsR);
#endif
			if(count!=0 && count!=1 && count!=2){
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
					std::cout << " Object " << i << " is present at location  x " << ObjectComparisonL.objLocX<<"and y 						"<<ObjectComparisonL.objLocY<<" on left image \n";
#if DETECTION_BLOB
					blobObjectL = cvCloneImage(ObjectComparisonL.salimg);
					cvAdd(blobObjectL,blobsAllObjectsL,blobsAllObjectsL);
#endif
				}
				if(ObjectComparisonR.countMatching>5)
				{
					std::cout << " Object " << i << " is present at location  x " << ObjectComparisonR.objLocX<<"and y 						"<<ObjectComparisonR.objLocY<<" on right image \n";
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
			key = cvWaitKey(33);
			if (key == 'e') 
				g_quit = true;
			cvReleaseImage(&blobsAllObjectsR);
#endif
			count++;

#endif
	//Pramod added ends

		}



		ToRichardLdisp.execute();
		ToRichardRdisp.execute();

		sal.inib();

		ToRichardLdisp.release();
		ToRichardRdisp.release();


	}

	imagePortLeft.interrupt();
	imagePortLeft.close();
	imagePortRight.interrupt();
	imagePortRight.close();
	SavePort.interrupt();
	SavePort.close();

	sleep(1);

	return 0;


}


	


