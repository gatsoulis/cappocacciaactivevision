#include "cscsave.h"
#include "cscload.h"
#include <stdio.h>
#include <string.h>



//! Constructor for CSCSave module
/*! Constructor for CSCSave module */
CSCSave::CSCSave(std::string name) : Module(name){
	std::cout<< "Constructing " << getName() << "...";

	cvLeftImageIn.setModule(this);
	cvRightImageIn.setModule(this);
	trackIn.setModule(this);
	vergIn.setModule(this);
	eyeneckIn.setModule(this);
	labelIn.setModule(this);

	mdp_dataLeft = NULL;
	mdp_dataRight = NULL;

	mp_winners = NULL;
	mp_vergs = NULL;
	mp_eyenecks = NULL;

	mp_cvleftImg = NULL;
	mp_cvrightImg = NULL;
	mp_cvleftImgdata = NULL;
	mp_cvrightImgdata = NULL;
	cvleftImage = NULL;
	cvrightImage = NULL;
	
	m_counter = 0;
	m_initialized = false;
	m_bufferFull = false;
	m_bufferSize = 0;
	
	debug = false;
	
	winner = NULL;
	verg = NULL;
	eyeneck = NULL;

	std::cout<< "finished.\n";

}

//! Destructor for CSCSave module
/*! Destructor for CSCSave module */
CSCSave::~CSCSave(){
	std::cout<< "\nDestructing " << getName() << "...";
	if(mdp_dataLeft != NULL) {
		for(int i = 0;i<m_bufferSize;i++) {
			delete mdp_dataLeft[i];
		}
		delete [] mdp_dataLeft;
	}
	if(mdp_dataRight != NULL) {
		for(int i = 0;i<m_bufferSize;i++) {
			delete mdp_dataRight[i];
		}
		delete [] mdp_dataRight;
	}
	
	if(mp_winners != NULL) delete [] mp_winners;
	if(mp_vergs != NULL) delete [] mp_vergs;
	if(mp_eyenecks != NULL) delete [] mp_eyenecks;
	
	if(mp_cvleftImg != NULL) {
		mp_cvleftImg->ipl->imageData = mp_cvleftImgdata;
		delete mp_cvleftImg;
	}
	if(mp_cvrightImg != NULL) {
		mp_cvrightImg->ipl->imageData = mp_cvrightImgdata;
		delete mp_cvrightImg;
	}
	
	std::cout<< "finished.\n";
}

bool CSCSave::isBufferFull() {
	return m_bufferFull;
}

//std::string* CSCSave::getXMLFilename() {
//	return m_filenameXML;
//}

