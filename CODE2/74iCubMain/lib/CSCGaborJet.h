#ifndef CSCGaborJet_H
#define CSCGaborJet_H

#include <string.h>
#include <iostream>
#include <list>

#include "CSCFeature.h"
//#include "CSCDatabase.h"				AGGIUNGERE!!!!!!!!!!!!!!!!!! 3
//#include "CSCTreeSearch.h"
//#include "csctype.h"

#define CSCGaborJet_datasize 40


	//!the base class for different feature types (region descriptors)
	class CSCGaborJet : public CSCFeature {

		private:    
//			static CSCTreeSearch m_theTree;			AGGIUNGERE!!!!
			//CSCTreeSearch* mp_theTree;

		protected:    
			bool allocatedhere;

		public: 
			//constructor
			CSCGaborJet();
			//destructor
			virtual ~CSCGaborJet();
			//copy constructor
			CSCGaborJet(const CSCGaborJet&);
			//copy assignment operator
			CSCGaborJet& operator=(const CSCGaborJet&);

			//returns size of the heap memory allocated in bytes
			virtual unsigned int size() { return CSCGaborJet_datasize*sizeof(float); }
			//returns the type of data
			//static const std::string& type() { return m_type; }  
			virtual std::string type() { return "GJ"; }  
			//serializes the data into stream (for network transfer, file saving, etc)
			virtual void toStream(std::ostream* str);
			//recreates the data from stream
			virtual CSCViewData* fromStream(std::istream* str);
			//computes similarity between data
			virtual float similarity(const CSCFeature&);

			virtual void print();

			virtual void normalize();

			virtual void allocate();
			
			virtual void deallocate();
			
			virtual void point(float* input);
			
			virtual void copy(float* input, unsigned int size);

//AGGINGERE			virtual void nearestNeighbors(CSCDatabase* database, CSCFeature** dst, int k = 1, float eps = 1.0, float* dist = NULL);
			
//AGGIUNGERE			virtual CSCTreeSearch* getTree() { return &m_theTree; }
			//static void setTree(CSCTreeSearch* input) { m_theTree = *input; }
//AGGIUNGERE			virtual CSCTreeSearch* buildTree(CSCDatabase* database) { std::cerr << "CSCGaborJet::buildTree()::ERROR::Function not defined for base-class!\n"; return NULL; }

	};


#endif
