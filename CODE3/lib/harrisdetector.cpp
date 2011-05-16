#include "harrisdetector.h"

using namespace std;


HarrisDetector::HarrisDetector(std::string name) : Module(name) {
	std::cout << "\nConstructing " << getName() << "...";

	cvGrayImageIn.setModule(this);
	//maskIn.setModule(this);

	mp_regions = NULL;
	mp_eig_image = NULL;
	mp_temp_image = NULL;
	mp_corners = NULL;

	m_block_size = 3; // Size of the averaging block, passed to underlying cvCornerMinEigenVal or cvCornerHarris used by the function
	m_use_harris = 1; // if 0, instead of cvCornerHarris, cvCornerMinEigenVal is used
//	m_use_harris = 0; // if 0, instead of cvCornerHarris, cvCornerMinEigenVal is used
	m_k = 0.04; // Free parameter of Harris detector; used only if use_harris≠0

	m_fixBlobSize = true;
	m_fixed_blob_size = 27.5;

//	this->setParam(2000, 0.02, 4, cvSize(640, 480));

	debug = false;
	
	std::cout << "finished.";
}

HarrisDetector::~HarrisDetector() {
	std::cout << "\nDestructing " << getName() << "...";

	if(mp_regions) delete mp_regions;

	if(mp_eig_image) cvReleaseImage(&mp_eig_image);
	if(mp_temp_image) cvReleaseImage(&mp_temp_image);
	if(mp_corners) delete [] mp_corners;

	std::cout << "finished.";
}

void HarrisDetector::setParam(unsigned int cornerCount, double qualityLevel, double minDistance) {
	m_cornerCount = cornerCount; //maximum number of corners returned
	m_qualityLevel = qualityLevel;  //threshold for corners
	m_minDistance = minDistance; // minimum distance between corners

	if(mp_corners) delete [] mp_corners;
	mp_corners = new CvPoint2D32f[m_cornerCount];

	if(mp_regions) {
		if(mp_regions->getHeight() != m_cornerCount) {
			delete mp_regions;
			mp_regions = NULL;
		}
	}
	if(!mp_regions) mp_regions = new CSCEllipticRegions(m_cornerCount, 5);
	regionsOut.setBuffer(mp_regions);

	if(mp_eig_image) cvReleaseImage(&mp_eig_image);
	mp_eig_image = NULL;

	if(mp_temp_image) cvReleaseImage(&mp_temp_image);
	mp_temp_image = NULL;

}

void HarrisDetector::execute() {

	CVImage* cvimg = cvGrayImageIn.getBuffer();
	if(!cvimg) { std::cerr << getName() << "::execute()::ERROR::cvGrayImageIn is NULL!\n"; return; }
	if(cvimg->nChannels != 1) { std::cerr << getName() << "::execute()::ERROR::cvGrayImageIn must be single-channel 8-bit image!\n"; return; }

	if(!mp_eig_image) {
		CvSize imgsize = cvSize(cvimg->width, cvimg->height);
		mp_eig_image = cvCreateImage(imgsize, IPL_DEPTH_32F, 1);
		mp_temp_image = cvCreateImage(imgsize, IPL_DEPTH_32F, 1);
	}

	IplImage* img = cvimg->ipl;

	CvRect* newroi = roiIn.getBuffer();
	if(newroi) {
		//if(debug) {
		//	std::cout << getName() << "::execute()::roi = [" << newroi->x << "," << newroi->y << "," << newroi->width << "," << newroi->height << "]\n";
		//}
		cvSetImageROI(img, *newroi);
		cvSetImageROI(mp_eig_image, *newroi);
		cvSetImageROI(mp_temp_image, *newroi);
	}

	int cornerCount = m_cornerCount;
//	std::cout << getName() << "::execute()::cornerCount1 = " << cornerCount << "\n";
	cvGoodFeaturesToTrack( img, mp_eig_image, mp_temp_image, mp_corners, &(cornerCount), m_qualityLevel, m_minDistance);
//	std::cout << getName() << "::execute()::cornerCount2 = " << cornerCount << "\n";

	if(newroi) {
		cvResetImageROI(img);
		cvResetImageROI(mp_eig_image);
		cvResetImageROI(mp_temp_image);
	}

//	if(debug) std::cout << getName() << "::execute()::cornerCount2 = " << cornerCount << "\n";

	if(cornerCount <= 0) {
		mp_regions->setCurrentSize(0);
		regionsOut.out();
		return;
	}

	float* mat = mp_regions->getData();
	unsigned int width = mp_regions->getWidth();
	for(int j = 0; j < cornerCount; j++) {
		unsigned int temp = j*width;

		if(newroi) {
			mat[temp] = mp_corners[j].x + newroi->x;
			mat[temp + 1] = mp_corners[j].y + newroi->y;
		}
		else{
			mat[temp] = mp_corners[j].x;
			mat[temp + 1] = mp_corners[j].y;
		}

		if(m_fixBlobSize) {
			mat[temp + 2] = m_fixed_blob_size;
			mat[temp + 3] = m_fixed_blob_size;
		}
		else {
			//std::cout << getName() << "::execute()::Currently HarrisDetector only detects fixed size blobs\n";
			mat[temp + 2] = m_fixed_blob_size;
			mat[temp + 3] = m_fixed_blob_size;
		}
		mat[temp + 4] = 0.0f;
	}
	mp_regions->setCurrentSize(cornerCount);
	regionsOut.out();
	
	//if(debug) std::cout << getName() << "::execute()::cornerCount = " << cornerCount << "\n";

}
