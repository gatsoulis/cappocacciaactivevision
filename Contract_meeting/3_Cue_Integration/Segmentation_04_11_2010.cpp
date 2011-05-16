#include "Segmentation.h"
#include "ImgBasics.h"

#define EPS 0.000000001
#define gMatSize 5
#define gaussSigma ((float)gMatSize/4.0)

#define colorHistlength 64
#define colorHistfactor ((float)colorHistlength/256)
#define textureHistlength 64
#define textureHistfactor ((float)textureHistlength/128)


void Cues::computeLiklihoodObjectRep(IplImage *IF_posterior,objFeature *ObjKt[objRows][objCols], objFeature *ObjKt_1[objRows][objCols],int Obj_rect_t[],CvSeq *ObjtsSeq, IplImage *IGray, IplImage *image,int index)
{
	int numObj=0;
	float fSearchScore;
	CvScalar a,rgb;
	float fGridAvg;

	cvSet(IF_Likelihood,cvScalar(0.2)); // outside the object region it is 0.5 for both object and bg
	//	cvSetZero(IF_Likelihood);
	cvSetZero(IB_Likelihood);

	Object obj;

//	IplImage *bestMatch=cvCloneImage(image);

	for(numObj=0;numObj<ObjtsSeq->total;numObj++)
	{
		obj=*CV_GET_SEQ_ELEM( Object, ObjtsSeq, numObj );
	
		if(obj.iInitializationCount>=4)
		{

      		obj.rectt=fobjectMatching(image,IF_posterior, ObjKt, ObjKt_1, obj.rectt, obj.rectt_1);
	
			// likelihood computation from the top-down (obj representation) Cue
			for(int j=obj.rectt.x,n=obj.rectt_1.x;j<obj.rectt.x+obj.rectt.width;j+=iGridSize,n+=iGridSize)
			for(int i=obj.rectt.y,m=obj.rectt_1.y;i<obj.rectt.y+obj.rectt.height;i+=iGridSize,m+=iGridSize)
			{
			//	iSeqPos=(m/iGridSize)*(image->width/iGridSize)+(n/iGridSize);
				int p=m/iGridSize, q=n/iGridSize;
				fSearchScore=0.0; 

				if(NULL!=ObjKt_1[p][q])
				{			
					cvSetImageROI(image,cvRect(j,i,iGridSize,iGridSize)); 
					rgb=cvAvg(image);
					cvResetImageROI(image);

					fSearchScore=( 1.0-((float)1.0/255.0)*sqrt(((float)1.0/3.0) *
										((ObjKt_1[p][q]->iColor[0]- rgb.val[0]) * (ObjKt_1[p][q]->iColor[0]- rgb.val[0])
										+(ObjKt_1[p][q]->iColor[1]- rgb.val[1]) * (ObjKt_1[p][q]->iColor[1]- rgb.val[1])
										+(ObjKt_1[p][q]->iColor[2]- rgb.val[2]) * (ObjKt_1[p][q]->iColor[2]- rgb.val[2]) ) ) );
				
					cvSetImageROI(IF_posterior,cvRect(j,i,iGridSize,iGridSize));
					a=cvAvg(IF_posterior);
					cvResetImageROI(IF_posterior);
					fGridAvg=a.val[0];
					
				//	cvSetImageROI(IF_Likelihood,cvRect(j,i,iGridSize,iGridSize));
				//	cvSet( IF_Likelihood,cvScalar(fGridAvg*fSearchScore) );
				//	cvResetImageROI(IF_Likelihood);

					
					if(fGridAvg*fSearchScore>=0.5)
					{
						cvSetImageROI(IF_Likelihood,cvRect(j,i,iGridSize,iGridSize));
						cvSet( IF_Likelihood,cvScalar(fGridAvg*fSearchScore) );
						cvResetImageROI(IF_Likelihood);
					}
					else
					{
						cvSetImageROI(IF_Likelihood,cvRect(j,i,iGridSize,iGridSize));
						cvSet( IF_Likelihood,cvScalar(0.5) );
						cvResetImageROI(IF_Likelihood);
					}
					
				//	cvSetImageROI(bestMatch,cvRect(j+2,i+2,3,3));
				///	cvSet( bestMatch,cvScalar(0,255,0) );
				//	cvResetImageROI(bestMatch);
					
				}
				
			}
		}
	}
 
	for(int	i=0;i<image->height;i++)
	for(int j=0;j<image->width;j++)	
	{
		setPixel32F(IB_Likelihood,i,j,( 1.0-getPixel32F(IF_Likelihood,i,j,0)) ,0 );
	}

//	showImg(bestMatch,"likelihood");
//	cvReleaseImage(&bestMatch);
/*
	{
		cvCopyImage(IF_posterior,IF_Likelihood);
		for(int	i=0;i<image->height;i++)
		for(int j=0;j<image->width;j++)	
		{
			setPixel32F(IB_Likelihood,i,j,( 1.0-getPixel32F(IF_Likelihood,i,j,0)) ,0 );
		}
	}
*/
//	showImg(IF_Likelihood,"IF_obj");
//	showImg(IB_Likelihood,"IB_obj");
	
}

