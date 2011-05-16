#ifndef DrawVector_H
#define DrawVector_H

#include "cv.h"
//#include "highgui.h"

#include "module.h"
#include "inputconnector.h"
#include "outputconnector.h"
#include "CSCVector.h"
#include "cvimage.h"

#include <string>




//! class DrawVector  -- DrawVector Module. Connectors: InputConnector<float*> dataIn
/*! class DrawVector  -- DrawVector Module. Connectors: InputConnector<float*> dataIn
*/
class DrawVector : public Module
{
	public:
	
		DrawVector(std::string name="DrawVector", unsigned int dim = 0);
		~DrawVector();

		InputConnector<float* > dataFltIn;
		InputConnector<double* > dataDblIn;
		InputConnector<CSCVector* > dataIn;

		OutputConnector<CVImage*> cvImageOut;

		void execute();
		void setDim(unsigned int dim);
		
		bool debug;

		void setParam(bool vert, int w, int h, int mode, float c);

	private:

		unsigned int m_dim;
		bool vertical;
		int width, height;
		int normalize_mode; // 0 for no normalization, 1 for normalize with a fixed constant, 2 for normalize with min & max
		float norm_constant;

		CVImage* mp_cvimage;
	
};


#endif