//!Saves input left-right image pair and other info to memory
/*!Saves input left-right image pair and other info to memory */
void CSCSave::execute() {

	if(m_initialized == false) { std::cerr << getName() << "::execute()::ERROR::Call setParam() first!\n"; return; }
	if(m_bufferFull == true) return;
	if(m_counter >= m_bufferSize){
		std::cerr << getName() << "::execute()::ERROR::Buffer full!\n";
		m_bufferFull = true;
		return;
	}

	cvleftImage = cvLeftImageIn.getBuffer();
	cvrightImage = cvRightImageIn.getBuffer();
	winner = trackIn.getBuffer();
	verg = vergIn.getBuffer();
	eyeneck = eyeneckIn.getBuffer();

	if(cvleftImage == NULL && cvrightImage == NULL) { std::cerr<< getName() << "::execute()::ERROR::cvLeftImageIn and cvRightImageIn are all NULL!...\n"; return; }
//	if(winner == NULL) { std::cerr<< getName() << "::execute()::ERROR::trackIn is NULL!...\n"; return; }
//	if(verg == NULL) { std::cerr<< getName() << "::execute()::ERROR::vergIn is NULL!...\n"; return; }
//	if(eyeneck == NULL) { std::cerr<< getName() << "::execute()::ERROR::eyeneckIn is NULL!...\n"; return; }

	if(cvleftImage != NULL) {
		char* dst = mdp_dataLeft[m_counter];
		memcpy(dst, cvleftImage->ipl->imageData, m_sizeAFrameL);
	}
	if(cvrightImage != NULL) {
		char* dst = mdp_dataRight[m_counter];
		memcpy(dst, cvrightImage->ipl->imageData, m_sizeAFrameR);
	}

	if(winner != NULL) {

		if(winner->imageSize.width != cvleftImage->width || winner->imageSize.height != cvleftImage->height) {
			// Rescaling the tracking data coordinates to match the stored image size
			float horscale, verscale;
			horscale = (float)(cvleftImage->width) / (float)(winner->imageSize.width);
			verscale = (float)(cvleftImage->height) / (float)(winner->imageSize.height);
			
			mp_winners[m_counter].winnerPos.x = cvRound( (winner->winnerPos.x) * horscale );
			mp_winners[m_counter].winnerPos.y = cvRound( (winner->winnerPos.y) * verscale );
			mp_winners[m_counter].winnerSize.width = cvRound( (winner->winnerSize.width) * horscale );
			mp_winners[m_counter].winnerSize.height = cvRound( (winner->winnerSize.height) * verscale );
			mp_winners[m_counter].winnerRect.x = cvRound( (winner->winnerRect.x) * horscale );
			mp_winners[m_counter].winnerRect.y = cvRound( (winner->winnerRect.y) * verscale );
			mp_winners[m_counter].winnerRect.width = cvRound( (winner->winnerRect.width) * horscale );
			mp_winners[m_counter].winnerRect.height = cvRound( (winner->winnerRect.height) * verscale );
			mp_winners[m_counter].reliability = winner->reliability;
			mp_winners[m_counter].imageSize.width = cvleftImage->width;
			mp_winners[m_counter].imageSize.height = cvleftImage->height;
		}
		else {
			mp_winners[m_counter].winnerPos = winner->winnerPos;
			mp_winners[m_counter].winnerSize = winner->winnerSize;
			mp_winners[m_counter].winnerRect = winner->winnerRect;
			mp_winners[m_counter].reliability = winner->reliability;
			mp_winners[m_counter].imageSize = winner->imageSize;
		}

//		std::cout << "winnerPos = (" << mp_winners[m_counter].winnerPos.x << ", " << mp_winners[m_counter].winnerPos.y << ")\n";
//		std::cout << "winnerSize = (" << mp_winners[m_counter].winnerSize.width << ", " << mp_winners[m_counter].winnerSize.height << ")\n";
//		std::cout << "winnerRect = (" << mp_winners[m_counter].winnerRect.x << ", " << mp_winners[m_counter].winnerRect.y << ", " << mp_winners[m_counter].winnerRect.width << ", " << mp_winners[m_counter].winnerRect.height << ")\n";
//		std::cout << "reliability = " << mp_winners[m_counter].reliability << "\n";
//		std::cout << "imageSize = (" << mp_winners[m_counter].imageSize.width << ", " << mp_winners[m_counter].imageSize.height << ")\n";
	}
	else{
		mp_winners[m_counter].winnerPos.x = 0;
		mp_winners[m_counter].winnerPos.y = 0;
		mp_winners[m_counter].winnerSize.width = 0;
		mp_winners[m_counter].winnerSize.height = 0;
		mp_winners[m_counter].winnerRect.x = 0;
		mp_winners[m_counter].winnerRect.y = 0;
		mp_winners[m_counter].winnerRect.width = 0;
		mp_winners[m_counter].winnerRect.height = 0;
		mp_winners[m_counter].reliability = 0.0;
		mp_winners[m_counter].imageSize.width = cvleftImage->width;
		mp_winners[m_counter].imageSize.height = cvleftImage->height;
	}

	// leftHorDisp, leftVerDisp, rightHorDisp, rightVerDisp
	if(verg != NULL) {
		mp_vergs[m_counter].leftEyeHor = verg->leftEyeHor;
		mp_vergs[m_counter].leftEyeVer = verg->leftEyeVer;
		mp_vergs[m_counter].rightEyeHor = verg->rightEyeHor;
		mp_vergs[m_counter].rightEyeVer = verg->rightEyeVer;
	}
	else{
		mp_vergs[m_counter].leftEyeHor = 0.0;
		mp_vergs[m_counter].leftEyeVer = 0.0;
		mp_vergs[m_counter].rightEyeHor = 0.0;
		mp_vergs[m_counter].rightEyeVer = 0.0;
	}
	
	// REyeV, NeckV, Jaw, Mouth, Eyebrow, LEyeH, LEyeV, REyeH, NeckH
	if(eyeneck != NULL) {
		mp_eyenecks[m_counter].rightEyeVer = eyeneck->rightEyeVer;
		mp_eyenecks[m_counter].neckVer = eyeneck->neckVer;
		mp_eyenecks[m_counter].jaw = eyeneck->jaw;
		mp_eyenecks[m_counter].mouth = eyeneck->mouth;
		mp_eyenecks[m_counter].eyebrows = eyeneck->eyebrows;
		mp_eyenecks[m_counter].leftEyeHor = eyeneck->leftEyeHor;
		mp_eyenecks[m_counter].leftEyeVer = eyeneck->leftEyeVer;
		mp_eyenecks[m_counter].rightEyeHor = eyeneck->rightEyeHor;
		mp_eyenecks[m_counter].neckHor = eyeneck->neckHor;
	}
	else{
		mp_eyenecks[m_counter].rightEyeVer = 0.0;
		mp_eyenecks[m_counter].neckVer = 0.0;
		mp_eyenecks[m_counter].jaw = 0.0;
		mp_eyenecks[m_counter].mouth = 0.0;
		mp_eyenecks[m_counter].eyebrows = 0.0;
		mp_eyenecks[m_counter].leftEyeHor = 0.0;
		mp_eyenecks[m_counter].leftEyeVer = 0.0;
		mp_eyenecks[m_counter].rightEyeHor = 0.0;
		mp_eyenecks[m_counter].neckHor = 0.0;
	}

	if(debug) std::cout << getName() << "::execute()::counter = " << m_counter << "\n";

	m_counter = m_counter + 1;

}


