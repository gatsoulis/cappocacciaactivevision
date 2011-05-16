#include "CSCEllipticRegions.h"
#include "serializer.h"
#include <fstream>
#include <cmath>

using namespace std;


//constructor
CSCEllipticRegions::CSCEllipticRegions() : CSCMatrix2D() {
	fl = NULL;
	width = 5; // width
	height = 0; // height
	step = 0; // aligned row size in bytes
	heightused = 0;
	dim = 2;
}

//destructor
CSCEllipticRegions::~CSCEllipticRegions() {
	this->deallocate();
}

//copy constructor
CSCEllipticRegions::CSCEllipticRegions(const CSCEllipticRegions& right) : CSCMatrix2D(right) {
	height = right.height; // height
	width = right.width; // width
	step = right.step;
	fl = this->allocate(height, width);
	if(!fl) { std::cerr << type() << "::Copy constructor()::ERROR::Memory allocation error!\n"; return; }

	heightused = right.heightused;
	unsigned int temp = height * step; // bytes
	memcpy(fl, right.fl, temp*sizeof(char));
}

//copy assignment operator
CSCEllipticRegions& CSCEllipticRegions::operator=(const CSCEllipticRegions& right) {

	CSCData::operator=(right);

	height = right.height; // height
	width = right.width; // width
	step = right.step;
	//this->deallocate();
	fl = this->allocate(height, width);
	if(!fl) std::cerr << type() << "::Copy assignement operator()::ERROR::Memory allocation error!\n";

	heightused = right.heightused;
	unsigned int temp = heightused * step; // bytes
	memcpy(fl, right.fl, temp*sizeof(char));

	return *this;
}

//constructor with parameters
CSCEllipticRegions::CSCEllipticRegions(unsigned int h, unsigned int w) : CSCMatrix2D() {
	if(w != 5) { std::cerr << type() << "::Constructor()::ERROR::w(=" << w << ") must be 5!\n"; fl = NULL; return; }
	height = h;
	width = w;
	step = width * 4;
	fl = this->allocate(height, width);
	if(!fl) std::cerr << type() << "::Constructor w param()::ERROR::Memory allocation error!\n";
}

/*
CSCEllipticRegions::CSCEllipticRegions(unsigned int height, unsigned int width, unsigned int tempstepsize) : CSCMatrix2D() {
	if(width != 5) { std::cerr << type() << "::Constructor()::ERROR::width(=" << width << ") must be 5!\n"; fl = NULL; return; }
	height = height;
	width = width;
	step = tempstepsize;
	if(step < 4 * width) std::cerr << type() << "::Constructor w param()::ERROR::stepsize too small!\n";
	fl = this->allocate(height, width, step);
	if(!fl) std::cerr << type() << "::Constructor w param()::ERROR::Memory allocation error!\n";
}

//returns size of the heap memory allocated in bytes
unsigned int CSCEllipticRegions::size() {
//	return (step*height*sizeof(char));
	return CSCMatrix2D::size();
}
*/

//Serializes the fl
void CSCEllipticRegions::toStream(std::ostream* str) {
	*str << type() << std::endl;
	*str << detectortype << std::endl;
	blockToStream(str, (char*)&(heightused), sizeof(unsigned int));
	blockToStream(str, (char*)&(height), sizeof(unsigned int));
	blockToStream(str, (char*)&(width), sizeof(unsigned int));
	blockToStream(str, (char*)&(step), sizeof(unsigned int));
	blockToStream(str, (char*)fl, height*width*sizeof(float));
}  

//Recreates the fl
CSCEllipticRegions* CSCEllipticRegions::fromStream(std::istream* str) {

	std::string temp;
	getline(*str, temp); 
		
	if(temp != this->type()) {
		std::cerr << "CSCEllipticRegions::fromStream()::ERROR::Stream type [" << temp << "] is different from data type [" << type() << "]\n";
		return NULL;
	}

//	std::string temp;
	getline(*str, temp);
	detectortype = temp;

	unsigned int tempwidth, tempheight, tempstep;
    blockFromStream(str, (char*)&(heightused), sizeof(unsigned int));
    blockFromStream(str, (char*)&(tempheight), sizeof(unsigned int));
    blockFromStream(str, (char*)&(tempwidth), sizeof(unsigned int));
    blockFromStream(str, (char*)&(tempstep), sizeof(unsigned int));

	//this->deallocate();
	fl = this->allocate(tempheight, tempwidth);
	if(!fl) { std::cerr << type() << "::fromStream()::ERROR::Memory allocation error!\n"; return NULL; }

	unsigned int tempsize = tempheight*tempwidth; // bytes
    blockFromStream(str, (char*)fl, tempsize*sizeof(float));
	return this;
}

void CSCEllipticRegions::writeToFile(const std::string& filename) {
	ofstream outFile(filename.c_str());
	if(outFile.is_open()) {
		this->toStream(&outFile);
		outFile.close();
	}
	else std::cerr << "CSCEllipticRegions::writeToFile()::ERROR::Could not open [" << filename << "] for writing!\n";
}

bool CSCEllipticRegions::readFromFile(const std::string& filename) {
	ifstream inFile(filename.c_str());
	if (inFile.is_open()) {

		CSCEllipticRegions* temp = this->fromStream(&inFile);
		if(!temp) {	std::cerr << "CSCEllipticRegions::readFromFile()::ERROR::Failed reading fromStream() file[" << filename << "]!\n"; return false; }
/*
		std::string temp;
		getline(inFile, temp); 
		
		if(temp == this->type()) {
			CSCEllipticRegions* temp = this->fromStream(&inFile);
			if(!temp) {	std::cerr << "CSCEllipticRegions::readFromFile()::ERROR::Failed reading fromStream()!\n"; return false; }
		}
		else{
			std::cerr << "CSCEllipticRegions::readFromFile()::ERROR::File type [" << temp << "] is different from fl type [" << type() << "]\n";
			return false;
		}
*/		
		inFile.close();
		return true;
	}
	else {
		std::cerr << "CSCEllipticRegions::readFromFile()::ERROR::Could not open [" << filename << "] for reading!\n";
		return false;
	}

}

void CSCEllipticRegions::print() {
	std::cout << "=====================================\n";
	std::cout << type() << "\n";
	std::cout << detectortype << "\n";
	std::cout << "size = [" << height << " x " << width << "]\n";
	std::cout << "step = " << step << "\n";
	std::cout << "heightused = " << heightused << "\n";

	unsigned int index = 0;
	std::cout << "fl[addr:" << fl << "] = [\n";
	for(unsigned int j = 0;j<heightused;j++) {
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


float* CSCEllipticRegions::allocate(unsigned int h, unsigned int w) {
	heightused = 0;
	float* fl = CSCMatrix2D::allocate(h, w);
	return fl;
}

void CSCEllipticRegions::deallocate() {
	heightused = 0;
	CSCMatrix2D::deallocate();
}

/*
float* CSCEllipticRegions::allocate(unsigned int height, unsigned int width, unsigned int tempstepsize) {
	heightused = 0;
	float* fl = CSCMatrix2D::allocate(height, width, tempstepsize);
	return fl;
}
*/
