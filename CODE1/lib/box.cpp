#include "box.h"

using namespace std;

Box::Box(std::string name) : Module(name) {
	std::cout << "\nConstructing " << getName() << "...";
	
	cvImageInLeft.setModule(this);
	cvImageInRight.setModule(this);
	regionsInLeft.setModule(this);
	regionsInRight.setModule(this);
	
	newFrameRGBLeft=NULL;
	newFrameRGBRight=NULL;
	newFrameGrayLeft=NULL;
	newFrameGrayRight=NULL;
	outputImageLeft=NULL;
	outputImageRight=NULL;
	sideBySideImage=NULL;

	cvimgleft=NULL;
	cvimgright=NULL;
	regionLeft=NULL;
	regionRight=NULL;

	debug = true;
	
	std::cout << "finished.";
}

Box::~Box() {
	std::cout << "\nDestructing " << getName() << "...";
	
	cvReleaseImage(&newFrameRGBLeft);
	cvReleaseImage(&newFrameRGBRight);
	cvReleaseImage(&newFrameGrayLeft);
	cvReleaseImage(&newFrameGrayRight);
	cvReleaseImage(&outputImageLeft);
	cvReleaseImage(&outputImageRight);
	cvReleaseImage(&sideBySideImage);

/*	if(cvimgleft) delete cvimgleft;
	if(cvimgright) delete cvimgright;
	if(regionLeft) delete regionLeft;
	if(regionRight) delete regionRight;
*/
	std::cout << "finished.";
}

// Initializes the application
void Box::memoryall(){
	
	// Memory Allocations
	newFrameGrayLeft = cvCreateImage(cvSize(FRAME_WIDTH, FRAME_HEIGHT), IPL_DEPTH_8U, 1);
	newFrameGrayRight = cvCreateImage(cvSize(FRAME_WIDTH, FRAME_HEIGHT), IPL_DEPTH_8U, 1);
	outputImageLeft = cvCreateImage(cvSize(FRAME_WIDTH, FRAME_HEIGHT), IPL_DEPTH_8U, 3);
	outputImageRight = cvCreateImage(cvSize(FRAME_WIDTH, FRAME_HEIGHT), IPL_DEPTH_8U, 3);
	sideBySideImage = cvCreateImage(cvSize(2 * FRAME_WIDTH, FRAME_HEIGHT), IPL_DEPTH_8U, 3);
	
}

void Box::createwin()
{
	if (debug)	
		cvNamedWindow("WINDOW", 0);

}

int Box::moveWindow(int x, int y) {
//	int width = imageIn.getBuffer()->width;
	cvMoveWindow("WINDOW", x, y);
	return 0;
}

// Main function of the class
void Box::execute(){
	IplImage* resultLeft = outputImageLeft;
	IplImage* resultRight = outputImageRight;
	takeNewFrame();
	cvCopyImage(newFrameRGBLeft, outputImageRight);
	cvCopyImage(newFrameRGBRight, outputImageRight);
	putImagesSideBySide(sideBySideImage, newFrameRGBLeft, newFrameRGBRight);
    //	doDetection();
	if (debug){	
		showKeypoints();
		resultLeft = sideBySideImage;
		cvShowImage("WINDOW",resultLeft);
		cvWaitKey(33);
	}
	//cvReleaseImage(&resultLeft);
	//cvReleaseImage(&resultRight);


}

// Captures a new frame. Returns if capture is taken without problem or not.
bool Box::takeNewFrame()
{
	cvimgleft = cvImageInLeft.getBuffer();
	cvimgright = cvImageInRight.getBuffer();
	if ((newFrameRGBLeft = cvimgleft->ipl) &&(newFrameRGBRight = cvimgright->ipl))
	{
		cvCvtColor(newFrameRGBLeft, newFrameGrayLeft, CV_BGR2GRAY);
		cvCvtColor(newFrameRGBRight, newFrameGrayRight, CV_BGR2GRAY);
	}
	else
		return false;
	return true;
}

void Box::destroywin(){
	if (debug)
		cvDestroyWindow("WINDOW");

}

// Puts img1 and img2 side by side and stores into result
void Box::putImagesSideBySide(IplImage* result, const IplImage* img1, const IplImage* img2)
{
	// widthStep of the resulting image
	const int bigWS = result->widthStep;
	// half of the widthStep of the resulting image
	const int bigHalfWS = result->widthStep >> 1;
	// widthStep of the image which will be put in the left
	const int lWS = img1->widthStep;
	// widthStep of the image which will be put in the right
	const int rWS = img2->widthStep;

	// pointer to the beginning of the left image
	char *p_big = result->imageData;
	// pointer to the beginning of the right image
	char *p_bigMiddle = result->imageData + bigHalfWS;
	// pointer to the image data which will be put in the left
	const char *p_l = img1->imageData;
	// pointer to the image data which will be put in the right
	const char *p_r = img2->imageData;

	for (int i = 0; i < FRAME_HEIGHT; ++i, p_big += bigWS, p_bigMiddle += bigWS) {
		// copy a row of the left image till the half of the resulting image
		memcpy(p_big, p_l + i*lWS, lWS);
		// copy a row of the right image from the half of the resulting image to the end of it
		memcpy(p_bigMiddle, p_r + i*rWS, rWS);
	}
}

// Marks the keypoints with plus signs on img
void Box::showKeypoints()
{

	regionLeft = regionsInLeft.getBuffer();
	regionRight = regionsInRight.getBuffer();
	unsigned int heightLeft = regionLeft->getCurrentSize();
	unsigned int widthLeft = regionLeft->getWidth();
	unsigned int heightRight = regionRight->getCurrentSize();
	unsigned int widthRight = regionRight->getWidth();
	float* dataLeft = regionLeft->getData();
	float* dataRight = regionRight->getData();

	for(unsigned int i = 0;i<heightLeft;i++) {
		float posxLeft = dataLeft[i*widthLeft];
		float posyLeft = dataLeft[i*widthLeft+1];
		float posxRight = dataRight[i*widthRight] + 320;
		float posyRight = dataRight[i*widthRight+1];
		drawAPlusandLine(posxLeft,posyLeft,posxRight,posyRight);
	}
	cvCircle(sideBySideImage,cvPoint(160,120),70,CV_RGB(0,0,255));
	cvCircle(sideBySideImage,cvPoint(160,120),3,CV_RGB(0,0,255),2);	
	cvCircle(sideBySideImage,cvPoint(160+320,120),70,CV_RGB(0,0,255));
	cvCircle(sideBySideImage,cvPoint(160+320,120),3,CV_RGB(0,0,255),2);

}

// Draws a plus sign on img given (x, y) coordinate
void Box::drawAPlusandLine(const int xL, const int yL, const int xR, const int yR)
{

  // Draw a line between matching keypoints
  cvLine(sideBySideImage, cvPoint(xL, yL), cvPoint(xR,yR), cvScalar(0, 255, 0), 0.5);

  cvLine(sideBySideImage, cvPoint(xL - 5, yL), cvPoint(xL + 5, yL), CV_RGB(255, 0, 0));
  cvLine(sideBySideImage, cvPoint(xL, yL - 5), cvPoint(xL, yL + 5), CV_RGB(255, 0, 0));
  cvLine(sideBySideImage, cvPoint(xR - 5, yR), cvPoint(xR + 5, yR), CV_RGB(255, 0, 0));
  cvLine(sideBySideImage, cvPoint(xR, yR - 5), cvPoint(xR, yR + 5), CV_RGB(255, 0, 0));


}