void Cues::computeObjectRep(IplImage *IF_posterior,objFeature *ObjKt[objRows][objCols], objFeature *ObjKt_1[objRows][objCols],int Obj_rect_t[],CvSeq *ObjtsSeq, IplImage *IGray, IplImage *image,int index)
{
	CvScalar a,rgb;	
	float fGridAvg;

	objFeature *point;

	cvCvtColor(image,IGray,CV_BGR2GRAY);
	IplImage *IBinary=cvCreateImage(cvGetSize(image),IPL_DEPTH_8U, 1);
	
	cvSetZero(IBinary);

	for(int	i=0;i<image->height;i+=iGridSize)
	for(int j=0;j<image->width; j+=iGridSize)
	{
		cvSetImageROI(IF_posterior,cvRect(j,i,iGridSize,iGridSize));
		a=cvAvg(IF_posterior);
		cvResetImageROI(IF_posterior);
		fGridAvg=a.val[0];
		
		if(fGridAvg>=0.5)
		{
			cvSetImageROI(IBinary,cvRect(j,i,iGridSize,iGridSize));
			cvSet(IBinary,cvScalar(255));
			cvResetImageROI(IBinary);
		
			cvSetImageROI(image,cvRect(j,i,iGridSize,iGridSize)); 
			rgb=cvAvg(image);
			cvResetImageROI(image);
				
			point=(objFeature *)malloc(sizeof(objFeature ));


			point->fPrior=fGridAvg;
			point->iColor[0]=(int)rgb.val[0];
			point->iColor[1]=(int)rgb.val[1];
			point->iColor[2]=(int)rgb.val[2];

/*			ObjFeat.fTexture[0]=0.0; ObjFeat.fTexture[1]=0.0; ObjFeat.fTexture[2]=0.0;
			//textrue feature is to be computed and consider further////////*********
*/			
		//	if(NULL!=ObjKt[i/iGridSize][j/iGridSize])
		//		free(ObjKt[i/iGridSize][j/iGridSize]);
		//	ObjKt[i/iGridSize][j/iGridSize]=point;
		}
	}
//	showImg(IF_posterior, "post");
//	cvDilate( IBinary, IBinary,NULL, 2 );
//	cvErode ( IBinary, IBinary,NULL, 2 );
	cvSmooth(IBinary, IBinary,CV_MEDIAN,3,3);
	cvThreshold(IBinary, IBinary,100,255, CV_THRESH_BINARY);
	
	// to find all connected components in the image 
	int NoConnComp = 0;
	int Connectivity = 8;
	CvConnectedComp CComp;

//	 showImg(IBinary, "postBinary");
	for(int	l=0; l<IBinary->width;l++)
	for(int m=0; m<IBinary->height;m++)
	{
		CvPoint pt = {l,m};
		uchar* temp_ptr = &((uchar*)(IBinary->imageData + IBinary->widthStep*pt.y))[pt.x];
		if(temp_ptr[0]==255)
		{ 
			cvFloodFill(IBinary,pt, cvScalar((NoConnComp+1)*2), cvScalarAll(0), cvScalarAll(0),&(CComp), Connectivity, NULL );
			if( CComp.area>100 ) // considering at least 30x30 pixel size object
			{
				Object obj;
				float d=0.0;
				bool bFindObj=false;
				for(int nObj=0;nObj<ObjtsSeq->total;nObj++)
				{
					obj=*CV_GET_SEQ_ELEM( Object, ObjtsSeq, nObj );	
					
					float blobCenx, blobCeny, objCenx, objCeny;

					blobCenx=(float)(CComp.rect.x+CComp.rect.width)/2.0; blobCeny=(float)(CComp.rect.y+CComp.rect.height)/2.0;
					objCenx=(float)(obj.rectt_1.x+obj.rectt_1.width)/2.0; objCeny=(float)(obj.rectt_1.y+obj.rectt_1.height)/2.0;

					float d=sqrt( (blobCenx-objCenx) * (blobCenx-objCenx) + (blobCeny-objCeny) * (blobCeny-objCeny) );
					if( d<0.3*(CComp.rect.height>CComp.rect.width?CComp.rect.height:CComp.rect.width) || d<20 )
					{
						int minX,maxX,minY,maxY;
						float fEtaA=(float)1.0, fEtaB=(float)TD_MOD_UPDATE_FACTOR, fLambda=(float)0.6;
						float fNodeConf=0.5;
						
						obj.iInitializationCount++;
						obj.iUpdate++;
						/* deletion of object out of boundary or disappear*/

						if(obj.iInitializationCount>4) {	fEtaA=(float)0.95; fEtaB=(float)0.65; } //with leaky modification
						
						CComp.rect.x+=((float)iGridSize/2.0); CComp.rect.x = ((int) (CComp.rect.x/iGridSize) ) * iGridSize;
						CComp.rect.y+=((float)iGridSize/2.0); CComp.rect.y = ((int) (CComp.rect.y/iGridSize) ) * iGridSize;

						minX=obj.rectt_1.x<=CComp.rect.x?obj.rectt_1.x:CComp.rect.x;
						minY=obj.rectt_1.y<=CComp.rect.y?obj.rectt_1.y:CComp.rect.y;

						maxX=obj.rectt_1.x+obj.rectt_1.width>=CComp.rect.x+CComp.rect.width?obj.rectt_1.x+obj.rectt_1.width:CComp.rect.x+CComp.rect.width;
						maxY=obj.rectt_1.y+obj.rectt_1.height>=CComp.rect.y+CComp.rect.height?obj.rectt_1.y+obj.rectt_1.height:CComp.rect.y+CComp.rect.height;
						

						obj.rectt.x=(maxX+minX)/2;
						obj.rectt.y=(maxY+minY)/2;
						obj.rectt.width=0;
						obj.rectt.height=0;
					
						for(int j=minX,n=obj.rectt_1.x;j<maxX && n<image->width/*&& n<obj.rectt_1.x+obj.rectt_1.width*/;j+=iGridSize,n+=iGridSize)
						for(int i=minY,m=obj.rectt_1.y;i<maxY && m<image->height/*&& m<obj.rectt_1.y+obj.rectt_1.height*/;i+=iGridSize,m+=iGridSize)							
						{
							int p=i/iGridSize, q=j/iGridSize;
							int r=m/iGridSize, s=n/iGridSize;
						
							cvSetImageROI(IF_posterior,cvRect(j,i,iGridSize,iGridSize));
							a=cvAvg(IF_posterior);
							cvResetImageROI(IF_posterior);
							fGridAvg=a.val[0];
							
							if(fGridAvg>=fNodeConf) //checking for the valid nodes
							{
								cvSetImageROI(image,cvRect(j,i,iGridSize,iGridSize)); 
								rgb=cvAvg(image);
								cvResetImageROI(image);
									
								point=(objFeature *)malloc(sizeof(objFeature ));

								point->fPrior=fGridAvg;
								point->iColor[0]=(int)rgb.val[0];
								point->iColor[1]=(int)rgb.val[1];
								point->iColor[2]=(int)rgb.val[2];
								
								if(NULL!= ObjKt_1[r][s])
								{	
									if( fEtaA* fGridAvg >= fNodeConf && ObjKt_1[r][s]->fPrior < fNodeConf)
									{
										point->fPrior=fEtaA* fGridAvg;

										{ free(ObjKt_1[r][s]);  ObjKt_1[r][s]=NULL; }
										if(NULL != ObjKt[p][q]) { free(ObjKt[p][q]); }
										ObjKt[p][q]=point;
									}	
									else if( fGridAvg < fNodeConf && fEtaB* ObjKt_1[r][s]->fPrior >= fNodeConf)
									{
										point->fPrior=fEtaB* ObjKt_1[r][s]->fPrior;
										point->iColor[0] = ObjKt_1[r][s]->iColor[0];
										point->iColor[1] = ObjKt_1[r][s]->iColor[1];
										point->iColor[2] = ObjKt_1[r][s]->iColor[2];

										{ free(ObjKt_1[r][s]); ObjKt_1[r][s]=NULL; }
										if(NULL != ObjKt[p][q]) { free(ObjKt[p][q]); }
										ObjKt[p][q]=point;
									}
									else if( fGridAvg >= fNodeConf && ObjKt_1[r][s]->fPrior >= fNodeConf)
									{
										point->fPrior=fLambda* point->fPrior + (1.0-fLambda)*ObjKt_1[r][s]->fPrior;
										point->iColor[0] =fLambda* point->iColor[0] + (1.0-fLambda)*ObjKt_1[r][s]->iColor[0];
										point->iColor[1] =fLambda* point->iColor[1] + (1.0-fLambda)*ObjKt_1[r][s]->iColor[1];
										point->iColor[2] =fLambda* point->iColor[2] + (1.0-fLambda)*ObjKt_1[r][s]->iColor[2];

										free(ObjKt_1[r][s]); ObjKt_1[r][s]=NULL;
										if(NULL != ObjKt[p][q]) { free(ObjKt[p][q]); }
										ObjKt[p][q]=point;
									}
								}
								else if( fEtaA* fGridAvg >= fNodeConf )
								{
									point->fPrior=fEtaA*point->fPrior;

									if(NULL != ObjKt[p][q]) { free(ObjKt[p][q]); }
									ObjKt[p][q]=point;
								}

							} // end of valid node condition in posterior image
							else if(NULL != ObjKt_1[r][s])  // contribution from the past model
							{
								if( fEtaB* ObjKt_1[r][s]->fPrior >= fNodeConf)
								{
									point=(objFeature *)malloc(sizeof(objFeature ));
									
									point->fPrior=fEtaB * ObjKt_1[r][s]->fPrior;
									point->iColor[0]= ObjKt_1[r][s]->iColor[0];
									point->iColor[1]= ObjKt_1[r][s]->iColor[1];
									point->iColor[2]= ObjKt_1[r][s]->iColor[2];

									free(ObjKt_1[r][s]); ObjKt_1[r][s]=NULL;
									if(NULL != ObjKt[p][q]) { free(ObjKt[p][q]); }
									ObjKt[p][q]=point;
								}	
							}
	
						} // end of for loop for Kt obj model modification

						//if( ||  obj.rectt.x > image.width-10 || obj.rectt.x < 10 || obj.rectt.y > image.height-10 || obj.rectt.y < 10  )
						//{

						//}
						for(int j=minX;j<maxX;j+=iGridSize)
						for(int i=minY;i<maxY;i+=iGridSize)							
						{
							int p=i/iGridSize, q=j/iGridSize;

							if(NULL != ObjKt[p][q]) 
							{
								if(obj.rectt.x>=j) obj.rectt.x=j;
								if(obj.rectt.y>=i) obj.rectt.y=i;								
							}

						}

						for(int j=minX;j<maxX;j+=iGridSize)
						for(int i=minY;i<maxY;i+=iGridSize)							
						{
							int p=i/iGridSize, q=j/iGridSize;

							if(NULL != ObjKt[p][q]) 
							{
								if(obj.rectt.width<=j-obj.rectt.x) obj.rectt.width = j-obj.rectt.x;
								if(obj.rectt.height<=i-obj.rectt.y) obj.rectt.height = i-obj.rectt.y;
							}
						}

						///*************** rect size prob
						obj.rectt.width += (obj.rectt.width<image->width-iGridSize?iGridSize:0);
						obj.rectt.height +=(obj.rectt.height<image->height-iGridSize?iGridSize:0);
						
 						obj.rectt_1.x =obj.rectt.x;
						obj.rectt_1.y =obj.rectt.y;
						obj.rectt_1.width =obj.rectt.width;
						obj.rectt_1.height =obj.rectt.height;

						bFindObj=true;

						cvSeqRemove( ObjtsSeq, nObj );
						cvSeqInsert(ObjtsSeq,nObj,&obj);
					//	cvSeqPush(ObjtsSeq\,&obj);
						
						break;
					}
				}

				if(false==bFindObj)
				{
					Object obj;  //obj.Object();
   					obj.rectt.x=CComp.rect.x; obj.rectt.y=CComp.rect.y; obj.rectt.width=CComp.rect.width;obj.rectt.height=CComp.rect.height;
					obj.rectt_1.x=CComp.rect.x; obj.rectt_1.y=CComp.rect.y; obj.rectt_1.width=CComp.rect.width;obj.rectt_1.height=CComp.rect.height;

					obj.iInitializationCount=1;
					obj.iUpdate=0;
					
					for(int	i=CComp.rect.x; i<CComp.rect.x+CComp.rect.width;i+=iGridSize)
					for(int j=CComp.rect.y; j<CComp.rect.y+CComp.rect.height;j+=iGridSize)
					{
						cvSetImageROI(IF_posterior,cvRect(i, j, iGridSize, iGridSize));
						a=cvAvg(IF_posterior);
						cvResetImageROI(IF_posterior);
						fGridAvg=a.val[0];
						
						if(fGridAvg>=0.5)
						{
							cvSetImageROI(image,cvRect(i,j,iGridSize,iGridSize)); 
							rgb=cvAvg(image);
							cvResetImageROI(image);
								
							point=(objFeature *)malloc(sizeof(objFeature ));

							point->fPrior=fGridAvg;
							point->iColor[0]=(int)rgb.val[0];
							point->iColor[1]=(int)rgb.val[1];
							point->iColor[2]=(int)rgb.val[2];

				/*			ObjFeat.fTexture[0]=0.0; ObjFeat.fTexture[1]=0.0; ObjFeat.fTexture[2]=0.0;
							//textrue feature is to be computed and consider further////////*********
				*/			
							if(NULL!=ObjKt[j/iGridSize][i/iGridSize])	free(ObjKt[j/iGridSize][i/iGridSize]);
							ObjKt[j/iGridSize][i/iGridSize]=point;
						}
					}
					cvSeqPush(ObjtsSeq,&obj);
				}
				NoConnComp++;
			}
		
		}
		
	}
	
/*	IplImage *bestMatch=cvCloneImage(image);
	for(  	i=0;i<image->height;i+=iGridSize)
	for(int j=0;j<image->width;j+=iGridSize)	
	{
		int p=i/iGridSize;
		int q=j/iGridSize;

		if(NULL!=ObjKt[p][q])
		{
			cvSetImageROI(bestMatch,cvRect(j,i,2,2));
			cvSet( bestMatch,cvScalar(0,255,0) );
			cvResetImageROI(bestMatch);
			showImg(bestMatch,"objModel");
		}
	}
	showImg(bestMatch,"objModel");
	cvReleaseImage(&bestMatch);
*/
	for(int i=0;i<image->height;i+=iGridSize)
	for(int j=0;j<image->width; j+=iGridSize)
	{
		int p=i/iGridSize,q=j/iGridSize;
		if(NULL!=ObjKt_1[p][q])
		{	free(ObjKt_1[p][q]); ObjKt_1[p][q]=NULL;}

		if(NULL!=ObjKt[p][q])
		{
			point=(objFeature *)malloc(sizeof(objFeature ));
				
			point->fPrior=ObjKt[p][q]->fPrior;
			point->iColor[0]=ObjKt[p][q]->iColor[0];
			point->iColor[1]=ObjKt[p][q]->iColor[1];
			point->iColor[2]=ObjKt[p][q]->iColor[2];

			ObjKt_1[p][q]=point;		

			free(ObjKt[p][q]);  ObjKt[p][q]=NULL;
		}
	}
	/*there is a trouble if image->height and image->width are not divisible by iGridSize  */

//	Object objt;
//	for(int nObj=0;nObj<ObjtsSeq->total;nObj++)
//	{
//		objt=*CV_GET_SEQ_ELEM( Object, ObjtsSeq, nObj );
//	}
					
/*	for(int numObj=0;numObj<ObjtsSeq->total;numObj++)
	{
		obj=*CV_GET_SEQ_ELEM( Object, ObjtsSeq, numObj );
	
		
	}
*/
/*---old code
//	showImg(IBinary,"IB");
	CvRect rect=cvBoundingRect( IBinary,0);

	Obj_rect_t[0]=rect.x;
	Obj_rect_t[1]=rect.y;
	Obj_rect_t[2]=rect.width;
	Obj_rect_t[3]=rect.height;

	int minX=0,minY=0,maxX=image->width, maxY=image->height;

	if(0==index) goto end; //for first initialization of the object model

//	IplImage* kt=cvCloneImage(image);
//	cvRectangle( kt, cvPoint(Obj_rect_t[0], Obj_rect_t[1] ), cvPoint(Obj_rect_t[0]+Obj_rect_t[2],Obj_rect_t[1]+Obj_rect_t[3] ), cvScalar(255,0,255),2, 8,0 );

	if(index>0)
	{		
		 // Modification of Generalized Kt
		float fEtaA=(float)1.0, fEtaB=(float)1.0, fLambda=(float)0.6;
		float fNodeConf=0.5;
	
		if(index>4) {	fEtaA=(float)0.95; fEtaB=(float)0.65; } //with leaky modification

		minX=Obj_rect_t[0]<=Obj_rect_t_1[0]?Obj_rect_t[0]:Obj_rect_t_1[0];
		minY=Obj_rect_t[1]<=Obj_rect_t_1[1]?Obj_rect_t[1]:Obj_rect_t_1[1];

		maxX=Obj_rect_t[0]+Obj_rect_t[2]<=Obj_rect_t_1[0]+Obj_rect_t_1[2]?Obj_rect_t_1[0]+Obj_rect_t_1[2]:Obj_rect_t[0]+Obj_rect_t[2];
		maxY=Obj_rect_t[1]+Obj_rect_t[3]<=Obj_rect_t_1[1]+Obj_rect_t_1[3]?Obj_rect_t_1[1]+Obj_rect_t_1[3]:Obj_rect_t[1]+Obj_rect_t[3];
		
	
		for(int j=Obj_rect_t[0],n=Obj_rect_t_1[0];j<Obj_rect_t[0]+Obj_rect_t[2] && n<Obj_rect_t_1[0]+Obj_rect_t_1[2];j+=iGridSize,n+=iGridSize)
		for(int i=Obj_rect_t[1],m=Obj_rect_t_1[1];i<Obj_rect_t[1]+Obj_rect_t[3] && m<Obj_rect_t_1[1]+Obj_rect_t_1[3];i+=iGridSize,m+=iGridSize)
		{
 			int p=i/iGridSize, q=j/iGridSize;
			int r=m/iGridSize, s=n/iGridSize;
		
			if(NULL!=ObjKt[p][q])
			{
				if(NULL!=ObjKt_1[r][s])
				{	
					if( ObjKt[p][q]->fPrior >= fNodeConf && ObjKt_1[r][s]->fPrior < fNodeConf)
					{
						if(fEtaA* ObjKt[p][q]->fPrior>=fNodeConf)
						{	
							ObjKt[p][q]->fPrior=fEtaA* ObjKt[p][q]->fPrior;
						}
						else { free(ObjKt[p][q]); ObjKt[p][q]=NULL; }
					}
					else if( ObjKt[p][q]->fPrior < fNodeConf && ObjKt_1[r][s]->fPrior >= fNodeConf)
					{
						if(fEtaB* ObjKt_1[r][s]->fPrior>=fNodeConf)
						{
							ObjKt[p][q]->fPrior=fEtaB* ObjKt_1[r][s]->fPrior;
							ObjKt[p][q]->iColor[0]=ObjKt_1[r][s]->iColor[0];
							ObjKt[p][q]->iColor[1]=ObjKt_1[r][s]->iColor[1];
							ObjKt[p][q]->iColor[2]=ObjKt_1[r][s]->iColor[2];
						}
						else { free(ObjKt[p][q]); ObjKt[p][q]=NULL;}
					}
					else if( ObjKt[p][q]->fPrior >= fNodeConf && ObjKt_1[r][s]->fPrior >= fNodeConf)
					{
						if( fLambda* ObjKt[p][q]->fPrior + (1.0-fLambda)*ObjKt_1[r][s]->fPrior >= fNodeConf)
						{
							ObjKt[p][q]->fPrior=fLambda* ObjKt[p][q]->fPrior + (1.0-fLambda)*ObjKt_1[r][s]->fPrior;
							ObjKt[p][q]->iColor[0]=fLambda* ObjKt[p][q]->iColor[0] + (1.0-fLambda)*ObjKt_1[r][s]->iColor[0];
							ObjKt[p][q]->iColor[1]=fLambda* ObjKt[p][q]->iColor[1] + (1.0-fLambda)*ObjKt_1[r][s]->iColor[1];
							ObjKt[p][q]->iColor[2]=fLambda* ObjKt[p][q]->iColor[2] + (1.0-fLambda)*ObjKt_1[r][s]->iColor[2];
						}
						else { free(ObjKt[p][q]); ObjKt[p][q]=NULL; }
					}
					
				}
				else 
				{	
					if(fEtaA*ObjKt[p][q]->fPrior>= fNodeConf)
						ObjKt[p][q]->fPrior=fEtaA* ObjKt[p][q]->fPrior;
					else { free(ObjKt[p][q]); ObjKt[p][q]=NULL; }
				}
				
			}
			else if(NULL!=ObjKt_1[r][s])
			{
				if( fEtaB* ObjKt_1[r][s]->fPrior >= fNodeConf)
				{
					point=(objFeature *)malloc(sizeof(objFeature ));
					
					point->fPrior=fEtaB* ObjKt_1[r][s]->fPrior;
					point->iColor[0]=ObjKt_1[r][s]->iColor[0];
					point->iColor[1]=ObjKt_1[r][s]->iColor[1];
					point->iColor[2]=ObjKt_1[r][s]->iColor[2];

					ObjKt[p][q]=point;
					free(ObjKt_1[r][s]);
					ObjKt_1[r][s]=NULL;
				}	
			}
		}

	}

	end:; // lable for object represetation modification
	//to store the current frame as previsous frame for further processing
	minX=999,minY=999, maxX=0,maxY=0;
	if(index>0)
	{
	
		for(    i=0;i<image->height;i+=iGridSize)
		for(int j=0;j<image->width; j+=iGridSize)
		{
			int p=i/iGridSize, q=j/iGridSize;
			if(NULL!=ObjKt[p][q])
			{
				minX=minX>j?j:minX;
				minY=minY>i?i:minY;
				maxX=maxX<j?j:maxX;
				maxY=maxY<i?i:maxY;
			}

		}
		
		Obj_rect_t_1[0]=minX;
		Obj_rect_t_1[1]=minY;
		Obj_rect_t_1[2]=maxX-minX+iGridSize;
		Obj_rect_t_1[3]=maxY-minY+iGridSize;
	
	}
	else 
	{
		Obj_rect_t_1[0]=Obj_rect_t[0];
		Obj_rect_t_1[1]=Obj_rect_t[1];
		Obj_rect_t_1[2]=Obj_rect_t[2];
		Obj_rect_t_1[3]=Obj_rect_t[3];
	}
	
	for(    i=0;i<image->height;i+=iGridSize)
	for(int j=0;j<image->width; j+=iGridSize)
	{
		int p=i/iGridSize,q=j/iGridSize;
		if(NULL!=ObjKt_1[p][q])
		{	free(ObjKt_1[p][q]); ObjKt_1[p][q]=NULL;}

		if(NULL!=ObjKt[p][q])
		{
			point=(objFeature *)malloc(sizeof(objFeature ));
				
				point->fPrior=ObjKt[p][q]->fPrior;
				point->iColor[0]=ObjKt[p][q]->iColor[0];
				point->iColor[1]=ObjKt[p][q]->iColor[1];
				point->iColor[2]=ObjKt[p][q]->iColor[2];

				ObjKt_1[p][q]=point;		
	
			free(ObjKt[p][q]);  ObjKt[p][q]=NULL;
		}
	}			
*/
	cvReleaseImage(&IBinary);
}

