#include "CSCFeature.h"
#include "serializer.h"

using namespace std;


//std::string CSCFeature::m_type = "CF";

//contructor
CSCFeature::CSCFeature(CSCViewData* parent) : CSCViewData() {
	this->addParent(parent);
	parent->addChild(this);
}

//copy assignment operator
CSCFeature& CSCFeature::operator=(const CSCFeature& right) {
/*
	CSCData::operator=(right);

	unsigned int leftsize = width*height*channel;
	unsigned int rightsize = (right.width)*(right.height)*(right.channel);

	if(leftsize != rightsize) {
		if(data) delete [] data;
		data = new float [rightsize];
		if(!data) std::cerr << type() << "::Copy assignement operator()::ERROR::Memory allocation error! size = " << rightsize << "\n";
	}
	width = right.width;
	height = right.height;
	channel = right.channel;

	memcpy(data, right.data, rightsize*sizeof(float));
*/
	return *this;
}

float CSCFeature::similarity(const CSCFeature& right) {
	std::cerr << "CSCFeature::similarity()::BAD!  Don't want rfeature similarity\n";
	return 0.0;
}


