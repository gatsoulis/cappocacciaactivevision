
void setPixel(IplImage *image, int r, int c, unsigned char data, int color)
{
	((unsigned char *)(image->imageData + image->widthStep*r))[c*image->nChannels+color] = data;
}
unsigned char getPixel(IplImage *image, int r, int c, int color)
{
	return ((unsigned char *)(image->imageData + image->widthStep*r))[c*image->nChannels+color];
}

float getPixel32F(IplImage *image, int r, int c, int color)
{
	return ((float *)(image->imageData + image->widthStep*r))[c*image->nChannels+color];
}
void setPixel32F(IplImage *image, int r, int c,  float ch, int color)
{
	((float *)(image->imageData + image->widthStep*r))[c*image->nChannels+color] = ch;
}

short getPixel16S(IplImage *image, int r, int c, int color)
{
	return ((short *)(image->imageData + image->widthStep*r))[c*image->nChannels+color];
}

void setPixel16S(IplImage *image, int r, int c,short ch,  int color)
{
	
	((short *)(image->imageData + image->widthStep*r))[c*image->nChannels+color] = ch;
}

short getPixe32S(IplImage *image, int r, int c, int color)
{
	return ((int *)(image->imageData + image->widthStep*r))[c*image->nChannels+color];
}

void setPixel32S(IplImage *image, int r, int c,short ch,  int color)
{
	
	((int *)(image->imageData + image->widthStep*r))[c*image->nChannels+color] = ch;
}

void plot2d(float **pattern,int index1,int index2,int NoofPattern,char *cName)
{
	float fMax1=-9999,fMin1=9999,fMax2=-9999,fMin2=9999;
	int i;

	for( i=0;i<NoofPattern;i++)
	{
		if(pattern[i][index1]>fMax1) fMax1=pattern[i][index1];
		if(pattern[i][index1]<fMin1) fMin1=pattern[i][index1];
		
		if(pattern[i][index2]>fMax2) fMax2=pattern[i][index2];
		if(pattern[i][index2]<fMin2) fMin2=pattern[i][index2];
	}
	
	int bound1,bound2;
	int lowb1,lowb2;

	/*(fMax1-fMin1)>256?lowb1=0:lowb1= (int)(0.1*(fMax1-fMin1)) ;
	(fMax1-fMin1)>256?lowb2=0:lowb2= (int)(0.1*(fMax2-fMin2)) ;

	
	fMax1-fMin1>256?bound1=256:bound1=(int) ( (fMax1-fMin1)+0.2*(fMax1-fMin1) );
	fMax2-fMin2>256?bound2=256:bound2=(int) ( (fMax2-fMin2)+0.2*(fMax2-fMin2) );
	*/
	lowb1=0; lowb2=0;
	bound1=512; bound2=512;

	IplImage *image=cvCreateImage(cvSize(bound1,bound2), 8,1);	
	cvSetZero(image);
	/*for(i=0;i<bound1;i++)
	for(j=0;j<bound2;j++)
	{
		setPixel(image,i,j,255,0);
		setPixel(image,i,j,255,1);
		setPixel(image,i,j,255,2);
	}
*/

	for(i=0;i<NoofPattern;i++)
	{
		setPixel(image,(int)(lowb1+(pattern[i][index1]-fMin1)/(fMax1-fMin1) * (bound1) ), (int)(lowb2+(pattern[i][index2]-fMin2)/(fMax2-fMin2) * (bound2) ),255,0 );
	//	setPixel(image,(int)(lowb1+(pattern[i][index1]-fMin1)/(fMax1-fMin1) * (bound1) ), (int)(lowb2+(pattern[i][index2]-fMin2)/(fMax2-fMin2) * (bound2) ),0,1 );
	//	setPixel(image,(int)(lowb1+(pattern[i][index1]-fMin1)/(fMax1-fMin1) * (bound1) ), (int)(lowb2+(pattern[i][index2]-fMin2)/(fMax2-fMin2) * (bound2) ),0,0 );
	}
	
//	showImg(image,cName);
	cvReleaseImage(&image);
}

void showMatrixAsImage(float **pattern,int index1,int index2,int SizeInd1,int SizeInd2,char *cName)
{
	float fMax1=-9999,fMin1=9999,fMax2=-9999,fMin2=9999;
	int i,j;

	for( i=0;i<SizeInd1;i++)
	{
		if(pattern[i][index1]>fMax1) fMax1=pattern[i][index1];
		if(pattern[i][index1]<fMin1) fMin1=pattern[i][index1];
	}

	for( i=0;i<SizeInd2;i++)
	{
		if(pattern[i][index2]>fMax2) fMax2=pattern[i][index2];
		if(pattern[i][index2]<fMin2) fMin2=pattern[i][index2];
	}
	
	int bound1,bound2;
	int lowb1,lowb2;

	lowb1=0; lowb2=0;
	bound1=512; bound2=512;

	IplImage *image=cvCreateImage(cvSize(SizeInd2,SizeInd1), IPL_DEPTH_32F,1);	
	IplImage *Grayimage=cvCreateImage(cvSize(SizeInd2,SizeInd1), 8,1);	

	cvSetZero(image);

	for(i=0;i<SizeInd1;i++)
	for(j=0;j<SizeInd2;j++)
	{
		setPixel32F(image,i,j,pattern[i][j],0);
	//	setPixel(image,(int)(lowb1+(pattern[i][index1]-fMin1)/(fMax1-fMin1) * (bound1) ), (int)(lowb2+(pattern[i][index2]-fMin2)/(fMax2-fMin2) * (bound2) ),0,1 );
	//	setPixel(image,(int)(lowb1+(pattern[i][index1]-fMin1)/(fMax1-fMin1) * (bound1) ), (int)(lowb2+(pattern[i][index2]-fMin2)/(fMax2-fMin2) * (bound2) ),0,0 );
	}
	cvConvertScaleAbs( image, Grayimage, 1,0 );
//	showImg(Grayimage,cName);
	cvReleaseImage(&image);
	cvReleaseImage(&Grayimage);



}
void showSubImage(IplImage *image, int r, int c, int height, int width)
{
	if(r+height<=image->height && c+width<=image->width)
	{	
		CvSize size={width,height};
		IplImage *imgTemp=cvCreateImage(size,IPL_DEPTH_8U,3);
		for(int i=0;i<height;i++)
		for(int j=0;j<width;j++)
		{
			//int iPixValue=getPixel(image,r+i,c+j,1);
			setPixel(imgTemp,i,j,getPixel(image,r+i,c+j,1),1);
			setPixel(imgTemp,i,j,getPixel(image,r+i,c+j,2),2);
			setPixel(imgTemp,i,j,getPixel(image,r+i,c+j,3),3);
			
		}
	//	showImg(imgTemp,"subImage");
		cvReleaseImage(&imgTemp);
	}
	
}

