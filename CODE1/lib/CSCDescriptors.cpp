#include "CSCDescriptors.h"
#include "serializer.h"
#include <fstream>
#include <cmath>

using namespace std;


//std::string CSCDescriptors::m_type = "CDesc";

//constructor
CSCDescriptors::CSCDescriptors() : CSCMatrix2D() {
	fl = NULL;
	width = 0; // width
	height = 0; // height
	step = 0; // aligned row size in bytes
	heightused = 0;
	dim = 2;
//	descriptortype.clear();
	regions = NULL;
	regions_createdhere = false;
}

//destructor
CSCDescriptors::~CSCDescriptors() {
	this->deallocate();
	if(regions_createdhere) {
		regions->deallocate();
		delete regions;
	}
}

//copy constructor
CSCDescriptors::CSCDescriptors(const CSCDescriptors& right) : CSCMatrix2D(right) {
	descriptortype = right.descriptortype;
	regions = right.regions;
	regions_createdhere = false;

	height = right.height; // height
	width = right.width; // width
	step = right.step;
	heightused = right.heightused;
	fl = this->allocate(height, width);
	if(!fl) std::cerr << type() << "::Copy constructor()::ERROR::Memory allocation error!\n";
	unsigned int temp = height * step; // bytes
	memcpy(fl, right.fl, temp*sizeof(char));

}

//copy assignment operator
CSCDescriptors& CSCDescriptors::operator=(const CSCDescriptors& right) {

	CSCData::operator=(right);

	descriptortype = right.descriptortype;
//	regions = right.regions;
//	regions_createdhere = false;

	height = right.height; // height
	width = right.width; // width
	step = right.step;
//	this->deallocate();
	fl = this->allocate(height, width);
	if(!fl) std::cerr << type() << "::Copy assignement operator()::ERROR::Memory allocation error!\n";

	heightused = right.heightused;
	unsigned int temp = height * step; // bytes
	memcpy(fl, right.fl, temp*sizeof(char));

	if(right.regions) {
		if(!regions) regions = new CSCEllipticRegions;
		(*regions) = (*(right.regions));
		regions_createdhere = true;
	}

	return *this;
}

//constructor with parameters
CSCDescriptors::CSCDescriptors(unsigned int h, unsigned int w, const std::string& temptype) : CSCMatrix2D() {
	descriptortype = temptype;
	height = h;
	width = w;
	step = width * 4;
//	if(step < 4 * width) std::cerr << type() << "::Constructor w param()::ERROR::stepsize too small!\n";
	fl = this->allocate(height, width);
	if(!fl) std::cerr << type() << "::Constructor w param()::ERROR::Memory allocation error!\n";
}

/*
//returns size of the heap memory allocated in bytes
unsigned int CSCDescriptors::size() {
//	return (step*height*sizeof(char));
	return CSCMatrix2D::size();
}
*/

//Serializes the fl
void CSCDescriptors::toStream(std::ostream* str) {
	*str << type() << std::endl;
	*str << descriptortype << std::endl;
	if(debug) std::cout << "CSCDescriptors::toStream()::heightused = " << heightused << "\n";
	if(debug) std::cout << "CSCDescriptors::toStream()::height = " << height << "\n";
	if(debug) std::cout << "CSCDescriptors::toStream()::width = " << width << "\n";
	if(debug) std::cout << "CSCDescriptors::toStream()::step = " << step << "\n";
	blockToStream(str, (char*)&(heightused), sizeof(unsigned int));
	blockToStream(str, (char*)&(height), sizeof(unsigned int));
	blockToStream(str, (char*)&(width), sizeof(unsigned int));
	blockToStream(str, (char*)&(step), sizeof(unsigned int));
	blockToStream(str, (char*)fl, height*width*sizeof(float));

	bool regions_exist = false;
	if(regions) {
		regions_exist = true;
		blockToStream(str, (char*)&regions_exist, sizeof(regions_exist));
		regions->toStream(str);
	}
	else{
		blockToStream(str, (char*)&regions_exist, sizeof(regions_exist));
	}

}  

/*
float CSCDescriptors::similarity(const CSCDescriptors& right) {
}
*/