//! Initializes CSCSave module for storing image pairs and relevant information to the memory first
/*! Initializes CSCSave module for storing image pairs and relevant information to the memory first */
void CSCSave::setParam(std::string Path, int frameSize, std::string imagetype, CvSize imageSize) {
	if(debug) std::cout << getName() << "::setParam() start\n";

	if(mdp_dataLeft != NULL) {
		for(int i = 0;i<m_bufferSize;i++) {
			delete mdp_dataLeft[i];
		}
		delete [] mdp_dataLeft;
	}
	if(mdp_dataRight != NULL) {
		for(int i = 0;i<m_bufferSize;i++) {
			delete mdp_dataRight[i];
		}
		delete [] mdp_dataRight;
	}

	m_path = Path;
	int ind = m_path.size() - 1;
	if(m_path[ind] != '/') m_path = m_path + "/";

	checkncreatefolder(m_path);
	std::string dtdfile = m_path + "list.dtd";
	checkncreatelistdtdfile(dtdfile);
	
/*
	int returnValue;
	std::string cwd;
	char* currentworkingdirectory = new char[400];
	getcwd(currentworkingdirectory, 400);
	cwd = currentworkingdirectory;
	delete [] currentworkingdirectory;

	mode_t mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH;
	bool createdtd = false;
	// Check the existence of the directory
	if(debug) std::cout<< getName() << "::setParam()::Checking existence of directory [" << m_path << "]\n";
	returnValue = chdir(m_path.c_str());
	// directory exists
	if(returnValue == 0) {
		if(debug) std::cout << getName() << "::setParam()::Directory [" << m_path << "] found\n";
		returnValue = chdir(cwd.c_str());
		// Check for the [list.dtd] file in the directory
		std::fstream fileptr;
		std::string filename = m_path + "list.dtd";
		fileptr.open (filename.c_str(), std::ios::in );
		if(fileptr.is_open()) {
			fileptr.close();
		}
		else {
			createdtd = true;
		}

	}
	else{
		// directory doesnt exist, create one
		returnValue = mkdir(m_path.c_str(), mode);
		if(returnValue == 0) {
			std::cout << getName() << "::setParam()::Creating directory [" << m_path << "] successful\n";
			// Create the [list.dtd] file in the directory
			createdtd = true;
		}
		else{ std::cerr << getName() << "::setParam()::ERROR::Creating directory [" << m_path << "] failed !\n"; return; }
	}
	
	if(createdtd) {
		if(debug) std::cout << getName() << "::setParam()::Creating [list.dtd] file\n";
		std::fstream fileptr;
		std::string filename = m_path + "list.dtd";
		fileptr.open (filename.c_str(), std::ios::out );
		fileptr << "<!ELEMENT Database (Object*)>\n";	
		fileptr << "<!ELEMENT Object EMPTY>\n";	
		fileptr << "<!ATTLIST Object id CDATA #REQUIRED>\n";	
		fileptr << "<!ATTLIST Object objname CDATA #REQUIRED>\n";	
		fileptr << "<!ATTLIST Object filename CDATA #REQUIRED>\n";	
		fileptr.close();
		
//		filename = m_path + "list.xml";
//		fileptr.open (filename.c_str(), std::ios::out );
//		fileptr << "<!DOCTYPE Database SYSTEM \"list.dtd\">\n";
//		fileptr << "<Database>\n";	
//		fileptr << "</Database>\n";	
//		fileptr.close();
		
	}
*/


	m_imagetype = imagetype;
	m_bufferSize = frameSize;

	m_extLeftImage = "-L." + imagetype;
	m_extRightImage = "-R." + imagetype;

	if(mp_winners != NULL) delete [] mp_winners;
	if(mp_vergs != NULL) delete [] mp_vergs;
	if(mp_eyenecks != NULL) delete [] mp_eyenecks;

	mp_winners = new TrackData [m_bufferSize];
	mp_vergs = new VergenceData [m_bufferSize];
	mp_eyenecks = new MotorCommand [m_bufferSize];

	if(mp_cvleftImg != NULL) delete mp_cvleftImg;
	if(mp_cvrightImg != NULL) delete mp_cvrightImg;

	// This is for saving stream of images to memory first and then saving to hard drive later
	// 30fps * 60sec = 1800 frames per minute.
	if(mdp_dataLeft == NULL){
		mp_cvleftImg = new CVImage(imageSize, CV_8UC3, 0);
		mp_cvleftImgdata = mp_cvleftImg->ipl->imageData;
		m_sizeAFrameL = (mp_cvleftImg->ipl->widthStep) * imageSize.height;

		mdp_dataLeft = new char* [m_bufferSize];
		for(int i = 0;i<m_bufferSize;i++) {
			mdp_dataLeft[i] = new char [m_sizeAFrameL];
		}		
	}
	if(mdp_dataRight == NULL){
		mp_cvrightImg = new CVImage(imageSize, CV_8UC3, 0);
		mp_cvrightImgdata = mp_cvrightImg->ipl->imageData;
		m_sizeAFrameR = (mp_cvrightImg->ipl->widthStep) * imageSize.height;
		mdp_dataRight = new char* [m_bufferSize];
		for(int i = 0;i<m_bufferSize;i++) {
			mdp_dataRight[i] = new char [m_sizeAFrameR];
		}
	}

	m_initialized = true;
	m_bufferFull = false;
	
	if(debug) std::cout << getName() << "::initialize() finished\n";
}

