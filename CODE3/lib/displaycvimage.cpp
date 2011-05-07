#include "displaycvimage.h"
#include "highgui.h"



DisplayCVImage::DisplayCVImage(std::string name, bool autoResize) : Module(name) {
	std::cout<<"\nConstructing " << getName() << "...";

	imageIn.setModule(this);
	cvImageIn.setModule(this);
//	pointIn.setModule(this);
//	point2In.setModule(this);

//	init = false;


	windowX = 0;
	windowY = 0;
	wndName = name;
	
/*	this->autoResize = autoResize;
	if(autoResize) cvNamedWindow( name.c_str(), 0);
	else cvNamedWindow( name.c_str(), 1);
*/
	//WHY	
	//displayImage = NULL;
	
	debug = false;

	delay = false;
	delaytime = 2;
	
//	mpFont = new CvFont;
//	cvInitFont(mpFont, CV_FONT_HERSHEY_SIMPLEX, 1.0f, 1.0f, 0.0, 2, 8);

	std::cout<<"finished.";
}
 
DisplayCVImage::~DisplayCVImage() {
	std::cout<<"\nDestructing " << getName() << "...";
	
	//WHY	
	//if(displayImage) delete displayImage;
	
	//cvDestroyWindow(wndName.c_str());
//	delete mpFont;
	std::cout<<"finished.";
}

void DisplayCVImage::createwin(){

	this->autoResize = autoResize;
	if(autoResize) cvNamedWindow( wndName.c_str(), 0);
	else cvNamedWindow( wndName.c_str(), 1);

}
void DisplayCVImage::destroywin(){

	cvDestroyWindow(wndName.c_str());
}

