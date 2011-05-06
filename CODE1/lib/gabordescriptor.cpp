#include "gabordescriptor.h"
#include <sys/stat.h>

using namespace std;


GaborDescriptor::GaborDescriptor(std::string name): Module(name) {

	cvGrayImageIn.setModule(this);
	regionsIn.setModule(this);
	dataOut.setBuffer(&m_descriptor);

/*
//	mvec_filtersize = NULL;
	mdp_filtersReal = NULL;
	mdp_filtersImag = NULL;
	mdp_responsesReal = NULL;
	mdp_responsesImag = NULL;
//	mp_img32f = NULL;
	mp_rescaled32f = NULL;
	mp_filterimgreal = NULL;
	mp_filterimgimag = NULL;
*/
	mdp_filtersReal = NULL;
	mp_filterimgreal = NULL;

	m_offsetx = -1;
	m_offsety = -1;

	mp_GaborFilter = new GaborFilter;//OK

	m_norm = true;//OK

	mpc_ResizeBiggerImage = new Resize("ResizeBigger"+name, Resize::CROP, true);

	this->setDescriptorType("GJ");

	debug = false;//OK

	mp_rescaledimg = NULL;
}

GaborDescriptor::~GaborDescriptor() {

//	if(mvec_filtersize) delete [] mvec_filtersize;


	unsigned int temp = m_L*m_D;
	if(mdp_filtersReal) {
		for(unsigned int i = 0; i<temp;i++) cvReleaseMat(&(mdp_filtersReal[i]));
		delete [] mdp_filtersReal;
	}
/*	if(mdp_filtersImag) {
		for(unsigned int i = 0; i<temp;i++) cvReleaseMat(&(mdp_filtersImag[i]));
		delete [] mdp_filtersImag;
	}
	if(mdp_responsesReal) {
		for(unsigned int i = 0; i<temp;i++) cvReleaseMat(&(mdp_responsesReal[i]));
		delete [] mdp_responsesReal;
	}
	if(mdp_responsesImag) {
		for(unsigned int i = 0; i<temp;i++) cvReleaseMat(&(mdp_responsesImag[i]));
		delete [] mdp_responsesImag;
	}
*/

	if(mpc_ResizeBiggerImage) delete mpc_ResizeBiggerImage;
	
//	if(mp_img32f) cvReleaseImage(&mp_img32f);
	
//	if(mp_rescaled32f) cvReleaseImage(&mp_rescaled32f);
	
	if(mp_filterimgreal) delete mp_filterimgreal;

//	if(mp_filterimgimag) delete mp_filterimgimag;

	if(mp_GaborFilter) delete mp_GaborFilter;

	if(mp_rescaledimg) delete mp_rescaledimg;
}

