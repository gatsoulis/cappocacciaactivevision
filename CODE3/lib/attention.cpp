#include "attention.h"
//#include "BRIEF.h"


enum APPLICATION_MODEL
  {
    DO_NOTHINGL, // Application only captures a frame and shows it
    DETECTIONL, // Application detects the planar object whose features are stored
    TAKING_NEW_TEMPLATEL, // Application captures a new template
    ENDL // Application quits
  } appModeL;

enum APPLICATION_MODER
{
  DO_NOTHINGR, // Application only captures a frame and shows it
  DETECTIONR, // Application detects the planar object whose features are stored
  TAKING_NEW_TEMPLATER, // Application captures a new template
  ENDR // Application quits
} appModeR;

// Number  of the  points selected  by the  user on  the  new template
// image:
int noOfPointsPickedOnTemplateImageL;
int noOfPointsPickedOnTemplateImageR;


// Image coordinates  of the  points selected by  the user in  the new
// template image:
CvPoint templateCornersL[2];
CvPoint templateCornersR[2];


/****************************************** INLINE FUNCTIONS **************************************/

// Returns radian equivalent of an angle in degrees
inline float degreeToRadian(const float d)
{
  return (d / 180.0) * M_PI;
}

// Converts processor tick counts to milliseconds
inline double toElapsedTime(const double t)
{
  return t / (1e3 * cvGetTickFrequency());
}

/**************************************************************************************************/

using namespace yarp::os;
using namespace yarp::sig;

Att::Att(std::string name) : Module(name) {

	std::cout << "\nConstructing " << getName() << "...";

	cvImageIn.setModule(this);
	pointIn.setModule(this);
	debug = false;

	std::cout << "finished.";

}




Att::~Att() {

	std::cout << "\nDestructing " << getName() << "...";

	/*if(cvimg) delete cvimg;

	//cvReleaseImage(&newFrameRGB);
	//cvReleaseImage(&outputImage);
	cvReleaseImage(&result);*/

/*	if(cv_templateRGB) delete cv_templateRGB;
	if(cv_fullRGB) delete cv_fullRGB;
	if(mp_cvimgL) delete mp_cvimgL;
	if(mp_tempcvimgL) delete mp_tempcvimgL;
*/

	std::cout << "finished.\n";

}

// Function for handling mouse inputs
/*void mouseHandlerL(int event, int x, int y, int flags, void* params)
{
  if (appModeL == TAKING_NEW_TEMPLATEL) {
    templateCornersL[1] = cvPoint(x, y);
    switch (event) {
    case CV_EVENT_LBUTTONDOWN:
      templateCornersL[noOfPointsPickedOnTemplateImageL++] = cvPoint(x, y);
      break;
    case CV_EVENT_RBUTTONDOWN:
      break;
    case CV_EVENT_MOUSEMOVE:
      if (noOfPointsPickedOnTemplateImageL == 1)
	templateCornersL[1] = cvPoint(x, y);
      break;
    }
  }
}

void mouseHandlerR(int event, int x, int y, int flags, void* params)
{
  if (appModeR == TAKING_NEW_TEMPLATER) {
    templateCornersR[1] = cvPoint(x, y);
    switch (event) {
    case CV_EVENT_LBUTTONDOWN:
      templateCornersR[noOfPointsPickedOnTemplateImageR++] = cvPoint(x, y);
      break;
    case CV_EVENT_RBUTTONDOWN:
      break;
    case CV_EVENT_MOUSEMOVE:
      if (noOfPointsPickedOnTemplateImageR == 1)
	templateCornersR[1] = cvPoint(x, y);
      break;
    }
  }
}
*/
void Att::memoryall()
{
  // Memory Allocations
  newFrameGray = cvCreateImage(cvSize(Frame_Width, Frame_Height), IPL_DEPTH_8U, 1);
  outputImage = cvCreateImage(cvSize(Frame_Width, Frame_Height), IPL_DEPTH_8U, 3);
  templateImageRGBFull = cvCreateImage(cvSize(Frame_Width, Frame_Height), IPL_DEPTH_8U, 3);
  sideBySideImage = cvCreateImage(cvSize(2 * Frame_Width, Frame_Height), IPL_DEPTH_8U, 3);
  templateImageRGB = cvCreateImage(cvSize(1, 1), IPL_DEPTH_8U, 3);
  templateImageGray = cvCreateImage(cvSize(1, 1), IPL_DEPTH_8U, 1);
  for (int s = 0; s < NUMBER_OF_SCALE_STEPS; s++) {
   for (int r = 0; r < NUMBER_OF_ROTATION_STEPS; r++) {
      templateObjectCornersData[s][r] = new double[8];
      templateObjectCorners[s][r] = cvMat(1, 4, CV_64FC2, templateObjectCornersData[s][r]);
    }
  }

  bella = getName();


}

void Att::clean(){

	cvDestroyWindow((bella).c_str());
/*	  if(bella =="Attention_Left")
	  	  cvDestroyWindow("Template_Left");
	  if(bella == "Attention_Right")
		  cvDestroyWindow("Template_Right");*/
	 //cvReleaseImage(&templateImageRGB);
	//cvReleaseImage(&templateImageGray);

	if(cv_templateRGB) delete cv_templateRGB;
	if(cv_fullRGB) delete cv_fullRGB;
	if(mp_cvimgL) delete mp_cvimgL;
	if(mp_tempcvimgL) delete mp_tempcvimgL;
	
	PointOutPort.interrupt();
	PointOutPort.close();

}

void Att::execute(){

	cvNamedWindow((bella).c_str(), CV_WINDOW_AUTOSIZE);

	templateImageRGB = NULL;
	templateImageGray = NULL;

	hasAnyTemplateChosenBefore = false;

	doDBSearch = true;

	cv_templateRGB = NULL;
	cv_fullRGB = NULL;
	mp_cvimgL = NULL;
	mp_tempcvimgL = NULL;

	pointx = 160;
	pointy = 120;

	point.x = 160;
	point.y = 120;
	
	reliability.x = 0;
	reliability.y = 0;

	distx = 160;
	disty = 120;

	noOfPointsPickedOnTemplateImageL=0;
	noOfPointsPickedOnTemplateImageR=0;
	templateCornersL[0].x=0;
	templateCornersL[0].y=0;
	templateCornersL[1].x=0;
	templateCornersL[1].y=0;
	templateCornersR[0].x=0;
	templateCornersR[0].y=0;
	templateCornersR[1].x=0;
	templateCornersR[1].y=0;

	one = true;
	gettemp=false;
	pointsent=false;
	init();

}
int Att::moveWindow(int x, int y) {
//	int width = imageIn.getBuffer()->width;
	cvMoveWindow(bella.c_str(), x, y);
	return 0;
}

