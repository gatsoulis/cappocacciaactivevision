#ifndef Convert32FTo8U_H
#define Convert32FTo8U_H

#include "cv.h"

#include "module.h"
#include "inputconnector.h"
#include "outputconnector.h"
#include "cvimage.h"

#include <iostream>


  
	class Convert32FTo8U : public Module{

		public:
			Convert32FTo8U(std::string name="Convert32FTo8U");
			~Convert32FTo8U(); 
			void execute(); 

			InputConnector<CVImage*> cvImageIn;
			OutputConnector<CVImage*> cvImageOut; 

			bool debug;

		private:

			CVImage* mp_cvimg8u;
			//CVImage* mp_cvimg32f;

  };


#endif

