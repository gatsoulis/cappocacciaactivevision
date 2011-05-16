#ifndef CSCType_H
#define CSCType_H

//#include "ipp.h"
#include "cv.h"

#include "cvimage.h"
#include "CSCViewData.h"

#include <string>
#include <vector>
#include <time.h>



	CSCViewData* GetNewDataPtr(const std::string& type);
	
	struct NodeInfo {
		CvPoint pos;
		CSCViewData* nodeptr;
		unsigned int nodeindex;
		float weight;
	};

	struct ViewInfo {
		std::vector<NodeInfo> nodes;
		float weight;
		CvPoint pos;
		CSCViewData* viewptr;
	};

	struct CSCRecData {
		CvMat* cvMat;
		std::string dataType;
		int dataSize; //number of floats per pixel
		std::vector<CvPoint> dataPoints;     
	};

	struct CSCRecQuery {
		CvPoint loc;
		std::string name;
		float similarity;
		CSCViewData* ptrobject;
		CSCViewData* ptrview;
		//the feature used to compute similariy at loc in image
		//union {
		//	cscnode* pnode;
		//	cscview* pview;
		//} feature; /* feature pointers */
		CvRect croprect; //to do segmentation near the target area
		//bool operator< (const CSCRecQuery &right) const {return name < right.name;}
	};

    struct CSCPose {
		float pan;
		float tilt;
		float rotation;
		float distance;
    };

    struct DisparityVectors
    {
            CvPoint* pos;
            CvPoint* vector;
			float* distance;
			float* similarity;
            bool* flag;
            int numVectors;
    };

/*
    struct ForegroundObject
    {
            CvPoint* pos;
			//Ipp32fc* feature;
    };
*/
    struct VergenceInput
    {
		CvPoint winnerPos;
		CvSize winnerSize;
		CvRect winnerRect;
		CVImage* leftImg;
		CVImage* rightImg;
	};

    struct VergenceOutput
    {
		float leftHorDisp;
		float leftVerDisp;
		float rightHorDisp;
		float rightVerDisp;
	};

    struct EyeneckOutput
    {
		float* leftEyeHor;
		float* leftEyeVer;
		float* rightEyeHor;
		float* rightEyeVer;
		float* neckHor;
		float* neckVer;
	};

    struct TrackData
    {
		CvPoint winnerPos;
		CvSize winnerSize;
		CvRect winnerRect;
		float reliability;

		CvSize imageSize;
	};

    struct MotorCommand
    {
		float leftEyeHor;
		float leftEyeVer;
		float rightEyeHor;
		float rightEyeVer;
		float neckHor;
		float neckVer;
		float eyebrows;
		float jaw;
		float mouth;
	};

    struct VergenceData
    {
		float leftEyeHor;
		float leftEyeVer;
		float rightEyeHor;
		float rightEyeVer;
	};

    struct EyeNeckData
    {
		float leftEyeHor;
		float leftEyeVer;
		float rightEyeHor;
		float rightEyeVer;
		float neckHor;
		float neckVer;
		float eyebrows;
		float jaw;
		float mouth;
	};

    struct HeadPose
    {
		std::string label;
		float pan; // Horizontal rotational movement, in degrees. 0 for center position
		float tilt; // Vertical rotational movement, in degrees. 0 for center position
	};

    struct CornerPoint {
		// Represented as ellipse w/ a(x-u)(x-u) + 2*b(x-u)(y-v) + c(y-v)(y-v) = 1
		float u;
		float v;
		float a;
		float b;
		float c;
	};
	
    struct CornerPoints {
		std::string type;
		CornerPoint* points;
		int numPoints;
	};
	
    struct CSCEllipseRegion {
		// Represent region as ellipse : a(x-u)(x-u) + 2*b(x-u)(y-v) + c(y-v)(y-v) = 1
		float u;
		float v;
		float a;
		float b;
		float c;
	};
	
	struct CSCEllipseRegions {
		CvMat* data;
		std::string type;
		int size;
		int limit;
	};
	
	struct CSCDescriptor {
		CvMat* data;
		CvMat* regions;
		std::string type;
		int size;
		int limit;
		int dim;
	};

/*
    struct MotorCommand2
    {
		std::vector<int> servo;
		std::vector<int> pos;
		std::vector<int> speed;
		int time;
	};
*/


#endif
