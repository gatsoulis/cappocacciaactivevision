#ifndef CuePrediction_H
#define CuePrediction_H

#define MAX_ORDER 4

#include "cue.h"
#include "csctype.h"
#include "cvimage.h"
//#include "outputconnector.h"


	class CuePrediction : public Cue  {

		public: 

			typedef enum {LINEAR = 0, KALMAN = 1} modes;

			CuePrediction(std::string name = "CuePrediction");

			~CuePrediction();

			void execute();

			void adapt();

			void initialize();

			void setParam(int mode = KALMAN, int size = 40); // size : size of the gray ball to put on the winner location of the saliency map

			//void print();
			
			bool debug;

  private:                              

		int m_mode;
		int m_size;
		int m_windowSize;

		double m_min, m_max;
		CvPoint m_minLoc, m_maxLoc;

		TrackData m_track;
		TrackData m_prevtrack;

		//CVImage* mp_cvoutputimg;
		int m_height, m_width;

		void predictLinear();

		void predictKalman();

		// A matrix data 
		float* A_x;
		float* A_y;
	    CvKalman* kalman_x;
	    CvKalman* kalman_y;
	    // state is (phi, delta_phi) - angle and angle increment
	    CvMat* state_x;
	    CvMat* state_y;
	    CvMat* process_noise_x;
	    CvMat* process_noise_y;
		// only phi (angle) is measured
	    CvMat* measurement_x;
	    CvMat* measurement_y;

	    CvRandState rng_x;
	    CvRandState rng_y;

		float m_noise;
		float m_diffrelthres;

/*
		float m_Kpredict;
		bool m_track;
		float m_prevRel;
		TrackData m_lastWinnerData;
*/		
	};

#endif
