#include "cueprediction.h"



CuePrediction::CuePrediction(std::string name) : Cue(name) {
	std::cout << "Constructing " << getName() << "...";
	
	mp_cvoutputimg = NULL;

	A_x = NULL;
	A_y = NULL;
	state_x = NULL;
	state_y = NULL;
	process_noise_x = NULL;
	process_noise_y = NULL;
	measurement_x = NULL;
	measurement_y = NULL;
	kalman_x = NULL;
	kalman_y = NULL;

	m_tfacs = 0.1;
	m_threshold = 0.6;
	m_noise = 0.1;
	m_diffrelthres = 0.4;
	
	m_init = false;
	
	debug = true;
	
	setParam();

	std::cout << "finished.\n";
}

CuePrediction::~CuePrediction() {
	std::cout<<"Destructing " << getName() << "...";
	
	if(mp_cvoutputimg) delete mp_cvoutputimg;

	if(A_x) delete [] A_x;
	if(A_y) delete [] A_y;
	if(state_x) cvReleaseMat(&state_x);
	if(state_y) cvReleaseMat(&state_y);
	if(process_noise_x) cvReleaseMat(&process_noise_x);
	if(process_noise_y) cvReleaseMat(&process_noise_y);
	if(measurement_x) cvReleaseMat(&measurement_x);
	if(measurement_y) cvReleaseMat(&measurement_y);
	if(kalman_x) cvReleaseKalman(&kalman_x);
	if(kalman_y) cvReleaseKalman(&kalman_y);

	std::cout << "finished.\n";
}

void CuePrediction::setParam(int mode, int size) {
	m_mode = mode;
	m_size = size*size;	
	m_windowSize = 2*size;
}

void CuePrediction::execute() {

	if(!m_init) initialize();

	if(debug) std::cout << getName() << "::execute()\n";

	if(m_mode == LINEAR) {
		this->predictLinear();
	}
	else if(m_mode == KALMAN) {
		this->predictKalman();
	}
	else{
		std::cerr << getName() << "::execute()::ERROR::Unsupported mode [" << m_mode << "]!\n";
		return;
	}

	cvSalImageOut.out();

	if(debug) std::cout << getName() << "::execute() complete\n";
}  

void CuePrediction::adapt() {
	// Do nothing
}

