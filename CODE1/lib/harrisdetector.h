#ifndef HarrisDetector_H
#define HarrisDetector_H

//#include "ipp.h"
#include "cv.h"

#include "module.h"
#include "inputconnector.h"
#include "outputconnector.h"
#include "cvimage.h"
//#include "recdata.h"
#include "CSCEllipticRegions.h"

#include <iostream>


  
	class HarrisDetector : public Module{

		public:
			HarrisDetector(std::string name="HarrisDetector");  
			~HarrisDetector(); 

			void setParam(unsigned int cornerCount, double qualityLevel, double minDistance);

			InputConnector<CVImage*> cvGrayImageIn;
			InputConnector<CvRect*> roiIn;
			//InputConnector<CVImage*> maskIn;
			OutputConnector<CSCEllipticRegions*> regionsOut; 
      
			void execute(); 
	  
			bool debug;
			bool m_fixBlobSize;

		private:

			IplImage* mp_eig_image;
			IplImage* mp_temp_image;
			CvPoint2D32f* mp_corners;

			//float* m_temp1; //temporary storage data
			//float* m_temp2;
			//CvPoint2D32f* m_cornerTemp;
			//int m_dataSize;  //Size of allocated memory space in bytes
			unsigned int m_cornerCount; //maximum number of corners returned
			double m_qualityLevel;  //threshold for corners
			double m_minDistance; // minimum distance between corners
			CSCEllipticRegions* mp_regions;
			//CvRect imgroi;
			//CVImage* grayimg;
			//IplImage *tempImg1, *tempImg2;

			int m_block_size;
			int m_use_harris;
			double m_k;

			float m_fixed_blob_size;
      
	};


#endif
