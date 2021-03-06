#include "detectsavedobject.h"
//#include "drawcircle.h"
#include "cv.h"
#include "highgui.h"


DetectSavedObject::DetectSavedObject(std::string name) : Module(name){
	std::cout<< "\nConstructing " << getName() << "...started";



//start Gab's add
	oriCnt=8;
	arrayL=(double *)malloc(oriCnt*sizeof(double));
	arrayR=(double *)malloc(oriCnt*sizeof(double));

	Dog = false;
	Harris = true;
	firstL=true;
	firstR=true;

	if (Dog == true){
		mp_DogLeft = new DogDetector("DogLeft"+getName());
		mp_DogLeft->setParam(1000);
		mp_DogRight = new DogDetector("DogRight"+getName());
		mp_DogRight->setParam(1000);
	}
//end Gab's add

	if (Harris == true){
		mp_HarrisLeft = new HarrisDetector("HarrisLeft"+getName());
		mp_HarrisLeft->setParam(1000, 0.015, 3.0);
		mp_HarrisRight = new HarrisDetector("HarrisRight"+getName());
		mp_HarrisRight->setParam(1000, 0.015, 3.0);
	}
	
	mp_simmat = cvCreateMat(1, 1000, CV_32FC1);
	checkmat = cvCreateMat(240, 320, CV_32FC1);

	m_leftregiondetect.allocate(1000, 5);
	m_rightregiondetect.allocate(1000, 5);

	mp_GaborLeft = new GaborDescriptor("GaborLeft"+getName());
	mp_GaborLeft->setParam(); // default parameter set
	mp_GaborRight = new GaborDescriptor("GaborRight"+getName());
	mp_GaborRight->setParam();

	mp_featleft = NULL;
	mp_featright = NULL;
	
	
	trackOutL.setBuffer(NULL);
	trackOutR.setBuffer(NULL);
	
//	m_opmode = EXHAUSTIVE;
	
	debug = false;

	m_pointsthres = 10;
	m_detectcounter = 0;
	m_detectthres = 5;
	forceMaxSizeL = false;//true
	forceMaxSizeR = false;//true
	m_maxsize = 100;
	
	setParam();
		
/*
	cvLeftImageIn.setModule(this);
	cvRightImageIn.setModule(this);
	cvLeftGrayImageIn.setModule(this);
	cvRightGrayImageIn.setModule(this);

	this->setParam(thres, horizontalDisp, verticalDisp);

	mp_leftInterestPoints = NULL;
	mp_rightInterestPoints = NULL;
	mp_leftGaborResponses = NULL;
	mp_rightGaborResponses = NULL;
	mp_featureLeft = NULL;
	mp_featureRight = NULL;

	m_maxSizeResponsesLeft =  1000;
	double qualityLevelLeft = 0.015;
	double minDistanceLeft = 3;
	m_maxSizeResponsesRight =  1000;
	double qualityLevelRight = 0.015;
	double minDistanceRight = 3;
	mp_CornerFinderLeft = new CornerFinder2(m_maxSizeResponsesLeft, qualityLevelLeft, minDistanceLeft, "CornerFinder Left module");  
	mp_CornerFinderRight = new CornerFinder2(m_maxSizeResponsesRight, qualityLevelRight, minDistanceRight, "CornerFinder Right module");  
	mp_CornerDrawerLeft = new CornerDrawer2("CornerDrawer2 Left module");
	mp_CornerDrawerRight = new CornerDrawer2("CornerDrawer2 Right module");
	mp_DisparityDrawer = new CSCDrawDisparity2("CSCDrawDisparity2 left module", 3);
#ifdef Use_FastGabor
	mp_GaborLeft = new FastGabor("FastGabor left module");
	mp_GaborRight = new FastGabor("FastGabor right module");
	mp_GaborLeft->initialize(5, 8, 0.8, 0.6, 19, 1.5*3.14159, 4.0);
	mp_GaborRight->initialize(5, 8, 0.8, 0.6, 19, 1.5*3.14159, 4.0);
#else
	mp_GaborLeft = new GaborResponse(false, "GaborResponse left module");
	mp_GaborRight = new GaborResponse(false, "GaborResponse right module");
	mp_GaborLeft->initialize(51, 51, 5, 8, 1.0/sqrt(2.0), 1.7, 2*3.1416, 1.0);
	mp_GaborRight->initialize(51, 51, 5, 8, 1.0/sqrt(2.0), 1.7, 2*3.1416, 1.0);
#endif

	// method 0 for exhaustive comparison, method 1 for only finding IPs on the plane of fixation
	m_opmode = 0;
	m_drawResult = true;
	m_drawIP = true;
	m_sizeResponsesLeft = 0;
	m_sizeResponsesRight = 0;
	m_count = 0;
	m_count2 = 0;
	m_ignore = 5;
	m_noIPthreshold	= 10;
	m_detect = false;

	mp_classified = new int [m_maxSizeResponsesLeft];
	mp_disparityx = new float [m_maxSizeResponsesLeft];
	mp_disparityy = new float [m_maxSizeResponsesLeft];
	mp_responsesLeft = new float [40*m_maxSizeResponsesLeft];
	mp_responsesRight = new float [40*m_maxSizeResponsesRight];
	mp_magLeft = new float [m_maxSizeResponsesLeft];
	mp_magRight = new float [m_maxSizeResponsesRight];
//	mp_simmat = new float [m_maxSizeResponsesRight];
	IP_x = new Ipp32f [m_maxSizeResponsesLeft];
	IP_y = new Ipp32f [m_maxSizeResponsesLeft];
	median_x = new Ipp32f [m_maxSizeResponsesLeft];
	median_y = new Ipp32f [m_maxSizeResponsesLeft];
//	mp_winnerData160x120 = new TrackData [1];
//	mp_winnerData320x240 = new TrackData [1];
//	mp_winnerData640x480 = new TrackData [1];
	
	min_val = new double [1];
	max_val = new double [1];
	min_loc = new CvPoint [1];
	max_loc = new CvPoint [1];

//	winner160x120Out.setBuffer(mp_winnerData160x120);
//	winner320x240Out.setBuffer(mp_winnerData320x240);
//	winner640x480Out.setBuffer(mp_winnerData640x480);
	winnerOut.setBuffer(&m_winner);

	debug = false;

	endThread = false;
#ifdef USE_PTHREADS

	pthread_mutex_init(&mutex_threadleft, NULL);
//	pthread_mutex_init(&mutex_threadright, NULL);
	pthread_mutex_init(&mutex_leftwaitlock, NULL);
//	pthread_mutex_init(&mutex_rightwaitlock, NULL);

	pthread_cond_init(&condition_startthread, NULL);

	int  iret1, iret2;
	iret1 = pthread_create( &thread1, NULL, computeleft, this);
//	iret2 = pthread_create( &thread2, NULL, computeright, this);
#else
#endif

//	scale = 0.5;
	forceMinSize = true;
	minSize = 50;

	std::cout<< "finished.\n";
*/

	std::cout<< "\nConstructing " << getName() << "...finished.";
}

DetectSavedObject::~DetectSavedObject(){

	std::cout << "\nDestructing " << getName() << "...started";
	if(arrayL)
		free(arrayL);
	if(arrayR)
		free(arrayR);

	if(mp_featleft) {
		mp_featleft->point(NULL);
		delete mp_featleft;
	}
	if(mp_featright) {
		mp_featright->point(NULL);
		delete mp_featright;
	}

	cvReleaseMat(&mp_simmat);
	cvReleaseMat(&checkmat);

	delete mp_HarrisLeft;
	delete mp_HarrisRight;
//start Gab's add
	if(mp_DogLeft)
		delete mp_DogLeft;
	if(mp_DogRight)
		delete mp_DogRight;
//end Gab's add
	delete mp_GaborLeft;
	delete mp_GaborRight;


/*
	std::cout<< "Destructing " << getName() << "...";
#ifdef USE_PTHREADS

	endThread = true;

	pthread_mutex_lock( &(mutex_threadleft) );
//	pthread_mutex_lock( &(mutex_threadright) );

	pthread_mutex_unlock( &(mutex_threadleft) );
//	pthread_mutex_unlock( &(mutex_threadright) );

	pthread_cond_broadcast( &condition_startthread);

	pthread_mutex_lock( &(mutex_leftwaitlock) );
//	pthread_mutex_lock( &(mutex_rightwaitlock) );

	pthread_mutex_lock( &(mutex_threadleft) );
//	pthread_mutex_lock( &(mutex_threadright) );

	pthread_mutex_unlock( &(mutex_threadleft) );
//	pthread_mutex_unlock( &(mutex_threadright) );

	pthread_mutex_unlock( &(mutex_leftwaitlock) );
//	pthread_mutex_unlock( &(mutex_rightwaitlock) );


	pthread_mutex_destroy(&mutex_threadleft);
//	pthread_mutex_destroy(&mutex_threadright);
	pthread_mutex_destroy(&mutex_leftwaitlock);
//	pthread_mutex_destroy(&mutex_rightwaitlock);

	pthread_cond_destroy(&condition_startthread);

#else
#endif

//	delete [] mp_winnerData160x120;
//	delete [] mp_winnerData320x240;
//	delete [] mp_winnerData640x480;
	delete [] median_x;
	delete [] median_y;
	delete [] IP_x;
	delete [] IP_y;
//	delete [] mp_simmat;
	delete [] mp_magLeft;
	delete [] mp_magRight;
	delete [] mp_responsesLeft;
	delete [] mp_responsesRight;
	delete [] mp_disparityx;
	delete [] mp_disparityy;
	delete [] mp_classified;
	delete [] min_val;
	delete [] max_val;
	delete [] min_loc;
	delete [] max_loc;
	std::cout<< "finished.\n";
*/

	std::cout << "\nfinished.";
}