// Initializes the application
void Att::init(void)
{
  // Seed the random number generator
  srand(time(NULL));
  // In the beginning, only capture a frame and show it to the user
  if(bella=="Attention_Left")
	  appModeL = DO_NOTHINGL;
  if(bella=="Attention_Right")
	  appModeR = DO_NOTHINGR;

//  stop = false;

/*  if (bella=="Attention_Left")
  {
	  cvSetMouseCallback((bella).c_str(), mouseHandlerL, NULL);
	  //cvNamedWindow("Template_Left", CV_WINDOW_AUTOSIZE);
  }

  if (bella=="Attention_Right"){
	  cvSetMouseCallback((bella).c_str(), mouseHandlerR, NULL);
	  //cvNamedWindow("Template_Right", CV_WINDOW_AUTOSIZE);
  }
*/
  cvInitFont(&font, CV_FONT_HERSHEY_PLAIN | CV_FONT_ITALIC, 1, 1, 0, 1);
}


// Function for handling keyboard inputs
void Att::waitKeyAndHandleKeyboardInput(int timeout,bool *one)
{
	//bella = getName();
	// Wait for the keyboard input
	const char key = cvWaitKey(timeout);
	// Change the application mode according to the keyboard input
	switch (key) {
	case 'q': case 'Q':

		if(bella=="Attention_Left")
			appModeL = ENDL;
		if(bella=="Attention_Right")
			appModeR = ENDR;
		*one = false;
		break;

	case 't': case 'T':
		if (appModeL == TAKING_NEW_TEMPLATEL) {

			noOfPointsPickedOnTemplateImageL = 0;

			// if a template has been taken before, go back to detection of last template
			// otherwise
			appModeL = hasAnyTemplateChosenBefore ? DETECTIONL : DO_NOTHINGL;
		}
		else
			appModeL = TAKING_NEW_TEMPLATEL;

		if (appModeR == TAKING_NEW_TEMPLATER) {

			noOfPointsPickedOnTemplateImageR = 0;

			appModeR = hasAnyTemplateChosenBefore ? DETECTIONR : DO_NOTHINGR;
		}
		else
			appModeR = TAKING_NEW_TEMPLATER;

		break;


/*
	case 't': case 'T':
		if(bella=="Attention_Left"){
			if (appModeL == TAKING_NEW_TEMPLATEL) {

				noOfPointsPickedOnTemplateImageL = 0;

				// if a template has been taken before, go back to detection of last template
				// otherwise
				appModeL = hasAnyTemplateChosenBefore ? DETECTIONL : DO_NOTHINGL;
			}
				else
					appModeL = TAKING_NEW_TEMPLATEL;


		}
		break;

	case 'y': case 'Y':
		if(bella=="Attention_Right"){
			if (appModeR == TAKING_NEW_TEMPLATER) {

				noOfPointsPickedOnTemplateImageR = 0;

				appModeR = hasAnyTemplateChosenBefore ? DETECTIONR : DO_NOTHINGR;
			}
				else
					appModeR = TAKING_NEW_TEMPLATER;

		}
		break;
*/
	case 'd': case 'D':
		doDBSearch = !doDBSearch;
		break;
	}
}



// Draws a quadrangle on an image given (u, v) coordinates, color and thickness
void Att::drawQuadrangle(IplImage* frame,
		    const int u0, const int v0,
		    const int u1, const int v1,
		    const int u2, const int v2,
		    const int u3, const int v3,
		    const CvScalar color, const int thickness)
{
	cvLine(frame, cvPoint(u0, v0), cvPoint(u1, v1), color, thickness);
	cvLine(frame, cvPoint(u1, v1), cvPoint(u2, v2), color, thickness);
	cvLine(frame, cvPoint(u2, v2), cvPoint(u3, v3), color, thickness);
	cvLine(frame, cvPoint(u3, v3), cvPoint(u0, v0), color, thickness);
	const int barx = (u0+u1+u2+u3)/4;
	const int bary = (v0+v1+v2+v3)/4;

	cvCircle(frame,cvPoint(barx,bary),3,color,thickness);
	cvCircle(frame,cvPoint(160,120),3,cvScalar(0,0,255),thickness);
	

	/*point = cvPoint(barx,bary);
	pointOut.setBuffer(&point);
	pointOut.out();*/
	
	distx = barx - 160;
	disty = bary - 120;

	/*stop = false;
	
	
	if((bella == "Attention_Left") &(sqrt(x*x+y*y)<=10))
		stop = true;
	if((bella == "Attention_Right") &(sqrt(x*x+y*y)<=10))
		stop = true;
	
	if (debug)
	{
		if( (bella == "Attention_Left") )
		{
			if( sqrt(x*x+y*y)<=2 )
			{		
				//Reliability = 2 Icub won't move
				std::cout << "\n" << getName() << "::drawQuadrangle()::sending reliability = 2\nDistance from center of the image < 2";
				printf("\ndistance: %f",sqrt(x*x+y*y));
		
				reliability = cvPoint (2,0);
				//stop = true;
			}
			else
			{
				//Reliability = 0 Icub will move
				std::cout << "\n" << getName() << "::drawQuadrangle()::sending reliability = 1\nDistance from center of the image > 2";
				printf("\ndistance: %f",sqrt(x*x+y*y));
		
				reliability = cvPoint (1,0);
			}
			
			
		}
	
		if( (bella == "Attention_Right") )
		{
			if( sqrt(x*x+y*y)<=2 )
			{
				//Reliability = 2 Icub won't move
				std::cout << "\n" << getName() << "::drawQuadrangle()::sending reliability = 2\nDistance from center of the image < 2";
				printf("\ndistance: %f",sqrt(x*x+y*y));
		
				reliability = cvPoint (2,0);
				//stop = true;
			}
			else
			{
				//Reliability = 0 Icub will move
				std::cout << "\n" << getName() << "::drawQuadrangle()::sending reliability = 1\nDistance from center of the image > 2";
				printf("\ndistance: %f",sqrt(x*x+y*y));

				reliability = cvPoint (1,0);
			}

		}
	}*/
	reliability = cvPoint (1,0);
	pointx = barx;
	pointy = bary;



}

