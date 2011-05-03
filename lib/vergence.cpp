#include "vergence.h"



Vergence::Vergence(std::string name) : Module(name){
	std::cout<< "Constructing " << getName() << "\n";

	mdp_responses = NULL;

	m_trackthreshold = 0.2;
	
	vergOut.setBuffer(&m_verg);
	
	mp_simmat = NULL;
	mp_kjx = NULL;
	mp_kjy = NULL;
	m_runningavg = false;
	m_alpha = 0.9;
	debug = false;
	
	m_disp.x = 0.0;
	m_disp.y = 0.0;

	setParam();
	setFilterParam();
	
//	m_GaborFilter.debug = true;
	
	mp_cvleftgrayimgflt = NULL;
	mp_cvrightgrayimgflt = NULL;

	std::cout<< "Constructing " << getName() << " finished\n";

/*

	cvLeftGrayImageIn.setModule(this);
	cvRightGrayImageIn.setModule(this);
	trackIn.setModule(this);

#ifdef USE_THREADS_VERGENCE
	pthread_barrier_init(&barrier_threads, NULL, count);
	pthread_cond_init(&condition_startthreads, NULL);
	pthread_mutex_init(&mutex_waitlock, NULL);
	pthread_cond_init(&condition_threadsended, NULL);
	pthread_mutex_init(&mutex_threadsended, NULL);
	int  iret1, iret2;
	iret1 = pthread_create( &(m_threadleft), NULL, computeleft, this);
	iret2 = pthread_create( &(m_threadright), NULL, computeright, this);
#else
#endif

	pdx = new float [2];

	dataTemp = NULL;

	dataLeft = NULL;
	dataRight = NULL;
	dataLeftAmpl = NULL;
	dataRightAmpl = NULL;
	dataLeftAngle = NULL;
	dataRightAngle = NULL;
	absSim = NULL;

	dataO = new VergenceOutput [1];
	
	m_initialized = false;

	gabor = new CSCGaborFilters("CSCGaborFilters module");
	int method = 1; // 0 for D, 1 for L*D

	gabor->initialize(method);

	m_run = true;
	
	mp_disparity = cvCreateMat(1, 4, CV_32FC1);
	mp_newdisparity = cvCreateMat(1, 4, CV_32FC1);
	cvSetZero(mp_disparity);
	m_alpha = 0.8;
	
	m_runningAvg = false;
	
*/
}

Vergence::~Vergence(){
	std::cout<< "Destructing " << getName() << "\n";

	if(mdp_responses) {
		for(unsigned int i = 0;i<m_gridsize;++i) delete mdp_responses[i];
		delete mdp_responses;
	}

	if(mp_simmat) cvReleaseMat(&mp_simmat);
	if(mp_kjx) delete [] mp_kjx;
	if(mp_kjy) delete [] mp_kjy;
	
	if(mp_cvleftgrayimgflt) delete mp_cvleftgrayimgflt;
	if(mp_cvrightgrayimgflt) delete mp_cvrightgrayimgflt;
	
	std::cout<< "Destructing " << getName() << " finished\n";
/*

#ifdef USE_THREADS_VERGENCE
	pthread_barrier_destroy(&barrier_threads);
	pthread_cond_destroy(&condition_startthreads);
	pthread_mutex_destroy(&(mutex_waitlock) );
	pthread_cond_destroy(&condition_threadsended);
	pthread_mutex_destroy(&mutex_threadsended);
#else
#endif

	delete [] pdx;
	delete [] dataO;

	if(dataLeft != NULL) delete [] dataLeft;
	if(dataRight != NULL) delete [] dataRight;
	if(dataLeftAmpl != NULL) delete [] dataLeftAmpl;
	if(dataRightAmpl != NULL) delete [] dataRightAmpl;
	if(dataLeftAngle != NULL) delete [] dataLeftAngle;
	if(dataRightAngle != NULL) delete [] dataRightAngle;
	if(absSim != NULL) delete [] absSim;

	std::cout<< "finished.\n";
*/
}