void DetectSavedObject::setParam(float simthres, float hordispthres, float verdispthres, unsigned int opmode) {
	if(simthres >= 1.0 || simthres <= 0.0) { std::cerr << getName() << "::setParam()::ERROR::simthres out of range (0, 1)!...\n"; return; }

	if(hordispthres <= 0.0) { std::cerr << getName() << "::setParam()::ERROR::hordispthres out of range (0, inf)!...\n"; return; }

	if(verdispthres <= 0.0) { std::cerr << getName() << "::setParam()::ERROR::verdispthres out of range (0, inf)!...\n"; return; }

	cvSetZero(checkmat);
	m_simthres = simthres;
	m_hordispthres = hordispthres;
	m_verdispthres = verdispthres;
	m_opmode = opmode;

}

void DetectSavedObject::execute() {

	extractinterestregions();

	computegabordescriptors();

	unsigned int points = compareleftright();

	bool detect = checkcondition(points);
	if(debug) {
		if(detect) { std::cout << getName() << "::execute()::Object detected " << m_detectcounter << " !\n"; m_detectcounter++; }
		else { std::cout << getName() << "::execute()::Object NOT detected!\n"; }
	}
	
	if(detect && m_detectcounter > m_detectthres) {
		trackOutL.setBuffer(&m_trackL);
		trackOutR.setBuffer(&m_trackR);
	}
	else{
		trackOutL.setBuffer(NULL);
		trackOutR.setBuffer(NULL);
	}
	
	trackOutL.out();
	trackOutR.out();
	
	
/*
	if(debug) std::cout << getName() << "::execute()::Line 1\n";

	m_detect = false;

	mp_cvleftrgbimg = cvLeftImageIn.getBuffer();
	mp_cvrightrgbimg = cvRightImageIn.getBuffer();
	if(mp_cvleftrgbimg == NULL || mp_cvrightrgbimg == NULL) { std::cerr << getName() << "::execute()::ERROR::One of inputs is NULL!...\n"; return; }

	mp_cvleftgrayimg = cvLeftGrayImageIn.getBuffer();
	mp_cvrightgrayimg = cvRightGrayImageIn.getBuffer();
	if(mp_cvleftgrayimg == NULL || mp_cvrightgrayimg == NULL) { std::cerr << getName() << "::execute()::ERROR::One of inputs is NULL!...\n"; return; }

#ifdef USE_PTHREADS
	pthread_mutex_lock( &(mutex_threadleft) );
//	pthread_mutex_lock( &(mutex_threadright) );

	pthread_mutex_unlock( &(mutex_threadleft) );
//	pthread_mutex_unlock( &(mutex_threadright) );

	pthread_cond_broadcast( &condition_startthread);

//	if(debug) std::cout << getName() << "::execute()::Waiting for threads to complete\n";
//	usleep(1); // Sleep for 1 microseconds (calling thread)
	this->computerightnothread();

	pthread_mutex_lock( &(mutex_leftwaitlock) );
//	pthread_mutex_lock( &(mutex_rightwaitlock) );

	pthread_mutex_lock( &(mutex_threadleft) );
//	pthread_mutex_lock( &(mutex_threadright) );

	pthread_mutex_unlock( &(mutex_threadleft) );
//	pthread_mutex_unlock( &(mutex_threadright) );

	pthread_mutex_unlock( &(mutex_leftwaitlock) );
//	pthread_mutex_unlock( &(mutex_rightwaitlock) );

	if(debug) std::cout << getName() << "::execute()::Line 2\n";
#else
	this->extractIP();
	this->extractGaborResponse();
#endif

	this->compareResponses();
	if(m_drawResult == true) this->drawDisparity();
	this->checkCondition();	
*/
}

void DetectSavedObject::extractinterestregions() {

	mp_cvleftgrayimg = cvCameraGrayImageIn.getBuffer();
	mp_cvrightgrayimg = cvDatabaseGrayImageIn.getBuffer();
	if(!mp_cvleftgrayimg || !mp_cvrightgrayimg) { std::cerr << getName() << "::extractinterestregions()::One of imageIn is NULL!\n"; return; }

/*	cvNamedWindow("1");
	cvShowImage("1",mp_cvleftgrayimg->ipl);
	cvNamedWindow("2");
	cvShowImage("2",mp_cvrightgrayimg->ipl);
	cvWaitKey(1);*/
	m_width = mp_cvleftgrayimg->width;
	m_height = mp_cvleftgrayimg->height;

	if(Dog == true){
		// Extract interest points from left gray image
		mp_DogLeft->cvGrayImageIn.setBuffer(mp_cvleftgrayimg);
		mp_DogLeft->execute();
		mp_leftregions = mp_DogLeft->regionsOut.getBuffer();
		// Extract interest points from right gray image
		mp_DogRight->cvGrayImageIn.setBuffer(mp_cvrightgrayimg);
		mp_DogRight->executeR();
		mp_rightregions = mp_DogRight->regionsOut.getBuffer();
	}

	if(Harris == true){
		// Extract interest points from left gray image
		mp_HarrisLeft->cvGrayImageIn.setBuffer(mp_cvleftgrayimg);
		mp_HarrisLeft->execute();
		mp_leftregions = mp_HarrisLeft->regionsOut.getBuffer();
		// Extract interest points from right gray image
		mp_HarrisRight->cvGrayImageIn.setBuffer(mp_cvrightgrayimg);
		mp_HarrisRight->execute();
		mp_rightregions = mp_HarrisRight->regionsOut.getBuffer();
	}

//	regionsLeftOut.setBuffer(mp_leftregions);
//	regionsLeftOut.out();
//	regionsRightOut.setBuffer(mp_rightregions);
//	regionsRightOut.out();

/*
	// Draw interest points on left & right color images
	if(m_drawIP == true){
		mp_CornerDrawerLeft->cvImageIn.in(mp_cvleftrgbimg);	
		mp_CornerDrawerLeft->pointsIn.in(mp_leftInterestPoints);	
		mp_CornerDrawerLeft->execute();	
		CVImage* tempLeft = mp_CornerDrawerLeft->cvImageOut.getBuffer();	
		mp_CornerDrawerRight->cvImageIn.in(mp_cvrightrgbimg);	
		mp_CornerDrawerRight->pointsIn.in(mp_rightInterestPoints);	
		mp_CornerDrawerRight->execute();	
		CVImage* tempRight = mp_CornerDrawerRight->cvImageOut.getBuffer();	
		cvLeftImageOut.setBuffer(tempLeft);
		cvLeftImageOut.out();
		cvRightImageOut.setBuffer(tempRight);
		cvRightImageOut.out();
	}
*/
}

void DetectSavedObject::computegabordescriptors() {
	if(!mp_cvleftgrayimg || !mp_cvrightgrayimg || !mp_leftregions || !mp_rightregions) { std::cerr << getName() << "::computegabordescriptors()::One of input is NULL!\n"; return; }

	// Extract gabor responses from left gray image
	mp_GaborLeft->cvGrayImageIn.in(mp_cvleftgrayimg);
	mp_GaborLeft->regionsIn.in(mp_leftregions);
	mp_GaborLeft->execute();
	mp_leftdesc = mp_GaborLeft->dataOut.getBuffer();
	// Extract gabor responses from right gray image
	mp_GaborRight->cvGrayImageIn.in(mp_cvrightgrayimg);
	mp_GaborRight->regionsIn.in(mp_rightregions);
	mp_GaborRight->execute();
	mp_rightdesc = mp_GaborRight->dataOut.getBuffer();
}

