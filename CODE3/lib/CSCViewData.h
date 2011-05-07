#ifndef CSCViewData_H
#define CSCViewData_H

#include <string>
#include <iostream>
#include <list>
//#include <vector>

#include "CSCData.h"


	class CSCViewData;

	typedef std::list<CSCViewData* > CVDConnectType;

	//!the base class for node data types in object recognition
	class CSCViewData : public CSCData {
	
		public: 

			//constructor
			CSCViewData();

			//destructor
			virtual ~CSCViewData();

			//copy constructor
			CSCViewData(const CSCViewData&);

			//copy assignment operator
			CSCViewData& operator=(const CSCViewData&); 

			//return the number of each data type per pixel
			virtual unsigned int size() { return 0; } 

			//Returns the type of data
			virtual std::string type() { return "CVD"; }  

			//Serializes the data into stream (for network transfer, file saving, etc)
			virtual void toStream(std::ostream* str);  

			//Recreates the data from stream
			virtual CSCViewData* fromStream(std::istream* str);  

			void writeToFile(const std::string& filename);

			bool readFromFile(const std::string& filename);

			//computes similarity between data
			virtual float similarity(const CSCViewData&) { std::cout << "CSCViewData::is the base-class for all derived data types. function similarity() not defined!\n"; return -1.0; }
			
			virtual void print() { std::cout << "CSCViewData::is the base-class for all derived data types. function print() not defined!\n"; }

			CVDConnectType* getParents() { return &parents; }
			CVDConnectType* getChilds() { return &childs; }
			CVDConnectType* getNeighbors() { return &neighbors; }

			void clearParents() { parents.clear(); }
			void clearChilds() { childs.clear(); }
			void clearNeighbors() { neighbors.clear(); }

			void setParents(CVDConnectType* tempparents) { parents = *tempparents; }
			void setChilds(CVDConnectType* tempchilds) { childs = *tempchilds; }
			void setNeighbors(CVDConnectType* tempneighbors) { neighbors = *tempneighbors; }

			CSCViewData* getParent(unsigned int index);
			CSCViewData* getChild(unsigned int index);
			CSCViewData* getNeighbor(unsigned int index);

			unsigned int getParentsSize() { return parents.size(); }
			unsigned int getChildsSize() { return childs.size(); }
			unsigned int getNeighborsSize() { return neighbors.size(); }

			CSCViewData* addParent(CSCViewData* parent) { parents.push_back(parent); return parent; }
			CSCViewData* addChild(CSCViewData* child) { childs.push_back(child); return child; }
			CSCViewData* addNeighbor(CSCViewData* neighbor) { neighbors.push_back(neighbor); return neighbor; }

			bool removeParent(CSCViewData* parent);
			bool removeChild(CSCViewData* child);
			bool removeNeighbor(CSCViewData* neighbor);

			bool updateParent(CSCViewData* oldparent, CSCViewData* newparent);
			bool updateChild(CSCViewData* oldchild, CSCViewData* newchild);
			bool updateNeighbor(CSCViewData* oldneighbor, CSCViewData* newneighbor);

			bool removePtrToThisFromParents(CSCViewData* input);
			bool removePtrToThisFromChilds(CSCViewData* input);
			bool removePtrToThisFromNeighbors(CSCViewData* input);
			bool removePtrToThis(CSCViewData* input);

			const unsigned int& getIndex() { return index; }
			void setIndex(unsigned int input) { index = input; }

			//virtual void merge(CSCViewData* right);
			
			//virtual float* allocate(unsigned int size);
			//virtual void deallocate();
			
			//virtual void setData(float* input);
			//virtual float* getData();
			
			virtual CVDConnectType* evaluate() { std::cout << "CSCViewData::is the base-class for all derived data types. function evaluate() not defined!\n"; return NULL;}

		private:    
			//static std::string m_type;

		protected:    
			unsigned int index;

			CVDConnectType parents;
			CVDConnectType childs;
			CVDConnectType neighbors;

	};

#endif