//! Outputs memory-stored image pairs and relevant information 
/*! Outputs memory-stored image pairs and relevant information */
void CSCSave::getData(int index) {

	if(index < m_counter){
		char* dst = mdp_dataLeft[index];
		mp_cvleftImg->ipl->imageData = dst;
		dst = mdp_dataRight[index];
		mp_cvrightImg->ipl->imageData = dst;

		cvLeftImageOut.setBuffer(mp_cvleftImg);
		cvRightImageOut.setBuffer(mp_cvrightImg);
		cvLeftImageOut.out();
		cvRightImageOut.out();
		trackOut.setBuffer(&(mp_winners[index]));
		trackOut.out();
		vergOut.setBuffer(&(mp_vergs[index]));
		vergOut.out();
		eyeneckOut.setBuffer(&(mp_eyenecks[index]));
		eyeneckOut.out();
	}
	else{
		std::cerr << getName() << "::getData()::ERROR::index out of range!\n";
	}
}

//! Saves memory-stored image pairs and relevant information into prespecified directory (will not create directory)
/*! Saves memory-stored image pairs and relevant information into prespecified directory (will not create directory) */
void CSCSave::saveToDisk() {
	if(debug) std::cout<< getName() << "::saveToDisk()::Saving memory-stored images to hard drive....\n";
	if(m_counter == 0) { std::cerr << getName() << "::saveToDisk()::ERROR::No left-right image pairs stored in the buffer\n"; return; }

	mp_label = labelIn.getBuffer();
	if(mp_label == NULL){ std::cerr << getName() << "::saveToDisk()::ERROR::labelIn is Null!\n"; return; }

	std::string pathImages = m_path + *(mp_label) + "/";
	checkncreatefolder(pathImages);
	std::string tempXMLPath = m_path + "XMLfolder/";
	checkncreatefolder(tempXMLPath);

	std::string trackdtdfile = tempXMLPath + "track2.dtd";
	checkncreatetrackdtdfile(trackdtdfile);	

/*
	mode_t mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH;
	
	if(debug) std::cout << getName() << "::saveToDisk()::pathImages [" << pathImages << "]\n";

	int returnValue;
	std::string cwd;
	char* currentworkingdirectory = new char[400];
	getcwd(currentworkingdirectory, 400);
	cwd = currentworkingdirectory;
	delete [] currentworkingdirectory;
//	if(debug) std::cout << "::Current working directory [" << currentworkingdirectory << "]\n";

	// Check the existence of the directory (to save image files)
	if(debug) std::cout<< getName() << "::saveToDisk()::Checking existence of directory [" << pathImages << "]\n";
	returnValue = chdir(pathImages.c_str());
	// directory for the object exists
	if(returnValue == 0) {
		if(debug) std::cout << "::Directory [" << pathImages << "] found\n";
	}
	else {
		std::cout << "::Directory [" << pathImages << "] not found\n";
		// directory doesnt exist, create one
		returnValue = mkdir(pathImages.c_str(), mode);
		if(returnValue == 0) std::cout << "::Creating directory [" << pathImages << "] successful\n";
		else{ std::cerr << "::ERROR::Creating directory [" << pathImages << "] failed !\n"; return; }
	}
	returnValue = chdir(cwd.c_str());

	int i, temp;
	
	// Check the existence of the directory (to save the xml file)
	std::string tempXMLPath = m_path + "XMLfolder/";
	if(debug) std::cout<< getName() << "::saveToDisk()::Checking existence of directory [" << tempXMLPath << "]to store xml files\n";
	returnValue = chdir(tempXMLPath.c_str());
	// directory for the xml data exists
	if(returnValue == 0) {
		if(debug) std::cout << "::Directory [" << tempXMLPath << "] found\n";
		returnValue = chdir(cwd.c_str());
	}
	else {
		std::cout << getName() << "::saveToDisk()::Directory [" << tempXMLPath << "] not found\n";
		// directory for the xml data doesnt exist, create one.
		returnValue = mkdir(tempXMLPath.c_str(), mode);
		if(returnValue == 0) std::cout << "::saveToDisk()::Creating directory [" << tempXMLPath << "] successful\n";
		else{ std::cerr << getName() << "::saveToDisk()::ERROR::Creating directory [" << tempXMLPath << "] failed !\n"; return; }

		//returnValue = chdir(tempXMLPath.c_str());
		// create track.dtd file for use in track xml data
		
		std::string filename = tempXMLPath + "track2.dtd";
		
		std::ofstream datafileNew;
		datafileNew.open(filename.c_str(), std::ios::out);
		datafileNew << "<!ELEMENT Sequence (Object, Path, Slide*)>\n";
		datafileNew << "<!ELEMENT Object EMPTY>\n";
		datafileNew << "<!ATTLIST Object name CDATA #REQUIRED>\n";
		datafileNew << "<!ATTLIST Object totalSlideNumber CDATA #REQUIRED>\n";
		datafileNew << "<!ELEMENT Path EMPTY>\n";
		datafileNew << "<!ATTLIST Path name CDATA #REQUIRED>\n";
		datafileNew << "<!ELEMENT Slide (Image, Image, WinnerPos, WinnerSize, WinnerRect, Reliability, Vergence, EyeNeck)>\n";
		datafileNew << "<!ATTLIST Slide number CDATA #REQUIRED>\n";
		datafileNew << "<!ELEMENT Image EMPTY>\n";
		datafileNew << "<!ATTLIST Image side CDATA #REQUIRED>\n";
		datafileNew << "<!ATTLIST Image id CDATA #REQUIRED>\n";
		datafileNew << "<!ELEMENT WinnerPos EMPTY>\n";
		datafileNew << "<!ATTLIST WinnerPos x CDATA #REQUIRED>\n";
		datafileNew << "<!ATTLIST WinnerPos y CDATA #REQUIRED>\n";
		datafileNew << "<!ELEMENT WinnerSize EMPTY>\n";
		datafileNew << "<!ATTLIST WinnerSize width CDATA #REQUIRED>\n";
		datafileNew << "<!ATTLIST WinnerSize height CDATA #REQUIRED>\n";
		datafileNew << "<!ELEMENT WinnerRect EMPTY>\n";
		datafileNew << "<!ATTLIST WinnerRect x CDATA #REQUIRED>\n";
		datafileNew << "<!ATTLIST WinnerRect y CDATA #REQUIRED>\n";
		datafileNew << "<!ATTLIST WinnerRect width CDATA #REQUIRED>\n";
		datafileNew << "<!ATTLIST WinnerRect height CDATA #REQUIRED>\n";
		datafileNew << "<!ELEMENT Reliability EMPTY>\n";
		datafileNew << "<!ATTLIST Reliability val CDATA #REQUIRED>\n";
		datafileNew << "<!ELEMENT Vergence EMPTY>\n";
		datafileNew << "<!ATTLIST Vergence leftH CDATA #REQUIRED >\n";
		datafileNew << "<!ATTLIST Vergence leftV CDATA #REQUIRED >\n";
		datafileNew << "<!ATTLIST Vergence rightH CDATA #REQUIRED >\n";
		datafileNew << "<!ATTLIST Vergence rightV CDATA #REQUIRED >\n";
		datafileNew << "<!ELEMENT EyeNeck EMPTY>\n";
		datafileNew << "<!ATTLIST EyeNeck LEyeH CDATA #REQUIRED >\n";
		datafileNew << "<!ATTLIST EyeNeck LEyeV CDATA #REQUIRED >\n";
		datafileNew << "<!ATTLIST EyeNeck REyeH CDATA #REQUIRED >\n";
		datafileNew << "<!ATTLIST EyeNeck REyeV CDATA #REQUIRED >\n";
		datafileNew << "<!ATTLIST EyeNeck NeckH CDATA #REQUIRED >\n";
		datafileNew << "<!ATTLIST EyeNeck NeckV CDATA #REQUIRED >\n";
		datafileNew.close();
		
	}
*/	
	m_filenameXML = tempXMLPath + *mp_label + "_tracking.xml";

	char* tempNameLeft = new char[200];
	char* tempNameRight = new char[200];
	char* tempNameLeft2 = new char[50];
	char* tempNameRight2 = new char[50];

	if(debug) std::cout<< "::Total number of images : " << m_counter << "\n";
	dataFile.open (m_filenameXML.c_str(), std::ios::out );
	dataFile << "<!DOCTYPE Sequence SYSTEM \"track2.dtd\"> \n";
	dataFile << "<Sequence> \n";
	dataFile << "<Object name=\"" << *mp_label << "\" totalSlideNumber=\"" << m_counter << "\"/>          \n";
	dataFile << "<Path name=\"" << pathImages << "\"/> \n";
	for(int i=0;i<m_counter;i++){
		int temp = i + 1001;
		//if( i % 10 == 0) if(debug) std::cout<< i << " images saved...\n";
		if( i % 100 == 0) std::cout<< i << "\n";

		sprintf(tempNameLeft, "%s%i%s", pathImages.c_str(), temp, m_extLeftImage.c_str());
		sprintf(tempNameRight, "%s%i%s", pathImages.c_str(), temp, m_extRightImage.c_str());
		sprintf(tempNameLeft2, "%i%s", temp, m_extLeftImage.c_str());
		sprintf(tempNameRight2, "%i%s", temp, m_extRightImage.c_str());
		
		if(debug) {
			std::cout << "tempNameLeft = " << tempNameLeft << "\n";
			std::cout << "tempNameRight = " << tempNameRight << "\n";
		}

		dataFile << "<Slide number=\"" << (i+1) << "\"> \n";
		dataFile << "<Image side=\"left\" id=\"" << tempNameLeft2 << "\"/> \n";
		dataFile << "<Image side=\"right\" id=\"" << tempNameRight2 << "\"/> \n";
		dataFile << "<WinnerPos x=\"" << mp_winners[i].winnerPos.x << "\" y=\"" << mp_winners[i].winnerPos.y << "\"/> \n";
		dataFile << "<WinnerSize width=\"" << mp_winners[i].winnerSize.width << "\" height=\"" << mp_winners[i].winnerSize.height << "\"/> \n";
		dataFile << "<WinnerRect x=\"" << mp_winners[i].winnerRect.x << "\" y=\"" << mp_winners[i].winnerRect.y << "\" width=\"" << mp_winners[i].winnerRect.width << "\" height=\"" << mp_winners[i].winnerRect.height << "\"/> \n";
		dataFile << "<Reliability val=\"" << mp_winners[i].reliability << "\"/> \n";
		dataFile << "<Vergence leftH=\"" << mp_vergs[i].leftEyeHor << "\" leftV=\"" << mp_vergs[i].leftEyeVer << "\" rightH=\"" << mp_vergs[i].rightEyeHor << "\" rightV=\"" << mp_vergs[i].rightEyeVer << "\"/> \n";
		dataFile << "<EyeNeck LEyeH=\"" << mp_eyenecks[i].leftEyeHor << "\" LEyeV=\"" << mp_eyenecks[i].leftEyeVer << "\" REyeH=\"" << mp_eyenecks[i].rightEyeHor << "\" REyeV=\"" << mp_eyenecks[i].rightEyeVer << "\" NeckH=\"" << mp_eyenecks[i].neckHor << "\" NeckV=\"" << mp_eyenecks[i].neckVer << "\"/> \n";
		dataFile << "</Slide> \n";

		char* dst = mdp_dataLeft[i];
		mp_cvleftImg->ipl->imageData = dst;
		dst = mdp_dataRight[i];
		mp_cvrightImg->ipl->imageData = dst;
		cvSaveImage(tempNameLeft, mp_cvleftImg->ipl);	
		cvSaveImage(tempNameRight, mp_cvrightImg->ipl);	
	}
	dataFile << "</Sequence> \n";
	dataFile.flush();
	dataFile.close();
	
//	returnValue = chdir(currentworkingdirectory);

	delete [] tempNameLeft;
	delete [] tempNameRight;
	delete [] tempNameLeft2;
	delete [] tempNameRight2;
//	delete [] currentworkingdirectory;

	if(debug) std::cout<< getName() << "::saveToDisk()::Saving memory-stored data to hard drive finished....\n";
}

