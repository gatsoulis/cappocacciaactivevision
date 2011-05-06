#ifndef Vergence_H
#define Vergence_H

//#include "ipp.h"
#include "cv.h"

#include "module.h"
#include "inputconnector.h"
#include "outputconnector.h"
#include "cvimage.h"
#include "csctype.h"
#include "gaborfilter.h"
//#include "cscgaborfilters.h"
//#include "convertRGBtoGRAY.h"
//#include "recdata.h"

#include <ctime>
#include <iostream> 
#include <fstream>

#define PI 3.14159

//#define Dimension 2
//#define OUTPUT_DIM 4

//#define USE_THREADS_VERGENCE


	class Vergence : public Module  {
		public: 
			Vergence(std::string name="Vergence");
			~Vergence();

			InputConnector<CVImage*> cvLeftGrayImageIn;
			InputConnector<CVImage*> cvRightGrayImageIn;
			InputConnector<TrackData*> trackIn;
			OutputConnector<VergenceData*> vergOut;

			void execute();

			void setParam(unsigned int gridsizehor = 5, unsigned int gridsizever = 1, unsigned int griddisphor = 12, unsigned int griddispver = 12);

			void setFilterParam(unsigned int L = 5, unsigned int D = 8, float f=1.0/sqrt(2.0), float kmax=1.7, float sigma = 2*3.1416, float N=1.0, std::vector<int>* filtersize = NULL);

			bool debug;
/*
			void setParam(int gridX = 5, int gridY = 1, int gridDispX = 12, int gridDispY = 12, int L = 5, int D = 8, float sigma = 2*PI, int* filterSize = NULL);
//			bool print_result;
			int shiftX, shiftY;	// for testing purpose only
			bool m_run;
			bool m_runningAvg;

*/
		private:

			VergenceData m_verg;

			unsigned int m_gridsizehor, m_gridsizever;
			unsigned int m_gridsize;
			unsigned int m_griddisphor, m_griddispver;
			unsigned int m_L, m_D, m_LD;

			GaborFilter m_GaborFilter;
			
			CSCVector** mdp_responses;

			CVImage* mp_cvleftgrayimg;
			CVImage* mp_cvrightgrayimg;

			CVImage* mp_cvleftgrayimgflt;
			CVImage* mp_cvrightgrayimgflt;
			
			float m_trackthreshold;
			
			float computesim(CSCVector* vec1, CSCVector* vec2);
			
			CvMat* mp_simmat;
			float m_simthres;

			CvPoint2D32f computephasedisplacement(CSCVector* vec1, CSCVector* vec2, int focus = 0);
		    float* mp_kjx;
			float* mp_kjy;
			
			bool m_runningavg;
			double m_alpha;
			CvPoint2D32f m_disp;

			ConvertFlt m_ConvertFltLeft;
			ConvertFlt m_ConvertFltRight;

			void normalize(CSCVector* vec);
			
			float computerunningavg(float newval, float oldval, float alpha);
			
			
/*
#ifdef USE_THREADS_VERGENCE
			pthread_barrier_t barrier_threads;
			pthread_mutex_t mutex_waitlock;
			pthread_cond_t  condition_startthreads;
			pthread_cond_t  condition_threadsended;
			pthread_mutex_t mutex_threadsended;
			static void *computeleft(void *);
			static void *computeright(void *);
			pthread_t m_threadleft;
			pthread_t m_threadright;
#else
#endif


			TrackData m_winner;

//			IplImage* mp_leftGrayImage;
//			IplImage* mp_rightGrayImage;
		
			void computePhaseDisplacement(int ind, int focus);
			float round(const float &number, const int num_digits);

			Ipp32fc *dataLeft, *dataRight, *dataTemp;
			Ipp32f *dataLeftAmpl, *dataRightAmpl;
			Ipp32f *dataLeftAngle, *dataRightAngle;

			float* absSim;

			int centerX, centerY;
			float *pdx;
			int gridX, gridY; // number of grid points
			int gridDispX, gridDispY;
			int L, D;
			int* filterSize;
			float sigma;
			bool m_initialized;

			CSCGaborFilters* gabor;
			
			int size;
			int m_width, m_height;			
			
			VergenceOutput* dataO;
			
			CvMat* mp_disparity;
			CvMat* mp_newdisparity;
			double m_alpha;
*/
	};


#endif