unsigned int DetectSavedObject::compareleftright() {
	// Setting rfeature pointer to correct type
	if(!mp_featleft){
		mp_featleft = reinterpret_cast<CSCFeature*>(GetNewDataPtr(mp_leftdesc->getDescriptorType()));
		mp_featright = reinterpret_cast<CSCFeature*>(GetNewDataPtr(mp_rightdesc->getDescriptorType()));
	}	
	unsigned int sizeleft = mp_leftdesc->getCurrentSize();
	unsigned int sizeright = mp_rightdesc->getCurrentSize();
//	if( m_sizeResponsesLeft > m_maxSizeResponsesLeft || m_sizeResponsesRight > m_maxSizeResponsesRight){ std::cerr << getName() << "::ERROR::compareResponses()::Either m_sizeResponsesLeft or m_sizeResponsesRight over the limit!\n"; return; }
//	m_stepResponse = mp_featureLeft->size();

//	int i, j;
	//float sim2;
	//int* pIndex = new int [1];
//	float tempdispx, tempdispy;

	unsigned int descwidthleft = mp_leftdesc->getWidth();
	unsigned int descwidthright = mp_rightdesc->getWidth();
	float* descdataleft = mp_leftdesc->getData();
	float* descdataright = mp_rightdesc->getData();

	unsigned int regionwidthleft = mp_leftregions->getWidth();
	unsigned int regionwidthright = mp_rightregions->getWidth();
	float* regiondataleft = mp_leftregions->getData();
	float* regiondataright = mp_rightregions->getData();

	// Exhaustive comparison (all to all)
	
	unsigned int pointsbehind = 0, pointsinfront = 0, pointson = 0, pointsbelowthres = 0, pointsaboveverdispthres = 0;

	mvec_indexesonleft.clear();
	mvec_indexesonright.clear();
	mvec_hordisp.clear();
	mvec_verdisp.clear();
	
	float* detectregionleft = m_leftregiondetect.getData();
	float* detectregionright = m_rightregiondetect.getData();
	unsigned int detectregionwidth = m_leftregiondetect.getWidth();
	unsigned int detectregioncounter = 0;
	
	int point_not_stable = 0;

	cvSetZero(mp_simmat);

	if(m_opmode == 0){

		for(unsigned int i = 0;i<sizeleft;++i){
			mp_featleft->point(&(descdataleft[i*descwidthleft]));
			for(unsigned int j = 0;j<sizeright;++j){
				mp_featright->point(&(descdataright[j*descwidthright]));
				cvmSet(mp_simmat, 0, j, mp_featright->similarity(*mp_featleft) );
			}
			//sim2 = this->findMax(mp_simmat, sizeright, pIndex);
			cvMinMaxLoc(mp_simmat, &m_minval, &m_maxval, &m_minloc, &m_maxloc, NULL);
			//sim2 = *max_val;			
			if(m_maxval > (double)m_simthres) {
				unsigned int rightindex = m_maxloc.x;
				float dispx = regiondataleft[i*regionwidthleft + 0] - regiondataright[(rightindex)*regionwidthright + 0];
				float dispy = regiondataleft[i*regionwidthleft + 1] - regiondataright[(rightindex)*regionwidthright + 1];

				float distxL = regiondataleft[i*regionwidthleft + 0] - 160;
				float distyL = regiondataleft[i*regionwidthleft + 1]-120;
				float distxR = regiondataright[(rightindex)*regionwidthright + 0] - 160;
				float distyR = regiondataright[(rightindex)*regionwidthright + 1]-120;

				float distleft = sqrt(distxL*distxL + distyL*distyL);
				float distright = sqrt(distxR*distxR + distyR*distyR);

				if (false)
				{
					printf("x_left: %lf, y_left: %lf, x_right: %lf, y_right: %lf, Disp x: %lf, Disp y: %lf \n", 						regiondataleft[i*regionwidthleft + 0], regiondataleft[i*regionwidthleft + 1],  						regiondataright[(rightindex)*regionwidthright + 0], regiondataright[(rightindex)*regionwidthright + 1],  						dispx, dispy);
				}

				int xchk=regiondataleft[i*regionwidthleft + 0];
				int ychk=regiondataleft[i*regionwidthleft + 1];
				int t = cvmGet(checkmat,ychk,xchk)+2.0;
				cvmSet(checkmat,ychk,xchk,t);
				int g = cvmGet(checkmat,ychk,xchk);
				//if(g>80)
				//{


					//if(distleft <70 && distright<70){

						//mp_disparityx[i] = mp_leftInterestPoints->points[i].x - mp_rightInterestPoints->points[m_maxloc.x].x;
						//mp_disparityy[i] = mp_leftInterestPoints->points[i].y - mp_rightInterestPoints->points[m_maxloc.x].y;
						if(fabs(dispy) < m_verdispthres){
							if(dispx < -m_hordispthres){
								// Points behind the plane of fixation (blue circles)
								//mp_classified[i] = 1;
								pointsbehind++;
							}
							else if(dispx > m_hordispthres){
								// Points in front of the plane of fixation (red circles)
								//mp_classified[i] = -1;

								pointsinfront++;
								mvec_indexesonleft.push_back(i);
								mvec_indexesonright.push_back(rightindex);
								mvec_hordisp.push_back(dispx);
								mvec_verdisp.push_back(dispy);

								detectregionleft[detectregioncounter*detectregionwidth+0] = 									regiondataleft[i*regionwidthleft + 0];
								
								detectregionleft[detectregioncounter*detectregionwidth+1] = 									regiondataleft[i*regionwidthleft + 1];

								detectregionright[detectregioncounter*detectregionwidth+0] = 									regiondataright[rightindex*regionwidthright + 0];

								detectregionright[detectregioncounter*detectregionwidth+1] = 									regiondataright[rightindex*regionwidthright + 1];



								detectregioncounter++;

							}
							else{
								// Points on the plane of fixation (green circles)
								//mp_classified[i] = 0;
								pointson++;
								mvec_indexesonleft.push_back(i);
								mvec_indexesonright.push_back(rightindex);
								mvec_hordisp.push_back(dispx);
								mvec_verdisp.push_back(dispy);

								detectregionleft[detectregioncounter*detectregionwidth+0] = 									regiondataleft[i*regionwidthleft + 0];

								detectregionleft[detectregioncounter*detectregionwidth+1] = 									regiondataleft[i*regionwidthleft + 1];

								detectregionright[detectregioncounter*detectregionwidth+0] = 									regiondataright[rightindex*regionwidthright + 0];

								detectregionright[detectregioncounter*detectregionwidth+1] = 									regiondataright[rightindex*regionwidthright + 1];

								detectregioncounter++;
							}
						}
						else{
							// No match found (too large vertical disparity)
							//mp_classified[i] = 100;
							pointsaboveverdispthres++;
						}
					/*}
					else
					{	
						if(false)
							printf("Point out of fovea");
					}*/
				/*}
				else{
					point_not_stable++;
				}*/

			}
			else{
				// No match found (max. similarity is below threshold)
				//mp_classified[i] = 100;
				pointsbelowthres++;
			}
		}



		CvScalar inc[1]={

				{{0.1,0,0,0}},
		};
		//cvSubS(checkmat,inc[0],checkmat,NULL);
		//cvThreshold(checkmat,checkmat,0,10,CV_THRESH_TOZERO);
		//cvThreshold(checkmat,checkmat,10,10,CV_THRESH_TRUNC);



	}







	// Only comparing interest points within certain bounding box
	else if(m_opmode == 1){
		// ERROR :: Currently this method is finding faulty interest points with thres = 0.93, higher the thres?
		for(unsigned int i = 0;i<sizeleft;++i){
			mp_featleft->point(&(descdataleft[i*descwidthleft]));
			int counter = 0;
			for(unsigned int j = 0;j<sizeright;++j){
				mp_featright->point(&(descdataright[j*descwidthright]));
				float dispx = regiondataleft[i*regionwidthleft + 0] - regiondataright[j*regionwidthright + 0];
				float dispy = regiondataleft[i*regionwidthleft + 1] - regiondataright[j*regionwidthright + 1];
				if(fabs(dispy) < m_verdispthres && fabs(dispx) < m_hordispthres){
					//mp_simmat[tempcounter] = mp_featright->similarity(*mp_featleft);
					cvmSet(mp_simmat, 0, counter, mp_featright->similarity(*mp_featleft) );
					counter++;
				}
			}
			//sim2 = this->findMax(mp_simmat, tempcounter, pIndex);
			cvMinMaxLoc(mp_simmat, &m_minval, &m_maxval, &m_minloc, &m_maxloc, NULL);
			//sim2 = *max_val;			
			if(m_maxval > (double)m_simthres){
				unsigned int rightindex = m_maxloc.x;
				float dispx = regiondataleft[i*regionwidthleft + 0] - regiondataright[(rightindex)*regionwidthright + 0];
				float dispy = regiondataleft[i*regionwidthleft + 1] - regiondataright[(rightindex)*regionwidthright + 1];
				pointson++;
				mvec_indexesonleft.push_back(i);
				mvec_indexesonright.push_back(rightindex);
				mvec_hordisp.push_back(dispx);
				mvec_verdisp.push_back(dispy);

				detectregionleft[detectregioncounter*detectregionwidth+0] = regiondataleft[i*regionwidthleft + 0];
				detectregionleft[detectregioncounter*detectregionwidth+1] = regiondataleft[i*regionwidthleft + 1];
						
				detectregionright[detectregioncounter*detectregionwidth+0] = regiondataright[rightindex*regionwidthright + 0];
				detectregionright[detectregioncounter*detectregionwidth+1] = regiondataright[rightindex*regionwidthright + 1];
						
				detectregioncounter++;

			}
			else{
				// No match found (max. similarity is below threshold)
				//mp_classified[i] = 100;
				pointsbelowthres++;
			}
		}
	}
	// Only comparing interest points within vertical limit
	else if(m_opmode == 2){
		for(unsigned int i = 0;i<sizeleft;++i){
			mp_featleft->point(&(descdataleft[i*descwidthleft]));
			int counter = 0;
			for(unsigned int j = 0;j<sizeright;++j){
				mp_featright->point(&(descdataright[j*descwidthright]));
				//float dispx = regiondataleft[i*regionwidthleft + 0] - regiondataright[j*regionwidthleft + 0];
				float dispy = regiondataleft[i*regionwidthleft + 1] - regiondataright[j*regionwidthright + 1];

				if(fabs(dispy) < m_verdispthres){
					//mp_simmat[tempcounter] = mp_featright->similarity(*mp_featleft);
					cvmSet(mp_simmat, 0, counter, mp_featright->similarity(*mp_featleft) );
					counter++;
				}
			}
			//sim2 = this->findMax(mp_simmat, tempcounter, pIndex);
			cvMinMaxLoc(mp_simmat, &m_minval, &m_maxval, &m_minloc, &m_maxloc, NULL);
			//sim2 = *max_val;			
			if(m_maxval > (double)m_simthres){
				unsigned int rightindex = m_maxloc.x;
				float dispx = regiondataleft[i*regionwidthleft + 0] - regiondataright[(rightindex)*regionwidthright + 0];
				float dispy = regiondataleft[i*regionwidthleft + 1] - regiondataright[(rightindex)*regionwidthright + 1];

				if(dispx < -m_hordispthres){
					// Points behind the plane of fixation (blue circles)
					//mp_classified[i] = 1;
					pointsbehind++;
				}
				else if(dispx > m_hordispthres){
					// Points in front of the plane of fixation (red circles)
					//mp_classified[i] = -1;
					pointsinfront++;
					mvec_indexesonleft.push_back(i);
					mvec_indexesonright.push_back(rightindex);
					mvec_hordisp.push_back(dispx);
					mvec_verdisp.push_back(dispy);

					detectregionleft[detectregioncounter*detectregionwidth+0] = regiondataleft[i*regionwidthleft + 0];
					detectregionleft[detectregioncounter*detectregionwidth+1] = regiondataleft[i*regionwidthleft + 1];
						
					detectregionright[detectregioncounter*detectregionwidth+0] = regiondataright[rightindex*regionwidthright + 0];
					detectregionright[detectregioncounter*detectregionwidth+1] = regiondataright[rightindex*regionwidthright + 1];
						
					detectregioncounter++;
				}
				else{
					// Points on the plane of fixation (green circles)
					//mp_classified[i] = 0;
					pointson++;
					mvec_indexesonleft.push_back(i);
					mvec_indexesonright.push_back(rightindex);
					mvec_hordisp.push_back(dispx);
					mvec_verdisp.push_back(dispy);

					detectregionleft[detectregioncounter*detectregionwidth+0] = regiondataleft[i*regionwidthleft + 0];
					detectregionleft[detectregioncounter*detectregionwidth+1] = regiondataleft[i*regionwidthleft + 1];
						
					detectregionright[detectregioncounter*detectregionwidth+0] = regiondataright[rightindex*regionwidthright + 0];
					detectregionright[detectregioncounter*detectregionwidth+1] = regiondataright[rightindex*regionwidthright + 1];
						
					detectregioncounter++;
				}
			}
			else{
				// No match found (max. similarity is below threshold)
				//mp_classified[i] = 100;
				pointsbelowthres++;
			}
		}
	}
	else{
		std::cerr << getName() << "::compareleftright()::ERROR::Unsupported mode!\n";
	}

	if(debug) {
		std::cout << getName() << "::compareleftright()\n";
		std::cout << "sizeleft = " << sizeleft << "\n";
		std::cout << "pointson = " << pointson << "\n";
		std::cout << "pointsbehind = " << pointsbehind << "\n";
		std::cout << "pointsinfront = " << pointsinfront << "\n";
		std::cout << "pointsbelowthres = " << pointsbelowthres << "\n";
		std::cout << "pointsaboveverdispthres = " << pointsaboveverdispthres << "\n";
	}

	m_leftregiondetect.setCurrentSize(detectregioncounter);
	m_rightregiondetect.setCurrentSize(detectregioncounter);

	regionsLeftOut.setBuffer(&m_leftregiondetect);
	regionsLeftOut.out();
	regionsRightOut.setBuffer(&m_rightregiondetect);
	regionsRightOut.out();

	int feat_count =0;
	for(int i =0; i<320;i++){
				for(int j=0; j<240;j++){
					double y = (double)cvmGet(checkmat,j,i);
					if (y>0){
						y=y-1;
						cvmSet(checkmat,j,i,y);
						feat_count ++;
					}
					else cvmSet(checkmat,j,i,0);
				}
			}

	return (pointson+pointsinfront);
}