void GaborDescriptor::execute() {

	if(debug) std::cout << getName() << "::execute()\n";
	
	if(m_init == false) { 
		std::cerr << getName() << "::execute()::ERROR::Call setParam() first!\n"; 
		m_descriptor.setCurrentSize(0);
		dataOut.out();
		return; 
	}

	CVImage* cvimg = cvGrayImageIn.getBuffer();
	//if(debug) std::cout << "cvimg.size = (" << cvimg->width << ", " << cvimg->height << ")\n";
	mp_regions = regionsIn.getBuffer();
	if(cvimg == NULL || mp_regions == NULL) { 
		std::cerr << getName() << "::execute()::ERROR::One or more input is NULL!\n"; 
		m_descriptor.setCurrentSize(0);
		dataOut.out();
		return; 
	}
	if(cvimg->cvMatType != CV_8UC1 && cvimg->cvMatType != CV_32FC1) { 
		std::cerr << getName() << "::execute()::ERROR::Only work with single channel gray-scale image!\n"; 
		m_descriptor.setCurrentSize(0);
		dataOut.out();
		return; 
	}

	if(m_offsetx == -1) {
		m_imagesize.width = cvimg->width;
		m_imagesize.height = cvimg->height;
		m_biggerimagesize.width = m_imagesize.width + (m_gap-1);
		m_biggerimagesize.height = m_imagesize.height + (m_gap-1);
		m_offsetx = cvRound((m_gap-1)/2);
		m_offsety = cvRound((m_gap-1)/2);
		if(debug) std::cout << getName() << "::execute()::m_gap = " << m_gap << "\n";
		if(debug) std::cout << getName() << "::execute()::m_imagesize = [" << m_imagesize.width << " " << m_imagesize.height << "]\n";
		if(debug) std::cout << getName() << "::execute()::m_biggerimagesize = [" << m_biggerimagesize.width << " " << m_biggerimagesize.height << "]\n";
		if(debug) std::cout << getName() << "::execute()::m_offsetx = " << m_offsetx << "\n";
		mpc_ResizeBiggerImage->fixOutputSize(m_biggerimagesize);
	}


	if(mp_regions->getCurrentSize() <= 0) {
		m_descriptor.setCurrentSize(0);
		dataOut.out();
		return;
	}

	float* descriptorsdata = m_descriptor.getData();
	if(descriptorsdata == NULL) {
		descriptorsdata = m_descriptor.allocate(mp_regions->getHeight(), m_L*m_D);
	}
	m_descriptor.setRegions(mp_regions);

	// copy the input image to a bigger image so that convolution at the boundaries doesnt cause trouble
	mpc_ResizeBiggerImage->cvImageIn.setBuffer(cvimg);
	mpc_ResizeBiggerImage->execute();
	CVImage* biggercvimg = mpc_ResizeBiggerImage->cvImageOut.getBuffer();

//	this->convertTo32F();
	// If image is 8bit convert to 32bit
	if(biggercvimg->cvMatType == CV_8UC1) { 
		mp_cvgrayimgflt = m_ConvertFlt.convert(biggercvimg);
	}
	else{
		mp_cvgrayimgflt = biggercvimg;
	}

	this->compute();
/*
	unsigned int regionssize = mp_regions->getCurrentSize();
	unsigned int regionswidth = mp_regions->getWidth();
	float* regionsdata = mp_regions->getData();
	
	if(debug) std::cout << getName() << "::execute()::regionssize = " << regionssize << "\n";

	unsigned int descwidth = m_descriptor.getWidth();

	unsigned int temp = m_L*m_D;
	for(unsigned int i = 0;i<regionssize;++i){
		this->rescaleImage( &(regionsdata[i*regionswidth]) );
		for(unsigned int j = 0; j < temp; ++j) {
			GDDataType result = this->conv2DPoint(mp_rescaled32f, j);
			descriptorsdata[i*descwidth + j] = (float)result;
		}
	}
*/	
	unsigned int regionssize = mp_regions->getCurrentSize();

	m_descriptor.setCurrentSize(regionssize);
	
	if(m_norm) m_descriptor.normalize();

//	if(debug) m_descriptor.print();
	
	dataOut.out();

}

void GaborDescriptor::setParam(unsigned int L, unsigned int D, float f, float kmax, float sigma, float N, bool norm, std::vector<int>* filtersize) {

	mpc_ResizeBiggerImage->debug = debug;

	m_L = L;
	m_D = D;
	m_f = (GDDataType)f;
	m_kmax = (GDDataType)kmax;
	m_sigma = (GDDataType)sigma;
	m_N = (GDDataType)N;
	m_norm = norm;

	if(filtersize) mvec_filtersize = *filtersize;
//	if(mvec_filtersize) delete [] mvec_filtersize;
//	mvec_filtersize = new int [m_L];
//	if(m_L == 5) {
	else {
		mvec_filtersize.clear();
		mvec_filtersize.push_back(55);
		mvec_filtersize.push_back(43);
		mvec_filtersize.push_back(35);
		mvec_filtersize.push_back(25);
		mvec_filtersize.push_back(19);
	}
//	else{
//		std::cerr << getName() << "::initialize()::ERROR::Currently only L = 5 is supported\n";
//	}

	m_gap = -1000;
	for(std::vector<int>::iterator iter = mvec_filtersize.begin(); iter != mvec_filtersize.end(); ++iter) {
		int temp = *iter;
		if( temp > m_gap) m_gap = temp;
	}

	mp_GaborFilter->setParam(L, D, f, kmax, sigma, N, &mvec_filtersize);

	mp_rescaledimg = new CVImage(cvSize(m_gap, m_gap), CV_32FC1, 0);


/*
	this->computeFilter();

	m_imagesize = imageSize;
	m_biggerimagesize.width = m_imagesize.width + (mvec_filtersize[0]-1);
	m_biggerimagesize.height = m_imagesize.height + (mvec_filtersize[0]-1);
	m_offsetx = cvRound((mvec_filtersize[0]-1)/2);
	m_offsety = cvRound((mvec_filtersize[0]-1)/2);
	mpc_ResizeBiggerImage->fixOutputSize(m_biggerimagesize);
*/
	m_offsetx = -1;
	m_offsety = -1;

	m_init = true;

}

