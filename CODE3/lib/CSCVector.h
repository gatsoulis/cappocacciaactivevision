#ifndef CSCVector_H
#define CSCVector_H

#include <string.h>
#include <iostream>

#include "CSCData.h"


	//!the matrix class
	class CSCVector : public CSCData {

		private:    

		protected:    

			float* fl;
			unsigned int width;
			bool heapallocated;
/*
			unsigned int type; // CSCData Type
			unsigned int step; // full row-length in bytes

			union {
				unsigned int width;
				unsigned int height;
				unsigned int size;
			};

			// data pointers
			union {
				unsigned char* ptr;
				short* s;
				int* i;
				float* fl;
				double* db;
			} data;
			
			bool createdhere;
*/
		public: 

			//constructor
			CSCVector();

			//destructor
			virtual ~CSCVector();

			//copy constructor
			CSCVector(const CSCVector&);

			//copy assignment operator
			CSCVector& operator=(const CSCVector&); 

			// Compound operator
			CSCVector& operator+=(const CSCVector&); 
			CSCVector& operator-=(const CSCVector&); 

			//constructor with initial parameter
			CSCVector(unsigned int size);

			//returns size of the heap memory allocated in bytes
			virtual unsigned int size();

			//returns the type of data
			virtual std::string type() { return "CVec"; }  

			// serializes the data into stream (for network transfer, file saving, etc)
			virtual void toStream(std::ostream* str);  

			// recreates the data from stream
			virtual CSCVector* fromStream(std::istream* str);  

			virtual void writeToFile(const std::string& filename);

			virtual bool readFromFile(const std::string& filename);

			virtual void print();

			//computes similarity between data
			//virtual float similarity(const CSCVector&) { std::cout << "Current data type [" << type() << "] does not have the similarity function defined!\n"; return 0.0f; }

			float* getData() { return fl; }
			
			void copyData(float* input, unsigned int size);

			// This will delete any allocated heap space
			void pointData(float* input, unsigned int size);
			
			unsigned int getWidth() { return width; }
			unsigned int getSize() { return width; }

			virtual float* allocate(unsigned int size);
			void deallocate();

			void set(float input);
			void setZero() { this->set(0.0); }

			void normalize();
			
			void findMin(float* min, unsigned int* pos = NULL);
			void findMax(float* max, unsigned int* pos = NULL);
			void findMinMax(float* min, float* max, unsigned int* minpos = NULL, unsigned int* maxpos = NULL);

			unsigned int countZero();
			unsigned int countNonZero();
			unsigned int countBelowZero();
			unsigned int countAboveOrEqualToZero();
			unsigned int countAboveZero();
			unsigned int countBelowOrEqualToZero();

			float dotproduct(const CSCVector& right);
			//CSCVector* crossproduct(const CSCVector& right);
			

	};


#endif