// Draws a quadrangle with the corners of the object detected on img
void Att::markDetectedObject(IplImage* frame, const double * detectedCorners)
{
  drawQuadrangle(frame,
		 detectedCorners[0], detectedCorners[1],
		 detectedCorners[2], detectedCorners[3],
		 detectedCorners[4], detectedCorners[5],
		 detectedCorners[6], detectedCorners[7],
		 cvScalar(255, 0, 0), 3);
}

// Draws a plus sign on img given (x, y) coordinate
void Att::drawAPlus(IplImage* img, const int x, const int y)
{
  cvLine(img, cvPoint(x - 5, y), cvPoint(x + 5, y), CV_RGB(255, 0, 0));
  cvLine(img, cvPoint(x, y - 5), cvPoint(x, y + 5), CV_RGB(255, 0, 0));
}

// Marks the keypoints with plus signs on img
void Att::showKeypoints(IplImage* img, const vector<cv::KeyPoint>& kpts)
{
  for (unsigned int i = 0; i < kpts.size(); ++i)
    drawAPlus(img, kpts[i].pt.x, kpts[i].pt.y);
}

// Captures a new frame. Returns if capture is taken without problem or not.
bool Att::takeNewFrame(void)
{



  cvimg = cvImageIn.getBuffer();

  IplImage* local = cvCloneImage(cvimg->ipl);
  if (local){
	  newFrameRGB = cvCloneImage(local);
	  cvCvtColor(newFrameRGB, newFrameGray, CV_BGR2GRAY);
	  cvReleaseImage(&local);

  }
  else
  {
    return false;
    cvReleaseImage(&local);
  }
  return true;

}

// Puts img1 and img2 side by side and stores into result
void Att::putImagesSideBySide(IplImage* result, const IplImage* img1, const IplImage* img2)
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

  for (int i = 0; i < Frame_Height; ++i, p_big += bigWS, p_bigMiddle += bigWS) {
    // copy a row of the left image till the half of the resulting image
    memcpy(p_big, p_l + i*lWS, lWS);
    // copy a row of the right image from the half of the resulting image to the end of it
    memcpy(p_bigMiddle, p_r + i*rWS, rWS);
  }
}

// Marks the matching keypoints on two images which were put side by side
void Att::showMatches(const int matchCount)
{
  const int iterationEnd = 2 * matchCount;

  for (int xCoor = 0, yCoor = 1; xCoor < iterationEnd; xCoor += 2, yCoor += 2) {
    // Draw a line between matching keypoints
    cvLine(sideBySideImage,
	   cvPoint(match2Data[xCoor], match2Data[yCoor]),
	   cvPoint(pointsOnOriginalImage[xCoor] + templateROIX + Frame_Width,
		   pointsOnOriginalImage[yCoor] + templateROIY),
	   cvScalar(0, 255, 0), 1);
  }
}

// Returns whether H is a nice homography matrix or not
bool Att::niceHomography(const CvMat * H)
{
  const double det = cvmGet(H, 0, 0) * cvmGet(H, 1, 1) - cvmGet(H, 1, 0) * cvmGet(H, 0, 1);
  if (det < 0)
    return false;

  const double N1 = sqrt(cvmGet(H, 0, 0) * cvmGet(H, 0, 0) + cvmGet(H, 1, 0) * cvmGet(H, 1, 0));
  if (N1 > 4 || N1 < 0.1)
    return false;

  const double N2 = sqrt(cvmGet(H, 0, 1) * cvmGet(H, 0, 1) + cvmGet(H, 1, 1) * cvmGet(H, 1, 1));
  if (N2 > 4 || N2 < 0.1)
    return false;

  const double N3 = sqrt(cvmGet(H, 2, 0) * cvmGet(H, 2, 0) + cvmGet(H, 2, 1) * cvmGet(H, 2, 1));
  if (N3 > 0.002)
    return false;

  return true;
}

// Rotates src around center with given angle and assigns the result to dst
void Att::rotateImage(IplImage* dst, IplImage* src, const CvPoint2D32f& center, float angle)
{
  static CvMat *rotMat = cvCreateMat(2, 3, CV_32FC1);
  cv2DRotationMatrix(center, angle, 1.0, rotMat);
  cvWarpAffine(src, dst, rotMat);
}

// Transforms the coordinates of the keypoints of a template image whose matrix index is
// (scaleInd, rotInd) into the original template image's (scale = 1, rotation angle = 0) coordinates
void Att::transformPointsIntoOriginalImageCoordinates(const int matchNo, const int scaleInd, const int rotInd)
{
  // Difference between the angles of two consecutive samples
  static const float ROT_ANGLE_INCREMENT = 360.0 / NUMBER_OF_ROTATION_STEPS;

  // Take the scale samples in a logarithmic base
  static const float k = exp(log(SMALLEST_SCALE_CHANGE) / (NUMBER_OF_SCALE_STEPS - 1));
  const float scale = pow(k, scaleInd);

  // Center of the original image
  const float orgCenterX = templateImageGray->width / 2.0;
  const float orgCenterY = templateImageGray->height / 2.0;

  // Center of the scaled image
  const float centerX = orgCenterX * scale;
  const float centerY = orgCenterY * scale;

  // Rotation angle for the template
  const float angle = ROT_ANGLE_INCREMENT * rotInd;
  // Avoid repeatition of the trigonometric calculations
  const float cosAngle = cos(degreeToRadian(-angle));
  const float sinAngle = sin(degreeToRadian(-angle));

  const float iterationEnd = 2 * matchNo;
  for (int xCoor = 0, yCoor = 1; xCoor < iterationEnd; xCoor += 2, yCoor += 2) {
    // Translate the point so that the origin is in the middle of the image
    const float translatedX = match1Data[xCoor] - centerX;
    const float translatedY = match1Data[yCoor] - centerY;

    // Rotate the point so that the angle between this template and the original template will be zero
    const float rotatedBackX = translatedX * cosAngle - translatedY * sinAngle;
    const float rotatedBackY = translatedX * sinAngle + translatedY * cosAngle;

    // Scale the point so that the size of this template will be equal to the original one
    pointsOnOriginalImage[xCoor] = rotatedBackX / scale + orgCenterX;
    pointsOnOriginalImage[yCoor] = rotatedBackY / scale + orgCenterY;
  }
}

