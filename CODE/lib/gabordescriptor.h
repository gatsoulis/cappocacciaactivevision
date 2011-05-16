#ifndef GaborDescriptor_H
#define GaborDescriptor_H

#include "module.h"
#include "cv.h"
#include "inputconnector.h"
#include "outputconnector.h"
#include "cvimage.h"
//#include "csctype.h"
//#include "imagecropresize.h"
//#include "imagecrop2.h"
#include "CSCEllipticRegions.h"
#include "CSCDescriptors.h"
#include "resize.h"
#include "convertflt.h"
#include "gaborfilter.h"

#include <vector>
#include <sstream>
//#include <iostream>
#include <fstream>
//#include <pthread.h>



	typedef float GDDataType;

	class GaborDescriptor : public Module {
	
		public:
			GaborDescriptor(std::string name = "GaborDescriptor");
			~GaborDescriptor();
			void execute();
			
			InputConnector<CVImage*> cvGrayImageIn;
			InputConnector<CSCEllipticRegions* > regionsIn;

			OutputConnector<CVImage*> cvImageOut;
			OutputConnector<CSCDescriptors*> dataOut;
			
			void setParam(unsigned int L=5, unsigned int D=8, float f=1.0/sqrt(2.0), float kmax=1.7, float sigma=2*3.1416, float N=1.0, bool norm = true, std::vector<int>* filtersize = NULL);
			
			void setDescriptorType(const std::string& type) { m_descriptor.setDescriptorType(type); }

			bool debug;

/*
			CvMat* getFilterReal(unsigned int index);
			CvMat* getFilterImag(unsigned int index);
			void setNormalize(bool norm) { m_norm = norm; } 
			std::string type() { return "GaborFilters"; }
			void writeToFile(std::string filename = "untitled.desc");
			bool readFromFile(std::string filename);

			void printFilterReal(unsigned int index);
			void printFilterImag(unsigned int index);

			CVImage* getFilterImageReal(unsigned int index);
			CVImage* getFilterImageImag(unsigned int index);
*/
			CVImage* getFilterImageReal(unsigned int index);

		private:

			bool m_norm;

			CSCDescriptors m_descriptor;

			unsigned int m_L, m_D;
			GDDataType m_f, m_kmax, m_sigma, m_N;
			CvSize m_imagesize;
			std::vector<int> mvec_filtersize;
			bool m_init;

/*
			void computeFilter();
			CvMat** mdp_filtersReal;
			CvMat** mdp_filtersImag;
			CvMat** mdp_responsesReal;
			CvMat** mdp_responsesImag;
*/
			Resize* mpc_ResizeBiggerImage;
//			CVImage* mp_biggercvimg;
			CvSize m_biggerimagesize;
			int m_offsetx, m_offsety, m_gap;

//			void convertTo32F();
//			IplImage* mp_img32f;
			ConvertFlt m_ConvertFlt;
			CVImage* mp_cvgrayimgflt;

//			void rescaleImage(float* ellipse);
//			IplImage* mp_rescaled32f;

//			GDDataType conv2DPoint(IplImage* input, unsigned int index);
			
//			CVImage* mp_filterimgreal;
//			CVImage* mp_filterimgimag;
			
			CVImage* mp_filterimgreal;
			CvMat** mdp_filtersReal;
			
			void compute();
			GaborFilter* mp_GaborFilter;
			CVImage* mp_rescaledimg;
			CSCEllipticRegions* mp_regions;


	};


#endif


