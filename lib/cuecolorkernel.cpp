#include "cuecolorkernel.h"
#include <math.h>
#include <algorithm>



CueColorKernel::CueColorKernel(std::string name) : Cue(name) {
	std::cout << "Constructing " << getName() << "...";

	mp_cvimg1 = NULL;
	mp_cvimg2 = NULL;
	mp_cvimg3 = NULL;
	mp_cvhsvimg = NULL;
	mp_target_candidate = NULL;
	mp_cvoutputimg = NULL;
	mpDrawModel = NULL;
	mpDrawCandidate = NULL;
	mdpCropResize = NULL;
	mdp_cvcroppedimg = NULL;
	mp_cvmaskimg = NULL;
	mdp_cvimg = NULL;
	mp_LUT = NULL;
	mp_pixeldata = NULL;
	mp_index = NULL;
	mp_rel = NULL;
	mp_y1 = NULL;

	m_minSize = 50;
	c_d = 10.0; // ??
	d = -1.9; // ??
	g_index = 0.5*(1.0/c_d)*(d+2.0);
	m_threshold = 0.2;
	h = 1.0;
	epsilon = 0.1;
	m_tfacs = 0.01;
	m_normsize = 50;
	m_objsizenorm.width = m_normsize;
	m_objsizenorm.height = m_normsize;

	m_init = false;
	m_setParam = false;
	debug = false;
	draw = false;
	m_forceMinSize = false;
	m_forceNormSize = false;
	
	setParam();
	
	std::cout << "finished.\n";
}

CueColorKernel::~CueColorKernel() {
	std::cout << "Destructing " << getName() << "...";

	if(mp_cvimg1) delete mp_cvimg1;
	if(mp_cvimg2) delete mp_cvimg2;
	if(mp_cvimg3) delete mp_cvimg3;
	if(mp_cvhsvimg) delete mp_cvhsvimg;
	if(mp_target_candidate) delete [] mp_target_candidate;
	if(mp_cvoutputimg) delete mp_cvoutputimg;
	if(mpDrawModel) delete mpDrawModel;
	if(mpDrawCandidate) delete mpDrawCandidate;
	if(mdpCropResize) {
		for(unsigned int i = 0;i<m_dimData;i++) if(mdpCropResize[i]) delete mdpCropResize[i];
		delete [] mdpCropResize;
	}
	if(mdp_cvcroppedimg) delete [] mdp_cvcroppedimg;
	if(mp_cvmaskimg) delete mp_cvmaskimg;
	if(mdp_cvimg) delete [] mdp_cvimg;
	if(mp_LUT) {
		for(unsigned int i = 0;i<m_dimData;i++) if(mp_LUT[i]) delete [] mp_LUT[i];
		delete [] mp_LUT;
	}
	if(mp_pixeldata) delete [] mp_pixeldata;
	if(mp_index) delete [] mp_index;
	if(mp_rel) delete [] mp_rel;
	if(mp_y1) delete [] mp_y1;

	std::cout << "finished.\n";
}