void Vergence::setFilterParam(unsigned int L, unsigned int D, float f, float kmax, float sigma, float N, std::vector<int>* filtersize) {
	if(debug) std::cout << getName() << "::setFilterParam()\n";

	m_L = L;
	m_D = D;
	m_LD = L*D;
	
	m_GaborFilter.setParam(L, D, f, kmax, sigma, N, filtersize);
	m_GaborFilter.setMode(GaborFilter::AMPLITUDEPHASE);

	if(mp_kjx) delete [] mp_kjx;
	if(mp_kjy) delete [] mp_kjy;
	mp_kjx = NULL;
	mp_kjy = NULL;

	if(debug) std::cout << getName() << "::setFilterParam() finished\n";
}

void Vergence::setParam(unsigned int gridsizehor, unsigned int gridsizever, unsigned int griddisphor, unsigned int griddispver) {
	if(debug) std::cout << getName() << "::setParam()\n";

	if(gridsizehor % 2 != 1) { std::cerr << getName() << "::setParam()::ERROR::gridsizehor must be an odd number!\n"; return; }
	if(gridsizever % 2 != 1) { std::cerr << getName() << "::setParam()::ERROR::gridsizever must be an odd number!\n"; return; }

	m_gridsizehor = gridsizehor;
	m_gridsizever = gridsizever;
	m_gridsize = gridsizehor * gridsizever;
	m_griddisphor = griddisphor;
	m_griddispver = griddispver;

	if(mp_simmat) cvReleaseMat(&mp_simmat);
	mp_simmat = cvCreateMat(m_gridsizever, m_gridsizehor, CV_32FC1);
		
	if(debug) std::cout << getName() << "::setParam() finished\n";

/*

//	std::cout<< getName() << "::initialize()::Line 1\n";
	//! generate Gabor filters for use

	this->gridX = gridX;
	this->gridY = gridY;
	this->gridDispX = gridDispX;
	this->gridDispY = gridDispY;

	this->L = L;
	this->D = D;
	this->sigma = sigma;

	if(filterSize != NULL) this->filterSize = filterSize;
	else {
		int tempFilterSize[] = {75, 51, 31, 17, 11};
		this->filterSize = tempFilterSize;
	}

	gabor->generateFilter(L, D, sigma, filterSize);

//	std::cout<< getName() << "::initialize()::Line 2\n";
	
	if(dataLeft != NULL) delete [] dataLeft;
	if(dataRight != NULL) delete [] dataRight;
	if(dataLeftAmpl != NULL) delete [] dataLeftAmpl;
	if(dataRightAmpl != NULL) delete [] dataRightAmpl;
	if(dataLeftAngle != NULL) delete [] dataLeftAngle;
	if(dataRightAngle != NULL) delete [] dataRightAngle;
	if(mp_kjx != NULL) delete [] mp_kjx;
	if(mp_kjy != NULL) delete [] mp_kjy;
	if(absSim != NULL) delete [] absSim;

//	std::cout<< getName() << "::initialize()::Line 3\n";

	dataLeft = new Ipp32fc[L*D];
	dataRight = new Ipp32fc[L*D*gridX*gridY];
	dataLeftAmpl = new Ipp32f[L*D];
	dataRightAmpl = new Ipp32f[L*D*gridX*gridY];
	dataLeftAngle = new Ipp32f[L*D];
	dataRightAngle = new Ipp32f[L*D*gridX*gridY];
	absSim = new float[gridX * gridY];

//	std::cout<< getName() << "::initialize()::Line 4\n";

	size = L*D;


	m_initialized = true;

//	std::cout<< getName() << "::initialize()::Line 5\n";
*/
}