CvRect fobjectMatching(IplImage *image,IplImage *IF_posterior,objFeature *ObjKt[objRows][objCols], objFeature *ObjKt_1[objRows][objCols], CvRect rectt, CvRect rectt_1)
{
	int iSearchX1,iSearchX2, iSearchY1,iSearchY2;
	int iSearchShift=iGridSize;
	int iSearchArea=iGridSize*5;
	float fSearchScoreMax=-999.0;
//	int iMaxX,iMaxY;
	int x,y;


	IplImage* bestMatch=cvCloneImage(image);
//	cvRectangle( bestMatch, cvPoint(Obj_rect_t_1[0], Obj_rect_t_1[1] ), cvPoint(Obj_rect_t_1[0]+Obj_rect_t_1[2],Obj_rect_t_1[1]+Obj_rect_t_1[3] ), cvScalar(255,0,255),2, 8,0 );

	iSearchX1=(rectt_1.x-iSearchArea)<0?0:(rectt_1.x-iSearchArea);
	iSearchY1=(rectt_1.y-iSearchArea)<0?0:(rectt_1.y-iSearchArea);

	iSearchX2=(rectt_1.x+rectt_1.width+iSearchArea)>image->width?(image->width-rectt_1.width):(rectt_1.x+iSearchArea);
	iSearchY2=(rectt_1.y+rectt_1.height+iSearchArea)>image->height?(image->height-rectt_1.height):(rectt_1.y+iSearchArea);
	
//	int iFetX=iSearchX2,iFeatY=iSearchY2;

//	cvRectangle( image, cvPoint(iSearchX1, iSearchY1 ), cvPoint(iSearchX2,iSearchY2 ), cvScalar(0,0,255),2, 8,0 );
//	showImg(image,"searchArea");

	CvScalar rgb;

	for( x=iSearchX1;x<iSearchX2;x+=iSearchShift)
	for( y=iSearchY1;y<iSearchY2;y+=iSearchShift)	
	{
//		int iSeqPos;
		int countValid=0;
		float fSearchScore=0.0;
//		objFeature objfeat;

		for(int m=x,j=rectt_1.x;m<(x+rectt_1.width);m+=iGridSize,j+=iGridSize)
		for(int n=y,i=rectt_1.y;n<(y+rectt_1.height);n+=iGridSize,i+=iGridSize)
		{
//			iSeqPos=(i/iGridSize)*(image->width/iGridSize)+(j/iGridSize); //i on height and j on width
			int p=i/iGridSize;
			int q=j/iGridSize;

			if(NULL!=ObjKt_1[p][q])
			{
				countValid++;
				// objfeat=*CV_GET_SEQ_ELEM( objFeature, Obj_Kt_1[iSeqPos], 0 );
					
				cvSetImageROI(image,cvRect(m,n,iGridSize,iGridSize)); 
				rgb=cvAvg(image);
				cvResetImageROI(image);

				fSearchScore+=( 1.0-(1.0/255.0)*sqrt((1.0/(float)3.0) * (float)
									((ObjKt_1[p][q]->iColor[0]- rgb.val[0]) * (ObjKt_1[p][q]->iColor[0]- rgb.val[0])
									+(ObjKt_1[p][q]->iColor[1]- rgb.val[1]) * (ObjKt_1[p][q]->iColor[1]- rgb.val[1])
									+(ObjKt_1[p][q]->iColor[2]- rgb.val[2]) * (ObjKt_1[p][q]->iColor[2]- rgb.val[2]) ) ) );
			}

		}
		char score[100];
		sprintf(score,"fSearchScore::%f",fSearchScore);
		fSearchScore/=countValid;
		if((fSearchScore-fSearchScoreMax) > EPS)
		{
			fSearchScoreMax=fSearchScore;
			rectt.x=x;	//iMaxX=x; 
			rectt.y=y;	//iMaxY=y;
			rectt.width=rectt_1.width;
			rectt.height=rectt_1.height;

		//	cvSetImageROI(bestMatch,cvRect(x,y,2,2));
		//	cvSet( bestMatch,cvScalar(0,0,255) );
		//	cvResetImageROI(bestMatch);

		}

	}
/*	for(int	i=0;i<image->height;i+=iGridSize)
	for(int j=0;j<image->width;j+=iGridSize)	
	{
		int p=i/iGridSize;
		int q=j/iGridSize;

		if(NULL!=ObjKt_1[p][q])
		{
			cvSetImageROI(bestMatch,cvRect( ( j+rectt.x-rectt_1.x+2 <=0 ? 0:rectt.x-rectt_1.x+2 ),( i+rectt.y-rectt_1.y+2 <=0?0:i+rectt.y-rectt_1.y+2 ),1,1));
			cvSet( bestMatch,cvScalar(0,255,0) );
			cvResetImageROI(bestMatch);
		}
	}
	*/
	cvRectangle( bestMatch, cvPoint(rectt.x, rectt.y), cvPoint(rectt.x+rectt.width,rectt.y+rectt.height ), cvScalar(255,0,255),1, 8,0 );
//	showImg(bestMatch,"matching");

//	cvReleaseImage(&bestMatch);
	return rectt;
}

