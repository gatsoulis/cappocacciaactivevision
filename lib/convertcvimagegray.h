#ifndef ConvertCVImageGray_H
#define ConvertCVImageGray_H

#include "cv.h"
#include "highgui.h"

#include "cvimage.h"

#include <fstream>
#include <string>
#include <sys/stat.h>



	class ConvertCVImageGray {

	    public:

			//! ConvertCVImageGray();
			/*! ConvertCVImageGray()  -- the standard constructor
			*/
			ConvertCVImageGray();

			//! ~ConvertCVImageGray()  -- the destructor
			~ConvertCVImageGray();

			CVImage* convert(CVImage* input);
				

		private:
			
			CVImage* mp_cvgrayimg;
			

	};

#endif
