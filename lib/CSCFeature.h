#ifndef CSCFeature_H
#define CSCFeature_H

#include <string>
#include <iostream>
#include <list>

#include "CSCViewData.h"
//#include "CSCTreeSearch.h"   AGGIUNGERE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//	class CSCDatabase;  AGGIUNGERE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1

	//!the base class for different feature types (region descriptors)
	class CSCFeature : public CSCViewData {

		private:    

		protected:    
			float* data;
			long int count;

		public: 
			//constructor
			CSCFeature() {}
			//destructor
			virtual ~CSCFeature() {}
			//copy constructor
			CSCFeature(const CSCFeature&) {}
			//copy assignment operator
			CSCFeature& operator=(const CSCFeature&);
			//constructor
			CSCFeature(CSCViewData* parent);
			//returns size of the heap memory allocated in bytes
			virtual unsigned int size() { return 0; }
			//returns the type of data
			//static const std::string& type() { return m_type; }  
			virtual std::string type() { return "CF"; }  
			//serializes the data into stream (for network transfer, file saving, etc)
			virtual void toStream(std::ostream* str) {}
			//recreates the data from stream
			virtual CSCViewData* fromStream(std::istream* str) { return NULL;}
			//computes similarity between data
			virtual float similarity(const CSCFeature&);

			virtual void print() {}

			virtual void point(float* input) { data = input; }

			virtual void allocate() {}

			virtual void deallocate() {}

			virtual void copy(float* input, unsigned int size) {}

			//virtual void nearestNeighbors(CSCDatabase* database, CSCFeature** dst, int k = 1, float eps = 1.0, float* dist = NULL) {}   AGGIUNGERE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

			float* getData() { return data; }

//			virtual CSCTreeSearch* getTree() { return NULL; }   AGGIUNGERE!!!!!!!!!!!!!!!!!!!
			//static void setTree(CSCTreeSearch* input) {}
//			virtual CSCTreeSearch* buildTree(CSCDatabase* database) { std::cerr << "CSCFeature::buildTree()::ERROR::Function not defined for base-class!\n"; return NULL; }   AGGIUNGERE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

			long int getCount() { return count; }
			void setCount(long int input) { count = input; }
			void incCount(long int val = 1) { count += val; }
			void decCount(long int val = 1) { count -= val; }

	};

#endif
