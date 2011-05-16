#include "CSCViewData.h"
#include "serializer.h"
#include <fstream>

using namespace std;


CSCViewData::CSCViewData() : CSCData() {
	index = 0;
	parents.clear();
	childs.clear();
	neighbors.clear();
}

CSCViewData::~CSCViewData() {
	parents.clear();
	childs.clear();
	neighbors.clear();
}

//copy constructor
CSCViewData::CSCViewData(const CSCViewData& right) : CSCData(right) {
	index = right.index;
	parents = right.parents;
	childs = right.childs;
	neighbors = right.neighbors;
}

//copy assignment operator
CSCViewData& CSCViewData::operator=(const CSCViewData& right) {
	CSCData::operator=(right);

	index = right.index;
	parents = right.parents;
	childs = right.childs;
	neighbors = right.neighbors;
	return *this;
}

bool CSCViewData::removeParent(CSCViewData* parent) {
	CVDConnectType::iterator enditer = parents.end();
	for(CVDConnectType::iterator iter = parents.begin(); iter != enditer; ++iter) {
		CSCViewData* ptr = *iter;
		if(parent == ptr) {
			parents.erase(iter);
			return true;
		}
	}
	return false;
}

bool CSCViewData::removeChild(CSCViewData* child) {
	CVDConnectType::iterator enditer = childs.end();
	for(CVDConnectType::iterator iter = childs.begin(); iter != enditer; ++iter) {
		CSCViewData* ptr = *iter;
		if(child == ptr) {
			childs.erase(iter);
			return true;
		}
	}
	return false;
}

bool CSCViewData::removeNeighbor(CSCViewData* neighbor) {
	CVDConnectType::iterator enditer = neighbors.end();
	for(CVDConnectType::iterator iter = neighbors.begin(); iter != enditer; ++iter) {
		CSCViewData* ptr = *iter;
		if(neighbor == ptr) {
			neighbors.erase(iter);
			return true;
		}
	}
	return false;
}

bool CSCViewData::removePtrToThisFromParents(CSCViewData* input) {
	CVDConnectType::iterator enditer = parents.end();
	for(CVDConnectType::iterator iter = parents.begin(); iter != enditer; ++iter) {
		CSCViewData* parent = *iter;
		bool temp = parent->removeChild(input);
		if(!temp) {
			std::cerr << "Removing pointer to this from parents failed!\n";
			return false;
		}
	}
	return true;
}

bool CSCViewData::removePtrToThisFromChilds(CSCViewData* input) {
	CVDConnectType::iterator enditer = childs.end();
	for(CVDConnectType::iterator iter = childs.begin(); iter != enditer; ++iter) {
		CSCViewData* child = *iter;
		bool temp = child->removeParent(input);
		if(!temp) {
			std::cerr << "Removing pointer to this from childs failed!\n";
			return false;
		}
	}
	return true;
}

bool CSCViewData::removePtrToThisFromNeighbors(CSCViewData* input) {
	CVDConnectType::iterator enditer = neighbors.end();
	for(CVDConnectType::iterator iter = neighbors.begin(); iter != enditer; ++iter) {
		CSCViewData* neighbor = *iter;
		bool temp = neighbor->removeNeighbor(input);
		if(!temp) {
			std::cerr << "Removing pointer to this from neighbors failed!\n";
			return false;
		}
	}
	return true;
}

bool CSCViewData::removePtrToThis(CSCViewData* input) {
	bool temp;
	temp = this->removePtrToThisFromParents(input);
	if(!temp) return false;
	temp = this->removePtrToThisFromChilds(input);
	if(!temp) return false;
	temp = this->removePtrToThisFromNeighbors(input);
	if(!temp) return false;

	return true;
}

