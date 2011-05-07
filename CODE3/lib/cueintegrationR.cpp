#include "cueintegrationR.h"
#include <math.h> 



using namespace yarp::os;
using namespace yarp::sig;


CueIntegrationR::CueIntegrationR(std::string name) : Module(name) {
	std::cout << "\nConstructing " << getName() << "...";

/*
	targetPortRight.open("/tutorial/Right/target/out");
*/

	distxR = 160;
	distyR = 120;

	m_cuecount = 0;
	m_threshold = 0.2;
	
	mdp_Cues = NULL;
	mp_initrel = NULL;
	mp_rel = NULL;
	mp_qual = NULL;
	
	//m_rgbused = false;
	//m_hsvused = false;

	mp_cvrimg = NULL;
	mp_cvgimg = NULL;
	mp_cvbimg = NULL;
	mp_cvhsvimg = NULL;
	mp_cvhimg = NULL;
	mp_cvsimg = NULL;
	mp_cvvimg = NULL;
	mp_cvoutputimg = NULL;
	
	m_forceMinSize = false;
	m_forceSquare = false;
	m_minSize.width = 100;
	m_minSize.height = 100;

	m_tauadapt = 0.1;
	m_taucueadapt = 0.05;
	
	m_init = false;
	debug = false;

#ifdef USE_THREADS
	m_endthreads = false;
//	mp_threads = NULL;
//	mp_mutex_waitlock = NULL;
#endif

	std::cout<<"finished.";
}

CueIntegrationR::~CueIntegrationR() {
	std::cout<<"\nDestructing " << getName() << "started...";

/*
	targetPortRight.interrupt();
	targetPortRight.close();
*/
	mdp_Cues = NULL;
	mp_initrel = NULL;
	mp_rel = NULL;
	mp_qual = NULL;

	if(mdp_Cues) delete [] mdp_Cues;
	if(mp_initrel) delete [] mp_initrel;
	if(mp_rel) delete [] mp_rel;
	if(mp_qual) delete [] mp_qual;
	
	if(mp_cvrimg) delete mp_cvrimg;
	if(mp_cvgimg) delete mp_cvgimg;
	if(mp_cvbimg) delete mp_cvbimg;
	if(mp_cvhsvimg) delete mp_cvhsvimg;
	if(mp_cvhimg) delete mp_cvhimg;
	if(mp_cvsimg) delete mp_cvsimg;
	if(mp_cvvimg) delete mp_cvvimg;
	if(mp_cvoutputimg) delete mp_cvoutputimg;

#ifdef USE_THREADS
	m_endthreads = true;
	pthread_cond_broadcast( &m_condition_threadsstart);
	pthread_mutex_lock( &m_mutex_threadsended );
	pthread_cond_wait( &m_condition_threadsended, &m_mutex_threadsended );
	pthread_mutex_unlock( &m_mutex_threadsended );
	
	pthread_barrier_destroy(&m_barrier);
	pthread_cond_destroy(&m_condition_threadsstart);
	pthread_cond_destroy(&m_condition_threadsended);
	pthread_mutex_destroy(&m_mutex_threadsended);

	pthread_mutex_destroy(&(m_mutex_waitlock1) );
	pthread_mutex_destroy(&(m_mutex_waitlock2) );
	pthread_mutex_destroy(&(m_mutex_waitlock3) );
	pthread_mutex_destroy(&(m_mutex_waitlock4) );
	pthread_mutex_destroy(&(m_mutex_waitlock5) );
	pthread_mutex_destroy(&(m_mutex_waitlock6) );
	pthread_mutex_destroy(&(m_mutex_waitlock7) );
	pthread_mutex_destroy(&(m_mutex_waitlock8) );
/*
	if(mp_mutex_waitlock) {
		for(unsigned int i = 0;i<m_cuecount;++i) {
			pthread_mutex_destroy(&(mp_mutex_waitlock[i]) );
		}
		delete [] mp_mutex_waitlock;
	}
	if(mp_threads) delete [] mp_threads;
*/
#endif

	std::cout<<"Destructing " << getName() <<"...finished.\n";
}

void CueIntegrationR::connect(){

	PointCueOutPort.open("/cueintegration/right/out");
	Network::connect("/cueintegration/right/out","/motion/right/in");

}

void CueIntegrationR::clean(){
	std::cout<<"\nDisconnetting /cueintegration/right/out ...";
	PointCueOutPort.interrupt();
	PointCueOutPort.close();
	std::cout<<"finished.\n";

}

void CueIntegrationR::setthres(float val) {
	m_threshold = val;
/*
	for (int i=0; i<cueCount; i++) {
		cue[i]->setThreshold(m_threshold);
	}
*/
}

void CueIntegrationR::addCue(Cue* newcue, float reliability) {

	if(!newcue) { std::cerr << getName() << "::addCue()::ERROR::newcue is NULL!\n"; return; } 

	mvec_cues.push_back(newcue);
	mvec_rel.push_back(reliability);

/*
	if (cueCount<MAX_CUES) {
		cueCount++;
		cue[cueCount-1] = newCue;
		this->reliability[cueCount-1] = new float(reliability);
		this->defaultReliability[cueCount-1] = new float(reliability);
		this->quality[cueCount-1] = new float(0);
		initialized_cues[cueCount-1] = false;
		return cueCount-1;
	}
*/
}

