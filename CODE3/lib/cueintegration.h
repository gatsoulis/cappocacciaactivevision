
#ifndef CueIntegration_H
#define CueIntegration_H

#include "cv.h"
#include "highgui.h"
#include "cvaux.h"

#include "module.h"
#include "inputconnector.h"
#include "outputconnector.h"
#include "cvimage.h"
#include "csctype.h"
//#include "imagestrip.h"
#include "cue.h"
#include "cuemotion.h"
#include "cuetemplate.h"
#include "cueprediction.h"
#include "cuecolorkernel.h"
//#include "cuecolorsimple.h"
#include "cuecontrastkernel.h"


#include <iostream> 
#include <string>
#include <vector>
#include <pthread.h>
#include <math.h> 

#include <yarp/os/all.h>
#include <yarp/sig/all.h>

#define MAX_CUES 8
//#define CROSS_HAIR_SIZE 3 // n pixel to each direction from the center makes n*2+1 width and height

#define USE_THREADS


	class CueIntegration : public Module  {

	//private:
		//typedef enum {RELIABILITY = 0, QUALITY = 1} dataType;

	public: 
		CueIntegration(std::string name = "CueIntegration");
		~CueIntegration();

		InputConnector<CVImage*> cvImageIn;
		InputConnector<CVImage*> cvGrayImageIn;
		InputConnector<TrackData*> trackIn;
		
		OutputConnector<CVImage*> cvSalImageOut;
		OutputConnector<CVImage*> cvTemplateImageOut;
		OutputConnector<TrackData*> trackOut;
		OutputConnector<CVImage*> cvCueImageOut;
		
		//OutputConnector<CvPoint*> cvpointOutL;

		//OutputConnector<IplImage*> templateImageOut;

		//yarp::os::BufferedPort<yarp::sig::Vector> targetPortLeft;  
//		yarp::os::BufferedPort<yarp::sig::Vector> PointCueOutPort;
		yarp::os::BufferedPort<yarp::os::Bottle> PointCueOutPort;

		
		void clean();
		void connect();


		void execute();

		void initialize();






/*
		void profile_cues(int times=20);
		void profile_cues_ms();
		void setParam(bool displayCues = true, float tauCueAdapt = 0.02, float tauAdapt=0.1, float tauInit=0.01, float threshold = 0.6);
		void constructCueMotion(double threshold = 15, int kernelSize = 7, float temprel = 1);
		CueColorKernel* constructCueColorKernel(int opmode = 0, int colormode = 2, int maxIter = 10, float tfacs = 0.01, float temprel = 1);
		void constructCuePrediction(int mode = 1, int size = 10, float temprel = 0.2);
		void constructCueTemplate(int templateSize = 7, int method = CV_TM_CCOEFF_NORMED, float tfacs = 0.01, float temprel = 0.2 );
		void constructCueContrastKernel(int mode = 0, int maxIter = 20, int ts1 = 5, int ts2 = 9, float tfacs = 0.01, float temprel = 0.2);
//		void constructCueColorSimple(int templateSize = 3, int kernelSize = 7, float tfacs = 0.01, float temprel = 0.2);

		bool m_run;
		bool displayCues;
		bool saveData;
		bool m_useThread;

*/		
		void setthres(float val);

		void addCue(Cue* newcue, float reliability=1.0);

		CvSize m_minSize;
		bool m_forceMinSize;
		bool m_forceSquare;
		bool debug;
		double xL,yL;

		int distxL;
		int distyL;

	private:

		CvPoint point;

		void obtainInput();

		float m_threshold;

		Cue** mdp_Cues;
		float* mp_initrel;
		float* mp_rel;
		float* mp_qual;
		
		unsigned int m_cuecount;

		std::vector<Cue*> mvec_cues;
		std::vector<float> mvec_rel;

		//bool m_rgbused;
		//bool m_hsvused;

		CVImage* mp_cvgrayimg;
		CVImage* mp_cvrgbimg;

		CVImage* mp_cvrimg;
		CVImage* mp_cvgimg;
		CVImage* mp_cvbimg;
		CVImage* mp_cvhsvimg;
		CVImage* mp_cvhimg;
		CVImage* mp_cvsimg;
		CVImage* mp_cvvimg;
		CVImage* mp_cvoutputimg;
		unsigned int m_width, m_height;

		TrackData m_prevtrack;
		TrackData m_track;

		bool m_init;

		void computesaliencymap();

		void computequality();

		void computereliability();
		// Parameter for adaptation of cue's reliability value
		float m_tauadapt;
		// Parameter for adaptation of cue's internal object model
		float m_taucueadapt;

#ifdef USE_THREADS
		pthread_barrier_t m_barrier;
		//pthread_t* mp_threads;
		pthread_t m_thread1, m_thread2, m_thread3, m_thread4, m_thread5, m_thread6, m_thread7, m_thread8;
		static void *compute1(void *);
		static void *compute2(void *);
		static void *compute3(void *);
		static void *compute4(void *);
		static void *compute5(void *);
		static void *compute6(void *);
		static void *compute7(void *);
		static void *compute8(void *);
		bool m_endthreads;
		pthread_cond_t  m_condition_threadsstart;
		pthread_cond_t  m_condition_threadsended;
		pthread_mutex_t m_mutex_threadsended;
		//pthread_mutex_t* mp_mutex_waitlock;
		pthread_mutex_t m_mutex_waitlock1, m_mutex_waitlock2, m_mutex_waitlock3, m_mutex_waitlock4, m_mutex_waitlock5, m_mutex_waitlock6, m_mutex_waitlock7, m_mutex_waitlock8;
#endif

/*

		void normalizeReliabilities();
		
		int getTileCount();


		bool* initialized_cues;
		Cue* cue[MAX_CUES];
		float* reliability[MAX_CUES];
		float* defaultReliability[MAX_CUES];
		float* defaultQuality[MAX_CUES];
		float* quality[MAX_CUES];

		double* mp_minVal;
		double* mp_maxVal;
		CvPoint* mp_minLoc;
		CvPoint* mp_maxLoc;
		
		CueMotion* cueMotion;
		CueColorKernel* cueColor;
		CuePrediction* cuePrediction;

//		CueColorSimple* cueColorSimple;
		CueTemplate* cueTemplate;
		CueContrastKernel* cueContrast;
	
		int cueCount, tempCueCount;
		ImageStrip cueDisplay;
		float tauInit;
		float qualitySum;
		float defaultQualitySum;
		int index;
		int delay1, delay2, count;
		bool step1, step2;			
		int count_step1;
		float tfacsColor, tfacsTemplate, tfacsContrast;
		bool objectFound;
*/		
	};

#endif
