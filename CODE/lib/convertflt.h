#ifndef ConvertFlt_H
#define ConvertFlt_H

#include "cv.h"
#include "cvimage.h"

//#include <vector>
//#include <sstream>
//#include <fstream>



	class ConvertFlt {
	
		public:
			ConvertFlt();
			~ConvertFlt();

			CVImage* convert(CVImage* img);
			
		private:

			CVImage* mp_cvoutputimg;

	};


#endif