void Vergence::execute() {

	if(debug) std::cout << getName() << "::execute()\n";

	mp_cvleftgrayimg = cvLeftGrayImageIn.getBuffer();
	mp_cvrightgrayimg = cvRightGrayImageIn.getBuffer();
	
	if(!mp_cvleftgrayimg || !mp_cvrightgrayimg) { std::cerr << getName() << "::execute()::ERROR::One of input is NULL!\n"; return; }

	if( (mp_cvleftgrayimg->cvMatType == CV_8UC1 || mp_cvleftgrayimg->cvMatType == CV_32FC1) && (mp_cvrightgrayimg->cvMatType == CV_8UC1 || mp_cvrightgrayimg->cvMatType == CV_32FC1)) {
		//m_width = mp_cvleftgrayimg->width;
		//m_height = mp_cvleftgrayimg->height;
	}
	else{
		std::cerr << getName() << "::execute()::ERROR::input images must be single-channel!\n"; 
		return;
	}

	CVImage* cvleftgrayimgflt;
	CVImage* cvrightgrayimgflt;

	if( mp_cvleftgrayimg->cvMatType == CV_8UC1 ) {
		cvleftgrayimgflt = m_ConvertFltLeft.convert(mp_cvleftgrayimg);
		cvrightgrayimgflt = m_ConvertFltRight.convert(mp_cvrightgrayimg);
	}
	else{
		cvleftgrayimgflt = mp_cvleftgrayimg;
		cvrightgrayimgflt = mp_cvrightgrayimg;
	}



	if(!mdp_responses) {
		unsigned int size = m_gridsize+1;
		mdp_responses = new CSCVector* [size];
		for(unsigned int i = 0;i<size;++i) {
			mdp_responses[i] = new CSCVector;
			mdp_responses[i]->allocate(2*m_LD);
		}
	}
	
	if(!mp_kjx) {
		mp_kjx = new float [m_LD];
		mp_kjy = new float [m_LD];

		int count = 0;
		for(unsigned int v=0;v<m_L;++v){
			float kv = 3.1416 * pow(2.0,  -(((float)v + 2.0)/2.0));
			for(unsigned int mu=0;mu<m_D;++mu){
				float pimu = (float)mu * 3.1416 / (float)m_D;
				mp_kjx[count] = kv * cos(pimu);
				mp_kjy[count] = kv * sin(pimu);
				count++;
			}
		}
	}

	m_verg.leftEyeHor = 0.0;
	m_verg.leftEyeVer = 0.0;
	m_verg.rightEyeHor = 0.0;
	m_verg.rightEyeVer = 0.0;

	TrackData* track = trackIn.getBuffer();
	if( !track ) { std::cerr << getName() << "::execute()::trackIn is NULL!\n"; return; }

	if(track->reliability < m_trackthreshold) { return; }

	float xratio = (float)(track->imageSize.width) / (float)(mp_cvleftgrayimg->width);
	float yratio = (float)(track->imageSize.height) / (float)(mp_cvleftgrayimg->height);

//	if(debug) std::cout << getName() << "::execute()::xratio = " << xratio << ", yratio = " << yratio << "\n";

	unsigned int centerx = cvRound((float)(track->winnerPos.x) / xratio);
	unsigned int centery = cvRound((float)(track->winnerPos.y) / yratio);
	CvPoint pos;

	// compute left gabor jet and store it
//	if(debug) std::cout << getName() << "::execute()::computing left gabor\n";
	pos.x = centerx;
	pos.y = centery;
	//std::cout << "cvleftgrayimgflt = " << cvleftgrayimgflt << "\n";
	//std::cout << "pos = [" << pos.x << " " << pos.y << "]\n";
	//std::cout << "mdp_responses[0] = " << mdp_responses[0] << "\n";
	m_GaborFilter.compute(cvleftgrayimgflt, &pos, mdp_responses[0]);
	//mdp_responses[0]->print();
	
	// compute right gabor jets and store it
//	if(debug) std::cout << getName() << "::execute()::computing right gabors\n";
	for(unsigned int j=0;j<m_gridsizever;++j) {
		pos.y = centery + (j-((m_gridsizever-1)/2))*m_griddispver;
		for(unsigned int i=0;i<m_gridsizehor;++i) {
			pos.x = centerx + (i-((m_gridsizehor-1)/2))*m_griddisphor;
			unsigned int counter = j*m_gridsizehor + i;
			if(pos.x >= 0 && pos.x < mp_cvrightgrayimg->width && pos.y >= 0 && pos.y < mp_cvrightgrayimg->height) {
				//std::cout << "counter = " << counter << "\n";
				//std::cout << "cvrightgrayimgflt = " << cvrightgrayimgflt << "\n";
				//std::cout << "pos = [" << pos.x << " " << pos.y << "]\n";
				//std::cout << "mdp_responses[counter+1] = " << mdp_responses[counter+1] << "\n";
				m_GaborFilter.compute(cvrightgrayimgflt, &pos, mdp_responses[counter+1]);
				//mdp_responses[counter+1]->print();
			}
			else {
				mdp_responses[counter+1]->setZero();
			}
		}
	}
		
	for(unsigned int i=0;i<(m_gridsizever*m_gridsizehor+1);++i) {
		normalize(mdp_responses[i]);
	}

	// compare absolute similarity of left and right response
	for(unsigned int j=0;j<m_gridsizever;++j) {
		for(unsigned int i=0;i<m_gridsizehor;++i) {
			unsigned int counter = j*m_gridsizehor + i;
			float sim = computesim(mdp_responses[0], mdp_responses[counter+1]);
			//if(debug) std::cout << sim << " ";
			cvmSet(mp_simmat, j, i, (double)sim);
		}
		//if(debug) std::cout << "\n";
	}
	
	double maxval;
	CvPoint maxloc;
	cvMinMaxLoc(mp_simmat, NULL, &maxval, NULL, &maxloc, NULL);
	
	//if(debug) std::cout << getName() << "::execute()::maxval = " << maxval << ", maxloc.x = " << maxloc.x << "\n";

	if(maxval < (double)m_simthres) {
		vergOut.out();
	}	

	//if(debug) { std::cout << getName() << "::execute()::maxloc = (" << maxloc.x << "," << maxloc.y << ")\n"; }
	
	int focus = 0;
	int maxind = maxloc.y * m_gridsizehor + maxloc.x;
	CvPoint2D32f newdisp = computephasedisplacement(mdp_responses[0], mdp_responses[maxind+1], focus);

	//if(debug) { std::cout << getName() << "::execute()::newdisp1 = (" << newdisp.x << "," << newdisp.y << ")\n"; }

	float dispx = ((float)maxloc.x - (((float)m_gridsizehor-1.0)/2.0))*(float)m_griddisphor;
	float dispy = ((float)maxloc.y - (((float)m_gridsizever-1.0)/2.0))*(float)m_griddispver;

	//if(debug) { std::cout << getName() << "::execute()::disp = (" << dispx << "," << dispy << ")\n"; }

	newdisp.x += dispx;
	newdisp.y += dispy;

	//if(debug) { std::cout << getName() << "::execute()::newdisp2 = (" << newdisp.x << "," << newdisp.y << ")\n"; }

	if(!m_runningavg) {
		m_disp = newdisp;
	}
	else{
		//std::cout << "newdisp.x = " << newdisp.x << "\n";
		//std::cout << "m_disp.x = " << m_disp.x << "\n";
		m_disp.x = computerunningavg(newdisp.x, m_disp.x, m_alpha);
		//std::cout << "m_disp.x = " << m_disp.x << "\n";
		m_disp.y = computerunningavg(newdisp.y, m_disp.y, m_alpha);
	}

	m_verg.leftEyeHor = 0.0;
	m_verg.leftEyeVer = 0.0;
	m_verg.rightEyeHor = -m_disp.x;
	m_verg.rightEyeVer = m_disp.y;
	
	if(debug) std::cout << getName() << "::execute()::verg = [" << m_verg.leftEyeHor << " " << m_verg.leftEyeVer << " " << m_verg.rightEyeHor << " " << m_verg.rightEyeVer << "]\n";

	vergOut.out();

	if(debug) std::cout << getName() << "::execute() finished\n";

//	while(focus <= L){
//		this->computePhaseDisplacement(ind, focus);
//		pdx[0] = pdx[0] * 5;
//		pdx[1] = pdx[1] * 5;
//		pdx[0] = round( pdx[0], 0 );
//		pdx[1] = round( pdx[1], 0 );
//		if (pdx[0] > 1.0 || pdx[0] < -1.0 || pdx[1] > 1.0 || pdx[1] < -1.0){
//			*dispXRight = pdx[0];
//			*dispYRight = pdx[1];
//			break;
//		}
//		focus = focus + 1;
//	}


//		cvSetZero(mp_newdisparity);
//		cvmSet(mp_newdisparity, 0, 0, (pdx[0] * 5.0 - (indX- ((gridX-1)/2))*gridDispX) );
//		cvmSet(mp_newdisparity, 0, 1, (pdx[1] * 5.0 - (indY- ((gridY-1)/2))*gridDispY) );

//		cvmSet(mp_newdisparity, 0, 2, (pdx[0] * 5.0 - (indX- ((gridX-1)/2))*gridDispX) );
//		cvmSet(mp_newdisparity, 0, 3, (pdx[1] * 5.0 - (indY- ((gridY-1)/2))*gridDispY) );


/*
	int gabor_index = 0;
	gabor->convolveTD(mp_cvleftgrayimg->ipl, &(m_winner.winnerPos), 0, gabor_index);
	gabor->copyTD(dataLeft, 0);
	// compute right gabor jets and store it
	CvPoint pos;
	centerX = m_winner.winnerPos.x;
	centerY = m_winner.winnerPos.y;
	int count = 0;
	int tempSize = D;
	for(i=0;i<gridX;i++) {
		for(j=0;j<gridY;j++) {
			pos = cvPoint(centerX + (i-((gridX-1)/2))*gridDispX, centerY + (j-((gridY-1)/2))*gridDispY);
			gabor->convolveTD(mp_cvrightgrayimg->ipl, &pos, 0, gabor_index);
			gabor->copyTD(&(dataRight[tempSize*count]));
			count = count + 1;
		}
	}
*/

/*
	if(m_run == false) return;
	if(m_initialized == false) this->setParam();




	if( winnerData != NULL && (winnerData->winnerPos.x < 0 || winnerData->winnerPos.x > winnerData->imageSize.width || winnerData->winnerPos.y < 0 || winnerData->winnerPos.y > winnerData->imageSize.height) ){
//		std::cerr << getName() << "::execute()::winnerPos out of range! winnerPos = (" << winnerData->winnerPos.x << ", " << winnerData->winnerPos.y << ")\n";
		dataOut.setBuffer(NULL);
		dataOut.out();
		return;
	}

	if(debug) std::cout << getName() << "::execute()...\n";

	if(winnerData != NULL){

		if(winnerData->imageSize.width == m_width && winnerData->imageSize.height == m_height) {
			m_winner.winnerPos = winnerData->winnerPos;
			m_winner.winnerSize = winnerData->winnerSize;
			m_winner.winnerRect = winnerData->winnerRect;
			m_winner.imageSize = winnerData->imageSize;
			m_winner.reliability = winnerData->reliability;
		}
		else{
			// Rescaling the tracking data coordinates to match the stored image size
			float horscale, verscale;
			horscale = (float)(m_width) / (float)(winnerData->imageSize.width);
			verscale = (float)(m_height) / (float)(winnerData->imageSize.height);

			m_winner.winnerPos.x = cvRound( (winnerData->winnerPos.x) * horscale );
			m_winner.winnerPos.y = cvRound( (winnerData->winnerPos.y) * verscale );
			m_winner.winnerSize.width = cvRound( (winnerData->winnerSize.width) * horscale );
			m_winner.winnerSize.height = cvRound( (winnerData->winnerSize.height) * verscale );
			m_winner.winnerRect.x = cvRound( (winnerData->winnerRect.x) * horscale );
			m_winner.winnerRect.y = cvRound( (winnerData->winnerRect.y) * verscale );
			m_winner.winnerRect.width = cvRound( (winnerData->winnerRect.width) * horscale );
			m_winner.winnerRect.height = cvRound( (winnerData->winnerRect.height) * verscale );
			m_winner.reliability = winnerData->reliability;
			m_winner.imageSize.width = m_width;
			m_winner.imageSize.height = m_height;
		}
	}
	else{
//		dataOut.setBuffer(NULL);
//		dataOut.out();
		return;
	}

	int i, j, k;
	int ind, indX, indY;
	float max;

	Ipp32f tempSim = (Ipp32f)0.0;
	// compute left gabor jet and store it
	int gabor_index = 0;
	gabor->convolveTD(mp_cvleftgrayimg->ipl, &(m_winner.winnerPos), 0, gabor_index);
	gabor->copyTD(dataLeft, 0);
	// compute right gabor jets and store it
	CvPoint pos;
	centerX = m_winner.winnerPos.x;
	centerY = m_winner.winnerPos.y;
	int count = 0;
	int tempSize = D;
	for(i=0;i<gridX;i++) {
		for(j=0;j<gridY;j++) {
			pos = cvPoint(centerX + (i-((gridX-1)/2))*gridDispX, centerY + (j-((gridY-1)/2))*gridDispY);
			gabor->convolveTD(mp_cvrightgrayimg->ipl, &pos, 0, gabor_index);
			gabor->copyTD(&(dataRight[tempSize*count]));
			count = count + 1;
		}
	}

	// compute absolute similarity of left and right response
	for(j=0;j<gridY;j++) {
		for(i=0;i<gridX;i++) {
			k = j*gridX + i;
			pos = cvPoint(centerX + (i-((gridX-1)/2))*gridDispX, centerY + (j-((gridY-1)/2))*gridDispY);

			if(pos.x > 0 && pos.x < mp_cvleftgrayimg->width && pos.y > 0 && pos.y < mp_cvleftgrayimg->height){
				tempSim = gabor->similarity(dataLeft, &(dataRight[tempSize*k]), tempSize, 0); // 0 for absolute similarity, 1 for phase similarity
				absSim[k] = tempSim;
			}
			else{
				absSim[k] = 0.0;
			}
		}
	}
	// find the max. abs. similarity
	count = 0;
	max = -255.0;
	ind = 2;
	indX = 2;
	indY = 0;
	for(i=0;i<gridX;i++){
		for(j=0;j<gridY;j++) {
			if(absSim[count]>max){
				max = absSim[count];
				ind = count;
				indX = i;
				indY = j;
		    }
			count = count + 1;
		}
	}
	
	if(max > 0.8){
		// calculate the phase-based disparity on the max abs. similarity point
		gabor->absPha(dataLeft, dataLeftAmpl, dataLeftAngle, tempSize);
		gabor->absPha(dataRight, dataRightAmpl, dataRightAngle, tempSize*gridX*gridY);

	
		int focus = 0;
		this->computePhaseDisplacement(ind, focus);

//	while(focus <= L){
//		this->computePhaseDisplacement(ind, focus);
//		pdx[0] = pdx[0] * 5;
//		pdx[1] = pdx[1] * 5;
//		pdx[0] = round( pdx[0], 0 );
//		pdx[1] = round( pdx[1], 0 );
//		if (pdx[0] > 1.0 || pdx[0] < -1.0 || pdx[1] > 1.0 || pdx[1] < -1.0){
//			*dispXRight = pdx[0];
//			*dispYRight = pdx[1];
//			break;
//		}
//		focus = focus + 1;
//	}


//		cvSetZero(mp_newdisparity);
//		cvmSet(mp_newdisparity, 0, 0, (pdx[0] * 5.0 - (indX- ((gridX-1)/2))*gridDispX) );
//		cvmSet(mp_newdisparity, 0, 1, (pdx[1] * 5.0 - (indY- ((gridY-1)/2))*gridDispY) );
		cvmSet(mp_newdisparity, 0, 2, (pdx[0] * 5.0 - (indX- ((gridX-1)/2))*gridDispX) );
		cvmSet(mp_newdisparity, 0, 3, (pdx[1] * 5.0 - (indY- ((gridY-1)/2))*gridDispY) );

		if(m_runningAvg == false){
			dataO->leftHorDisp = cvRound(cvmGet(mp_newdisparity, 0, 0));
			dataO->leftVerDisp = cvRound(cvmGet(mp_newdisparity, 0, 1));
			dataO->rightHorDisp = cvRound(cvmGet(mp_newdisparity, 0, 2));
			dataO->rightVerDisp = cvRound(cvmGet(mp_newdisparity, 0, 3));
		}
		else{
			cvRunningAvg(mp_newdisparity, mp_disparity, m_alpha, NULL);
			dataO->leftHorDisp = cvRound(cvmGet(mp_disparity, 0, 0));
			dataO->leftVerDisp = cvRound(cvmGet(mp_disparity, 0, 1));
			dataO->rightHorDisp = cvRound(cvmGet(mp_disparity, 0, 2));
			dataO->rightVerDisp = cvRound(cvmGet(mp_disparity, 0, 3));
		}

	}
	else{
		dataO->leftHorDisp = 0.0;
		dataO->leftVerDisp = 0.0;
		dataO->rightHorDisp = 0.0;
		dataO->rightVerDisp = 0.0;
	}

	dataOut.setBuffer(dataO);
	dataOut.out();

	if(debug){
		std::cout << getName() << "::execute():: verg = (" << dataO->leftHorDisp << ", " << dataO->leftVerDisp << ", " << dataO->rightHorDisp << ", " << dataO->rightVerDisp << ")\n";
	}
*/
}