void CueIntegrationR::initialize() {

	TrackData* track = trackIn.getBuffer();
	if(!track) { std::cerr << getName() << "::ERROR::initialize()::trackIn is NULL!\n"; return; }

	if(debug) {
		std::cout << getName() << "::initialize()::track\n";
		std::cout << "trackPos = (" << track->winnerPos.x << ", " << track->winnerPos.y << ")\n";
		std::cout << "trackSize = (" << track->winnerSize.width << ", " << track->winnerSize.height << ")\n";
		std::cout << "trackRect = (" << track->winnerRect.x << ", " << track->winnerRect.y << ", " << track->winnerRect.width << ", " << track->winnerRect.height << ")\n";
		std::cout << "imageSize = (" << track->imageSize.width << ", " << track->imageSize.height << ")\n";
		std::cout << "rel = " << track->reliability << "\n";
	}

	if(track->reliability < m_threshold) { return; }

	if(!mdp_Cues) {
		m_cuecount = mvec_cues.size();
		mdp_Cues = new Cue* [m_cuecount];
		mp_initrel = new float [m_cuecount];
		mp_rel = new float [m_cuecount];
		mp_qual = new float [m_cuecount];
	}

	for(unsigned int i = 0;i<m_cuecount;++i) {
		mdp_Cues[i] = mvec_cues[i];
		mp_initrel[i] = mvec_rel[i];
		mp_rel[i] = mp_initrel[i];
		//mp_qual[i] = mp_rel[i];
		mp_qual[i] = 0.0;
		mdp_Cues[i]->setthres(m_threshold);
	}

#ifdef USE_THREADS
	//if(!mp_threads) mp_threads = new pthread_t [m_cuecount];
/*
	if(!mp_mutex_waitlock) mp_mutex_waitlock = new pthread_mutex_t [m_cuecount];
	for(unsigned int i = 0;i<m_cuecount;++i) {
		pthread_mutex_init(&mp_mutex_waitlock[i], NULL);
	}
*/
	pthread_mutex_init(&m_mutex_waitlock1, NULL);
	pthread_mutex_init(&m_mutex_waitlock2, NULL);
	pthread_mutex_init(&m_mutex_waitlock3, NULL);
	pthread_mutex_init(&m_mutex_waitlock4, NULL);
	pthread_mutex_init(&m_mutex_waitlock5, NULL);
	pthread_mutex_init(&m_mutex_waitlock6, NULL);
	pthread_mutex_init(&m_mutex_waitlock7, NULL);
	pthread_mutex_init(&m_mutex_waitlock8, NULL);

	pthread_barrier_init(&m_barrier, NULL, m_cuecount);
	pthread_cond_init(&m_condition_threadsstart, NULL);
	pthread_cond_init(&m_condition_threadsended, NULL);
	pthread_mutex_init(&m_mutex_threadsended, NULL);
		
//	if(!mp_threads) {
//		mp_threads = new pthread_t [m_cuecount];
		int  iret;
		if(debug) std::cout << getName() << "::initialize()::m_cuecount = " << m_cuecount << "\n";
		switch(m_cuecount) {
			case 8:
				//iret = pthread_create( &(mp_threads[7]), NULL, compute8, this);
				iret = pthread_create( &(m_thread8), NULL, compute8, this);
			case 7:
				//iret = pthread_create( &(mp_threads[6]), NULL, compute7, this);
				iret = pthread_create( &(m_thread7), NULL, compute7, this);
			case 6:
				//iret = pthread_create( &(mp_threads[5]), NULL, compute6, this);
				iret = pthread_create( &(m_thread6), NULL, compute6, this);
			case 5:
				//iret = pthread_create( &(mp_threads[4]), NULL, compute5, this);
				iret = pthread_create( &(m_thread5), NULL, compute5, this);
			case 4:
				//iret = pthread_create( &(mp_threads[3]), NULL, compute4, this);
				iret = pthread_create( &(m_thread4), NULL, compute4, this);
			case 3:
				//iret = pthread_create( &(mp_threads[2]), NULL, compute3, this);
				iret = pthread_create( &(m_thread3), NULL, compute3, this);
			case 2:
				//if(debug) std::cout << getName() << "::execute()::case 2\n";
				//iret = pthread_create( &(mp_threads[1]), NULL, compute2, this);
				iret = pthread_create( &(m_thread2), NULL, compute2, this);
			case 1:
				//if(debug) std::cout << getName() << "::execute()::case 1\n";
				//iret = pthread_create( &(mp_threads[0]), NULL, compute1, this);
				iret = pthread_create( &(m_thread1), NULL, compute1, this);
				break;
			default:
				break;
		}
//	}

#endif	

	obtainInput();

	int upperx = track->winnerPos.x + cvCeil(track->winnerSize.width/2.0);
	int lowerx = track->winnerPos.x - cvFloor(track->winnerSize.width/2.0);
	int uppery = track->winnerPos.y + cvCeil(track->winnerSize.height/2.0);
	int lowery = track->winnerPos.y - cvFloor(track->winnerSize.height/2.0);

	if(lowerx < 0 || lowery < 0 || upperx >= track->imageSize.width || uppery >= track->imageSize.height) {
		std::cerr << getName() << "::initialize()::ERROR::Obj size too large!\n";
		std::cerr << getName() << "::initialize()::lowerx = " << lowerx << ", upperx = " << upperx << ", lowery = " << lowery << ", uppery = " << uppery << "\n";
		return; 
	}

	float xratio, yratio;
	xratio = (float)track->imageSize.width / (float)mp_cvgrayimg->width;
	yratio = (float)track->imageSize.height / (float)mp_cvgrayimg->height;

	m_track.winnerPos.x = cvRound((float)(track->winnerPos.x) * xratio);
	m_track.winnerPos.y = cvRound((float)(track->winnerPos.y) * xratio);
	m_track.reliability = track->reliability;
	if(m_forceMinSize) {
		if(m_forceSquare) {
			int minsize = m_minSize.width > m_minSize.height ? m_minSize.width : m_minSize.height;
			int temp1 = cvRound((float)(track->winnerSize.width) * xratio);
			int temp2 = cvRound((float)(track->winnerSize.height) * yratio);
			int tracksize = temp1 > temp2 ? temp1 : temp2;
			int size = minsize > tracksize ? minsize : tracksize;
			m_track.winnerSize.width = size;
			m_track.winnerSize.height = size;
		}
		else{
			int temp1 = cvRound((float)(track->winnerSize.width) * xratio);
			int temp2 = cvRound((float)(track->winnerSize.height) * yratio);
			int width = m_minSize.width > temp1 ? m_minSize.width : temp1;
			int height = m_minSize.height > temp2 ? m_minSize.height : temp2;
			m_track.winnerSize.width = width;
			m_track.winnerSize.height = height;

		}
		m_track.winnerRect.x = m_track.winnerPos.x - cvRound(m_track.winnerSize.width/2.0);
		m_track.winnerRect.y = m_track.winnerPos.y - cvRound(m_track.winnerSize.height/2.0);
		m_track.winnerRect.width = m_track.winnerSize.width;
		m_track.winnerRect.height = m_track.winnerSize.height;
	}

	//start adds gabriele
	else{

		m_track.winnerSize.width = cvRound( (float)(track->winnerSize.width) * xratio );
		m_track.winnerSize.height = cvRound( (float)(track->winnerSize.height) * yratio );
		m_track.winnerRect.x = cvRound( (float)(track->winnerRect.x)* xratio);
		m_track.winnerRect.y = cvRound( (float)(track->winnerRect.y)* yratio);
		m_track.winnerRect.width = cvRound( (float)(track->winnerRect.width)* xratio);
		m_track.winnerRect.height = cvRound( (float)(track->winnerRect.height)* yratio);
	}
	//end adds gabriele

	m_track.imageSize.width = mp_cvgrayimg->width;
	m_track.imageSize.height = mp_cvgrayimg->height;

	m_prevtrack.winnerPos = m_track.winnerPos;
	m_prevtrack.winnerSize = m_track.winnerSize;
	m_prevtrack.winnerRect = m_track.winnerRect;
	m_prevtrack.imageSize = m_track.imageSize;
	m_prevtrack.reliability = m_track.reliability;

	if(debug){
		std::cout << getName() << "::initialize()::m_track\n";
		std::cout << "winnerPos = ( " << m_track.winnerPos.x << ", " << m_track.winnerPos.y << ")\n";
		std::cout << "winnerSize = ( " << m_track.winnerSize.width << ", " << m_track.winnerSize.height << ")\n";
		std::cout << "winnerRect = ( " << m_track.winnerRect.x << ", " << m_track.winnerRect.y << ", " << m_track.winnerRect.width << ", " << m_track.winnerRect.height << ")\n";
		std::cout << "reliability = " << m_track.reliability << "\n";
		std::cout << "imageSize = ( " << m_track.imageSize.width << ", " << m_track.imageSize.height << ")\n";
	}

    float relsum = 0.0;
	for (unsigned int i=0; i<m_cuecount; ++i){
		relsum += mp_rel[i];
	}
	if(relsum != 0.0) {
		for (unsigned int i=0; i<m_cuecount; ++i){
			mp_rel[i] /= relsum;
		}
	}

	if(debug) std::cout << getName() << "::initialize()::m_track.reliability = " << m_track.reliability << "\n";

	for(unsigned int i=0; i<m_cuecount; ++i) {
		if(debug) std::cout << getName() << "::initialize()::i = " << i << ", initializing " << mdp_Cues[i]->getName() << "\n";
		if(debug) std::cout << getName() << "::initialize()::m_track.reliability = " << m_track.reliability << "\n";
		mdp_Cues[i]->setthres(m_threshold);
		mdp_Cues[i]->trackIn.setBuffer(&m_track);
		mdp_Cues[i]->initialize();
		//initialized_cues[i] = true;
		//*(reliability[i]) = *(defaultReliability[i]);
	}

	if(debug) std::cout << getName() << "::initialize()::complete\n";

	m_init = true;

/*
	if(debug == true) std::cout << getName() << "::initialize()::Start\n";

	int i;

	cvSetZero(outputImage->ipl);
	cvSaliencyImageOut.setBuffer(outputImage);

	if(m_forceMinSize == true){
		if(winner->winnerSize.width < m_minSize){
			winner->winnerSize.width = m_minSize;
		}
		if(winner->winnerSize.height < m_minSize){
			winner->winnerSize.height = m_minSize;
		}
	}

//	if(m_forceSquare == true) {
//		int temp = min( winner->winnerSize.width, winner->winnerSize.height );
//		winner->winnerSize.width = temp;
//		winner->winnerSize.height = temp;
//	}

//	std::cout << getName() << "::initialize()::initObjData->winnerSize = (" << initObjData->winnerSize.width << ", " << initObjData->winnerSize.height << "\n";

	if(winner->imageSize.width == m_width && winner->imageSize.height == m_height){
		m_track.imageSize.width = m_width;
		m_track.imageSize.height = m_height;
		
		m_track.winnerPos.x = winner->winnerPos.x;
		m_track.winnerPos.y = winner->winnerPos.y;
		
		m_track.winnerSize.width = winner->winnerSize.width;
		m_track.winnerSize.height = winner->winnerSize.height;

		m_track.winnerRect.x = winner->winnerRect.x;
		m_track.winnerRect.y = winner->winnerRect.y;
		m_track.winnerRect.width = winner->winnerRect.width;
		m_track.winnerRect.height = winner->winnerRect.height;
	}
	else{
		float xratio = (float)(m_width) / (float)(winner->imageSize.width);
		float yratio = (float)(m_height) / (float)(winner->imageSize.height);

		m_track.imageSize.width = m_width;
		m_track.imageSize.height = m_height;
		
		m_track.winnerPos.x = cvRound( (float)(winner->winnerPos.x) * xratio );
		m_track.winnerPos.y = cvRound( (float)(winner->winnerPos.y) * yratio );
		
		m_track.winnerSize.width = cvRound( (float)(winner->winnerSize.width) * xratio );
		m_track.winnerSize.height = cvRound( (float)(winner->winnerSize.height) * yratio );

		m_track.winnerRect.x = cvRound( (float)(winner->winnerRect.x) * xratio );
		m_track.winnerRect.y = cvRound( (float)(winner->winnerRect.y) * yratio );
		m_track.winnerRect.width = cvRound( (float)(winner->winnerRect.width) * xratio );
		m_track.winnerRect.height = cvRound( (float)(winner->winnerRect.height) * yratio );
	}
	m_track.reliability = winner->reliability;

	if(debug == true){
		std::cout << "winnerPos = ( " << m_track.winnerPos.x << ", " << m_track.winnerPos.y << ")\n";
		std::cout << "winnerSize = ( " << m_track.winnerSize.width << ", " << m_track.winnerSize.height << ")\n";
		std::cout << "winnerRect = ( " << m_track.winnerRect.x << ", " << m_track.winnerRect.y << ", " << m_track.winnerRect.width << ", " << m_track.winnerRect.height << ")\n";
		std::cout << "reliability = " << m_track.reliability << "\n";
		std::cout << "imageSize = ( " << m_track.imageSize.width << ", " << m_track.imageSize.height << ")\n";
	}

	m_prevtrack.imageSize = m_track.imageSize;
	m_prevtrack.winnerPos = m_track.winnerPos;
	m_prevtrack.winnerSize = m_track.winnerSize;
	m_prevtrack.winnerRect = m_track.winnerRect;
	m_prevtrack.reliability = m_track.reliability;

	// This function will initialize all cues
	// with object pos, and size
    float reliabilitySum = 0;
	for (i=0; i<cueCount; i++){
		reliabilitySum += *reliability[i];
	}
    float defaultQualitySum = 0;
	for (i=0; i<cueCount; i++) {
		*reliability[i] = *reliability[i] / reliabilitySum;
		*defaultReliability[i] = *defaultReliability[i] / reliabilitySum;

		double temprel = *(reliability[i]) /= reliabilitySum;
		defaultQuality[i] = new float(temprel);
		defaultQualitySum += temprel;
		*quality[i] = temprel;
	}

	for (i=0; i<cueCount; i++) {
		cue[i]->winnerIn.setBuffer(&m_track);
		cue[i]->initialize();
		initialized_cues[i] = true;
		*(reliability[i]) = *(defaultReliability[i]);
		cue[i]->setThreshold(m_threshold);
	}
	initialized = true;
	m_run = true;
	
//	IplImage* templateImage = cueColor->templateImageOut.getBuffer();
//	templateImageOut.setBuffer(templateImage);
//	templateImageOut.out();

	if(debug == true) std::cout << getName() << "::initialize()::Finished\n";
*/
}

