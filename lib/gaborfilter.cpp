#include "gaborfilter.h"
#include "highgui.h"
//#include <sys/stat.h>

using namespace std;


GaborFilter::GaborFilter() {

	mdp_filtersreal = NULL;
	mdp_filtersimag = NULL;

	m_windowcreated = false;
	
	mp_displayreal = NULL;
	mp_displayimag = NULL;
	
	mdp_responsesreal = NULL;
	mdp_responsesimag = NULL;
	
	mp_result = NULL;
	mp_resultvector = NULL;
	
	m_computemode = ABSOLUTE;
	
	debug = false;

/*
	cvImageIn.setModule(this);
	regionsIn.setModule(this);
	dataOut.setBuffer(&m_descriptor);

	mp_filtersize = NULL;
	mdp_filtersReal = NULL;
	mdp_filtersImag = NULL;
	mdp_responsesReal = NULL;
	mdp_responsesImag = NULL;
	mp_img32f = NULL;
	mp_rescaled32f = NULL;
	mp_filterimgreal = NULL;
	mp_filterimgimag = NULL;
	
	m_norm = true;

	mpc_ResizeBiggerImage = new Resize("ResizeBigger"+name, Resize::CROP, true);

	this->setDescriptorType("GJ");

	debug = false;
*/
}

GaborFilter::~GaborFilter() {

	if(mdp_filtersreal) {
		for(unsigned int i = 0;i<m_LD;++i) delete mdp_filtersreal[i];
		delete [] mdp_filtersreal;
	}

	if(mdp_filtersimag) {
		for(unsigned int i = 0;i<m_LD;++i) delete mdp_filtersimag[i];
		delete [] mdp_filtersimag;
	}

	if(m_windowcreated) {
		cvDestroyWindow( m_windownamereal.c_str() );
		cvDestroyWindow( m_windownameimag.c_str() );
	}

	if(mp_displayreal) cvReleaseImage(&mp_displayreal);
	if(mp_displayimag) cvReleaseImage(&mp_displayimag);

	if(mdp_responsesreal) {
		for(unsigned int i = 0;i<m_L;++i) cvReleaseImage(&(mdp_responsesreal[i]));
		delete [] mdp_responsesreal;
	}
	if(mdp_responsesimag) {
		for(unsigned int i = 0;i<m_L;++i) cvReleaseImage(&(mdp_responsesimag[i]));
		delete [] mdp_responsesimag;
	}
	
	if(mp_result) delete [] mp_result;
	if(mp_resultvector) delete mp_resultvector;

/*
	if(mp_filtersize) delete [] mp_filtersize;

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

	if(mpc_ResizeBiggerImage) delete mpc_ResizeBiggerImage;
	
	if(mp_img32f) cvReleaseImage(&mp_img32f);
	
	if(mp_rescaled32f) cvReleaseImage(&mp_rescaled32f);
	
	if(mp_filterimgreal) delete mp_filterimgreal;

	if(mp_filterimgimag) delete mp_filterimgimag;
*/
}

/*
void GaborFilter::execute() {

	if(m_init == false) { 
		std::cerr << getName() << "::execute()::ERROR::Call setParam() first!\n"; 
		m_descriptor.setCurrentSize(0);
		dataOut.out();
		return; 
	}

	CVImage* cvimg = cvImageIn.getBuffer();
	//if(debug) std::cout << "cvimg.size = (" << cvimg->width << ", " << cvimg->height << ")\n";
	CSCEllipticRegions* regions = regionsIn.getBuffer();
	if(cvimg == NULL || regions == NULL) { 
		std::cerr << getName() << "::execute()::ERROR::One or more input is NULL!\n"; 
		m_descriptor.setCurrentSize(0);
		dataOut.out();
		return; 
	}
	if(cvimg->cvMatType != CV_8UC1) { 
		std::cerr << getName() << "::execute()::ERROR::Only work with single channel gray-scale image!\n"; 
		m_descriptor.setCurrentSize(0);
		dataOut.out();
		return; 
	}

	if(regions->getCurrentSize() <= 0) {
		m_descriptor.setCurrentSize(0);
		dataOut.out();
		return;
	}

	float* descriptorsdata = m_descriptor.getData();
	if(descriptorsdata == NULL) {
		descriptorsdata = m_descriptor.allocate(regions->getHeight(), m_L*m_D);
	}
	m_descriptor.setRegions(regions);

	// copy the input image to a bigger image so that convolution at the boundaries doesnt cause trouble
	mpc_ResizeBiggerImage->cvImageIn.setBuffer(cvimg);
	mpc_ResizeBiggerImage->execute();
	mp_biggercvimg = mpc_ResizeBiggerImage->cvImageOut.getBuffer();

	this->convertTo32F();

	unsigned int regionssize = regions->getCurrentSize();
	unsigned int regionswidth = regions->getWidth();
	float* regionsdata = regions->getData();

	unsigned int descwidth = m_descriptor.getWidth();

	unsigned int temp = m_L*m_D;
	for(unsigned int i = 0;i<regionssize;++i){
		this->rescaleImage( &(regionsdata[i*regionswidth]) );
		for(unsigned int j = 0; j < temp; ++j) {
			float result = this->conv2DPoint(mp_rescaled32f, j);
			descriptorsdata[i*descwidth + j] = (float)result;
		}
	}
	
	m_descriptor.setCurrentSize(regionssize);
	
	if(m_norm) m_descriptor.normalize();

	if(debug) m_descriptor.print();
	
	dataOut.out();

}
*/

