#ifndef CropResize_H
#define CropResize_H

//#include "ipp.h"
#include "cv.h"

#include "module.h"
#include "inputconnector.h"
#include "outputconnector.h"
#include "cvimage.h"

#include <iostream> 


	class CropResize : public Module  {

		public: 

			CropResize(std::string name="CropResize");
			~CropResize();

			void execute();

			InputConnector<CVImage*> cvImageIn;
			InputConnector<CvRect*> rectIn;
			InputConnector<CvSize*> sizeIn;

			OutputConnector<CVImage*> cvImageOut;

			void fixOutputSize(const CvSize& size) { m_outputsize = size; }

			void setParam(int interpolation = CV_INTER_LINEAR) { m_interpolation = interpolation; }

			bool debug;

		private:

			CvRect m_inputrect;
			CVImage* mp_cvimage;
			CvSize m_outputsize;
			/*	
				Interpolation method:

				CV_INTER_NN - nearest-neigbor interpolation,
				CV_INTER_LINEAR - bilinear interpolation (used by default)
				CV_INTER_AREA - resampling using pixel area relation. It is preferred method for image decimation that gives moire-free results. In case of zooming it is similar to CV_INTER_NN method.
				CV_INTER_CUBIC - bicubic interpolation. 
			*/
			int m_interpolation;  
			
			//CvRect temprect;
	};


#endif
