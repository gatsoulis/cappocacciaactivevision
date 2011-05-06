#include "cvimage.h"
//#include <iostream>
//#include "highgui.h"
#include "serializer.h"
//#include <float.h>
//#include <cmath>
#include <fstream>

using namespace std;


CVImage::CVImage() {
	width = 2;
	height = 2;
	zeroPad = 0;
	resetROI();
	cvMatType = CV_8UC1;
	data = NULL;
	ipl = NULL;
	mat = NULL;
	allocateMemory();
}

/*
CVImage::CVImage(IppiSize sizeVar, int typeVar, int zPad) {
	zeroPad = zPad;
	iSize = sizeVar;
	cvMatType = typeVar;
	resetROI();
	allocateMemory();
}
*/

CVImage::CVImage(CvSize sizeVar, int typeVar, int zPad) {
	zeroPad = zPad; 
	cSize = sizeVar;
	cvMatType = typeVar;
	data = NULL;
	ipl = NULL;
	mat = NULL;
	resetROI();
	allocateMemory();
}

CVImage::CVImage(int widthVar, int heightVar, int typeVar, int zPad) {
	zeroPad = zPad; 
	width = widthVar;
	height = heightVar;
	cvMatType = typeVar;
	data = NULL;
	ipl = NULL;
	mat = NULL;
	resetROI();
	allocateMemory();
}

CVImage::~CVImage() {
	deallocateMemory();
}

void CVImage::deallocateMemory() {
	if(ipl) cvReleaseImage(&ipl);
	ipl = NULL;
	data = NULL;
	if(mat) cvReleaseMatHeader(&mat);
	mat = NULL;
	
/*
	if(ipl) cvReleaseImageHeader(&ipl);
	if(data) {
		data8u -= zeroPad * byteDepth() * nChannels + zeroPad * step; 
		ippiFree(data);
	}
*/
}
    
int CVImage::allocateMemory() {

    switch(cvMatType) {
    	default:
		    std::cerr << "ERROR:  Cannot allocate image of that type! using CV_8UC1 instead!";
	    	cvMatType = CV_8UC1;
			break;

		case CV_8UC1:
			//data8u = ippiMalloc_8u_C1(width+zeroPad*2, height+zeroPad*2,&step);
			nChannels = 1;
			iplDepth = IPL_DEPTH_8U;
			break;
			
		case CV_8UC2:
			//data8u = ippiMalloc_8u_C2(width+zeroPad*2, height+zeroPad*2,&step);
	    	nChannels = 2;	
			iplDepth = IPL_DEPTH_8U;
			break;

		case CV_8UC3:
	    	//data8u = ippiMalloc_8u_C3(width+zeroPad*2, height+zeroPad*2,&step);
			nChannels = 3;
			iplDepth = IPL_DEPTH_8U;
			break;

		case CV_8UC4:
	    	//data8u = ippiMalloc_8u_C4(width+zeroPad*2, height+zeroPad*2,&step);
			nChannels = 4;
			iplDepth = IPL_DEPTH_8U;
			break;

		case CV_16SC4:
			//data16s = ippiMalloc_16s_C4(width+zeroPad*2, height+zeroPad*2,&step);
			nChannels = 4;
			iplDepth = IPL_DEPTH_16S;
			break;
	
		case CV_32SC1:
			//data32s = ippiMalloc_32s_C1(width+zeroPad*2, height+zeroPad*2,&step);
			nChannels = 1;
			iplDepth = IPL_DEPTH_32S;
			break;

		case CV_32SC2:
			//data32s = ippiMalloc_32s_C2(width+zeroPad*2, height+zeroPad*2,&step);
			nChannels = 2;
			iplDepth = IPL_DEPTH_32S;
			break;

		case CV_32SC3:
			//data32s = ippiMalloc_32s_C3(width+zeroPad*2, height+zeroPad*2,&step);
			nChannels = 3;
			iplDepth = IPL_DEPTH_32S;
			break;

		case CV_32SC4:
			//data32s = ippiMalloc_32s_C4(width+zeroPad*2, height+zeroPad*2,&step);
			nChannels = 4;
			iplDepth = IPL_DEPTH_32S;
			break;
	
		case CV_32FC1:
			//data32f = ippiMalloc_32f_C1(width+zeroPad*2, height+zeroPad*2,&step);
			nChannels = 1;
			iplDepth = IPL_DEPTH_32F;
			break;

		case CV_32FC2:
			//data32f = ippiMalloc_32f_C2(width+zeroPad*2, height+zeroPad*2,&step);
			nChannels = 2;
			iplDepth = IPL_DEPTH_32F;
			break;

		case CV_32FC3:
			//data32f = ippiMalloc_32f_C3(width+zeroPad*2, height+zeroPad*2,&step);
			nChannels = 3;
			iplDepth = IPL_DEPTH_32F;
			break;

		case CV_32FC4:
			//data32f = ippiMalloc_32f_C4(width+zeroPad*2, height+zeroPad*2,&step);
			nChannels = 4;
			iplDepth = IPL_DEPTH_32F;
			break;
	
	}

	if(ipl) cvReleaseImage(&ipl);
	ipl = cvCreateImage( cSize, iplDepth, nChannels);

	data = ipl->imageData;
	step = ipl->widthStep;

	if(mat) cvReleaseMatHeader(&mat);
   	mat = cvCreateMatHeader(height, width, cvMatType);

	cvSetData(mat, data, step);

	return 0;
/*    
	if(data) {
		data8u += zeroPad * byteDepth() * nChannels + zeroPad * step;
		//SETS image to zero if using zeropad.  Must Occur after pointer is moved
		if(zeroPad)  {
			zeroOut();	
		}
    	mat = cvCreateMatHeader(height, width, cvMatType);
    	ipl = cvCreateImageHeader(cSize, iplDepth, nChannels);
		cvSetData(mat, data, step);
		cvSetData(ipl, data, step);
		return 0;
	}
    else {
		cerr << "ERROR:  Cannot Allocate Memory for CVIMage!";
		return 1;
    }
*/
}