void CueColorKernel::setParam(int opmode, int colormode, int maxIter, std::vector<int>* binSize) {
//	if(debug) std::cout << getName() << "::setParam()::Started\n";

	m_opmode = opmode;
	m_colormode = colormode;
	m_maxIter = maxIter;

	if(m_colormode == USE_RGB || m_colormode == USE_HSV){
		m_dimData = 3;
	}
	else if(m_colormode == USE_HS){
		m_dimData = 2;
	}
	else{
		std::cerr << getName() << "::setParam()::ERROR::Unsupported colormode [" << colormode << "]!\n";
		return;
	}

	if(binSize) {
		if(binSize->size() != m_dimData) {
			std::cerr << getName() << "::setParam()::ERROR::binSize dim does not match colormode!\n";
			return;
		}

		m_vecbinsize = *binSize;
	}
	else{
		m_vecbinsize.push_back(20);
		m_vecbinsize.push_back(20);
		m_vecbinsize.push_back(20);
	}

	m_totalnumbins = 1;
	for(std::vector<int>::iterator iter = m_vecbinsize.begin();iter != m_vecbinsize.end();++iter) {
		int temp = *iter;
		m_totalnumbins *= temp;
	}
	
	m_target_model.allocate(m_totalnumbins);
	m_target_orig.allocate(m_totalnumbins);
	
//	if(mp_target_candidate) delete [] mp_target_candidate;
	if(!mp_target_candidate) mp_target_candidate = new CSCVector [3];
	for(int i = 0;i<3;i++) mp_target_candidate[i].allocate(m_totalnumbins);

	m_init = false;

	if(draw) {
		if(!mpDrawModel) mpDrawModel = new DrawVector("DrawModel"+getName(), m_totalnumbins);
		else mpDrawModel->setDim(m_totalnumbins);

		if(!mpDrawCandidate) mpDrawCandidate = new DrawVector("DrawCandidate"+getName(), m_totalnumbins);
		else mpDrawCandidate->setDim(m_totalnumbins);
	}

	if(mdpCropResize) {
		for(unsigned int i = 0;i<m_dimData;i++) if(mdpCropResize[i]) delete mdpCropResize[i];
		delete [] mdpCropResize;
	}
	mdpCropResize = new CropResize* [m_dimData];
	for(unsigned int i = 0;i<m_dimData;++i) {
		std::ostringstream ost;
		ost << i;
		std::string modulename = "CropResize" + ost.str() + getName();
		mdpCropResize[i] = new CropResize(modulename);
	}

	if(mdp_cvcroppedimg) delete [] mdp_cvcroppedimg;
	mdp_cvcroppedimg = new CVImage* [m_dimData];

	//if(mdp_cvimg) delete [] mdp_cvimg;
	if(!mdp_cvimg) mdp_cvimg = new CVImage* [3];

	if(mp_LUT) {
		for(unsigned int i = 0;i<m_dimData;i++) if(mp_LUT[i]) delete [] mp_LUT[i];
		delete [] mp_LUT;
	}
	mp_LUT = new int* [m_dimData];
	if(m_colormode == USE_RGB){
		for(unsigned int i=0;i<m_dimData;i++) mp_LUT[i] = new int [256];
		for(unsigned int j=0;j<m_dimData;j++){
			int binsize = m_vecbinsize[j];
			for(int i=0;i<=255;i++){
				mp_LUT[j][i] = cvFloor( (float)(binsize) / 256.0 * (float)i );
			}
		}
	}
	else if(m_colormode == USE_HSV || m_colormode == USE_HS){
		mp_LUT[0] = new int [180];
		int binsize1 = m_vecbinsize[0];
		for(int i=0;i<=179;i++){
			mp_LUT[0][i] = cvFloor( (float)(binsize1) / 180.0 * (float)i );
		}
		for(unsigned int i = 1;i<m_dimData;i++) mp_LUT[i] = new int [256];
		for(unsigned int j=1;j<m_dimData;j++){
			int binsize2 = m_vecbinsize[j];
			for(int i=0;i<=255;i++){
				mp_LUT[j][i] = cvFloor( (float)(binsize2) / 256.0 * (float)i );
			}
		}
	}

	if(mp_pixeldata) delete [] mp_pixeldata;
	mp_pixeldata = new unsigned char [m_dimData];
	if(mp_index) delete [] mp_index;
	mp_index = new int [m_dimData];

	if(!mp_rel) mp_rel = new float [3];
	if(!mp_y1) mp_y1 = new CvPoint2D32f [3];

	m_setParam = true;
}

