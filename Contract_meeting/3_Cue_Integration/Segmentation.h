#include "cxcore.h"
#include "cv.h"
#include "cvaux.h"
#include "highgui.h"

#include <iomanip>

using namespace std;

#define Noof_B_Cues 2
#define Noof_T_Cues 1 // only 1 Top-down cue is used
#define Noof_Cues (Noof_B_Cues+Noof_T_Cues)

#define iGridSize 5 	/*there is a trouble if image->height and image->width are not divisible by iGridSize  */

#define iSeqLength  (320/iGridSize)*(240/iGridSize) // =(image->height/iGridSize)*(image->width/iGridSize);
#define objRows  (240/iGridSize)
#define objCols  (320/iGridSize)


#ifndef _SEGMENTATION_H
#define  _SEGMENTATION_H

	#define  iBgModelSize 5
	#define TD_MOD_UPDATE_FACTOR 0.50
	#define  iDataChoice 0 //0 for iCub 1 for camera and 2 for stored frames
	#define iInitialFrame 1
	#define iFinalFrame 300



//	void updateBg(IplImage *IBgModel[],IplImage *frame,int iIndex);
	void clearBgModel(IplImage *IBgModel[]);
	void Normalize(IplImage *IFggray,IplImage *dst, int a, int b);
	void Normalize32F(IplImage *I32F,IplImage *gray, int a, int b);
	void showImg(IplImage *Image,char *cName);
	 
	struct histpoint
	{
		int idx;
//		int i,j,k;
//		float count;
	};
	struct histpoint1
	{
		int i,j,k;
		float count;
	};
	void pushInSeq(CvSeq* ColorSeqF, histpoint point);
	void pushInSeq_1(CvSeq* ColorSeqF, int idx);

	struct objFeature
	{
		float fPrior;
		int iColor[3];
	//	float fTexture[3];
		
	};
//	void objectMatching(IplImage *image,CvSeq *Obj_Kt[], CvSeq *Obj_Kt_1[],int  Obj_rect_t[],int  Obj_rect_t_1[]);
	void objectMatching(IplImage *image,IplImage *IF_posterior,objFeature *ObjKt[objRows][objCols], objFeature *ObjKt_1[objRows][objCols],int  Obj_rect_t[],int  Obj_rect_t_1[]);
	CvRect fobjectMatching(IplImage *image,IplImage *IF_posterior,objFeature *ObjKt[objRows][objCols], objFeature *ObjKt_1[objRows][objCols], CvRect rectt, CvRect rectt_1);

	struct Object
	{
		CvRect rectt,rectt_1;
		int iInitializationCount;
		int iUpdate;
	//	objFeature* ObjKt_1[objRows][objCols];

	//	Object();
	//	~Object();
	};

	class Cues
	{
	public:
		IplImage *IF_Likelihood,*IB_Likelihood;
		float fCueWeight,fQuality;
		void computeLiklihood(IplImage *image1,IplImage *image2);
		void computeLiklihoodColor(IplImage *IF_posterior,IplImage *IB_posterior,CvMatND *FGColorModel,CvMatND *BGColorModel,IplImage *frame, int index,CvSeq *,CvSeq *);
		void computeColorRep(IplImage *IF_posterior,IplImage *IB_posterior,CvMatND *FGColorModel,CvMatND *BGColorModel,IplImage *image, int index,CvSeq *ColorSeqF, CvSeq *ColorSeqB);
		void computeLiklihoodTexture(IplImage *IF_posterior,IplImage *IB_posterior,CvMatND *FGTextureModel,CvMatND *BGTextureModel,IplImage *image,int index);
		void gaussianSmoothing(CvMatND *FGTextureModel);
		void gaussianSmoothingNormalization_1(CvMatND *FGColorModel,CvMatND *BGColorModel,CvMatND * FGModelt_1,CvMatND *BGModelt_1,CvSeq *ColorSeqF, CvSeq *ColorSeqB,int index);
//		void computeLiklihoodObjectRep(IplImage *IF_posterior,CvSeq *Obj_Kt[],CvSeq *Obj_Kt_1[],int Obj_rect[],int Obj_rect_t_1[], IplImage *Igray, IplImage *image,int index);
		void computeLiklihoodObjectRep(IplImage *IF_posterior,objFeature *ObjKt[objRows][objCols],objFeature *ObjKt_1[objRows][objCols],int Obj_rect[],CvSeq *ObjtsSeq, IplImage *Igray, IplImage *image,int index);
		void computeObjectRep(IplImage *IF_posterior,objFeature *ObjKt[objRows][objCols], objFeature *ObjKt_1[objRows][objCols],int Obj_rect_t[],CvSeq *ObjtsSeq, IplImage *IGray, IplImage *image,int index);
		
	
		void computeQuality(IplImage *IF_posterior);
		void clearAllocation();
	};

	class Cue_integration
	{
	public:
		int iRows,iCols;
		int iFrameIndex;
		int iNoofB_Cues,iNoofT_Cues,iNoof_Cues;
		int iObject[4];

		CvSeq *ColorSeqF,*ColorSeqB;//, *TextureSeq;
		
	//	CvMemStorage *storgearray1[iSeqLength],*storgearray2[iSeqLength];
	//	CvSeq *Obj_Kt[iSeqLength], *Obj_Kt_1[iSeqLength];
		int Obj_rect_t[4],Obj_rect_t_1[4];

		CvSeq *Obj_rectt,*Ojb_rectt_1;
		CvMemStorage* strgObj;
		CvSeq *ObjtsSeq;

		objFeature* ObjKt[objRows][objCols];
		objFeature* ObjKt_1[objRows][objCols];
		//	int iNoof_cues;
		//[Noof_Cues],fQuality[Noof_Cues];//={4.0,0.0,0.0,0.0};
		IplImage *IF_posterior,*IB_posterior;
							// only two layers are considered here Figure and Background
							//Multiple layers has to be implemented.
		IplImage *IBGModel[iBgModelSize+1];;
		CvMatND *FGColorModel,*BGColorModel;
		CvMatND *FGTextureModel,*BGTextureModel;
		
//		CvMat *Obj_Kt_1,*Obj_Kt;
		Cues Cue[Noof_Cues];
		IplImage *IgrayDisplay;
		
		void initialize(IplImage *image);
		void integratedPosteriorProbs();
		void updateBg(IplImage *IBgModel[],IplImage *frame,int iIndex, IplImage *IF_posterior);
		void clearBgModel(IplImage *IBgModel[]);
		void clearAllocations();
		bool bObjAppearance(IplImage *IF_Likelihood);


		
	private:
	};
#endif