//Serializes the data
void CSCViewData::toStream(std::ostream* str) {
    *str << type() << std::endl;
	CSCViewData* tempptr = this;
	blockToStream(str, (char*)&tempptr, sizeof(tempptr));
	CSCViewData* ptr;
	unsigned int length = parents.size();
    blockToStream(str, (char*)&length, sizeof(length));
	CVDConnectType::iterator enditer = parents.end();
	for(CVDConnectType::iterator iter = parents.begin(); iter != enditer; ++iter) {
		ptr = *iter;
		blockToStream(str, (char*)&ptr, sizeof(ptr));
    }
	length = childs.size();
    blockToStream(str, (char*)&length, sizeof(length));
	enditer = childs.end();
	for(CVDConnectType::iterator iter = childs.begin(); iter != enditer; ++iter) {
		ptr = *iter;
		blockToStream(str, (char*)&ptr, sizeof(ptr));
    }
	length = neighbors.size();
    blockToStream(str, (char*)&length, sizeof(length));
	enditer = neighbors.end();
	for(CVDConnectType::iterator iter = neighbors.begin(); iter != enditer; ++iter) {
		ptr = *iter;
		blockToStream(str, (char*)&ptr, sizeof(ptr));
    }

}  

//Recreates the data
CSCViewData* CSCViewData::fromStream(std::istream* str) {
	parents.clear();
	childs.clear();
	neighbors.clear();

	CSCViewData* oldptr = NULL;
	blockFromStream(str, (char*)&oldptr, sizeof(oldptr)); // stored ptr this

	CSCViewData* ptr;
	unsigned int length;
	blockFromStream(str, (char*)&length, sizeof(length));
    for(unsigned int i = 0; i < length; i++) {
		blockFromStream(str, (char*)&ptr, sizeof(ptr));
		parents.push_back(ptr);
	}
	blockFromStream(str, (char*)&length, sizeof(length));
    for(unsigned int i = 0; i < length; i++) {
		blockFromStream(str, (char*)&ptr, sizeof(ptr));
		childs.push_back(ptr);
	}
	blockFromStream(str, (char*)&length, sizeof(length));
    for(unsigned int i = 0; i < length; i++) {
		blockFromStream(str, (char*)&ptr, sizeof(ptr));
		neighbors.push_back(ptr);
	}
	return oldptr;
}

void CSCViewData::writeToFile(const std::string& filename) {
	ofstream outFile(filename.c_str());
	if(outFile.is_open()) {
		this->toStream(&outFile);
		outFile.close();
	}
	else std::cerr << "CSCViewData::writeToFile()::ERROR::Could not open [" << filename << "] for writing!\n";
}

bool CSCViewData::readFromFile(const std::string& filename) {
	ifstream inFile(filename.c_str());
	if (inFile.is_open()) {
		std::string temp;
		getline(inFile, temp); 
		
		if(temp == this->type()) {
			CSCViewData* temp = this->fromStream(&inFile);
			if(!temp) {	std::cerr << "CSCViewData::readFromFile()::ERROR::Failed reading fromStream()!\n"; return false; }
		}
		else{
			std::cerr << "CSCViewData::readFromFile()::ERROR::File type [" << temp << "] is different from data type [" << type() << "]\n";
			return false;
		}
		
		inFile.close();
		return true;
	}
	else {
		std::cerr << "CSCViewData::readFromFile()::ERROR::Could not open [" << filename << "] for reading!\n";
		return false;
	}
}

CSCViewData* CSCViewData::getParent(unsigned int index) {
	CVDConnectType::iterator iter = parents.begin();
	for(unsigned int i = 0;i<index;i++) ++iter;
	CSCViewData* ptr = *iter;
	return ptr;
}

CSCViewData* CSCViewData::getChild(unsigned int index) {
	CVDConnectType::iterator iter = childs.begin();
	for(unsigned int i = 0;i<index;i++) ++iter;
	CSCViewData* ptr = *iter;
	return ptr;
}

CSCViewData* CSCViewData::getNeighbor(unsigned int index) {
	CVDConnectType::iterator iter = neighbors.begin();
	for(unsigned int i = 0;i<index;i++) ++iter;
	CSCViewData* ptr = *iter;
	return ptr;
}