void GaborFilter::setParam(unsigned int L, unsigned int D, float f, float kmax, float sigma, float N, std::vector<int>* filtersize) {

	m_L = L;
	m_D = D;
	m_LD = L*D;
	m_f = f;
	m_kmax = kmax;
	m_sigma = sigma;
	m_N = N;

	if(!filtersize) { 
		if(debug) std::cout << "GaborFilter::setParam()::filtersize = " << filtersize << "\n";
		mvec_filtersize.clear();
		mvec_filtersize.push_back(55);
		mvec_filtersize.push_back(43);
		mvec_filtersize.push_back(35);
		mvec_filtersize.push_back(25);
		mvec_filtersize.push_back(19);
	}
	else {
		mvec_filtersize = *filtersize;
	}
	
	if(debug) std::cout << "GaborFilter::setParam()::mvec_filtersize.size() = " << mvec_filtersize.size() << "\n";
	if(debug) std::cout << "GaborFilter::setParam()::m_L = " << m_L << "\n";

	if(mvec_filtersize.size() != m_L) { std::cerr << "GaborFilter::setParam()::ERROR::number of filtersizes do not match L\n"; return; }

	generatefilters();

/*
	if(mp_filtersize) delete [] mp_filtersize;
	mp_filtersize = new int [m_L];
	if(m_L == 5) {
		mp_filtersize[0] = 55;
		mp_filtersize[1] = 43;
		mp_filtersize[2] = 35;
		mp_filtersize[3] = 25;
		mp_filtersize[4] = 19;
	}
	else{
		std::cerr << getName() << "::initialize()::ERROR::Currently only L = 5 is supported\n";
	}

	this->computeFilter();

	m_biggerimagesize.width = m_imagesize.width + (mp_filtersize[0]-1);
	m_biggerimagesize.height = m_imagesize.height + (mp_filtersize[0]-1);
	m_offsetx = cvRound((mp_filtersize[0]-1)/2);
	m_offsety = cvRound((mp_filtersize[0]-1)/2);
	mpc_ResizeBiggerImage->fixOutputSize(m_biggerimagesize);

	m_init = true;
*/
}