void CuePrediction::initialize() {
	CVImage* cvgrayimg = cvGrayImageIn.getBuffer();
	if(!cvgrayimg) { std::cerr<< getName() << "::ERROR::execute()::cvGrayImageIn is NULL!...\n"; return; }
	IplImage* grayimg = cvgrayimg->ipl;

	TrackData* track = trackIn.getBuffer();
	if(!track) { std::cerr<< getName() << "::ERROR::execute()::trackIn is NULL!...\n"; return; }

	if(track->reliability < m_threshold) { std::cerr<< getName() << "::ERROR::execute()::rel [" << m_track.reliability << "] below thres [" << m_threshold << "]\n"; return; }

	if(debug) std::cout << getName() << "::initialize()\n";

	m_track.winnerPos = track->winnerPos;
	m_track.winnerSize = track->winnerSize;
	m_track.winnerRect = track->winnerRect;
	m_track.reliability = track->reliability;
	m_track.imageSize.width = grayimg->width;
	m_track.imageSize.height = grayimg->height;


	if (!mp_cvoutputimg) {
		m_width = grayimg->width;
		m_height = grayimg->height;
		mp_cvoutputimg = new CVImage( cvSize(m_width, m_height), CV_32FC1, 0);
		cvSalImageOut.setBuffer(mp_cvoutputimg);
	}

	m_prevtrack.winnerPos = m_track.winnerPos;
	m_prevtrack.winnerSize = m_track.winnerSize;
	m_prevtrack.winnerRect = m_track.winnerRect;
	m_prevtrack.reliability = m_track.reliability;
	m_prevtrack.imageSize = m_track.imageSize;

	if(m_mode == KALMAN){

		if(A_x) delete [] A_x;
		if(A_y) delete [] A_y;
		if(state_x) cvReleaseMat(&state_x);
		if(state_y) cvReleaseMat(&state_y);
		if(process_noise_x) cvReleaseMat(&process_noise_x);
		if(process_noise_y) cvReleaseMat(&process_noise_y);
		if(measurement_x) cvReleaseMat(&measurement_x);
		if(measurement_y) cvReleaseMat(&measurement_y);
		if(kalman_x) cvReleaseKalman(&kalman_x);
		if(kalman_y) cvReleaseKalman(&kalman_y);

		A_x = new float [4];
		A_x[0] = 1.0;
		A_x[1] = 1.0;
		A_x[2] = 0.0;
		A_x[3] = 1.0;
		A_y = new float [4];
		A_y[0] = 1.0;
		A_y[1] = 1.0;
		A_y[2] = 0.0;
		A_y[3] = 1.0;

		kalman_x = cvCreateKalman( 2, 1, 0 );
		kalman_y = cvCreateKalman( 2, 1, 0 );

	    // state is (phi, delta_phi) - angle and angle increment
		state_x = cvCreateMat( 2, 1, CV_32FC1 );
		state_y = cvCreateMat( 2, 1, CV_32FC1 );

		process_noise_x = cvCreateMat( 2, 1, CV_32FC1 );
		process_noise_y = cvCreateMat( 2, 1, CV_32FC1 );

	    // only phi (angle) is measured
		measurement_x = cvCreateMat( 1, 1, CV_32FC1 );
	    cvZero( measurement_x );
		measurement_y = cvCreateMat( 1, 1, CV_32FC1 );
	    cvZero( measurement_y );

	    cvRandInit( &rng_x, 0, 1, -1, CV_RAND_UNI );
	    cvRandInit( &rng_y, 0, 1, -1, CV_RAND_UNI );

		memcpy( kalman_x->transition_matrix->data.fl, A_x, sizeof(A_x));
		memcpy( kalman_y->transition_matrix->data.fl, A_y, sizeof(A_y));

		cvSetIdentity( kalman_x->measurement_matrix, cvRealScalar(1) );
		cvSetIdentity( kalman_x->process_noise_cov, cvRealScalar(1e-5) );
		cvSetIdentity( kalman_x->measurement_noise_cov, cvRealScalar(1e-20) );
		cvSetIdentity( kalman_x->error_cov_post, cvRealScalar(1));
		cvSetIdentity( kalman_y->measurement_matrix, cvRealScalar(1) );
		cvSetIdentity( kalman_y->process_noise_cov, cvRealScalar(1e-5) );
		cvSetIdentity( kalman_y->measurement_noise_cov, cvRealScalar(1e-20) );
		cvSetIdentity( kalman_y->error_cov_post, cvRealScalar(1));

		cvRandSetRange( &rng_x, 0, 0.1, 0 );
		rng_x.disttype = CV_RAND_NORMAL;
		cvRand( &rng_x, state_x );
		cvRandSetRange( &rng_y, 0, 0.1, 0 );
		rng_y.disttype = CV_RAND_NORMAL;
		cvRand( &rng_y, state_y );

		kalman_x->state_post->data.fl[0] = m_prevtrack.winnerPos.x; // position
		kalman_x->state_post->data.fl[1] = 0.0; // velocity
		kalman_y->state_post->data.fl[0] = m_prevtrack.winnerPos.y; // position
		kalman_y->state_post->data.fl[1] = 0.0; // velocity
	}
	else if(m_mode == LINEAR){
			// Do nothing
	}
	else{ 
		std::cerr<< getName() << "::ERROR::initialize()::Unsupported mode[" << m_mode << "]!\n"; 
		return; 
	}
	m_init = true;

	if(debug) std::cout << getName() << "::initialize() complete\n";
}

void CuePrediction::predictLinear() {
	TrackData* track = trackIn.getBuffer();
	if(!track) { std::cerr<< getName() << "::ERROR::execute()::trackIn is NULL!...\n"; return; }

	m_track.winnerPos = track->winnerPos;
	m_track.winnerSize = track->winnerSize;
	m_track.winnerRect = track->winnerRect;
	m_track.reliability = track->reliability;
	m_track.imageSize = track->imageSize;

	if ( m_track.reliability < m_threshold ) {
		cvSet(mp_cvoutputimg->ipl, cvScalarAll(m_noise));
		m_init = false;
		return;
	}

	CvPoint newwinnerpos;

	newwinnerpos.x = -1;
	newwinnerpos.y = -1;
	cvSetZero(mp_cvoutputimg->ipl);

	float vecX = 0.0;
	float vecY = 0.0;
	float diff_rel = m_prevtrack.reliability - m_track.reliability;

	vecX = m_track.winnerPos.x - m_prevtrack.winnerPos.x;
	vecY = m_track.winnerPos.y - m_prevtrack.winnerPos.y;
	newwinnerpos.x = cvRound( m_track.winnerPos.x + vecX );
	newwinnerpos.y = cvRound( m_track.winnerPos.y + vecY );

	if (newwinnerpos.x < 0 || newwinnerpos.x > m_width || newwinnerpos.y < 0 || newwinnerpos.y > m_height || diff_rel > m_diffrelthres){
		// predicted position is outside the image
		cvSet(mp_cvoutputimg->ipl, cvScalarAll(m_noise));
		m_init = false;
	}
	else{
		IplImage* tempimg = mp_cvoutputimg->ipl;
		for(int x = -m_windowSize; x <= m_windowSize; ++x) {
			for(int y = -m_windowSize; y <= m_windowSize; ++y) {
				int posx = newwinnerpos.x + x;
				int posy = newwinnerpos.y + y;
				if(posx >= 0 && posy >= 0 && posx < m_width && posy < m_height) {
					//((float*)(mp_cvoutputimg->ipl->imageData))[(m_newWinnerData.winnerPos.x+x)+m_width*(m_newWinnerData.winnerPos.y+y)] = exp(-0.5/m_size*(x*x+y*y));
					double val = exp(-0.5/m_size*(x*x+y*y));
					cvSetReal2D(tempimg, posy, posx, val);
				}
			}
		}
		
		if(debug) std::cout << getName() << "::predictLinear::predicted pos = [" << m_track.winnerPos.x << " " << m_track.winnerPos.y << "]\n";
		if(debug) std::cout << getName() << "::predictLinear::previous pos = [" << m_prevtrack.winnerPos.x << " " << m_prevtrack.winnerPos.y << "]\n";

		m_prevtrack.winnerPos = m_track.winnerPos;
		m_prevtrack.winnerSize = m_track.winnerSize;
		m_prevtrack.winnerRect = m_track.winnerRect;
		m_prevtrack.imageSize = m_track.imageSize;
		m_prevtrack.reliability = m_track.reliability;
	}
}

