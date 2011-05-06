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
using namespace std;
using namespace yarp::os;
using namespace yarp::sig;

int main() {
  Network yarp; // set up yarp
  BufferedPort<ImageOf<PixelRgb> > imagePort;  // crea una istanza (la porta imagePort) dalla classe BudderedPort che manda/riceve un'immagine rgb in background senza stoppare il processo.
  BufferedPort<Vector> targetPort; //crea una istanza (la porta targetPort) della classe BufferedPort che manda/riceve un vettore

  imagePort.open("/tutorial/image/in");  // give the port a name
  targetPort.open("/tutorial/target/out");
  //Network::connect("/icubSim/cam/left","/tutorial/image/in");
  Network::connect("/icub/cam/left","/tutorial/image/in");//la porta /icub/cam/left è aperta dal modulo camera

  while (1) { // repeat forever
    ImageOf<PixelRgb> *image = imagePort.read();  // read an image: crea un puntatore image del tipo ImageOf<PixelRgb> e gli assegna quello che viene letto sulla porta imagePort.
    if (image!=NULL) { // check we actually got something
       printf("We got an image of size %dx%d\n", image->width(), image->height());

//pramod added nov 15       
    IplImage *frame_temp = (IplImage*)image->getIplImage();
		
		IplImage *frame1 = cvCreateImage( cvSize(frame_temp->width,  frame_temp->height), frame_temp->depth, 3 );
	    	
  	  	cvCvtColor(frame_temp,frame1, CV_RGB2BGR);
 		frame1->origin=0;

	cvNamedWindow("cName",CV_WINDOW_AUTOSIZE);
	cvShowImage("cName",frame1);
 //   cvWaitKey(0);
//pramod added ends

int nl= frame1->height; // number of lines
        int nc= frame1->width * frame1->nChannels; // total number of element per line
          int step= frame1->widthStep; // effective width

      // get the pointer to the image buffer
      unsigned char *data= reinterpret_cast<unsigned char*>(frame1->imageData);
      int ct=0;double xmean=0.0;double ymean=0.0;
      for (int i=1; i<nl; i++) {
            for (int j=0; j<nc; j+= frame1->nChannels) {
            // finding blue pixels ---------------------
//printf("%d %d %d \n",(int)data[j],(int)data[j+1],(int)data[j+2]);

    if((int)data[j]>(int)data[j+1]&&(int)data[j]>(int)data[j+2])
                {
                   // printf("%d %d %d \n",(int)data[j],(int)data[j+1],(int)data[j+2]);
                    //getchar();
                    ct++;
                    xmean += (double)j/3.0;
                    ymean += (double)i;
                   // data[j]=data[j+1]=data[j+2]=0;

data[j]=255;
                }
            // end of pixel processing ----------------
            } // end of line
            data+= step;  // next line
      }
	cvNamedWindow("cName1",CV_WINDOW_AUTOSIZE);
	cvShowImage("cName1",frame1);
    char c = cvWaitKey(33);
		if(c==27) break;
      if (ct>0) {
         xmean /= (double)ct;
         ymean /= (double)ct;
       }
    printf("current value of ct %d \n",ct);
       if (ct>(frame1->width/20)*(frame1->height/20))
       {
         printf("Best guess at blue target: %f %f \n", xmean, ymean);
    cvCircle(frame1,cvPoint((int)xmean,(int)ymean),5,cvScalar(0,0,255,0));
	cvNamedWindow("cName1",CV_WINDOW_AUTOSIZE);
	cvShowImage("cName1",frame1);
   char c = cvWaitKey(33);
		if(c==27) break;
         Vector& target = targetPort.prepare();
         target.resize(3);
         target[0] = xmean;
         target[1] = ymean;
         target[2] = 1;
         targetPort.write();
       } else {
         Vector& target = targetPort.prepare();
         target.resize(3);
         target[0] = 0;
         target[1] = 0;
         target[2] = 0;
         targetPort.write();
       }

/*     double xMean = 0;
       double yMean = 0;
       int ct = 0;
       for (int x=0; x<image->width(); x++) {
         for (int y=0; y<image->height(); y++) {
           PixelRgb& pixel = image->pixel(x,y); //definisco la variabile pixel (di tipo reference) un'istanza della classe PixelRgb.  Allo stesso modo di una variabile puntatore, il tipo reference fa riferimento alla locazione di memoria di un'altra variabile (in questo caso la variabile image che punta al pixel(x,y), ma come una comune variabile, non richiede nessun operatore specifico di deindirizzamento. 
           // very simple test for blueishness
           // make sure blue level exceeds red and green by a factor of 2
           if (pixel.b>pixel.g){ //&& pixel.b>pixel.g){//*1.2+10) {
            // there's a blueish pixel at (x,y)!
            // let's find the average location of these pixels
            xMean += x;
            yMean += y;
            ct++;
           }
         }
       }
       if (ct>0) {
         xMean /= ct;
         yMean /= ct;
       }
    printf("current value of ct %d \n",ct);
       if (ct>(image->width()/20)*(image->height()/20)) {
         printf("Best guess at blue target: %g %g\n", xMean, yMean);
         Vector& target = targetPort.prepare();
         target.resize(3);
         target[0] = xMean;
         target[1] = yMean;
         target[2] = 1;
         targetPort.write();
       } else {
         Vector& target = targetPort.prepare();
         target.resize(3);
         target[0] = 0;
         target[1] = 0;
         target[2] = 0;
         targetPort.write();
       }*/
    }
  }
  
  return 0;
}
