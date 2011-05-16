#ifndef Resize_H
#define Resize_H

//#include "ipp.h"
#include "cv.h"

#include "module.h"
#include "inputconnector.h"
#include "outputconnector.h"
#include "cvimage.h"

#include <iostream> 


	class Resize : public Module  {

		public: 

			typedef enum {CROP = 0, RESCALE = 1} mode;

			Resize(std::string name="Resize", unsigned int mode = RESCALE, bool addnoise = false);
			~Resize();

			void execute();

			InputConnector<CVImage*> cvImageIn;
			InputConnector<CvRect*> rectIn;
			InputConnector<CvSize*> outputSizeIn;
			InputConnector<float*> scaleIn;

			OutputConnector<CVImage*> cvImageOut;
			OutputConnector<CvRect*> rectOut;

			void fixOutputSize(const CvSize& size);

			void setParam(unsigned int mode, bool addnoise = false);

			bool debug;


		private:

			CvRect m_outputrect;
			CVImage* mp_cvimage;
			//CvRect* mp_fixRect;
			CvSize* mp_fixedOutputSize;
			unsigned int m_mode;

			/*	
				Interpolation method:

				CV_INTER_NN - nearest-neigbor interpolation,
				CV_INTER_LINEAR - bilinear interpolation (used by default)
				CV_INTER_AREA - resampling using pixel area relation. It is preferred method for image decimation that gives moire-free results. In case of zooming it is similar to CV_INTER_NN method.
				CV_INTER_CUBIC - bicubic interpolation. 
			*/
			int m_interpolation;  

			bool m_addnoise;
			void addNoise(); // add white noise to unfilled areas
			CvRNG m_rand;
			
			CvRect temprect;
	};


#endif
