#ifndef DetectObject_H
#define DetectObject_H

//#include "ipp.h"
#include "cv.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "module.h"
#include "inputconnector.h"
#include "outputconnector.h"
#include "cvimage.h"
#include "csctype.h"
#include "harrisdetector.h"
#include "gabordescriptor.h"
#include "CSCFeature.h"
#include "CSCEllipticRegions.h"
#include "CSCDescriptors.h"
//start Gab's add
#include "dogdetector.h"
//end Gab's add

/*
//#include "disparitydrawer2.h"
#include "convertRGBtoGRAY.h"
#include "cornerfinder2.h"
#include "cornerdrawer2.h"
#include "rfeature.h"
#include "cscdrawdisparity2.h"
#include "fastgabor.h"
#include "gaborresponse.h"
#include "rhinterface2.h"
*/
#include <vector>
#include <set>

#include <ctime>
#include <iostream> 
#include <fstream>
//#include <pthread.h>

//#define Use_FastGabor
//#define USE_PTHREADS

#define PI 3.14159265

//! class DetectObject  -- Module for detecting presence of an object in a scene based on stereo (depth) information
/*! class DetectObject  -- Module for detecting presence of an object in a scene based on stereo (depth) information
    revised 03/2006 by Hyundo Kim
*/
	class DetectObject : public Module  {
		public: 
		
			typedef enum {EXHAUSTIVE = 0, BOUNDARY_BOX = 1, BOUNDARY_VERTICAL = 2} opmodes;
		
			// Parameters 
			// 320x240 : 0, 0.96, 5, 3
			// 640x480 : 0, 0.93, 11, 5

			//! DetectObject(std::string name="DetectObject")  -- the standard constructor
			/*! DetectObject(std::string name="DetectObject")  -- the standard constructor */
			DetectObject(std::string name="DetectObject");

			//! ~DetectObject()  -- the destructor
			/*! ~DetectObject()  -- the destructor */
			~DetectObject();

			void setParam(float simthres = 0.90, float hordispthres = 320.0, float verdispthres = 19.0, unsigned int opmode = EXHAUSTIVE);

			//! void execute()  -- main function that does the job
			/*! void execute()  -- main function that does the job */
			void execute();

//added gabriele & pramod

			void calculatesizeL();
			void calculatesizeR();
//end

			//InputConnector<CVImage*> cvLeftImageIn; 
			//InputConnector<CVImage*> cvRightImageIn; 
			InputConnector<CVImage*> cvLeftGrayImageIn; 
			InputConnector<CVImage*> cvRightGrayImageIn; 

		    OutputConnector<CSCEllipticRegions*> regionsLeftOut; 
		    OutputConnector<CSCEllipticRegions*> regionsRightOut; 

		    //OutputConnector<CVImage*> cvDisparityImageOut; 
	
//		    OutputConnector<TrackData*> winner160x120Out; // In 160x120 resolution
//		    OutputConnector<TrackData*> winner320x240Out; // In 320x240 resolution
//		    OutputConnector<TrackData*> winner640x480Out; // In 640x480 resolution
		    OutputConnector<TrackData*> trackOutL;
		    OutputConnector<TrackData*> trackOutR;
/*
			bool m_detect;
			bool m_drawResult;
			bool m_drawIP;
			int m_noIPthreshold;

			float m_horizontalDisp;
			float m_verticalDisp;

			int m_ignore;
			int m_opmode;


			bool forceMinSize;
			int minSize;

			bool detected() { return m_detect; }
*/
			bool debug;
			bool forceMaxSizeL;
			bool forceMaxSizeR;
			int m_maxsize;
//start Gab's add
			bool Dog;
			bool Harris;
//end Gab's add

		private:

			unsigned int m_opmode;
			TrackData m_trackL;
			TrackData m_trackR;
//start adds gabriele
			CvPoint  maxPosL;
			CvPoint  minPosL;
			CvPoint  maxPosR;
			CvPoint  minPosR;
//end adds gabriele
			float m_simthres, m_hordispthres, m_verdispthres;
			int m_width, m_height;
//start Gab's add
			DogDetector* mp_DogLeft;
			DogDetector* mp_DogRight;
			float squarelenL;
			float squarelenR;
			bool firstL;
			bool firstR;
			double* arrayL;
			double* arrayR;
			int oriCnt;
//end Gab's add

			HarrisDetector* mp_HarrisLeft;
			HarrisDetector* mp_HarrisRight;
			CSCEllipticRegions* mp_leftregions;
			CSCEllipticRegions* mp_rightregions;

			CSCEllipticRegions m_leftregiondetect;
			CSCEllipticRegions m_rightregiondetect;

			GaborDescriptor* mp_GaborLeft;
			GaborDescriptor* mp_GaborRight;
			CSCDescriptors* mp_leftdesc;
			CSCDescriptors* mp_rightdesc;

			void extractinterestregions();
			CVImage* mp_cvleftgrayimg;
			CVImage* mp_cvrightgrayimg;

			void computegabordescriptors();

			unsigned int compareleftright();
			CSCFeature* mp_featleft;
			CSCFeature* mp_featright;

			CvMat* mp_simmat;
			CvMat* checkmat;
			double m_minval, m_maxval;
			CvPoint m_minloc, m_maxloc;
			std::vector<unsigned int> mvec_indexesonleft;
			std::vector<unsigned int> mvec_indexesonright;
			std::vector<float> mvec_hordisp;
			std::vector<float> mvec_verdisp;

			bool checkcondition(unsigned int points);
			unsigned int m_pointsthres;
			unsigned int m_detectthres, m_detectcounter;




/*
			bool endThread;
			pthread_mutex_t mutex_threadright;
			pthread_mutex_t mutex_rightwaitlock;
			pthread_mutex_t mutex_threadleft;
			pthread_mutex_t mutex_leftwaitlock;
			pthread_cond_t  condition_startthread;

			static void *computeleft(void *);
			static void *computeright(void *);
			pthread_t thread1, thread2;

			void computeleftnothread();
			void computerightnothread();

			void computeVariance();
//			float scale;

			CVImage* mp_cvleftrgbimg;
			CVImage* mp_cvrightrgbimg;

			CVImage* mp_disparityimg;

			int m_width, m_height;
			float m_thres;


			CornerFinder2* mp_CornerFinderLeft;
			CornerFinder2* mp_CornerFinderRight;
			CornerDrawer2* mp_CornerDrawerLeft;
			CornerDrawer2* mp_CornerDrawerRight;
			void drawDisparity();
			//DisparityDrawer2* mp_DisparityDrawer;
			CSCDrawDisparity2* mp_DisparityDrawer;

			InterestPoints* mp_leftInterestPoints;
			InterestPoints* mp_rightInterestPoints;

			void extractGaborResponse();

			int m_stepResponse;

#ifdef Use_FastGabor
			FastGabor* mp_GaborLeft;
			FastGabor* mp_GaborRight;
#else
			GaborResponse* mp_GaborLeft;
			GaborResponse* mp_GaborRight;
#endif

			RecData* mp_leftGaborResponses;
			RecData* mp_rightGaborResponses;
			rfeature* mp_featureLeft;
			rfeature* mp_featureRight;
			int m_sizeResponsesLeft, m_sizeResponsesRight;
			float* mp_responsesLeft;
			float* mp_responsesRight;
			float* mp_magLeft;
			float* mp_magRight;

			int m_maxSizeResponsesLeft, m_maxSizeResponsesRight;
			int* mp_classified;
			float* mp_disparityx;
			float* mp_disparityy;

			void compareResponses();
			//float findMax(float* data, int size, int* index);
			//float* mp_simmat;

			void checkCondition();
			int m_count;
			int m_count2;
			Ipp32f* IP_x;
			Ipp32f* IP_y;
			Ipp32f* median_x;
			Ipp32f* median_y;

//			TrackData* mp_winnerData160x120;
//			TrackData* mp_winnerData320x240;
//			TrackData* mp_winnerData640x480;
*/
	};


#endif
