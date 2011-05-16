#include "CSCGaborJet.h"
#include "serializer.h"

using namespace std;


//CSCTreeSearch CSCGaborJet::m_theTree;			AGGIUNGERE!!!!!!!!!!!!!!!!!!
//std::string CSCGaborJet::m_type = "GJ";

//contructor
CSCGaborJet::CSCGaborJet() : CSCFeature() {
	data = NULL;
	allocatedhere = false;
	count = 0;
//	mp_theTree = &m_theTree;
}

//destructor
CSCGaborJet::~CSCGaborJet() {
	this->deallocate();
}

//copy contructor
CSCGaborJet::CSCGaborJet(const CSCGaborJet& right) : CSCFeature(right) {
	if(!allocatedhere) this->allocate();
	memcpy(data, right.data, this->size());
	this->parents = right.parents;
	this->childs = right.childs;
	this->neighbors = right.neighbors;
	this->count = right.count;
}

//copy assignment operator
CSCGaborJet& CSCGaborJet::operator=(const CSCGaborJet& right) {
	CSCGaborJet::operator=(right);
//	if(data != NULL) {
//		std::cerr << "CSCGaborJet::operator=::ERROR::The memory is already pointing to some other place !\n";
//		return;
//	}
	if(!allocatedhere) this->allocate();
	memcpy(data, right.data, this->size());
	this->parents = right.parents;
	this->childs = right.childs;
	this->neighbors = right.neighbors;
	this->count = right.count;
	return *this;
}

void CSCGaborJet::toStream(std::ostream* str) {
	//std::cout << "CSCGaborJet::toStream()::this = " << this << "\n";
	*str << type() << std::endl;
	blockToStream(str, (char*)&index, sizeof(unsigned int));
	blockToStream(str, (char*)&count, sizeof(long int));
//	CSCGaborJet* ptr = this;
//	blockToStream(str, (char*)ptr, sizeof(ptr));
	blockToStream(str, (char*)data, this->size());
	CSCViewData::toStream(str);
}

CSCViewData* CSCGaborJet::fromStream(std::istream* str) {
	//std::cout << "CSCGaborJet::fromStream()::\n";
//	std::string datatype;
//	getline(*str, datatype);
//	if(datatype != type()) std::cerr << type() << "::fromStream()::ERROR::data-type [" << datatype << "] do not match type[" << type() << "]!\n";

	blockFromStream(str, (char*)&index, sizeof(unsigned int));
	blockFromStream(str, (char*)&count, sizeof(long int));
//	CSCGaborJet* oldptr = NULL;
//	blockFromStream(str, (char*)oldptr, sizeof(oldptr));
	this->allocate();
	blockFromStream(str, (char*)data, this->size());
//	CSCViewData::fromStream(str);
//	return oldptr;
	// Discard next line
	std::string discard;
	getline(*str, discard);
	return CSCViewData::fromStream(str);
}

float CSCGaborJet::similarity(const CSCFeature& right) {
//	std::cout << "CSCGaborJet::similarity()\n";
    const CSCGaborJet* tempright = reinterpret_cast<const CSCGaborJet* >(&right);
	float* rightdata = tempright->data;
	float sim = 0.0;
	for(unsigned int i = 0;i<CSCGaborJet_datasize;i++) {
		sim += data[i] * rightdata[i];
	}
	// What if feature is not normalized??
	return sim;
}

void CSCGaborJet::normalize() {
	float sim = 0.0;
	for(unsigned int i = 0;i<CSCGaborJet_datasize;i++) {
		sim += data[i];
	}
	if(sim != 0.0) {
		for(unsigned int i = 0;i<CSCGaborJet_datasize;i++) {
			data[i] /= sim;
		}
	}
}

