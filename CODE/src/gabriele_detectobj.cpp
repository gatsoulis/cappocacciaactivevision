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

	DisplayCVImage dispLeft("Left", false);
	DisplayCVImage dispRight("Right", false);

	DetectObject detect("DetectObject");
	detect.debug = true;

	Box DrawBox("DrawBox");

	ToRichard ToRichardL("RichardLeft");
	ToRichard ToRichardR("RichardRight");

	//////////////////////////////////
	//				//
	//	Gabor Matching		//
	//				//	CONNECTION
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	cam.cvLeftImage2Out.connectTo(&(drawLeft.cvImageIn), false);
	cam.cvRightImage2Out.connectTo(&(drawRight.cvImageIn), false);

	cam.cvLeftGrayImage2Out.connectTo(&(detect.cvLeftGrayImageIn), false);
	cam.cvRightGrayImage2Out.connectTo(&(detect.cvRightGrayImageIn), false);

	detect.regionsLeftOut.connectTo(&(drawLeft.regionsIn), false);
	detect.regionsRightOut.connectTo(&(drawRight.regionsIn), false);

	drawLeft.cvImageOut.connectTo(&(dispLeft.cvImageIn), false);
	drawRight.cvImageOut.connectTo(&(dispRight.cvImageIn), false);

	detect.regionsLeftOut.connectTo(&(ToRichardL.regionsIn), false);
	detect.regionsRightOut.connectTo(&(ToRichardR.regionsIn), false);

	ToRichardL.arrayOut.connectTo(&(sal.arrayInL),false);
	ToRichardR.arrayOut.connectTo(&(sal.arrayInR),false);

	ToRichardL.sizearrayOut.connectTo(&(sal.sizearrayInL),false);
	ToRichardL.sizearrayOut.connectTo(&(sal.sizearrayInR),false);


	//////////////////////////////////
	//				//
	//	Draw Box		//
	//				//	CONNECTION
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	// image from camera to module that draws the green crosses (matching Gabor features)
	cam.cvLeftImage2Out.connectTo(&(DrawBox.cvImageInLeft), false);
	cam.cvRightImage2Out.connectTo(&(DrawBox.cvImageInRight), false);

	detect.regionsLeftOut.connectTo(&(DrawBox.regionsInLeft), false);
	detect.regionsRightOut.connectTo(&(DrawBox.regionsInRight), false);


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

	DrawBox.memoryall();


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
		CuePrediction cuepredictionL("CuePrediction Left");
		CueMotion cuemotionL("CueMotion Left");
		CueColorKernel cuecolorkernelL("CueColorKernel Left");
		CueContrastKernel cuecontrastkernelL("CueContrastKernel Left");

		CueTemplate cuetemplateR("CueTemplate Right");
		CuePrediction cuepredictionR("CuePrediction Right");
		CueMotion cuemotionR("CueMotion Right");
		CueColorKernel cuecolorkernelR("CueColorKernel Right");
		CueContrastKernel cuecontrastkernelR("CueContrastKernel Right");

		CueIntegration cueintegrationL("CueIntegration left");
		CueIntegrationR cueintegrationR("CueIntegrationR right");

		cueintegrationL.setthres(0.2);
		cueintegrationL.addCue(&cuetemplateL, 1.0);
		cueintegrationL.addCue(&cuepredictionL, 0.1);
		cueintegrationL.addCue(&cuemotionL, 0.1);
		cueintegrationL.addCue(&cuecolorkernelL, 0.4);
		cueintegrationL.addCue(&cuecontrastkernelL, 0.3);

		cueintegrationR.setthres(0.2);
		cueintegrationR.addCue(&cuetemplateR, 1.0);
		cueintegrationR.addCue(&cuepredictionR, 0.1);
		cueintegrationR.addCue(&cuemotionR, 0.1);
		cueintegrationR.addCue(&cuecolorkernelR, 0.4);
		cueintegrationR.addCue(&cuecontrastkernelR, 0.3);

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


		objcount++;
		std::cout << "\nOBJECT NUMBER "<<objcount<<"\nPress any key to go to the next step: \n";
		std::cin >> go;
		std::cout << "\nOk...let's go!";


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
		
		//MOVE
		/*
		cvNamedWindow( "ORIGINAL_LEFT", 1);
		IplImage* OriginalL;
		cvNamedWindow( "ORIGINAL_RIGHT", 1);
		IplImage* OriginalR;
		CvPoint resultsL;
		CvPoint resultsR;
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
			
			/*
			resultsL = sal.pointOL;
			resultsR = sal.pointOR;

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

			if(sal.move==true)
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
				if (key == 'q') 
					g_quit = true;
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

/////////////////////////////////////
		std::cout << "\nAgain? [Y/N] : ";
		std::cin >> go;
		if (go == "n" || go == "N"){

			std::cout << "You chose not to look at another object..\nSee you man\n";
			select = false;
		}
		else if (object == "y" || object == "Y"){

			std::cout << "E che palle..\niCub è stressato...\n";
		}
/////////////////////////////////////

		cueintegrationL.clean();
		cueintegrationR.clean();
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


	