void GaborFilter::generatefilters() {

	if(mdp_filtersreal) {
		for(unsigned int i = 0;i<m_LD;++i) delete mdp_filtersreal[i];
		delete [] mdp_filtersreal;
	}

	if(mdp_filtersimag) {
		for(unsigned int i = 0;i<m_LD;++i) delete mdp_filtersimag[i];
		delete [] mdp_filtersimag;
	}

	mdp_filtersreal = new CVImage* [m_LD];
	mdp_filtersimag = new CVImage* [m_LD];

/*
	unsigned int temp = m_L*m_D;
	if(mdp_filtersReal) {
		for(unsigned int i = 0; i<temp;i++) cvReleaseMat(&(mdp_filtersReal[i]));
		delete [] mdp_filtersReal;
	}
	if(mdp_filtersImag) {
		for(unsigned int i = 0; i<temp;i++) cvReleaseMat(&(mdp_filtersImag[i]));
		delete [] mdp_filtersImag;
	}
	mdp_filtersReal = new CvMat* [temp];
	mdp_filtersImag = new CvMat* [temp];

	if(mdp_responsesReal) {
		for(unsigned int i = 0; i<temp;i++) cvReleaseMat(&(mdp_responsesReal[i]));
		delete [] mdp_responsesReal;
	}
	if(mdp_responsesImag) {
		for(unsigned int i = 0; i<temp;i++) cvReleaseMat(&(mdp_responsesImag[i]));
		delete [] mdp_responsesImag;
	}
	mdp_responsesReal = new CvMat* [temp];
	mdp_responsesImag = new CvMat* [temp];
*/

	for(unsigned int i = 0; i < m_L; i++) {
		int size = mvec_filtersize[i];
		for(unsigned int j = 0; j < m_D; j++) {
			unsigned int tempindex = i*m_D+j;

			CvSize filtersize = cvSize(size, size);

			mdp_filtersreal[tempindex] = new CVImage(filtersize, CV_32FC1, 0);
			mdp_filtersimag[tempindex] = new CVImage(filtersize, CV_32FC1, 0);
			//mdp_responsesReal[tempindex] = cvCreateMat(filtersize, filtersize, CV_32FC1);
			//mdp_responsesImag[tempindex] = cvCreateMat(filtersize, filtersize, CV_32FC1);


//			mdp_filtersReal[tempindex] = cvCreateMat(filtersize, filtersize, CV_64FC1);
//			mdp_filtersImag[tempindex] = cvCreateMat(filtersize, filtersize, CV_64FC1);
//			mdp_responsesReal[tempindex] = cvCreateMat(filtersize, filtersize, CV_64FC1);
//			mdp_responsesImag[tempindex] = cvCreateMat(filtersize, filtersize, CV_64FC1);
		}
	}

    int count = 0;
    float kv, phimu, sigma2 = m_sigma*m_sigma;
    float kv1, kv2, k2, xx, xy, x2, kx;
    float temp3, temp2 = exp(-sigma2/2.0);
	int v2;
    for(unsigned int v = 0;v<m_L;v++){ // 0 ~ L-1
		v2 = m_L - v - 1;
		int size = mvec_filtersize[v];
        for(unsigned int mu = 0;mu<m_D;mu++){ // 0 ~ D-1
            phimu = (float)mu*3.1416 / (float)m_D;
			//kv = kmax / (float)pow((double)f, (double)v); // as in Jochen's paper
            //kv = 3.1416 * (float)pow((double)2.0, (double)-((v+2.0)/2.0));  // as in Wiskott's paper
			kv = 3.1416 * (float)pow((double)2.0, (double)-((m_f * v2+2.0)/2.0));  // Erik's CHANGE!!
            kv1 = cos(phimu) * kv;
            kv2 = sin(phimu) * kv;
            k2 = kv1*kv1 + kv2*kv2;

			IplImage* filterreal = mdp_filtersreal[count]->ipl;
			IplImage* filterimag = mdp_filtersimag[count]->ipl;

            for(int j=0;j<size;j++){
                for(int i=0;i<size;i++){

                    xx = ((float)i-((float)size-1.0)/2.0)/m_N;
                    xy = ((float)j-((float)size-1.0)/2.0)/m_N;
                    x2 = xx*xx + xy*xy;
                    kx = kv1*xx + kv2*xy;
                    temp3 = k2/sigma2*exp(-k2*x2/(2.0*sigma2));

					//cvmSet( filterreal, j, i, temp3 * (cos(kx) - temp2) );
					//cvmSet( filterimag, j, i, temp3 * sin(kx) );
					
					double val1 = temp3 * (cos(kx) - temp2);
					double val2 = temp3 * sin(kx);
					
					cvSetReal2D( filterreal, j, i, val1 );
					cvSetReal2D( filterimag, j, i, val2 );
                }
            }
            count++;
        }
    }

}

unsigned int GaborFilter::size() {
	std::cout << "sizeof(this) = " << sizeof(this) << "\n";
}