void DisplayCVImage::execute() {

	//std::cout << getName() << "::execute()::\n";

	IplImage* img = imageIn.getBuffer();
	CVImage* cvimg = cvImageIn.getBuffer();

//	std::cout << getName() << "::execute()::cvimg = " << cvimg << ", cvimg->ipl = " << cvimg->ipl << "\n";
//	std::cout << getName() << "::execute()::cvimg->width = " << cvimg->width << ", cvimg->height = " << cvimg->height << "\n";

	if (img == NULL && cvimg == NULL) { if(debug) std::cerr << getName() << "::ERROR::execute()::imageIn and cvImageIn are both NULL!\n"; return; }
	if (img == NULL) img = cvimg->ipl;

//	std::cout << getName() << "::execute()::img = " << img << "\n";

	if(autoResize) cvResizeWindow( wndName.c_str(), img->width+10, img->height+10 );
	
/*
	CvPoint* point1 = pointIn.getBuffer();
	CvPoint* point2 = point2In.getBuffer();
	CvRect* cvrect = rectIn.getBuffer();
	TrackData* cvtrack = trackIn.getBuffer();

	//if(debug) std::cout << getName() << "::execute()::started\n";

	if (point1 != NULL || point2 != NULL || cvrect != NULL || cvtrack != NULL) {
		if(displayImage != NULL && (displayImage->width != img->width || displayImage->height != img->height)){
			delete displayImage;
			displayImage = NULL;
		}

		if(displayImage == NULL){
			displayImage = new CVImage(cvSize(img->width, img->height), CV_8UC3, 0);
		}
		if(img->nChannels == 1){
			cvCvtColor(img, displayImage->ipl, CV_GRAY2BGR);
		}
		else{
			cvCopy(img, displayImage->ipl);
		}
		img = displayImage->ipl;
		cvimg = displayImage;
	}

	if (cvtrack != NULL && cvtrack->reliability > 0.0 ) {
//		std::cout << getName() << "::execute()::cvtrack != NULL. winnerPos = [" << cvtrack->winnerPos.x << "," << cvtrack->winnerPos.y << "]\n";
		point1 = &(cvtrack->winnerPos);
	}
	else{
//		std::cout << getName() << "::execute()::cvtrack = " << cvtrack << "\n";
//		std::cout << getName() << "::execute()::cvtrack->reliability = " << cvtrack->reliability << "\n";
	}


	if (point1 != NULL || point2 != NULL) {

		CvScalar color1hi, color1lo, color2hi, color2lo;
	 	if (img->nChannels == 3) {
			color1hi = CV_RGB(255,100,100);
			color1lo = CV_RGB(255,0,0);
			color2hi = CV_RGB(0,0,255);
			color2lo = CV_RGB(50,50,255);
		}
		else {
			color1hi = color2lo = CV_RGB(255,255,255);
			color1lo = color2hi = CV_RGB(0,0,0);
		}

		// draw a crosshair
		if(point1 != NULL){
			cvLine( img, cvPoint(point1->x-1, point1->y-CROSS_HAIR_SIZE), cvPoint(point1->x-1, point1->y+CROSS_HAIR_SIZE),color1lo, 1);
			cvLine( img, cvPoint(point1->x+1, point1->y-CROSS_HAIR_SIZE), cvPoint(point1->x+1, point1->y+CROSS_HAIR_SIZE),color1lo, 1);
			cvLine( img, cvPoint(point1->x,   point1->y-CROSS_HAIR_SIZE), cvPoint(point1->x,   point1->y+CROSS_HAIR_SIZE),color1hi, 1);
	
			cvLine( img, cvPoint(point1->x-CROSS_HAIR_SIZE, point1->y-1), cvPoint(point1->x+CROSS_HAIR_SIZE, point1->y-1),color1lo, 1);
			cvLine( img, cvPoint(point1->x-CROSS_HAIR_SIZE, point1->y+1), cvPoint(point1->x+CROSS_HAIR_SIZE, point1->y+1),color1lo, 1);
			cvLine( img, cvPoint(point1->x-CROSS_HAIR_SIZE, point1->y  ), cvPoint(point1->x+CROSS_HAIR_SIZE, point1->y  ),color1hi, 1);
		}

		// draw a crosshair
		if(point2 != NULL){
			cvLine( img, cvPoint(point2->x-1, point2->y-CROSS_HAIR_SIZE), cvPoint(point2->x-1, point2->y+CROSS_HAIR_SIZE),color2lo, 1);
			cvLine( img, cvPoint(point2->x+1, point2->y-CROSS_HAIR_SIZE), cvPoint(point2->x+1, point2->y+CROSS_HAIR_SIZE),color2lo, 1);
			cvLine( img, cvPoint(point2->x,   point2->y-CROSS_HAIR_SIZE), cvPoint(point2->x,   point2->y+CROSS_HAIR_SIZE),color2hi, 1);
	
			cvLine( img, cvPoint(point2->x-CROSS_HAIR_SIZE, point2->y-1), cvPoint(point2->x+CROSS_HAIR_SIZE, point2->y-1),color2lo, 1);
			cvLine( img, cvPoint(point2->x-CROSS_HAIR_SIZE, point2->y+1), cvPoint(point2->x+CROSS_HAIR_SIZE, point2->y+1),color2lo, 1);
			cvLine( img, cvPoint(point2->x-CROSS_HAIR_SIZE, point2->y  ), cvPoint(point2->x+CROSS_HAIR_SIZE, point2->y  ),color2hi, 1);
		}
	}

	if (cvrect != NULL) {
		CvScalar color1hi;
	 	if (img->nChannels == 3) {
			color1hi = CV_RGB(255, 0, 0);
		}
		else {
			color1hi = CV_RGB(255,255,255);
		}
		CvPoint pt1 = cvPoint(cvrect->x, cvrect->y);
		CvPoint pt2 = cvPoint(cvrect->x + cvrect->width, cvrect->y + cvrect->height);
		cvRectangle( img, pt1, pt2, color1hi, 1, 8, 0);
	}
	
	std::string* label = labelIn.getBuffer();
	if(label != NULL) {
		cvPutText(img, label->c_str(), cvPoint(img->width/2 - 50, img->height - 50), mpFont, CV_RGB(255, 0, 0));
	}

	CSCRecQuery* rec = recogIn.getBuffer();
	unsigned int* depthind = depthIndexIn.getBuffer();
	CvPoint2D32f* disp = dispIn.getBuffer();
	if(rec != NULL) {
		ostringstream temp;
		temp << depthind;
		std::string text = rec->name + ":" + temp.str();
		cvPutText(img, text.c_str(), cvPoint(img->width/2 - 50, img->height - 50), mpFont, CV_RGB(255, 0, 0));
	}
*/

	//std::cout << getName() << "::execute()::test 1\n";

	cvShowImage(wndName.c_str(), img);

	//std::cout << getName() << "::execute()::test 2\n";

	if(delay) cvWaitKey(delaytime); // This line takes a lot of time !!!

/*
	imageOut.setBuffer(img);
	imageOut.out();

	cvImageOut.setBuffer(cvimg);
	cvImageOut.out();
*/
}

int DisplayCVImage::moveWindow(int x, int y) {
//	int width = imageIn.getBuffer()->width;
	cvMoveWindow(wndName.c_str(), x, y);
	return 0;
}

void DisplayCVImage::setParam(bool d, unsigned int t) {
	delay = d;
	delaytime = t;
}

