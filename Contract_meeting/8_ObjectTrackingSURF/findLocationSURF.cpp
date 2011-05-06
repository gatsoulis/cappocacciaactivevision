// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/**
 * @ingroup icub_tutorials
 *
 * \defgroup findLocation Find Location
 *
 * This tutorials shows how to do basic
 * image processsing from images coming from 
 * the robot or the simulator
 *
 * \author Tutorial Fitzpatrick
 */ 

#include <stdio.h>
// Get all OS and signal processing YARP classes
#include <iostream>
#include <time.h>
#include <cv.h>
#include <highgui.h>
#include <cxcore.h>
#include <yarp/os/all.h>
#include <yarp/sig/all.h>
#include "highgui.h"
#include "SURF.h"

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;

int main()
 {
      Network yarp; // set up yarp
      BufferedPort<ImageOf<PixelRgb> > imagePort;  
      BufferedPort<Vector> targetPort; 
      imagePort.open("/tutorial/image/in");  
      targetPort.open("/tutorial/target/out");
      //Network::connect("/icubSim/cam/left","/tutorial/image/in");
      Network::connect("/icub/cam/left","/tutorial/image/in");
      
    
      IplImage* object_color;
      while (1) 
      { // repeat forever
          ImageOf<PixelRgb> *image = imagePort.read();
         if (image!=NULL) 
         { 
                printf("We got an image of size %dx%d\n", image->width(), image->height());


               IplImage *frame_temp = (IplImage*)image->getIplImage();
		
        		IplImage *frames = cvCreateImage( cvSize(frame_temp->width,  frame_temp->height), frame_temp->depth, 3 );
	    	
         	  	cvCvtColor(frame_temp,frames, CV_RGB2BGR);
        		frames->origin=0;
 				            	            
            	cvNamedWindow("cNameselect",CV_WINDOW_AUTOSIZE);
            	cvShowImage("cNameselect",frames);
                
                char c = cvWaitKey(33);
                if(c==27) 
                {
                    cvReleaseImage(&frames);
                    cvReleaseImage(&frame_temp);
                    object_color = cvCreateImage( cvSize(frame_temp->width,  frame_temp->height), frame_temp->depth, 3 );
                    cvCopy(frames,object_color);
                    break;
                 }
                else
                {
                    cvReleaseImage(&frame_temp);
                    cvReleaseImage(&frames);
                }
           }
       }
	    cvNamedWindow("cNameoriginal",CV_WINDOW_AUTOSIZE);
	    cvShowImage("cNameoriginal",object_color);


      while (1) 
      { 
            ImageOf<PixelRgb> *image = imagePort.read();
            if (image!=NULL) 
            { 
               printf("We got an image of size %dx%d\n", image->width(), image->height());

                IplImage *frame_temp = cvCreateImage( cvSize(image->width(),  image->height()), IPL_DEPTH_8U, 1 );

                frame_temp = (IplImage*)image->getIplImage();
		
		        IplImage *frame1 = cvCreateImage( cvSize(frame_temp->width,  frame_temp->height), IPL_DEPTH_8U, 1 );
	        	
      	  	    cvCvtColor(frame_temp,frame1, CV_RGB2GRAY);



                    //send values to the lookatlocation
                 Vector& target = targetPort.prepare();
                 target.resize(3);
                 target[0] = xmean;
                 target[1] = ymean;
                 target[2] = 1;
                 targetPort.write();

            }

      }
      return 0;
}
