//======================================================================================================================================
// Name        : main.cpp
// Revisited by: GABBOSHOW
// Version     : 1
// Copyright   : Your copyright notice
// Description : Tentativo (presumibilmente fallimentare) di implementare il codice di new_segmentation con immagini da webcam
//======================================================================================================================================


#include <iostream>
#include <time.h>
#include <cv.h>
#include <highgui.h>
#include <cxcore.h>
#include <yarp/os/all.h>
#include <yarp/sig/all.h>
#include "highgui.h"
#include "Segmentation.h"

using namespace std;
using namespace yarp::sig;
using namespace yarp::os;

#define BILLION  1000000000L;   //non so perchè

void imAdujstEnahancement(IplImage* image);
void imAdjust(IplImage *image);
void showImg(IplImage *Image, char *cName);


int main( int argc, char** argv) { //aggiunto al posto di int main() {
        cout << "!!!Cue Integration!!!" << endl; // prints !!!Cue Integration!!!

        struct timespec start, stop;
        double accum;
        if( clock_gettime( CLOCK_REALTIME, &start) == -1 ) {
                perror( "clock gettime" );
                exit( EXIT_FAILURE );
        }



//======================================================================================================================================
// Added
//======================================================================================================================================        
	//cvNamedWindow( "Original_Video", CV_WINDOW_AUTOSIZE );
        //cvNamedWindow( "Segmentation_Video", 0 );

        //cvNamedWindow( "IBG_MODEL", 0 );

        //cvNamedWindow( "textureLikelihood", CV_WINDOW_AUTOSIZE );
        //cvNamedWindow( "output", CV_WINDOW_AUTOSIZE );
        

	int fCount=1;
	IplImage* frame1;//=cvCreateImage(size, IPL_DEPTH_8U, 3);
	IplImage* frame_temp;
	ImageOf<PixelRgb> *img_new;
	BufferedPort<ImageOf<PixelRgb> > img_in_port;  // added make a port for reading images
//	BufferedPort<ImageOf<PixelRgb> > img_out_port;    // added
	img_in_port.setStrict();
//	img_out_port.setStrict();//added
	CvCapture* original_capture;

	if(iDataChoice==0){
		typedef yarp::sig::ImageOf<yarp::sig::PixelRgb> DataType;

//GGG		DataType *img_new ;
			
		//CvBGStatModel* bg_model;

		yarp::os::Network net;
///GGG		yarp::os::BufferedPort<DataType> img_in_port;
		Network yarp;



		
		
		//img_out_port.open("/img_out");
		img_in_port.open("/img_inn");
		net.connect("/icub/cam/left", "/img_inn");

		img_new = img_in_port.read();
		frame_temp = (IplImage*)img_new->getIplImage();
		
		frame1 = cvCreateImage( cvSize(frame_temp->width,  frame_temp->height), frame_temp->depth, 3 );
	    	
  	  	cvCvtColor(frame_temp,frame1, CV_RGB2BGR);
 		frame1->origin=0;
 		
	}


	else{

		
	        original_capture=cvCreateCameraCapture(0);
	       	assert(original_capture!=NULL);
        	CvSize size = cvSize(
        	                (int)cvGetCaptureProperty(original_capture,CV_CAP_PROP_FRAME_WIDTH),
        	                (int)cvGetCaptureProperty(original_capture,CV_CAP_PROP_FRAME_HEIGHT)
        	                );
		frame1 = cvQueryFrame( original_capture);
		printf("The video image is %d X %d",size.width, size.height);     	
	}

   

//======================================================================================================================================


        
        

        
	//showImg(frame1, "First_frame");
        //cvWaitKey();
	

	IplImage* frame=cvCreateImage(cvSize(320,240), IPL_DEPTH_8U, 3);
	cvResize(frame1,frame);
	showImg(frame, "First_frame (converted)");
        cvWaitKey();

        imAdujstEnahancement(frame);  //chiama la funzione imAdukstEnancement

	

//======================================================================================================================================
// Cue Integration Initialization
//======================================================================================================================================

        Cue_integration CI_obj;
        CI_obj.initialize(frame);       //chiama la funzione che si trova in segmentation.cpp: "void Cue_integration::initialize(IplImage *image)"

        //showImg(frame, "initialize");     //add
        //cvWaitKey();                                    //add

        CI_obj.IBGModel[iBgModelSize] = cvCloneImage(frame);

        //showImg(CI_obj.IBGModel[iBgModelSize],"IBG_MODEL");       //add
        //cvWaitKey();                                    //add


        long tsFrame, teFrame, tsFun1, teFun1, tsFun2, teFun2, tsFun3, teFun3, tFun1 = 0, tFun2 = 0, tFun3 = 0;
        char msg[100];
        bool bObjA, bPosterioInit = false;

int cnt=-1;
	for (fCount = iInitialFrame + 1; ; fCount++){
                printf("\n Frame No:%d", fCount);
                
		if(iDataChoice==0){
			img_new = img_in_port.read();
cnt++; //change sample rate
if (cnt%10==0 || cnt<13)
{

			frame_temp = (IplImage*)img_new->getIplImage();
			
			frame1 = cvCreateImage( cvSize(frame_temp->width,  frame_temp->height), frame_temp->depth, 3 );
		    	
	  	  	cvCvtColor(frame_temp,frame1, CV_RGB2BGR);
	 		frame1->origin=0;
printf("cnt %d",cnt);
}
else
continue;
		}
		else{

			frame1 = cvQueryFrame( original_capture);
			
		}
		
       		if (!frame1){
                        printf("\n Can not Load the image!!!");
                        exit(0);
                }
		cvResize(frame1,frame);
                showImg(frame, "Original_Video");
		char c = cvWaitKey(33);
                if(c==27) break;
                imAdujstEnahancement(frame);
                


//======================================================================================================================================
// Cue Integration Integratioin
//======================================================================================================================================

                CI_obj.Cue[0].computeLiklihood(CI_obj.IBGModel[iBgModelSize], frame);
		bObjA = CI_obj.bObjAppearance(CI_obj.Cue[0].IF_Likelihood);
	        
		
		if (iBgModelSize > (fCount - iInitialFrame - 1)){
			CI_obj.updateBg(CI_obj.IBGModel, frame,	(fCount - iInitialFrame - 1), CI_obj.IF_posterior);
                        //printf("\n primo if %d", (fCount - iInitialFrame - 1));
			
			//showImg(CI_obj.IBGModel[iBgModelSize], "IDC_STATIC2");
			//cvWaitKey();
			continue;
		} 
		else if (true == bObjA && false == bPosterioInit && (iBgModelSize	<= (fCount - iInitialFrame - 1))){
		//	CI_obj.updateBg(CI_obj.IBGModel,frame,(fCount-iInitialFrame-1));
		//	showImage(CI_obj.IBGModel[iBgModelSize],IDC_STATIC2);
                        
			cvCopyImage(CI_obj.Cue[0].IF_Likelihood,CI_obj.IF_posterior);
			cvCopyImage(CI_obj.Cue[0].IB_Likelihood,CI_obj.IB_posterior);
			bPosterioInit = true;
			for (int iCue = 0; iCue < CI_obj.iNoof_Cues; iCue++)
			{
				if (0 == iCue) CI_obj.Cue[0].fCueWeight = CI_obj.iNoof_Cues;
				else CI_obj.Cue[0].fCueWeight = 0.0;
			}
			goto jumpIntegration;
		} 
		else if (true == bPosterioInit){
			printf("\n terzo if");
			for (int iCue = 0; iCue < CI_obj.iNoof_Cues; iCue++) {
				CI_obj.Cue[iCue].computeQuality(CI_obj.IF_posterior);
				CI_obj.Cue[iCue].fCueWeight = 0.0;
			}
			sprintf(msg, "quality 0:%f:: 1:%f:: 2:%f", CI_obj.Cue[0].fQuality,
					CI_obj.Cue[1].fQuality, CI_obj.Cue[2].fQuality);
			//				AfxMessageBox(msg);


			//quality normalization and cue weight modification
			float fSum = 0.0;
			for (int iCue = 0; iCue < CI_obj.iNoof_Cues; iCue++)
				fSum += CI_obj.Cue[iCue].fQuality;

			for (int iCue = 0; iCue < CI_obj.iNoof_Cues; iCue++) {
				CI_obj.Cue[iCue].fQuality /= (fSum / (float) CI_obj.iNoof_Cues);
				CI_obj.Cue[iCue].fCueWeight += (CI_obj.Cue[iCue].fQuality
						- CI_obj.Cue[iCue].fCueWeight);
			}
		} 
		else
			continue;
			//showImg(CI_obj.IF_posterior,"FGpost");
			//cvWaitKey();
	//	tsFun2 = GetTickCount();
		//Color (Cue index:1)

		if (CI_obj.iNoofB_Cues >= 2) {
			CI_obj.Cue[1].computeLiklihoodColor(CI_obj.IF_posterior,
					CI_obj.IB_posterior, CI_obj.FGColorModel,
					CI_obj.BGColorModel, frame, (fCount - iInitialFrame - 1
							- iBgModelSize), CI_obj.ColorSeqF, CI_obj.ColorSeqB);
		}
	//	teFun2 = GetTickCount();

		//texture (Cue index:2)
		if (CI_obj.iNoofB_Cues >= 3) {
			CI_obj.Cue[2].computeLiklihoodTexture(CI_obj.IF_posterior,
					CI_obj.IB_posterior, CI_obj.FGTextureModel,
					CI_obj.BGTextureModel, frame, (fCount - iInitialFrame - 1
							- iBgModelSize));
			showImg(CI_obj.Cue[2].IF_Likelihood, "textureLikelihood");
		}

	//	tsFun3 = GetTickCount();
		//Top-down Object representation (Cue index:3)
		if (CI_obj.iNoofT_Cues >= 1) {
			CI_obj.Cue[2].computeLiklihoodObjectRep(CI_obj.IF_posterior,
					CI_obj.ObjKt, CI_obj.ObjKt_1, CI_obj.Obj_rect_t,
					CI_obj.ObjtsSeq, CI_obj.IgrayDisplay, frame, (fCount
							- iInitialFrame - 1 - iBgModelSize));
		}

		//Cue Integration and FG posterior computation

		CI_obj.integratedPosteriorProbs();
		jumpIntegration: bPosterioInit = CI_obj.bObjAppearance(
				CI_obj.IF_posterior);
	//	teFun3 = GetTickCount();

	//	tFun1 += (teFun1 - tsFun1);
	//	tFun2 += (teFun2 - tsFun2);
	//	tFun3 += (teFun3 - tsFun3);

	//	tsFun2 = GetTickCount();
		//Color (Cue index:1)
		if (CI_obj.iNoofB_Cues >= 2) {
			CI_obj.Cue[1].computeColorRep(CI_obj.IF_posterior,
					CI_obj.IB_posterior, CI_obj.FGColorModel,
					CI_obj.BGColorModel, frame, (fCount - iInitialFrame - 1
							- iBgModelSize), CI_obj.ColorSeqF, CI_obj.ColorSeqB);
		}
	//	teFun2 = GetTickCount();

	//	tsFun3 = GetTickCount();
		if (CI_obj.iNoofT_Cues >= 1) {
			CI_obj.Cue[2].computeObjectRep(CI_obj.IF_posterior, CI_obj.ObjKt,
					CI_obj.ObjKt_1, CI_obj.Obj_rect_t, CI_obj.ObjtsSeq,
					CI_obj.IgrayDisplay, frame, (fCount - iInitialFrame - 1
							- iBgModelSize));
		}
 
	//	teFun3 = GetTickCount();

	//	tsFun1 = GetTickCount();
		// back ground (Cue index \:0)
		if (CI_obj.iNoofB_Cues >= 1 && (iBgModelSize <= (fCount - iInitialFrame
				- 1))) {
			CI_obj.updateBg(CI_obj.IBGModel, frame,
					(fCount - iInitialFrame - 1), CI_obj.IF_posterior);
		}
	//	teFun1 = GetTickCount();

	//	tFun1 += (teFun1 - tsFun1);
	//	tFun2 += (teFun2 - tsFun2);
	//	tFun3 += (teFun3 - tsFun3);
		//	if(iBgModelSize<(fCount-iInitialFrame-1))
		//		CI_obj.updateBg(CI_obj.IBGModel,frame,(fCount-iInitialFrame-1));

		//showImg(CI_obj.IF_posterior,"FGposterior");

		Normalize32F(CI_obj.IF_posterior, CI_obj.IgrayDisplay, 0, 255);

		//	showImg(CI_obj.Cue[0].IF_Likelihood,"motionlikeli");
		//	Normalize32F(CI_obj.Cue[0].IF_Likelihood,CI_obj.IgrayDisplay,0,255);
		//	showImage(CI_obj.IgrayDisplay,IDC_STATIC3);


		Normalize32F(CI_obj.IB_posterior,CI_obj.IgrayDisplay, 0, 255);
		if (CI_obj.iNoofT_Cues >= 1) {
			Normalize32F(CI_obj.IF_posterior, CI_obj.IgrayDisplay, 0, 255);
               		cvShowImage("output",CI_obj.IgrayDisplay); //	showImg(CI_obj.IgrayDisplay,"output");
		//	showImage(CI_obj.IgrayDisplay, IDC_STATIC4);
		}
		//	teFrame = GetTickCount();
		//	sprintf(msg,"frame time:%d and colorLikilehood:%d",(teFrame-tsFrame),(teFun-tsFun));
		//			AfxMessageBox(msg);
	}//for
//	teFrame = GetTickCount();
//	float fFPS = (float) (iFinalFrame - iInitialFrame) / (float) (teFrame - tsFrame) * (float) 1000.0;
	if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
	      perror( "clock gettime" );
	      exit( EXIT_FAILURE );
	}
	accum = ( stop.tv_sec - start.tv_sec ) + (float)( stop.tv_nsec - start.tv_nsec ) / BILLION;
	float fFps = (float) (iFinalFrame - iInitialFrame) / accum;

	int noofObjs = CI_obj.ObjtsSeq->total;

