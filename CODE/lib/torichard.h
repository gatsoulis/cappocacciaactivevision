#ifndef ToRichard_H
#define ToRichard_H

#include "cv.h"
#include "highgui.h"

#include "module.h"
#include "inputconnector.h"
#include "outputconnector.h"
#include "cvimage.h"
#include "csctype.h"
#include "CSCEllipticRegions.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#define CROSS_HAIR_SIZE 3


  
	class ToRichard : public Module{

		public:
			ToRichard(std::string name="ToRichard");
			~ToRichard();
			void execute(); 
			void release();




			InputConnector<CSCEllipticRegions*> regionsIn;
			OutputConnector<int*> arrayOut;
			OutputConnector<int*> sizearrayOut;

			bool debug;

		private:

			int* array;
			int* arraysize;



  };


#endif