// Estimates the fps of the application
void Att::fpsCalculation(void)
{
  static int64 currentTime, lastTime = cvGetTickCount();
  static int fpsCounter = 0;
  currentTime = cvGetTickCount();
  ++fpsCounter;

  // If 1 second has passed since the last FPS estimation, update the fps
  if (currentTime - lastTime > 1e6 * cvGetTickFrequency()) {
    fps = fpsCounter;
    lastTime = currentTime;
    fpsCounter = 0;
  }
}

// Writes the statistics showing the performance of the application to img
void Att::showOutput(IplImage* img)
{
	static char text[256];

  if((bella == "Attention_Left") && (appModeL == TAKING_NEW_TEMPLATEL)){
		sprintf(text, "PLEASE DRAW A BOX");
		cvPutText(img, text, cvPoint(10, 30), &font, cvScalar(255, 0, 0));
  }
  if((bella == "Attention_Right") && (appModeR == TAKING_NEW_TEMPLATER)){
		sprintf(text, "PLEASE DRAW A BOX");
		cvPutText(img, text, cvPoint(10, 30), &font, cvScalar(255, 0, 0));
  }
/*
  if (appModeL != TAKING_NEW_TEMPLATEL && appModeR !=TAKING_NEW_TEMPLATER) {
    sprintf(text, "FPS: %d", fps);
    cvPutText(img, text, cvPoint(10, 30), &font, cvScalar(255, 0, 0));
  
    sprintf(text, "KP Extract: %f", toElapsedTime(keypointExtractTime));
    cvPutText(img, text, cvPoint(10, 50), &font, cvScalar(255, 0, 0));

    sprintf(text, "Bit Desc: %f", toElapsedTime(bitDescTime));
    cvPutText(img, text, cvPoint(10, 70), &font, cvScalar(255, 0, 0));

    sprintf(text, "Match Time: %f", toElapsedTime(matchTime));
    cvPutText(img, text, cvPoint(10, 90), &font, cvScalar(255, 0, 0));

    sprintf(text, "Total Matching Time: %f", toElapsedTime(totalMatchTime));
    cvPutText(img, text, cvPoint(10, 110), &font, cvScalar(255, 0, 0));

    sprintf(text, "RANSAC: %f", toElapsedTime(hmgEstTime));
    cvPutText(img, text, cvPoint(10, 130), &font, cvScalar(255, 0, 0));

    sprintf(text, "Match Percentage: %d%%", matchPercentage);
    cvPutText(img, text, cvPoint(10, 150), &font, cvScalar(255, 0, 0));
    
    }
    
*/
  

  cvShowImage((bella).c_str(), img);
/*  if(bella =="Attention_Left")
	  cvShowImage("Template_Left", templateImageRGB);
  if(bella == "Attention_Right")
	  cvShowImage("Template_Right", templateImageRGB);*/
}

// Detect keypoints of img with FAST and store them to kpts given the threshold kptDetectorThreshold.
int Att::extractKeypoints(vector< cv::KeyPoint >& kpts, int kptDetectorThreshold, IplImage* img)
{
  CvRect r = cvRect(CVLAB::IMAGE_PADDING_LEFT, CVLAB::IMAGE_PADDING_TOP,
		    CVLAB::SUBIMAGE_WIDTH(img->width), CVLAB::SUBIMAGE_HEIGHT(img->height));

  // Don't detect keypoints on the image borders:
  cvSetImageROI(img, r);

  // Use FAST corner detector to detect the image keypoints
  cv::FAST(img, kpts, kptDetectorThreshold, true);

  // Get the borders back:
  cvResetImageROI(img);

  // Transform the points to their actual image coordinates:
  for (unsigned int i = 0, sz = kpts.size(); i < sz; ++i)
    kpts[i].pt.x += CVLAB::IMAGE_PADDING_LEFT, kpts[i].pt.y += CVLAB::IMAGE_PADDING_TOP;

  return kpts.size();
}

// Tries to find a threshold for FAST that gives a number of keypoints between lowerBound and upperBound:
int Att::chooseFASTThreshold(const IplImage* img, const int lowerBound, const int upperBound)
{
  static vector<cv::KeyPoint> kpts;

  int left = 0;
  int right = 255;
  int currentThreshold = 128;
  int currentScore = 256;

  IplImage* copyImg = cvCreateImage(cvGetSize(img), img->depth, img->nChannels);
  cvCopyImage(img, copyImg);

  while (currentScore < lowerBound || currentScore > upperBound) {
    currentScore = extractKeypoints(kpts, currentThreshold, copyImg);

    if (lowerBound > currentScore) {
      // we look for a lower threshold to increase the number of corners:
      right = currentThreshold;
      currentThreshold = (currentThreshold + left) >> 1;
      if (right == currentThreshold)
	break;
    } else {
      // we look for a higher threshold to decrease the number of corners:
      left = currentThreshold;
      currentThreshold = (currentThreshold + right) >> 1;
      if (left == currentThreshold)
	break;
    }
  }
  cvReleaseImage(&copyImg);

  return currentThreshold;
}

// Saves the coordinates of the corners of the rectangle drawn by the user when
// capturing a new template image
void Att::saveCornerCoors(void)
{
  const double templateWidth = templateImageGray->width;
  const double templateHeight = templateImageGray->height;

  double* corners = templateObjectCornersData[0][0];
  corners[0] = 0;
  corners[1] = 0;
  corners[2] = templateWidth;
  corners[3] = 0;
  corners[4] = templateWidth;
  corners[5] = templateHeight;
  corners[6] = 0;
  corners[7] = templateHeight;
}