void CSCGaborJet::print() {
	std::cout << "=====================================\n";
	std::cout << type() << "\n";
	std::cout << "index = " << index << "\n";
	std::cout << "count = " << count << "\n";
	if(data != NULL) {
		std::cout << "data = [";
		for(unsigned int i = 0;i<CSCGaborJet_datasize;i++) {
			std::cout << data[i] << " ";
		}
		std::cout << "]\n";
	}
	if(parents.size() > 0){
		std::cout << "parent_size = " << parents.size() << "\n";
		std::cout << "parents = [";
		for(CVDConnectType::iterator iter = parents.begin(); iter != parents.end(); iter++) {
			std::cout << *iter << " ";
		}
		std::cout << "]\n";
	}
	if(childs.size() > 0){
		std::cout << "child_size = " << childs.size() << "\n";
		std::cout << "childs = [";
		for(CVDConnectType::iterator iter = childs.begin(); iter != childs.end(); iter++) {
			std::cout << *iter << " ";
		}
		std::cout << "]\n";
	}
	if(neighbors.size() > 0){
		std::cout << "neighbor_size = " << neighbors.size() << "\n";
		std::cout << "neighbors = [";
		for(CVDConnectType::iterator iter = neighbors.begin(); iter != neighbors.end(); iter++) {
			std::cout << *iter << " ";
		}
		std::cout << "]\n";
	}
	std::cout << "=====================================\n";
}

void CSCGaborJet::allocate() {
	if(allocatedhere == false) {
		//if(data != NULL) std::cerr << "CSCGaborJet::allocate()::ERROR::The memory is pointing to another place !\n";
		//else data = new float [CSCGaborJet_datasize];
		data = new float [CSCGaborJet_datasize];
		allocatedhere = true;
	}
	//else std::cerr << "CSCGaborJet::allocate()::ERROR::The memory is already allocated !\n";
}

void CSCGaborJet::deallocate() {
	if(allocatedhere == true && data != NULL) {
		delete [] data;
		data = NULL;
		allocatedhere = false;
	}
}

void CSCGaborJet::point(float* input) {
	this->deallocate();
	data = input;
}

void CSCGaborJet::copy(float* input, unsigned int size) {
	memcpy(data, input, size*sizeof(float));
}

/*  AGGIUNGERE DA QUI

void CSCGaborJet::nearestNeighbors(CSCDatabase* database, CSCFeature** dst, int k, float eps, float* dist) {
	std::cerr << "CSCGaborJet::nearestNeighbors()::ERROR::Do nothing function - redefined in the derived classes\n";

	for(int i = 0;i<k;i++) {
		dst[i] = NULL;
		//dist[i] = FLT_MAX;
	}
*/ //AGGIUNGERE FINO A QUI

/* do not remove
	if(!m_theTree.dataIsSet()) {
		//std::cout << "Setting the data for the tree\n";
		CSCGaborJet* currentJet;
		int whichNum = 0;
		int numDataPts = 0;
		CVDConnectType indices;

		std::list<CSCViewData*>* features = database->getFeatures();
		
		for(std::list<CSCViewData*>::iterator iter = features->begin(); iter != features->end();iter++) {
			if( (currentJet = dynamic_cast<CSCGaborJet*>(*iter)) ) {
				CSCViewData* temp = (CSCViewData*) (*iter);
				indices.push_back(temp);
				numDataPts++;
			}
		}

		ANNpointArray dataPts = annAllocPts(numDataPts, size() );
	
		for(std::list<CSCViewData*>::iterator iter = features->begin(); iter != features->end();iter++) {
			//std::cout << "(*iter)->type() = " << (*iter)->type() << "\n";
			//std::cout << "this->type() = " << this->type() << "\n";
			if( (*iter)->type() == this->type() ) {
				currentJet = (CSCGaborJet*) (*iter);

				memcpy(dataPts[whichNum], currentJet->data, sizeof(ANNcoord)*size() );

				//std::cout << "setdata[" << whichNum << "] = [";
				//for(unsigned int i = 0;i<size();i++) {
				//	std::cout << currentJet->data[i] << " ";
				//}
				//std::cout << "]\n";

				whichNum++;
			}
		}
		//std::cout << "size() = " << size() << "\n";
		//std::cout << "whichNum = " << whichNum << "\n";
		m_theTree.setData(size(), dataPts, indices);
	}

	m_theTree.findQuery((ANNcoord*)this->data, k, eps, (CSCViewData**)dst, dist); 
*/ //do not remove
// }  AGGIUNGERE!!!!!!!!!!!!!!!!!!!