void CueColorKernel::execute() {

	if(debug) std::cout << getName() << "::execute()\n";

	if(!m_setParam) { std::cerr << getName() << "::execute()::ERROR::Call setParam() first!\n"; return; }

	if(!m_init) { initialize(); return; }

	obtainInput();

	TrackData* track = trackIn.getBuffer();
	if(!track) { 
		//if(debug) std::cerr<< getName() << "::ERROR::execute()::trackIn is NULL!...\n"; 
		track = &m_track; 
	}

	m_y0.x = (float)(track->winnerPos.x);
	m_y0.y = (float)(track->winnerPos.y);
	if(debug) std::cout << getName() << "::execute()::m_y0 = [" << m_y0.x << " " << m_y0.y << "]\n";


	for(int i=0;i<3;++i) mp_rel[i] = 0.0;
	runkernel(&(mp_y1[0]), &(mp_target_candidate[0]), &(mp_rel[0]), h);
	//runkernel(&(mp_y1[1]), &(mp_target_candidate[1]), &(mp_rel[1]), h*1.1);
	//runkernel(&(mp_y1[2]), &(mp_target_candidate[2]), &(mp_rel[2]), h*0.9);
	unsigned int maxindex = 0;
	//maxindex = findmax(mp_rel, 3);


	m_track.reliability = mp_rel[maxindex];

	if(mp_rel[maxindex] > m_threshold){
		//m_y0.x = mp_y1[maxindex].x;
		//m_y0.y = mp_y1[maxindex].y;

		//if(maxindex == 1){
		//	h = h * 1.1;
		//	m_objsize.width = cvRound((float)m_objsize.width * h);
		//	m_objsize.height = cvRound((float)m_objsize.height * h);
		//}
		//else if(maxindex == 2){
		//	h = h * 0.9;
		//	m_objsize.width = cvRound((float)m_objsize.width * h);
		//	m_objsize.height = cvRound((float)m_objsize.height * h);
		//}
		this->drawGrayBall(&(mp_y1[maxindex]), &m_objsize);

		m_track.winnerPos.x = cvRound(mp_y1[maxindex].x);
		m_track.winnerPos.y = cvRound(mp_y1[maxindex].y);
		m_track.winnerSize.width = m_objsize.width;
		m_track.winnerSize.height = m_objsize.height;

		m_track.winnerRect.x = cvRound(mp_y1[maxindex].x - (float)(m_track.winnerSize.width/2.0));
		m_track.winnerRect.y = cvRound(mp_y1[maxindex].y - (float)(m_track.winnerSize.height/2.0));
		m_track.winnerRect.width = m_objsize.width;
		m_track.winnerRect.height = m_objsize.height;
		
		if(debug) std::cout << getName() << "::execute()::maxindex = " << maxindex << ", mp_y1[" << maxindex << "] = [" << mp_y1[maxindex].x << " " << mp_y1[maxindex].y << "] size = [" << m_track.winnerSize.width << " " << m_track.winnerSize.height << "]\n";
	}
	else{
		cvSetZero(mp_cvoutputimg->ipl);

		if(debug) std::cout << getName() << "::execute()::rel = " << mp_rel[maxindex] << " below threshold [" << m_threshold << "]. Tracking lost\n";

		m_track.reliability = 0.0;
		
		m_init = false;
	}

	trackOut.setBuffer(&m_track);
	trackOut.out();
	
	cvSalImageOut.out();

	if(debug) std::cout << getName() << "::execute() complete\n";

}

void CueColorKernel::runkernel(CvPoint2D32f* nextpos, CSCVector* prob_cand, float* rel, float hval) {
	if(debug) std::cout << getName() << "::runkernel()\n";
	bool result = false;
	int count = 0;
	CvPoint2D32f oldpos = m_y0;
	//CvSize objsize = m_track.winnerSize;

	if(debug) std::cout << getName() << "::runkernel()::oldpos = [" << oldpos.x << " " << oldpos.y << "]\n";

	while(!result){
	
		switch(m_opmode) {
			case SIMPLE:
				//cropNResize(&oldpos, &objsize, hval);
				cropNResize(&oldpos, &m_objsizenorm, hval);
				calculateProbability(prob_cand, hval);
				break;
			default:
				std::cerr << getName() << "::runkernel()::Currently only SIMPLE mode is supported!\n";
				return;
		}
		computeWeights(prob_cand); // Step2
		findNextLocation(nextpos, hval); // Step 3
		//if(debug) std::cout << getName() << "::runkernel()::nextpos = [" << nextpos->x << " " << nextpos->y << "]\n";
		result = checkCondition(&oldpos, nextpos, count); // Step 6
		if(debug) std::cout << getName() << "::runkernel()::count = " << count << ", nextpos = [" << nextpos->x << ", " << nextpos->y << "]\n";
		count++;
	}
	(*rel) = computeSimilarity(prob_cand, &m_target_model);
	if(debug) std::cout << getName() << "::runkernel()::while() loop complete::count = " << count << ", rel = " << (*rel) << ", nextpos = [" << nextpos->x << ", " << nextpos->y << "]\n";
}