//	sprintf(msg,"fps:%f :: total:%d:: pixDif:%d :: Colorf:%d :: obj:%d :: nObjtx:%d",
	//		fFPS, teFrame - tsFrame, tFun1, tFun2, tFun3, noofObjs);
	printf("\n Noof. Objects:%d :: fps::%f", noofObjs, fFps);

	CI_obj.clearAllocations();
	cvReleaseCapture( &original_capture ); //added
	cvReleaseImage( &frame );		//added
	cvReleaseImage( &frame_temp );
	if(iDataChoice==0){		//added
		img_in_port.interrupt();
		img_in_port.close();
	}
 
	return 0;
} //main
/*
		
		char c = cvWaitKey(33);
                if(c==27) break;

        }//chiuso for

        cvReleaseCapture( &original_capture );
//      cvReleaseCapture( &segmentation_frame );
        cvDestroyWindow( "Original_Video");
        cvDestroyWindow( "Segmentation_Video");
        cvDestroyWindow( "IBG_MODEL");
        cvDestroyWindow( "textureLikelihood");
        cvDestroyWindow( "output");
        return 0;
}//chiuso main

*/


	














//======================================================================================================================================
// Name        : imAdujstEnahncement
// Parameters  : IplImage
// Description : Prende un'immagine se è formata di un canale soltanto la elabora con imAdjust, se è a 3 canali la divide in tre 			 immagini da un canale, le elabora con inAdjust e le ricompone in un'immagine a 3 canali.
//======================================================================================================================================