/*
void GaborDescriptor::computeFilter() {

	unsigned int temp = m_L*m_D;
	if(mdp_filtersReal) {
		for(unsigned int i = 0; i<temp;i++) cvReleaseMat(&(mdp_filtersReal[i]));
		delete [] mdp_filtersReal;
	}
	if(mdp_filtersImag) {
		for(unsigned int i = 0; i<temp;i++) cvReleaseMat(&(mdp_filtersImag[i]));
		delete [] mdp_filtersImag;
	}
	if(mdp_responsesReal) {
		for(unsigned int i = 0; i<temp;i++) cvReleaseMat(&(mdp_responsesReal[i]));
		delete [] mdp_responsesReal;
	}
	if(mdp_responsesImag) {
		for(unsigned int i = 0; i<temp;i++) cvReleaseMat(&(mdp_responsesImag[i]));
		delete [] mdp_responsesImag;
	}

	mdp_filtersReal = new CvMat* [temp];
	mdp_filtersImag = new CvMat* [temp];
	mdp_responsesReal = new CvMat* [temp];
	mdp_responsesImag = new CvMat* [temp];

	for(unsigned int i = 0; i < m_L; i++) {
		int filtersize = mvec_filtersize[i];
		for(unsigned int j = 0; j < m_D; j++) {
			unsigned int tempindex = i*m_D+j;

			mdp_filtersReal[tempindex] = cvCreateMat(filtersize, filtersize, CV_32FC1);
			mdp_filtersImag[tempindex] = cvCreateMat(filtersize, filtersize, CV_32FC1);
			mdp_responsesReal[tempindex] = cvCreateMat(filtersize, filtersize, CV_32FC1);
			mdp_responsesImag[tempindex] = cvCreateMat(filtersize, filtersize, CV_32FC1);


//			mdp_filtersReal[tempindex] = cvCreateMat(filtersize, filtersize, CV_64FC1);
//			mdp_filtersImag[tempindex] = cvCreateMat(filtersize, filtersize, CV_64FC1);
//			mdp_responsesReal[tempindex] = cvCreateMat(filtersize, filtersize, CV_64FC1);
//			mdp_responsesImag[tempindex] = cvCreateMat(filtersize, filtersize, CV_64FC1);
		}
	}


    int count = 0;
    GDDataType kv, phimu, sigma2 = m_sigma*m_sigma;
    GDDataType kv1, kv2, k2, xx, xy, x2, kx;
    GDDataType temp3, temp2 = exp(-sigma2/2.0);
	int v2;
    for(unsigned int v = 0;v<m_L;v++){ // 0 ~ L-1
		v2 = m_L - v - 1;
		unsigned int filtersize = (unsigned int)(mvec_filtersize[v]);
        for(unsigned int mu = 0;mu<m_D;mu++){ // 0 ~ D-1
            phimu = (GDDataType)mu*3.1416 / (GDDataType)m_D;
			//kv = kmax / (GDDataType)pow((double)f, (double)v); // as in Jochen's paper
            //kv = 3.1416 * (GDDataType)pow((double)2.0, (double)-((v+2.0)/2.0));  // as in Wiskott's paper
			kv = 3.1416 * (GDDataType)pow((double)2.0, (double)-((m_f * v2+2.0)/2.0));  // Erik's CHANGE!!
            kv1 = cos(phimu) * kv;
            kv2 = sin(phimu) * kv;
            k2 = kv1*kv1 + kv2*kv2;

			CvMat* filterreal = mdp_filtersReal[count];
			CvMat* filterimag = mdp_filtersImag[count];

            for(unsigned int j=0;j<filtersize;j++){
                for(unsigned int i=0;i<filtersize;i++){

                    xx = ((GDDataType)i-((GDDataType)filtersize-1.0)/2.0)/m_N;
                    xy = ((GDDataType)j-((GDDataType)filtersize-1.0)/2.0)/m_N;
                    x2 = xx*xx + xy*xy;
                    kx = kv1*xx + kv2*xy;
                    temp3 = k2/sigma2*exp(-k2*x2/(2.0*sigma2));

					cvmSet( filterreal, j, i, temp3 * (cos(kx) - temp2) );
					cvmSet( filterimag, j, i, temp3 * sin(kx) );
                }
            }
            count++;
        }
    }

}


void GaborDescriptor::convertTo32F() {
	if(!mp_img32f) mp_img32f = cvCreateImage(m_biggerimagesize, IPL_DEPTH_32F, 1);
//	if(!mp_img32f) mp_img32f = cvCreateImage(m_biggerimagesize, IPL_DEPTH_64F, 1);

	// Convert single channel 8bit to 32bit image
	cvConvert(mp_biggercvimg->ipl, mp_img32f);
}
*/