// Saves the image inside the rectangle drawn by the user as the new template image.
// Returns if it can be used as a new template or not.
bool Att::saveNewTemplate(void)
{
	//bella= getName();

	if(bella=="Attention_Left")
	{
  		// Calculate the size of the new template
		/*const int templateWidth = templateCornersL[1].x - templateCornersL[0].x;
		const int templateHeight = templateCornersL[1].y - templateCornersL[0].y;*/

		int templateW = templateCornersL[1].x - templateCornersL[0].x;
		int templateH = templateCornersL[1].y - templateCornersL[0].y;
		int cornerx = templateCornersL[0].x;
		int cornery = templateCornersL[0].y;

		  // If the size of of the new template is illegal, return false
		if ((SMALLEST_SCALE_CHANGE * templateW) < CVLAB::IMAGE_PADDING_TOTAL || (SMALLEST_SCALE_CHANGE * templateH) < CVLAB::IMAGE_PADDING_TOTAL)
		{

			while(((SMALLEST_SCALE_CHANGE * templateW) <= CVLAB::IMAGE_PADDING_TOTAL) 							 				|| ((SMALLEST_SCALE_CHANGE * templateH) <= CVLAB::IMAGE_PADDING_TOTAL))
			{
				std::cout << getName() << "::save new template()::illegal size of template\n";  
				templateW = (templateW + 2);
				templateH = (templateH + 2);
				cornerx = cornerx - 1;
				cornery = cornery - 1;
			}
		}	    
	
		//return false;

		const int templateWidth = templateW;
		const int templateHeight = templateH;
		
		templateCornersL[0].x = cornerx;
		templateCornersL[0].y = cornery;

		// Store the upper left corner coordinate of the rectangle (ROI)
		templateROIX = templateCornersL[0].x, templateROIY = templateCornersL[0].y;


		const CvSize templateSize = cvSize(templateWidth, templateHeight);
		const CvRect templateRect = cvRect(templateCornersL[0].x, templateCornersL[0].y,
				     templateWidth, templateHeight);

		// Store the original version of the new template(all image)
		cvCopyImage(newFrameRGB, templateImageRGBFull);

		cvReleaseImage(&templateImageRGB);
		templateImageRGB = cvCreateImage(templateSize, IPL_DEPTH_8U, 3);

		cvReleaseImage(&templateImageGray);
		templateImageGray = cvCreateImage(templateSize, IPL_DEPTH_8U, 1);

		// Store a Grayscale version of the new template(only ROI)
		cvSetImageROI(newFrameGray, templateRect);
		cvCopyImage(newFrameGray, templateImageGray);
		cvResetImageROI(newFrameGray);


		// Store an RGB version of the new template(only ROI)
		cvSetImageROI(newFrameRGB, templateRect);
		cvCopyImage(newFrameRGB, templateImageRGB);
		cvResetImageROI(newFrameRGB);
	}


	if(bella=="Attention_Right")
	{
		// Calculate the size of the new template
		/*const int templateWidth = templateCornersR[1].x - templateCornersR[0].x;
		const int templateHeight = templateCornersR[1].y - templateCornersR[0].y;*/

		int templateW = templateCornersR[1].x - templateCornersR[0].x;
		int templateH = templateCornersR[1].y - templateCornersR[0].y;
		int cornerx = templateCornersR[0].x;
		int cornery = templateCornersR[0].y;

		// If the size of of the new template is illegal, return false
		if ((SMALLEST_SCALE_CHANGE * templateW) < CVLAB::IMAGE_PADDING_TOTAL || (SMALLEST_SCALE_CHANGE * templateH) < CVLAB::IMAGE_PADDING_TOTAL)
		{
			while(((SMALLEST_SCALE_CHANGE * templateW) <= CVLAB::IMAGE_PADDING_TOTAL) 							 				|| ((SMALLEST_SCALE_CHANGE * templateH) <= CVLAB::IMAGE_PADDING_TOTAL))
			{
				std::cout << getName() << "::save new template()::illegal size of template\n";  
				templateW = (templateW + 2);
				templateH = (templateH + 2);
				cornerx = cornerx - 1;
				cornery = cornery - 1
;
			}
		}	    
	
		//return false;

		const int templateWidth = templateW;
		const int templateHeight = templateH;

		templateCornersR[0].x = cornerx;
		templateCornersR[0].y = cornery;

		// Store the upper left corner coordinate of the rectangle (ROI)
		templateROIX = templateCornersR[0].x, templateROIY = templateCornersR[0].y;
		
		const CvSize templateSize = cvSize(templateWidth, templateHeight);
		const CvRect templateRect = cvRect(templateCornersR[0].x, templateCornersR[0].y,
				     templateWidth, templateHeight);

		// Store the original version of the new template(all image)
		cvCopyImage(newFrameRGB, templateImageRGBFull);

		cvReleaseImage(&templateImageRGB);
		templateImageRGB = cvCreateImage(templateSize, IPL_DEPTH_8U, 3);

		cvReleaseImage(&templateImageGray);
		templateImageGray = cvCreateImage(templateSize, IPL_DEPTH_8U, 1);

		// Store a Grayscale version of the new template(only ROI)
		cvSetImageROI(newFrameGray, templateRect);
		cvCopyImage(newFrameGray, templateImageGray);
		cvResetImageROI(newFrameGray);


		// Store an RGB version of the new template(only ROI)
		cvSetImageROI(newFrameRGB, templateRect);
		cvCopyImage(newFrameRGB, templateImageRGB);
		cvResetImageROI(newFrameRGB);

	}




	saveCornerCoors();

	return true;
}

// Finds the coordinates of the corners of the template image after scaling and rotation applied
void Att::estimateCornerCoordinatesOfNewTemplate(int scaleInd, int rotInd, float scale, float angle)
{
  static double* corners = templateObjectCornersData[0][0];

  // Center of the original image
  const float orgCenterX = templateImageGray->width / 2.0, orgCenterY = templateImageGray->height / 2.0;
  // Center of the scaled image
  const float centerX = orgCenterX * scale, centerY = orgCenterY * scale;

  const float cosAngle = cos(degreeToRadian(angle));
  const float sinAngle = sin(degreeToRadian(angle));

  for (int xCoor = 0, yCoor = 1; xCoor < 8; xCoor += 2, yCoor += 2) {
    // Scale the point and translate it so that the origin is in the middle of the image
    const float resizedAndTranslatedX = (corners[xCoor] * scale) - centerX,
      resizedAndTranslatedY = (corners[yCoor] * scale) - centerY;

    // Rotate the point with the given angle
    templateObjectCornersData[scaleInd][rotInd][xCoor] =
      (resizedAndTranslatedX * cosAngle - resizedAndTranslatedY * sinAngle) + centerX;
    templateObjectCornersData[scaleInd][rotInd][yCoor] =
      (resizedAndTranslatedX * sinAngle + resizedAndTranslatedY * cosAngle) + centerY;
  }
}

