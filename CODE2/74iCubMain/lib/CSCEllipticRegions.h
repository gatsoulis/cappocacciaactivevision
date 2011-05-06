#ifndef CSCEllipticRegions_H
#define CSCEllipticRegions_H

#include <string.h>
#include <iostream>

#include "CSCMatrix2D.h"


	//!the elliptic regions class
	class CSCEllipticRegions : public CSCMatrix2D {

		private:    

		protected:    
			unsigned int heightused;
			std::string detectortype;

		public: 
			//constructor
			CSCEllipticRegions();
			//destructor
			~CSCEllipticRegions();
			//copy constructor
			CSCEllipticRegions(const CSCEllipticRegions&);
			//copy assignment operator
			CSCEllipticRegions& operator=(const CSCEllipticRegions&); 

			//constructor with parameters
			CSCEllipticRegions(unsigned int h, unsigned int w);

			//returns size of the heap memory allocated in bytes
			//virtual unsigned int size();

			//returns the type of fl
			std::string type() { return "CEll"; }  

			//serializes the fl into stream (for network transfer, file saving, etc)
			void toStream(std::ostream* str);  

			//recreates the fl from stream
			CSCEllipticRegions* fromStream(std::istream* str);  

			void writeToFile(const std::string& filename);

			bool readFromFile(const std::string& filename);

			//computes similarity between fl
			//virtual float similarity(const CSCEllipticRegions&) { std::cout << "Current data type [" << type() << "] does not have the similarity function defined!\n"; return 0.0f; }

			void print();

			unsigned int getCurrentSize() { return heightused; }
			void setCurrentSize(unsigned int val) { this->heightused = val; }

			float* allocate(unsigned int h, unsigned int w);

			void deallocate();

			void setDetectorType(const std::string& type) { detectortype = type; }
			
			const std::string& getDetectorType() { return detectortype; }

	};

#endif