void CSCSave::appendToDisk() {
	if(debug) std::cout<< getName() << "::appendToDisk()::Appending memory-stored images to hard drive....\n";
	if(m_counter == 0) { std::cerr << getName() << "::appendToDisk()::ERROR::No left-right image pairs stored in the buffer\n"; return; }

	mp_label = labelIn.getBuffer();
	if(mp_label == NULL){ std::cerr << getName() << "::appendToDisk()::ERROR::labelIn is Null!\n"; return; }


	std::string pathImages = m_path + *(mp_label) + "/";
	bool tempcheck = checkfolder(pathImages);
	if(!tempcheck) {
		std::cerr << getName() << "::appendToDisk()::ERROR::Folder [" << pathImages << "] doesnt exist. Unable to append\n";
		return;
	}

	std::string tempXMLPath = m_path + "XMLfolder/";
	tempcheck = checkfolder(tempXMLPath);
	if(!tempcheck) {
		std::cerr << getName() << "::appendToDisk()::ERROR::Folder [" << tempXMLPath << "] doesnt exist. Unable to append\n";
		return;
	}

	m_filenameXML = tempXMLPath + *mp_label + "_tracking.xml";
	tempcheck = checkfile(m_filenameXML);
	if(!tempcheck) {
		std::cerr << getName() << "::appendToDisk()::ERROR::File [" << m_filenameXML << "] doesnt exist. Unable to append\n";
		return;
	}

	CSCLoad cscload("CSCLoad"+getName());
	cscload.setXMLFilename(&m_filenameXML);
	int totalBefore = cscload.getNumSlides();

	char* tempNameLeft = new char[200];
	char* tempNameRight = new char[200];
	char* tempNameLeft2 = new char[50];
	char* tempNameRight2 = new char[50];

	if(debug) std::cout<< "::Total number of images : " << m_counter << "\n";

	dataFile.open (m_filenameXML.c_str(), std::ios::out | std::ios::in );
	long pos;
	dataFile.seekp(0, std::ios_base::beg);
	pos = dataFile.tellp();
	dataFile.seekp(pos);

	dataFile << "<!DOCTYPE Sequence SYSTEM \"track2.dtd\"> \n";
	dataFile << "<Sequence> \n";
	dataFile << "<Object name=\"" << *mp_label << "\" totalSlideNumber=\"" << (totalBefore+m_counter) << "\"/>         \n";
	dataFile << "<Path name=\"" << pathImages << "\"/> \n";

	dataFile.seekp(0, std::ios_base::end);
	pos = dataFile.tellp();
	dataFile.seekp(pos - 13);
	for(int i=0;i<m_counter;i++){
		int temp = i + 1001 + totalBefore;
		if( i % 10 == 0) std::cout<< "CSCSave::Total of " << i << " images saved...\n";
		sprintf(tempNameLeft, "%s%i%s", pathImages.c_str(), temp, m_extLeftImage.c_str());
		sprintf(tempNameRight, "%s%i%s", pathImages.c_str(), temp, m_extRightImage.c_str());
		sprintf(tempNameLeft2, "%i%s", temp, m_extLeftImage.c_str());
		sprintf(tempNameRight2, "%i%s", temp, m_extRightImage.c_str());

		if(debug) {
			std::cout << "tempNameLeft = " << tempNameLeft << "\n";
			std::cout << "tempNameRight = " << tempNameRight << "\n";
		}

		dataFile << "<Slide number=\"" << (i+1+totalBefore) << "\"> \n";
		dataFile << "<Image side=\"left\" id=\"" << tempNameLeft2 << "\"/> \n";
		dataFile << "<Image side=\"right\" id=\"" << tempNameRight2 << "\"/> \n";
		dataFile << "<WinnerPos x=\"" << mp_winners[i].winnerPos.x << "\" y=\"" << mp_winners[i].winnerPos.y << "\"/> \n";
		dataFile << "<WinnerSize width=\"" << mp_winners[i].winnerSize.width << "\" height=\"" << mp_winners[i].winnerSize.height << "\"/> \n";
		dataFile << "<WinnerRect x=\"" << mp_winners[i].winnerRect.x << "\" y=\"" << mp_winners[i].winnerRect.y << "\" width=\"" << mp_winners[i].winnerRect.width << "\" height=\"" << mp_winners[i].winnerRect.height << "\"/> \n";
		dataFile << "<Reliability val=\"" << mp_winners[i].reliability << "\"/> \n";
		dataFile << "<Vergence leftH=\"" << mp_vergs[i].leftEyeHor << "\" leftV=\"" << mp_vergs[i].leftEyeVer << "\" rightH=\"" << mp_vergs[i].rightEyeHor << "\" rightV=\"" << mp_vergs[i].rightEyeVer << "\"/> \n";
		dataFile << "<EyeNeck LEyeH=\"" << mp_eyenecks[i].leftEyeHor << "\" LEyeV=\"" << mp_eyenecks[i].leftEyeVer << "\" REyeH=\"" << mp_eyenecks[i].rightEyeHor << "\" REyeV=\"" << mp_eyenecks[i].rightEyeVer << "\" NeckH=\"" << mp_eyenecks[i].neckHor << "\" NeckV=\"" << mp_eyenecks[i].neckVer << "\"/> \n";
		dataFile << "</Slide> \n";

		char* dst = mdp_dataLeft[i];
		mp_cvleftImg->ipl->imageData = dst;
		dst = mdp_dataRight[i];
		mp_cvrightImg->ipl->imageData = dst;
		cvSaveImage(tempNameLeft, mp_cvleftImg->ipl);	
		cvSaveImage(tempNameRight, mp_cvrightImg->ipl);	
	}
	dataFile << "</Sequence> \n";
	dataFile.flush();
	dataFile.close();

//	returnValue = chdir(currentworkingdirectory);

	delete [] tempNameLeft;
	delete [] tempNameRight;
	delete [] tempNameLeft2;
	delete [] tempNameRight2;
//	delete [] currentworkingdirectory;
	
	if(debug) std::cout<< getName() << "::appendToDisk()::Appending memory-stored data to hard drive finished....\n\n";
}