// Generates new templates with different scales and orientations and stores their keypoints and
// BRIEF descriptors.
void Att::learnTemplate(void)
{
  static const float ROT_ANGLE_INCREMENT = 360.0 / NUMBER_OF_ROTATION_STEPS;
  static const float k = exp(log(SMALLEST_SCALE_CHANGE) / (NUMBER_OF_SCALE_STEPS - 1));

  // Estimate a feasible threshold value for FAST keypoint detector
  fastThreshold = chooseFASTThreshold(templateImageGray, 200, 250);

  // For every scale generate templates
  for (int scaleInd = 0; scaleInd < NUMBER_OF_SCALE_STEPS; ++scaleInd) {
    // Calculate the template size in a log basis
    const float currentScale = pow(k, scaleInd);

    // Scale the image
    IplImage* scaledTemplateImg = cvCreateImage(cvSize(templateImageGray->width * currentScale,
						       templateImageGray->height * currentScale),
						IPL_DEPTH_8U, 1);
    cvResize(templateImageGray, scaledTemplateImg);

    const CvPoint2D32f center = cvPoint2D32f(scaledTemplateImg->width >> 1, scaledTemplateImg->height >> 1);

    // For a given scale, generate templates with several rotations
    float currentAngle = 0.0;
    for (int rotInd = 0; rotInd < NUMBER_OF_ROTATION_STEPS; ++rotInd, currentAngle += ROT_ANGLE_INCREMENT) {
      // Rotate the image
      IplImage* rotatedImage = cvCreateImage(cvGetSize(scaledTemplateImg),
					     scaledTemplateImg->depth,
					     scaledTemplateImg->nChannels);
      rotateImage(rotatedImage, scaledTemplateImg, center, -currentAngle);

      // Detect FAST keypoints
      extractKeypoints(templateKpts[scaleInd][rotInd], fastThreshold, rotatedImage);

      // Describe the keypoints with BRIEF descriptors
      brief.getBriefDescriptors(templateDescs[scaleInd][rotInd],
				templateKpts[scaleInd][rotInd],
				rotatedImage);

      // Store the scaled and rotated template corner coordinates
      estimateCornerCoordinatesOfNewTemplate(scaleInd, rotInd, currentScale, currentAngle);

      cvReleaseImage(&rotatedImage);
    }
    cvReleaseImage(&scaledTemplateImg);
  }
}

// Manages the capture of the new template image according to the points picked by the user
void Att::takeNewTemplateImage(void)
{
	//bella = getName();
	cvCopyImage(newFrameRGB, outputImage);


	if(bella=="Attention_Left"){

	attpoint = pointIn.getBuffer();
	//if (debug)
		printf("\nLEFT:\npoint1x:%d, point1y:%d\n",attpoint->x,attpoint->y);
	int xL, yL;
	xL = attpoint->x;
	yL = attpoint->y;
	noOfPointsPickedOnTemplateImageL=2;
	templateCornersL[0] = cvPoint(xL,yL);
	templateCornersL[1] = cvPoint(xL,yL);

		switch (noOfPointsPickedOnTemplateImageL) {
		case 1:
			cvRectangle(outputImage, templateCornersL[0], templateCornersL[1], cvScalar(0, 255, 0), 3);
			break;
		case 2:
			if (saveNewTemplate()) {
				learnTemplate();
				appModeL = DETECTIONL;
				hasAnyTemplateChosenBefore = true;
			}
			noOfPointsPickedOnTemplateImageL = 0;
			break;
		default:
			break;
		}
	}

	if(bella=="Attention_Right"){

		attpoint = pointIn.getBuffer();
		//if (debug)
			printf("\nRIGHT\npoint1x:%d, point1y:%d\n",attpoint->x,attpoint->y);

		noOfPointsPickedOnTemplateImageR=2;

		int xR, yR;
		xR = attpoint->x;
		yR = attpoint->y;
		noOfPointsPickedOnTemplateImageR=2;
		templateCornersR[0] = cvPoint(xR,yR);
		templateCornersR[1] = cvPoint(xR,yR);

		switch (noOfPointsPickedOnTemplateImageR) {
		case 1:
			cvRectangle(outputImage, templateCornersR[0], templateCornersR[1], cvScalar(0, 255, 0), 3);
			break;
		case 2:
			if (saveNewTemplate()) {
				learnTemplate();
				appModeR = DETECTIONR;
				hasAnyTemplateChosenBefore = true;
			}
			noOfPointsPickedOnTemplateImageR = 0;
			break;
		default:
			break;
		}
	}

}

// Matches Brief descriptors descs1 and descs2 in terms of Hamming Distance.
int Att::matchDescriptors(
		     CvMat& match1, CvMat& match2,
		     const vector< bitset<CVLAB::DESC_LEN> > descs1,
		     const vector< bitset<CVLAB::DESC_LEN> > descs2,
		     const vector<cv::KeyPoint>& kpts1,
		     const vector<cv::KeyPoint>& kpts2)
{
  // Threshold value for matches.
  static const int MAX_MATCH_DISTANCE = 50;

  int numberOfMatches = 0;
  // Index of the best BRIEF descriptor match on descs2
  int bestMatchInd2 = 0;

  // For every BRIEF descriptor in descs1 find the best fitting BRIEF descriptor in descs2
  for (unsigned int i = 0; i < descs1.size() && numberOfMatches < MAXIMUM_NUMBER_OF_MATCHES; ++i) {
    int minDist = CVLAB::DESC_LEN;

    for (unsigned int j = 0; j < descs2.size(); ++j) {
      const int dist = CVLAB::HAMMING_DISTANCE(descs1[i], descs2[j]);

      // If dist is less than the optimum one observed so far, the new optimum one is current BRIEF descriptor
      if (dist < minDist) {
	minDist = dist;
	bestMatchInd2 = j;
      }
    }
    // If the Hamming Distance is greater than the threshold, ignore this match
    if (minDist > MAX_MATCH_DISTANCE)
      continue;

    // Save the matching keypoint coordinates
    const int xInd = 2 * numberOfMatches;
    const int yInd = xInd + 1;

    match1Data[xInd] = kpts1[i].pt.x;
    match1Data[yInd] = kpts1[i].pt.y;

    match2Data[xInd] = kpts2[bestMatchInd2].pt.x;
    match2Data[yInd] = kpts2[bestMatchInd2].pt.y;

    numberOfMatches++;
  }

  if (numberOfMatches > 0) {
    cvInitMatHeader(&match1, numberOfMatches, 2, CV_64FC1, match1Data);
    cvInitMatHeader(&match2, numberOfMatches, 2, CV_64FC1, match2Data);
  }

  return numberOfMatches;
}