void CueIntegrationR::obtainInput() {
	mp_cvgrayimg = cvGrayImageIn.getBuffer();
	if(!mp_cvgrayimg){ std::cerr << getName() << "::initialize()::ERROR::cvGrayImageIn is NULL!...\n"; return; }	

	mp_cvrgbimg = cvImageIn.getBuffer();
	if(!mp_cvrgbimg){ std::cerr << getName() << "::initialize()::ERROR::cvImageIn is NULL!...\n"; return; }	

	if(!mp_cvoutputimg) {
		m_width = mp_cvrgbimg->width;
		m_height = mp_cvrgbimg->height;
		mp_cvoutputimg = new CVImage( cvSize(m_width, m_height), CV_32FC1, 0);
//		mp_cvrimg = new CVImage( cvSize(m_width, m_height), CV_8UC1, 0);
//		mp_cvgimg = new CVImage( cvSize(m_width, m_height), CV_8UC1, 0);
//		mp_cvbimg = new CVImage( cvSize(m_width, m_height), CV_8UC1, 0);
//		mp_cvhsvimg = new CVImage( cvSize(m_width, m_height), CV_8UC3, 0);
//		mp_cvhimg = new CVImage( cvSize(m_width, m_height), CV_8UC1, 0);
//		mp_cvsimg = new CVImage( cvSize(m_width, m_height), CV_8UC1, 0);
//		mp_cvvimg = new CVImage( cvSize(m_width, m_height), CV_8UC1, 0);

		cvSalImageOut.setBuffer(mp_cvoutputimg);

		for(unsigned int i=0; i<m_cuecount; ++i) {
			mdp_Cues[i]->cvImageIn.setBuffer(mp_cvrgbimg);
			mdp_Cues[i]->cvGrayImageIn.setBuffer(mp_cvgrayimg);
			//cue[i]->cvSalImageIn.setBuffer(mp_cvoutputimg);
/*
			cue[i]->cvHSVImageIn.setBuffer(hsvImage);
			cue[i]->cvHImageIn.setBuffer(hImage);
			cue[i]->cvSImageIn.setBuffer(sImage);
			cue[i]->cvVImageIn.setBuffer(vImage);
			//cue[i]->rgbImageIn.setBuffer(image);
			cue[i]->cvRImageIn.setBuffer(rImage);
			cue[i]->cvGImageIn.setBuffer(gImage);
			cue[i]->cvBImageIn.setBuffer(bImage);
			cue[i]->cvGRAYImageIn.setBuffer(vImage);
			//cue[i]->setThreshold(m_threshold);
*/
		}
	}

//	cvCvtPixToPlane( rgbImage->ipl, bImage->ipl, gImage->ipl, rImage->ipl, NULL );
//	cvCvtColor( rgbImage->ipl, hsvImage->ipl, CV_BGR2HSV);
//	cvCvtPixToPlane( hsvImage->ipl, hImage->ipl, sImage->ipl, vImage->ipl, NULL );

}