bool CSCViewData::updateParent(CSCViewData* oldparent, CSCViewData* newparent) {
	CVDConnectType::iterator enditer = parents.end();
	for(CVDConnectType::iterator iter = parents.begin(); iter != enditer; ++iter) {
		CSCViewData* ptr = *iter;
		if(ptr == oldparent) {
			*iter = newparent;
			return true;
		}
	}
	return false;
}

bool CSCViewData::updateChild(CSCViewData* oldchild, CSCViewData* newchild) {
	CVDConnectType::iterator enditer = childs.end();
	for(CVDConnectType::iterator iter = childs.begin(); iter != enditer; ++iter) {
		CSCViewData* ptr = *iter;
		if(ptr == oldchild) {
			*iter = newchild;
			return true;
		}
	}
	return false;
}

bool CSCViewData::updateNeighbor(CSCViewData* oldneighbor, CSCViewData* newneighbor) {
	CVDConnectType::iterator enditer = neighbors.end();
	for(CVDConnectType::iterator iter = neighbors.begin(); iter != enditer; ++iter) {
		CSCViewData* ptr = *iter;
		if(ptr == oldneighbor) {
			*iter = newneighbor;
			return true;
		}
	}
	return false;
}


/*
void CSCViewData::updatePtrToThisFromParents(CSCViewData* oldptr) {
	CSCViewData* currentchildptr = this;
	std::cout << "CSCViewData::updatePtrToThisFromParents()::currentchildptr = " << currentchildptr << "\n";
	for(CVDConnectType::iterator iter = parents.begin(); iter != parents.end(); ++iter) {
		CSCViewData* parent = *iter;
		parent->print();
		std::cout << "CSCViewData::updatePtrToThisFromParents()::oldptr = " << oldptr << ", currentchildptr = " << currentchildptr << "\n";
		parent->updateChild(oldptr, currentchildptr);
		//(*iter)->updateChild(oldptr, currentchildptr);
	}
}

void CSCViewData::updateChild(CSCViewData* oldchild, CSCViewData* newchild) {
	for(CVDConnectType::iterator iter = childs.begin(); iter != childs.end(); ++iter) {
		CSCViewData* tempchild = *iter;
		tempchild->print();
		std::cout << "CSCViewData::updateChild()::tempchild = " << tempchild << ", oldchild = " << oldchild << ", newchild = " << newchild << "\n";
		if( tempchild == oldchild ) {
			(*iter) = newchild;
			break;
		}
		std::cout << "CSCViewData::updateChild()::(*iter) = " << (*iter) << "\n";
	}
}

void CSCViewData::updatePtrToThisFromChilds(CSCViewData* oldptr) {
	CSCViewData* currentparentptr = this;
	for(CVDConnectType::iterator iter = childs.begin(); iter != childs.end(); ++iter) {
		(*iter)->updateParent(oldptr, currentparentptr);
	}
}

void CSCViewData::updateParent(CSCViewData* oldparent, CSCViewData* newparent) {
	for(CVDConnectType::iterator iter = parents.begin(); iter != parents.end(); ++iter) {
		CSCViewData* tempparent = *iter;
		if( tempparent == oldparent ) {
			(*iter) = newparent;
			break;
		}
	}
}

void CSCViewData::updatePtrToThisFromNeighbors(CSCViewData* oldptr) {
	CSCViewData* currentneighborptr = this;
	for(CVDConnectType::iterator iter = neighbors.begin(); iter != neighbors.end(); ++iter) {
		(*iter)->updateNeighbor(oldptr, currentneighborptr);
	}
}

void CSCViewData::updateNeighbor(CSCViewData* oldneighbor, CSCViewData* newneighbor) {
	for(CVDConnectType::iterator iter = neighbors.begin(); iter != neighbors.end(); ++iter) {
		CSCViewData* tempneighbor = *iter;
		if( tempneighbor == oldneighbor ) {
			(*iter) = newneighbor;
			break;
		}
	}
}
*/