void Att::connecting(){

	if(bella=="Attention_Left")
	{
		PointOutPort.open(("/"+bella+"/out").c_str());
		Network::connect(("/"+bella+"/out").c_str(),"/motion/left/in");
		/*StopInPort.open(("/"+bella+"/stop/in").c_str());
		Network::connect("/motion/stop/out",("/"+bella+"/stop/in").c_str());*/
	
	}
	if(bella=="Attention_Right")
	{
		PointOutPort.open(("/"+bella+"/out").c_str());
		Network::connect(("/"+bella+"/out").c_str(),"/motion/right/in");
		/*StopInPort.open(("/"+bella+"/stop/in").c_str());
		Network::connect("/motion/stop/out",("/"+bella+"/stop/in").c_str());*/
	}



}



// Detects the template object in the incoming frame
void Att::doDetection(void)
{
	// Variables for elapsed time estimations
	static int64 startTime, endTime;

	// Homography Matrix
	static CvMat* H = cvCreateMat(3, 3, CV_64FC1);

	// Corners of the detected planar object
	static double detectedObjCornersData[8];
	static CvMat detectedObjCorners = cvMat(1, 4, CV_64FC2, detectedObjCornersData);

	// Keypoints of the incoming frame
	vector<cv::KeyPoint> kpts;

	// BRIEF descriptors of kpts
	vector< bitset<CVLAB::DESC_LEN> > descs;

	// Coordinates of the matching keypoints
	CvMat match1, match2;

	float maxRatio = 0.0;
	int maxScaleInd = 0;
	int maxRotInd = 0;
	int maximumNumberOfMatches = 0;

	// If !doDBSearch then only try to match the original template image
	const int dbScaleSz = doDBSearch ? NUMBER_OF_SCALE_STEPS : 1;
	const int dbRotationSz = doDBSearch ? NUMBER_OF_ROTATION_STEPS : 1;

	startTime = cvGetTickCount();
	// Detect the FAST keypoints of the incoming frame
	extractKeypoints(kpts, fastThreshold, newFrameGray);
	endTime = cvGetTickCount();
	keypointExtractTime = endTime - startTime;


	startTime = cvGetTickCount();
	// Describe the keypoints with BRIEF descriptors
	brief.getBriefDescriptors(descs, kpts, newFrameGray);
	endTime = cvGetTickCount();
	bitDescTime = endTime - startTime;

	startTime = cvGetTickCount();
	// Search through all the templates
	for (int scaleInd = 0; scaleInd < dbScaleSz; ++scaleInd) {
		for (int rotInd = 0; rotInd < dbRotationSz; ++rotInd) {
			const int numberOfMatches = matchDescriptors(match1, match2,
						   templateDescs[scaleInd][rotInd], descs,
						   templateKpts[scaleInd][rotInd], kpts);

			// Since RANSAC needs at least 4 points, ignore this match
			if (numberOfMatches < 4)
				continue;

			// Save the matrix index of the best fitting template to the incoming frame
			const float currentRatio = float(numberOfMatches) / templateKpts[scaleInd][rotInd].size();
			if (currentRatio > maxRatio) {
				maxRatio = currentRatio;
				maxScaleInd = scaleInd;
				maxRotInd = rotInd;
				maximumNumberOfMatches = numberOfMatches;
			}
		}
	}
	endTime = cvGetTickCount();
	totalMatchTime = endTime - startTime;


	matchPercentage = int(maxRatio * 100.0);

	if (maximumNumberOfMatches > 3) {
		startTime = cvGetTickCount();
		// Match the best fitting template's BRIEF descriptors with the incoming frame
		matchDescriptors(match1, match2,
		templateDescs[maxScaleInd][maxRotInd], descs,
		templateKpts[maxScaleInd][maxRotInd], kpts);
		endTime = cvGetTickCount();
		matchTime = endTime - startTime;

		// Calculate the homography matrix via RANSAC
		cvFindHomography(&match1, &match2, H, CV_RANSAC, 10, 0);

		// If H is not a feasible homography matrix, ignore it
		if (niceHomography(H)) {
			startTime = cvGetTickCount();
			// Transform the coordinates of the corners of the template into image coordinates
			cvPerspectiveTransform(&templateObjectCorners[maxScaleInd][maxRotInd], &detectedObjCorners, H);
			endTime = cvGetTickCount();
			hmgEstTime = endTime - startTime;

			// Draw the detected object on the image
			markDetectedObject(sideBySideImage, detectedObjCornersData);

			// Scale and rotate the coordinates of the template keypoints to transform them into the original
			// template image's coordinates to show the matches
			transformPointsIntoOriginalImageCoordinates(maximumNumberOfMatches, maxScaleInd, maxRotInd);
		}
		else
		{	
			//Reliability = 0 Icub won't move
			if (debug)
				std::cout << "\n" << getName() << "::doDetection()::sending reliability = 0\nNiceHomography = false";
			reliability = cvPoint (0,0);
		}
	}
	else
	{	
		//Reliability = 0 Icub won't move
		if (debug)
			std::cout << "\n" << getName() << "::doDetection()::sending reliability = 0\nMaximum number of matches < 3";
		reliability = cvPoint (0,0);
	}

	point = cvPoint(pointx,pointy);

	// Mark the keypoints detected with plus signs:
	//showKeypoints(sideBySideImage, kpts);

	// Indicate the matches with lines:
	//showMatches(maximumNumberOfMatches);
}