void copySubImage(IplImage *imageS,IplImage *imageD, int r, int c, int height, int width)
{
	cvSetImageROI(imageS,cvRect(r,c,height,width));
	cvSetImageROI(imageD,cvRect(r,c,height,width));
	
	cvCopyImage(imageS,imageD);
	
	cvResetImageROI(imageS);
	cvResetImageROI(imageD);
	return;
}

/*
void imAdjust(IplImage *image)
{
	#define MAX_GRAY_VALUE 256
	#define OUTLIER_PROPORTION_HIST	0.03
	#define DESIRED_MIN_GRAYLEVEL  0	
	#define DESIRED_MAX_GRAYLEVEL  255

	int i,j;
	int inp_img_gray_max, inp_img_gray_min;
	int Range_Min,Range_Max;
	int m_pROIimageHist[MAX_GRAY_VALUE];

	for(i=0;i<MAX_GRAY_VALUE; i++)
		m_pROIimageHist[i]=0;
		
	////////////////////////////////////////////////////////////////
	// Compute Image Histogram
	// Initialize the histogram
	//memset(m_pROIimageHist,0,MAX_GRAY_VALUE*sizeof(int));
	inp_img_gray_max = -1;		// to find minimum gray level in the image
	inp_img_gray_min = 257;		// to find maximum gray level in the image
	for(i=0;i<image->height;i++)
	for(j=0;j<image->width;j++)
	{
		int pixelvalue=(unsigned char)((image->imageData + image->widthStep*i)[j]);
		m_pROIimageHist[pixelvalue]++;
		
	}

	////////////////////////////////////////////////////////////////
	// j outliers : outliers are considered to be pixels proportion
	// (indicated by OUTLIER_PROPORTION_HIST) present on left extreme of gray level histogram 
	// The new minimum gray value will be computed
	i = 0;
	double outlier = 0.0;
	double ROI_area = image->width*image->height;
	while((i < MAX_GRAY_VALUE)&&(outlier < OUTLIER_PROPORTION_HIST))
	{
		outlier += m_pROIimageHist[i]/ROI_area;
		i++;			
	}
	inp_img_gray_min = i-1;

	////////////////////////////////////////////////////////////////
	// Remove outliers : outliers are considered to be pixels proportion
	// (indicated by OUTLIER_PROPORTION_HIST) present on right extreme of gray level histogram
	// The new maximum gray value will be computed
	i = 255;
	outlier = 0.0;
	while((i >= 0)&&(outlier < OUTLIER_PROPORTION_HIST))
	{
		outlier += m_pROIimageHist[i]/ROI_area;
		i--;			
	}
	inp_img_gray_max = i+1;


	///////////////////////////////////////////////////////////////////
	// Contrast normalization on outlier removed image
	Range_Min = DESIRED_MIN_GRAYLEVEL;	//Output (contrast normalized) gray value range - min
	Range_Max = DESIRED_MAX_GRAYLEVEL;	//Output (contrast normalized) gray value range - max	

	// Reassign the image pixel values based on input gray range (inp_img_gray_min : inp_img_gray_max)
	// and desired gray range ( DESIRED_MIN_GRAYLEVEL : DESIRED_MAX_GRAYLEVEL)
	for(i=0;i<image->height;i++)
	for(j=0;j<image->width;j++)
	{
		int pixelval=(unsigned char)((image->imageData + image->widthStep*i)[j]);

		pixelval=(int)(Range_Min+((int)(double)(pixelval - inp_img_gray_min)/(double)(inp_img_gray_max - inp_img_gray_min))*(Range_Max-Range_Min));

		if(pixelval < 0)
			pixelval = 0;
		if(pixelval > 255)
			pixelval = 255;
		
		((image->imageData + image->widthStep*i)[j]) = pixelval;
	
	}
	return ;
}

 void imAdujstEnahancement(IplImage *image)
{
	if(1==image->nChannels)
		imAdjust(image);
	else if(3==image->nChannels)
	{
		IplImage* r_plane = cvCreateImage( cvGetSize(image), 8, 1 );
		IplImage* g_plane = cvCreateImage( cvGetSize(image), 8, 1 );
		IplImage* b_plane = cvCreateImage( cvGetSize(image), 8, 1 );
	
		cvCvtPixToPlane( image, b_plane, g_plane, r_plane, 0 );

		imAdjust(b_plane);
		imAdjust(g_plane);
		imAdjust(r_plane);

		cvCvtPlaneToPix( b_plane, g_plane, r_plane, 0,image );
		
		cvReleaseImage(&r_plane);
		cvReleaseImage(&g_plane);
		cvReleaseImage(&b_plane);
	}
}	
*/
