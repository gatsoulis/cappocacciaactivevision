#include "CSCMatrix2D.h"
#include "serializer.h"
#include <fstream>
#include <cmath>

using namespace std;


//constructor
CSCMatrix2D::CSCMatrix2D() : CSCData() {
	fl = NULL;
	heapallocated = false;
	dim = 2;
	height = 0;
	width = 0;
	step = 0;
}

//destructor
CSCMatrix2D::~CSCMatrix2D() {
	this->deallocate();
}

//copy constructor
CSCMatrix2D::CSCMatrix2D(const CSCMatrix2D& right) : CSCData(right) {
	height = right.height; // height
	width = right.width; // width
	step = right.step;
	fl = this->allocate(height, width, step);
	if(!fl) std::cerr << type() << "::Copy constructor()::ERROR::Memory allocation error!\n";
	unsigned int temp = height * step; // bytes
	memcpy(fl, right.fl, temp*sizeof(char));
}

//copy assignment operator
CSCMatrix2D& CSCMatrix2D::operator=(const CSCMatrix2D& right) {

	CSCData::operator=(right);

	height = right.height; // height
	width = right.width; // width
	step = right.step;

	//this->deallocate();
	fl = this->allocate(height, width, step);
	if(!fl) std::cerr << type() << "::Copy assignement operator()::ERROR::Memory allocation error!\n";

	unsigned int temp = height * step; // bytes
	memcpy(fl, right.fl, temp*sizeof(char));

	return *this;
}

//constructor with parameters
/*
CSCMatrix2D::CSCMatrix2D(unsigned int height, unsigned int width) : CSCData() {
	height = height;
	width = width;
	height = width * 4;
	if(height < 4 * width) std::cerr << type() << "::Constructor w param()::ERROR::size too small!\n";
	fl = this->allocate(height, width, height);
	if(!fl) std::cerr << type() << "::Constructor w param()::ERROR::Memory allocation error!\n";
}
*/

CSCMatrix2D::CSCMatrix2D(unsigned int h, unsigned int w, unsigned int s) : CSCData() {
	if(s == 0) s = w*4;
	height = h;
	width = w;
	step = s;
	if(step < 4 * width) std::cerr << type() << "::Constructor w param()::ERROR::step too small!\n";
	fl = this->allocate(height, width, step);
	if(!fl) std::cerr << type() << "::Constructor w param()::ERROR::Memory allocation error!\n";
}

//returns size of the heap memory allocated in bytes
unsigned int CSCMatrix2D::size() {
	return (height*step*sizeof(char));
}

//Serializes the fl
void CSCMatrix2D::toStream(std::ostream* str) {
	*str << type() << std::endl;
	blockToStream(str, (char*)&(height), sizeof(unsigned int));
	blockToStream(str, (char*)&(width), sizeof(unsigned int));
	blockToStream(str, (char*)&(step), sizeof(unsigned int));
	blockToStream(str, (char*)fl, height*step*sizeof(char));
}  

//Recreates the fl
CSCMatrix2D* CSCMatrix2D::fromStream(std::istream* str) {
//	std::string temp;
//	getline(*str, temp); 
//	if(temp != this->type()) {
//		std::cerr << "CSCMatrix2D::fromStream()::ERROR::Stream type [" << temp << "] is different from data type [" << type() << "]\n";
//		return NULL;
//	}

	unsigned int tempwidth, tempheight, tempstep;
    blockFromStream(str, (char*)&(tempheight), sizeof(unsigned int));
    blockFromStream(str, (char*)&(tempwidth), sizeof(unsigned int));
    blockFromStream(str, (char*)&(tempstep), sizeof(unsigned int));
/*
	if(fl && heapallocated) {
		if(width == tempwidth && height == tempheight && step == tempstep) {
		}
		else{
			//this->deallocate();
			fl = this->allocate(tempheight, tempwidth, tempstep);
			if(!fl) { std::cerr << type() << "::fromStream()::ERROR::Memory allocation error!\n"; return NULL; }
		}
	}
	else{
		fl = this->allocate(tempheight, tempwidth, tempstep);
		if(!fl) { std::cerr << type() << "::fromStream()::ERROR::Memory allocation error!\n"; return NULL; }
	}
*/
	fl = this->allocate(tempheight, tempwidth, tempstep);
	if(!fl) { std::cerr << type() << "::fromStream()::ERROR::Memory allocation error!\n"; return NULL; }

	unsigned int tempsize = tempheight*tempstep; // bytes
    blockFromStream(str, (char*)fl, tempsize*sizeof(char));
	return this;
}

void CSCMatrix2D::writeToFile(const std::string& filename) {
	ofstream outFile(filename.c_str());
	if(outFile.is_open()) {
		this->toStream(&outFile);
		outFile.close();
	}
	else std::cerr << "CSCMatrix2D::writeToFile()::ERROR::Could not open [" << filename << "] for writing!\n";
}

