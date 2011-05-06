#ifndef CueContrastKernel_H
#define CueContrastKernel_H

#include "ipp.h"
#include "cv.h"

#include "cue.h"
//#include "inputconnector.h"
#include "outputconnector.h"
#include "csctype.h"
#include "cropresize.h"
#include "CSCVector.h"
#include "drawvector.h"

#include <string>
#include <vector>
#include <sstream>

//#define MIN(X,Y) ((X) < (Y) ? : (X) : (Y))
//#define MAX(X,Y) ((X) > (Y) ? : (X) : (Y))


//! class CueContrastKernel  -- Color cue based on local color histogram. (adaptive?)
/*! class CueContrastKernel  -- Color cue based on local color histogram.

		written 03/2005 by Hyundo Kim
*/
	class CueContrastKernel : public Cue
	{

		public: 

			typedef enum {SIMPLE=0, WBGHIST=1, WKALMAN=2, BGHISTKALMAN=4} operation_modes;

			CueContrastKernel(std::string name = "CueContrastKernel");
			~CueContrastKernel();

			void execute();
			void adapt();
			void initialize();
			void setParam(int opmode = SIMPLE, int maxIter = 20, std::vector<int>* binSize = NULL, std::vector<int>* contrastScale = NULL);
			bool debug;

			OutputConnector<CVImage*> cvImage1Out;
			OutputConnector<CVImage*> cvImage2Out;
			OutputConnector<CVImage*> cvImage3Out;

			OutputConnector<CVImage*> cvModelOut;
			OutputConnector<CVImage*> cvCandidateOut;
			OutputConnector<CVImage*> cvTemplateOut;
			OutputConnector<CVImage*> cvMaskOut;
/*
			OutputConnector<CSCVector*> vecModelOut;
			OutputConnector<CVImage*> cvHistImageOut;
*/
			bool draw;
			
			void test();

		private:

			int m_opmode;
			int m_colormode;
			int m_maxIter;
			std::vector<int> m_vecbinsize;
			int** mp_LUT;

			unsigned int m_dimData;
			int m_totalnumbins;
			bool m_setParam;

			CVImage* mp_cvimg1;
			CVImage* mp_cvimg2;
			CVImage* mp_cvimg3;
			CVImage* mp_cvhsvimg;
			CVImage** mdp_cvimg;

			void obtainInput();
			
			CSCVector m_target_model;
			CSCVector* mp_target_candidate;
			CSCVector m_target_orig;

			DrawVector* mpDrawModel;
			DrawVector* mpDrawCandidate;
			
			bool m_forceMinSize;
			int m_minSize;
			//CvSize* mp_objSize;
			CSCVector m_weight;
			CvSize m_objsize;
			bool m_forceNormSize;
			CvSize m_objsizenorm;
			int m_normsize;

			void cropNResize(CvPoint2D32f* pos, CvSize* size, float hval);
			CropResize** mdpCropResize;
			CVImage** mdp_cvcroppedimg;
			CVImage* mp_cvmaskimg;
			void drawEllipse(CVImage* maskimg, CvPoint* maskcenter, CvSize* maskaxis);
			void calculateProbability(CSCVector* prob, float hval);
			float computeSimilarity(CSCVector* p, CSCVector* q); // Bhattacharyya coefficient
			void drawGrayBall(CvPoint2D32f* pos, CvSize* size);

			unsigned char* mp_pixeldata;
			int* mp_index;
			int findIndex(unsigned char* input);
			float computeKernel(float dist);
			float c_d;
			float d;
			float g_index;

			CvPoint2D32f m_y0;
			float* mp_rel;
			CvPoint2D32f* mp_y1;
			void runkernel(CvPoint2D32f* nextpos, CSCVector* prob_cand, float* rel, float hval);
			void computeWeights(CSCVector* p);
			void findNextLocation(CvPoint2D32f* pos, float hval);
			bool checkCondition(CvPoint2D32f* pos, CvPoint2D32f* nextpos, int count);
			float h;
			float epsilon;
			int findmax(float* a, int size); // returns index


			CVImage* mp_cvminimg;
			CVImage* mp_cvmaximg;
			std::vector<int> m_contrastscale;


	};

#endif