float Vergence::computerunningavg(float newval, float oldval, float alpha) {
	float temp = 1.0 - alpha;
	float val = alpha * newval + temp * oldval;
	return val;
}


float Vergence::computesim(CSCVector* vec1, CSCVector* vec2) {

	float sim = 0.0;
	unsigned int size = vec1->getWidth() / 2;
	float* vec1data = vec1->getData();
	float* vec2data = vec2->getData();
	for(unsigned int i = 0;i<size;++i) {
		float temp = vec1data[i*2] * vec2data[i*2];
		sim += temp;
	}

	return sim;
}

void Vergence::normalize(CSCVector* vec) {
	unsigned int size = vec->getWidth() / 2;
	float* vecdata = vec->getData();
	float sum = 0.0;
	for(unsigned int i = 0;i<size;++i) {
		float temp1 = vecdata[i*2];
		float temp2 = pow(temp1, 2.0f);
		sum += temp2;
	}
	float temp3 = sqrt(sum);
	for(unsigned int i = 0;i<size;++i) {
		float temp1 = vecdata[i*2];
		vecdata[i*2] = temp1 / temp3;
	}
}

CvPoint2D32f Vergence::computephasedisplacement(CSCVector* vec1, CSCVector* vec2, int focus) {
	
	CvPoint2D32f verg;
	verg.x = 0.0;
	verg.y = 0.0;

	if(!vec1 || !vec2) { std::cerr << getName() << "::computephasedisplacement()::ERROR::One of input is NULL!\n"; return verg; }

	float phiX = 0.0, phiY = 0.0, gammaXY = 0.0, gammaYX = 0.0, gammaXX = 0.0, gammaYY = 0.0;

	float* vecleft = vec1->getData();
	float* vecright = vec2->getData();

	for(unsigned int j=0;j<(focus+1)*m_D;j++){
		float phadiff = vecleft[j*2+1] - vecright[j*2+1];

		float temp;

		temp = 2.0*PI;
		while(phadiff > PI) phadiff -= temp;
		while(phadiff < - PI) phadiff += temp;
		if(phadiff > PI || phadiff < -PI) std::cerr<<"ERROR::phase difference is not within [-pi,pi]\n";

		temp = (vecleft[j*2] * vecright[j*2] * mp_kjx[j] * phadiff);
		phiX += temp;
		temp = (vecleft[j*2] * vecright[j*2] * mp_kjy[j] * phadiff);
		phiY += temp;
		
		temp = (vecleft[j*2] * vecright[j*2] * mp_kjx[j] * mp_kjy[j]);
		gammaXY += temp;
		temp = (vecleft[j*2] * vecright[j*2] * mp_kjy[j] * mp_kjx[j]);
		gammaYX += temp;
		temp = (vecleft[j*2] * vecright[j*2] * mp_kjx[j] * mp_kjx[j]);
		gammaXX += temp;
		temp = (vecleft[j*2] * vecright[j*2] * mp_kjy[j] * mp_kjy[j]);
		gammaYY += temp;
	}

	float disp = gammaXX*gammaYY - gammaXY * gammaYX;
	//if(debug) std::cout << getName() << "::computephasedisplacement()::gammaXX = " << gammaXX << ", gammaYY = " << gammaYY << ", gammaXY = " << gammaXY << ", gammaYX = " << gammaYX << ", disp = " << disp << "\n";

	//if(debug) std::cout << getName() << "::computephasedisplacement()::phiX = " << phiX << ", phiY = " << phiY << "\n";

	if (disp != 0.0){
		float temp;
		temp = 1.0 / disp * (gammaYY * phiX - gammaYX * phiY);
		//if(debug) std::cout << getName() << "::computephasedisplacement()::verg.x = " << temp << "\n";
		verg.x = temp;
		temp = 1.0 / disp * (- gammaXY * phiX + gammaXX * phiY);
		//if(debug) std::cout << getName() << "::computephasedisplacement()::verg.y = " << temp << "\n";
		verg.y = temp;
	}

	//if(debug) std::cout << getName() << "::computephasedisplacement()::verg = (" << verg.x << "," << verg.y << ")\n";

	return verg;
}


