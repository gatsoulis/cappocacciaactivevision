#include "serializer.h"



void blockToStream(std::ostream* str, char* data, long size) {
	str->write(data, size);
}

void blockFromStream(std::istream* str, char* data, long size){
	str->read(data, size);
}

/*
void csc::toStream(float* data, std::ostream* str) {
	blockToStream(str, (char*)data, sizeof(*data));
	*str << std::endl;
}

bool csc::fromStream(float** data, std::istream* str) {
	if (!(*data)) *data = new float;
	blockFromStream(str, (char*)*data, sizeof(**data));
	std::string name;
	getline(*str,name);
	return true;
}
*/
