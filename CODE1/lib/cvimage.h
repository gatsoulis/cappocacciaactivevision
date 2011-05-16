#ifndef CVImage_H
#define CVImage_H

//#include "ippi.h"
#include "cv.h"
#include <iostream>
#include <string>

#include "CSCData.h"



	class CVImage : public CSCData {

		public:
    
			IplImage* ipl;
			CvMat* mat;
    
			union{
				void* data;
				/*
				Ipp8u* data8u;
				Ipp8s* data8s;
				Ipp16u* data16u;
				Ipp16s* data16s;
				Ipp16sc* data16sc;
				Ipp32u* data32u;
				Ipp32s* data32s;
				Ipp32sc* data32sc;
				Ipp32f* data32f;
				Ipp32fc* data32fc;
				Ipp64s* data64s;
				Ipp64f* data64f;
				*/
			};

    	    //! union for Image Size
			union{   
				struct{
					union{
						int cols;
						int width;
					};
					union{
						int rows;
						int height;
					};
				};
				//IppiSize iSize;
				CvSize cSize;
			};
			
    	    //! union for ROI
			union{
				struct{
					int x;
					int y;
					int width;
					int height;
				} roi;
				//IppiRect iROI;
				CvRect cROI;
			};

			int step;  //!< Step Size in Bytes
			int nChannels;  //!number of channels
			int cvMatType;
			int iplDepth;
			int zeroPad;
    
			// Constructors use openCV's types ie CV_<bit_depth>(S|U|F)C<number_of_channels>
			CVImage();
			//CVImage(IppiSize sizeVar, int typeVar, int zPad = 0);  
			CVImage(CvSize sizeVar, int typeVar, int zPad = 0);  
			CVImage(int widthVar, int heightVar, int typeVar, int zPad = 0);
			~CVImage();
    
			void resetROI();
    
			//bool resize(IppiSize sizeVar);
			bool resize(CvSize sizeVar);
			bool resize(int widthVar, int heightVar);
			//bool changeAttributes(IppiSize sizeVar, int typeVar, int zPad = 0);
			bool changeAttributes(CvSize sizeVar, int typeVar, int zPad = 0);
			bool changeAttributes(int widthVar, int heightVar, int typeVar, int zPad = 0);
	 
			void cloneAttributes(const IplImage* donorIpl);
			void cloneAttributes(const CvMat* donorMat);
			void cloneAttributes(const CVImage* donorImg);
	 
			void cloneImage(const IplImage* donorIpl);
			void cloneImage(const CvMat* donorMat);
			void cloneImage(const CVImage* donorImg);
    
			//void displayInWindow(char* windowName);  //!< Intended for debugging use only
    
			static int iplToCvMatType(int depthOfIpl, int numberOfIplChannels);
	 
			static int getNumChannels(int cvMatType);

			int byteDepth();

			void zeroOut();
	 
			int allocateMemory();
			void deallocateMemory();
    
			void toStream(std::ostream* str);
			CVImage* fromStream(std::istream* str);
	
			void writeToFile(const std::string& filename);
			bool readFromFile(const std::string& filename);

			std::string type() { return "CVImage"; }  

			void print();
	};

#endif