void CueIntegrationR::execute() {

	if(!m_init) {
		//if(debug) std::cout << getName() << "::execute()::Tracking lost!\n";
		return;
	}

	if(debug) std::cout << getName() << "::execute()\n";
	obtainInput();

#ifdef USE_THREADS
/*
	if(!mp_threads) {
		mp_threads = new pthread_t [m_cuecount];
		int  iret;
		if(debug) std::cout << getName() << "::execute()::m_cuecount = " << m_cuecount << "\n";
		switch(m_cuecount) {
			case 8:
				iret = pthread_create( &(mp_threads[7]), NULL, compute8, this);
			case 7:
				iret = pthread_create( &(mp_threads[6]), NULL, compute7, this);
			case 6:
				iret = pthread_create( &(mp_threads[5]), NULL, compute6, this);
			case 5:
				iret = pthread_create( &(mp_threads[4]), NULL, compute5, this);
			case 4:
				iret = pthread_create( &(mp_threads[3]), NULL, compute4, this);
			case 3:
				iret = pthread_create( &(mp_threads[2]), NULL, compute3, this);
			case 2:
				//if(debug) std::cout << getName() << "::execute()::case 2\n";
				iret = pthread_create( &(mp_threads[1]), NULL, compute2, this);
			case 1:
				//if(debug) std::cout << getName() << "::execute()::case 1\n";
				iret = pthread_create( &(mp_threads[0]), NULL, compute1, this);
				break;
			default:
				break;
		}
	}
*/
	if(debug) std::cout << getName() << "::execute()::m_cuecount = " << m_cuecount << "\n";

	int ret = pthread_cond_broadcast( &m_condition_threadsstart);
//	if(debug) std::cout << getName() << "::execute()::ret = " << ret << "\n";
	
	pthread_mutex_lock( &m_mutex_threadsended );
	pthread_cond_wait( &m_condition_threadsended, &m_mutex_threadsended );
	pthread_mutex_unlock( &m_mutex_threadsended );

#else	
	// executing cues (this part can be parallelized)
	for (unsigned int i=0; i<m_cuecount; ++i) {
		mdp_Cues[i]->execute();
	}

#endif

	// compute overall saliency map (weighted sum (weight = reliability) of individual saliency maps from each cues)
	if(debug) std::cout << getName() << "::execute()::computing saliency map\n";
	this->computesaliencymap();

	if(debug){
		std::cout << getName() << "::execute()::m_track\n";
		std::cout << "winnerPos = ( " << m_track.winnerPos.x << ", " << m_track.winnerPos.y << ")\n";
		std::cout << "winnerSize = ( " << m_track.winnerSize.width << ", " << m_track.winnerSize.height << ")\n";
		std::cout << "winnerRect = ( " << m_track.winnerRect.x << ", " << m_track.winnerRect.y << ", " << m_track.winnerRect.width << ", " << m_track.winnerRect.height << ")\n";
		std::cout << "imageSize = ( " << m_track.imageSize.width << ", " << m_track.imageSize.height << ")\n";
		std::cout << "reliability = " << m_track.reliability << "\n";
	}

	// Compute individual cue's quality
	if(debug) std::cout << getName() << "::execute()::computing quality for each cue\n";
	this->computequality();

	// Compute reliabilities and adapt the cues
	if(debug) std::cout << getName() << "::execute()::computing reliability and adapting each cue\n";
	this->computereliability();

	trackOut.setBuffer(&(m_track));
	trackOut.out();

	cvSalImageOut.out();

	xR = m_track.winnerPos.x;
	yR = m_track.winnerPos.y;
	if(debug)
	{	
		std::cout<<"xr : "<< xR <<"\n";
		std::cout<<"yr : "<< yR <<"\n";
	}

	distxR = xR - 160;
	distyR = yR - 120;

	/*point = cvPoint(xR,yR);
	cvpointOutR.setBuffer(&point);
	cvpointOutR.out();*/

	/*Vector& targetR = PointCueOutPort.prepare();
        targetR.resize(3);
        targetR[0] = xR;
        targetR[1] = yR;
        targetR[2] = 1;
        PointCueOutPort.write();*/

	Bottle& targetR = PointCueOutPort.prepare();
        targetR.clear();
	targetR.addDouble(xR);//millimeters to meters
	targetR.addDouble(yR);//millimeters to meters
	targetR.addDouble(1);//millimeters to meters
	targetR.addString("LookAtStereoPixel");
        PointCueOutPort.write();

	
	if(debug) std::cout << getName() << "::execute() complete\n";


/*
	if(m_run == false) {
		std::cerr << getName() << "::execute()::call initialize() first!\n";
		return;
	}

	rgbImage = cvImageIn.getBuffer();
	if(rgbImage == NULL){ std::cerr<<"ERROR::CueIntegration::execute()::cvImageIn is NULL!...\n"; return; }	

	if(m_track.imageSize.width < 0){
		m_track.imageSize.width = rgbImage->width;
		m_track.imageSize.height = rgbImage->height;
	}

	cvCvtPixToPlane( rgbImage->ipl, bImage->ipl, gImage->ipl, rImage->ipl, NULL );
	cvCvtColor( rgbImage->ipl, hsvImage->ipl, CV_BGR2HSV);
	cvCvtPixToPlane( hsvImage->ipl, hImage->ipl, sImage->ipl, vImage->ipl, NULL );

	// executing cues
	for (int i=0; i<cueCount; i++) {
		cue[i]->execute();
	}

	// compute overall saliency map
	this->computeSaliencies();

//	if(cueColor != NULL) {
//		if(cueColor->templateImageOut.getBuffer() != NULL) {
//			templateImageOut.setBuffer(cueColor->templateImageOut.getBuffer());
//			templateImageOut.out();
//		} 
//	}
//	if(cueTemplate != NULL) {
//		if(cueTemplate->templateImageOut.getBuffer() != NULL) {
//			templateImageOut.setBuffer(cueTemplate->templateImageOut.getBuffer());
//			templateImageOut.out();
//		}
//	}


	// search for winner position
	cvMinMaxLoc( outputImage->ipl, mp_minVal, mp_maxVal, mp_minLoc, mp_maxLoc);
	m_track.reliability = (float)(*mp_maxVal);
//	if(debug) std::cout << "mp_maxLoc = (" << mp_maxLoc->x << ", " << mp_maxLoc->y << ")\n";
//	if(debug) std::cout << "mp_maxVal = " << *mp_maxVal << "\n";

	if(debug) std::cout << getName() << "::execute()::rel = " << m_track.reliability << "\n";

	if(m_track.reliability < m_threshold) {
		// tracking reliability below threshold
		m_track.winnerPos.x = -1;
		m_track.winnerPos.y = -1;
		m_track.reliability = 0;
		objectFound = false;

		if(debug) std::cout << getName() << "::execute()::Tracking Lost!\n";
	}
	else {
		// tracking reliability above threshold
		m_prevtrack.winnerPos = m_track.winnerPos;
		m_prevtrack.reliability = m_track.reliability;
		m_track.winnerPos = *mp_maxLoc;
		m_track.winnerRect.x = m_track.winnerPos.x - cvRound((float)(m_track.winnerSize.width) / 2.0);
		m_track.winnerRect.y = m_track.winnerPos.y - cvRound((float)(m_track.winnerSize.height) / 2.0);
		objectFound = true;

		if(debug) std::cout << getName() << "::execute()::Tracking...\n";
	}


	// Compute qualities
	this->computeQualities();

	// Compute reliabilities and adapt the cues
	this->computeReliabilities();

	trackOut.setBuffer(&(m_track));
	trackOut.out();

	if(debug){
		std::cout << getName() << "::execute()::\n";
		std::cout << "winnerPos = ( " << m_track.winnerPos.x << ", " << m_track.winnerPos.y << ")\n";
		std::cout << "winnerSize = ( " << m_track.winnerSize.width << ", " << m_track.winnerSize.height << ")\n";
		std::cout << "winnerRect = ( " << m_track.winnerRect.x << ", " << m_track.winnerRect.y << ", " << m_track.winnerRect.width << ", " << m_track.winnerRect.height << ")\n";
		std::cout << "imageSize = ( " << m_track.imageSize.width << ", " << m_track.imageSize.height << ")\n";
		std::cout << "reliability = " << m_track.reliability << "\n";
	}

	cvSaliencyImageOut.out();

	if (displayCues == true) {

		float tempqual = 0.0;
		cueDisplay.imageInArr[0].setBuffer(outputImage->ipl);
		cueDisplay.dataInArr[RELIABILITY][0].setBuffer(&(m_track.reliability));
		cueDisplay.dataInArr[QUALITY][0].setBuffer(&tempqual);
		cueDisplay.textInArr[0].setBuffer(new std::string("Overall"));

		for (int i=0; i<cueCount; i++){
			cueDisplay.imageInArr[i+1].setBuffer(cue[i]->salImageOut.getBuffer());
			cueDisplay.dataInArr[RELIABILITY][i+1].setBuffer(this->reliability[i]);
			cueDisplay.dataInArr[QUALITY][i+1].setBuffer(this->quality[i]);
			cueDisplay.textInArr[i+1].setBuffer(new std::string(cue[i]->getName()));
		}

		cueDisplay.execute();

//		cvCueImageOut.setBuffer(cueDisplay.imageOut.getBuffer());
//		cvCueImageOut.out();
	}
*/
}