void imAdujstEnahancement(IplImage* image) {
	
	if (1 == image->nChannels) //get the image nChannels
		imAdjust(image);   //call the function imAdjust
	else if (3 == image->nChannels) {
		IplImage* r_plane = cvCreateImage(cvGetSize(image), 8, 1);//cvCreateImage(CvSize size, int depth, int channels)
		IplImage* g_plane = cvCreateImage(cvGetSize(image), 8, 1);
		IplImage* b_plane = cvCreateImage(cvGetSize(image), 8, 1);

		cvCvtPixToPlane(image, b_plane, g_plane, r_plane, 0); //Divides multi-channel array into several single-channel 									arrays or extracts a single channel from the array.

		imAdjust(b_plane);
		imAdjust(g_plane);
		imAdjust(r_plane);

		cvCvtPlaneToPix(b_plane, g_plane, r_plane, 0, image);

		cvReleaseImage(&r_plane);
		cvReleaseImage(&g_plane);
		cvReleaseImage(&b_plane);
	}
	return;
}

//======================================================================================================================================




//======================================================================================================================================
// Name        : imAdujst
// Parameters  : IplImage
// Description : Prende un'immagine
//======================================================================================================================================
void imAdjust(IplImage *image) {
	#define MAX_GRAY_VALUE 256
	#define OUTLIER_PROPORTION_HIST	0.01
	#define DESIRED_MIN_GRAYLEVEL  0
	#define DESIRED_MAX_GRAYLEVEL  255

	int i, j;
	int inp_img_gray_max, inp_img_gray_min;
	int Range_Min, Range_Max;
	int m_pROIimageHist[MAX_GRAY_VALUE];

	for (i = 0; i < MAX_GRAY_VALUE; i++)
		m_pROIimageHist[i] = 0;	//set to 0 all the cells of array

	////////////////////////////////////////////////////////////////
	// Compute Image Histogram
	// Initialize the histogram
	//memset(m_pROIimageHist,0,MAX_GRAY_VALUE*sizeof(int));
	inp_img_gray_max = -1; // to find minimum gray level in the image
	inp_img_gray_min = 257; // to find maximum gray level in the image
	for (i = 0; i < image->height; i++)
		for (j = 0; j < image->width; j++) {
			int pixelvalue = (unsigned char) ((image->imageData + image->widthStep * i)[j]);//valuta quant'è il valore del pixel
			m_pROIimageHist[pixelvalue]++; //incrementa di uno la cella corrispondente al valore del pixel

		}

	////////////////////////////////////////////////////////////////
	// j outliers : outliers are considered to be pixels proportion
	// (indicated by OUTLIER_PROPORTION_HIST) present on left extreme of gray level histogram
	// The new minimum gray value will be computed
	i = 0;
	double outlier = 0.0;
	double ROI_area = image->width * image->height;
	while ((i < MAX_GRAY_VALUE) && (outlier < OUTLIER_PROPORTION_HIST)) {
		outlier += m_pROIimageHist[i] / ROI_area;
		i++;
	}
	inp_img_gray_min = i - 1;

	////////////////////////////////////////////////////////////////
	// Remove outliers : outliers are considered to be pixels proportion
	// (indicated by OUTLIER_PROPORTION_HIST) present on right extreme of gray level histogram
	// The new maximum gray value will be computed
	i = 255;
	outlier = 0.0;
	while ((i >= 0) && (outlier < OUTLIER_PROPORTION_HIST)) {
		outlier += m_pROIimageHist[i] / ROI_area;
		i--;
	}
	inp_img_gray_max = i + 1;

	///////////////////////////////////////////////////////////////////
	// Contrast normalization on outlier removed image
	Range_Min = DESIRED_MIN_GRAYLEVEL; //Output (contrast normalized) gray value range - min
	Range_Max = DESIRED_MAX_GRAYLEVEL; //Output (contrast normalized) gray value range - max

	// Reassign the image pixel values based on input gray range (inp_img_gray_min : inp_img_gray_max)
	// and desired gray range ( DESIRED_MIN_GRAYLEVEL : DESIRED_MAX_GRAYLEVEL)
	for (i = 0; i < image->height; i++)
		for (j = 0; j < image->width; j++) {
			int pixelval = (unsigned char) ((image->imageData
					+ image->widthStep * i)[j]);

			pixelval = (int) (Range_Min + ((int) (double) (pixelval
					- inp_img_gray_min) / (double) (inp_img_gray_max
					- inp_img_gray_min)) * (Range_Max - Range_Min));

			if (pixelval < 0)
				pixelval = 0;
			if (pixelval > 255)
				pixelval = 255;

			((image->imageData + image->widthStep * i)[j]) = pixelval;

		}
	return;
}

//======================================================================================================================================


//======================================================================================================================================
// Name        : showImg
// Parameters  : IplImage char
// Description : Crea una finestra di nome (cName) che contiene l'immagine (Image)
//======================================================================================================================================
void showImg(IplImage *Image, char *cName){
	cvNamedWindow(cName,CV_WINDOW_AUTOSIZE);
	cvShowImage(cName,Image);
	return;
}