void CueColorKernel::initialize() {

	if(debug) std::cout << getName() << "::initialize()\n";

	obtainInput();

	TrackData* track = trackIn.getBuffer();
	if(!track) { 
		if(debug) std::cerr << getName() << "::ERROR::initialize()::trackIn is NULL!...\n"; 
		return; 
	}

	if(track->reliability < m_threshold) {
		if(debug) std::cerr << getName() << "::ERROR::initialize()::track->reliability below threshold!...\n"; 
		return;
	}

	if (!mp_cvoutputimg) {
		unsigned int width = mp_cvimg1->width;
		unsigned int height = mp_cvimg1->height;
		if(debug) std::cout << getName() << "::initialize()::mp_cvimg1 size = [" << width << " " << height << "]\n"; 
		mp_cvoutputimg = new CVImage(cvSize(width, height), CV_32FC1, 0);
		cvSalImageOut.setBuffer(mp_cvoutputimg);
	}
	
	cvSetZero(mp_cvoutputimg->ipl);

	if(!mp_cvimg1) { std::cerr << getName() << "::ERROR::initialize()::mp_cvimg1 is NULL!\n"; return; }

	float xratio, yratio;
	xratio = (float)(mp_cvimg1->width) / (float)(track->imageSize.width);
	yratio = (float)(mp_cvimg1->height) / (float)(track->imageSize.height);

	m_track.winnerPos.x = cvRound( (float)(track->winnerPos.x) * xratio );
	m_track.winnerPos.y = cvRound( (float)(track->winnerPos.y) * yratio );
	m_track.winnerSize.width = cvRound( (float)(track->winnerSize.width) * xratio );
	m_track.winnerSize.height = cvRound( (float)(track->winnerSize.height) * xratio );

	m_track.winnerRect.x = m_track.winnerPos.x - cvRound( (float)(m_track.winnerSize.width)/2.0 );
	m_track.winnerRect.y = m_track.winnerPos.y - cvRound( (float)(m_track.winnerSize.height)/2.0 );
	m_track.winnerRect.width = m_track.winnerSize.width;
	m_track.winnerRect.height = m_track.winnerSize.height;

	m_track.reliability = track->reliability;

	m_track.imageSize.width = mp_cvimg1->width;
	m_track.imageSize.height = mp_cvimg1->height;

//	mp_objSize = &(m_track.winnerSize);
	m_objsize = m_track.winnerSize;
	if(!m_forceNormSize) m_objsizenorm = m_objsize;

	m_target_model.setZero();
	for(int i = 0;i<3;i++) mp_target_candidate[i].setZero();

	unsigned int size;
	if(m_forceMinSize) {
		size = std::max(m_track.winnerSize.width, m_minSize);
		m_track.winnerSize.width = size;	
		size = std::max(m_track.winnerSize.height, m_minSize);
		m_track.winnerSize.height = size;	
	}
	size = (m_track.winnerSize.width) * (m_track.winnerSize.height);
	m_weight.allocate(size);
	m_weight.setZero();

	CvPoint2D32f winner;
	winner.x = (float)(m_track.winnerPos.x);
	winner.y = (float)(m_track.winnerPos.y);
	
	CvSize objsize;
	objsize = m_track.winnerSize;
	
	if(debug) std::cout << getName() << "::initialize()::obj size = [" << objsize.width << " " << objsize.height << "]\n";
	
	if(mp_cvmaskimg) delete mp_cvmaskimg;
	mp_cvmaskimg = NULL;

	float* src = m_target_model.getData();
	float* dst = mp_target_candidate[0].getData();
	float* orig = m_target_orig.getData();
	float sim;
	
	switch(m_opmode) {
		case SIMPLE:
			//cropNResize(&winner, &objsize, 1.0);
			cropNResize(&winner, &m_objsizenorm, 1.0);
			calculateProbability(&m_target_model, 1.0);
			memcpy((char*)dst, (char*)src, m_totalnumbins*sizeof(float));
			memcpy((char*)orig, (char*)src, m_totalnumbins*sizeof(float));
			sim = computeSimilarity(&(mp_target_candidate[0]), &m_target_model);
			drawGrayBall(&winner, &objsize);

			break;
		default:
			std::cerr << getName() << "::initialize()::ERROR::Currently only SIMPLE mode is supported!\n";
			return;	
	}

	cvSalImageOut.out();

	if(draw) {
		mpDrawModel->dataIn.setBuffer(&m_target_model);
		mpDrawCandidate->dataIn.setBuffer(&(mp_target_candidate[0]));
		mpDrawModel->execute();
		mpDrawCandidate->execute();
		cvModelOut.setBuffer(mpDrawModel->cvImageOut.getBuffer());
		cvCandidateOut.setBuffer(mpDrawCandidate->cvImageOut.getBuffer());
		cvModelOut.out();
		cvCandidateOut.out();
	}

	m_init = true;
	
	if(debug) std::cout << getName() << "::initialize() complete\n";
}