/*
bool CVImage::resize(IppiSize sizeVar) {
    if(iSize.width != sizeVar.width || iSize.height != sizeVar.height) {
    	iSize = sizeVar;
		resetROI();
    	deallocateMemory();
    	allocateMemory();
		return 1;
    }
	return 0;
}
*/

bool CVImage::resize(CvSize sizeVar) {
    if(cSize.width != sizeVar.width || cSize.height != sizeVar.height) {
    	cSize = sizeVar;
		resetROI();
    	deallocateMemory();
    	allocateMemory();
		return 1;
    }
	return 0;
}

bool CVImage::resize(int widthVar, int heightVar) {
    if(width != widthVar || height != heightVar) {
    	width = widthVar;
    	height = heightVar;
		resetROI();
    	deallocateMemory();
    	allocateMemory();
		return 1;
    }
	return 0;
}
    
/*
bool CVImage::changeAttributes(IppiSize sizeVar, int typeVar, int zPad) {
    if(iSize.width != sizeVar.width || iSize.height != sizeVar.height || cvMatType != typeVar || zeroPad != zPad) {
		iSize = sizeVar;
		resetROI();
    	cvMatType = typeVar;
    	deallocateMemory();
		zeroPad = zPad;  // must occur after deallocation!
    	allocateMemory();
		return 1;
	}
	return 0;
}
*/

bool CVImage::changeAttributes(CvSize sizeVar, int typeVar, int zPad) {
    if(cSize.width != sizeVar.width || cSize.height != sizeVar.height || cvMatType != typeVar || zeroPad != zPad) {
		cSize = sizeVar;
		resetROI();
    	cvMatType = typeVar;
    	deallocateMemory();
		zeroPad = zPad;  // must occur after deallocation!
    	allocateMemory();
		return 1;
    }
	return 0;
}

