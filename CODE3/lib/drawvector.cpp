#include "drawvector.h"



DrawVector::DrawVector(std::string name, unsigned int dim) : Module(name) {
	//std::cout<<"Constructing " << getName() << "...";

	dataFltIn.setModule(this);
	dataDblIn.setModule(this);
	dataIn.setModule(this);

	m_dim = dim;
	vertical = true;
	width = 10;
	height = 10;
	mp_cvimage = NULL;

	normalize_mode = 2;
	norm_constant = 1.0;

	debug = false;
	//std::cout<<"finished.\n";
}
 
DrawVector::~DrawVector() {
	//std::cout<<"Destructing " << getName() << "...";
	if(mp_cvimage != NULL) delete mp_cvimage;
	//std::cout<<"finished.\n";
}

void DrawVector::execute() {

	float* dataFlt = dataFltIn.getBuffer();
	double* dataDbl = dataDblIn.getBuffer();
	CSCVector* vec = dataIn.getBuffer();
	if (!dataFlt && !dataDbl && !vec) { 
		std::cerr << getName() << "::ERROR::execute()::dataIn is NULL!\n"; 
		cvImageOut.setBuffer(NULL);
		cvImageOut.out();
		return; 
	}

	if(vec) m_dim = vec->getWidth();

	if(m_dim < 1)  { std::cerr << getName() << "::ERROR::execute()::dim<1. Call setParam() first!\n"; return; }

	if(!mp_cvimage) {
		if(vertical == true) {
			mp_cvimage = new CVImage(cvSize(width, height*m_dim), CV_8UC1, 0);
		} 
		else {
			mp_cvimage = new CVImage(cvSize(width*m_dim, height), CV_8UC1, 0);
		}
	}
	
	// Set the image to all zero (black)
	cvSetZero(mp_cvimage->ipl);

	if(dataFlt) {
		// If normalization mode is 2, find min & max
		bool notnecessary = false;
		float min = 100000, max = -100000;
		if(normalize_mode == 2) {
			for(unsigned int i = 0;i < m_dim;i++) {
				if(dataFlt[i] > max) max = dataFlt[i];
				if(dataFlt[i] < min) min = dataFlt[i];
			}
			if(max == min) {  notnecessary = true; }
		}
		else if(normalize_mode == 1) {
		}
		else if(normalize_mode == 0) {
			max = 1.0;
			min = 0.0;		
		}
		// Draw rectangles (vector)
		if(notnecessary == false) {
			CvPoint pt1, pt2;
			CvScalar color;
			float temp = max - min;
			if(temp == 0.0) temp = 1.0;
			for(unsigned int i = 0;i < m_dim;i++) {
				if(vertical == true) {
					pt1 = cvPoint(0, i*height);
					pt2 = cvPoint(pt1.x + width, pt1.y + height);
				}
				else{
					pt1 = cvPoint(i*height, 0);
					pt2 = cvPoint(pt1.x + width, pt1.y + height);
				}
				if(normalize_mode == 0){
					color = cvScalarAll(dataFlt[i]);
				}
				else if(normalize_mode == 1){
					color = cvScalarAll( cvRound(dataFlt[i] * 255.0 / norm_constant));
				}
				else {
					color = cvScalarAll( cvRound(255.0*(dataFlt[i] - min) / (temp)) );
				}
				cvRectangle(mp_cvimage->ipl, pt1, pt2, color, CV_FILLED, 8, 0);
			}
		}
	}
	else if(dataDbl) {
		// If normalization mode is 2, find min & max
		bool notnecessary = false;
		double min = 100000, max = -100000;
		if(normalize_mode == 2) {
			for(unsigned int i = 0;i < m_dim;i++) {
				if(dataDbl[i] > max) max = dataDbl[i];
				if(dataDbl[i] < min) min = dataDbl[i];
			}
			if(max == min) { 
				notnecessary = true;
			}
			//std::cout << getName() << "::execute()::max = " << max << ", min = " << min << "\n";
		}
		else if(normalize_mode == 1) {
			//std::cout << getName() << "::execute()::mode = 1, max = 1.0, min = 0.0\n";
		}
		else if(normalize_mode == 0) {
			max = 1.0;
			min = 0.0;		
		}
		// Draw rectangles (vector)
		if(notnecessary == false) {
			CvPoint pt1, pt2;
			CvScalar color;
			double temp = max - min;
			if(temp == 0.0) temp = 1.0;
			for(unsigned int i = 0;i < m_dim;i++) {
				if(vertical == true) {
					pt1 = cvPoint(0, i*height);
					pt2 = cvPoint(pt1.x + width, pt1.y + height);
				}
				else{
					pt1 = cvPoint(i*height, 0);
					pt2 = cvPoint(pt1.x + width, pt1.y + height);
				}
				if(normalize_mode == 0){
					color = cvScalarAll(dataDbl[i]);
				}
				else if(normalize_mode == 1){
					color = cvScalarAll( cvFloor(dataDbl[i] * 255.0 / norm_constant));
				}
				else {
					color = cvScalarAll( cvFloor(255.0*(dataDbl[i] - min) / temp ) );
				}
				cvRectangle(mp_cvimage->ipl, pt1, pt2, color, CV_FILLED, 8, 0);
			}
		}
	}
	else if(vec) {
		// If normalization mode is 2, find min & max
		float* tempdata = vec->getData();
		bool notnecessary = false;
		float min = 100000, max = -100000;
		if(normalize_mode == 2) {
			for(unsigned int i = 0;i < m_dim;i++) {
				if(tempdata[i] > max) max = tempdata[i];
				if(tempdata[i] < min) min = tempdata[i];
			}
			if(max == min) { 
				notnecessary = true;
			}
			//std::cout << getName() << "::execute()::max = " << max << ", min = " << min << "\n";
		}
		else if(normalize_mode == 1) {
			//std::cout << getName() << "::execute()::mode = 1, max = 1.0, min = 0.0\n";
		}
		else if(normalize_mode == 0) {
			max = 1.0;
			min = 0.0;		
		}
		// Draw rectangles (vector)
		if(notnecessary == false) {
			CvPoint pt1, pt2;
			CvScalar color;
			float temp = max - min;
			if(temp == 0.0) temp = 1.0;
			if(debug) std::cout << "vec = [";
			for(unsigned int i = 0;i < m_dim;i++) {
				if(vertical == true) {
					pt1 = cvPoint(0, i*height);
					pt2 = cvPoint(pt1.x + width, pt1.y + height);
				}
				else{
					pt1 = cvPoint(i*height, 0);
					pt2 = cvPoint(pt1.x + width, pt1.y + height);
				}
				
				double val = 0.0;
				if(normalize_mode == 0){
					val = (double)(tempdata[i]);
					color = cvScalarAll(val);
				}
				else if(normalize_mode == 1){
					val = (double)(cvFloor(tempdata[i] * 255.0 / norm_constant));
					color = cvScalarAll( cvFloor(tempdata[i] * 255.0 / norm_constant));
				}
				else {
					val = (double)(cvFloor(255.0*(tempdata[i] - min) / temp ));
					color = cvScalarAll( cvFloor(255.0*(tempdata[i] - min) / temp ) );
				}
				if(debug) std::cout << val << " ";
				cvRectangle(mp_cvimage->ipl, pt1, pt2, color, CV_FILLED, 8, 0);
			}
			if(debug) std::cout << "]\n";
		}
	}

	cvImageOut.setBuffer(mp_cvimage);
	cvImageOut.out();

}

void DrawVector::setDim(unsigned int dim) {
	if(dim < 1)  { std::cerr << getName() << "::ERROR::setParam()::Value out of range [dim<1] \n"; return; }
	m_dim = dim;
}

void DrawVector::setParam(bool vert, int w, int h, int mode, float c) {
	if(mode < 0 || mode > 2)  { std::cerr << getName() << "::ERROR::setParam()::Value out of range\n"; return; }
	vertical = vert;
	width = w;
	height = h;
	normalize_mode = mode;
	norm_constant = c;
}
