
#include <cv.h>
#include <highgui.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <vector>

using namespace std;

// define whether to use approximate nearest-neighbor search
#define USE_FLANN


double compareSURFDescriptors( const float* d1, const float* d2, double best, int length );
int naiveNearestNeighbor( const float* vec, int laplacian, const CvSeq* model_keypoints,const CvSeq* model_descriptors );
void findPairs( const CvSeq* objectKeypoints, const CvSeq* objectDescriptors, const CvSeq* imageKeypoints, const CvSeq* imageDescriptors, vector<int>& ptpairs );
void flannFindPairs( const CvSeq*, const CvSeq* objectDescriptors, const CvSeq*, const CvSeq* imageDescriptors, vector<int>& ptpairs );
int locatePlanarObject( const CvSeq* objectKeypoints, const CvSeq* objectDescriptors, const CvSeq* imageKeypoints, const CvSeq* imageDescriptors, const CvPoint src_corners[4], CvPoint dst_corners[4] );
void surf_match(IplImage* object_color, IplImage* object, IplImage* image,const CvSeq *objectKeypoints,const CvSeq *imageKeypoints,const CvSeq * objectDescriptors,const CvSeq * imageDescriptors, CvPoint val[4]);