void CueColorKernel::adapt() {

	if(!m_init) return;

	obtainInput();

	TrackData* track = trackIn.getBuffer();
	if(!track) {  track = &m_track; }

	float* modeldata = m_target_model.getData();
	float* canddata = mp_target_candidate[0].getData();
	float* origdata = m_target_orig.getData();

	if(track->reliability > m_threshold){
		// adapt toward new param.
		float temp1 = m_tfacs / 100.0;
		float temp2 = 1.0 - temp1;
		for(int i=0;i<m_totalnumbins;i++){
			//prob_target_model[i] = tfacsNew*prob_adapt[i] + temp*prob_target_model[i];
			float temp3 = temp1 * canddata[i] + temp2 * modeldata[i];
			modeldata[i] = temp3;
		}
	}
	else{
		// adapting back to the original param.
		//float temp1 = tfacsNew/10.0;
		float temp1 = m_tfacs/1000.0;
		float temp2 = 1.0-temp1;
		//if(debug) std::cout << getName() << "::adapt()::adapting back to original model, temp1 = " << temp1 << "\n";
		for(int i=0;i<m_totalnumbins;i++){
			float temp3 = temp1*origdata[i] + temp2*modeldata[i];
			modeldata[i] = temp3;
		}
	}

}

void CueColorKernel::obtainInput() {
	CVImage* cvrgbimg = cvImageIn.getBuffer();
	if(!cvrgbimg) { std::cerr << getName() << "::execute()::ERROR::cvImageIn is NULL!\n";  return; }

	if(!mp_cvimg1) {
		CvSize imgsize = cvSize(cvrgbimg->width, cvrgbimg->height);
		mp_cvimg1 = new CVImage(imgsize, CV_8UC1, 0);
		mp_cvimg2 = new CVImage(imgsize, CV_8UC1, 0);
		mp_cvimg3 = new CVImage(imgsize, CV_8UC1, 0);
	}

	if(m_colormode == USE_RGB){
		cvCvtPixToPlane( cvrgbimg->ipl, mp_cvimg3->ipl, mp_cvimg2->ipl, mp_cvimg1->ipl, NULL );
	}
	else if(m_colormode == USE_HSV || m_colormode == USE_HS){
		if(!mp_cvhsvimg) {
			CvSize imgsize = cvSize(cvrgbimg->width, cvrgbimg->height);
			mp_cvhsvimg = new CVImage(imgsize, CV_8UC3, 0);
		}
		cvCvtColor( cvrgbimg->ipl, mp_cvhsvimg->ipl, CV_BGR2HSV);
		cvCvtPixToPlane( mp_cvhsvimg->ipl, mp_cvimg1->ipl, mp_cvimg2->ipl, mp_cvimg3->ipl, NULL );
	}			

	mdp_cvimg[0] = mp_cvimg1;
	mdp_cvimg[1] = mp_cvimg2;
	mdp_cvimg[2] = mp_cvimg3;

	cvImage1Out.setBuffer(mp_cvimg1);
	cvImage1Out.out();

	cvImage2Out.setBuffer(mp_cvimg2);
	cvImage2Out.out();

	cvImage3Out.setBuffer(mp_cvimg3);
	cvImage3Out.out();

}