bool DetectSavedObject::checkcondition(unsigned int points) {
//	m_count = m_count + 1;
//	if(m_count < m_ignore) return;

	m_trackL.reliability = 0.0;
	m_trackL.imageSize.width = mp_cvleftgrayimg->width;
	m_trackL.imageSize.height = mp_cvleftgrayimg->height;

	m_trackR.reliability = 0.0;
	m_trackR.imageSize.width = mp_cvrightgrayimg->width;
	m_trackR.imageSize.height = mp_cvrightgrayimg->height;

	if(points < m_pointsthres) return false;

	// Computing the object position (median) and size (std dev) (left image and right)

	unsigned int regionwidthleft = mp_leftregions->getWidth();
	unsigned int regionwidthright = mp_rightregions->getWidth();
	float* regiondataleft = mp_leftregions->getData();
	float* regiondataright = mp_rightregions->getData();

	unsigned int sizeL = mvec_indexesonleft.size();
	float meanxL = 0.0;
	float meanyL = 0.0;
	float stdvxL = 0.0;
	float stdvyL = 0.0;
	float medianxL = 0.0;
	float medianyL = 0.0;

	unsigned int sizeR = mvec_indexesonright.size();
	float meanxR = 0.0;
	float meanyR = 0.0;
	float stdvxR = 0.0;
	float stdvyR = 0.0;
	float medianxR = 0.0;
	float medianyR = 0.0;



	std::set<float> tempsetxL;
	std::set<float> tempsetyL;

	std::set<float> tempsetxR;
	std::set<float> tempsetyR;

	std::vector<unsigned int>::iterator enditerL = mvec_indexesonleft.end();
	for(std::vector<unsigned int>::iterator iterL = mvec_indexesonleft.begin(); iterL != enditerL; ++iterL) {
		unsigned int indexL = *iterL;
		float posxL = regiondataleft[indexL*regionwidthleft + 0];
		float posyL = regiondataleft[indexL*regionwidthleft + 1];
		
		meanxL += posxL;
		meanyL += posyL;
		
		stdvxL += (posxL*posxL);
		stdvyL += (posyL*posyL);

		tempsetxL.insert(posxL);
		tempsetyL.insert(posyL);
	}

	std::vector<unsigned int>::iterator enditerR = mvec_indexesonright.end();
		for(std::vector<unsigned int>::iterator iterR = mvec_indexesonright.begin(); iterR != enditerR; ++iterR) {
			unsigned int indexR = *iterR;
			float posxR = regiondataright[indexR*regionwidthright + 0];
			float posyR = regiondataright[indexR*regionwidthright + 1];

			meanxR += posxR;
			meanyR += posyR;

			stdvxR += (posxR*posxR);
			stdvyR += (posyR*posyR);

			tempsetxR.insert(posxR);
			tempsetyR.insert(posyR);
		}


	meanxL /= (float)sizeL;
	meanyL /= (float)sizeL;


	meanxR /= (float)sizeR;
	meanyR /= (float)sizeR;


	stdvxL /= (float)sizeL;
	stdvyL /= (float)sizeL;

	
	stdvxR /= (float)sizeR;
	stdvyR /= (float)sizeR;


	stdvxL -= (meanxL*meanxL);
	stdvyL -= (meanyL*meanyL);

	
	stdvxR -= (meanxR*meanxR);
	stdvyR -= (meanyR*meanyR);

	if( sizeL%2 == 0) {
		unsigned int indexL = sizeL/2 - 1;
		std::set<float>::iterator iterxL = tempsetxL.begin();
		std::set<float>::iterator iteryL = tempsetyL.begin();
		for(unsigned int i = 0;i<indexL;++i) {
			++iterxL;
			++iteryL;
		}

		medianxL += (*iterxL);
		medianyL += (*iteryL);

		++iterxL;
		++iteryL;

		medianxL += (*iterxL);
		medianyL += (*iteryL);

		medianxL /= 2.0;
		medianyL /= 2.0;

	}
	else{
		unsigned int indexL = (sizeL-1)/2;
		std::set<float>::iterator iterxL = tempsetxL.begin();
		std::set<float>::iterator iteryL = tempsetyL.begin();
		for(unsigned int i = 0;i<indexL;++i) {
			++iterxL;
			++iteryL;
		}

		medianxL = *iterxL;
		medianyL = *iteryL;

	}


	if( sizeR%2 == 0) {
		unsigned int indexR = sizeR/2 - 1;
		std::set<float>::iterator iterxR = tempsetxR.begin();
		std::set<float>::iterator iteryR = tempsetyR.begin();
		for(unsigned int i = 0;i<indexR;++i) {
			++iterxR;
			++iteryR;
		}

		medianxR += (*iterxR);
		medianyR += (*iteryR);

		++iterxR;
		++iteryR;

		medianxR += (*iterxR);
		medianyR += (*iteryR);

		medianxR /= 2.0;
		medianyR /= 2.0;

	}
	else{
		unsigned int indexR = (sizeR-1)/2;
		std::set<float>::iterator iterxR = tempsetxR.begin();
		std::set<float>::iterator iteryR = tempsetyR.begin();
		for(unsigned int i = 0;i<indexR;++i) {
			++iterxR;
			++iteryR;
		}
		
		medianxR = *iterxR;
		medianyR = *iteryR;
		
	}


	stdvxL = sqrt(stdvxL);
	stdvyL = sqrt(stdvyL);


	stdvxR = sqrt(stdvxR);
	stdvyR = sqrt(stdvyR);



	if(debug) {
		std::cout << getName() << "::checkcondition()\n";
		std::cout << "points = " << points << "\n";
		std::cout << "mean left = (" << meanxL << "," << meanyL << ")\n";
		std::cout << "median left = (" << medianxL << "," << medianyL << ")\n";
		std::cout << "std left = (" << stdvxL << "," << stdvyL << ")\n";

		std::cout << "mean right = (" << meanxR << "," << meanyR << ")\n";
		std::cout << "median right = (" << medianxR << "," << medianyR << ")\n";
		std::cout << "std right = (" << stdvxR << "," << stdvyR << ")\n";
	}



//start adds gabriele
	float maxXL = 160.0;
	float minXL = 160.0;
	float maxYL = 120.0;
	float minYL = 120.0;

	float maxXR = 160.0;
	float minXR = 160.0;
	float maxYR = 120.0;
	float minYR = 120.0;



	std::vector<unsigned int>::iterator enditermL = mvec_indexesonleft.end();
	for(std::vector<unsigned int>::iterator itermL = mvec_indexesonleft.begin(); itermL != enditermL; ++itermL) {
		unsigned int indexmL = *itermL;
		float posxmL = regiondataleft[indexmL*regionwidthleft + 0];
		float posymL = regiondataleft[indexmL*regionwidthleft + 1];

		if (posxmL>maxXL){// && posxmL-maxXL<stdvxL){
			maxXL=posxmL;
		}
		if (posxmL<minXL){// && minXL-posxmL<stdvxL){
			minXL=posxmL;
		}
		if (posymL>maxYL){// && posymL-maxYL<stdvyL){
			maxYL=posymL;
		}
		if (posymL<minYL){// && minYL-posymL<stdvyL){
			minYL=posymL;
		}

	}


	maxPosL = cvPoint(cvRound(maxXL),cvRound(maxYL));
	minPosL = cvPoint(cvRound(minXL),cvRound(minYL));


//	m_trackL.winnerPos.x = cvRound((meanxL + ((maxPosL.x+minPosL.x)/2+minPosL.x))/2);
//	m_trackL.winnerPos.y = cvRound((meanyL + ((maxPosL.y+minPosL.y)/2+minPosL.y))/2);

	m_trackL.winnerPos.x = cvRound((maxPosL.x+minPosL.x)/2);//COMM
	m_trackL.winnerPos.y = cvRound((maxPosL.y+minPosL.y)/2);//COMM

//	m_trackL.winnerPos.x = cvRound(meanxL);//COMM
//	m_trackL.winnerPos.y = cvRound(meanyL);//COMM

	std::vector<unsigned int>::iterator enditermR = mvec_indexesonright.end();
	for(std::vector<unsigned int>::iterator itermR = mvec_indexesonright.begin(); itermR != enditermR; ++itermR) {
		unsigned int indexmR = *itermR;
		float posxmR = regiondataright[indexmR*regionwidthright + 0];
		float posymR = regiondataright[indexmR*regionwidthright + 1];

		if (posxmR>maxXR){// && posxmR-maxXR<stdvxR){
			maxXR=posxmR;
		}
		if (posxmR<minXR){// && minXR-posxmR<stdvxR){
			minXR=posxmR;
		}
		if (posymR>maxYR){// && posymR-maxYR<stdvyR){
			maxYR=posymR;
		}
		if (posymR<minYR){// && minYR-posymR<stdvyR){
			minYR=posymR;
		}

	}


	maxPosR = cvPoint(cvRound(maxXR),cvRound(maxYR));
	minPosR = cvPoint(cvRound(minXR),cvRound(minYR));


//	m_trackR.winnerPos.x = cvRound((meanxR + ((maxPosR.x+minPosR.x)/2+minPosR.x))/2);
//	m_trackR.winnerPos.y = cvRound((meanyR + ((maxPosR.y+minPosR.y)/2+minPosR.y))/2);

	m_trackR.winnerPos.x = cvRound((maxPosR.x+minPosR.x)/2);//COMM
	m_trackR.winnerPos.y = cvRound((maxPosR.y+minPosR.y)/2);//COMM

//	m_trackR.winnerPos.x = cvRound(meanxR);//COMM
//	m_trackR.winnerPos.y = cvRound(meanyR);//COMM

//if(firstL==true)
	//calculatesizeL();
//if(firstR==true)
//	calculatesizeR();

//end adds gabriele

	int posxL = m_trackL.winnerPos.x;
	int posyL = m_trackL.winnerPos.y;

	int posxR = m_trackR.winnerPos.x;
	int posyR = m_trackR.winnerPos.y;





	double centerxL =160.0;//(double)posxL;// 160.0;//(double)posxL;
	double centeryL= 120.0;//(double)posyL;//120.0;//(double)posyL;
	double centerxR = 160.0;//(double)posxR;//160.0;//(double)posxR;
	double centeryR= 120.0;//(double)posyR;//120.0;//(double)posyR;

	for( int i1 = 0; i1 < oriCnt; i1++ )
	{
		*(arrayL+i1)=0.0;
		*(arrayR+i1)=0.0;
	}


	std::vector<unsigned int>::iterator enditersL = mvec_indexesonleft.end();
	for(std::vector<unsigned int>::iterator itersL = mvec_indexesonleft.begin(); itersL != enditersL; ++itersL) {
		unsigned int indexsL = *itersL;
		float posxsL = regiondataleft[indexsL*regionwidthleft + 0];
		float posysL = regiondataleft[indexsL*regionwidthleft + 1];

		   double distL = sqrt((posxsL-centerxL)*(posxsL-centerxL)+(posysL-centeryL)*(posysL-centeryL));
		   double angleL = 180.0+(atan2((posysL-centeryL),(posxsL-centerxL))*180.0/PI);
		   //printf("\nAngleR %f",angleR);
		   int angBinL=floor(angleL/(360.05/oriCnt));

		   /*if(posysL-centeryL<0.0)
		   {
			   if(posxsR-centerxR<0.0)
			   {
				   angBinR=(int)(binR+oriCnt/2);
			   }else
			   {
				   angBinR=(int)(binR+oriCnt/4*3);
			   }

		   }
		   else
		   {
			   if(posxsR-centerxR<0.0)
			   {
					angBinR=(int)(binR+oriCnt/4);
			   }
			   else
			   {
				   angBinR=(int)(binR);
			   }

		   }*/
		   if(distL>arrayL[angBinL])
			   arrayL[angBinL]=distL;


		   if(angBinL>oriCnt || angBinL<0){
		   	printf("\n\n\nERROR BINNING LEFT\n\n\n");
		   printf("angbilL = %d\n",angBinL);
		   }

	   }
	   double minAllL=10000.0;
	   int i3;
	   int i4;

		for( int i2 = 0; i2 < oriCnt; i2++ )
		{
			//printf("Array left: %f",arrayL[i2]);
			if(*(arrayL+i2)<minAllL)
				minAllL=*(arrayL+i2);
				if (minAllL <0.1)
				{
					i3 =i2-1;
					i4=i2+1;
					if(i2==0)
						i3=oriCnt-1;
					if(i2==(oriCnt-1))
						i4=0;

					minAllL=(arrayL[i3]>arrayL[i4])?arrayL[i4]:arrayL[i3];
				}
		}
		printf("\n");


		squarelenL = sqrt(2.0)*minAllL;


	std::vector<unsigned int>::iterator enditersR = mvec_indexesonright.end();
	for(std::vector<unsigned int>::iterator itersR = mvec_indexesonright.begin(); itersR != enditersR; ++itersR) {
		unsigned int indexsR = *itersR;
		float posxsR = regiondataright[indexsR*regionwidthright + 0];
		float posysR = regiondataright[indexsR*regionwidthright + 1];

	   //cvCircle(img,cvPoint(posxsR,posysR),2,CV_RGB(255,255,255), CV_FILLED );

	   double distR = sqrt((posxsR-centerxR)*(posxsR-centerxR)+(posysR-centeryR)*(posysR-centeryR));
	   double angleR = 180.0+(atan2((posysR-centeryR),(posxsR-centerxR))*180.0/PI);
	   //printf("\nAngleR %f",angleR);
	   int angBinR=floor(angleR/(360.05/oriCnt));

	   /*if(posysR-centeryR<0.0)
	   {
		   if(posxsR-centerxR<0.0)
		   {
			   angBinR=(int)(binR+oriCnt/2);
		   }else
		   {
			   angBinR=(int)(binR+oriCnt/4*3);
		   }

	   }
	   else
	   {
		   if(posxsR-centerxR<0.0)
		   {
				angBinR=(int)(binR+oriCnt/4);
		   }
		   else
		   {
			   angBinR=(int)(binR);
		   }

	   }*/
	   if(distR>arrayR[angBinR])
		   arrayR[angBinR]=distR;


	   if(angBinR>oriCnt || angBinR<0){
	   	printf("\n\n\nERROR BINNING RIGHT\n\n\n");
	   printf("angbilR = %d\n",angBinR);
	}
   }

 double minAllR=10000.0;


		for( int i2 = 0; i2 < oriCnt; i2++ )
		{
			//printf("Array right: %f",arrayR[i2]);
			if(*(arrayR+i2)<minAllR)
				minAllR=*(arrayR+i2);
				if (minAllR <0.1)
				{
					i3 =i2-1;
					i4=i2+1;
					if(i2==0)
						i3=oriCnt-1;
					if(i2==(oriCnt-1))
						i4=0;

					minAllR=(arrayR[i3]>arrayR[i4])?arrayR[i4]:arrayR[i3];
				}
		}


printf("\n");
	squarelenR = sqrt(2.0)*minAllR;

	











//start adds gabriele
	//float sizexL = cvRound(maxPosL.x-minPosL.x);
	//float sizeyL = cvRound(maxPosL.y-minPosL.y);

	//float sizexR = cvRound(maxPosR.x-minPosR.x);
	//float sizeyR = cvRound(maxPosR.y-minPosR.y);

	float sizexL;
	float sizeyL;

	float sizexR;
	float sizeyR;
	if(squarelenL>=squarelenR)
	{
		sizexL = squarelenL;
		sizeyL = squarelenL;

		sizexR = squarelenL;
		sizeyR = squarelenL;
	}
	else
	{
		sizexL = squarelenR;
		sizeyL = squarelenR;

		sizexR = squarelenR;
		sizeyR = squarelenR;
	}
//end adds gabriele

	/*float sizexL = stdvxL;//COMM
	float sizeyL = stdvyL;//COMM
	float sizexR = stdvxR;//COMM
	float sizeyR = stdvyR;//COMM*/

	int upperxL = posxL + cvRound(sizexL/2.0) + 1;
	int upperyL = posyL + cvRound(sizeyL/2.0) + 1;
	if(upperxL > m_width-1) sizexL -= (upperxL - m_width + 1);
	if(upperyL > m_height-1) sizeyL -= (upperyL - m_height + 1);

	int upperxR = posxR + cvRound(sizexR/2.0) + 1;
	int upperyR = posyR + cvRound(sizeyR/2.0) + 1;
	if(upperxR > m_width-1) sizexR -= (upperxR - m_width + 1);
	if(upperyR > m_height-1) sizeyR -= (upperyR - m_height + 1);

	int lowerxL = posxL - cvRound(sizexL/2.0) - 1;
	int loweryL = posyL - cvRound(sizeyL/2.0) - 1;
	if(lowerxL < 0) sizexL -= (lowerxL);
	if(loweryL < 0) sizeyL -= (loweryL);

	int lowerxR = posxR - cvRound(sizexR/2.0) - 1;
	int loweryR = posyR - cvRound(sizeyR/2.0) - 1;
	if(lowerxR < 0) sizexR -= (lowerxR);
	if(loweryR < 0) sizeyR -= (loweryR);


	if(forceMaxSizeL) {
		float temp1L = std::max( (float)m_maxsize, sizexL );
		float temp2L = std::max( (float)m_maxsize, sizeyL );
		m_trackL.winnerSize.width = cvRound(temp1L);
		m_trackL.winnerSize.height = cvRound(temp2L);
	}
	else{
		m_trackL.winnerSize.width = cvRound(sizexL);
		m_trackL.winnerSize.height = cvRound(sizeyL);
	}
	

	if(forceMaxSizeR) {
			float temp1R = std::max( (float)m_maxsize, sizexR );
			float temp2R = std::max( (float)m_maxsize, sizeyR );
			m_trackR.winnerSize.width = cvRound(temp1R);
			m_trackR.winnerSize.height = cvRound(temp2R);
		}
		else{
			m_trackR.winnerSize.width = cvRound(sizexR);
			m_trackR.winnerSize.height = cvRound(sizeyR);
		}

//start adds gabriele
	m_trackL.winnerRect.x = cvRound((float)(m_trackL.winnerPos.x) - (float)(m_trackL.winnerSize.width)/2.0);//minPosL.x;//
	m_trackL.winnerRect.y = cvRound((float)(m_trackL.winnerPos.y) - (float)(m_trackL.winnerSize.height)/2.0);//minPos.y;//
	m_trackL.winnerRect.width = m_trackL.winnerSize.width;//maxPosL.x-minPosL.x;//
	m_trackL.winnerRect.height = m_trackL.winnerSize.height;//maxPosL.y-minPosL.y;//

	m_trackR.winnerRect.x = cvRound((float)(m_trackR.winnerPos.x) - (float)(m_trackR.winnerSize.width)/2.0);//minPosR.x;//
	m_trackR.winnerRect.y = cvRound((float)(m_trackR.winnerPos.y) - (float)(m_trackR.winnerSize.height)/2.0);//minPosR.y;//
	m_trackR.winnerRect.width = m_trackR.winnerSize.width; //maxPosR.x-minPosR.x;//
	m_trackR.winnerRect.height = m_trackR.winnerSize.height;//maxPosR.y-minPosR.y;//
//end adds gabriele


/*	m_trackL.winnerRect.x = cvRound((float)(m_trackL.winnerPos.x) - (float)(m_trackL.winnerSize.width)/2.0);
	m_trackL.winnerRect.y = cvRound((float)(m_trackL.winnerPos.y) - (float)(m_trackL.winnerSize.height)/2.0);
	m_trackL.winnerRect.width = m_trackL.winnerSize.width;
	m_trackL.winnerRect.height = m_trackL.winnerSize.height;

	m_trackR.winnerRect.x = cvRound((float)(m_trackR.winnerPos.x) - (float)(m_trackR.winnerSize.width)/2.0);
	m_trackR.winnerRect.y = cvRound((float)(m_trackR.winnerPos.y) - (float)(m_trackR.winnerSize.height)/2.0);
	m_trackR.winnerRect.width = m_trackR.winnerSize.width;
	m_trackR.winnerRect.height = m_trackR.winnerSize.height;*/


	m_trackL.reliability = 1.0;
	m_trackR.reliability = 1.0;
	
	if(debug) {
		std::cout << getName() << "::checkcondition()\n";
		std::cout << "winnerPos left = [" << m_trackL.winnerPos.x << " " << m_trackL.winnerPos.y << "]\n";
		std::cout << "winnerSize left = [" << m_trackL.winnerSize.width << " " << m_trackL.winnerSize.height << "]\n";
		std::cout << "winnerRect left = [" << m_trackL.winnerRect.x << " " << m_trackL.winnerRect.y << " " << m_trackL.winnerRect.width << " " << m_trackL.winnerRect.height << "]\n";
		std::cout << "imageSize left = [" << m_trackL.imageSize.width << " " << m_trackL.imageSize.height << "]\n";
		std::cout << "reliability left = " << m_trackL.reliability << "\n";

		std::cout << "winnerPos right = [" << m_trackR.winnerPos.x << " " << m_trackR.winnerPos.y << "]\n";
		std::cout << "winnerSize right = [" << m_trackR.winnerSize.width << " " << m_trackR.winnerSize.height << "]\n";
		std::cout << "winnerRect right = [" << m_trackR.winnerRect.x << " " << m_trackR.winnerRect.y << " " << m_trackR.winnerRect.width << " " << m_trackR.winnerRect.height << "]\n";
		std::cout << "imageSize right = [" << m_trackR.imageSize.width << " " << m_trackR.imageSize.height << "]\n";
		std::cout << "reliability right = " << m_trackR.reliability << "\n";

	}

	return true;
	
/*
	// count # of IPs that are green (on the plane of fixation)
	int total = mp_leftInterestPoints->numPoints;
	int i, tempCounter = 0;
	int sum = 0;
	for(i=0;i<total;i++){
		if (mp_classified[i] == 0 || mp_classified[i] == -1){
			sum = sum + 1;
			IP_x[tempCounter] = mp_leftInterestPoints->points[i].x;
			IP_y[tempCounter] = mp_leftInterestPoints->points[i].y;
			tempCounter = tempCounter + 1;
		}
	}
	if(sum > m_noIPthreshold){
//		m_count2 = m_count2 + 1;
//		if(m_count2 > m_ignore){
			// Compute the median of horizontal and vertical positions of interest points which are on the plane of fixation
			ippsFilterMedian_32f(IP_x, median_x, sum, sum);
			ippsFilterMedian_32f(IP_y, median_y, sum, sum);

			int posIndex = cvRound((float)sum/2.0);

			m_winner.winnerPos.x = cvRound(median_x[posIndex]);
			m_winner.winnerPos.y = cvRound(median_y[posIndex]);

			this->computeVariance();
			
			CvPoint point1, point2;
			point1.x = m_winner.winnerPos.x - cvRound(m_winner.winnerSize.width / 2.0);
			point1.y = m_winner.winnerPos.y - cvRound(m_winner.winnerSize.height / 2.0);
			point2.x = m_winner.winnerPos.x + cvRound(m_winner.winnerSize.width / 2.0);
			point2.y = m_winner.winnerPos.y + cvRound(m_winner.winnerSize.height / 2.0);
			if(m_drawResult){
				cvRectangle( mp_disparityimg->ipl, point1, point2, CV_RGB(0,0,0), 1, 8, 0 );
				cvDisparityImageOut.setBuffer(mp_disparityimg);
			}
			else{
				cvRectangle( mp_cvleftrgbimg->ipl, point1, point2, CV_RGB(0,0,0), 1, 8, 0 );
				cvDisparityImageOut.setBuffer(mp_cvleftrgbimg);
			}
			cvDisparityImageOut.out();
			
			m_winner.reliability = 1.0;
			
			// Estimate the size of the object using OpenCV's BoundingRect function (in 3.1?)
			// CvRect cvBoundingRect( CvArr* contour, int update );
			//int seqFlags = CV_SEQ_ELTYPE_POINT;
			//int headerSize = sizeof(CvSeq);
			//int elemSize = sizeof(CvPoint);
			//CvMemStorage* storage = cvCreateMemStorage(0);
			//CvSeq* tempPoints = cvCreateSeq( seqFlags, headerSize, elemSize, storage );
			//CvPoint tempPoint;
			//int j = 0;
			//for(i=0;i<total;i++){
			//	if (classified[i] == 0 || classified[i] == -1){
			//		tempPoint.x = cvRound(IP_x[j]);
			//		tempPoint.y = cvRound(IP_y[j]);
			//		cvSeqPush( tempPoints, &tempPoint );
			//		j = j + 1;
			//	}
			//}
			//CvRect tempRect = cvBoundingRect( tempPoints, 0 );
			//m_ptr_winnerRect->x = tempRect.x;
			//m_ptr_winnerRect->y = tempRect.y;
			//m_ptr_winnerRect->width = tempRect.width;
			//m_ptr_winnerRect->height = tempRect.height;
			//m_ptr_winnerSize->width = tempRect.width;
			//m_ptr_winnerSize->height = tempRect.height;
			//std::cout << "DetectSavedObjected object rect is ( " << tempRect.x << ", " << tempRect.y << ", " << tempRect.width << ", " << tempRect.height << " )...\n";
			//if(m_drawOutput == true) {
			//	if (m_mode == 0) cvSetReal2D( m_ptr_winnerImage, m_ptr_winnerPoint1->y, m_ptr_winnerPoint1->x, 1.0 );
			//	else if (m_mode == 1) cvSetReal2D( m_ptr_winnerImage, m_ptr_winnerPoint2->y, m_ptr_winnerPoint2->x, 1.0 );
			//}
			//if(m_ptr_RHI != NULL) m_ptr_RHI->raiseEyebrow(80);
			//m_run = false;

			m_winner.winnerRect.x = cvRound(m_winner.winnerPos.x - m_winner.winnerSize.width/2.0);
			m_winner.winnerRect.y = cvRound(m_winner.winnerPos.y - m_winner.winnerSize.height/2.0);
			m_winner.winnerRect.width = m_winner.winnerSize.width;
			m_winner.winnerRect.height = m_winner.winnerSize.height;

			if(debug) {
				std::cout << getName() << "::winner = \n";
				std::cout << "winnerPos = ( " << m_winner.winnerPos.x << ", " << m_winner.winnerPos.y << ")\n";
				std::cout << "winnerSize = ( " << m_winner.winnerSize.width << ", " << m_winner.winnerSize.height << ")\n";
				std::cout << "winnerRect = ( " << m_winner.winnerRect.x << ", " << m_winner.winnerRect.y << ", " << m_winner.winnerRect.width << ", " << m_winner.winnerRect.height << ")\n";
				std::cout << "imageSize = ( " << m_winner.imageSize.width << ", " << m_winner.imageSize.height << ")\n";
				std::cout << "reliability = " << m_winner.reliability << "\n";
			}

			m_detect = true;
//		}
	}

	if(m_winner.reliability == 1.0) {
		winnerOut.setBuffer(&m_winner);
		winnerOut.out();
	}
	else{
//		m_winner.reliability = 0.0;
//		winnerOut.setBuffer(&m_winner);
		winnerOut.setBuffer(NULL);
		winnerOut.out();
	}
*/
}