//void objectMatching(IplImage *image,cvSeq *Obj_Kt[], cvSeq *ObjKt_1[],int  Obj_rect_t[],int  Obj_rect_t_1[])
void objectMatching(IplImage *image,IplImage *IF_posterior,objFeature *ObjKt[objRows][objCols], objFeature *ObjKt_1[objRows][objCols],int  Obj_rect_t[],int  Obj_rect_t_1[])
{
	int iSearchX1,iSearchX2, iSearchY1,iSearchY2;
	int iSearchShift=iGridSize;
	int iSearchArea=iGridSize*5;
	float fSearchScoreMax=-999.0;
//	int iMaxX,iMaxY;
	int x,y;


	IplImage* bestMatch=cvCloneImage(image);
//	cvRectangle( bestMatch, cvPoint(Obj_rect_t_1[0], Obj_rect_t_1[1] ), cvPoint(Obj_rect_t_1[0]+Obj_rect_t_1[2],Obj_rect_t_1[1]+Obj_rect_t_1[3] ), cvScalar(255,0,255),2, 8,0 );

/*
	for(i=0;i<image->height;i+=iGridSize)
	for(int j=0;j<image->width; j+=iGridSize)
	{
//		int iSeqPos=(i/iGridSize)*(image->width/iGridSize)+(j/iGridSize);
		
		if(NULL != ObjKt[i/iGridSize][j/iGridSize])
		{		
			cvSetImageROI(bestMatch,cvRect(j,i,iGridSize,iGridSize));
		//	cvSetImageROI(bestMatch,cvRect(j,i,iGridSize,iGridSize));
		
			cvSet( bestMatch,cvScalar(255,0,255) );
			cvResetImageROI(bestMatch);
		}
	}
	showImg(bestMatch,"bestMatch");
*/
	iSearchX1=(Obj_rect_t_1[0]-iSearchArea)<0?0:(Obj_rect_t_1[0]-iSearchArea);
	iSearchY1=(Obj_rect_t_1[1]-iSearchArea)<0?0:(Obj_rect_t_1[1]-iSearchArea);

	iSearchX2=(Obj_rect_t_1[0]+Obj_rect_t_1[2]+iSearchArea)>image->width?(image->width-Obj_rect_t_1[2]):(Obj_rect_t_1[0]+iSearchArea);
	iSearchY2=(Obj_rect_t_1[1]+Obj_rect_t_1[3]+iSearchArea)>image->height?(image->height-Obj_rect_t_1[3]):(Obj_rect_t_1[1]+iSearchArea);
	
//	int iFetX=iSearchX2,iFeatY=iSearchY2;

//	cvRectangle( image, cvPoint(iSearchX1, iSearchY1 ), cvPoint(iSearchX2,iSearchY2 ), cvScalar(0,0,255),2, 8,0 );
//	showImg(image,"searchArea");

	CvScalar rgb;

	for( x=iSearchX1;x<iSearchX2;x+=iSearchShift)
	for( y=iSearchY1;y<iSearchY2;y+=iSearchShift)	
	{
//		int iSeqPos;
		int countValid=0;
		float fSearchScore=0.0;
//		objFeature objfeat;

		for(int m=x,j=Obj_rect_t_1[0];m<(x+Obj_rect_t_1[2]);m+=iGridSize,j+=iGridSize)
		for(int n=y,i=Obj_rect_t_1[1];n<(y+Obj_rect_t_1[3]);n+=iGridSize,i+=iGridSize)
		{
//			iSeqPos=(i/iGridSize)*(image->width/iGridSize)+(j/iGridSize); //i on height and j on width
			int p=i/iGridSize;
			int q=j/iGridSize;

			if(NULL!=ObjKt_1[p][q])
			{
				countValid++;
				// objfeat=*CV_GET_SEQ_ELEM( objFeature, Obj_Kt_1[iSeqPos], 0 );
					
				cvSetImageROI(image,cvRect(m,n,iGridSize,iGridSize)); 
				rgb=cvAvg(image);
				cvResetImageROI(image);

				fSearchScore+=( 1.0-(1.0/255.0)*sqrt((1.0/(float)3.0) * (float)
									((ObjKt_1[p][q]->iColor[0]- rgb.val[0]) * (ObjKt_1[p][q]->iColor[0]- rgb.val[0])
									+(ObjKt_1[p][q]->iColor[1]- rgb.val[1]) * (ObjKt_1[p][q]->iColor[1]- rgb.val[1])
									+(ObjKt_1[p][q]->iColor[2]- rgb.val[2]) * (ObjKt_1[p][q]->iColor[2]- rgb.val[2]) ) ) );
			}

		}
		char score[100];
		sprintf(score,"fSearchScore::%f",fSearchScore);
		fSearchScore/=countValid;
		if((fSearchScore-fSearchScoreMax) > EPS)
		{
			fSearchScoreMax=fSearchScore;
			Obj_rect_t[0]=x;	//iMaxX=x; 
			Obj_rect_t[1]=y;	//iMaxY=y;
			Obj_rect_t[2]=Obj_rect_t_1[2];
			Obj_rect_t[3]=Obj_rect_t_1[3];



		/*	cvSetImageROI(bestMatch,cvRect(x,y,2,2));
			cvSet( bestMatch,cvScalar(0,0,255) );
			cvResetImageROI(bestMatch);
			*/
		}
/*		cvCopyImage(image,bestMatch);
		
		for(    m=x,j=Obj_rect_t_1[0];m<(x+Obj_rect_t_1[2]);m+=iGridSize,j+=iGridSize)
		for(int n=y,i=Obj_rect_t_1[1];n<(y+Obj_rect_t_1[3]);n+=iGridSize,i+=iGridSize)
		{
//			iSeqPos=(i/iGridSize)*(image->width/iGridSize)+(j/iGridSize); //i on height and j on width
			int p=i/iGridSize;
			int q=j/iGridSize;

			if(NULL!=ObjKt_1[p][q])
			{
				cvSetImageROI(bestMatch,cvRect(m,n,2,2));
				cvSet( bestMatch,cvScalar(ObjKt_1[p][q]->iColor[0],ObjKt_1[p][q]->iColor[1],ObjKt_1[p][q]->iColor[2]) );
		//		cvSet( bestMatch,cvScalar(0,255,0) );
				cvResetImageROI(bestMatch);
			}
		}
		
		CvFont font;
		double hScale=0.5;
		double vScale=0.5;
		int    lineWidth=1;
		cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, hScale,vScale,0,lineWidth);

		cvPutText (bestMatch,score,cvPoint(10,20), &font, cvScalar(0,0,255));
*/
		//		showImg(bestMatch,"bestMatch");
	
		//cvRectangle( bestMatch, cvPoint(x, y ), cvPoint(x+2,y+2 ), cvScalar(0,255,0),2, 2,0 );

	}

//	IplImage* bestMatch=cvCloneImage(image);
	
// checking of the best matching location
//	cvRectangle( bestMatch, cvPoint(Obj_rect_t_1[0], Obj_rect_t_1[1] ), cvPoint(Obj_rect_t_1[0]+Obj_rect_t[2],Obj_rect_t_1[1]+Obj_rect_t[3] ), cvScalar(0,0,255),2, 8,0 );

//	cvRectangle( bestMatch, cvPoint(Obj_rect_t[0], Obj_rect_t[1] ), cvPoint(Obj_rect_t[0]+Obj_rect_t[2],Obj_rect_t[1]+Obj_rect_t[3] ), cvScalar(0,255,0),2, 8,0 );
	int j,m;
	
	for(    m=Obj_rect_t[0],j=Obj_rect_t_1[0];m<(Obj_rect_t[0]+Obj_rect_t[2]);m+=iGridSize,j+=iGridSize)
	for(int n=Obj_rect_t[1],i=Obj_rect_t_1[1];n<(Obj_rect_t[1]+Obj_rect_t[3]);n+=iGridSize,i+=iGridSize)
	{
		int p=i/iGridSize;
		int q=j/iGridSize;
		if(NULL != ObjKt_1[p][q])
		{		
			cvSetImageROI(bestMatch,cvRect(m,n,2,2));
		//	cvSetImageROI(bestMatch,cvRect(j,i,iGridSize,iGridSize));
		
			cvSet( bestMatch,cvScalar(255,0,0) );
			cvResetImageROI(bestMatch);
		}
	}

/*
	for( i=0;i<image->height;i+=iGridSize)
	for(int j=0;j<image->width; j+=iGridSize)
	{
		
		if(NULL != ObjKt_1[i/iGridSize][j/iGridSize])
		{		
			cvSetImageROI(bestMatch,cvRect(j+Obj_rect_t[0]-Obj_rect_t_1[0],i+Obj_rect_t[1]-Obj_rect_t_1[1],2,2));
		//	cvSetImageROI(bestMatch,cvRect(j,i,iGridSize,iGridSize));
		
			cvSet( bestMatch,cvScalar(0,255,0) );
			cvResetImageROI(bestMatch);
		}
	}
*/
//    showImg(bestMatch,"bestMatch");
	cvReleaseImage(&bestMatch);
}

void Cues::computeLiklihoodColor(IplImage *IF_posterior,IplImage *IB_posterior,CvMatND *FGColorModel,CvMatND *BGColorModel,IplImage *image, int index,CvSeq *ColorSeqF, CvSeq *ColorSeqB)
{
	cvSetZero(IF_Likelihood);
	cvSetZero(IB_Likelihood);
	
	//compute color likelihood
	//if(0<index)
	int count=cvCountNonZero( FGColorModel );
	if(cvCountNonZero( FGColorModel )>0)
	{
		for(int	i=0;i<image->height;i++)
		for(int j=0;j<image->width;j++)	
		{
			int iHistb=(int) ((int)getPixel(image,i,j,0)*colorHistfactor);
			int iHistg=(int) ((int)getPixel(image,i,j,1)*colorHistfactor);
			int iHistr=(int) ((int)getPixel(image,i,j,2)*colorHistfactor);

			setPixel32F(IF_Likelihood,i,j,cvGetReal3D(FGColorModel,iHistr,iHistg,iHistb),0 );	
			setPixel32F(IB_Likelihood,i,j,cvGetReal3D(BGColorModel,iHistr,iHistg,iHistb),0 );
		}
	}
	else 
	{
		cvCopy(IF_posterior,IF_Likelihood);
		cvCopy(IB_posterior,IB_Likelihood);
	}
//	showImg(IF_posterior,"FGpost");
//	showImg(IB_posterior,"BGpost");
//	showImg(IF_Likelihood,"colorFGLikelihood");
//	showImg(IB_Likelihood,"colorBGLikelihood");
	return;
}

void Cues::computeColorRep(IplImage *IF_posterior,IplImage *IB_posterior,CvMatND *FGColorModel,CvMatND *BGColorModel,IplImage *image, int index,CvSeq *ColorSeqF, CvSeq *ColorSeqB)
{
	
	int size[]={colorHistlength,colorHistlength,colorHistlength};
	CvMatND *FGColorModel_t=cvCreateMatND( 3,size, CV_32FC1);
	CvMatND *BGColorModel_t=cvCreateMatND( 3,size, CV_32FC1);
	CvMatND *oprMat=cvCreateMatND( 3,size, CV_32FC1);

	cvSetZero(FGColorModel_t);
	cvSetZero(BGColorModel_t);
	cvSetZero(oprMat);

	//probability accumulation
	float fCf,fCb,total;

	for(int i=0;i<image->height;i++)
	for(int j=0;j<image->width;j++)	
	{	
		int b=(int) getPixel(image,i,j,0);
		int g=(int) getPixel(image,i,j,1);
		int r=(int) getPixel(image,i,j,2);
		
		int iHistr=(int)(r*colorHistfactor);
		int iHistg=(int)(g*colorHistfactor);
		int iHistb=(int)(b*colorHistfactor);

		//Normalization
		
		fCf= cvGetReal3D(FGColorModel_t,iHistr,iHistg,iHistb)+getPixel32F(IF_posterior,i,j,0) ;
		fCb= cvGetReal3D(BGColorModel_t,iHistr,iHistg,iHistb)+getPixel32F(IB_posterior,i,j,0) ;

/*		if( ( (0.0-fCf)*(0.0-fCf) > EPS) || ( (0.0-fCb)*(0.0-fCb) > EPS) ) 
		{
			total=fCf+fCb;
			fCf=( fCf/ (total) );
			fCb=( fCb/ (total) );
		}
		else { fCf=0.5; fCb=0.5; }
*/
		cvSetReal3D(FGColorModel_t,iHistr,iHistg,iHistb, fCf);
		cvSetReal3D(BGColorModel_t,iHistr,iHistg,iHistb, fCb);
	}

	//normalization after smothing
/*	cvAdd(FGColorModel_t,BGColorModel_t,oprMat);
	cvDiv(FGColorModel_t,oprMat,FGColorModel_t);
	cvDiv(BGColorModel_t,oprMat,BGColorModel_t);
	*/
	for(int    i=0;i<FGColorModel_t->dim[0].size;i++)
	for(int j=0;j<FGColorModel_t->dim[1].size;j++)
	for(int k=0;k<FGColorModel_t->dim[2].size;k++)
	{
		fCf=cvGetReal3D(FGColorModel_t,i,j,k); 
		fCb=cvGetReal3D(BGColorModel_t,i,j,k); 

		if( ( (0.0-fCf)*(0.0-fCf) > EPS) || ( (0.0-fCb)*(0.0-fCb) > EPS) ) 
		{
			total=fCf+fCb;
			fCf=( fCf/ (total) );
			fCb=( fCb/ (total) );
		}
		else { fCf=0.5; fCb=0.5; }
	
		cvSetReal3D(FGColorModel_t,i,j,k,fCf);
		cvSetReal3D(BGColorModel_t,i,j,k,fCb);
		
	}
	//gaussian smoothing, needs vectorization for computatoin adv
	gaussianSmoothing(FGColorModel_t);
	gaussianSmoothing(BGColorModel_t);

	
	//normalization after smothing
/*	cvAdd(FGColorModel_t,BGColorModel_t,oprMat);
	cvDiv(FGColorModel_t,oprMat,FGColorModel_t);
	cvDiv(BGColorModel_t,oprMat,BGColorModel_t);
 
*/	for(int i=0;i<FGColorModel_t->dim[0].size;i++)
	for(int j=0;j<FGColorModel_t->dim[1].size;j++)
	for(int k=0;k<FGColorModel_t->dim[2].size;k++)
	{
		fCf=cvGetReal3D(FGColorModel_t,i,j,k); 
		fCb=cvGetReal3D(BGColorModel_t,i,j,k); 

		if( ( (0.0-fCf)*(0.0-fCf) > EPS) || ( (0.0-fCb)*(0.0-fCb) > EPS) ) 
		{
			total=fCf+fCb;
			fCf=( fCf/ (total) );
			fCb=( fCb/ (total) );
		}
		else { fCf=0.5; fCb=0.5; }
	
		cvSetReal3D(FGColorModel_t,i,j,k,fCf);
		cvSetReal3D(BGColorModel_t,i,j,k,fCb);
		
	}
	//updatation of C_t  // Ct+dCt-1/ (1+d) // d is 0.8
// 	cvAddWeighted( FGColorModel, 0.45,FGColorModel_t, 0.55,0.0, FGColorModel );
//	cvAddWeighted( BGColorModel, 0.45,BGColorModel_t, 0.55,0.0, BGColorModel );

	if(0<index)
	{
		cvConvertScale(FGColorModel_t,FGColorModel_t,0.65,0.0);
		cvConvertScale(BGColorModel_t,BGColorModel_t,0.65,0.0);
		
		cvConvertScale(FGColorModel,FGColorModel,0.35,0.0);
		cvConvertScale(BGColorModel,BGColorModel,0.35,0.0);

		cvAdd(FGColorModel_t,FGColorModel,FGColorModel);
		cvAdd(BGColorModel_t,BGColorModel,BGColorModel);
		
	}
	else 
	{
		cvCopy(FGColorModel_t,FGColorModel);
		cvCopy(BGColorModel_t,BGColorModel);
	}
	cvReleaseMatND(&FGColorModel_t);
	cvReleaseMatND(&BGColorModel_t);
	cvReleaseMatND(&oprMat);

	return;
}

