#include "resize.h"


Resize::Resize(std::string name, unsigned int mode, bool addnoise) : Module(name){
	std::cout<< "\nConstructing " << getName() << "...";

	cvImageIn.setModule(this);
	rectIn.setModule(this);
	outputSizeIn.setModule(this);

	mp_cvimage = NULL;
	mp_fixedOutputSize = NULL;

	this->setParam(mode, addnoise);

	/*	
		Interpolation method:

		CV_INTER_NN - nearest-neigbor interpolation,
		CV_INTER_LINEAR - bilinear interpolation (used by default)
		CV_INTER_AREA - resampling using pixel area relation. It is preferred method for image decimation that gives moire-free results. In case of zooming it is similar to CV_INTER_NN method.
		CV_INTER_CUBIC - bicubic interpolation. 
	*/
	m_interpolation = CV_INTER_NN;

	rectOut.setBuffer(&m_outputrect);

	debug = false;

	std::cout<< "finished.";
}

Resize::~Resize(){
	std::cout<< "\nDestructing " << getName() << "...";
	if(mp_cvimage != NULL) delete mp_cvimage;
	if(mp_fixedOutputSize != NULL) delete mp_fixedOutputSize;
	std::cout<< "finished.";
}

void Resize::execute() {

	CVImage* cvimg = cvImageIn.getBuffer();
	if (cvimg == NULL) { std::cerr << getName() << "::execute()::ERROR::cvImageIn is NULL!...\n"; return; }

	CvRect* inputrect = rectIn.getBuffer();
	CvSize* outputsize = outputSizeIn.getBuffer();
	if(outputsize == NULL && mp_fixedOutputSize == NULL) { std::cerr << getName() << "::execute()::ERROR::sizeIn is NULL!...\n"; return; }
	if(outputsize == NULL) outputsize = mp_fixedOutputSize;
	//std::cout << getName() << "::execute()::outputsize = [" << outputsize->width << ", " << outputsize->height << "]\n";

	float* scaleptr = scaleIn.getBuffer();

	int width, height;
	if(inputrect != NULL) {
		width = inputrect->width;
		height = inputrect->height;
	}
	else{
		width = cvimg->width;
		height = cvimg->height;
	}

	if(mp_cvimage != NULL) {
		if(mp_cvimage->width != outputsize->width || mp_cvimage->height != outputsize->height) {
			delete mp_cvimage;
			mp_cvimage = NULL;
		}
	}

	if(mp_cvimage == NULL) {
		mp_cvimage = new CVImage(*outputsize, cvimg->cvMatType, cvimg->zeroPad);
		cvImageOut.setBuffer(mp_cvimage);
	}
	//std::cout << getName() << "::execute()::mp_cvimage->width = " << mp_cvimage->width << ", mp_cvimage->height = " << mp_cvimage->height << "\n";

	if(m_addnoise == false) {
		cvSetZero(mp_cvimage->ipl);
	}
	else{
		this->addNoise();
	}

	switch(m_mode) {
		case CROP :
			if(debug) std::cout << "case CROP\n";
			//std::cout << getName() << "::execute()::mode CROP\n";	

			if(inputrect != NULL) {
				if(debug) std::cout << "inputrect != NULL\n";

				//std::cout << getName() << "::execute()::inputrect != NULL\n";	
				if(width == mp_cvimage->width && height == mp_cvimage->height) {
					m_outputrect.x = cvRound((mp_cvimage->width - width)/2.0);
					m_outputrect.y = cvRound((mp_cvimage->height - height)/2.0);
					m_outputrect.width = width;
					m_outputrect.height = height;
					cvSetImageROI(cvimg->ipl, *inputrect);
					cvSetImageROI(mp_cvimage->ipl, m_outputrect);
					cvCopy(cvimg->ipl, mp_cvimage->ipl);
					cvResetImageROI(cvimg->ipl);
					cvResetImageROI(mp_cvimage->ipl);
				}
				else {
					std::cerr << getName() << "::execute()::ERROR::Incorrect inputrect or outputsize!\n";
				}
			}
			else{
				if(debug) std::cout << "inputrect == NULL\n";

				//std::cout << getName() << "::execute()::width = " << width << ", height = " << height << "\n";
				//std::cout << getName() << "::execute()::mp_cvimage->width = " << mp_cvimage->width << ", mp_cvimage->height = " << mp_cvimage->height << "\n";

				int newwidth = mp_cvimage->width;
				int newheight = mp_cvimage->height;

				if(width <= newwidth && height <= newheight) {
					m_outputrect.x = cvRound((newwidth - width)/2.0);
					m_outputrect.y = cvRound((newheight - height)/2.0);
					m_outputrect.width = width;
					m_outputrect.height = height;
					//std::cout << getName() << "::execute()::m_outputrect = [" << m_outputrect.x << ", " << m_outputrect.y << ", " << m_outputrect.width << ", " << m_outputrect.height << "]\n";
					//CvRect temprect = cvGetImageROI(cvimg->ipl);
					//std::cout << getName() << "::execute()::temprect = [" << temprect.x << ", " << temprect.y << ", " << temprect.width << ", " << temprect.height << "]\n";
					cvSetImageROI(mp_cvimage->ipl, m_outputrect);
					cvCopy(cvimg->ipl, mp_cvimage->ipl);
					cvResetImageROI(mp_cvimage->ipl);
				}
				else {
					std::cerr << getName() << "::execute()::ERROR::Incorrect inputrect or outputsize!\n";
				}
				//std::cout << getName() << "::execute()::mp_cvimage->width = " << mp_cvimage->width << ", mp_cvimage->height = " << mp_cvimage->height << "\n";
				//CvRect temprect = cvGetImageROI(mp_cvimage->ipl);
				//std::cout << getName() << "::execute()::temprect = [" << temprect.x << ", " << temprect.y << ", " << temprect.width << ", " << temprect.height << "]\n";
			}
			cvImageOut.out();
			rectOut.out();

			break;

		case RESCALE :

			if(debug) std::cout << getName() << "::execute()::case RESCALE\n";	

			//if(tempptr == NULL) scale = 1.0;
			//else scale = *tempptr;

			//if(debug) std::cout << "case RESCALE, scale = " << scale << "\n";
			if(inputrect != NULL) {
				//if(debug) std::cout << "inputrect != NULL\n";
				//std::cout << getName() << "::execute()::inputrect != NULL\n";	

				int newwidth, newheight;
				if(scaleptr != NULL) {
					float scale = *scaleptr;
					newwidth = cvRound( (inputrect->width)*scale );
					newheight = cvRound( (inputrect->height)*scale );
				}
				else{
					newwidth = outputsize->width;
					newheight = outputsize->height;
				}

				//if(debug) std::cout << "inputrect = [" << inputrect->x << ", " << inputrect->y << ", " << inputrect->width << ", " << inputrect->height << "]\n";

				temprect.x = cvRound(( (mp_cvimage->width) - newwidth )/2.0);
				temprect.y = cvRound(( (mp_cvimage->height) - newheight )/2.0);
				temprect.width = newwidth;
				temprect.height = newheight;
				//if(debug) std::cout << "temprect = [" << temprect.x << ", " << temprect.y << ", " << temprect.width << ", " << temprect.height << "]\n";

				//CvRect tt = cvGetImageROI(cvimg->ipl);
				//std::cout << "cvimg->size = [" << cvimg->width << ", " << cvimg->height << "]\n";
				//std::cout << "cvimg->roi = [" << tt.x << ", " << tt.y << ", " << tt.width << ", " << tt.height << "]\n";
				//tt = cvGetImageROI(mp_cvimage->ipl);
				//std::cout << "mp_cvimage->size = [" << mp_cvimage->width << ", " << mp_cvimage->height << "]\n";
				//std::cout << "mp_cvimage->roi = [" << tt.x << ", " << tt.y << ", " << tt.width << ", " << tt.height << "]\n";

				if(temprect.x < 0 || temprect.y < 0 || (temprect.x+temprect.width) > mp_cvimage->width || (temprect.y+temprect.height) > mp_cvimage->height) {
					std::cerr << getName() << "::execute()::ERROR::Output image is too small\n";
					std::cerr << "RESCALE::inputrect != NULL\n";
					std::cerr << "temprect = [" << temprect.x << "," << temprect.y << "," << temprect.width << "," << temprect.height << "]\n";
					std::cerr << "mp_cvimage = [" << mp_cvimage->width << "," << mp_cvimage->height << "]\n";
				}
				else{
					//std::cout << "inputrect = [" << inputrect->x << ", " << inputrect->y << ", " << inputrect->width << ", " << inputrect->height << "]\n";
					//std::cout << "temprect = [" << temprect.x << ", " << temprect.y << ", " << temprect.width << ", " << temprect.height << "]\n";
/*
					cvSetImageROI(cvimg->ipl, *inputrect);
					cvSetImageROI(mp_cvimage->ipl, temprect);
					cvResize(cvimg->ipl, mp_cvimage->ipl, m_interpolation);
					cvResetImageROI(cvimg->ipl);
					cvResetImageROI(mp_cvimage->ipl);
*/

					if(inputrect->width == temprect.width && inputrect->height == temprect.height) {
						//std::cout << getName() << "::execute()::Line 1a\n";	

						cvSetImageROI(cvimg->ipl, *inputrect);

						/*
						cvSetImageROI(cvimg->ipl, *inputrect);
						cvSetImageROI(mp_cvimage->ipl, temprect);
						cvCopy(cvimg->ipl, mp_cvimage->ipl, NULL);
						cvResetImageROI(cvimg->ipl);
						cvResetImageROI(mp_cvimage->ipl);
						*/
						cvImageOut.setBuffer(cvimg);
						cvImageOut.out();
					}
					else{
						//std::cout << getName() << "::execute()::Line 1b\n";	
						cvSetImageROI(cvimg->ipl, *inputrect);
						cvSetImageROI(mp_cvimage->ipl, temprect);
						cvResize(cvimg->ipl, mp_cvimage->ipl, m_interpolation);
						cvResetImageROI(cvimg->ipl);
						cvResetImageROI(mp_cvimage->ipl);
						cvImageOut.setBuffer(mp_cvimage);
						cvImageOut.out();
					}

				}
				//rectOut.setBuffer(&temprect);
				//rectOut.out();
			}
			else{
				//if(debug) std::cout << "inputrect == NULL\n";
				//std::cout << getName() << "::execute()::inputrect == NULL\n";	

				float scale = 1.0;
				int newwidth, newheight;
				if(scaleptr != NULL) {
					scale = *scaleptr;
					//std::cout << "scale = " << scale << "\n";	
					newwidth = cvRound( (cvimg->width)*scale);
					newheight = cvRound( (cvimg->height)*scale);
				}
				else{
					newwidth = outputsize->width;
					newheight = outputsize->height;
				}

				temprect.x = cvRound(( (mp_cvimage->width) - newwidth )/2.0);
				temprect.y = cvRound(( (mp_cvimage->height) - newheight )/2.0);
				temprect.width = newwidth;
				temprect.height = newheight;
				//if(debug) std::cout << "temprect = [" << temprect.x << ", " << temprect.y << ", " << temprect.width << ", " << temprect.height << "]\n";

				if(temprect.x < 0 || temprect.y < 0 || (temprect.x+temprect.width) > mp_cvimage->width || (temprect.y+temprect.height) > mp_cvimage->height) {
					//std::cerr << getName() << "::execute()::ERROR::Output image is too small\n";
					//std::cerr << "RESCALE::inputrect == NULL\n";
					//std::cerr << "temprect = [" << temprect.x << "," << temprect.y << "," << temprect.width << "," << temprect.height << "]\n";
					//std::cerr << "mp_cvimage = [" << mp_cvimage->width << "," << mp_cvimage->height << "]\n";
					
					temprect.width = cvRound( (cvimg->width)/scale);
					temprect.height = cvRound( (cvimg->width)/scale);

					temprect.x = cvRound(( cvimg->width - temprect.width )/2.0);
					temprect.y = cvRound(( cvimg->height - temprect.height )/2.0);
					
					cvSetImageROI(cvimg->ipl, temprect);
					cvResize(cvimg->ipl, mp_cvimage->ipl, m_interpolation);
					cvResetImageROI(cvimg->ipl);
					
					
				}
				else{
					//std::cout << getName() << "::execute()::Line 2a\n";	
					cvSetImageROI(mp_cvimage->ipl, temprect);
					cvResize(cvimg->ipl, mp_cvimage->ipl, m_interpolation);
					cvResetImageROI(mp_cvimage->ipl);
				}
				cvImageOut.out();

				rectOut.setBuffer(&temprect);
				rectOut.out();
			}

			break;

		default :
			std::cerr << getName() << "::execute()::ERROR::Unsupported mode[" << m_mode << "]!\n";

	}

	//cvImageOut.out();
	//rectOut.out();
}

void Resize::fixOutputSize(const CvSize& size) {
	if(mp_fixedOutputSize == NULL) {
		mp_fixedOutputSize = new CvSize;
	}
	*mp_fixedOutputSize = size;

}

void Resize::setParam(unsigned int mode, bool addnoise) {
	switch(mode) {
		case CROP:
		case RESCALE:
			m_mode = mode;
			m_addnoise = addnoise;
			break;
		default:
			std::cerr << getName() << "::setParam()::ERROR::Unsupported mode[" << mode << "]!\n";
	}

	if(m_addnoise == true) 	{
		//m_mode = CROP;
		m_rand = cvRNG(-1);
	}
}

void Resize::addNoise() {
	cvRandArr( &m_rand, mp_cvimage->ipl, CV_RAND_UNI, cvScalar(0,0,0,0), cvScalar(255,255,255,0) );
}