void GaborFilter::display(unsigned int index) {

	if(index >= m_LD) { std::cerr << "GaborFilter::display()::ERROR::index out of range!\n"; return; }

	m_windownamereal = "Gabor filter real";
	m_windownameimag = "Gabor filter imag";

	if(!m_windowcreated) {
		cvNamedWindow( m_windownamereal.c_str(), CV_WINDOW_AUTOSIZE);
		cvNamedWindow( m_windownameimag.c_str(), CV_WINDOW_AUTOSIZE);
		m_windowcreated = true;

		cvMoveWindow(m_windownamereal.c_str(), 50, 50);
		cvMoveWindow(m_windownameimag.c_str(), 250, 50);
	}
	
	IplImage* filterreal = mdp_filtersreal[index]->ipl;
	IplImage* filterimag = mdp_filtersimag[index]->ipl;

	if(mp_displayreal) cvReleaseImage(&mp_displayreal);
	if(mp_displayimag) cvReleaseImage(&mp_displayimag);

	mp_displayreal = cvCreateImage(cvSize(filterreal->width, filterreal->height), IPL_DEPTH_8U, 1);
	mp_displayimag = cvCreateImage(cvSize(filterimag->width, filterimag->height), IPL_DEPTH_8U, 1);

	double minval, maxval, scale, shift;
	cvMinMaxLoc(filterreal, &minval, &maxval, NULL, NULL);
	if(maxval != minval) scale = 255.0/(maxval - minval);
	else scale = 1.0;
	shift = - (minval) * scale;
//	std::cout << "minval = " << minval << ", maxval = " << maxval << ", scale = " << scale << ", shift = " << shift << "\n";
	cvConvertScale(filterreal, mp_displayreal, scale, shift );

//	cvMinMaxLoc(mp_displayreal, &minval, &maxval, NULL, NULL);
//	std::cout << "minval = " << minval << ", maxval = " << maxval << "\n";
	
	cvMinMaxLoc(filterimag, &minval, &maxval, NULL, NULL);
	if(maxval != minval) scale = 255.0/(maxval - minval);
	else scale = 1.0;
	shift = - (minval) * scale;
//	std::cout << "minval = " << minval << ", maxval = " << maxval << ", scale = " << scale << ", shift = " << shift << "\n";
	cvConvertScale(filterimag, mp_displayimag, scale, shift );

//	cvMinMaxLoc(mp_displayimag, &minval, &maxval, NULL, NULL);
//	std::cout << "minval = " << minval << ", maxval = " << maxval << "\n";

	cvShowImage(m_windownamereal.c_str(), mp_displayreal);
	cvShowImage(m_windownameimag.c_str(), mp_displayimag);

}

