#ifndef CSCDescriptors_H
#define CSCDescriptors_H

#include <string.h>
#include <iostream>

#include "CSCMatrix2D.h"
#include "CSCEllipticRegions.h"


	//!the elliptic regions class
	class CSCDescriptors : public CSCMatrix2D {

		private:    

		protected:    
			unsigned int heightused;
			std::string descriptortype;
			CSCEllipticRegions* regions;
			bool regions_createdhere;

		public: 
			//constructor
			CSCDescriptors();
			//destructor
			//virtual ~CSCDescriptors();
			~CSCDescriptors();
			//copy constructor
			CSCDescriptors(const CSCDescriptors&);
			//copy assignment operator
			CSCDescriptors& operator=(const CSCDescriptors&); 

			//constructor with parameters
			CSCDescriptors(unsigned int h, unsigned int w, const std::string& temptype);

			//returns size of the heap memory allocated in bytes
			//virtual unsigned int size();
			//returns the type of fl
			virtual std::string type() { return "CDesc"; }  
			//serializes the fl into stream (for network transfer, file saving, etc)
			virtual void toStream(std::ostream* str);  
			//recreates the fl from stream
			virtual CSCDescriptors* fromStream(std::istream* str);  

			virtual void writeToFile(const std::string& filename);

			virtual bool readFromFile(const std::string& filename);

			//computes similarity between fl
			//virtual float similarity(const CSCDescriptors&);

			virtual void print();

			unsigned int getCurrentSize() { return heightused; }
			void setCurrentSize(unsigned int input) { this->heightused = input; }

			//returns the type of descriptor
			const std::string& getDescriptorType() { return descriptortype; }  
			void setDescriptorType(std::string input) { descriptortype = input; }  

			//returns the type of descriptor
			CSCEllipticRegions* getRegions() { return regions; }  
			void setRegions(CSCEllipticRegions* input) { regions = input; regions_createdhere = false; }  
			void setRegionsCreatedHere(bool temp) { regions_createdhere = temp; }

			float* allocate(unsigned int h, unsigned int w);

			void deallocate();

			void normalize();

			bool debug;
	};

#endif
