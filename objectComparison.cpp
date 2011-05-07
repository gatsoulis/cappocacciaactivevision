#include "objectComparison.h"


objectCompare::objectCompare(std::string name) : Module(name) {

	//salimg = cvCreateImage(cvSize(320,240),IPL_DEPTH_32F,1);
	salimg = cvCreateImage(cvSize(320,240),IPL_DEPTH_8U,3);
	regionsInCamera.setModule(this);
	regionsInDatabase.setModule(this);
	count = 0;

	debug = false;

	//std::cout << "finished.\n";
}

objectCompare::~objectCompare() {
	//std::cout << "Destructing " << getName() << "...";
	//std::cout << "finished.\n";
}
void objectCompare::release(){
	free(array);
}
void objectCompare::saveImg() {
	//cvgrayimg = new CVImage(cvSize(320, 240), CV_8UC1, 0);
	cvgrayimg = cvGrayImageIn.getBuffer();
	inputImage2 = cvgrayimg->ipl;
	/*cvNamedWindow("display");
	cvShowImage("display",inputImage);
	cvWaitKey(2);*/
	char temp[150];
	sprintf(temp,"/home/icub/Downloads/4objects/save%d.jpg",1000+count);
	cvSaveImage(temp,cvgrayimg->ipl);
	//savedImages.push_back(cvgrayimg);
	count++;

}
void objectCompare::saveRect(CvRect savRect) {
	savedRectangles.push_back(savRect);
}
void objectCompare::readImg(int itr) {

	char temp[150];
	sprintf(temp,"/home/icub/Downloads/4objects/save%d.jpg",1000+itr);
	inputImage = cvLoadImage(temp,0);

		//image2 = new CVImage( cvSize(320, 240), CV_32FC1, 0);
		image2 = convert2CVImageLeft(inputImage);
		image2->ipl=inputImage;
/*	IplImage* inputImage2 = savedImages[itr]->ipl;
		cvNamedWindow("display");
			cvShowImage("display",inputImage2);
			cvWaitKey(2);*/

		cvsavedImageOut.setBuffer(image2);
		cvsavedImageOut.out();
		iterationObject=itr;

}

void objectCompare::execute() {

	CSCEllipticRegions* regionDatabase = regionsInDatabase.getBuffer();
	CSCEllipticRegions* regionCamera = regionsInCamera.getBuffer();

	countMatching=0;
	if(!regionDatabase) {
		std::cout <<"\nNO REGION on Database\n";
		return;
	}
	if(!regionCamera) {
		std::cout <<"\nNO REGION on Camera\n";
		return;
	}

	//unsigned int heightCamera = regionCamera->getCurrentSize();
	unsigned int widthCamera = regionCamera->getWidth();
	float* dataCamera = regionCamera->getData();
	objLocX=0;
	objLocY=0;

		unsigned int heightDatabase = regionDatabase->getCurrentSize();
		unsigned int widthDatabase = regionDatabase->getWidth();
		float* dataDatabase = regionDatabase->getData();
		for(unsigned int i = 0;i<heightDatabase;i++) {
			float posxDatabase = dataDatabase[i*widthDatabase];
			float posyDatabase = dataDatabase[i*widthDatabase+1];
			float posxCamera = dataCamera[i*widthCamera];
			float posyCamera = dataCamera[i*widthCamera+1];
			if(posxDatabase>savedRectangles[iterationObject].x && posxDatabase<savedRectangles[iterationObject].x + savedRectangles[iterationObject].width && posyDatabase>savedRectangles[iterationObject].y && posyDatabase<savedRectangles[iterationObject].y + savedRectangles[iterationObject].height){
				countMatching++;
				objLocX+=posxCamera;
				objLocY+=posyCamera;
			}
		}
		objLocX/=countMatching;
		objLocY/=countMatching;

		//cvSetImageROI()
/*		CVImage* cameraInput = cvGrayImageIn.getBuffer();
		IplImage* cameraIpl = cvCloneImage(cameraInput->ipl);
		double *detectionQuality;
		IplImage* colorMatchSaliency = histBackProject(object_color,cameraIpl,detectionQuality,1);*/

		/*for(int i = 0; i<2*height;i++)
		{
			std::cout<<getName()<<"::array["<<i<<"]="<<array[i]<<"\n";

		}*/
		//arrayOut.setBuffer(array);
		//arrayOut.out();
		detectionBlob(iterationObject);
	}