float GaborFilter::conv2DPointReal(CVImage* img, CvPoint* pos, unsigned int index) {

	float result = 0.0;

	if(!img || !pos) { std::cerr << "GaborFilter::conv2DPointReal()::ERROR::One of input is NULL!\n"; return result; }

	if(index >= m_LD) { std::cerr << "GaborFilter::conv2DPointReal()::ERROR::index out of range!\n"; return result; }

	CVImage* imgflt;
	if(img->cvMatType == CV_8UC1) {
		//if(debug) std::cout << "GaborFilter::conv2DPointReal()::converting 8bit unsigned img to 32bit float\n";
		imgflt = m_ConvertFlt.convert(img);
	}
	else if(img->cvMatType == CV_32FC1) {
		imgflt = img;
	}
	else {
		std::cerr << "GaborFilter::conv2DPointReal()::ERROR::Unsupported img type!\n"; return result;
	}

	//if(debug) std::cout << "GaborFilter::conv2DPointReal()::img = " << img << "\n";
//	if(debug) std::cout << "GaborFilter::conv2DPointReal()::imgflt = " << imgflt << "\n";
//	if(debug) std::cout << "GaborFilter::conv2DPointReal()::pos = [" << pos->x << " " << pos->y << "]\n";

	if(!mdp_responsesreal) {
		//if(debug) std::cout << "GaborFilter::conv2DPointReal()::m_L = " << m_L << "\n";
		mdp_responsesreal = new IplImage* [m_L];
		for(unsigned int i = 0; i < m_L; i++) {
			int size = mvec_filtersize[i];
			//if(debug) std::cout << "GaborFilter::conv2DPointReal()::i = " << i << ", size = " << size << "\n";
			CvSize filtersize = cvSize(size, size);
			mdp_responsesreal[i] = cvCreateImage(filtersize, IPL_DEPTH_32F, 1);
		}
	}

	unsigned int L = cvFloor(index / m_D);
	//if(debug) std::cout << "GaborFilter::conv2DPointReal()::L = " << L << "\n";

	IplImage* input = imgflt->ipl;
	IplImage* filter = mdp_filtersreal[index]->ipl;
	IplImage* response = mdp_responsesreal[L];

	CvRect inputrect;
	int size = mvec_filtersize[L];
	int posx = pos->x - cvRound((size-1)/2);
	int posy = pos->y - cvRound((size-1)/2);
	int endx = posx + size;
	int endy = posy + size;
	inputrect.x = posx > 0 ? posx : 0;
	inputrect.y = posy > 0 ? posy : 0;

	if(endx >= input->width) {
		int diff = endx - input->width + 1;
		inputrect.x -= diff;
	}
	if(endy >= input->height) {
		int diff = endy - input->height + 1;
		inputrect.y -= diff;
	}

	inputrect.width = size;
	inputrect.height = size;

//	if(debug) std::cout << "GaborFilter::conv2DPointReal()::inputrect = [" << inputrect.x << " " << inputrect.y << " " << inputrect.width << " " << inputrect.height << "]\n";
//	if(debug) std::cout << "GaborFilter::conv2DPointReal()::filter size = [" << filter->width << " " << filter->height << "]\n";
//	if(debug) std::cout << "GaborFilter::conv2DPointReal()::response size = [" << response->width << " " << response->height << "]\n";
//	if(debug) std::cout << "GaborFilter::conv2DPointReal()::input->depth = " << input->depth << ", input->nChannels = " << input->nChannels << "\n";
//	if(debug) std::cout << "GaborFilter::conv2DPointReal()::filter->depth = " << filter->depth << ", filter->nChannels = " << filter->nChannels << "\n";
//	if(debug) std::cout << "GaborFilter::conv2DPointReal()::response->depth = " << response->depth << ", response->nChannels = " << response->nChannels << "\n";

	cvSetImageROI(input, inputrect);

	cvMul(input, filter, response, 1.0);
	
	cvResetImageROI(input);

	CvScalar temp = cvSum(response);

	result = (float)(temp.val[0]);

//	if(debug) std::cout << "GaborFilter::conv2DPointReal()::result = " << result << "\n";

//	if(debug) std::cout << "GaborFilter::conv2DPointReal() complete\n";
	return result;
}

float GaborFilter::conv2DPointImag(CVImage* img, CvPoint* pos, unsigned int index) {
	float result = 0.0;

	if(!img || !pos) { std::cerr << "GaborFilter::conv2DPointReal()::ERROR::One of input is NULL!\n"; return result; }

	if(index >= m_LD) { std::cerr << "GaborFilter::conv2DPointReal()::ERROR::index out of range!\n"; return result; }

	//if(debug) std::cout << "GaborFilter::conv2DPointReal()::img = " << img << "\n";

	CVImage* imgflt;

	if(img->cvMatType == CV_8UC1) {
		//if(debug) std::cout << "GaborFilter::conv2DPointReal()::converting 8bit unsigned img to 32bit float\n";
		imgflt = m_ConvertFlt.convert(img);
	}
	else if(img->cvMatType == CV_32FC1) {
		imgflt = img;
	}
	else {
		std::cerr << "GaborFilter::conv2DPointReal()::ERROR::Unsupported img type!\n"; return result;
	}

	//if(debug) std::cout << "GaborFilter::conv2DPointReal()::imgflt = " << imgflt << "\n";

	if(!mdp_responsesimag) {
		//if(debug) std::cout << "GaborFilter::conv2DPointReal()::m_L = " << m_L << "\n";
		mdp_responsesimag = new IplImage* [m_L];
		for(unsigned int i = 0; i < m_L; i++) {
			int size = mvec_filtersize[i];
			//if(debug) std::cout << "GaborFilter::conv2DPointReal()::i = " << i << ", size = " << size << "\n";
			CvSize filtersize = cvSize(size, size);
			mdp_responsesimag[i] = cvCreateImage(filtersize, IPL_DEPTH_32F, 1);
		}
	}

	unsigned int L = cvFloor(index / m_D);
	//if(debug) std::cout << "GaborFilter::conv2DPointReal()::L = " << L << "\n";

	IplImage* input = imgflt->ipl;
	IplImage* filter = mdp_filtersimag[index]->ipl;
	IplImage* response = mdp_responsesimag[L];

	CvRect inputrect;
	int size = mvec_filtersize[L];
	int posx = pos->x - cvRound((size-1)/2);
	int posy = pos->y - cvRound((size-1)/2);
	int endx = posx + size;
	int endy = posy + size;
	inputrect.x = posx > 0 ? posx : 0;
	inputrect.y = posy > 0 ? posy : 0;

	if(endx >= input->width) {
		int diff = endx - input->width + 1;
		inputrect.x -= diff;
	}
	if(endy >= input->height) {
		int diff = endy - input->height + 1;
		inputrect.y -= diff;
	}

	inputrect.width = size;
	inputrect.height = size;

//	if(debug) std::cout << "GaborFilter::conv2DPointImag()::inputrect = [" << inputrect.x << " " << inputrect.y << " " << inputrect.width << " " << inputrect.height << "]\n";
//	if(debug) std::cout << "GaborFilter::conv2DPointImag()::filter size = [" << filter->width << " " << filter->height << "]\n";
//	if(debug) std::cout << "GaborFilter::conv2DPointImag()::response size = [" << response->width << " " << response->height << "]\n";
//	if(debug) std::cout << "GaborFilter::conv2DPointImag()::input->depth = " << input->depth << ", input->nChannels = " << input->nChannels << "\n";
//	if(debug) std::cout << "GaborFilter::conv2DPointImag()::filter->depth = " << filter->depth << ", filter->nChannels = " << filter->nChannels << "\n";
//	if(debug) std::cout << "GaborFilter::conv2DPointImag()::response->depth = " << response->depth << ", response->nChannels = " << response->nChannels << "\n";

	cvSetImageROI(input, inputrect);

	cvMul(input, filter, response, 1.0);
	
	cvResetImageROI(input);

	CvScalar temp = cvSum(response);

	result = (float)(temp.val[0]);

//	if(debug) std::cout << "GaborFilter::conv2DPointImag() complete\n";
	return result;
}

