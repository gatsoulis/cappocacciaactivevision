#ifndef objectComparison_H
#define objectComparison_H

#include "cv.h"
#include "highgui.h"
#include "cxcore.h"
#include "camera.h"

#include "module.h"
#include "inputconnector.h"
#include "outputconnector.h"
#include "cvimage.h"
#include "csctype.h"
#include "CSCEllipticRegions.h"
#include "cvimage.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#define CROSS_HAIR_SIZE 3
using namespace std;

  
	class objectCompare : public Module{

		public:
		objectCompare(std::string name="obj");
			~objectCompare();
			void execute();
			void saveImg();
			void readImg(int itr);
			void release();
			void saveRect(CvRect savRect);
			void detectionBlob(int iterationObjectSal);
			CVImage* convert2CVImageLeft(IplImage* InputImage);
			IplImage* salimg;
			CVImage* image2;

			InputConnector<CVImage*> cvGrayImageIn;
			OutputConnector<CVImage*> cvsavedImageOut;
			//IplImage** savedImages;// = cvCreateImage(cvSize(320,240),8,1);
			int count;
			int countMatching;
			int iterationObject;

			InputConnector<CSCEllipticRegions*> regionsInCamera;
			InputConnector<CSCEllipticRegions*> regionsInDatabase;

			float objLocX;
			float objLocY;

			OutputConnector<int*> arrayOut;

			bool debug;

		private:

			int* array;

			CVImage* cvi;
			CVImage* cvgrayimg;
			vector<CVImage*> savedImages;
			vector<CvRect> savedRectangles;
			IplImage* inputImage;
			IplImage* inputImage2;
  };


#endif

