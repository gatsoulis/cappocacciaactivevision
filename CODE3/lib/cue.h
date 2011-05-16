#ifndef Cue_H
#define Cue_H

#include "cv.h"
//#include <cc++/thread.h>

#include "module.h"
#include "inputconnector.h"
#include "outputconnector.h"
#include "cvimage.h"
#include "csctype.h"

#include <time.h>
#include <string>


	// Base-class for all cue modules (for tracking)
	class Cue : public Module  {

		public:

			Cue(std::string name="Cue");
			~Cue();
			virtual void execute()=0;

			InputConnector<CVImage*> cvImageIn;
			InputConnector<CVImage*> cvRImageIn;
			InputConnector<CVImage*> cvGImageIn;
			InputConnector<CVImage*> cvBImageIn;
			InputConnector<CVImage*> cvHImageIn;
			InputConnector<CVImage*> cvSImageIn;
			InputConnector<CVImage*> cvVImageIn;
			InputConnector<CVImage*> cvGrayImageIn; // V
			InputConnector<CVImage*> cvSalImageIn; // V
			InputConnector<CvPoint*> winnerIn;
			InputConnector<CvRect*> roiIn;
			InputConnector<TrackData*> trackIn;

			OutputConnector<CVImage*> cvSalImageOut;
			OutputConnector<CvPoint*> winnerOut;
			OutputConnector<CvRect*> roiOut;
			OutputConnector<TrackData*> trackOut;

			virtual void adapt()=0; // 1 for initialisation
			double adapt_profiled(int times=1);
			long adapt_profiled_ms();
			long adapt_profiled_ms_stdout();

			virtual void initialize()=0;

			void setthres(float thres) { m_threshold = thres; }

			void settfacs(float tfacs) { m_tfacs = tfacs; }
			
			void setinit(bool val) { m_init = val; }

			virtual void print();
			
			virtual float computequality(CVImage* overallsaliencymap, TrackData* track);

		protected:

			bool m_init;
			float m_threshold;
			float m_tfacs;
			TrackData m_track;
			CVImage* mp_cvoutputimg;

	
	};

#endif