bool CSCSave::checkfolder(const std::string& path) {
	std::string temppath = path;
	int ind = temppath.size() - 1;
	if(temppath[ind] != '/') temppath = temppath + "/";

	int returnValue;
	std::string cwd;
	char* currentworkingdirectory = new char[400];
	getcwd(currentworkingdirectory, 400);
	cwd = currentworkingdirectory;
	delete [] currentworkingdirectory;

//	mode_t mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH;

	// Check the existence of the directory
	if(debug) std::cout<< getName() << "::checkfolder()::Checking existence of directory [" << temppath << "]\n";
	returnValue = chdir(temppath.c_str());
	// directory exists
	if(returnValue == 0) {
		if(debug) std::cout << getName() << "::checkfolder()::Directory [" << temppath << "] found\n";
		returnValue = chdir(cwd.c_str());
	}
	else{
		if(debug) std::cout << getName() << "::checkfolder()::Directory [" << temppath << "] not found\n";
		return false;
	}
	return true;	
}

bool CSCSave::checkfile(const std::string& file) {
	// Check for the [list.xml] file in the directory
	std::fstream fileptr;
	fileptr.open (file.c_str(), std::ios::in );
	if(fileptr.is_open()) {
		if(debug) std::cout << getName() << "::checkfile()::File [" << file << "] found\n";
		fileptr.close();
	}
	else {
		if(debug) std::cout << getName() << "::checkfile()::ERROR::File [" << file << "] not found\n";
		return false;
	}
	return true;
}

