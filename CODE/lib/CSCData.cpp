#include "CSCData.h"
#include "serializer.h"
#include <fstream>

using namespace std;


//copy assignment operator
CSCData& CSCData::operator=(const CSCData& right) {
	return *this;
}

//Serializes the data
void CSCData::toStream(std::ostream* str) {
	*str << type() << std::endl;
}  

//Recreates the data
CSCData* CSCData::fromStream(std::istream* str) {

	std::string temp;
	getline(*str, temp); 
	if(temp != this->type()) {
		std::cerr << "CSCData::fromToStream()::ERROR::Stream type [" << temp << "] is different from data type [" << type() << "]\n";
		return false;
	}

	return this;
}

void CSCData::writeToFile(const std::string& filename) {
	ofstream outFile(filename.c_str());
	if(outFile.is_open()) {
		this->toStream(&outFile);
		outFile.close();
	}
	else std::cerr << "CSCData::writeToFile()::ERROR::Could not open [" << filename << "] for writing!\n";
}

bool CSCData::readFromFile(const std::string& filename) {
	ifstream inFile(filename.c_str());
	if (inFile.is_open()) {

		CSCData* temp = this->fromStream(&inFile);
		if(!temp) {	std::cerr << "CSCData::readFromFile()::ERROR::Failed reading fromStream() filename[" << filename << "]!\n"; return false; }
		
		inFile.close();
		return true;
	}
	else {
		std::cerr << "CSCData::readFromFile()::ERROR::Could not open [" << filename << "] for reading!\n";
		return false;
	}
}

