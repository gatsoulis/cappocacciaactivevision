#ifndef GaborFilter_H
#define GaborFilter_H

//#include "module.h"
#include "cv.h"
//#include "inputconnector.h"
//#include "outputconnector.h"
#include "cvimage.h"
#include "CSCVector.h"
//#include "imagecropresize.h"
//#include "imagecrop2.h"
//#include "CSCEllipticRegions.h"
//#include "CSCDescriptors.h"
#include "convertflt.h"

#include <vector>
#include <sstream>
//#include <cmath>
#include <fstream>
//#include <pthread.h>



	//typedef float float;

	class GaborFilter {
	
		public:
		
			typedef enum {ABSOLUTE = 0, COMPLEX = 1, AMPLITUDEPHASE = 2} computemodes;
		
			GaborFilter();
			~GaborFilter();

			void setParam(unsigned int L=5, unsigned int D=8, float f=1.0/sqrt(2.0), float kmax=1.7, float sigma=2*3.1416, float N=1.0, std::vector<int>* filtersize = NULL);

			void setMode(unsigned int computemode) { m_computemode = computemode; }

			void generatefilters();
			
			CVImage** getfiltersreal() { return mdp_filtersreal; }
			CVImage** getfiltersimag() { return mdp_filtersimag; }
			
			void printfilters();

			void display(unsigned int index);

			float conv2DPointReal(CVImage* img, CvPoint* pos, unsigned int index);
			float conv2DPointImag(CVImage* img, CvPoint* pos, unsigned int index);
			
			float conv2DPointMag(CVImage* img, CvPoint* pos, unsigned int index);
			float conv2DPointPha(CVImage* img, CvPoint* pos, unsigned int index);

			float* compute(CVImage* img, CvPoint* pos, float* dst);
			float* computereal(CVImage* img, CvPoint* pos, float* dst);
			float* computeimag(CVImage* img, CvPoint* pos, float* dst);
			float* computemag(CVImage* img, CvPoint* pos, float* dst);
			float* computepha(CVImage* img, CvPoint* pos, float* dst);

			CSCVector* compute(CVImage* img, CvPoint* pos, CSCVector* dst);
			CSCVector* computereal(CVImage* img, CvPoint* pos, CSCVector* dst);
			CSCVector* computeimag(CVImage* img, CvPoint* pos, CSCVector* dst);
			CSCVector* computemag(CVImage* img, CvPoint* pos, CSCVector* dst);
			CSCVector* computepha(CVImage* img, CvPoint* pos, CSCVector* dst);
			
			bool debug;
			
			unsigned int size();

/*
			void execute();
			
			InputConnector<CVImage*> cvImageIn;
			InputConnector<CSCEllipticRegions* > regionsIn;

			OutputConnector<CVImage*> cvImageOut;
			OutputConnector<CSCDescriptors*> dataOut;
			
			
			void setDescriptorType(const std::string& type) { m_descriptor.setDescriptorType(type); }

			bool debug;


			CVImage* getFilterImageReal(unsigned int index);
			CVImage* getFilterImageImag(unsigned int index);
*/
		private:

			unsigned int m_computemode;
			unsigned int m_L, m_D, m_LD;
			float m_f, m_kmax, m_sigma, m_N;
			std::vector<int> mvec_filtersize;

			CVImage** mdp_filtersreal;
			CVImage** mdp_filtersimag;
			
			std::string m_windownamereal;
			std::string m_windownameimag;
			bool m_windowcreated;
			IplImage* mp_displayreal;
			IplImage* mp_displayimag;

			IplImage** mdp_responsesreal;
			IplImage** mdp_responsesimag;
			
			ConvertFlt m_ConvertFlt;
			
			float* mp_result;
			CSCVector* mp_resultvector;
			
/*
			bool m_norm;

			CSCDescriptors m_descriptor;

			CvSize m_imagesize;
			int* mp_filtersize;
			bool m_init;

			void computeFilter();
			CvMat** mdp_filtersReal;
			CvMat** mdp_filtersImag;
			CvMat** mdp_responsesReal;
			CvMat** mdp_responsesImag;

			Resize* mpc_ResizeBiggerImage;
			CVImage* mp_biggercvimg;
			CvSize m_biggerimagesize;
			unsigned int m_offsetx, m_offsety;

			void convertTo32F();
			IplImage* mp_img32f;

			void rescaleImage(float* ellipse);
			IplImage* mp_rescaled32f;

			
*/

	};


#endif