bool CSCSave::checkncreatefolder(const std::string& path) {
	std::string temppath = path;
	int ind = temppath.size() - 1;
	if(temppath[ind] != '/') temppath = temppath + "/";

	int returnValue;
	std::string cwd;
	char* currentworkingdirectory = new char[400];
	getcwd(currentworkingdirectory, 400);
	cwd = currentworkingdirectory;
	delete [] currentworkingdirectory;

	mode_t mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH;

	// Check the existence of the directory
	if(debug) std::cout<< getName() << "::checkncreatefolder()::Checking existence of directory [" << temppath << "]\n";
	returnValue = chdir(temppath.c_str());
	// directory exists
	if(returnValue == 0) {
		if(debug) std::cout << getName() << "::checkncreatefolder()::Directory [" << temppath << "] found\n";
		returnValue = chdir(cwd.c_str());
	}
	else{
		// directory doesnt exist, create one
		returnValue = mkdir(temppath.c_str(), mode);
		if(returnValue == 0) {
			std::cout << getName() << "::checkncreatefolder()::Creating directory [" << temppath << "] successful\n";
		}
		else{ std::cerr << getName() << "::checkncreatefolder()::ERROR::Creating directory [" << temppath << "] failed !\n"; return false; }
	}

	return true;	
}

bool CSCSave::checkncreatelistdtdfile(const std::string& file) {
	// Check for the [list.dtd] file in the directory
	std::fstream fileptr;
	fileptr.open (file.c_str(), std::ios::in );
	if(fileptr.is_open()) {
		fileptr.close();
	}
	else {
		if(debug) std::cout << getName() << "::checkncreatelistdtdfile()::Creating [" << file << "] file\n";
		fileptr.open (file.c_str(), std::ios::out );
		fileptr << "<!ELEMENT Database (Object*)>\n";	
		fileptr << "<!ELEMENT Object EMPTY>\n";	
		fileptr << "<!ATTLIST Object id CDATA #REQUIRED>\n";	
		fileptr << "<!ATTLIST Object objname CDATA #REQUIRED>\n";	
		fileptr << "<!ATTLIST Object filename CDATA #REQUIRED>\n";	
		fileptr.close();
	}
	return true;
}