/*
void DetectSavedObject::calculatesizeL()
{


	unsigned int regionwidthleft = mp_leftregions->getWidth();
	float* regiondataleft = mp_leftregions->getData();



		int oriCnt=4;
		double centerx = (double) m_trackL.winnerPos.x;
		double centery= (double) m_trackL.winnerPos.y;

		double* array=(double *)malloc(oriCnt*sizeof(double));
		for( int i1 = 0; i1 < oriCnt; i1++ )
		{
			*(array+i1)=0.0;
		}

		std::vector<unsigned int>::iterator enditerL = mvec_indexesonleft.end();
		for(std::vector<unsigned int>::iterator iterL = mvec_indexesonleft.begin(); iterL != enditerL; ++iterL) {
			unsigned int indexL = *iterL;
			float xVal = regiondataleft[indexL*regionwidthleft + 0];
			float yVal = regiondataleft[indexL*regionwidthleft + 1];

			//cvCircle(img,cvPoint(xVal,yVal),2,CV_RGB(255,255,255), CV_FILLED );

		   double dist = sqrt((xVal-centerx)*(xVal-centerx)+(yVal-centery)*(yVal-centery));
		   double angle = atan(fabs(yVal-centery)/fabs(xVal-centerx))*180.0/3.14159265;
		   int bin=(int)(angle/(90.0/(oriCnt/4.0)));
		   int angBin;
		   if(yVal-centery<0.0)
		   {
			   if(xVal-centerx<0.0)
			   {
				   angBin=(int)(bin+oriCnt/2);
			   }else
			   {
				   angBin=(int)(bin+oriCnt/4*3);
			   }

		   }
		   else
		   {
			   if(xVal-centerx<0.0)
			   {
					angBin=(int)(bin+oriCnt/4);
			   }
			   else
			   {
				   angBin=(int)(bin);
			   }
		   }
		  // printf("%d ",angBin);
		   if(dist>*(array+angBin))
			   *(array+angBin)=dist;


	   }


	   float minAll=10000.0;
		for( int i2 = 0; i2 < oriCnt; i2++ )
		{
			//printf("%f ",*(array+i2));
			if(*(array+i2)<minAll)
				minAll=*(array+i2);
		}
		//printf("%f ",minAll);
		//cvCircle(img,cvPoint(centerx,centery),minAll,CV_RGB(255,0,0) );

	squarelenL = sqrt(2.0)*minAll;

	free(array);
	firstL=false;

}

void DetectSavedObject::calculatesizeR()
{

	unsigned int regionwidthright = mp_rightregions->getWidth();
	float* regiondataright = mp_rightregions->getData();



		int oriCnt=4;
		double centerx = (double) m_trackR.winnerPos.x;
		double centery= (double) m_trackR.winnerPos.y;

		double* array=(double *)malloc(oriCnt*sizeof(double));
		for( int i1 = 0; i1 < oriCnt; i1++ )
		{
			*(array+i1)=0.0;
		}

		std::vector<unsigned int>::iterator enditerR = mvec_indexesonright.end();
		for(std::vector<unsigned int>::iterator iterR = mvec_indexesonright.begin(); iterR != enditerR; ++iterR) {
			unsigned int indexR = *iterR;
			float xValR = regiondataright[indexR*regionwidthright + 0];
			float yValR = regiondataright[indexR*regionwidthright + 1];

			//cvCircle(img,cvPoint(xVal,yVal),2,CV_RGB(255,255,255), CV_FILLED );

		   double dist = sqrt((xValR-centerx)*(xValR-centerx)+(yValR-centery)*(yValR-centery));
		   double angle = atan(fabs(yValR-centery)/fabs(xValR-centerx))*180.0/3.14159265;
		   int bin=(int)(angle/(90.0/(oriCnt/4.0)));
		   int angBin;
		   if(yValR-centery<0.0)
		   {
			   if(xValR-centerx<0.0)
			   {
				   angBin=(int)(bin+oriCnt/2);
			   }else
			   {
				   angBin=(int)(bin+oriCnt/4*3);
			   }

		   }
		   else
		   {
			   if(xValR-centerx<0.0)
			   {
					angBin=(int)(bin+oriCnt/4);
			   }
			   else
			   {
				   angBin=(int)(bin);
			   }
		   }
		  // printf("%d ",angBin);
		   if(dist>*(array+angBin))
			   *(array+angBin)=dist;


	   }


	   float minAll=10000.0;
		for( int i2 = 0; i2 < oriCnt; i2++ )
		{
			//printf("%f ",*(array+i2));
			if(*(array+i2)<minAll)
				minAll=*(array+i2);
		}
		//printf("%f ",minAll);
		//cvCircle(img,cvPoint(centerx,centery),minAll,CV_RGB(255,0,0) );
	squarelenR = sqrt(2.0)*minAll;
	free(array);
	firstR=false;

}
*/

