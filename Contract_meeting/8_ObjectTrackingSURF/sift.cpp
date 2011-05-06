// sift.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <cxcore.h>
#include <cv.h>
#include <highgui.h>


int _tmain(int argc, _TCHAR* argv[])
{
        /*IplImage *image = cvLoadImage("blue.jpg", CV_LOAD_IMAGE_COLOR);

		if(image)
		{
		cvNamedWindow("Image");
        cvShowImage("image",image);
		cvSaveImage("save.jpg",image);
        cvWaitKey();
        cvDestroyWindow("Image");

		int nl= image->height; // number of lines
		int nc= image->width * image->nChannels; // total number of element per line
	      int step= image->widthStep; // effective width

      // get the pointer to the image buffer
      unsigned char *data= reinterpret_cast<unsigned char *>(image->imageData);
	  int ct=0;double xmean=0.0;double ymean=0.0;
      for (int i=1; i<nl; i++) {
            for (int j=0; j<nc; j+= image->nChannels) {
            // process each pixel ---------------------
                  //data[j]= data[j]/div * div + div/2;
                  //data[j+1]= data[j+1]/div * div + div/2;
                  //data[j+2]= data[j+2]/div * div + div/2;
				if((int)data[j]>1.2*(int)data[j+1]&&(int)data[j]>1.2*(int)data[j+2])
				{
					printf("%d %d %d \n",(int)data[j],(int)data[j+1],(int)data[j+2]);
					//getchar();
					ct++;
					xmean += (double)j/3.0;
					ymean += (double)i;
					data[j]=data[j+1]=data[j+2]=0;

				}
				
            // end of pixel processing ----------------
            } // end of line
            data+= step;  // next line
      }
	  
	  if (ct>0) {
         xmean /= (double)ct;
         ymean /= (double)ct; 
       }
    printf("current value of ct %d \n",ct);
       if (ct>(image->width/20)*(image->height/20)) 
       {
         printf("Best guess at blue target: %f %f \n", xmean, ymean); //%g %g\n",xMean, yMean);
	   }
	   cvSaveImage("save.pgm",image);
	   getchar();
        cvReleaseImage(&image);
		}*/
		system("siftWin32.exe <blue1.pgm >tmp1.txt");
		IplImage *image1 = cvLoadImage("blue1.pgm");
		FILE* fp1=fopen("tmp1.txt","rb");
		float *arr1x; float *arr1y; int *arr2;float scale,ori;int temp1, temp2;
		//while(fp1!=NULL)
		{
			fscanf(fp1,"%d %d \n",&temp1, &temp2);
			printf("%d %d \n",temp1,temp2);
			//getchar();
			arr1x=(float*)malloc(sizeof(float)*temp1);
			arr1y=(float*)malloc(sizeof(float)*temp1);
			arr2=(int*)malloc(sizeof(int)*temp2*temp1);
			for(int i2=1;i2<=temp1;i2++)
			{
				printf("temp1 %d \n",i2);
				fscanf(fp1,"%f %f %f %f \n", &arr1x[i2-1], &arr1y[i2-1], &scale,&ori);
				printf("%f %f %f %f \n", arr1x[i2-1], arr1y[i2-1], scale,ori);
				//getchar();
				for(int i3=1;i3<=temp2;i3++)
				{
					fscanf(fp1,"%d ", &arr2[(i2-1)*temp2+(i3-1)]);
					printf("%d ", arr2[(i2-1)*temp2+(i3-1)]);
					
				}fscanf(fp1,"\n");printf("\n");
				cvCircle(image1,cvPoint((int)arr1x[i2-1],(int)arr1y[i2-1]),3,cvScalar(0,255,255,255),1);
				//getchar();
			}

		}
		cvSaveImage("save_first.jpg",image1);
		cvReleaseImage(&image1);
		printf("first one finished \n");
		fclose(fp1);
		system("siftWin32.exe <blue2.pgm >tmp2.txt");
		FILE* fp1new=fopen("tmp2.txt","rb");
		float *arr1xnew; float *arr1ynew; int *arr2new;float scalenew,orinew;int temp1new, temp2new;
		//while(fp1new!=NULL)
		{
			fscanf(fp1new,"%d %d \n",&temp1new, &temp2new);
			printf("%d %d \n",temp1new,temp2new);
			//getchar();
			arr1xnew=(float*)malloc(sizeof(float)*temp1new);
			arr1ynew=(float*)malloc(sizeof(float)*temp1new);
			arr2new=(int*)malloc(sizeof(int)*temp2new*temp1new);
			for(int i2new=1;i2new<=temp1new;i2new++)
			{
				fscanf(fp1new,"%f %f %f %f \n", &arr1xnew[i2new-1], &arr1ynew[i2new-1], &scalenew,&orinew);
				printf("%f %f %f %f \n", arr1xnew[i2new-1], arr1ynew[i2new-1], scalenew,orinew);
				//getchar();
				for(int i3new=1;i3new<=temp2new;i3new++)
				{
					fscanf(fp1new,"%d ", &arr2new[(i2new-1)*temp2new+(i3new-1)]);
					printf("%d ", arr2new[(i2new-1)*temp2new+(i3new-1)]);
					
				}fscanf(fp1new,"\n");printf("\n");
				//getchar();
				
			}

		}
		//matching
		IplImage *imagematch = cvLoadImage("blue2.pgm");
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
			cvCircle(imagematch,cvPoint((int)bestmatchx[i4-1],(int)bestmatchy[i4-1]),3,cvScalar(0,255,255,255),1);
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
		getchar();
		cvSaveImage("savematch.jpg",imagematch);
		cvReleaseImage(&imagematch);
	return 0;
}

