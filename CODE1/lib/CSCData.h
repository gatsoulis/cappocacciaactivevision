#ifndef CSCData_H
#define CSCData_H

#include <string>
#include <iostream>



	typedef enum {CDAT_UCHAR = 1, CDAT_SHORT = 2, CDAT_INT = 4, CDAT_FLT = 8, CDAT_DBL = 16} datatypes;

	//!the base class for data types
	class CSCData {

		private:    

		protected:    

		public: 

			// constructor
			CSCData() {}

			// destructor
			virtual ~CSCData() {}

			// copy constructor
			CSCData(const CSCData&) {}

			// copy assignment operator
			CSCData& operator=(const CSCData&); 

			// returns the number of bytes dynamically allocated in the heap space
			virtual unsigned int size() { return 0; } 

			// returns the type of data
			virtual std::string type() { return "CDat"; }  

			// serializes the data into stream (for network transfer, file saving, etc)
			virtual void toStream(std::ostream* str);

			// recreates the data from stream
			virtual CSCData* fromStream(std::istream* str);

			virtual void writeToFile(const std::string& filename);

			virtual bool readFromFile(const std::string& filename);

			// print the data to the terminal screen
			virtual void print() { std::cerr << "CSCData::print()::ERROR::Function not defined for the base class!\n"; }

	};


#endif
