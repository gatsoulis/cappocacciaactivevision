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

	int savcount = 0;


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

	saliency salL("Saliency_Left");
	saliency salR("Saliency_Right");

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


	cam.cvLeftImage2Out.connectTo(&(salL.cvImageIn), false);
	cam.cvRightImage2Out.connectTo(&(salR.cvImageIn), false);

	cam.cvLeftImage2Out.connectTo(&(AttentionL.cvImageIn), false);
	cam.cvRightImage2Out.connectTo(&(AttentionR.cvImageIn), false);

	salL.pointOut.connectTo(&(AttentionL.pointIn));
	salR.pointOut.connectTo(&(AttentionR.pointIn));


	//////////////////////////////////
	//				//
	//	Gabor Matching		//
	//				//	DECLARATION
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


	//////////////////////////////////
	//				//
	//	Gabor Matching		//
	//				//	CONNECTION
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


	while(select==true)
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
	//	Cue Integration		//
	//				//	INITIALIZE
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	savcount = 0;


	//////////////////////////////////
	//				//
	//	Save Images		//
	//				//	CREATE MODULES
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	int framesize =500;
	std::string opt, path;
	opt = "--train";
	if(argc > 1) {
		opt = argv[1];
	}
	if(argc > 2) {
		framesize = atoi(argv[2]);
	}

	// Change the path to your preference
	if(opt == "--test1") path = "/media/Dati/Tesi_Specialistica/Software/WORKHOME/Database/testing1";
	else if(opt == "--test2") path = "/media/Dati/Tesi_Specialistica/Software/WORKHOME/Database/testing2";
	else if(opt == "--train") path = "/media/Dati/Tesi_Specialistica/Software/WORKHOME/Database/training";
	else { std::cerr << "Incorrect option [" << opt << "]\n Choose from [--train, --test1, --test2]."; return 0; }

	std::cout << "\n\nPath = " << path;
	std::cout << "\nFramesize = " << framesize << "\n";

	ManageObjList manage("ManageObjList");
	manage.debug = true;
	manage.setParam(path);

	CSCSave save("CSCSave");
	save.debug = true;
	save.setParam(path, framesize, "jpg", cvSize(320, 240));


	//////////////////////////////////
	//				//
	//	Save Images		//
	//				//	CONNECTION
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	cam.cvLeftImage2Out.connectTo(&(save.cvLeftImageIn), false);			//c'era cvLeftImage1Out.connect
	cam.cvRightImage2Out.connectTo(&(save.cvRightImageIn), false);			//c'era cvLeftImage1Out.connect cambiato in cue integrationL
	cueintegrationL.trackOut.connectTo(&(save.trackIn), false);		//ATTENZIONE ATTENZIONE PROTREBBE ESSERE UN PROBLEMA(sotto l'ho commentato


	//////////////////////////////////
	//				//
	//	Attention Mechanism	//	
	//				//	DRAW A GREEN BOX AROUND THE SALIENT OBJECT/S
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




		objcount++;
		std::cout << "\nOBJECT NUMBER "<<objcount<<"\nPress any key to go to the next step: \n";
		std::cin >> go;
		std::cout << "\nOk...let's go!";

	g_quit = false;
	while(!g_quit)
	{

		imageLeft = imagePortLeft.read();	
		imageRight = imagePortRight.read();	

		cam.imageL = (IplImage*)imageLeft->getIplImage();
		cam.imageR = (IplImage*)imageRight->getIplImage();
	
		if (!cam.imageL || !cam.imageR) break;

		cam.execute();

		salL.execute();
//cvWaitKey(0);
		salR.execute();

		key = cvWaitKey(delay);
		if (key == 'q') 
			g_quit = true;
	}

		AttentionL.execute();
		AttentionR.execute();
		AttentionL.moveWindow(50,50);	//cueintegrationL.cvpointOutL.connectTo(&(SendFromCue.pointInLeft));
	//cueintegrationR.cvpointOutR.connectTo(&(SendFromCue.pointInRight));
		AttentionR.moveWindow(50,350);
	
		cout << endl << "Press:" << endl;
		cout << " 't' to capture a new template on the left image;" << endl;
		cout << " 'y' to capture a new template on the right image;" << endl;
		cout << " 'd' to enable/disable scale and rotation invariance;" << endl;
		cout << " 'q' to quit." << endl;

		connection = 0;	
		centering = 0;
		x = 0.0;
		y = 0.0;

		while(true){
	
			imageLeft = imagePortLeft.read();	
			imageRight = imagePortRight.read();	

			cam.imageL = (IplImage*)imageLeft->getIplImage();
			cam.imageR = (IplImage*)imageRight->getIplImage();
		
			if (!cam.imageL || !cam.imageR) break;

			cam.execute();

			AttentionL.run();
			AttentionR.run();

			if((AttentionL.gettemp == true) && (AttentionR.gettemp == true))
			{
			
				if(connection<1)
				{
					AttentionL.connecting();
					AttentionR.connecting();
					connection ++;
				}

				if(fabs(AttentionL.distx)<4.1 && fabs(AttentionR.distx)<4.1 )
				{

					/*printf("\nSTATE_TRACK distanceL = %f, distanceR = %f", sqrt(AttentionL.distx*AttentionL.distx + 					AttentionL.disty*AttentionL.disty) , sqrt(AttentionR.distx*AttentionR.distx + AttentionR.disty*AttentionR.disty));*/
					printf("\nSTATE_TRACK distxL = %d, distxR = %d", AttentionL.distx, AttentionR.distx);
					centering ++;	
					if(centering>20)			
						break;

				}
			}

			if(AttentionL.one==false){

				key = cvWaitKey(33);
				if (key == 'e') break;
			}

			key = cvWaitKey(2);
			if (key == 'c') break;

		}

		std::cout << "\nPress any key to go to the next step: ";
		std::cin >> go;
		std::cout << "\nOk...let's go!";

		AttentionL.clean();
		AttentionR.clean();	

		

	//////////////////////////////////
	//				//	LOOP FOR CALCULATING THE CENTER AND THE SIZE OF THE OBJECT, IT SENDS THE CENTER OF 
	//	Gabor Matching		//	THE OBJECT AFTER THAT AT LEAST 10 MATCHING FEATURES FOR 5 FRAMES ARE DETECTED, WHEN   		//				//	YOU ARE READY PRESS "Q" TO JUMP TO THE NEXT SECTION. (CENTER AND STDV ARE DISPLAYED)
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

			DrawRectLeft.execute();
			dispOrLeft.execute();

			DrawRectRight.execute();
			dispOrRight.execute();
	
			DrawBox.execute();

			trackptrL = detect.trackOutL.getBuffer();
			trackptrR = detect.trackOutR.getBuffer();
			
			/*
			if(trackptrL && trackptrR) {
				trackL.winnerPos = trackptrL->winnerPos;
				trackL.winnerSize = trackptrL->winnerSize;
				trackL.winnerRect = trackptrL->winnerRect;
				trackL.reliability = trackptrL->reliability;

				trackR.winnerPos = trackptrR->winnerPos;
				trackR.winnerSize = trackptrR->winnerSize;
				trackR.winnerRect = trackptrR->winnerRect;
				trackR.reliability = trackptrR->reliability;

				std::cout << "Detected::winnerRect Left = [" << trackL.winnerRect.x << " " << trackL.winnerRect.y << " " 					<< trackL.winnerRect.width << " " << trackL.winnerRect.height << "]\n";
				std::cout << "Detected::winnerRect Right = [" << trackR.winnerRect.x << " " << trackR.winnerRect.y << " " 					 << trackR.winnerRect.width << " " << trackR.winnerRect.height << "]\n";
				g_quit = true;
			}
			*/
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
				std::cout << "\nPress any key to go to the next step: ";
				std::cin >> go;
				std::cout << "\nOk...let's go!";
				g_quit=true;

			}

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


		//////////////////////////////////
		//				//
		//	Cue Integration		//
		//				//	EXECUTE
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		if (key == 'q') return 0;

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



	//	SendFromCue.connecting();

		
		
		g_quit = false;
		while(!g_quit) {

			//std::cout << "main()::tracking\n";
			savcount++;

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
			
			//if(savcount>100)
			//	g_quit=true;

			key = cvWaitKey(delay);

			//if(savcount%5==0)
			save.execute();
			printf("\nITERATION=%d\n",savcount);

			if(save.isBufferFull())
			{
		 		Bottle& saveout = SavePort.prepare();
				saveout.clear();
				saveout.addString("ImageSaved");
				SavePort.write(); 
				g_quit = true;
			}

	
			if(key == 'q') 
				g_quit = true;
			

		}

	//	SendFromCue.disconnecting();


		//////////////////////////////////
		//				//
		//	Save Images		//
		//				//	STORING IMAGE TO HARD DRIVE
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		if(save.m_counter == 0) return 0;
		std::cout << "Total frames in the buffer : " << save.m_counter << " frames\n";
		if(key == 'q') return 0;

		std::string name, toAppend, toSave;
		bool temp1, temp2 = false, temp3 = false, temp4 = false;
		while(!temp3) {
			std::cout << "\nSave the buffered images to hard-drive ? [y/N] : ";
			std::cin >> toSave;
			if (toSave == "n" || toSave == "N"){
				std::cout << "You chose not to save the images..\n";
				temp3 = true;
			}
			else if(toSave == "y" || toSave == "Y") {
				while(!temp2) {
					std::cout << "Please type the name of the object : ";
					std::cin >> name;
					if (name == "") { std::cout << "\nYou didnt type the name.\n"; continue; }
					temp1 = manage.checkObjName(name);
					if (temp1 == true){
						std::cout << "Object name [" << name << "] already exists in the image DB\n";
						while(!temp4) {
							std::cout << "Do you want to append to the current DB? [y/N]";
							std::cin >> toAppend;
							if(toAppend == "y" || toAppend == "Y"){
								std::cout << "Adding buffered images to the DB. This will take a while. Please wait..\n";
								save.labelIn.in(&name);
								save.appendToDisk();
								temp2 = true;
								temp4 = true;
							}
							else if(toAppend == "n" || toAppend == "N") {
								std::cout << "You chose not to append.\n";
								temp4 = true;
							}
							else{
								std::cerr << "Incorrect input [" << toAppend << "] !\n";
							}
						}
						if(temp2 != true) temp4 = false;
					}
					else{
						std::cout << "The name of the object is : " << name << "\n";
						std::cout << "Saving buffered images to hard-drive. This will take a while. Please wait..\n";
						save.labelIn.in(&name);
						save.saveToDisk();
						manage.attachObj(name);
						temp2 = true;
					}
				}
				temp3 = true;
			}
			else {
				std::cerr << "Incorrect input [" << toSave << "] !\n";
			}
		}

		std::cout << "\nAll stages finished. Quitting the program..\n";

		dispRealL.destroywin();
		dispRealR.destroywin();
		dispSalL.destroywin();
		dispSalR.destroywin();
		dispTempL.destroywin();
		dispTempR.destroywin();

		sleep(1);

/////////////////////////////////////
		std::cout << "\nSelect another object? [Y/N] : ";
		std::cin >> object;
		if (object == "n" || object == "N"){

			std::cout << "You chose not to select another object..\nSee you man\n";
			select = false;
		}
		else if (object == "y" || object == "Y"){

			std::cout << "You chose to select another object..\niCub is stressed...\n";
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

