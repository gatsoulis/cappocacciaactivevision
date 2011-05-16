#ifndef DrawCross_H
#define DrawCross_H

#include "cv.h"

#include "module.h"
#include "inputconnector.h"
#include "outputconnector.h"
#include "cvimage.h"
#include "csctype.h"
#include "CSCEllipticRegions.h"

#include <iostream>

#define CROSS_HAIR_SIZE 3


  
	class DrawCross : public Module{

		public:
			DrawCross(std::string name="DrawCross", bool sameimage = false);
			~DrawCross(); 
			void execute(); 

			InputConnector<CVImage*> cvImageIn;
			InputConnector<CvPoint*> pointIn;
			InputConnector<CvPoint2D32f*> pointFltIn;
			InputConnector<CSCEllipticRegions*> regionsIn;
			InputConnector<TrackData*> trackIn;

			OutputConnector<CVImage*> cvImageOut; 

			void setColor(CvScalar color); 

			bool debug;

		private:

			CvScalar m_color;
			CVImage* mp_cvimg;

			int m_thickness;
			
			bool m_sameimage;

			CvScalar m_colorpt;

  };


#endif

