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

	cam.cvLeftImage2Out.connectTo(&(AttentionL.cvImageIn), false);
	cam.cvRightImage2Out.connectTo(&(AttentionR.cvImageIn), false);

	sal.pointOutL.connectTo(&(AttentionL.pointIn));
	sal.pointOutR.connectTo(&(AttentionR.pointIn));


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

	CvPoint pos;

	pos = cvPoint(160, 120);

	drawLeft.pointIn.in(&pos);
	drawRight.pointIn.in(&pos);

	DrawBox.memoryall();


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
while(true){

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
		if (key == 'q') 
			g_quit = true;


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
	//	Attention Mechanism	//	
	//				//	DRAW A GREEN BOX AROUND THE SALIENT OBJECT/S
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



	AttentionL.execute();
	AttentionR.execute();
	AttentionL.moveWindow(50,50);
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

			if(fabs(AttentionL.distx)<5.1 && fabs(AttentionR.distx)<5.1 )
			{


				printf("\nSTATE_TRACK distxL = %d, distxR = %d", AttentionL.distx, AttentionR.distx);
				centering ++;	
				if(centering>5)			
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

	AttentionL.clean();
	AttentionR.clean();	

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


	


