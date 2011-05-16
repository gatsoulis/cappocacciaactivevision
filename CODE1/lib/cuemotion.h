#ifndef CueMotion_H
#define CueMotion_H

#include "cue.h"
#include "outputconnector.h"
#include "cvimage.h"

#include <iostream>


//! class CueMotion  -- Motion cue based on gray-scale difference of two temporally adjacent images.
/*! class CueMotion  -- Motion cue based on gray-scale difference of two temporally adjacent images.

		revised 03/2006 by Hyundo Kim
*/
	class CueMotion : public Cue  {

		public:

			CueMotion(std::string name = "CueMotion");
			~CueMotion();
			void execute();

			// tfacs : parameter for speed of adaptation
			void adapt();

			void initialize();

			// threshold : Threshold for gray-level difference, if bigger mark 1, else mark 0
			// kernelSize : size of kernel for gaussian blurring
			void setParam(double threshold = 30.0, int kernelSize = 7) { m_thresholdMotion = threshold; m_kernelSize = kernelSize; }

			void print();

		private:

			int m_kernelSize;
			double m_thresholdMotion;

			CVImage* mp_cvprevimg;
			//CVImage* mp_cvoutputimg;

			CvRect m_cvgrayimgroi;

	};

#endif
