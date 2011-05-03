#ifndef CSCMatrix2D_H
#define CSCMatrix2D_H

#include <string.h>
#include <iostream>

#include "CSCData.h"


	//!the matrix class
	class CSCMatrix2D : public CSCData {

		private:    

		protected:    

			union {
				unsigned int rows;
				unsigned int height;
			};
			union {
				unsigned int cols;
				unsigned int width;
			};
			//unsigned int size[2]; // height: height or rows, width: width or columns, 

			unsigned int step; // step size in bytes, IPP uses matrix with row aligned by 32 bytes, OpenCV aligned by 4 bytes
			unsigned int dim;

			// fl pointers
			float* fl;
			bool heapallocated;

		public: 

			//constructor
			CSCMatrix2D();

			//destructor
			//virtual ~CSCMatrix2D();
			~CSCMatrix2D();

			//copy constructor
			CSCMatrix2D(const CSCMatrix2D&);

			//copy assignment operator
			CSCMatrix2D& operator=(const CSCMatrix2D&); 

			//constructor with parameters
			//CSCMatrix2D(unsigned int h, unsigned int w);
			CSCMatrix2D(unsigned int h, unsigned int w, unsigned int s = 0);

			//returns size of the heap memory allocated in bytes
			virtual unsigned int size();

			//returns the type of fl
			virtual std::string type() { return "CMat2D"; }  

			//serializes the fl into stream (for network transfer, file saving, etc)
			virtual void toStream(std::ostream* str);  

			//recreates the fl from stream
			virtual CSCMatrix2D* fromStream(std::istream* str);  

			virtual void writeToFile(const std::string& filename);

			virtual bool readFromFile(const std::string& filename);

			virtual void print();

			float* getData() { return fl; }

			void copyData(float* input, unsigned int h, unsigned int w, unsigned int s = 0);

			void pointData(float* input, unsigned int h, unsigned int w, unsigned int s = 0);

			unsigned int getWidth() { return width; }
			unsigned int getCols() { return width; }

			unsigned int getHeight() { return height; }
			unsigned int getRows() { return height; }

			unsigned int getStepsize() { return step; }

			unsigned int getDim() { return dim; }

			virtual float* allocate(unsigned int h, unsigned int w, unsigned int s = 0);
//			virtual float* allocate(unsigned int h, unsigned int w);

			virtual void deallocate();

			void set(float input);

			void setZero() { this->set(0.0); }

			//void normalize();
			
			//CSCMatrix2D* transpose();

	};

#endif