/*
void *DetectSavedObject::computeleft(void* ptr) {
	DetectSavedObject *obj = reinterpret_cast<DetectSavedObject*>(ptr);
	for(;;) {
		// Blocks the calling thread until the specified condition is signalled (pthread_cond_signal() or pthread_cond_broadcast())
		pthread_mutex_lock( &(obj->mutex_leftwaitlock) );
		pthread_cond_wait( &(obj->condition_startthread), &(obj->mutex_leftwaitlock) );
		pthread_mutex_lock( &(obj->mutex_threadleft) );

		// If condition is met, quit the thread
		if(obj->endThread == true) {
//			if(obj->debug) std::cout << "Ending computeleft() thread\n";
			pthread_mutex_unlock( &(obj->mutex_threadleft) );
			pthread_mutex_unlock( &(obj->mutex_leftwaitlock) );
			pthread_exit(NULL);	
			return NULL;
		}

		// Extract interest points from the gray image
		obj->mp_CornerFinderLeft->cvImageIn.setBuffer(obj->mp_cvleftgrayimg);
		obj->mp_CornerFinderLeft->execute();
		obj->mp_leftInterestPoints = obj->mp_CornerFinderLeft->pointsOut.getBuffer();
		if(obj->m_drawIP == true){
			obj->mp_CornerDrawerLeft->cvImageIn.in(obj->mp_cvleftrgbimg);	
			obj->mp_CornerDrawerLeft->pointsIn.in(obj->mp_leftInterestPoints);	
			obj->mp_CornerDrawerLeft->execute();	
			CVImage* tempLeft = obj->mp_CornerDrawerLeft->cvImageOut.getBuffer();	
			obj->cvLeftImageOut.setBuffer(tempLeft);
			obj->cvLeftImageOut.out();
		}
		// Extract gabor responses
		obj->mp_GaborLeft->cvImageIn.in(obj->mp_cvleftgrayimg);
		obj->mp_GaborLeft->pointsIn.in(obj->mp_leftInterestPoints);
		obj->mp_GaborLeft->execute();
		obj->mp_leftGaborResponses = obj->mp_GaborLeft->responseOut.getBuffer();

		pthread_mutex_unlock( &(obj->mutex_threadleft) );
		pthread_mutex_unlock( &(obj->mutex_leftwaitlock) );

	}
	return NULL;
}

void *DetectSavedObject::computeright(void* ptr) {
	DetectSavedObject *obj = reinterpret_cast<DetectSavedObject*>(ptr);
	for(;;) {
		// Blocks the calling thread until the specified condition is signalled (pthread_cond_signal() or pthread_cond_broadcast())
		pthread_mutex_lock( &(obj->mutex_rightwaitlock) );
		pthread_cond_wait( &(obj->condition_startthread), &(obj->mutex_rightwaitlock) );
		pthread_mutex_lock( &(obj->mutex_threadright) );

		// If condition is met, quit the thread
		if(obj->endThread == true) {
//			if(obj->debug) std::cout << "Ending computeright() thread\n";
			pthread_mutex_unlock( &(obj->mutex_threadright) );
			pthread_mutex_unlock( &(obj->mutex_rightwaitlock) );
			pthread_exit(NULL);	
			return NULL;
		}

		// Extract interest points from the gray image
		obj->mp_CornerFinderRight->cvImageIn.setBuffer(obj->mp_cvrightgrayimg);
		obj->mp_CornerFinderRight->execute();
		obj->mp_rightInterestPoints = obj->mp_CornerFinderRight->pointsOut.getBuffer();
		if(obj->m_drawIP == true){
			obj->mp_CornerDrawerRight->cvImageIn.in(obj->mp_cvrightrgbimg);	
			obj->mp_CornerDrawerRight->pointsIn.in(obj->mp_rightInterestPoints);	
			obj->mp_CornerDrawerRight->execute();	
			CVImage* tempRight = obj->mp_CornerDrawerRight->cvImageOut.getBuffer();	
			obj->cvRightImageOut.setBuffer(tempRight);
			obj->cvRightImageOut.out();
		}
		// Extract gabor responses
		obj->mp_GaborRight->cvImageIn.in(obj->mp_cvrightgrayimg);
		obj->mp_GaborRight->pointsIn.in(obj->mp_rightInterestPoints);
		obj->mp_GaborRight->execute();
		obj->mp_rightGaborResponses = obj->mp_GaborRight->responseOut.getBuffer();

		pthread_mutex_unlock( &(obj->mutex_threadright) );
		pthread_mutex_unlock( &(obj->mutex_rightwaitlock) );
	}

	return NULL;
}



void DetectSavedObject::drawDisparity() {
	// Draw interest points according to their horizontal disparity
	mp_DisparityDrawer->cvImageIn.setBuffer(mp_cvleftrgbimg);
	mp_DisparityDrawer->pointsIn.setBuffer(mp_leftInterestPoints);
	mp_DisparityDrawer->classifiedIn.setBuffer(mp_classified);
	mp_DisparityDrawer->dispXIn.setBuffer(mp_disparityx);
	mp_DisparityDrawer->dispYIn.setBuffer(mp_disparityy);
	mp_DisparityDrawer->execute();	
	mp_disparityimg = mp_DisparityDrawer->cvImageOut.getBuffer();	
	cvDisparityImageOut.setBuffer(mp_disparityimg);
	cvDisparityImageOut.out();
}


void DetectSavedObject::computeVariance() {

	// Uses median
	CvPoint* median = &(m_winner.winnerPos);
	CvSize* var = &(m_winner.winnerSize);

	float varX = 0.0, varY = 0.0, temp;
	int numPoints = mp_leftInterestPoints->numPoints;
	int counter = 0;

	for(int i = 0;i<numPoints;i++){
		if(mp_classified[i] == -1 || mp_classified[i] == 0){
			temp = mp_leftInterestPoints->points[i].x - median->x;
			varX = varX + temp*temp;
			temp = mp_leftInterestPoints->points[i].y - median->y;
			varY = varY + temp*temp;
			counter = counter + 1;
		}
	}
	varX = varX / (float)(counter);
	varY = varY / (float)(counter);

	float scale = 0.05;
	varX = varX * scale;
	varY = varY * scale;

	var->width = cvRound(varX);
	var->height = cvRound(varY);
	
	if(var->width < 0 || var->width > (mp_cvleftgrayimg->width)) var->width = minSize;
	if(var->height < 0 || var->height > (mp_cvleftgrayimg->height)) var->height = minSize;

	if(forceMinSize == true) {
		var->width = minSize;
		var->height = minSize;
	}

//	std::cout << getName() << "::comptueVariance()::var->width = " << var->width << "\n";
//	std::cout << getName() << "::comptueVariance()::var->height = " << var->height << "\n";
}

void DetectSavedObject::computeleftnothread() {
	// Extract interest points from the gray image
	mp_CornerFinderLeft->cvImageIn.setBuffer(mp_cvleftgrayimg);
	mp_CornerFinderLeft->execute();
	mp_leftInterestPoints = mp_CornerFinderLeft->pointsOut.getBuffer();
	if(m_drawIP == true){
		mp_CornerDrawerLeft->cvImageIn.in(mp_cvleftrgbimg);	
		mp_CornerDrawerLeft->pointsIn.in(mp_leftInterestPoints);	
		mp_CornerDrawerLeft->execute();	
		CVImage* tempLeft = mp_CornerDrawerLeft->cvImageOut.getBuffer();	
		cvLeftImageOut.setBuffer(tempLeft);
		cvLeftImageOut.out();
	}
	// Extract gabor responses
	mp_GaborLeft->cvImageIn.in(mp_cvleftgrayimg);
	mp_GaborLeft->pointsIn.in(mp_leftInterestPoints);
	mp_GaborLeft->execute();
	mp_leftGaborResponses = mp_GaborLeft->responseOut.getBuffer();
}

void DetectSavedObject::computerightnothread() {
	// Extract interest points from the gray image
	mp_CornerFinderRight->cvImageIn.setBuffer(mp_cvrightgrayimg);
	mp_CornerFinderRight->execute();
	mp_rightInterestPoints = mp_CornerFinderRight->pointsOut.getBuffer();
	if(m_drawIP == true){
		mp_CornerDrawerRight->cvImageIn.in(mp_cvrightrgbimg);	
		mp_CornerDrawerRight->pointsIn.in(mp_rightInterestPoints);	
		mp_CornerDrawerRight->execute();	
		CVImage* tempRight = mp_CornerDrawerRight->cvImageOut.getBuffer();	
		cvRightImageOut.setBuffer(tempRight);
		cvRightImageOut.out();
	}
	// Extract gabor responses
	mp_GaborRight->cvImageIn.in(mp_cvrightgrayimg);
	mp_GaborRight->pointsIn.in(mp_rightInterestPoints);
	mp_GaborRight->execute();
	mp_rightGaborResponses = mp_GaborRight->responseOut.getBuffer();
}
*/