float GaborFilter::conv2DPointMag(CVImage* img, CvPoint* pos, unsigned int index) {
	float realval = conv2DPointReal(img, pos, index);
	float imagval = conv2DPointImag(img, pos, index);
	float mag = sqrt( pow(realval, 2.0f) + pow(imagval, 2.0f) );
	return mag;
}

float GaborFilter::conv2DPointPha(CVImage* img, CvPoint* pos, unsigned int index) {
	float realval = conv2DPointReal(img, pos, index);
	float imagval = conv2DPointImag(img, pos, index);
	float pha = atan2(imagval, realval);
	return pha;
}

float* GaborFilter::compute(CVImage* img, CvPoint* pos, float* dst) {

	if(!img || !pos) { std::cerr << "GaborFilter::compute()::ERROR::One of input is NULL!\n"; return NULL; }
	
	CVImage* imgflt = NULL;
	if(img->cvMatType == CV_8UC1) {
		imgflt = m_ConvertFlt.convert(img);
	}
	else if(img->cvMatType == CV_32FC1) {
		imgflt = img;
	}
	else {
		std::cerr << "GaborFilter::conv2DPointImag()::ERROR::Unsupported img type!\n"; return NULL;
	}

	if(!dst) { std::cerr << "GaborFilter::compute()::ERROR::One of input is NULL!\n"; return NULL; }
	float* result = dst;

	for(unsigned int i = 0;i<m_LD;++i) {
		float tempreal = conv2DPointReal(imgflt, pos, i);
		float tempimag = conv2DPointImag(imgflt, pos, i);
		if(m_computemode == ABSOLUTE) {
			result[i] = sqrt(pow(tempreal, 2.0f)+pow(tempimag, 2.0f));
		}
		else if(m_computemode == COMPLEX) {
			result[i*2+0] = tempreal;
			result[i*2+1] = tempimag;
		}
		else  {
			result[i*2+0] = sqrt(pow(tempreal, 2.0f)+pow(tempimag, 2.0f));
			result[i*2+1] = atan2(tempimag, tempreal);
		}
	}

	return result;
}