void CueIntegrationR::computesaliencymap()  {
	IplImage* overallsaliencymap = mp_cvoutputimg->ipl;
	cvSetZero(overallsaliencymap);
	for (unsigned int i=0; i<m_cuecount; ++i){
		CVImage* cvnewsal = mdp_Cues[i]->cvSalImageOut.getBuffer();
		IplImage* newsal = cvnewsal->ipl;
		if(newsal) {
			CvScalar weight = cvScalarAll(mp_rel[i]);
			cvScaleAdd( newsal, weight, overallsaliencymap, overallsaliencymap ); 
		}
	}

	// searching for winner position
	double maxval;
	CvPoint maxloc;
	cvMinMaxLoc( overallsaliencymap, NULL, &maxval, NULL, &maxloc);

	if(debug) std::cout << getName() << "::computesaliencymap()::maxval = " << maxval << "\n";

	if( (float)maxval > m_threshold ) {
		m_track.winnerPos = maxloc;
		m_track.reliability = (float)maxval;
		m_track.winnerRect.x = m_track.winnerPos.x - cvRound((float)(m_track.winnerSize.width) / 2.0);
		m_track.winnerRect.y = m_track.winnerPos.y - cvRound((float)(m_track.winnerSize.height) / 2.0);
	}
	else{
		m_track.reliability = 0.0;
		m_init = false;
	}


}

