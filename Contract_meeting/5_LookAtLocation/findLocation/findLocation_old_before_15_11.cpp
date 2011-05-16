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
int cnt=0;bool temp;
  while (1) { // repeat forever
    ImageOf<PixelRgb> *image = imagePort.read();  // read an image: crea un puntatore image del tipo ImageOf<PixelRgb> e gli assegna quello che viene letto sulla porta imagePort.
    if (image!=NULL) { // check we actually got something
       printf("We got an image of size %dx%d\n", image->width(), image->height());
		IplImage *frame_temp = (IplImage*)image->getIplImage();
		
		IplImage *frame1 = cvCreateImage( cvSize(frame_temp->width,  frame_temp->height), frame_temp->depth, 3 );
	    	
  	  	cvCvtColor(frame_temp,frame1, CV_RGB2BGR);
 		frame1->origin=0;

	cvNamedWindow("cName",CV_WINDOW_AUTOSIZE);
	cvShowImage("cName",frame1);
    cvWaitKey(0);
       
       double xMean = 0;
       double yMean = 0;
       int ct = 0;
       for (int x=0; x<frame1->width; x++) {
         for (int y=0; y<frame1->height; y++) {
PixelRgb& pixel;
for (int z=0;z<3;z++){

         pixel->z= (unsigned char) (frame1->imageData + frame1->widthStep * y + frame1->nChannels*x  + z);}
          //PixelRgb& pixel = image->pixel(x,y); //definisco la variabile pixel (di tipo reference) un'istanza della classe PixelRgb.  Allo stesso modo di una variabile puntatore, il tipo reference fa riferimento alla locazione di memoria di un'altra variabile (in questo caso la variabile image che punta al pixel(x,y), ma come una comune variabile, non richiede nessun operatore specifico di deindirizzamento. 
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
       if (ct>(frame1->width/20)*(frame1->height/20)) 
       {
         printf("Best guess at blue target: %d %d \n", cnt, cnt); //%g %g\n",xMean, yMean);
         Vector& target = targetPort.prepare();
         target.resize(3);

         if(cnt>25)
temp =false;

else if(cnt<0)
temp =true;
if (temp ==true)
cnt++;
else if (temp==false)
cnt--; 
       // { cnt =100;}         
  //         target[0] = xMean;
   //      target[1] = yMean;
          target[0] = cnt;
         target[1] = cnt;
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
    }
  }
  
  return 0;
}
