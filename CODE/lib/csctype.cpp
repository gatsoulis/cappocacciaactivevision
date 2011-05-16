#include "csctype.h"
#include "CSCFeature.h"
#include "CSCGaborJet.h"
/*#include "CSCGaborJetFace.h"			AGGIUNGERE!!!!
#include "CSCGaborJetObject.h"
#include "CSCNode.h"
#include "CSCView.h"
#include "CSCObject.h"
#include "CSCCategory.h"
*/

using namespace std;


CSCViewData* GetNewDataPtr(const std::string& type) {
	CSCViewData* newptr = NULL;
	if(type == "CVD") newptr = new CSCViewData;
	else if(type == "CF") newptr = new CSCFeature;
	else if(type == "GJ") newptr = new CSCGaborJet;
/*	else if(type == "GJF") newptr = new CSCGaborJetFace;		AGGIUNGERE!!!!
	else if(type == "GJO") newptr = new CSCGaborJetObject;
	else if(type == "CN") newptr = new CSCNode;
	else if(type == "CV") newptr = new CSCView;
	else if(type == "CO") newptr = new CSCObject;
	else if(type == "CC") newptr = new CSCCategory;
*/	else std::cerr << "GetNewDataPtr()::No data type exists of [" << type << "]\n";

/*
	if(type == CSCViewData::type()) newptr = new CSCViewData;
	else if(type == CSCFeature::type()) newptr = new CSCFeature;
	else if(type == CSCGaborJet::type()) newptr = new CSCGaborJet;
//	else if(type == CSCGaborJetFace::type()) newptr = new CSCGaborJetFace;
//	else if(type == CSCGaborJetObject::type()) newptr = new CSCGaborJetObject;
	else if(type == CSCNode::type()) newptr = new CSCNode;
	else if(type == CSCView::type()) newptr = new CSCView;
	else if(type == CSCObject::type()) newptr = new CSCObject;
	else if(type == CSCCategory::type()) newptr = new CSCCategory;
	else std::cerr << "GetNewDataPtr()::No data type exists of [" << type << "]\n";
*/
	return newptr;
}