//Recreates the fl
CSCDescriptors* CSCDescriptors::fromStream(std::istream* str) {
	std::string temp;
	getline(*str, temp); 
		
	if(temp != this->type()) {
		std::cerr << "CSCDescriptors::fromStream()::ERROR::Stream type [" << temp << "] is different from data type [" << type() << "]\n";
		return NULL;
	}

//	std::string tempdesctype;
	getline(*str, temp);
	descriptortype = temp;

	unsigned int tempwidth, tempheight, tempstep, tempheightused;
//    blockFromStream(str, (char*)&(heightused), sizeof(unsigned int));
    blockFromStream(str, (char*)&(tempheightused), sizeof(unsigned int));
    blockFromStream(str, (char*)&(tempheight), sizeof(unsigned int));
    blockFromStream(str, (char*)&(tempwidth), sizeof(unsigned int));
    blockFromStream(str, (char*)&(tempstep), sizeof(unsigned int));

//	this->deallocate();
	fl = this->allocate(tempheight, tempwidth);
	if(!fl) { std::cerr << type() << "::fromStream()::ERROR::Memory allocation error!\n"; return NULL; }
	
	heightused = tempheightused;

	if(debug) std::cout << "CSCDescriptors::fromStream()::heightused = " << heightused << "\n";
	if(debug) std::cout << "CSCDescriptors::fromStream()::height = " << height << "\n";
	if(debug) std::cout << "CSCDescriptors::fromStream()::width = " << width << "\n";
	if(debug) std::cout << "CSCDescriptors::fromStream()::step = " << step << "\n";

	unsigned int tempsize = tempheight*tempwidth; // bytes
    blockFromStream(str, (char*)fl, tempsize*sizeof(float));

	bool regions_exist = false;
    blockFromStream(str, (char*)&regions_exist, sizeof(regions_exist));
	if(regions_exist == true) {
	
		if(regions && regions_createdhere == true) {
			delete regions;
			regions = NULL;
		}
		else if(regions && regions_createdhere == false) {
			regions = NULL;
		}
	
		if(!regions) {
			regions = new CSCEllipticRegions;
			regions_createdhere = true;
		}
		regions->fromStream(str);
	}

	return this;
}

void CSCDescriptors::writeToFile(const std::string& filename) {
	ofstream outFile(filename.c_str());
	if(outFile.is_open()) {
		this->toStream(&outFile);
		outFile.close();
	}
	else std::cerr << "CSCDescriptors::writeToFile()::ERROR::Could not open [" << filename << "] for writing!\n";
}

bool CSCDescriptors::readFromFile(const std::string& filename) {
	ifstream inFile(filename.c_str());
	if (inFile.is_open()) {

		CSCDescriptors* temp = this->fromStream(&inFile);
		if(!temp) {	std::cerr << "CSCDescriptors::readFromFile()::ERROR::Failed reading fromStream() file[" << filename << "]!\n"; return false; }
/*
		std::string temp;
		getline(inFile, temp); 
		
		if(temp == this->type()) {
			CSCDescriptors* temp = this->fromStream(&inFile);
			if(!temp) {	std::cerr << "CSCDescriptors::readFromFile()::ERROR::Failed reading fromStream()!\n"; return false; }
		}
		else{
			std::cerr << "CSCDescriptors::readFromFile()::ERROR::File type [" << temp << "] is different from fl type [" << type() << "]\n";
			return false;
		}
*/		
		inFile.close();
		return true;
	}
	else {
		std::cerr << "CSCDescriptors::readFromFile()::ERROR::Could not open [" << filename << "] for reading!\n";
		return false;
	}

}

void CSCDescriptors::print() {
	std::cout << "=====================================\n";
	std::cout << type() << "\n";
	std::cout << descriptortype << "\n";
	std::cout << "size = [" << height << " x " << width << "]\n";
	std::cout << "step = " << step << "\n";
	std::cout << "heightused = " << heightused << "\n";

	float* tempregions = NULL;
	if(regions) tempregions = regions->getData();
	if(tempregions) std::cout << "[regions data] = [\n";
	else std::cout << "[data] = [\n";

	unsigned int index1 = 0, index2 = 0;
	for(unsigned int j = 0;j<heightused;j++) {
		if(tempregions != NULL) {
			unsigned int regionwidth = regions->getWidth();
			for(unsigned int i = 0;i<regionwidth;i++) {
				std::cout << tempregions[index1] << " ";
				index1++;
			}
			std::cout << " : ";
			index1 += ((regions->getStepsize() - 4*regionwidth)/4);
		}
		unsigned int descwidth = this->getWidth();
		for(unsigned int i = 0;i<descwidth;i++) {
			std::cout << fl[index2] << " ";
			index2++;
		}
		std::cout << ";\n";

		index2 += ((this->getStepsize() - 4*descwidth)/4);
	}
	std::cout << "]\n";

	std::cout << "=====================================\n";
}

float* CSCDescriptors::allocate(unsigned int h, unsigned int w) {
	heightused = 0;
	float* fl = CSCMatrix2D::allocate(h, w);
	return fl;
}

void CSCDescriptors::deallocate() {
	heightused = 0;
	CSCMatrix2D::deallocate();
}

void CSCDescriptors::normalize() {
	unsigned int index = 0;
//	unsigned int descwidth = this->getWidth();
	
	for(unsigned int i = 0;i<heightused;i++) {
		float sum = 0.0;
		float* tempfl = &(fl[index]);
		for(unsigned int j = 0;j<width;j++) {
			float temp = tempfl[j];
			sum += temp * temp;
			index++;
		}
		
		if(sum > 0.0) {
			sum = sqrt(sum);
			for(unsigned int j = 0;j<width;j++) {
				tempfl[j] /= sum;
			}
		}

		index += ((step - 4*width)/4);
	}
}

/*
float* CSCDescriptors::allocate(unsigned int height, unsigned int width, unsigned int tempstepsize) {
	heightused = 0;
	float* fl = CSCMatrix2D::allocate(height, width, tempstepsize);
	return fl;
}
*/

