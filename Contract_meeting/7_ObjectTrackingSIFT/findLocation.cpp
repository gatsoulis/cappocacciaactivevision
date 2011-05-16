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

//training part

  while (1) { // repeat forever
    ImageOf<PixelRgb> *image = imagePort.read();  // read an image: crea un puntatore image del tipo ImageOf<PixelRgb> e gli assegna quello che viene letto sulla porta imagePort.
    if (image!=NULL) { // check we actually got something
       printf("We got an image of size %dx%d\n", image->width(), image->height());

//pramod added nov 15       
IplImage *frame_temp = cvCreateImage( cvSize(image->width(),  image->height()), IPL_DEPTH_8U, 1 );

    frame_temp = (IplImage*)image->getIplImage();
		
		IplImage *frame1 = cvCreateImage( cvSize(frame_temp->width,  frame_temp->height), IPL_DEPTH_8U, 1 );
	    	
  	  	cvCvtColor(frame_temp,frame1, CV_RGB2GRAY);
        cvSaveImage("object.pgm",frame1);
 		//frame1->origin=0;
//	cvNamedWindow("cNameoriginal",CV_WINDOW_AUTOSIZE);
//	cvShowImage("cNameoriginal",frame_temp);

	//cvSaveImage("save_first.jpg",image1);
		//cvReleaseImage(&image1);
	cvNamedWindow("cNameselect",CV_WINDOW_AUTOSIZE);
	cvShowImage("cNameselect",frame1);
cvReleaseImage(&frame1);
  char c = cvWaitKey(33);
if(c==27) break;
}
}
printf("breaked");
		system("/home/gabriele/siftWin32.exe <object.pgm >tmp1.txt");
		IplImage *image1 = cvLoadImage("object.pgm");

		FILE* fp1=fopen("tmp1.txt","rb");
		float *arr1x; float *arr1y; int *arr2;float scale,ori;int temp1, temp2;

			fscanf(fp1,"%d %d \n",&temp1, &temp2);
			//printf("%d %d \n",temp1,temp2);
			//getchar();
			arr1x=(float*)malloc(sizeof(float)*temp1);
			arr1y=(float*)malloc(sizeof(float)*temp1);
			arr2=(int*)malloc(sizeof(int)*temp2*temp1);
			for(int i2=1;i2<=temp1;i2++)
			{
				//printf("temp1 %d \n",i2);
				fscanf(fp1,"%f %f %f %f \n", &arr1x[i2-1], &arr1y[i2-1], &scale,&ori);
				//printf("%f %f %f %f \n", arr1x[i2-1], arr1y[i2-1], scale,ori);
				//getchar();
				for(int i3=1;i3<=temp2;i3++)
				{
					fscanf(fp1,"%d ", &arr2[(i2-1)*temp2+(i3-1)]);
					//printf("%d ", arr2[(i2-1)*temp2+(i3-1)]);
					
				}fscanf(fp1,"\n");printf("\n");
				cvCircle(image1,cvPoint((int)arr1x[i2-1],(int)arr1y[i2-1]),3,cvScalar(0,255,255,255),1);
				//getchar();
			}


		cvNamedWindow("cNameobject",CV_WINDOW_AUTOSIZE);
	cvShowImage("cNameobject",image1);
		printf("first one finished \n");
		fclose(fp1);