void CueColorKernel::cropNResize(CvPoint2D32f* pos, CvSize* size, float hval) {                

	if(!mp_cvmaskimg){
		mp_cvmaskimg = new CVImage(*size, CV_8UC1, 0);
	}


	CvRect inputrect;
	inputrect.x = cvRound((float)pos->x - (float)size->width/2.0);
	inputrect.y = cvRound((float)pos->y - (float)size->height/2.0);
	inputrect.width = size->width;
	inputrect.height = size->height;

	for(unsigned int i = 0;i<m_dimData;++i) {
		mdpCropResize[i]->cvImageIn.setBuffer(mdp_cvimg[i]);
		mdpCropResize[i]->rectIn.setBuffer(&inputrect);
		mdpCropResize[i]->sizeIn.setBuffer(size);
		mdpCropResize[i]->execute();
		mdp_cvcroppedimg[i] = mdpCropResize[i]->cvImageOut.getBuffer();
	}

	CvPoint maskcenter;
	maskcenter.x = cvRound((float)(size->width)/2.0 - 1.0);
	maskcenter.y = cvRound((float)(size->height)/2.0 - 1.0);
	CvSize maskaxis;
	maskaxis.width = cvRound((float)(size->width)/2.0 - 1.0);
	maskaxis.height = cvRound((float)(size->height)/2.0 - 1.0);
	drawEllipse(mp_cvmaskimg, &maskcenter, &maskaxis);

	cvTemplateOut.setBuffer(mdp_cvcroppedimg[0]);
	cvTemplateOut.out();
	cvMaskOut.setBuffer(mp_cvmaskimg);
	cvMaskOut.out();

}

void CueColorKernel::drawEllipse(CVImage* maskimg, CvPoint* maskcenter, CvSize* maskaxis) {
	cvSetZero(maskimg->ipl);
	float angle = 0;
	float startAngle = 0;
	float endAngle = 360;
	int thickness = -1;
	cvEllipse( maskimg->ipl, *maskcenter, *maskaxis, angle, startAngle, endAngle, CV_RGB(255,255,255), thickness );
}


void CueColorKernel::calculateProbability(CSCVector* prob, float hval) {

	float width = (float)(mdp_cvcroppedimg[0]->width);
	float height = (float)(mdp_cvcroppedimg[0]->height);
	float centerx = ((float)(mdp_cvcroppedimg[0]->width) - 1.0) / 2.0;
	float centery = ((float)(mdp_cvcroppedimg[0]->height) - 1.0) / 2.0;
	float* probdata = prob->getData();
	prob->setZero();
	float Cinv = 0.0;
	unsigned int widthstepmask = mp_cvmaskimg->ipl->widthStep;
	char* maskdata = mp_cvmaskimg->ipl->imageData;
	for(int j = 0; j < height; j++ ){
		for(int i = 0; i < width; i++ ){
			unsigned char temp1 = (unsigned char)(maskdata[j*widthstepmask + i]);
			if( temp1 > 100 ){
				float distx = (float)i - centerx;
				float disty = (float)j - centery;
				float normdistx = distx / (width*0.5*hval);
				float normdisty = disty / (height*0.5*hval);
				float norm_dist = sqrt(normdistx*normdistx + normdisty*normdisty);
				float temp2 = computeKernel(norm_dist);

				for(unsigned int k = 0;k<m_dimData;k++){
					unsigned char* cropdata = (unsigned char*)(mdp_cvcroppedimg[k]->ipl->imageData);
					unsigned int widthstepcrop = mdp_cvcroppedimg[k]->ipl->widthStep;
					mp_pixeldata[k] = cropdata[j*widthstepcrop + i];
				}
				unsigned int index = this->findIndex(mp_pixeldata);
				probdata[index] = probdata[index] + temp2;
				Cinv = Cinv + temp2;
			}
		}
	}

	// Normalization
	if(Cinv != 0.0) {
		for(int i = 0; i < m_totalnumbins; i++ ){
			probdata[i] /= Cinv;
		}
	}
}

float CueColorKernel::computeSimilarity(CSCVector* p, CSCVector* q) {
	float* vec1 = p->getData();
	float* vec2 = q->getData();
	float result = 0.0;
	for(int i=0;i<m_totalnumbins;++i){
		float temp = sqrt(vec1[i] * vec2[i]);
		result += temp;
	}
	if(debug) std::cout << getName() << "::computeSimilarity()::sim = " << result << "\n";
	return result;	
}