void pushInSeq_1(CvSeq* ColorSeqF, int idx)
{
	int s=0,mid, e=(ColorSeqF->total<=1?0:ColorSeqF->total-1);
	mid=((s+e)/2);
	int *t;
	if(0==ColorSeqF->total) { cvSeqPush( ColorSeqF, &idx ); }
	else
	{
		while(true)
		{	t=(int *)cvGetSeqElem( ColorSeqF, mid );
			if(*t<idx)
			{	
				if(s>=e) 
				{
					cvSeqInsert(ColorSeqF,mid+1,&idx);break;
				}
				s=mid+1; mid=(s+e)/2;
				
			}
			else if(*t>idx)
			{	
				if(s>=e) 
				{
					cvSeqInsert(ColorSeqF,mid,&idx);break;
				}
				e=mid-1; mid=(s+e)/2;  
			}
			else
			{
				cvSeqInsert(ColorSeqF,mid,&idx); 
				cvSeqRemove(ColorSeqF,mid+1);
				break;
			}  
		}
	}//else part
	
} 

void pushInSeq(CvSeq* ColorSeqF, histpoint point)
{
	int idx=point.idx;
	int s=0,mid, e=(ColorSeqF->total<=1?0:ColorSeqF->total-1);
	mid=((s+e)/2);
	histpoint t;

	int count=0;
	if(0==ColorSeqF->total) { cvSeqPush( ColorSeqF, &point ); }
	else
	{
		while(true)
		{
			t= *CV_GET_SEQ_ELEM( histpoint, ColorSeqF, mid );
			if(t.idx>idx)
			{	
				if(s>=e) 
				{
					cvSeqInsert(ColorSeqF,mid,&point);break;
				}
				e=mid-1; mid=(s+e)/2; 
			}
			else if(t.idx<idx)
			{	
				if(s>=e) 
				{
					cvSeqInsert(ColorSeqF,mid+1,&point);break;
				}
				s=mid+1; mid=(s+e)/2; 
			}
			else
			{
				cvSeqInsert(ColorSeqF,mid,&point); 
				cvSeqRemove(ColorSeqF,mid+1);
				break;
			}  
		}
	}//else part

}

void Cues::computeLiklihoodTexture(IplImage *IF_posterior,IplImage *IB_posterior,CvMatND *FGTextureModel,CvMatND *BGTextureModel,IplImage *image,int index)
{
	
	IplImage *IgrayImage=cvCreateImage(cvGetSize(image),IPL_DEPTH_8U, 1);

	IplImage *S=cvCreateImage(cvGetSize(image),IPL_DEPTH_8U, 1);
	IplImage *H=cvCreateImage(cvGetSize(image),IPL_DEPTH_8U, 1);
	IplImage *V=cvCreateImage(cvGetSize(image),IPL_DEPTH_8U, 1);
	
	cvSetZero(S);cvSetZero(H);cvSetZero(V);

	cvCvtColor(image,IgrayImage,CV_BGR2GRAY);

	int size[]={textureHistlength,textureHistlength,textureHistlength};
	CvMatND *FGTextureModel_t=cvCreateMatND( 3,size, CV_32FC1);
	CvMatND *BGTextureModel_t=cvCreateMatND( 3,size, CV_32FC1);
	CvMatND *oprMat=cvCreateMatND( 3,size, CV_32FC1);

	cvSetZero(FGTextureModel_t);
	cvSetZero(BGTextureModel_t);
	cvSetZero(oprMat);

	int ss,sh,sv;
	float fTf,fTb;//,total;
	int iHists,iHisth,iHistv;

	CvScalar a,s;
	for(int i=4;i<image->height-4;i++)
	for(int j=4;j<image->width-4;j++)	
	{
		cvSetImageROI(IgrayImage,cvRect(j-2,i-2,5,5));
		cvAvgSdv(IgrayImage,&a,&s);
		ss=(int)s.val[0];
		setPixel(S,i,j,ss,0);
		cvResetImageROI(IgrayImage);

		s.val[0]=0.0;
		cvSetImageROI(IgrayImage,cvRect(j-4,i,9,1));
		cvAvgSdv(IgrayImage,&a,&s); 
		sh=(int)s.val[0];
		setPixel(H,i,j,sh,0);
		cvResetImageROI(IgrayImage);

		s.val[0]=0.0;
		cvSetImageROI(IgrayImage,cvRect(j,i-4,1,9));
		cvAvgSdv(IgrayImage,&a,&s); 
		sv=(int)s.val[0];
		setPixel(V,i,j,sv,0);
		cvResetImageROI(IgrayImage);
	
		//probability accumulation
		//Normalization
		
		iHists=(int)(ss*textureHistfactor);
		iHisth=(int)(sh*textureHistfactor);
		iHistv=(int)(sv*textureHistfactor);

		fTf= cvGetReal3D(FGTextureModel_t,iHists,iHisth,iHistv)+getPixel32F(IF_posterior,i,j,0) ;
		fTb= cvGetReal3D(BGTextureModel_t,iHists,iHisth,iHistv)+getPixel32F(IB_posterior,i,j,0) ;
/*
		
		if( ( (0.0-fTf)*(0.0-fTf) > EPS) || ( (0.0-fTb)*(0.0-fTb) > EPS) ) 
		{
			total=( fTf/ (fTf+fTb) );
			fTf/=total ;
			fTb/=total ;
		}
		else { fTf=0.5; fTb=0.5; }
*/
		cvSetReal3D(FGTextureModel_t, iHists,iHisth,iHistv, fTf);
		cvSetReal3D(BGTextureModel_t, iHists,iHisth,iHistv, fTb);
	
	}

/*	showImg(S,"small");
	showImg(H,"horizontal");
	showImg(V,"vertical");
*/
	//normalization before smothing
/*	cvAdd(FGTextureModel_t,BGTextureModel_t,oprMat);
	cvDiv(FGTextureModel_t,oprMat,FGTextureModel_t);
	cvDiv(BGTextureModel_t,oprMat,BGTextureModel_t);
 */
	int countZero=0;
	float fCf,fCb,total;
	for(int i=0;i<FGTextureModel_t->dim[0].size;i++)
	for(int j=0;j<FGTextureModel_t->dim[1].size;j++)
	for(int k=0;k<FGTextureModel_t->dim[2].size;k++)
	{
		fCf=cvGetReal3D(FGTextureModel_t,i,j,k); 
		fCb=cvGetReal3D(BGTextureModel_t,i,j,k); 

		if( ( (0.0-fCf)*(0.0-fCf) > EPS) || ( (0.0-fCb)*(0.0-fCb) > EPS) ) 
		{
			total=fCf+fCb;
			fCf=( fCf/ (total) );
			fCb=( fCb/ (total) );
		}
//		else { fCf=0.5; fCb=0.5; countZero++;}
	
		cvSetReal3D(FGTextureModel_t,i,j,k,fCf);
		cvSetReal3D(BGTextureModel_t,i,j,k,fCb);
		
	}
 	countZero++;
	//gaussian smoothing, needs vectorization for computatoin adv

	gaussianSmoothing(FGTextureModel_t);
	gaussianSmoothing(BGTextureModel_t);
	
	//normalization after smothing
/*	cvAdd(FGTextureModel_t,BGTextureModel_t,oprMat);
	cvDiv(FGTextureModel_t,oprMat,FGTextureModel_t);
	cvDiv(BGTextureModel_t,oprMat,BGTextureModel_t);
 */
	countZero=0;
	for(int i=0;i<FGTextureModel_t->dim[0].size;i++)
	for(int j=0;j<FGTextureModel_t->dim[1].size;j++)
	for(int k=0;k<FGTextureModel_t->dim[2].size;k++)
	{
		fCf=cvGetReal3D(FGTextureModel_t,i,j,k); 
		fCb=cvGetReal3D(BGTextureModel_t,i,j,k); 

		if( ( (0.0-fCf)*(0.0-fCf) > EPS) || ( (0.0-fCb)*(0.0-fCb) > EPS) ) 
		{
			total=fCf+fCb;
			fCf=( fCf/ (total) );
			fCb=( fCb/ (total) );
		}
		else { fCf=0.5; fCb=0.5; countZero++; }
	
		cvSetReal3D(FGTextureModel_t,i,j,k,fCf);
		cvSetReal3D(BGTextureModel_t,i,j,k,fCb);
		
	}

	countZero++;
	//updatation of C_t  // Ct+dCt-1/ (1+d) // d is 0.8
// 	cvAddWeighted( FGColorModel, 0.45,FGColorModel_t, 0.55,0.0, FGColorModel );
//	cvAddWeighted( BGColorModel, 0.45,BGColorModel_t, 0.55,0.0, BGColorModel );

	if(0<index)
	{
		cvConvertScale(FGTextureModel_t,FGTextureModel_t,0.65,0.0);
		cvConvertScale(BGTextureModel_t,BGTextureModel_t,0.65,0.0);
		
		cvConvertScale(FGTextureModel,FGTextureModel,0.35,0.0);
		cvConvertScale(BGTextureModel,BGTextureModel,0.35,0.0);

		cvAdd(FGTextureModel_t,FGTextureModel,FGTextureModel);
		cvAdd(BGTextureModel_t,BGTextureModel,BGTextureModel);
		
	}
	else 
	{
		cvCopy(FGTextureModel_t,FGTextureModel);
		cvCopy(BGTextureModel_t,BGTextureModel);

	}
	cvReleaseMatND(&FGTextureModel_t);
	cvReleaseMatND(&BGTextureModel_t);
	cvReleaseMatND(&oprMat);


	//compute color likelihood
	if(0<index)
	{

		for(int	i=0;i<image->height;i++)
		for(int j=0;j<image->width;j++)	
		{
			int s=(int) ((int)getPixel(S,i,j,0)*textureHistfactor);
			int h=(int) ((int)getPixel(H,i,j,0)*textureHistfactor);
			int v=(int) ((int)getPixel(V,i,j,0)*textureHistfactor);

			setPixel32F(IF_Likelihood,i,j,cvGetReal3D(FGTextureModel,s,h,v),0 );	
			setPixel32F(IB_Likelihood,i,j,cvGetReal3D(BGTextureModel,s,h,v),0 );
		}
	}
	else 
	{
		cvCopy(IF_posterior,IF_Likelihood);
		cvCopy(IB_posterior,IB_Likelihood);
	}

	cvReleaseImage(&IgrayImage);
	cvReleaseImage(&S);
	cvReleaseImage(&H);
	cvReleaseImage(&V);

	
//	showImg(IF_Likelihood,"textureFGLikelihood");
//	showImg(IB_Likelihood,"textureBGLikelihood");

	return;
}