//training part ends


  while (1) { // repeat forever
    ImageOf<PixelRgb> *image = imagePort.read();  // read an image: crea un puntatore image del tipo ImageOf<PixelRgb> e gli assegna quello che viene letto sulla porta imagePort.
    if (image!=NULL) { // check we actually got something
       printf("We got an image of size %dx%d\n", image->width(), image->height());

//pramod added nov 15       
IplImage *frame_temp = cvCreateImage( cvSize(image->width(),  image->height()), IPL_DEPTH_8U, 1 );

    frame_temp = (IplImage*)image->getIplImage();
		
		IplImage *frame1 = cvCreateImage( cvSize(frame_temp->width,  frame_temp->height), IPL_DEPTH_8U, 1 );
	    	
  	  	cvCvtColor(frame_temp,frame1, CV_RGB2GRAY);
        cvSaveImage("object_test.pgm",frame1);
 		//frame1->origin=0;
	cvNamedWindow("cNameoriginal",CV_WINDOW_AUTOSIZE);
	cvShowImage("cNameoriginal",frame_temp);

//    cvWaitKey(0);
//pramod added ends


		system("/home/gabriele/siftWin32.exe <object_test.pgm >tmp2.txt");
		IplImage *image2 = cvLoadImage("object_test.pgm");
		FILE* fp1new=fopen("tmp2.txt","rb");
		float *arr1xnew; float *arr1ynew; int *arr2new;float scalenew,orinew;int temp1new, temp2new;
		//while(fp1new!=NULL)
		{
			fscanf(fp1new,"%d %d \n",&temp1new, &temp2new);
			//printf("%d %d \n",temp1new,temp2new);
			//getchar();
			arr1xnew=(float*)malloc(sizeof(float)*temp1new);
			arr1ynew=(float*)malloc(sizeof(float)*temp1new);
			arr2new=(int*)malloc(sizeof(int)*temp2new*temp1new);
			for(int i2new=1;i2new<=temp1new;i2new++)
			{
				fscanf(fp1new,"%f %f %f %f \n", &arr1xnew[i2new-1], &arr1ynew[i2new-1], &scalenew,&orinew);
				//printf("%f %f %f %f \n", arr1xnew[i2new-1], arr1ynew[i2new-1], scalenew,orinew);
				//getchar();
				for(int i3new=1;i3new<=temp2new;i3new++)
				{
					fscanf(fp1new,"%d ", &arr2new[(i2new-1)*temp2new+(i3new-1)]);
					//printf("%d ", arr2new[(i2new-1)*temp2new+(i3new-1)]);
					
				}fscanf(fp1new,"\n");printf("\n");
				cvCircle(image2,cvPoint((int)arr1xnew[i2new-1],(int)arr1ynew[i2new-1]),3,cvScalar(0,255,255,255),1);
				//getchar();
				
			}

		}
	cvNamedWindow("cNametest",CV_WINDOW_AUTOSIZE);
	cvShowImage("cNametest",image2);
		//matching
		//IplImage *imagematch = cvLoadImage("blue2.pgm");
		float *bestmatchx = (float*)malloc(sizeof(float)*temp1);
		float *bestmatchy = (float*)malloc(sizeof(float)*temp1);
		for (int i4=1;i4<=temp1;i4++)
		{
			float bestval = 1000000.0;
			for(int i5=1;i5<=temp1new;i5++)
			{
				double value=0.0;
				for(int i6=1;i6<=temp2new;i6++)
				{
					double temp = arr2[(i4-1)*temp2+(i6-1)]-arr2new[(i5-1)*temp2new+(i6-1)];
					value+= temp*temp;
				}
				value = sqrt(value)/temp2new;
				if (value < bestval)
				{
					bestval=value;
					bestmatchx[i4-1]=arr1xnew[i5-1];
					bestmatchy[i4-1]=arr1ynew[i5-1];
				}
			}
			cvCircle(frame1,cvPoint((int)bestmatchx[i4-1],(int)bestmatchy[i4-1]),3,cvScalar(0,255,255,255),1);
		}
		double xmean = 0.0; double ymean = 0.0;
		for(int i7=1;i7<=temp1;i7++)
		{
			xmean += bestmatchx[i7-1];
			ymean += bestmatchy[i7-1];
		}
		xmean = xmean/temp1;
		ymean = ymean/temp1;
		printf("%lf %lf",xmean,ymean);
		//getchar();
		//cvSaveImage("savematch.jpg",imagematch);
		//cvReleaseImage(&imagematch);

    cvCircle(frame1,cvPoint((int)xmean,(int)ymean),10,cvScalar(0,255,255,0));
	cvNamedWindow("cNamematch",CV_WINDOW_AUTOSIZE);
	cvShowImage("cNamematch",frame1);
   char c = cvWaitKey(33);

//int xmean=0;int ymean=0;
         Vector& target = targetPort.prepare();
         target.resize(3);
         target[0] = xmean;
         target[1] = ymean;
         target[2] = 1;
         targetPort.write();

cvReleaseImage(&image2);
    }

  }
  cvReleaseImage(&image1);
  return 0;
}