void objectCompare::detectionBlob(int iterationObjectSal) {

		CvPoint newwinnerpos;
	    static CvScalar colors[] =
	    {
	        {{0,0,255}},
	        {{0,128,255}},
	        {{0,255,255}},
	        {{0,255,0}},
	        {{255,128,0}},
	        {{255,255,0}},
	        {{255,0,0}},
	        {{255,0,255}},
	        {{255,255,255}}
	    };

		newwinnerpos.x = objLocX;
		newwinnerpos.y = objLocY;

			cvSetZero(salimg);
			int m_windowSize = 75,m_size=25*25;
			for(int x = -m_windowSize; x <= m_windowSize; ++x)
			{
				for(int y = -m_windowSize; y <= m_windowSize; ++y)
				{
					int posx = newwinnerpos.x + x;
					int posy = newwinnerpos.y + y;
					if(posx >= 0 && posy >= 0 && posx < 320 && posy < 240)
					{
						//((float*)(mp_cvSalImage->ipl->imageData))[(m_newWinnerData.winnerPos.x+x)+m_width*(m_newWinnerData.winnerPos.y+y)] = exp(-0.5/m_size*(x*x+y*y));
						double val = exp(-0.5/m_size*(x*x+y*y));

						/*switch(iterationObjectSal)
						{
						case 0: b=cvRound(val*128);
						break;
						case 1: g=cvRound(val*128);
						break;
						case 2: r=cvRound(val*128);
						break;
						}*/
						(salimg->imageData + posy*salimg->widthStep)[posx*salimg->nChannels+0]=cvRound(colors[iterationObjectSal].val[0]*val);
						(salimg->imageData + posy*salimg->widthStep)[posx*salimg->nChannels+1]=cvRound(colors[iterationObjectSal].val[1]*val);//cvRound(val*128);
						(salimg->imageData + posy*salimg->widthStep)[posx*salimg->nChannels+2]=cvRound(colors[iterationObjectSal].val[2]*val);//cvRound(val*128);
					}
				}
			}
}

CVImage* objectCompare::convert2CVImageLeft(IplImage* InputImage) {


	IplImage* tempimg = cvCloneImage(InputImage);

	if(!tempimg) { std::cerr<< getName() << "::convert2CVImage::ERROR opening file\n"; return NULL; }

/*	if(cvi) {
		if(cvi->width != tempimg->width || cvi->height != tempimg->height || cvi->ipl->nChannels != tempimg->nChannels || cvi->ipl->depth != tempimg->depth) {
			delete cvi;
			cvi = NULL;
		}
	}*/
	cvi = NULL;

	if(!cvi) {

		int cvMatType;
		if(tempimg->nChannels == 3){
			cvMatType = CV_8UC3;
		}
		else if(tempimg->nChannels == 1) {
			cvMatType = CV_8UC1;
		}

		cvi = new CVImage(cvSize(tempimg->width, tempimg->height), cvMatType, 0);
		if(!cvi) { std::cout<< getName() << "::convert2CVImage::ERROR::Failed to create new CVImage\n"; return NULL; }
	}

	int datasize = (tempimg->widthStep)*(tempimg->height);
	memcpy(cvi->ipl->imageData, tempimg->imageData, datasize );

	cvReleaseImage(&tempimg);
	if(tempimg) { std::cout<< getName() << "::convert2CVImage::ERROR::Failed to release IplImage\n"; return NULL; }

	//if(debug) { std::cout << "LoadCVImage::load()::finished\n"; }
	return cvi;
}

/*void objectCompare::detectionBlob(int iterationObjectSal) {

		CvPoint newwinnerpos;

		newwinnerpos.x = objLocX;
		newwinnerpos.y = objLocY;

			cvSetZero(salimg);
			int m_windowSize = 50,m_size=10*10;
			for(int x = -m_windowSize; x <= m_windowSize; ++x)
			{
				for(int y = -m_windowSize; y <= m_windowSize; ++y)
				{
					int posx = newwinnerpos.x + x;
					int posy = newwinnerpos.y + y;
					if(posx >= 0 && posy >= 0 && posx < 320 && posy < 240)
					{
						//((float*)(mp_cvSalImage->ipl->imageData))[(m_newWinnerData.winnerPos.x+x)+m_width*(m_newWinnerData.winnerPos.y+y)] = exp(-0.5/m_size*(x*x+y*y));
						double val = exp(-0.5/m_size*(x*x+y*y));
						cvSetReal2D(salimg, posy, posx, val);
					}
				}
			}
}*/