bool CVImage::changeAttributes(int widthVar, int heightVar, int typeVar, int zPad) {
    if(width != widthVar || height != heightVar || cvMatType != typeVar || zeroPad != zPad) {
		width = widthVar;
		height = heightVar;
		resetROI();
    	cvMatType = typeVar;
    	deallocateMemory();
		zeroPad = zPad;  // must occur after deallocation!
    	allocateMemory();
		return 1;
    }
	return 0;
}

void CVImage::cloneAttributes(const IplImage* donorIpl) {
	changeAttributes(donorIpl->width, donorIpl->height, iplToCvMatType(donorIpl->depth, donorIpl->nChannels));
}

void CVImage::cloneAttributes(const CvMat* donorMat) {
	changeAttributes(donorMat->width, donorMat->height, donorMat->type);
}

void CVImage::cloneAttributes(const CVImage* donorImg) {
	changeAttributes(donorImg->cSize, donorImg->cvMatType, donorImg->zeroPad);
}

void CVImage::cloneImage(const IplImage* donorIpl) {
	cloneAttributes(donorIpl);
	cvCopy(donorIpl,ipl);
}

void CVImage::cloneImage(const CvMat* donorMat) {
	cloneAttributes(donorMat);
	cvCopy(donorMat, mat);
}
		
void CVImage::cloneImage(const CVImage* donorImg) {
	cloneAttributes(donorImg);
	cvCopy(donorImg->mat, mat);
}


int CVImage::iplToCvMatType(int depthOfIpl, int numberOfIplChannels) {

	switch(depthOfIpl) {
		default:

		case IPL_DEPTH_8U:
			switch(numberOfIplChannels) {
				default:
				case 1:
					return CV_8UC1;
				case 2:
					return CV_8UC2;
				case 3:
					return CV_8UC3;
				case 4:
					return CV_8UC4;
			}

		case IPL_DEPTH_16S:
			switch(numberOfIplChannels) {
				default:
				case 1:
					return CV_16SC1;
				case 2:
					return CV_16SC2;	
				case 3:
					return CV_16SC3;			
				case 4:
					return CV_16SC4;	
			}

		case IPL_DEPTH_32S:
			switch(numberOfIplChannels) {
				default:
				case 1:
					return CV_32SC1;	
				case 2:
					return CV_32SC2;	
				case 3:
					return CV_32SC3;			
				case 4:
					return CV_32SC4;
			}

		case IPL_DEPTH_32F:
			switch(numberOfIplChannels) {
				default:
				case 1:
					return CV_32FC1;	
				case 2:
					return CV_32FC2;	
				case 3:
					return CV_32FC3;			
				case 4:
					return CV_32FC4;	
			}
	}
}

int CVImage::getNumChannels(int cvMatType) {
	switch(cvMatType) {
		default:
		case CV_8UC1:
		case CV_16SC1:
		case CV_32SC1:
		case CV_32FC1:
			return 1;
		case CV_8UC2:
		case CV_16SC2:
		case CV_32SC2:
		case CV_32FC2:
			return 2;
		case CV_8UC3:
		case CV_16SC3:
		case CV_32SC3:
		case CV_32FC3:
			return 3;
		case CV_8UC4:
		case CV_16SC4:
		case CV_32SC4:
		case CV_32FC4:
			return 4;
		
	}
}

/*
void CVImage::displayInWindow(char* windowName) {
    cvShowImage(windowName, ipl);
}
*/

void CVImage::resetROI() {
    roi.x = 0;
    roi.y = 0;
    roi.width = width;
    roi.height = height;
}

void CVImage::zeroOut() {
//	memset(data8u - zeroPad * byteDepth() * nChannels - zeroPad * step,0,step * (height+zeroPad*2));
	memset(reinterpret_cast<char*>(data) - zeroPad * byteDepth() * nChannels - zeroPad * step,0,step * (height+zeroPad*2));
}