void Cues::computeLiklihood( IplImage *image1,IplImage *image2)
{	
	float x0=30.0,sigma1=10.0,sigma2=50.0; 
	float a=(float)1.026,b=(float)0.388;
	int r1,g1,b1,r2,g2,b2;
	
	cvSetZero(IF_Likelihood);
	cvSetZero(IB_Likelihood);
//	IplImage *diff=cvCloneImage(image1);
//	cvSetZero(diff);

//	cvSub(image2,image1,diff);
//	cvMul(diff,diff,diff);
//	cvSqrt(diff,diff);


//	IplImage *temp=cvCloneImage(IF_Likelihood);

	for(int i=0;i<image1->height;i++)
	for(int j=0;j<image1->width;j++)
	{
		b1=(int)getPixel(image1,i,j,0);g1=(int)getPixel(image1,i,j,1);r1=(int)getPixel(image1,i,j,2);
		b2=(int)getPixel(image2,i,j,0);g2=(int)getPixel(image2,i,j,1);r2=(int)getPixel(image2,i,j,2);
		float d=sqrt( (float)( (b2-b1)*(b2-b1)+(g2-g1)*(g2-g1)+(r2-r1)*(r2-r1) ) /(float)3.0);
		
/*		b1=(int)getPixel(diff,i,j,0);g1=(int)getPixel(diff,i,j,1);r1=(int)getPixel(diff,i,j,2);
		float d=sqrt( (float)( (b1*b1)+(g1)*(g1)+(r1)*(r1) ) /(float)3.0);
*/
		float mapping=pow(a+b*(exp((x0-d)/sigma1) ),-1.0)* pow(a+b*(exp((x0-d)/sigma2) ),-1.0);

		setPixel32F(IF_Likelihood,i,j,mapping,0);
		setPixel32F(IB_Likelihood,i,j,(float) (1.0-mapping),0);
//		setPixel32F(temp,i,j,(int)(d/35.0),0);
	}
//	cvReleaseImage(&diff);
	//Background model modification 
//	cvAddWeighted( image1, 0.7,image2, 0.3,0.0, image1 );
	// this above line is commented as background model will be called in every frame
	// selectively back gound model modification (only bg) to be implemented
//	showImg(temp,"pixchange");
}
void Cues::gaussianSmoothingNormalization_1(CvMatND *FGModel,CvMatND *BGModel,CvMatND * FGModelt_1,CvMatND *BGModelt_1,CvSeq *SeqF, CvSeq *SeqB,int index)
{
//	histpoint point;
	int idx,*element,i,j,k;
//	float count;

	int size[]={gMatSize,gMatSize,gMatSize};
	CvMatND *gaussMat=cvCreateMatND( 3,size, CV_32FC1);
	
	for(    i=0;i<gMatSize;i++)
	for(int j=0;j<gMatSize;j++)
	for(int k=0;k<gMatSize;k++)
	{
		float fGuassStrength=(float)(exp( -1* ( pow( i-(int) (gMatSize/2),2.0)/ (2.0*gaussSigma*gaussSigma) +
												pow( j-(int) (gMatSize/2),2.0)/ (2.0*gaussSigma*gaussSigma) +
												pow( j-(int) (gMatSize/2),2.0)/ (2.0*gaussSigma*gaussSigma) ) ) );
		cvSetReal3D(gaussMat, i, j, k, fGuassStrength);
	}
	

	float likelihoodMf, likelihoodMold1f, likelihoodMold2f;
	float likelihoodMb, likelihoodMold1b, likelihoodMold2b;

	float fCf,fCb,total;
//	float fFGt,fFGt_1,fBGt,fBGt_1;

	for(int l=0;l<SeqF->total;l++)
	{
		element= (int *)cvGetSeqElem( SeqF, l );
		idx=*element;
		i=idx/(colorHistlength*colorHistlength);
		j=(idx-i*colorHistlength*colorHistlength)/colorHistlength;
		k=(idx-i*colorHistlength*colorHistlength-j*colorHistlength);
		//idx=point.idx, i=point.i; j=point.j; k=point.k; count=point.count;
		
		if(i>=2 && i < FGModel->dim[0].size-2 && j>=2 && j < FGModel->dim[1].size-2 && k>=2 && k < FGModel->dim[2].size-2)
		{
			likelihoodMf=cvGetReal3D(FGModel,i,j,k); 
		//	bool normalization=false;
			
			if(likelihoodMf>0.5) 
			for(int l=i-2;l<=i+2;l++)
			for(int m=j-2;m<=j+2;m++)
			for(int n=k-2;n<=k+2;n++)
			{
				likelihoodMold1f=cvGetReal3D(FGModel,l,m,n);
				likelihoodMold2f= likelihoodMf*cvGetReal3D(gaussMat,l-i+2,m-j+2,n-k+2);
				if(likelihoodMold1f<likelihoodMold2f)
				{
				//	cvSetReal3D(FGModel,l, m, n, likelihoodMold2f);

					fCf= likelihoodMold2f;
					fCb=cvGetReal3D(BGModel,l,m,n); 

					total=fCf+fCb;
					fCf=( fCf/ (total) );
					fCb=( fCb/ (total) );
				
					cvSetReal3D(FGModel,l,m,n,fCf);
					cvSetReal3D(BGModel,l,m,n,fCb);
					/*if(0==index)
					{
						cvSetReal3D(FGModelt_1,l,m,n,fCf);
						cvSetReal3D(BGModelt_1,l,m,n,fCb);
					}
					else
					{
						fFGt_1=cvGetReal3D(FGModelt_1,l,m,n);
						fBGt_1=cvGetReal3D(BGModelt_1,l,m,n);

						fFGt=(fCf*0.55+fFGt_1*1.0); fBGt=(fCb*0.55+fBGt_1*1.0);
						cvSetReal3D(FGModelt_1,l,m,n,fFGt);
						cvSetReal3D(BGModelt_1,l,m,n,fBGt);
					}*/

				}
			}
		}
	}
	for(int l=0;l<SeqB->total;l++)
	{
		element= (int *)cvGetSeqElem( SeqB, l );
		idx=*element;
		i=idx/(colorHistlength*colorHistlength);
		j=(idx-i*colorHistlength*colorHistlength)/colorHistlength;
		k=(idx-i*colorHistlength*colorHistlength-j*colorHistlength);
		//idx=point.idx, i=point.i; j=point.j; k=point.k; count=point.count;
		
		if(i>=2 && i < BGModel->dim[0].size-2 && j>=2 && j < BGModel->dim[1].size-2 && k>=2 && k < BGModel->dim[2].size-2)
		{
			likelihoodMb=cvGetReal3D(BGModel,i,j,k); 
		//	bool normalization=false;
			
			if(likelihoodMb>0.5) 
			for(int l=i-2;l<=i+2;l++)
			for(int m=j-2;m<=j+2;m++)
			for(int n=k-2;n<=k+2;n++)
			{
				likelihoodMold1b=cvGetReal3D(BGModel,l,m,n);
				likelihoodMold2b= likelihoodMb*cvGetReal3D(gaussMat,l-i+2,m-j+2,n-k+2);
				if(likelihoodMold1b<likelihoodMold2b)
				{
				//	cvSetReal3D(BGModel,l, m, n, likelihoodMold2b);

					fCf= cvGetReal3D(FGModel,l,m,n);;
					fCb=likelihoodMold2b;

					total=fCf+fCb;
					fCf=( fCf/ (total) );
					fCb=( fCb/ (total) );
				
					cvSetReal3D(FGModel,l,m,n,fCf);
					cvSetReal3D(BGModel,l,m,n,fCb);
					
					/*if(0==index)
					{
						cvSetReal3D(FGModelt_1,l,m,n,fCf);
						cvSetReal3D(BGModelt_1,l,m,n,fCb);
					}
					else
					{
						fFGt_1=cvGetReal3D(FGModelt_1,l,m,n);
						fBGt_1=cvGetReal3D(BGModelt_1,l,m,n);

						fFGt=(fCf*0.55+fFGt_1*1.0); fBGt=(fCb*0.55+fBGt_1*1.0);
						cvSetReal3D(FGModelt_1,l,m,n,fFGt);
						cvSetReal3D(BGModelt_1,l,m,n,fBGt);
					}*/

				}
			}
		}
	}
	
	cvReleaseMatND(&gaussMat);		
/*	histpoint point;
	int idx,i,j,k;
	float count;
	

	float likelihoodMf, likelihoodMold1f, likelihoodMold2f;
	float likelihoodMb, likelihoodMold1b, likelihoodMold2b;

	float fCf,fCb,total;
	float fFGt,fFGt_1,fBGt,fBGt_1;

	for(int l=0;l<SeqF->total;l++)
	{
		point= *CV_GET_SEQ_ELEM( histpoint, SeqF, l );
		idx=point.idx, i=point.i; j=point.j; k=point.k; count=point.count;
		
		if(i>=2 && i < FGModel->dim[0].size-2 && i>=2 && j < FGModel->dim[1].size-2 && k>=2 && k < FGModel->dim[2].size-2)
		{
			likelihoodMf=cvGetReal3D(FGModel,i,j,k); 
		//	bool normalization=false;
			
			if(likelihoodMf>0.5) 
			for(int l=i-2;l<=i+2;l++)
			for(int m=j-2;m<=j+2;m++)
			for(int n=k-2;n<=k+2;n++)
			{
				likelihoodMold1f=cvGetReal3D(FGModel,l,m,n);
				likelihoodMold2f= likelihoodMf*cvGetReal3D(gaussMat,l-i+2,m-j+2,n-k+2);
				if(likelihoodMold1f<likelihoodMold2f)
				{
					cvSetReal3D(FGModel,l, m, n, likelihoodMold2f);

					fCf= likelihoodMold2f;
					fCb=cvGetReal3D(BGModel,l,m,n); 

					total=fCf+fCb;
					fCf=( fCf/ (total) );
					fCb=( fCb/ (total) );
				
					
					if(0==index)
					{
						cvSetReal3D(FGModelt_1,l,m,n,fCf);
						cvSetReal3D(BGModelt_1,l,m,n,fCb);
					}
					else
					{
						fFGt_1=cvGetReal3D(FGModelt_1,l,m,n);
						fBGt_1=cvGetReal3D(BGModelt_1,l,m,n);

						fFGt=(fCf*0.75+fFGt_1*0.25); fBGt=(fCb*0.75+fBGt_1*0.25);
						cvSetReal3D(FGModelt_1,l,m,n,fFGt);
						cvSetReal3D(BGModelt_1,l,m,n,fBGt);
					}

				}
			}
		}
	}
	for( l=0;l<SeqB->total;l++)
	{
		point= *CV_GET_SEQ_ELEM( histpoint, SeqB, l );
		idx=point.idx, i=point.i; j=point.j; k=point.k; count=point.count;
		
		if(i>=2 && i < BGModel->dim[0].size-2 && i>=2 && j < BGModel->dim[1].size-2 && k>=2 && k < BGModel->dim[2].size-2)
		{
			likelihoodMb=cvGetReal3D(BGModel,i,j,k); 
		//	bool normalization=false;
			
			if(likelihoodMb>0.5) 
			for(int l=i-2;l<=i+2;l++)
			for(int m=j-2;m<=j+2;m++)
			for(int n=k-2;n<=k+2;n++)
			{
				likelihoodMold1b=cvGetReal3D(BGModel,l,m,n);
				likelihoodMold2b= likelihoodMb*cvGetReal3D(gaussMat,l-i+2,m-j+2,n-k+2);
				if(likelihoodMold1b<likelihoodMold2b)
				{
					cvSetReal3D(BGModel,l, m, n, likelihoodMold2b);

					fCf= cvGetReal3D(FGModel,l,m,n);;
					fCb=likelihoodMold2b;

					total=fCf+fCb;
					fCf=( fCf/ (total) );
					fCb=( fCb/ (total) );
				
					
					if(0==index)
					{
						cvSetReal3D(FGModelt_1,l,m,n,fCf);
						cvSetReal3D(BGModelt_1,l,m,n,fCb);
					}
					else
					{
						fFGt_1=cvGetReal3D(FGModelt_1,l,m,n);
						fBGt_1=cvGetReal3D(BGModelt_1,l,m,n);

						fFGt=(fCf*0.75+fFGt_1*0.25); fBGt=(fCb*0.75+fBGt_1*0.25);
						cvSetReal3D(FGModelt_1,l,m,n,fFGt);
						cvSetReal3D(BGModelt_1,l,m,n,fBGt);
					}

				}
			}
		}
	}
	
	cvReleaseMatND(&gaussMat);
*/}
void Cues::gaussianSmoothing(CvMatND *ModelMat)
{
	int size[]={gMatSize,gMatSize,gMatSize};
	CvMatND *gaussMat=cvCreateMatND( 3,size, CV_32FC1);
	
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq *seq = cvCreateSeq( CV_SEQ_KIND_GENERIC,sizeof(CvSeq),sizeof(histpoint1),storage);
	histpoint1 point; 

	for(int i=0;i<gMatSize;i++)
	for(int j=0;j<gMatSize;j++)
	for(int k=0;k<gMatSize;k++)
	{
		float fGuassStrength=(float)(exp( -1* ( pow( i-(int) (gMatSize/2),2.0)/ (2.0*gaussSigma*gaussSigma) +
												pow( j-(int) (gMatSize/2),2.0)/ (2.0*gaussSigma*gaussSigma) +
												pow( j-(int) (gMatSize/2),2.0)/ (2.0*gaussSigma*gaussSigma) ) ) );
		cvSetReal3D(gaussMat, i, j, k, fGuassStrength);
	}

	float likelihoodM, likelihoodMold1, likelihoodMold2;
	for(int i=2;i<ModelMat->dim[0].size-2;i++)
	for(int j=2;j<ModelMat->dim[1].size-2;j++)
	for(int k=2;k<ModelMat->dim[2].size-2;k++)
	{
		likelihoodM=cvGetReal3D(ModelMat,i,j,k);
		if(likelihoodM>0.5) 
		{
			for(int l=i-2;l<=i+2;l++)
			for(int m=j-2;m<=j+2;m++)
			for(int n=k-2;n<=k+2;n++)
			{
				likelihoodMold1=cvGetReal3D(ModelMat,l,m,n);
				likelihoodMold2= likelihoodM*cvGetReal3D(gaussMat,l-i+2,m-j+2,n-k+2);
				if(likelihoodMold1<likelihoodMold2)
				{
					point.i=l;point.j=m;point.k=n; point.count=likelihoodMold2;			
					cvSeqPush( seq, &point ); 	
				}//cvSetReal3D(ModelMat,l, m, n, likelihoodMold2);
			}
		}
	}
	for(int i=0;i<seq->total;i++)
	{
		point= *CV_GET_SEQ_ELEM( histpoint1, seq, i );
		cvSetReal3D(ModelMat,point.i,point.j,point.k,point.count);
	}
	cvClearSeq(seq);
	cvReleaseMemStorage(&storage);
	cvReleaseMatND(&gaussMat);
}
void Cues::computeQuality(IplImage *IF_posterior)
{	
//	IplImage *product=cvCreateImage(cvGetSize(IF_posterior),IPL_DEPTH_32F, 1);
	
	int pixCount=0.0;
	float pixSum=0.0,postPix,likePix,avgDiff;
	for(int i=0;i<IF_posterior->height;i++)
	for(int j=0;j<IF_posterior->width;j++)	
	{	
		postPix=getPixel32F(IF_posterior,i,j,0);
		if(postPix>=0.5)
		{
			likePix=getPixel32F(IF_Likelihood,i,j,0);
			pixSum+= ( (postPix-likePix) *(postPix-likePix));
			pixCount++;
		}
	}
	if(0!=pixCount)
		avgDiff=pixSum/(float)pixCount;
	else avgDiff=0.0;
	fQuality=(1.0-sqrt(avgDiff));
	

//	showImg(IF_posterior,"Fpost");
//	showImg(IF_Likelihood,"likelihood");
/*	cvSub(IF_posterior,IF_Likelihood,product);
	cvMul(product,product,product,1);


	CvScalar meanvalue=cvAvg(product);
	fQuality=(1.0-sqrt(meanvalue.val[0]));
*/	
//	cvReleaseImage(&product);
	return;
}

