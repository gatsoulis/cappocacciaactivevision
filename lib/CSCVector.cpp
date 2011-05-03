#include "CSCVector.h"
#include "serializer.h"
#include <float.h>
#include <cmath>
#include <fstream>

using namespace std;


//constructor
CSCVector::CSCVector() : CSCData() {
	fl = NULL;
	width = 0;
	heapallocated = false;
}

//destructor
CSCVector::~CSCVector() {
	this->deallocate();
	//std::cout << "CSCVector::Desctructor::succesful\n";
}

//copy constructor
CSCVector::CSCVector(const CSCVector& right) : CSCData(right) {
	width = right.width;
	fl = this->allocate(width);
	if(!fl) std::cerr << type() << "::Copy constructor()::ERROR::Memory allocation error! size = " << width << "\n";
	else memcpy(fl, right.fl, width*sizeof(float));
}

//copy assignment operator
CSCVector& CSCVector::operator=(const CSCVector& right) {
	CSCData::operator=(right);
	unsigned int rightsize = right.width;
	if(width != rightsize) {
		//if(fl) this->deallocate();
		fl = this->allocate(rightsize);
		if(!fl) std::cerr << type() << "::Copy assignement operator()::ERROR::Memory allocation error! size = " << rightsize << "\n";
		width = rightsize;
	}
	memcpy(fl, right.fl, width*sizeof(float));
	return *this;
}

//constructor with parameters
CSCVector::CSCVector(unsigned int size) : CSCData() {
	this->width = size;
	fl = this->allocate(width);
	if(!fl) std::cerr << type() << "::Constructor()::ERROR::Memory allocation error! size = " << width << "\n";
}

//returns size of the heap memory allocated in bytes
unsigned int CSCVector::size() {
	return (width*sizeof(float));
}

//Serializes the data
void CSCVector::toStream(std::ostream* str) {
	*str << type() << std::endl;
	blockToStream(str, (char*)&width, sizeof(unsigned int));
	blockToStream(str, (char*)fl, sizeof(float) * width);
}  

//Recreates the data
CSCVector* CSCVector::fromStream(std::istream* str) {
//	std::string temp;
//	getline(*str, temp); 
//	if(temp != this->type()) {
//		std::cerr << "CSCVector::fromStream()::ERROR::Stream type [" << temp << "] is different from data type [" << type() << "]\n";
//		return NULL;
//	}

	unsigned int tempwidth;
    blockFromStream(str, (char*)&tempwidth, sizeof(unsigned int));
	if(fl && heapallocated) {
		if(width != tempwidth) {
			//this->deallocate();
			fl = this->allocate(tempwidth);
			if(!fl) { std::cerr << type() << "::fromStream()::ERROR::Memory allocation error! size = " << tempwidth << "\n"; return NULL; }
		}
	}
	else{
		fl = this->allocate(tempwidth);
		if(!fl) { std::cerr << type() << "::fromStream()::ERROR::Memory allocation error! size = " << tempwidth << "\n"; return NULL; }
	}
//	width = tempwidth;
    blockFromStream(str, (char*)fl, sizeof(float) * width);
	return this;
}

void CSCVector::writeToFile(const std::string& filename) {
	ofstream outFile(filename.c_str());
	if(outFile.is_open()) {
		//outFile << type() << std::endl;
		this->toStream(&outFile);
		outFile.close();
	}
	else std::cerr << "CSCVector::writeToFile()::ERROR::Could not open [" << filename << "] for writing!\n";
}

bool CSCVector::readFromFile(const std::string& filename) {
	ifstream inFile(filename.c_str());
	if (inFile.is_open()) {

//		CSCVector* temp = this->fromStream(&inFile);
//		if(!temp) {	std::cerr << "CSCVector::readFromFile()::ERROR::Failed reading fromStream() file[" << filename << "]!\n"; return false; }

		std::string temp;
		getline(inFile, temp); 
		
		if(temp == this->type()) {
			CSCVector* temp = this->fromStream(&inFile);
			if(!temp) {	std::cerr << "CSCVector::readFromFile()::ERROR::Failed reading fromStream()!\n"; return false; }
		}
		else{
			std::cerr << "CSCVector::readFromFile()::ERROR::File type [" << temp << "] is different from data type [" << type() << "]\n";
			return false;
		}
		
		inFile.close();
		return true;
	}
	else {
		std::cerr << "CSCVector::readFromFile()::ERROR::Could not open [" << filename << "] for reading!\n";
		return false;
	}

}

void CSCVector::print() {
	std::cout << "=====================================\n";
	std::cout << type() << "[addr:" << this << "]\n";
	std::cout << "width = " << width << "\n";
	std::cout << "data[addr:" << fl << "] = [\n";
	for(unsigned int i = 0;i<width;i++) {
		std::cout << fl[i] << " ";
	}
	std::cout << "]\n";
	std::cout << "=====================================\n";
}

float* CSCVector::allocate(unsigned int size) {
	if(fl) {
		if(heapallocated) {
			if(width != size) {
				this->deallocate();
				width = size;
				fl = new float [width];
				if(!fl) { std::cerr << "CSCVector::allocate()::ERROR::Memory allocation error! size = " << width << "\n"; return NULL; }
				heapallocated = true;
			}
			else{
				//return fl;
			}
		}
		else{
			width = size;
			fl = new float [width];
			if(!fl) { std::cerr << "CSCVector::allocate()::ERROR::Memory allocation error! size = " << width << "\n"; return NULL; }
			heapallocated = true;
		}
	}
	else{
		width = size;
		fl = new float [width];
		if(!fl) { std::cerr << "CSCVector::allocate()::ERROR::Memory allocation error! size = " << width << "\n"; return NULL; }
		heapallocated = true;
	}
	for(unsigned int i = 0;i<width;++i) fl[i] = 0.0;
	return fl;
}