// Main loop of the program
void Att::run(void)
{

	IplImage* result = outputImage;

    //fpsCalculation();
    if(bella=="Attention_Left"){
    	switch (appModeL) {
    	case TAKING_NEW_TEMPLATEL:
		if(debug)
			printf("\nTaking a new template (left)");
    		takeNewTemplateImage();
    		break;
    	case DETECTIONL:
		if(debug)
			printf("\nDetection (left)");
    		cvReleaseImage(&newFrameRGB);
    		takeNewFrame();
    		cvCopyImage(newFrameRGB, outputImage);
    		putImagesSideBySide(sideBySideImage, newFrameRGB, templateImageRGBFull);
    		doDetection();
    		result = sideBySideImage;

    		//SEND CVIMAGE RGB LEFT
 /*   		cv_templateRGB = converttemp2CVImage(templateImageGray);
    		cvTemplateOut.setBuffer(cv_templateRGB);
    		cvTemplateOut.out();

    		cv_fullRGB = convert2CVImage(newFrameGray);
    		cvFullOut.setBuffer(cv_fullRGB);
    		cvFullOut.out();
*/    		gettemp=true;

    		break;
    	case DO_NOTHINGL:
		if(debug)
			printf("\nDo nothing (left)");
    		cvReleaseImage(&newFrameRGB);
    		takeNewFrame();
    		cvCopyImage(newFrameRGB, outputImage);
    		break;
    	case ENDL:
    		return;
    	default:
    		break;
    	}
    }

    if(bella=="Attention_Right"){
    	switch (appModeR) {
    	case TAKING_NEW_TEMPLATER:
		if(debug)
			printf("\nTacking a new template (right)");
    		takeNewTemplateImage();
    		break;
    	case DETECTIONR:
		if(debug)
			printf("\nDetection (right)");
    		cvReleaseImage(&newFrameRGB);
    		takeNewFrame();
    		cvCopyImage(newFrameRGB, outputImage);
    		putImagesSideBySide(sideBySideImage, newFrameRGB, templateImageRGBFull);
    		doDetection();
    		result = sideBySideImage;

		gettemp=true;

    		break;
    	case DO_NOTHINGR:
		if(debug)
			printf("\nDo nothing (right)");
    		cvReleaseImage(&newFrameRGB);
    		takeNewFrame();
    		cvCopyImage(newFrameRGB, outputImage);
    		break;
    	case ENDR:
    		return;
    	default:
    		break;
    	}
    }


	showOutput(result);
	waitKeyAndHandleKeyboardInput(40,&one);


	if (debug)
	{
		std::cout << "\n" << getName() << "::doDetection()::sending point";
		printf("\nPoint: [%d,%d]",point.x,point.y);
		printf("\nReliability: [%d]",reliability.x);
	}

	/*Vector& Pointtarget = PointOutPort.prepare();
	Pointtarget.resize(5);

	Pointtarget[0] = point.x;
	Pointtarget[1] = point.y;
	Pointtarget[2] = reliability.x;
	Pointtarget[3] = 1.0;
	Pointtarget[4] = 0.0;

	PointOutPort.write();*/

	Bottle& Pointtarget = PointOutPort.prepare();
	Pointtarget.clear();

	Pointtarget.addDouble(point.x);//millimeters to meters
	Pointtarget.addDouble(point.y);//millimeters to meters
	Pointtarget.addDouble(reliability.x);//millimeters to meters
	Pointtarget.addString("LookAtStereoPixel");

	PointOutPort.write();

	pointsent=true;

}




CVImage* Att::convert2CVImage(IplImage* InputImage) {

		if(!InputImage) { std::cout<< getName() << "::convert2CVImage::ERROR::File does not exist!\n"; return NULL; }

	IplImage* tempimg = cvCloneImage(InputImage);

	if(!tempimg) { std::cerr<< getName() << "::convert2CVImage::ERROR opening file\n"; return NULL; }

	if(mp_cvimgL) {
		if(mp_cvimgL->width != tempimg->width || mp_cvimgL->height != tempimg->height || mp_cvimgL->ipl->nChannels != tempimg->nChannels || mp_cvimgL->ipl->depth != tempimg->depth) {
			delete mp_cvimgL;
			mp_cvimgL = NULL;
		}
	}

	if(!mp_cvimgL) {

		int cvMatType;
		if(tempimg->nChannels == 3){
			cvMatType = CV_8UC3;
		}
		else if(tempimg->nChannels == 1) {
			cvMatType = CV_8UC1;
		}

		mp_cvimgL = new CVImage(cvSize(tempimg->width, tempimg->height), cvMatType, 0);
		if(!mp_cvimgL) { std::cout<< getName() << "::convert2CVImage::ERROR::Failed to create new CVImage\n"; return NULL; }
	}

	int datasize = (tempimg->widthStep)*(tempimg->height);
	memcpy(mp_cvimgL->ipl->imageData, tempimg->imageData, datasize );

	cvReleaseImage(&tempimg);
	if(tempimg) { std::cout<< getName() << "::convert2CVImage::ERROR::Failed to release IplImage\n"; return NULL; }

	//if(debug) { std::cout << "LoadCVImage::load()::finished\n"; }
	return mp_cvimgL;
}


CVImage* Att::converttemp2CVImage(IplImage* InputImage) {

		if(!InputImage) { std::cout<< getName() << "::convert2CVImage::ERROR::File does not exist!\n"; return NULL; }

	IplImage* tempimg = cvCloneImage(InputImage);

	if(!tempimg) { std::cerr<< getName() << "::convert2CVImage::ERROR opening file\n"; return NULL; }

	if(mp_tempcvimgL) {
		if(mp_tempcvimgL->width != tempimg->width || mp_tempcvimgL->height != tempimg->height || mp_tempcvimgL->ipl->nChannels != tempimg->nChannels || mp_tempcvimgL->ipl->depth != tempimg->depth) {
			delete mp_tempcvimgL;
			mp_tempcvimgL = NULL;
		}
	}

	if(!mp_tempcvimgL) {

		int cvMatType;
		if(tempimg->nChannels == 3){
			cvMatType = CV_8UC3;
		}
		else if(tempimg->nChannels == 1) {
			cvMatType = CV_8UC1;
		}

		mp_tempcvimgL = new CVImage(cvSize(tempimg->width, tempimg->height), cvMatType, 0);
		if(!mp_tempcvimgL) { std::cout<< getName() << "::convert2CVImage::ERROR::Failed to create new CVImage\n"; return NULL; }
	}

	int datasize = (tempimg->widthStep)*(tempimg->height);
	memcpy(mp_tempcvimgL->ipl->imageData, tempimg->imageData, datasize );

	cvReleaseImage(&tempimg);
	if(tempimg) { std::cout<< getName() << "::convert2CVImage::ERROR::Failed to release IplImage\n"; return NULL; }

	//if(debug) { std::cout << "LoadCVImage::load()::finished\n"; }
	return mp_tempcvimgL;
}