void CueColorKernel::drawGrayBall(CvPoint2D32f* pos, CvSize* size) {
	float temp = 1.0;
	temp = (mp_cvoutputimg->width) / 160.0;
	float K = 2.0 * size->width * temp; // smaller K, smaller circle
	int windowSizeX = cvRound(size->width * temp);
	int windowSizeY = cvRound(size->height * temp);
	int width = mp_cvoutputimg->width;
	int height = mp_cvoutputimg->height;
	IplImage* img = mp_cvoutputimg->ipl;
	cvSetZero(img);
	for(int x = -windowSizeX; x <= windowSizeX; ++x) {
		for(int y = -windowSizeY; y <= windowSizeY; ++y) {
		
			int posx = cvRound(pos->x) + x;
			int posy = cvRound(pos->y) + y;

			if(posx >= 0 && posy >= 0 && posx < width && posy < height) {
				double val = exp(-0.5/K*((float)x*(float)x + (float)y*(float)y));
				cvSetReal2D(img, posy, posx, val);
			}
		}
	}
}

int CueColorKernel::findIndex(unsigned char* input) {
	int temp, result;
	for(unsigned int i =0;i<m_dimData;i++){ 
		temp = (int)(input[i]);
		mp_index[i] = mp_LUT[i][temp];
	}
	result = 0;
	temp = 1;
	for(unsigned int i =0;i<m_dimData;i++){
		if(i>0) temp = temp * m_vecbinsize[i-1];
		result = result + mp_index[i] * temp;
	}
	if(result < 0 || result > m_totalnumbins) { std::cerr << getName() << "::findIndex()::ERROR::Result out of range!\n"; }
//	if(debug) {
//		if(m_dimData>2) std::cout << getName() << "::findIndex()::input = [" << (int)(input[0]) << " " << (int)(input[1]) << " " << (int)(input[2]) << "] index = [" << mp_index[0] << " " <<  mp_index[1] << " " << mp_index[2] << "] result = " << result << "\n";
//		else std::cout << getName() << "::findIndex()::input = [" << (int)(input[0]) << " " << (int)(input[1]) << "] index = [" << mp_index[0] << " " <<  mp_index[1] << "] result = " << result << "\n";
//	}
	return result;
}


float CueColorKernel::computeKernel(float dist) {

	if(dist < 0.0) std::cerr << getName() << "::computeKernel()::ERROR::dist < 0.0!\n"; 

	// Kernels with Epanechikov profile
	float result = 0.0;
//	if (fabs(dist) <= sqrt(2)){
	if ( dist <= 1.0){
		result = g_index*(1.0 - dist );
	}
	return result;
}

void CueColorKernel::test() {
	int startx = std::min(m_track.winnerSize.width, m_track.winnerPos.x - cvRound((float)m_track.winnerSize.width/2.0));
	int endx = std::max(m_track.imageSize.width-m_track.winnerSize.width, m_track.winnerPos.x + cvRound((float)m_track.winnerSize.width/2.0));
	int starty = std::min(m_track.winnerSize.height, m_track.winnerPos.y - cvRound((float)m_track.winnerSize.height/2.0));
	int endy = std::max(m_track.imageSize.height-m_track.winnerSize.height, m_track.winnerPos.y + cvRound((float)m_track.winnerSize.height/2.0));

	CvPoint2D32f pos;

	cvSetZero(mp_cvoutputimg->ipl);
	float* salmap = (float*)(mp_cvoutputimg->ipl->imageData);
	int salwidth = mp_cvoutputimg->width;

	for(int j = starty;j<endy;++j) {
		for(int i = startx;i<endx;++i) {
			pos.x = i;
			pos.y = j;
			cropNResize(&pos, &(m_track.winnerSize), 1.0);			
			calculateProbability(&(mp_target_candidate[0]), 1.0);
			float sim = computeSimilarity(&(mp_target_candidate[0]), &m_target_model);
			salmap[j*salwidth + i] = sim;			
		}
	}

	cvSalImageOut.out();
}

void CueColorKernel::computeWeights(CSCVector* p) {

	int width = mp_cvmaskimg->width;
	int height = mp_cvmaskimg->height;
	float* vecdata = p->getData();

 	// Step 2
	m_weight.setZero();
	float* weightdata = m_weight.getData();
	
	float* modeldata = m_target_model.getData();

	char* maskdata = mp_cvmaskimg->ipl->imageData;
	unsigned int widthstepmask = mp_cvmaskimg->ipl->widthStep;
	for(int j = 0; j < height; j++ ){
		for(int i = 0; i < width; i++ ){
			unsigned char temp = (unsigned char)(maskdata[j*widthstepmask + i]);
			if( temp > 200 ){
				for(unsigned int k = 0;k<m_dimData;k++) {
					char* cropdata = mdp_cvcroppedimg[k]->ipl->imageData;
					unsigned int widthstepcrop = mdp_cvcroppedimg[k]->ipl->widthStep;
					mp_pixeldata[k] = cropdata[j*widthstepcrop + i];
				}
				int index = findIndex(mp_pixeldata);
				float tempvecdata = vecdata[index];
				if (tempvecdata != 0.0){
					weightdata[j*widthstepmask + i] = sqrt(modeldata[index]/tempvecdata);
				}
				else{
				}
			}
		}
	}
}