void CSCVector::deallocate() {

	//std::cout << "CSCVector::deallocate()::width = " << width << "\n";
	//std::cout << "CSCVector::deallocate()::fl = " << fl << "\n";
	//std::cout << "CSCVector::deallocate()::heapallocated = " << heapallocated << "\n";

	if(fl && heapallocated) {
		//std::cout << "CSCVector::deallocate()::deleting fl\n";
		delete [] fl;
		//std::cout << "CSCVector::deallocate()::deleting fl succesful\n";
	}

	heapallocated = false;
	fl = NULL;
	width = 0;

	//std::cout << "CSCVector::deallocate()::succesful\n";
}

void CSCVector::set(float input) {
	for(unsigned int i = 0;i<width;++i) fl[i] = input;
}

CSCVector& CSCVector::operator+=(const CSCVector& right) {
//	CSCData::operator=(right);
	if(this->width != right.width) {
		std::cerr << "CSCVector::operator+=()::ERROR::Left and right vector sizes do not match!\n"; return *this;
	}
	for(unsigned int i = 0;i<width;++i) fl[i] += right.fl[i];
	return *this;	
}

CSCVector& CSCVector::operator-=(const CSCVector& right) {
//	CSCData::operator=(right);
	if(this->width != right.width) {
		std::cerr << "CSCVector::operator-=()::ERROR::Left and right vector sizes do not match!\n"; return *this;
	}
	for(unsigned int i = 0;i<width;++i) fl[i] -= right.fl[i];
	return *this;	
}

void CSCVector::normalize() {
	float sum = 0.0;
	for(unsigned int i = 0;i<width;++i) {
		float temp = fl[i];
		sum += temp * temp;
	}
	if(sum > 0.0) {
		float temp = sqrt(sum);	
		for(unsigned int i = 0;i<width;++i) (fl[i]) /= temp;
	}
}

void CSCVector::findMin(float* min, unsigned int* pos) {
	unsigned int tempind = 0;
	float tempmin = FLT_MAX;
	for(unsigned int i = 0;i<width;++i) {
		float temp = fl[i];
		if(temp < tempmin) {
			tempind = i;
			tempmin = temp;
		}
	}
	if(min) (*min) = tempmin;
	if(pos) (*pos) = tempind;
}

void CSCVector::findMax(float* max, unsigned int* pos) {
	unsigned int tempind = 0;
	float tempmax = FLT_MIN;
	for(unsigned int i = 0;i<width;++i) {
		float temp = fl[i];
		if(temp > tempmax) {
			tempind = i;
			tempmax = temp;
		}
	}
	if(max) (*max) = tempmax;
	if(pos) (*pos) = tempind;
}

void CSCVector::findMinMax(float* min, float* max, unsigned int* minpos, unsigned int* maxpos) {
	unsigned int tempindmin = 0;
	unsigned int tempindmax = 0;
	float tempmax = FLT_MIN;
	float tempmin = FLT_MAX;
	for(unsigned int i = 0;i<width;++i) {
		float temp = fl[i];
		if(temp > tempmax) {
			tempindmax = i;
			tempmax = temp;
		}
		if(temp < tempmin) {
			tempindmin = i;
			tempmin = temp;
		}
	}
	if(max) (*max) = tempmax;
	if(min) (*min) = tempmin;
	if(maxpos) (*maxpos) = tempindmax;
	if(minpos) (*minpos) = tempindmin;
}

unsigned int CSCVector::countZero() {
	unsigned int tempcount = 0;	
	for(unsigned int i = 0;i<width;++i) {
		if(fl[i] == 0.0) {
			tempcount++;
		}
	}
	return tempcount;
}

unsigned int CSCVector::countNonZero() {
	unsigned int temp = width - countZero();
	return temp;
}

unsigned int CSCVector::countBelowZero() {
	unsigned int tempcount = 0;	
	for(unsigned int i = 0;i<width;++i) {
		if(fl[i] < 0.0) {
			tempcount++;
		}
	}
	return tempcount;
}

unsigned int CSCVector::countAboveOrEqualToZero() {
	unsigned int temp = width - countBelowZero();
	return temp;
}

unsigned int CSCVector::countAboveZero() {
	unsigned int tempcount = 0;	
	for(unsigned int i = 0;i<width;++i) {
		if(fl[i] > 0.0) {
			tempcount++;
		}
	}
	return tempcount;
}

unsigned int CSCVector::countBelowOrEqualToZero() {
	unsigned int temp = width - countAboveZero();
	return temp;
}

void CSCVector::copyData(float* input, unsigned int size) {
	this->allocate(size);
	memcpy(fl, input, size*sizeof(float));
}

void CSCVector::pointData(float* input, unsigned int size) {
	this->deallocate();
	fl = input;
	width = size;
//	heapallocated = false;
}

float CSCVector::dotproduct(const CSCVector& right) {
	float tempsum = 0.0;
	if(this->width != right.width) {
		std::cerr << "CSCVector::dotproduct()::ERROR::Left and right vector sizes do not match!\n"; return 0.0;
	}
	for(unsigned int i = 0;i<width;++i) tempsum += (fl[i]) * (right.fl[i]);
	return tempsum;	
}