void GaborDescriptor::compute() {

	if(debug) std::cout << "compute()::waiting...\n";

	float* regiondata = mp_regions->getData();
	unsigned int regionwidth = mp_regions->getWidth();
	float* descdata = m_descriptor.getData();
	unsigned int descwidth = m_descriptor.getWidth();

	unsigned int startindex = 0;
	unsigned int endindex = mp_regions->getCurrentSize() - 1;

	CVImage* cvimg = mp_cvgrayimgflt;
	IplImage* img = cvimg->ipl;
	CVImage* cvrescaledimg = mp_rescaledimg;
	IplImage* rescaledimg = cvrescaledimg->ipl;
	unsigned int temp = m_gap;
		
	GaborFilter* gabor = mp_GaborFilter;

	for(unsigned int i = startindex; i<=endindex; ++i) {

		// Rescale image (hopefully cvCopy and cvResize functions are thread-safe)
		unsigned int posx = cvRound(regiondata[i*regionwidth + 0]) + m_offsetx;
		unsigned int posy = cvRound(regiondata[i*regionwidth + 1]) + m_offsety;
		unsigned int width = cvRound(regiondata[i*regionwidth + 2]);
		unsigned int height = cvRound(regiondata[i*regionwidth + 3]);
		//float rotation = ellipse[4];
		unsigned int startx = posx - cvRound((width-1)/2);
		unsigned int starty = posy - cvRound((height-1)/2);
		CvRect inputrect;
		inputrect.x = startx;
		inputrect.y = starty;
		inputrect.width = width;
		inputrect.height = height;

		cvSetImageROI(img, inputrect);
		if(width == temp && height == temp) {
			// Simple copy operation
			cvCopy(img, rescaledimg, NULL);
		}
		else{
			// Crop-resize the image to match the filter size
			cvResize(img, rescaledimg, CV_INTER_LINEAR);
		}
		cvResetImageROI(img);

		// Convolution
		CvPoint pos;
		pos.x = (width-1)/2;
		pos.y = (height-1)/2;
		float* output = &(descdata[i*descwidth]);
		gabor->compute(cvrescaledimg, &pos, output);
	}		
		
}

CVImage* GaborDescriptor::getFilterImageReal(unsigned int index) {
	CvMat* filter = mdp_filtersReal[index];
	if(mp_filterimgreal) { delete mp_filterimgreal; mp_filterimgreal = NULL; }
	mp_filterimgreal = new CVImage(cvSize(filter->width, filter->height), CV_8UC1, 0);
	
	double min, max;
	cvMinMaxLoc( filter, &min, &max, NULL, NULL, NULL );

	if(max == min) { max = 1.0; min = 0.0; }

	char* data = mp_filterimgreal->ipl->imageData;
	unsigned int widthstep = mp_filterimgreal->ipl->widthStep;

	for(int j = 0;j<filter->height;j++) {
		for(int i = 0;i<filter->width;i++) {
			data[j*widthstep + i] = (char)((cvmGet(filter, j, i) - min)*255 / (max-min));
		}
	}

	return mp_filterimgreal;
}