bool CSCSave::checkncreatetrackdtdfile(const std::string& file) {
	// Check for the [list.xml] file in the directory
	std::fstream fileptr;
	fileptr.open (file.c_str(), std::ios::in );
	if(fileptr.is_open()) {
		fileptr.close();
	}
	else {
		if(debug) std::cout << getName() << "::checkncreatetrackdtdfile()::Creating [" << file << "] file\n";
		fileptr.open (file.c_str(), std::ios::out );
		fileptr << "<!ELEMENT Sequence (Object, Path, Slide*)>\n";
		fileptr << "<!ELEMENT Object EMPTY>\n";
		fileptr << "<!ATTLIST Object name CDATA #REQUIRED>\n";
		fileptr << "<!ATTLIST Object totalSlideNumber CDATA #REQUIRED>\n";
		fileptr << "<!ELEMENT Path EMPTY>\n";
		fileptr << "<!ATTLIST Path name CDATA #REQUIRED>\n";
		fileptr << "<!ELEMENT Slide (Image, Image, WinnerPos, WinnerSize, WinnerRect, Reliability, Vergence, EyeNeck)>\n";
		fileptr << "<!ATTLIST Slide number CDATA #REQUIRED>\n";
		fileptr << "<!ELEMENT Image EMPTY>\n";
		fileptr << "<!ATTLIST Image side CDATA #REQUIRED>\n";
		fileptr << "<!ATTLIST Image id CDATA #REQUIRED>\n";
		fileptr << "<!ELEMENT WinnerPos EMPTY>\n";
		fileptr << "<!ATTLIST WinnerPos x CDATA #REQUIRED>\n";
		fileptr << "<!ATTLIST WinnerPos y CDATA #REQUIRED>\n";
		fileptr << "<!ELEMENT WinnerSize EMPTY>\n";
		fileptr << "<!ATTLIST WinnerSize width CDATA #REQUIRED>\n";
		fileptr << "<!ATTLIST WinnerSize height CDATA #REQUIRED>\n";
		fileptr << "<!ELEMENT WinnerRect EMPTY>\n";
		fileptr << "<!ATTLIST WinnerRect x CDATA #REQUIRED>\n";
		fileptr << "<!ATTLIST WinnerRect y CDATA #REQUIRED>\n";
		fileptr << "<!ATTLIST WinnerRect width CDATA #REQUIRED>\n";
		fileptr << "<!ATTLIST WinnerRect height CDATA #REQUIRED>\n";
		fileptr << "<!ELEMENT Reliability EMPTY>\n";
		fileptr << "<!ATTLIST Reliability val CDATA #REQUIRED>\n";
		fileptr << "<!ELEMENT Vergence EMPTY>\n";
		fileptr << "<!ATTLIST Vergence leftH CDATA #REQUIRED >\n";
		fileptr << "<!ATTLIST Vergence leftV CDATA #REQUIRED >\n";
		fileptr << "<!ATTLIST Vergence rightH CDATA #REQUIRED >\n";
		fileptr << "<!ATTLIST Vergence rightV CDATA #REQUIRED >\n";
		fileptr << "<!ELEMENT EyeNeck EMPTY>\n";
		fileptr << "<!ATTLIST EyeNeck LEyeH CDATA #REQUIRED >\n";
		fileptr << "<!ATTLIST EyeNeck LEyeV CDATA #REQUIRED >\n";
		fileptr << "<!ATTLIST EyeNeck REyeH CDATA #REQUIRED >\n";
		fileptr << "<!ATTLIST EyeNeck REyeV CDATA #REQUIRED >\n";
		fileptr << "<!ATTLIST EyeNeck NeckH CDATA #REQUIRED >\n";
		fileptr << "<!ATTLIST EyeNeck NeckV CDATA #REQUIRED >\n";
		fileptr.close();
	}
	return true;
}

void CSCSave::removeObj(const std::string& objectName) {
	std::string folder = m_path + objectName;
	bool temp = checkfolder(folder);
	if(!temp) { std::cerr << getName() << "::removeObj()::ERROR::Directory [" << folder << "] not found\n"; return; }
	
	std::string file = m_path + "XMLfolder/" + objectName + "_tracking.xml";
	temp = checkfile(file);
	if(!temp) { std::cerr << getName() << "::removeObj()::ERROR::File [" << file << "] not found\n"; return; }

	std::string command1 = "rm -Rf " + folder;
	std::string command2 = "rm " + file;

	if(debug) {
		std::cout << getName() << "::removeObj()::command1 = [" << command1 << "]\n";
		std::cout << getName() << "::removeObj()::command2 = [" << command2 << "]\n";
	}

	system(command1.c_str());
	system(command2.c_str());
		
}