void CuePrediction::predictKalman() {

	TrackData* track = trackIn.getBuffer();
	if(!track) { std::cerr<< getName() << "::ERROR::execute()::trackIn is NULL!...\n"; return; }

	if(debug) std::cout << getName() << "::predictKalman()::mp_cvoutputimg = " << mp_cvoutputimg << "\n";

	IplImage* salimg = mp_cvoutputimg->ipl;

	m_track.winnerPos = track->winnerPos;
	m_track.winnerSize = track->winnerSize;
	m_track.winnerRect = track->winnerRect;
	m_track.imageSize = track->imageSize;
	m_track.reliability = track->reliability;

	if ( m_track.reliability < m_threshold ) {
		cvSet(salimg, cvScalarAll(m_noise));
		m_init = false;
		return;
	}

	CvPoint newwinnerpos;

//	newwinnerpos.x = -1;
//	newwinnerpos.y = -1;

	const CvMat* prediction_x = cvKalmanPredict( kalman_x, 0 );
	//CvMat* prediction_x = (CvMat*)cvKalmanPredict( kalman_x, 0 );
		
	measurement_x->data.fl[0] = m_track.winnerPos.x;
	cvMatMulAdd( kalman_x->measurement_matrix, state_x, measurement_x, measurement_x );
		
	const CvMat* prediction_y = cvKalmanPredict( kalman_y, 0 );
	//CvMat* prediction_y = (CvMat*)cvKalmanPredict( kalman_y, 0 );
		
	measurement_y->data.fl[0] = m_track.winnerPos.y;
	cvMatMulAdd( kalman_y->measurement_matrix, state_y, measurement_y, measurement_y );

	newwinnerpos.x = cvRound( kalman_x->state_pre->data.fl[0] );
	newwinnerpos.y = cvRound( kalman_y->state_pre->data.fl[0] );

	if ( newwinnerpos.x < 0 || newwinnerpos.x > m_width || newwinnerpos.y < 0 || newwinnerpos.y > m_height ){
		// predicted position is outside the image
		cvSet(salimg, cvScalarAll(m_noise));
		m_init = false;
	}
	else{
		cvSetZero(salimg);
		for(int x = -m_windowSize; x <= m_windowSize; ++x) {
			for(int y = -m_windowSize; y <= m_windowSize; ++y) {
				int posx = newwinnerpos.x + x;
				int posy = newwinnerpos.y + y;
				if(posx >= 0 && posy >= 0 && posx < m_width && posy < m_height) {
					//((float*)(mp_cvSalImage->ipl->imageData))[(m_newWinnerData.winnerPos.x+x)+m_width*(m_newWinnerData.winnerPos.y+y)] = exp(-0.5/m_size*(x*x+y*y));
					double val = exp(-0.5/m_size*(x*x+y*y));
					cvSetReal2D(salimg, posy, posx, val);
				}
			}
		}

	}

	if(debug) std::cout << getName() << "::predictKalman::predicted pos = [" << newwinnerpos.x << " " << newwinnerpos.y << "]\n";
	if(debug) std::cout << getName() << "::predictKalman::previous pos = [" << m_track.winnerPos.x << " " << m_track.winnerPos.y << "]\n";

	// adjust Kalman filter state
	cvKalmanCorrect( kalman_x, measurement_x );
	cvRandSetRange( &rng_x, 0, sqrt(kalman_x->process_noise_cov->data.fl[0]), 0 );
	cvRand( &rng_x, process_noise_x );
	cvMatMulAdd( kalman_x->transition_matrix, state_x, process_noise_x, state_x );
	cvKalmanCorrect( kalman_y, measurement_y );
	cvRandSetRange( &rng_y, 0, sqrt(kalman_y->process_noise_cov->data.fl[0]), 0 );
	cvRand( &rng_y, process_noise_y );
	cvMatMulAdd( kalman_y->transition_matrix, state_y, process_noise_y, state_y );

}

