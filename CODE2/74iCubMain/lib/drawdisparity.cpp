/*
 * drawdisparity.cpp
 *
 *  Created on: Feb 25, 2011
 *      Author: gabriele
 */

#include "drawdisparity.h"

DrawDisp::DrawDisp(std::string name, bool sameimage) : Module(name) {

	cvImageIn.setModule(this);

	regionsInLeft.setModule(this);
	regionsInRight.setModule(this);

	mp_cvimg = NULL;

	m_sameimage = sameimage;

	debug = true;

}

DrawDisp::~DrawDisp() {

	if(mp_cvimg && !m_sameimage) delete mp_cvimg;

}

void DrawDisp::execute() {


	CvScalar color[15] = { 			// in RGB order BGR
			{{   0,   0,   0,   0 }},	// black
			{{   0,   0, 128,   0 }},	// deep red
			{{   0,   0, 255,   0 }},	// red
			{{   0, 128,   0,   0 }},	// deep green
			{{   0, 255,   0,   0 }},	// green
			{{   0, 165, 255,   0 }},	// orange
			{{   0, 255, 255,   0 }},	// yellow
			{{ 128,   0,   0,   0 }},	// deep blu	
			{{ 255,   0,   0,   0 }},	// blu
			{{ 128,   0, 128,   0 }},	// deep violet 
			{{ 255,   0, 255,   0 }},	// violet
			{{ 128, 128,   0,   0 }},	// deep light blu
			{{ 255, 255,   0,   0 }},	// light blu
			{{ 128, 128, 128,   0 }},	// grey		
			{{ 255, 255, 255,   0 }},	// white

			};

	int m_thickness =2;

	CVImage* cvimg = cvImageIn.getBuffer();

	CSCEllipticRegions* regionLeft = regionsInLeft.getBuffer();
	CSCEllipticRegions* regionRight = regionsInRight.getBuffer();




	if(!cvimg) { std::cerr << getName() << "::ERROR::cvImageIn is NULL!\n"; return; }
	if(!regionLeft || !regionRight) {
		cvImageOut.setBuffer(cvimg);
		cvImageOut.out();
		return;
	}

		if(debug) {
			std::cout << getName() << "::execute()::region.size = " << regionLeft->getCurrentSize() << "\n";
		}

		if(!mp_cvimg && !m_sameimage){
			mp_cvimg = new CVImage(cvSize(cvimg->width, cvimg->height), CV_8UC3, 0);
		}

		if(!m_sameimage) {
	//		cvCopy(cvimg->ipl, mp_cvimg->ipl);
			if(cvimg->cvMatType == mp_cvimg->cvMatType) {
				cvCopy(cvimg->ipl, mp_cvimg->ipl);
			}
			else{
				cvCvtColor(cvimg->ipl, mp_cvimg->ipl, CV_GRAY2BGR);
			}
		}
		else{
			mp_cvimg = cvimg;
		}

		IplImage* img = mp_cvimg->ipl;

		if(regionLeft && regionRight) {


			unsigned int height = regionLeft->getCurrentSize();
			unsigned int width = regionLeft->getWidth();

			float* dataLeft = regionLeft->getData();
			float* dataRight = regionRight->getData();
			float maxdispX = 0;
			float mindispX = 0;
			float dispX=0;
			float meandispX = 0;
			float stdvdispX = 0;

			for(unsigned int i = 0;i<height;i++) {
				float posxLeft = dataLeft[i*width];
				float posxRight = dataRight[i*width];
				dispX = posxLeft - posxRight;

				meandispX += dispX;

				stdvdispX += (dispX*dispX);

				if(dispX>maxdispX)
					maxdispX=dispX;
				if(dispX<mindispX)
					mindispX=dispX;
			}
			meandispX /= (float)height;
			stdvdispX /= (float)height;
			stdvdispX -= (meandispX*meandispX);
			stdvdispX = sqrt(stdvdispX);


			int minim = -110;//meandispX - 2 * stdvdispX;
			int maxim = 10;//meandispX + 2 * stdvdispX;
			int reg = (int)((maxim - minim) /15);

			if(debug){
				printf("minimum disparity: %f\n",mindispX);
				printf("maximum disparity: %f\n",maxdispX);
				printf("mean disparity: %f\n",meandispX);
				printf("standard deviation disparity: %f\n",stdvdispX);
			}

			CvScalar m_colordisp;

			dispX=0;

			for(unsigned int i = 0;i<height;i++) {
				float posxLeft = dataLeft[i*width];
				float posxRight = dataRight[i*width];
				float posyLeft = dataLeft[i*width+1];
				dispX = posxLeft - posxRight;
				if (dispX<=minim + reg)
					m_colordisp = color[0] ;
				if (dispX > minim + reg && dispX<= minim + 2*reg)
					m_colordisp = color[1] ;
				if (dispX > minim + 2*reg && dispX<= minim + 3*reg)
					m_colordisp = color[2] ;
				if (dispX > minim + 3*reg && dispX<= minim + 4*reg)
					m_colordisp = color[3] ;
				if (dispX > minim + 4*reg && dispX<= minim + 5*reg)
					m_colordisp = color[4] ;
				if (dispX > minim + 5*reg && dispX<= minim + 6*reg)
					m_colordisp = color[5] ;
				if (dispX > minim + 6*reg && dispX<= minim + 7*reg)
					m_colordisp = color[6] ;
				if (dispX > minim + 7*reg && dispX<= minim + 8*reg)
					m_colordisp = color[7] ;
				if (dispX > minim + 8*reg && dispX<= minim + 9*reg)
					m_colordisp = color[8] ;
				if (dispX > minim + 9*reg && dispX<= minim + 10*reg)
					m_colordisp = color[9] ;
				if (dispX > minim +11*reg && dispX<= minim + 12*reg)
					m_colordisp = color[10] ;
				if (dispX > minim +12*reg && dispX<= minim + 13*reg)
					m_colordisp = color[11] ;
				if (dispX > minim +13*reg && dispX<= minim + 14*reg)
					m_colordisp = color[14] ;
				if (dispX > minim + 14*reg)
					m_colordisp = color[15] ;


				/*int disp = (int)(dispX-(meandispX-2*stdvdispX))*256*3/(4*stdvdispX);
				int z = (int)(disp/32);
				double blue = (int) (z/3) + (z%3 > 0 ? 1 : 0) * 32 + (disp%32)*(z%3 == 0 ? 1 : 0);
				double green = (int) (z/3) + (z%3 > 1 ? 1 : 0) * 32 + (disp%32)*(z%3 == 1 ? 1 : 0);
				double red = (int) (z/3) + (z%3 > 2 ? 1 : 0) * 32 + (disp%32)*(z%3 == 2 ? 1 : 0);
				if(dispX<-2)
					m_colordisp = CV_RGB(255,0,0);
				if(dispX>-2 && dispX<=2)
					m_colordisp = CV_RGB(0,255,0);
				if(dispX>2)
					m_colordisp = CV_RGB(0,0,255);
				//m_colordisp =CV_RGB(red,green,blue);
				if(dispX>=mindispX && dispX<(maxdispX-mindispX)/4)
					m_colordisp = CV_RGB(255,0,0);
				else if (dispX>=(maxdispX-mindispX)/4 && dispX<(maxdispX+mindispX)/4)
					m_colordisp = CV_RGB(0,255,0);
				else
					m_colordisp = CV_RGB(0,0,255);*/
				//m_colordisp = CV_RGB(255*((dispX-mindispX)/(maxdispX-mindispX)),255*((dispX-mindispX)/(maxdispX-mindispX)),255*((dispX-mindispX)/(maxdispX-mindispX)));
				cvCircle(img,cvPoint(cvRound(posxLeft),cvRound(posyLeft)),m_thickness,m_colordisp, CV_FILLED );
			}
		}

		cvImageOut.setBuffer(mp_cvimg);
		cvImageOut.out();

}