void postProcessing(IplImage *IF_posterior)
{
	int i;
	IplImage *IBinary=cvCreateImage(cvGetSize(IF_posterior),IPL_DEPTH_8U, 1);
	cvSetZero(IBinary);

//	showImg(IF_Likelihood,"if");
	cvThreshold( IF_posterior, IBinary, 0.35,255,CV_THRESH_BINARY );
//	showImg(IF_Likelihood,"likelihoodComp");

/*	for(int	i=0;i<IBinary->height;i++)
	for(int j=0;j<IBinary->width; j++)
	{
		float pixVal=getPixel32F(IF_Likelihood, i, j, 0);
		if(pixVal>0.4)
			setPixel(IBinary,i,j,255,0);
		
	}
*/	
	cvDilate( IBinary, IBinary,NULL, 1 );
	cvErode ( IBinary, IBinary,NULL, 1 );
	
	cvErode ( IBinary, IBinary,NULL, 1 );
	cvDilate( IBinary, IBinary,NULL, 1 );
	
	
	
	IplImage *temp=cvCloneImage(IBinary);
	//	showImg(temp,"temp");
	
	// to find all connected components in the image 
	int NoConnComp = 0;
	int Connectivity = 8;
	CvConnectedComp CComp;

	for(	i=0;i<IBinary->width;i++)
	for(int j = 0;j<IBinary->height;j++)
	{
		CvPoint pt = {i,j};
		uchar* temp_ptr = &((uchar*)(IBinary->imageData + IBinary->widthStep*pt.y))[pt.x];
		if(temp_ptr[0]==255)
		{ 
			cvFloodFill(IBinary,pt, cvScalar((NoConnComp+1)*2), cvScalarAll(0), cvScalarAll(0),&(CComp), Connectivity, NULL );
		//	if( ( (float)CComp.rect.height/(float)CComp.rect.width) >=0.5 && ( (float)CComp.rect.height/(float)CComp.rect.width) < 2.0 && CComp.area>900 && ( ( (float)CComp.area/(float)(CComp.rect.height*CComp.rect.width) ) >0.4 ) ) // considering at least 30x30 pixel size object
			if( CComp.area>200 ) // considering at least 30x30 pixel size object
			{
				NoConnComp++;
			}
			else
			{
				cvSetImageROI(temp,CComp.rect);
				cvSetZero(temp);
				cvResetImageROI(temp);
			}
		//	showImg(temp,"temp");
		
		}
		
	}
	
//	showImg(temp,"temp");

	for( 	i=0;i<IBinary->height;i++)
	for(int j=0;j<IBinary->width; j++)
	{
		float pixVal=getPixel(temp, i, j, 0);
		if(pixVal<=0)
			setPixel32F(IF_posterior,i,j,0.0,0);
//		if(pixVal>0)
//			;//setPixel32F(IF_posterior,i,j,1.0,0);
//		else setPixel32F(IF_posterior,i,j,0.0,0);
		
	}
	

	cvReleaseImage(&temp);

	cvReleaseImage(&IBinary);
}

bool Cue_integration::bObjAppearance(IplImage *IF_Likelihood)
{
	IplImage *IBinary=cvCreateImage(cvGetSize(IF_Likelihood),IPL_DEPTH_8U, 1);
	cvSetZero(IBinary);

//	showImg(IF_Likelihood,"if");
//	cvThreshold( IF_Likelihood, IF_Likelihood, 0.4,1.0,CV_THRESH_BINARY );
//	showImg(IF_Likelihood,"likelihoodComp");

	for(int	i=0;i<IBinary->height;i++)
	for(int j=0;j<IBinary->width; j++)
	{
		float pixVal=getPixel32F(IF_Likelihood, i, j, 0);
		if(pixVal>0.4)
			setPixel(IBinary,i,j,255,0);
		
	}
	
//	cvErode ( IBinary, IBinary,NULL, 1 );
//	cvDilate( IBinary, IBinary,NULL, 1 );
		
	cvSmooth(IBinary, IBinary,CV_MEDIAN,3,3);
	cvThreshold(IBinary, IBinary,100,255, CV_THRESH_BINARY);
//	IplImage *temp=cvCloneImage(IBinary);

	//	showImg(IBinary,"binary");
	/* to find all connected components in the image */
	int NoConnComp = 0;
	int Connectivity = 8;
	CvConnectedComp CComp;

	for(int i=0;i<IBinary->width;i++)
	for(int j = 0;j<IBinary->height;j++)
	{
		CvPoint pt = {i,j};
		uchar* temp_ptr = &((uchar*)(IBinary->imageData + IBinary->widthStep*pt.y))[pt.x];
		if(temp_ptr[0]==255)
		{ 
			cvFloodFill(IBinary,pt, cvScalar((NoConnComp+1)*2), cvScalarAll(0), cvScalarAll(0),&(CComp), Connectivity, NULL );
 			if(CComp.area>200 && ( ( (float)CComp.area/(float)(CComp.rect.height*CComp.rect.width) ) >0.4 ) ) // considering at least 30x30 pixel size object
			{
				NoConnComp++;
				break;
			}
		/*	else
			{
				cvSetImageROI(temp,CComp.rect);
				cvSetZero(temp);
				cvResetImageROI(temp);
			}
		//	showImg(IBinary,"TB");
		*/
		}
		
	}
//	showImg(temp,"temp");
/*	
	for( 	i=0;i<IBinary->height;i++)
	for(int j=0;j<IBinary->width; j++)
	{
		float pixVal=getPixel(temp, i, j, 0);
		if(pixVal>0)
			setPixel32F(IF_Likelihood,i,j,1.0,0);
		else setPixel32F(IF_Likelihood,i,j,0.0,0);
		
	}
	
*/	
//	showImg(IF_Likelihood,"likelihoodComp");
	
	cvReleaseImage(&IBinary);
//	cvReleaseImage(&temp);
	if(NoConnComp>0) return true;
	else return false;

}
void Cues::clearAllocation()
{	
	cvReleaseImage(&IF_Likelihood);
	cvReleaseImage(&IB_Likelihood);
}
void Cue_integration::integratedPosteriorProbs()
{
//	float likelihood;
	char name[100];
	cvSet(IF_posterior,cvScalar(1.0));
	cvSet(IB_posterior,cvScalar(1.0));

	IplImage *optImage=cvCreateImage(cvGetSize(IF_posterior),IPL_DEPTH_32F, 1);
	
	for(int iCue=0;iCue<iNoof_Cues;iCue++)
	{
//		sprintf(name,"%d -Flikelihood w:%f",iCue,Cue[iCue].fCueWeight);
		sprintf(name,"%d -Flikelihood",iCue);
	//	showImg(Cue[iCue].IF_Likelihood,name);

	//	sprintf(name,"%d -Blikelihood",iCue);
	//	showImg(Cue[iCue].IB_Likelihood,name);


 		cvPow(Cue[iCue].IF_Likelihood,optImage,Cue[iCue].fCueWeight);
		cvMul(IF_posterior,optImage,IF_posterior);
		
		sprintf(name,"%d -Flikelihood",iCue);
	//	showImg(optImage,name);

		

		cvPow(Cue[iCue].IB_Likelihood,optImage,Cue[iCue].fCueWeight);
		cvMul(IB_posterior,optImage,IB_posterior);

	}

	cvAdd(IF_posterior,IB_posterior,optImage);
	cvDiv(IF_posterior,optImage,IF_posterior);
	cvDiv(IB_posterior,optImage,IB_posterior);
	
	cvReleaseImage(&optImage);

//	postProcessing(IF_posterior);

//	cvThreshold( IF_posterior, IF_posterior, 0.5,1.0,CV_THRESH_BINARY );

//	showImg(IF_posterior,"FGpost");
//	showImg(IB_posterior,"BGpost");
	
//	cvMul(Cue[iCue].fCueWeight

/*	for(int i=0;i<iRows;i++)
	for(int j=0;j<iCols;j++)
	{
		float fposterior=(float)1.0, bposterior=(float)1.0,total;

		fposterior=getPixel32F(IF_posterior,i,j,0);
		bposterior=getPixel32F(IB_posterior,i,j,0);
*//*		for(int iCue=0;iCue<iNoof_Cues;iCue++)
		{
			//foreground posterior
			flikelihood=getPixel32F(Cue[iCue].IF_Likelihood,i,j,0);
			flikelihood*=pow(likelihood,Cue[iCue].fCueWeight);

			//Background posterior
			blikelihood=getPixel32F(Cue[iCue].IB_Likelihood,i,j,0);
			blikelihood*=pow(likelihood,Cue[iCue].fCueWeight);
		}

*/
/*		//Nomalization of posterior on layer l: f or b
		// Alpha multiplicatoin 6.1 and 6.2
		total=fposterior+bposterior;
		fposterior/=total;
		bposterior/=total;

		setPixel32F(IF_posterior,i,j,fposterior,0);
		setPixel32F(IB_posterior,i,j,bposterior,0);
	}
*/ //	showImg(IF_posterior,"ifpost");
}