void CueIntegrationR::computequality() {

	float qualsum = 0.0;
	for (unsigned int i=0; i<m_cuecount; ++i){
		float qual = mdp_Cues[i]->computequality(mp_cvoutputimg, &m_track);
		mp_qual[i] = qual;
		qualsum += qual;
	}

	if(qualsum != 0.0) {
		for (unsigned int i=0; i<m_cuecount; ++i){
			mp_qual[i] /= qualsum;
		}
	}
	
	if(debug) {
		for (unsigned int i=0; i<m_cuecount; ++i){
			std::cout << "Quality for " << mdp_Cues[i]->getName() << " is " << mp_qual[i] << "\n";
		}
	}

/*
	IplImage* newSal;
	if (objectFound == true) {
		qualitySum = 0.0;
		for (int i=0; i<cueCount; i++) {
			if ((newSal = cue[i]->salImageOut.getBuffer()) ) {
				float avgsal = (cvAvg(newSal)).val[0];
				float avglocal = 0.0;
				int count = 0, tempx, tempy;
				float* data = (float*)(newSal->imageData);
			    int width = newSal->width;
				for (int x = -5; x<=5; x++) {
					for (int y=-5; y<=5; y++) {
						tempx = m_track.winnerPos.x + x;
						tempy = m_track.winnerPos.y + y;
						if(tempx >=0 && tempx < newSal->width && tempy >=0 && tempy < newSal->height){
							avglocal = avglocal + data[tempx + tempy * width];
							count = count + 1;
						}
					}
				}
				if(count > 0) avglocal = avglocal / (float)count;
				else avglocal = 0.0;
//				cvMinMaxLoc( newSal, mp_minVal, mp_maxVal, mp_minLoc, mp_maxLoc);
				if (avglocal>=0 && avglocal > (avgsal + 0.1)) qualitySum += *quality[i] = avglocal - avgsal;
				else *quality[i] = 0.0;
			}
		}
		for (int i=0; i<cueCount; i++) {
			if (qualitySum>0)  {
				*quality[i] /= qualitySum;
			}
			else *quality[i] = 0.0;
		}
	}
	else {
		//for (int i=0; i<cueCount; i++) *quality[i] = *defaultQuality[i];
		for (int i=0; i<cueCount; i++) *quality[i] = 0.0;
	}
*/
}

void CueIntegrationR::computereliability() {

	float temp = 1.0-m_tauadapt;
	float relsum = 0.0;
	for(unsigned int i=0; i<m_cuecount; ++i) {
		mp_rel[i] = mp_qual[i] * m_tauadapt + temp * mp_rel[i];
		relsum += mp_rel[i];
	}
	if(relsum != 0.0) {
		for (unsigned int i=0; i<m_cuecount; ++i) {
			mp_rel[i] /= relsum;
		}
	}

	if(debug) {
		for (unsigned int i=0; i<m_cuecount; ++i){
			std::cout << "Reliability for " << mdp_Cues[i]->getName() << " is " << mp_rel[i] << "\n";
		}
	}

	for(unsigned int i=0; i<m_cuecount; ++i) {
		float tfacs = 0.5*m_taucueadapt + m_taucueadapt * (1.0 - mp_qual[i]);
		if(debug) std::cout << "tfacs for " << mdp_Cues[i]->getName() << " is " << tfacs << "\n";
		mdp_Cues[i]->settfacs(tfacs);
		// adapt the cues
		mdp_Cues[i]->adapt();
	}


/*
	int i;
	if (objectFound == true) {
		for (i=0; i<cueCount; i++) {
			*reliability[i] = *quality[i] * tauAdapt + (1.0-tauAdapt) * (*reliability[i]);
		}
		this->normalizeReliabilities();
		for (i=0; i<cueCount; i++) {
			// adapt the cues
			cue[i]->adapt(0.5*tauCueAdapt + tauCueAdapt * (1.0 - *quality[i]*qualitySum));
		}
	}
*/
}

#ifdef USE_THREADS
void *CueIntegrationR::compute1(void* ptr) {
	CueIntegrationR *obj = reinterpret_cast<CueIntegrationR*>(ptr);
	unsigned int counter = 0;
	bool* endthreads = &(obj->m_endthreads);
	Cue** cues = obj->mdp_Cues;
	Cue* cue = cues[counter];
	pthread_barrier_t* barrier = &(obj->m_barrier);
	pthread_cond_t* threadsstart = &(obj->m_condition_threadsstart);
	pthread_cond_t* threadended = &(obj->m_condition_threadsended);
//	pthread_mutex_t* mutex_waitlock = &((obj->mp_mutex_waitlock)[counter]);
	pthread_mutex_t* mutex_waitlock = &((obj->m_mutex_waitlock1));
	while(!(*endthreads)) {
		//pthread_mutex_lock( &((obj->m_mutex_waitlock1)) );
		pthread_mutex_lock( mutex_waitlock );
		//if(obj->debug) std::cout << obj->getName() << "::compute1()::pause\n";
		//pthread_cond_wait( &(obj->m_condition_threadsstart), &((obj->m_mutex_waitlock1)) );
		//pthread_cond_wait( &(obj->m_condition_threadsstart), mutex_waitlock );
		pthread_cond_wait( threadsstart, mutex_waitlock );
		//if(obj->debug) std::cout << obj->getName() << "::compute1()::resume\n";
		//Cue* cue = (obj->mdp_Cues)[counter];
		//pthread_mutex_unlock( &((obj->m_mutex_waitlock1)) );
		pthread_mutex_unlock( mutex_waitlock );

		//if(obj->debug) std::cout << obj->getName() << "::compute1()::executing " << cue->getName() << "\n";
		cue->execute();

		int wait = pthread_barrier_wait(  barrier );
		//int wait = pthread_barrier_wait(  &(obj->m_barrier) );
		if(wait == -1) {
			pthread_cond_signal( threadended );
			//pthread_cond_signal( &(obj->m_condition_threadsended) );
		}
	}
	return NULL;
}

void *CueIntegrationR::compute2(void* ptr) {
	CueIntegrationR *obj = reinterpret_cast<CueIntegrationR*>(ptr);
	unsigned int counter = 1;
	bool* endthreads = &(obj->m_endthreads);
	Cue** cues = obj->mdp_Cues;
	Cue* cue = cues[counter];
	pthread_barrier_t* barrier = &(obj->m_barrier);
	pthread_cond_t* threadsstart = &(obj->m_condition_threadsstart);
	pthread_cond_t* threadended = &(obj->m_condition_threadsended);
//	pthread_mutex_t* mutex_waitlock = &((obj->mp_mutex_waitlock)[counter]);
	pthread_mutex_t* mutex_waitlock = &((obj->m_mutex_waitlock1));
	while(!(*endthreads)) {
		//pthread_mutex_lock( &((obj->m_mutex_waitlock1)) );
		pthread_mutex_lock( mutex_waitlock );
		//if(obj->debug) std::cout << obj->getName() << "::compute2()::pause\n";
		//pthread_cond_wait( &(obj->m_condition_threadsstart), &((obj->m_mutex_waitlock1)) );
		//pthread_cond_wait( &(obj->m_condition_threadsstart), mutex_waitlock );
		pthread_cond_wait( threadsstart, mutex_waitlock );
		//if(obj->debug) std::cout << obj->getName() << "::compute2()::resume\n";
		//Cue* cue = (obj->mdp_Cues)[counter];
		//pthread_mutex_unlock( &((obj->m_mutex_waitlock1)) );
		pthread_mutex_unlock( mutex_waitlock );

		//if(obj->debug) std::cout << obj->getName() << "::compute2()::executing " << cue->getName() << "\n";
		cue->execute();

		int wait = pthread_barrier_wait(  barrier );
		//int wait = pthread_barrier_wait(  &(obj->m_barrier) );
		if(wait == -1) {
			pthread_cond_signal( threadended );
			//pthread_cond_signal( &(obj->m_condition_threadsended) );
		}
	}
	return NULL;
}