int CVImage::byteDepth() {
	return static_cast<int>(iplDepth | IPL_DEPTH_SIGN) > 0 ? static_cast<int>(iplDepth | IPL_DEPTH_SIGN)/8: iplDepth/8;
}

	 
void CVImage::toStream(std::ostream* str) {
	*str << type() << std::endl;
	blockToStream(str, (char*)&cSize, sizeof(CvSize));
	blockToStream(str, (char*)&cvMatType, sizeof(int));
	blockToStream(str, (char*)&zeroPad, sizeof(int));
	blockToStream(str, (char*)&cROI, sizeof(CvRect));
	blockToStream(str, (char*)data, step*height);
/*
	str->write(reinterpret_cast<char*>(&iSize),sizeof(IppiSize));
	str->write(reinterpret_cast<char*>(&cvMatType),sizeof(int));
	str->write(reinterpret_cast<char*>(&zeroPad),sizeof(int));
	str->write(reinterpret_cast<char*>(&iROI),sizeof(IppiRect));
	str->write(reinterpret_cast<char*>(data),step * height);
*/
}

CVImage* CVImage::fromStream(std::istream* str) {

	std::string temp;
	getline(*str, temp); 
	if(temp != this->type()) {
		std::cerr << "CVImage::fromStream()::ERROR::Stream type [" << temp << "] is different from data type [" << type() << "]\n";
		return NULL;
	}

	CvSize cvSizeTemp;
	int cvMatTypeTemp;
	int zeroPadTemp;

    blockFromStream(str, (char*)&cvSizeTemp, sizeof(CvSize));
    blockFromStream(str, (char*)&cvMatTypeTemp, sizeof(int));
    blockFromStream(str, (char*)&zeroPadTemp, sizeof(int));
	
	changeAttributes(cvSizeTemp,cvMatTypeTemp,zeroPadTemp);
	
    blockFromStream(str, (char*)&cROI, sizeof(CvRect));
    blockFromStream(str, (char*)data, step*height);

	return this;
/*
	str->read(reinterpret_cast<char*>(&cvSizeTemp),sizeof(IppiSize));
	str->read(reinterpret_cast<char*>(&cvMatTypeTemp),sizeof(int));
	str->read(reinterpret_cast<char*>(&zeroPadTemp),sizeof(int));
	changeAttributes(cvSizeTemp,cvMatTypeTemp,zeroPadTemp);
	str->read(reinterpret_cast<char*>(&iROI),sizeof(IppiRect));
	str->read(reinterpret_cast<char*>(data),step * height);
*/
}

void CVImage::writeToFile(const std::string& filename) {
	ofstream outFile(filename.c_str());
	if(outFile.is_open()) {
		this->toStream(&outFile);
		outFile.close();
	}
	else std::cerr << "CVImage::writeToFile()::ERROR::Could not open [" << filename << "] for writing!\n";
}

bool CVImage::readFromFile(const std::string& filename) {
	ifstream inFile(filename.c_str());
	if (inFile.is_open()) {

		CVImage* temp = this->fromStream(&inFile);
		if(!temp) {	std::cerr << "CVImage::readFromFile()::ERROR::Failed reading fromStream()!\n"; return false; }
		
		inFile.close();
		return true;
	}
	else {
		std::cerr << "CVImage::readFromFile()::ERROR::Could not open [" << filename << "] for reading!\n";
		return false;
	}

}

void CVImage::print() {
	std::cout << "=====================================\n";
	std::cout << type() << "\n";
	std::cout << "size = [" << height << " x " << width << "]\n";
	std::cout << "step = " << step << "\n";
	std::cout << "nChannels = " << nChannels << "\n";
	std::cout << "cvMatType = " << cvMatType << "\n";

	std::cout << "[data] = [\n";
	for(int j = 0;j<height;j++) {
		for(int i = 0;i<width;i++) {
		
			CvScalar temp = cvGet2D(ipl, j, i);
			std::cout << "[";
			for(int k = 0;k<nChannels;k++) std::cout << temp.val[k] << " ";
			std::cout << "] ";
		
			//std::cout << cvGet2D(ipl, j, i) << " ";
		}
		std::cout << ";\n";
	}
	std::cout << "]\n";

	std::cout << "=====================================\n";
}

	