void Cue_integration::initialize(IplImage *image)
{

	int i;
	iNoofB_Cues=Noof_B_Cues; // uguale a 2
	iNoofT_Cues=Noof_T_Cues; // uguale a 1
	iNoof_Cues=Noof_Cues;	// uguale a Noof_B_Cues + Noof_T_Cues

	//initialization of the object rectangles
	for(i=0;i<4;i++)
	{
		Obj_rect_t[i]=0;
		Obj_rect_t_1[i]=0;
	}

	iRows=image->height;
	iCols=image->width;
//	Iinputframe=image;
	IF_posterior=cvCreateImage(cvGetSize(image),IPL_DEPTH_32F, 1);
	IB_posterior=cvCreateImage(cvGetSize(image),IPL_DEPTH_32F, 1);
//	IouputBinary=cvCreateImage(cvGetSize(image),IPL_DEPTH_8U, 1);
	IgrayDisplay=cvCreateImage(cvGetSize(image),IPL_DEPTH_8U, 1);

	int size[]={colorHistlength,colorHistlength,colorHistlength};
	FGColorModel=cvCreateMatND( 3,size, CV_32FC1);
	BGColorModel=cvCreateMatND( 3,size, CV_32FC1);

	cvSetZero(FGColorModel);
	cvSetZero(BGColorModel);

	int size1[]={textureHistlength,textureHistlength,textureHistlength};
	FGTextureModel=cvCreateMatND( 3,size1, CV_32FC1);
	BGTextureModel=cvCreateMatND( 3,size1, CV_32FC1);

	cvSetZero(FGTextureModel);
	cvSetZero(BGTextureModel);

//	Obj_Kt=cvCreateMat(iRows,iCols,

	strgObj = cvCreateMemStorage(0);
	ObjtsSeq=cvCreateSeq( CV_SEQ_KIND_GENERIC,sizeof(CvSeq),sizeof(Object),strgObj);

/*
//	CvMemStorage* storage = cvCreateMemStorage(0);
//	ColorSeqF = cvCreateSeq( CV_SEQ_KIND_GENERIC,sizeof(CvSeq),sizeof(histpoint),storage);
	ColorSeqF = cvCreateSeq( CV_32SC1,sizeof(CvSeq),sizeof(int),storage);
	
	CvMemStorage* storage1 = cvCreateMemStorage(0);
//	ColorSeqB = cvCreateSeq( CV_SEQ_KIND_GENERIC,sizeof(CvSeq),sizeof(histpoint),storage1);
	ColorSeqB = cvCreateSeq( CV_32SC1,sizeof(CvSeq),sizeof(int),storage1);
	
	//int iSeqLength=(image->height/iGridSize)*(image->width/iGridSize);
		
	for( i=0;i<iSeqLength;i++)
	{
		storgearray1[i] = cvCreateMemStorage(0);               
		Obj_Kt[i]= cvCreateSeq( CV_SEQ_KIND_GENERIC,sizeof(CvSeq),sizeof(objFeature),storgearray1[i]);

		storgearray2[i] = cvCreateMemStorage(0);               
		Obj_Kt_1[i]= cvCreateSeq( CV_SEQ_KIND_GENERIC,sizeof(CvSeq),sizeof(objFeature),storgearray2[i]);
	}
*/	
	for(	i=0;i<objRows;i++)
	for(int j=0;j<objCols;j++)
	{
		ObjKt[i][j]=NULL;
		ObjKt_1[i][j]=NULL;
	}	

	for(int iCue=0;iCue<iNoof_Cues;iCue++)
	{
		if(0==iCue) Cue[0].fCueWeight=Noof_Cues;
		else Cue[iCue].fCueWeight=0.0;
		Cue[iCue].IF_Likelihood=cvCreateImage(cvGetSize(image),IPL_DEPTH_32F, 1);
		Cue[iCue].IB_Likelihood=cvCreateImage(cvGetSize(image),IPL_DEPTH_32F, 1);
	}

}
/* 
Object::Object()
{
	iInitializationCount=0;
	for(int	i=0;i<objRows;i++)
	for(int j=0;j<objCols;j++)
	{
		ObjKt_1[i][j]=NULL;
	}
}
Object::~Object()
{
	for(int	i=0;i<objRows;i++)
	for(int j=0;j<objCols;j++)
	{

		if(NULL!=ObjKt_1[i][j])
			free(ObjKt_1[i][j]);
	}

}

*/  
void Cue_integration::clearAllocations()
{

	int i;
	cvReleaseImage(&IF_posterior);
	cvReleaseImage(&IB_posterior);
//	cvReleaseImage(&IouputBinary);
//	cvReleaseImage(&IBGModel);
	clearBgModel(IBGModel);
	cvReleaseMatND(&FGColorModel);
	cvReleaseMatND(&BGColorModel);

	cvReleaseMatND(&FGTextureModel);
	cvReleaseMatND(&BGTextureModel);

	cvReleaseImage(&IgrayDisplay);

//	cvClearSeq(ColorSeqF );
//	cvClearSeq(ColorSeqB);

	for(int iCue=0;iCue<iNoof_Cues;iCue++)
		Cue[iCue].clearAllocation();

	cvClearSeq(ObjtsSeq);
	cvReleaseMemStorage( &strgObj);
/*	
		
	for( i=0;i<iSeqLength;i++)
	{
		cvClearSeq(Obj_Kt[i] );
		cvReleaseMemStorage( &storgearray1[i]);

		cvClearSeq(Obj_Kt_1[i] );
		cvReleaseMemStorage( &storgearray2[i]);
	}

*/	for(	i=0;i<objRows;i++)
	for(int j=0;j<objCols;j++)
	{
		if(NULL!=ObjKt[i][j])
			free(ObjKt[i][j]);

		if(NULL!=ObjKt_1[i][j])
			free(ObjKt_1[i][j]);
	}	

}
	

void Cue_integration::updateBg(IplImage *IBgModel[],IplImage *frame,int iIndex, IplImage *IF_posterior)
{	
	int i=0;
	if(iIndex<iBgModelSize)
		IBgModel[iIndex]=cvCloneImage(frame);
	else if(iIndex==iBgModelSize)
	{
		cvReleaseImage(&IBgModel[(int)(iIndex%iBgModelSize)]);
		
		
		IBgModel[(int)(iIndex%iBgModelSize)]=cvCloneImage(frame);
		
		for(i=0;i<iBgModelSize;i++)
		{
/*		 printf("\n model idx %d", i);
		showImg(IBgModel[i],"model");
		cvWaitKey();
*/
printf("\n modelUpdation %d",(iIndex));
    //	cvAddWeighted(IBgModel[iBgModelSize],(0==i?0.0:1.0),IBgModel[i],(double)(1.0/(float)iBgModelSize),0.0,IBgModel[iBgModelSize]);
			cvAddWeighted(IBgModel[iBgModelSize],(0.9),IBgModel[i],(0.1),0.0,IBgModel[iBgModelSize]);

		}

	}
	else
	{
		
		cvReleaseImage(&IBgModel[(int)(iIndex%iBgModelSize)]);
		IBgModel[(int)(iIndex%iBgModelSize)]=cvCloneImage(frame);

		IplImage *IBg=cvCloneImage(IBgModel[iBgModelSize]);

		cvAddWeighted(IBgModel[iBgModelSize],(0.9),IBgModel[(int)(iIndex%iBgModelSize)],(0.1),0.0,IBgModel[iBgModelSize]);
	
		int minX=999,maxX=0, minY=999, maxY=0;
		 
		float fPixVal;
		CvScalar avt,avt_2;
		for(int i=2;i<frame->height-2;i++)
		for(int j=2;j<frame->width-2;j++)
		{
			fPixVal=getPixel32F(IF_posterior,i,j,0);
			if(fPixVal>=0.5)
			{
				
				cvSetImageROI(IBgModel[(int)(iIndex%iBgModelSize)],cvRect(j-2,i-2,5,5));
 				avt=cvAvg(IBgModel[(int)(iIndex%iBgModelSize)]);
				cvResetImageROI(IBgModel[(int)(iIndex%iBgModelSize)]);

				cvSetImageROI(IBgModel[(int)((iIndex-2)%iBgModelSize)],cvRect(j-2,i-2,5,5));
 				avt_2=cvAvg(IBgModel[(int)((iIndex-2)%iBgModelSize)]);
				cvResetImageROI(IBgModel[(int)((iIndex-2)%iBgModelSize)]);

				float sumAvgDiff=0.0;

				sumAvgDiff= fabs(avt.val[0]-avt_2.val[0])+fabs(avt.val[1]-avt_2.val[1])+fabs(avt.val[2]-avt_2.val[2]);
		
			 	if(sumAvgDiff>10)
				{
					int b=(int)getPixel(IBg,i,j,0);
					int g=(int)getPixel(IBg,i,j,1);
					int r=(int)getPixel(IBg,i,j,2);
				
					setPixel(IBgModel[iBgModelSize],i,j,b,0);
					setPixel(IBgModel[iBgModelSize],i,j,g,1);
					setPixel(IBgModel[iBgModelSize],i,j,r,2);
				}
			}
		}
		
		cvReleaseImage(&IBg);
			
		
/*		IplImage *IBg=cvCloneImage(IBgModel[iBgModelSize]);
		cvAddWeighted(IBgModel[iBgModelSize],0.6,frame,0.4,0,IBgModel[iBgModelSize]);

		float fPixVal;
		CvScalar av;
		for(int i=0;i<frame->height;i+=5)
		for(int j=0;j<frame->width;j+=5)
		{
			//fPixVal=getPixel32F(IF_posterior,i,j,0);
			//if(fPixVal>=0.5)
			cvSetImageROI(IF_posterior,cvRect(j,i,5,5));
 			av=cvAvg(IF_posterior);
			cvResetImageROI(IF_posterior);
			int avggray=(int)av.val[0];
			if(avggray>=0.5)
			{
				int b=(int)getPixel(IBg,i,j,0);
				int g=(int)getPixel(IBg,i,j,1);
				int r=(int)getPixel(IBg,i,j,2);
			
				setPixel(IBgModel[iBgModelSize],i,j,b,0);
				setPixel(IBgModel[iBgModelSize],i,j,g,1);
				setPixel(IBgModel[iBgModelSize],i,j,r,2);
			}

		}
		cvReleaseImage(&IBg);
	*/
	}

//	showImg(IBgModel[iBgModelSize],"BG");
	return; 	
}
void Cue_integration::clearBgModel(IplImage *IBgModel[])
{
	for(int i=0;i<iBgModelSize+1;i++)
		cvReleaseImage(&IBgModel[i]);
	return;	
}

void Normalize(IplImage *IFggray,IplImage *dst, int a, int b)
{
	int imageMin=999,imageMax=0;
	for(int i=0;i<IFggray->height;i++)
	for(int j=0;j<IFggray->width;j++)

	{
		int temp=(int) getPixel(IFggray,i,j,0);
		if(temp>imageMax) imageMax=temp;
		if(temp<imageMin) imageMin=temp;
	}

	imageMax=(int) (0.9*(float) imageMax);
	imageMin=(int) (0.1*(float) (imageMax-imageMin)+imageMin);

	for(int i=0;i<IFggray->height;i++)
	for(int j=0;j<IFggray->width;j++)
	{
		int temp=(int) getPixel(IFggray,i,j,0);
		if(temp<=imageMin)
			temp=0;
		else
		{	
			temp= (int) ((float) a+(float)(temp-imageMin)*(b-a)/(float)(imageMax-imageMin));
		}
			setPixel(IFggray,i,j,temp,0);
		
	}

	return;
}

void Normalize32F(IplImage *I32F,IplImage *gray, int a, int b)
{
	float imageMin=999.0,imageMax=-999.0;
	for(int i=0;i<I32F->height;i++)
	for(int j=0;j<I32F->width;j++)

	{
		float temp= getPixel32F(I32F,i,j,0);
		if(temp>imageMax) imageMax=temp;
		if(temp<imageMin) imageMin=temp;
	}

//	imageMax= (0.9*(float) imageMax);
//	imageMin= (0.1*(float) (imageMax-imageMin)+imageMin);

	for(int i=0;i<I32F->height;i++)
	for(int j=0;j<I32F->width;j++)
	{
		float temp=getPixel32F(I32F,i,j,0);
		int pixvalue;
		if(temp<=imageMin)
			pixvalue=0;
		else
		{	
			pixvalue= (int) ((float) a+(float)(temp-imageMin)*(float)(b-a)/(float)(imageMax-imageMin));
		}
		setPixel(gray,i,j,(pixvalue)/*>30?255:0)*/,0);
		
	}

	// Binarization of the normalized Image

	/*
	CvScalar av;
	for( i=0;i<I32F->height-4;i+=4)
	for(int j=0;j<I32F->width-4;j+=4)
	{
		cvSetImageROI(gray,cvRect(j,i,4,4));
 		av=cvAvg(gray);
		int avggray=(int)av.val[0];
		if(avggray>50)
			cvSet(gray, cvScalar(255));
		else cvSet(gray, cvScalar(0));

		cvResetImageROI(gray);		
	}
	*/
	return;
}