void CueColorKernel::findNextLocation(CvPoint2D32f* pos, float hval) {
	// Step 3
	float centerx = (float)(mdp_cvcroppedimg[0]->width) / 2.0;
	float centery = (float)(mdp_cvcroppedimg[0]->height) / 2.0;
	float sum = 0.0;
	float tempx = 0.0, tempy = 0.0;
	char* maskdata = mp_cvmaskimg->ipl->imageData;
	unsigned int widthstepmask = mp_cvmaskimg->ipl->widthStep;
	unsigned int widthmask = mp_cvmaskimg->width;
	unsigned int heightmask = mp_cvmaskimg->height;
	float* weightdata = m_weight.getData();
	for(unsigned int j = 0; j < heightmask; j++ ){
		for(unsigned int i = 0; i < widthmask; i++ ){
			unsigned char temp = (unsigned char)(maskdata[j*widthstepmask + i]);
			
			unsigned int index = j*widthmask + i;
			float tempweight = weightdata[index];
			
			if( temp > 200 && tempweight != 0.0){
				float temp1 = (float)(i - centerx);
				float temp2 = (float)(j - centery);

				tempx += ( temp1 * tempweight );
				tempy += ( temp2 * tempweight );
				sum += ( tempweight );
			}
		}
	}

	if(sum > 0.0){
		pos->x = m_y0.x + (tempx / sum);
		pos->y = m_y0.y + (tempy / sum);
		if(debug) std::cout << getName() << "::findNextLocation()::sum>0::pos = [" << pos->x << " " << pos->y << "]\n";
	}
	else{
		//pos->x = m_y0.x;
		//pos->y = m_y0.y;
		*pos = m_y0;
		if(debug) std::cout << getName() << "::findNextLocation()::sum<=0::pos = [" << pos->x << " " << pos->y << "]\n";
	}
}

bool CueColorKernel::checkCondition(CvPoint2D32f* pos, CvPoint2D32f* nextpos, int count) {
	if(debug) std::cout << getName() << "::checkCondition()::pos = [" << pos->x << " " << pos->y << "] nextpos = [" << nextpos->x << " " << nextpos->y << "] count = " << count << "\n";

	// Step 6
	float distx = (float)(pos->x) - (float)(nextpos->x);
	float disty = (float)(pos->y) - (float)(nextpos->y);
	float dist = sqrt( distx*distx + disty*disty );
	bool result;
	if (dist < epsilon || count >= m_maxIter){
		// condition is met, no more iteration
		result = true;
		if(debug) std::cout << getName() << "::checkCondition()::condition met. dist = " << dist << " epsilon = " << epsilon << ", count = " << count << ", maxiter = " << m_maxIter << "\n";
	}
	else{
		result = false;
		if(debug) std::cout << getName() << "::checkCondition()::condition not met. pos = [" << pos->x << " " << pos->y << "] is set to nextpos = [" << nextpos->x << " " << nextpos->y << "]\n";
		pos->x = nextpos->x;
		pos->y = nextpos->y;
	}
	return result;
}

int CueColorKernel::findmax(float* a, int size) { 
	int index = 0;
	float temp = a[0];
	for(int i=1;i<size;i++){
		if( a[i] > temp ){
			temp = a[i];
			index = i;
		}
	}
	return index;
}

/*
float CueColorKernel::similarityFunction(float* p, float* q, int size) {
	float result = 0.0;
	for(int i=0;i<size;i++){
		result += sqrt(p[i] * q[i]);
	}
	return result;	
}

float CueColorKernel::distance(float* p, float* q, int size) {
//	std::cout << getName() << "::distance()::Line 0\n";
	float sim = this->similarityFunction(p, q, size);
	float result = sqrt(1.0 - sim);
	return result;	
}

*/