void *CueIntegrationR::compute3(void* ptr) {
	CueIntegrationR *obj = reinterpret_cast<CueIntegrationR*>(ptr);
	unsigned int counter = 2;
	bool* endthreads = &(obj->m_endthreads);
	Cue** cues = obj->mdp_Cues;
	Cue* cue = cues[counter];
	pthread_barrier_t* barrier = &(obj->m_barrier);
	pthread_cond_t* threadsstart = &(obj->m_condition_threadsstart);
	pthread_cond_t* threadended = &(obj->m_condition_threadsended);
//	pthread_mutex_t* mutex_waitlock = &((obj->mp_mutex_waitlock)[counter]);
	pthread_mutex_t* mutex_waitlock = &((obj->m_mutex_waitlock1));
	while(!(*endthreads)) {
		//pthread_mutex_lock( &((obj->m_mutex_waitlock1)) );
		pthread_mutex_lock( mutex_waitlock );
		//if(obj->debug) std::cout << obj->getName() << "::compute2()::pause\n";
		//pthread_cond_wait( &(obj->m_condition_threadsstart), &((obj->m_mutex_waitlock1)) );
		//pthread_cond_wait( &(obj->m_condition_threadsstart), mutex_waitlock );
		pthread_cond_wait( threadsstart, mutex_waitlock );
		//if(obj->debug) std::cout << obj->getName() << "::compute2()::resume\n";
		//Cue* cue = (obj->mdp_Cues)[counter];
		//pthread_mutex_unlock( &((obj->m_mutex_waitlock1)) );
		pthread_mutex_unlock( mutex_waitlock );

		//if(obj->debug) std::cout << obj->getName() << "::compute2()::executing " << cue->getName() << "\n";
		cue->execute();

		int wait = pthread_barrier_wait(  barrier );
		//int wait = pthread_barrier_wait(  &(obj->m_barrier) );
		if(wait == -1) {
			pthread_cond_signal( threadended );
			//pthread_cond_signal( &(obj->m_condition_threadsended) );
		}
	}
	return NULL;
}

void *CueIntegrationR::compute4(void* ptr) {
	CueIntegrationR *obj = reinterpret_cast<CueIntegrationR*>(ptr);
	unsigned int counter = 3;
	bool* endthreads = &(obj->m_endthreads);
	Cue** cues = obj->mdp_Cues;
	Cue* cue = cues[counter];
	pthread_barrier_t* barrier = &(obj->m_barrier);
	pthread_cond_t* threadsstart = &(obj->m_condition_threadsstart);
	pthread_cond_t* threadended = &(obj->m_condition_threadsended);
//	pthread_mutex_t* mutex_waitlock = &((obj->mp_mutex_waitlock)[counter]);
	pthread_mutex_t* mutex_waitlock = &((obj->m_mutex_waitlock1));
	while(!(*endthreads)) {
		//pthread_mutex_lock( &((obj->m_mutex_waitlock1)) );
		pthread_mutex_lock( mutex_waitlock );
		//if(obj->debug) std::cout << obj->getName() << "::compute2()::pause\n";
		//pthread_cond_wait( &(obj->m_condition_threadsstart), &((obj->m_mutex_waitlock1)) );
		//pthread_cond_wait( &(obj->m_condition_threadsstart), mutex_waitlock );
		pthread_cond_wait( threadsstart, mutex_waitlock );
		//if(obj->debug) std::cout << obj->getName() << "::compute2()::resume\n";
		//Cue* cue = (obj->mdp_Cues)[counter];
		//pthread_mutex_unlock( &((obj->m_mutex_waitlock1)) );
		pthread_mutex_unlock( mutex_waitlock );

		//if(obj->debug) std::cout << obj->getName() << "::compute2()::executing " << cue->getName() << "\n";
		cue->execute();

		int wait = pthread_barrier_wait(  barrier );
		//int wait = pthread_barrier_wait(  &(obj->m_barrier) );
		if(wait == -1) {
			pthread_cond_signal( threadended );
			//pthread_cond_signal( &(obj->m_condition_threadsended) );
		}
	}
	return NULL;
}

void *CueIntegrationR::compute5(void* ptr) {
	CueIntegrationR *obj = reinterpret_cast<CueIntegrationR*>(ptr);
	unsigned int counter = 4;
	bool* endthreads = &(obj->m_endthreads);
	Cue** cues = obj->mdp_Cues;
	Cue* cue = cues[counter];
	pthread_barrier_t* barrier = &(obj->m_barrier);
	pthread_cond_t* threadsstart = &(obj->m_condition_threadsstart);
	pthread_cond_t* threadended = &(obj->m_condition_threadsended);
//	pthread_mutex_t* mutex_waitlock = &((obj->mp_mutex_waitlock)[counter]);
	pthread_mutex_t* mutex_waitlock = &((obj->m_mutex_waitlock1));
	while(!(*endthreads)) {
		//pthread_mutex_lock( &((obj->m_mutex_waitlock1)) );
		pthread_mutex_lock( mutex_waitlock );
		//if(obj->debug) std::cout << obj->getName() << "::compute2()::pause\n";
		//pthread_cond_wait( &(obj->m_condition_threadsstart), &((obj->m_mutex_waitlock1)) );
		//pthread_cond_wait( &(obj->m_condition_threadsstart), mutex_waitlock );
		pthread_cond_wait( threadsstart, mutex_waitlock );
		//if(obj->debug) std::cout << obj->getName() << "::compute2()::resume\n";
		//Cue* cue = (obj->mdp_Cues)[counter];
		//pthread_mutex_unlock( &((obj->m_mutex_waitlock1)) );
		pthread_mutex_unlock( mutex_waitlock );

		//if(obj->debug) std::cout << obj->getName() << "::compute2()::executing " << cue->getName() << "\n";
		cue->execute();

		int wait = pthread_barrier_wait(  barrier );
		//int wait = pthread_barrier_wait(  &(obj->m_barrier) );
		if(wait == -1) {
			pthread_cond_signal( threadended );
			//pthread_cond_signal( &(obj->m_condition_threadsended) );
		}
	}
	return NULL;
}