/*
void GaborDescriptor::rescaleImage(float* ellipse) {
	unsigned int temp = mvec_filtersize[0];
	if(!mp_rescaled32f) mp_rescaled32f = cvCreateImage(cvSize(temp, temp), IPL_DEPTH_32F, 1);
//	if(!mp_rescaled32f) mp_rescaled32f = cvCreateImage(cvSize(temp, temp), IPL_DEPTH_64F, 1);

	unsigned int posx = cvRound(ellipse[0]) + m_offsetx;
	unsigned int posy = cvRound(ellipse[1]) + m_offsety;
	unsigned int width = cvRound(ellipse[2]);
	unsigned int height = cvRound(ellipse[3]);
	//float rotation = ellipse[4];

	//if(debug) std::cout << getName() << "::rescaleImage()::ellipse = [" << posx << "," << posy << "," << width << "," << height << "," << rotation << "]\n";

	unsigned int startx = posx - cvRound((width-1)/2);
	unsigned int starty = posy - cvRound((height-1)/2);

	//if(debug) std::cout << getName() << "::rescaleImage()::startx = " << startx << ", starty = " << starty << "\n";

	CvRect inputrect;
	inputrect.x = startx;
	inputrect.y = starty;
	inputrect.width = width;
	inputrect.height = height;

	IplImage* img = mp_cvgrayimgflt->ipl;

	cvSetImageROI(img, inputrect);
	if(width == temp && height == temp) {
		//if(debug) std::cout << getName() << "::rescaleImage()::Copying\n";
		// Simple copy operation
		cvCopy(img, mp_rescaled32f, NULL);
	}
	else{
		//if(debug) std::cout << getName() << "::rescaleImage()::Resizing\n";
		// Crop-resize the image to match the filter size
		cvResize(img, mp_rescaled32f, CV_INTER_LINEAR);
	}
	cvResetImageROI(img);

}

GDDataType GaborDescriptor::conv2DPoint(IplImage* input, unsigned int index) {

	//if(debug) std::cout << getName() << "::conv2DPoint()::index = " << index << "\n";

	//if(debug) std::cout << getName() << "::conv2DPoint()::input.size = (" << input->width << "," << input->height << ")\n";

	unsigned int L = cvFloor(index / m_D);

//	unsigned int temp = cvRound((mvec_filtersize[L]-1)/2);

	CvRect inputrect;
	inputrect.x = cvRound((input->width - mvec_filtersize[L])/2);
	inputrect.y = cvRound((input->height - mvec_filtersize[L])/2);
	inputrect.width = mvec_filtersize[L];
	inputrect.height = mvec_filtersize[L];

	//if(debug) std::cout << getName() << "::conv2DPoint()::inputrect = (" << inputrect.x << "," << inputrect.y << "," << inputrect.width << "," << inputrect.height << ")\n";

	cvSetImageROI(input, inputrect);

	CvMat* filterreal = mdp_filtersReal[index];
	CvMat* filterimag = mdp_filtersImag[index];
	CvMat* responsereal = mdp_responsesReal[index];
	CvMat* responseimag = mdp_responsesImag[index];

	//if(debug) std::cout << getName() << "::conv2DPoint()::filterreal.size = (" << filterreal->width << "," << filterreal->height << ")\n";
	//if(debug) std::cout << getName() << "::conv2DPoint()::responsereal.size = (" << responsereal->width << "," << responsereal->height << ")\n";

	cvMul(input, filterreal, responsereal, 1.0);
	cvMul(input, filterimag, responseimag, 1.0);

	CvScalar tempreal = cvSum(responsereal);
	CvScalar tempimag = cvSum(responseimag);
	GDDataType temp1 = (GDDataType)(tempreal.val[0]);
	GDDataType temp2 = (GDDataType)(tempimag.val[0]);
	GDDataType val = sqrt( pow(temp1, 2.0) + pow(temp2, 2.0) );

	cvResetImageROI(input);

	//if(debug) std::cout << getName() << "::conv2DPoint()::val = " << val << "\n";

	return val;
}

void GaborDescriptor::printFilterReal(unsigned int index) {
	CvMat* filter = mdp_filtersReal[index];
	std::cout << "=====================================\n";
	std::cout << " filter-real index " << index << "\n";
	std::cout << "size = [" << filter->height << " x " << filter->width << "]\n";
	std::cout << "[data] = [";
	for(int j = 0;j<filter->height;j++) {
		for(int i = 0;i<filter->width;i++) {
			std::cout << cvmGet(filter, j, i) << " ";
		}
		std::cout << ";\n";
	}
	std::cout << "]\n";
	std::cout << "=====================================\n";
}

void GaborDescriptor::printFilterImag(unsigned int index) {
	CvMat* filter = mdp_filtersImag[index];
	std::cout << "=====================================\n";
	std::cout << " filter-imag index " << index << "\n";
	std::cout << "size = [" << filter->height << " x " << filter->width << "]\n";
	std::cout << "[data] = [";
	for(int j = 0;j<filter->height;j++) {
		for(int i = 0;i<filter->width;i++) {
			std::cout << cvmGet(filter, j, i) << " ";
		}
		std::cout << ";\n";
	}
	std::cout << "]\n";
	std::cout << "=====================================\n";
}

CVImage* GaborDescriptor::getFilterImageReal(unsigned int index) {
	CvMat* filter = mdp_filtersReal[index];
	if(mp_filterimgreal) { delete mp_filterimgreal; mp_filterimgreal = NULL; }
	mp_filterimgreal = new CVImage(cvSize(filter->width, filter->height), CV_8UC1, 0);
	
	double min, max;
	cvMinMaxLoc( filter, &min, &max, NULL, NULL, NULL );

	if(max == min) { max = 1.0; min = 0.0; }

	char* data = mp_filterimgreal->ipl->imageData;
	unsigned int widthstep = mp_filterimgreal->ipl->widthStep;

	for(int j = 0;j<filter->height;j++) {
		for(int i = 0;i<filter->width;i++) {
			data[j*widthstep + i] = (char)((cvmGet(filter, j, i) - min)*255 / (max-min));
		}
	}

	return mp_filterimgreal;
}

CVImage* GaborDescriptor::getFilterImageImag(unsigned int index) {
	CvMat* filter = mdp_filtersImag[index];
	if(mp_filterimgimag) { delete mp_filterimgimag; mp_filterimgimag = NULL; }
	mp_filterimgimag = new CVImage(cvSize(filter->width, filter->height), CV_8UC1, 0);
	
	double min, max;
	cvMinMaxLoc( filter, &min, &max, NULL, NULL, NULL );

	if(max == min) { max = 1.0; min = 0.0; }

	char* data = mp_filterimgimag->ipl->imageData;
	unsigned int widthstep = mp_filterimgimag->ipl->widthStep;

	for(int j = 0;j<filter->height;j++) {
		for(int i = 0;i<filter->width;i++) {
			data[j*widthstep + i] = (char)((cvmGet(filter, j, i) - min)*255 / (max-min));
		}
	}

	return mp_filterimgimag;
}


CvMat* GaborDescriptor::getFilterReal(unsigned int index) {
	if(m_init == false) { std::cerr << getName() << "::getFilterReal()::ERROR::Call setParam() first!\n"; return NULL; }
	if(index >= (m_L*m_D) ) { std::cerr << getName() << "::getFilterReal()::ERROR::Index out of range!\n"; return NULL; }
	return mdp_filtersReal[index];
}

CvMat* GaborDescriptor::getFilterImag(unsigned int index) {
	if(m_init == false) { std::cerr << getName() << "::getFilterReal()::ERROR::Call setParam() first!\n"; return NULL; }
	if(index >= (m_L*m_D) ) { std::cerr << getName() << "::getFilterReal()::ERROR::Index out of range!\n"; return NULL; }
	return mdp_filtersImag[index];
}

void GaborDescriptor::writeToFile(std::string filename) {
	std::ofstream outFile(filename.c_str());
	if(outFile.is_open()) {
	
		m_descriptor.toStream(&outFile);
		
		CSCEllipticRegions* region = m_descriptor.getRegions();
		if(region) region->toStream(&outFile);
	
		outFile.close();
	}
	else{
		std::cerr << getName() << "::readFromFile()::ERROR opening file [" << filename << "] for writing!\n";
	}
}

bool GaborDescriptor::readFromFile(std::string filename) {
	struct stat buffer;	
	if(stat(filename.c_str(), &buffer)) { std::cerr << getName() << "::readFromFile()::File [" << filename << "] does not exist!\n"; dataOut.setBuffer(NULL); dataOut.out(); return false; }

	string type;
	std::ifstream inFile(filename.c_str());
	if (inFile.is_open()) {

		getline(inFile, type); 
		while( !inFile.eof() ) {
			
			if(type == "CDesc") {
				m_descriptor.fromStream(&inFile);
			}
			else if(type == "CEll"){
				CSCEllipticRegions* region = new CSCEllipticRegions;
				region->fromStream(&inFile);
				m_descriptor.setRegions(region);
				m_descriptor.setRegionsCreatedHere(true);
			}
			getline(inFile, type);
		}	
		inFile.close();
	
		dataOut.setBuffer(&m_descriptor);
		dataOut.out();

		return true;
	}
	else{
		std::cerr << getName() << "::readFromFile()::ERROR opening file [" << filename << "] for reading!\n";

		dataOut.setBuffer(NULL);
		dataOut.out();
		
		return false;
	}
}
*/