bool CSCMatrix2D::readFromFile(const std::string& filename) {
	ifstream inFile(filename.c_str());
	if (inFile.is_open()) {

//		CSCMatrix2D* temp = this->fromStream(&inFile);
//		if(!temp) {	std::cerr << "CSCMatrix2D::readFromFile()::ERROR::Failed reading fromStream() file[" << filename << "]!\n"; return false; }

		std::string temp;
		getline(inFile, temp); 
		
		if(temp == this->type()) {
			CSCMatrix2D* temp = this->fromStream(&inFile);
			if(!temp) {	std::cerr << "CSCMatrix2D::readFromFile()::ERROR::Failed reading fromStream()!\n"; return false; }
		}
		else{
			std::cerr << "CSCMatrix2D::readFromFile()::ERROR::File type [" << temp << "] is different from fl type [" << type() << "]\n";
			return false;
		}
		
		inFile.close();
		return true;
	}
	else {
		std::cerr << "CSCMatrix2D::readFromFile()::ERROR::Could not open [" << filename << "] for reading!\n";
		return false;
	}

}

void CSCMatrix2D::print() {
	std::cout << "=====================================\n";
	std::cout << type() << "\n";
	std::cout << "size = [" << height << " x " << width << "]\n";
	std::cout << "step = " << step << "\n";

	unsigned int index = 0;
	std::cout << "data[addr:" << fl << "] = [\n";
	for(unsigned int j = 0;j<height;j++) {
		for(unsigned int i = 0;i<width;i++) {
			std::cout << fl[index] << " ";
			index++;
		}
		std::cout << ";\n";
		index += ((step - 4*width)/4);
	}
	std::cout << "]\n";

	std::cout << "=====================================\n";
}

float* CSCMatrix2D::allocate(unsigned int h, unsigned int w, unsigned int s) {

	if(s == 0) s = w*4;

	if(fl) {
		if(heapallocated) {
			if(width == w && height == h && step == s) {
			}
			else{
				this->deallocate();
				width = w;
				height = h;
				step = s;
				unsigned int tempsize = height*(step / 4);
				fl = new float [tempsize];
				if(!fl) { std::cerr << "CSCVector::allocate()::ERROR::Memory allocation error! size = " << width << "\n"; return NULL; }
				heapallocated = true;
			}
		}
		else{
			width = w;
			height = h;
			step = s;
			unsigned int tempsize = height*(step / 4);
			fl = new float [tempsize];
			if(!fl) { std::cerr << "CSCVector::allocate()::ERROR::Memory allocation error! size = " << width << "\n"; return NULL; }
			heapallocated = true;
		}
	}
	else{
		width = w;
		height = h;
		step = s;
		unsigned int tempsize = height*(step / 4);
		fl = new float [tempsize];
		if(!fl) { std::cerr << "CSCVector::allocate()::ERROR::Memory allocation error! size = " << width << "\n"; return NULL; }
		heapallocated = true;
	}

	if(step % 4 != 0) { std::cerr << "CSCMatrix2D::allocate()::ERROR::Incorrect step value!\n"; return NULL; }
	if(step < width * 4) { std::cerr << "CSCMatrix2D::allocate()::ERROR::step too small!\n"; return NULL; }

	unsigned int tempsize = height*(step / 4);
	for(unsigned int i = 0;i<tempsize;++i) fl[i] = 0.0;
	return fl;
}

void CSCMatrix2D::deallocate() {
	if(fl && heapallocated) delete [] fl;
	fl = NULL;
	heapallocated = false;
	height = 0;
	width = 0;
	step = 0;
}

void CSCMatrix2D::pointData(float* input, unsigned int h, unsigned int w, unsigned int s) {
	this->deallocate();
	if(s == 0) s = w*4;
	fl = input;
	height = h;
	width = w;
	step = s;
}

void CSCMatrix2D::copyData(float* input, unsigned int h, unsigned int w, unsigned int s) {
	if(s == 0) s = w*4;
	this->allocate(h, w, s);
	unsigned int tempsize = h*s;
	memcpy(fl, input, tempsize*sizeof(char));
}

void CSCMatrix2D::set(float input) {
	unsigned int index = 0;
	for(unsigned int j = 0;j<height;j++) {
		for(unsigned int i = 0;i<width;i++) {
			fl[index] = input;
			index++;
		}
		index += ((step - 4*width)/4);
	}
}

/*
void CSCMatrix2D::normalize() {
	float sum = 0.0;
	unsigned int index = 0;
	for(unsigned int j = 0;j<height;j++) {
		for(unsigned int i = 0;i<width;i++) {
			float temp1 = fl[index];
			sum += temp1*temp1;
			index++;
		}
		index += ((height - 4*width)/4);
	}
	if(sum > 0.0) {
		sum = sqrt(sum);	
		int index = 0;
		for(unsigned int j = 0;j<height;j++) {
			for(unsigned int i = 0;i<width;i++) {
				fl[index] /= sum;
				index++;
			}
			index += ((height - 4*width)/4);
		}
	}
}

CSCMatrix2D* CSCMatrix2D::transpose() {
	CSCMatrix2D* newmat = new CSCMatrix2D(width, height);
	float* dst = newmat->getData();
	for(unsigned int j = 0;j<height;j++) {
		for(unsigned int i = 0;i<width;i++) {
			unsigned int index1 = j*width + i;
			unsigned int index2 = i*height + j;
			dst[index2] = fl[index1];
		}
	}
	return newmat;
}
*/