void *CueIntegrationR::compute6(void* ptr) {
	unsigned int counter = 5;
	CueIntegrationR *obj = reinterpret_cast<CueIntegrationR*>(ptr);
	bool* endthreads = &(obj->m_endthreads);
	Cue* cue = (obj->mdp_Cues)[counter];
	pthread_barrier_t* barrier = &(obj->m_barrier);
	pthread_cond_t* threadstart = &(obj->m_condition_threadsstart);
	pthread_cond_t* threadended = &(obj->m_condition_threadsended);
//	pthread_mutex_t* mutex_waitlock = &((obj->mp_mutex_waitlock)[counter]);
	pthread_mutex_t* mutex_waitlock = &((obj->m_mutex_waitlock6));
	while(!(*endthreads)) {
		pthread_mutex_lock( mutex_waitlock );
		pthread_cond_wait( threadstart, mutex_waitlock );
		pthread_mutex_unlock( mutex_waitlock );

		cue->execute();

		int wait = pthread_barrier_wait( barrier );
		if(wait == -1) {
			pthread_cond_signal( threadended );
		}
	}
	return NULL;
}

void *CueIntegrationR::compute7(void* ptr) {
	unsigned int counter = 6;
	CueIntegrationR *obj = reinterpret_cast<CueIntegrationR*>(ptr);
	bool* endthreads = &(obj->m_endthreads);
	Cue* cue = (obj->mdp_Cues)[counter];
	pthread_barrier_t* barrier = &(obj->m_barrier);
	pthread_cond_t* threadstart = &(obj->m_condition_threadsstart);
	pthread_cond_t* threadended = &(obj->m_condition_threadsended);
//	pthread_mutex_t* mutex_waitlock = &((obj->mp_mutex_waitlock)[counter]);
	pthread_mutex_t* mutex_waitlock = &((obj->m_mutex_waitlock7));
	while(!(*endthreads)) {
		pthread_mutex_lock( mutex_waitlock );
		pthread_cond_wait( threadstart, mutex_waitlock );
		pthread_mutex_unlock( mutex_waitlock );

		cue->execute();

		int wait = pthread_barrier_wait( barrier );
		if(wait == -1) {
			pthread_cond_signal( threadended );
		}
	}
	return NULL;
}

void *CueIntegrationR::compute8(void* ptr) {
	unsigned int counter = 7;
	CueIntegrationR *obj = reinterpret_cast<CueIntegrationR*>(ptr);
	bool* endthreads = &(obj->m_endthreads);
	Cue* cue = (obj->mdp_Cues)[counter];
	pthread_barrier_t* barrier = &(obj->m_barrier);
	pthread_cond_t* threadstart = &(obj->m_condition_threadsstart);
	pthread_cond_t* threadended = &(obj->m_condition_threadsended);
//	pthread_mutex_t* mutex_waitlock = &((obj->mp_mutex_waitlock)[counter]);
	pthread_mutex_t* mutex_waitlock = &((obj->m_mutex_waitlock8));
	while(!(*endthreads)) {
		pthread_mutex_lock( mutex_waitlock );
		pthread_cond_wait( threadstart, mutex_waitlock );
		pthread_mutex_unlock( mutex_waitlock );

		cue->execute();

		int wait = pthread_barrier_wait( barrier );
		if(wait == -1) {
			pthread_cond_signal( threadended );
		}
	}
	return NULL;
}
#endif

/*
void CueIntegration::normalizeReliabilities()  {
	// Normalizing the reliabilities for cues which are initialized.
	int i;
	float tempReliabilitySum = 0;
	for (i=0; i<cueCount; i++) {
		if ( initialized_cues[i] == true ){
			tempReliabilitySum += *reliability[i];
		}
	}	
	for (i=0; i<cueCount; i++) {
		if ( initialized_cues[i] == true ){
			*(reliability[i]) /= tempReliabilitySum;
		}
	}	
}

int CueIntegration::getTileCount() {
	return cueCount+1; // 'cueCount' cues + overall saliency
}

void CueIntegration::profile_cues(int times) {
	std::cout<<"\n-----------------------------\n";
	std::cout<<getName()<<": Time profile in msecs\n(don't use exec connections for correct measurements!)\n\n";
	double time=0;
	double sum=0;
	double cueSum=0;
	// cue execution
	std::cout<<"CUE execution\n";
	int i;
	for (i=0; i<cueCount; i++) {
		time = cue[i]->execute_profiled(times);
		cueSum += time;
		std::cout << "    " << time << "  (" << cue[i]->getName()<<")\n";
	}
	std::cout << "  " << cueSum << " total\n";
	sum += cueSum;

	// cue adaption
	std::cout<<"CUE adaption\n";
	for (i=0; i<cueCount; i++) {
		time = cue[i]->adapt_profiled(tauAdapt, times);
		//time = cue[i]->adapt_profiled(times);
		cueSum += time;
		std::cout << "    " << time << "  (" << cue[i]->getName()<<")\n";
	}
	std::cout << "  " << cueSum << " total\n";
	sum += cueSum;

	// DI	
	std::cout<<"DI\n";
	time = execute_profiled(times);
	std::cout << "  " << (time-sum) << "\n";

	std::cout<<"TOTAL\n";
	std::cout << "  " << time << "\n\n";
}

void CueIntegration::profile_cues_ms() {
}

void CueIntegration::constructCueMotion(double threshold, int kernelSize, float temprel) {
	cueMotion = new CueMotion("CueMotion");
	cueMotion->setParam(threshold, kernelSize);
	this->addCue(cueMotion, temprel);
	//initialized_cues[0] = true;
}

CueColorKernel* CueIntegration::constructCueColorKernel(int opmode, int colormode, int maxIter, float tfacs, float temprel) {
	int size = 16;
	int binSize[3] = {size, size, size};
//	int binSize[3] = {16, 4, 8};
	tfacsColor = tfacs;
	cueColor = new CueColorKernel("CueColorKernel");
	cueColor->setParam(opmode, colormode, maxIter, binSize);
	this->addCue(cueColor, temprel);
	return cueColor;
}

void CueIntegration::constructCueContrastKernel(int mode, int maxIter, int ts1, int ts2, float tfacs, float temprel) {
	int size = 8;
	int binSize[2] = {size, size};
	int scales[2] = {ts1, ts2};
	int dim = 2;
	tfacsContrast = tfacs;
	cueContrast = new CueContrastKernel("CueContrastKernel");
	cueContrast->setParam(mode, dim, maxIter, binSize, scales);
	this->addCue(cueContrast, temprel);
}

void CueIntegration::constructCuePrediction(int mode, int size, float temprel) {
	cuePrediction = new CuePrediction("CuePrediction");
	cuePrediction->setParam(mode, size, 160, 120);
	this->addCue(cuePrediction, temprel);
}

void CueIntegration::constructCueTemplate(int templateSize, int method, float tfacs, float temprel ) {
	tfacsTemplate = tfacs;
	cueTemplate = new CueTemplate("CueTemplate");
	cueTemplate->setParam(templateSize, method, tfacsTemplate);
	this->addCue(cueTemplate, temprel);

}


//void CueIntegration::constructCueColorSimple(int templateSize, int kernelSize, float tfacs, float temprel) {
//	tfacsColor = tfacs;
//	cueColorSimple = new CueColorSimple("CueColorSimple module");
//	cueTemplate->setParam(templateSize, kernelSize);
//	this->addCue(cueColorSimple, temprel);
//}


*/

