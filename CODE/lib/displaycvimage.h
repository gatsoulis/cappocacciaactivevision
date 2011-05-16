#ifndef DisplayCVImage_H
#define DisplayCVImage_H

#include "cv.h"
#include "highgui.h"

#include "module.h"
#include "inputconnector.h"
#include "outputconnector.h"
#include "cvimage.h"

//#include "csctype.h"
//#include "cscrecdata.h"

#include <string>
//#include <sstream>

//#define CROSS_HAIR_SIZE 3



//! class DisplayCVImage  -- DisplayCVImage Module. Connectors: InputConnector<IplImage*> imageIn
/*! class DisplayCVImage  -- DisplayCVImage Module. Connectors: InputConnector<IplImage*> imageIn
*/
class DisplayCVImage : public Module
{
	public:
	
		DisplayCVImage(std::string name="DisplayCVImage", bool autoResize = true);
		~DisplayCVImage();

		InputConnector<IplImage*> imageIn;
		InputConnector<CVImage*> cvImageIn;

/*
		InputConnector<CvPoint*> pointIn; // used for drawing a crosshair at that position
		InputConnector<CvPoint*> point2In;
		InputConnector<CvRect*> rectIn;
		InputConnector<TrackData*> trackIn;
		InputConnector<std::string* > labelIn;

//		InputConnector<CSCRecQuery* > recogIn;
//		InputConnector<CvPoint2D32f* > dispIn;
//		InputConnector<unsigned int* > depthIndexIn;

		OutputConnector<IplImage*> imageOut;
		OutputConnector<CVImage*> cvImageOut;
*/
		void execute();

		void createwin();

		void destroywin();

		int moveWindow(int x=0, int y=0);
		
		void setParam(bool d, unsigned int t = 2);

		bool debug;

	private:

		//CVImage* displayImage;
	
		bool delay;
		unsigned int delaytime;

		std::string wndName;
//		bool init;
		int windowX, windowY;
		bool autoResize;
		
//		CvFont* mpFont;

};


#endif