float* GaborFilter::computereal(CVImage* img, CvPoint* pos, float* dst) {
	if(!img || !pos) { std::cerr << "GaborFilter::computereal()::ERROR::One of input is NULL!\n"; return NULL; }

	CVImage* imgflt = NULL;
	if(img->cvMatType == CV_8UC1) {
		imgflt = m_ConvertFlt.convert(img);
	}
	else if(img->cvMatType == CV_32FC1) {
		imgflt = img;
	}
	else {
		std::cerr << "GaborFilter::conv2DPointImag()::ERROR::Unsupported img type!\n"; return NULL;
	}

	if(!dst) { std::cerr << "GaborFilter::computereal()::ERROR::One of input is NULL!\n"; return NULL; }
	float* result = dst;

	for(unsigned int i = 0;i<m_LD;++i) {
		result[i] = conv2DPointReal(imgflt, pos, i);
	}

	return result;
}

float* GaborFilter::computeimag(CVImage* img, CvPoint* pos, float* dst) {

	if(!img || !pos) { std::cerr << "GaborFilter::computeimag()::ERROR::One of input is NULL!\n"; return NULL; }
/*	
	float* result = NULL;
	if(!dst) {
		if(!mp_result) mp_result = new float [m_LD];
		result = mp_result;
	}
	else {
		result = dst;
	}
*/
	CVImage* imgflt = NULL;
	if(img->cvMatType == CV_8UC1) {
		imgflt = m_ConvertFlt.convert(img);
	}
	else if(img->cvMatType == CV_32FC1) {
		imgflt = img;
	}
	else {
		std::cerr << "GaborFilter::conv2DPointImag()::ERROR::Unsupported img type!\n"; return NULL;
	}

	if(!dst) { std::cerr << "GaborFilter::computeimag()::ERROR::One of input is NULL!\n"; return NULL; }
	float* result = dst;

	if(debug) std::cout << "GaborFilter::compute()::result = " << result << "\n";

	for(unsigned int i = 0;i<m_LD;++i) {
		result[i] = conv2DPointImag(imgflt, pos, i);
	}

	return result;
}

float* GaborFilter::computemag(CVImage* img, CvPoint* pos, float* dst) {

	if(!img || !pos) { std::cerr << "GaborFilter::computemag()::ERROR::One of input is NULL!\n"; return NULL; }

	CVImage* imgflt = NULL;
	if(img->cvMatType == CV_8UC1) {
		imgflt = m_ConvertFlt.convert(img);
	}
	else if(img->cvMatType == CV_32FC1) {
		imgflt = img;
	}
	else {
		std::cerr << "GaborFilter::computemag()::ERROR::Unsupported img type!\n"; return NULL;
	}

	if(!dst) { std::cerr << "GaborFilter::computemag()::ERROR::One of input is NULL!\n"; return NULL; }
	float* result = dst;

	for(unsigned int i = 0;i<m_LD;++i) {
		result[i] = conv2DPointMag(imgflt, pos, i);
	}

	return result;
}

float* GaborFilter::computepha(CVImage* img, CvPoint* pos, float* dst) {

	if(!img || !pos) { std::cerr << "GaborFilter::computepha()::ERROR::One of input is NULL!\n"; return NULL; }

	CVImage* imgflt = NULL;
	if(img->cvMatType == CV_8UC1) {
		imgflt = m_ConvertFlt.convert(img);
	}
	else if(img->cvMatType == CV_32FC1) {
		imgflt = img;
	}
	else {
		std::cerr << "GaborFilter::computepha()::ERROR::Unsupported img type!\n"; return NULL;
	}

	if(!dst) { std::cerr << "GaborFilter::computepha()::ERROR::One of input is NULL!\n"; return NULL; }
	float* result = dst;

	for(unsigned int i = 0;i<m_LD;++i) {
		result[i] = conv2DPointPha(imgflt, pos, i);
	}

	return result;
}

CSCVector* GaborFilter::compute(CVImage* img, CvPoint* pos, CSCVector* dst) {
	if(!dst) { std::cerr << "GaborFilter::compute()::ERROR::One of input is NULL!\n"; return NULL; }
	float* data = dst->getData();
	if(debug) std::cout << "GaborFilter::compute()::img = " << img << "\n";
	if(debug) std::cout << "GaborFilter::compute()::pos = [" << pos->x << " " << pos->y << "]\n";
	if(debug) std::cout << "GaborFilter::compute()::dst = " << dst << "\n";
	if(debug) std::cout << "GaborFilter::compute()::dst->data = " << data << "\n";
	compute(img, pos, data);
	return dst;
}