/*
void Vergence::computePhaseDisplacement(int ind, int focus)
{
	int j;
	float phiX = 0.0, phiY = 0.0, gammaXY = 0.0, gammaYX = 0.0, gammaXX = 0.0, gammaYY = 0.0;
	float dx = 0.0, dy = 0.0;
	float temp1, temp2;

	int tempSize = D;

	for(j=0;j<(focus+1)*D;j++){
		temp1 = dataLeftAngle[j] - dataRightAngle[tempSize*ind+j];
		while (temp1 > PI) temp1 = temp1 - 2.0*PI;
		while (temp1 < - PI) temp1 = temp1 + 2.0*PI;
		
		if(temp1 > PI || temp1 < -PI) std::cerr<<"ERROR::phase difference is not within [-pi,pi]\n";

		phiX = phiX + dataLeftAmpl[j] * dataRightAmpl[tempSize*ind+j] * mp_kjx[j] * temp1;
		phiY = phiY + dataLeftAmpl[j] * dataRightAmpl[tempSize*ind+j] * mp_kjy[j] * temp1;
		gammaXY = gammaXY + dataLeftAmpl[j] * dataRightAmpl[tempSize*ind+j] * mp_kjx[j] * mp_kjy[j];
		gammaYX = gammaYX + dataLeftAmpl[j] * dataRightAmpl[tempSize*ind+j] * mp_kjy[j] * mp_kjx[j];
		gammaXX = gammaXX + dataLeftAmpl[j] * dataRightAmpl[tempSize*ind+j] * mp_kjx[j] * mp_kjx[j];
		gammaYY = gammaYY + dataLeftAmpl[j] * dataRightAmpl[tempSize*ind+j] * mp_kjy[j] * mp_kjy[j];
	}
	temp2 = gammaXX*gammaYY - gammaXY * gammaYX;
	if (temp2 != 0.0){
		dx = 1.0 / temp2 * (gammaYY * phiX - gammaYX * phiY);
		dy = 1.0 / temp2 * (- gammaXY * phiX + gammaXX * phiY);
	}
	else{
		dx = 0.0;
		dy = 0.0;
	}
	pdx[0] = dx;
	pdx[1] = dy;
}

float Vergence::round(const float &number, const int num_digits)
{
	float doComplete5i, doComplete5(number * powf(10.0f, (float) (num_digits + 1)));
    
	if(number < 0.0f)
		doComplete5 -= 5.0f;
	else
		doComplete5 += 5.0f;
    
	doComplete5 /= 10.0f;
	modff(doComplete5, &doComplete5i);
    
	return doComplete5i / powf(10.0f, (float) num_digits);
}
*/