CSCVector* GaborFilter::computereal(CVImage* img, CvPoint* pos, CSCVector* dst) {
	if(!dst) { std::cerr << "GaborFilter::computereal()::ERROR::One of input is NULL!\n"; return NULL; }
	float* data = dst->getData();
	computereal(img, pos, data);
	return dst;
}

CSCVector* GaborFilter::computeimag(CVImage* img, CvPoint* pos, CSCVector* dst) {
	if(!dst) { std::cerr << "GaborFilter::computeimag()::ERROR::One of input is NULL!\n"; return NULL; }
	float* data = dst->getData();
	computeimag(img, pos, data);
	return dst;
}

CSCVector* GaborFilter::computemag(CVImage* img, CvPoint* pos, CSCVector* dst) {
	if(!dst) { std::cerr << "GaborFilter::computemag()::ERROR::One of input is NULL!\n"; return NULL; }
	float* data = dst->getData();
	computemag(img, pos, data);
	return dst;
}

CSCVector* GaborFilter::computepha(CVImage* img, CvPoint* pos, CSCVector* dst) {
	if(!dst) { std::cerr << "GaborFilter::computepha()::ERROR::One of input is NULL!\n"; return NULL; }
	float* data = dst->getData();
	computepha(img, pos, data);
	return dst;
}

void GaborFilter::printfilters() {
    for(unsigned int i = 0;i<m_L*m_D;i++){ // 0 ~ L-1
		IplImage* filterreal = mdp_filtersreal[i]->ipl;
		IplImage* filterimag = mdp_filtersimag[i]->ipl;
		for(int j=0;j<filterreal->height;j++){
			for(int i=0;i<filterreal->width;i++){
				double realval = cvGetReal2D( filterreal, j, i );
				double imagval = cvGetReal2D( filterimag, j, i );
				std::cout << realval << "+i" << imagval << " ";
            }
			std::cout << "\n";
        }
    }
}

/*

void GaborFilter::convertTo32F() {
	if(!mp_img32f) mp_img32f = cvCreateImage(m_biggerimagesize, IPL_DEPTH_32F, 1);
//	if(!mp_img32f) mp_img32f = cvCreateImage(m_biggerimagesize, IPL_DEPTH_64F, 1);

	// Convert single channel 8bit to 32bit image
	cvConvert(mp_biggercvimg->ipl, mp_img32f);
}

void GaborFilter::rescaleImage(float* ellipse) {
	unsigned int temp = mp_filtersize[0];
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

	cvSetImageROI(mp_img32f, inputrect);
	if(width == temp && height == temp) {
		//if(debug) std::cout << getName() << "::rescaleImage()::Copying\n";
		// Simple copy operation
		cvCopy(mp_img32f, mp_rescaled32f, NULL);
	}
	else{
		//if(debug) std::cout << getName() << "::rescaleImage()::Resizing\n";
		// Crop-resize the image to match the filter size
		cvResize(mp_img32f, mp_rescaled32f, CV_INTER_LINEAR);
	}
	cvResetImageROI(mp_img32f);

}

void GaborFilter::printFilterReal(unsigned int index) {
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

void GaborFilter::printFilterImag(unsigned int index) {
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

CVImage* GaborFilter::getFilterImageReal(unsigned int index) {
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

CVImage* GaborFilter::getFilterImageImag(unsigned int index) {
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


CvMat* GaborFilter::getFilterReal(unsigned int index) {
	if(m_init == false) { std::cerr << getName() << "::getFilterReal()::ERROR::Call setParam() first!\n"; return NULL; }
	if(index >= (m_L*m_D) ) { std::cerr << getName() << "::getFilterReal()::ERROR::Index out of range!\n"; return NULL; }
	return mdp_filtersReal[index];
}

CvMat* GaborFilter::getFilterImag(unsigned int index) {
	if(m_init == false) { std::cerr << getName() << "::getFilterReal()::ERROR::Call setParam() first!\n"; return NULL; }
	if(index >= (m_L*m_D) ) { std::cerr << getName() << "::getFilterReal()::ERROR::Index out of range!\n"; return NULL; }
	return mdp_filtersImag[index];
}

void GaborFilter::writeToFile(std::string filename) {
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

bool GaborFilter::readFromFile(std::string filename) {
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

