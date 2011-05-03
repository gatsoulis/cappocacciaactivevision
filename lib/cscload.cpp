#include "cscload.h"
#include "serializer.h"



CSCLoad::CSCLoad(std::string name) : Module(name){
	std::cout<< "Constructing " << getName() << "...";
	
	debug = false;
	m_end = false;
	
	std::cout<< "finished.\n";
}

CSCLoad::~CSCLoad(){
	std::cout<< "Destructing " << getName() << "...";
	std::cout<< "finished.\n";
}

void CSCLoad::execute() {

	if(debug) {
		std::cout << getName() << "::execute()::counter = " << m_counter << "\n";
		std::cout << getName() << "::execute()::m_totalimagesize = " << m_totalimagesize << "\n";
	}

	if(m_counter < m_totalimagesize) {
	
		std::string* objname = m_parser.getObjName();

		labelOut.setBuffer(objname);
		labelOut.out();
		
		vergOut.setBuffer(m_parser.getVergenceData(m_counter));
		vergOut.out();

		eyeneckOut.setBuffer(m_parser.getMotorData(m_counter));
		eyeneckOut.out();
		
		std::string* path = m_parser.getPath();
		std::string* left = m_parser.getFilenameLeft(m_counter);
		std::string* right = m_parser.getFilenameRight(m_counter);

		std::string fullleft = *path + *left;
		std::string fullright = *path + *right;

		filenameLeftOut.setBuffer(left);
		filenameLeftOut.out();

		filenameRightOut.setBuffer(right);
		filenameRightOut.out();

		fullfilenameLeftOut.setBuffer(&fullleft);
		fullfilenameLeftOut.out();

		fullfilenameRightOut.setBuffer(&fullright);
		fullfilenameRightOut.out();
		
		CVImage* cvimgleft = m_loadleft.load(fullleft);
		CVImage* cvimgright = m_loadright.load(fullright);
		if(!cvimgleft || !cvimgright) { std::cerr << getName() << "::execute()::ERROR loading images\n"; return; }
		
		cvLeftImageOut.setBuffer(cvimgleft);
		cvLeftImageOut.out();
		
		cvRightImageOut.setBuffer(cvimgright);
		cvRightImageOut.out();
		
		CVImage* cvimgleftgray = m_convleft.convert(cvimgleft);
		CVImage* cvimgrightgray = m_convright.convert(cvimgright);
		if(!cvimgleftgray || !cvimgrightgray) { std::cerr << getName() << "::execute()::ERROR converting images to gray-scale\n"; return; }
		
		cvLeftGrayImageOut.setBuffer(cvimgleftgray);
		cvLeftGrayImageOut.out();

		cvRightGrayImageOut.setBuffer(cvimgrightgray);
		cvRightGrayImageOut.out();

		TrackData* winner = m_parser.getTrackData(m_counter);
		
		m_winner.winnerPos = winner->winnerPos;
		m_winner.winnerSize.width = cvRound((winner->winnerSize.width) * m_scale);
		m_winner.winnerSize.height = cvRound((winner->winnerSize.height) * m_scale);
		m_winner.winnerRect.x = cvRound(m_winner.winnerPos.x - m_winner.winnerSize.width/2.0);
		m_winner.winnerRect.y = cvRound(m_winner.winnerPos.y - m_winner.winnerSize.height/2.0);
		m_winner.winnerRect.width = m_winner.winnerSize.width;
		m_winner.winnerRect.height = m_winner.winnerSize.height;
		m_winner.reliability = winner->reliability;
		m_winner.imageSize.width = cvimgleft->width;
		m_winner.imageSize.height = cvimgleft->height;

		trackOut.setBuffer(&m_winner);
		trackOut.out();
	
		m_object.setName(*objname);
		m_location.x = 0;
		m_location.y = 0;
		m_location.width = winner->imageSize.width;
		m_location.height = winner->imageSize.height;
		m_object.setLocation(m_location);
		
		objectOut.setBuffer(&m_object);
		objectOut.out();

		m_counter += m_step;
	}
	else{
		m_end = true;
		std::cerr << getName() << "::execute()::ERROR::Counter [" << m_counter << " out of range [0, " << m_totalimagesize << "]\n";
	}
}

bool CSCLoad::setXMLFilename(std::string* filenamexml) {
	// check for file existence
	struct stat buffer;	
	if(stat(filenamexml->c_str(), &buffer)) { std::cerr << getName() << "::setParam()::File [" << filenamexml << "] does not exist!\n"; return false; }
	
	//m_parser.set_substitute_entities(true); // Set whether the parser will automatically substitute entity references with the text of the entities' definitions. 
	m_parser.clear();
	m_parser.parse_file(*filenamexml);
	m_counter = m_startindex-1;
	
	m_totalimagesize = m_parser.getSlideSize();
	m_end = false;

	return true;
}

int CSCLoad::getNumSlides() {
	return m_totalimagesize;
}

int CSCLoad::getCounter() {
	return m_counter;
}

void CSCLoad::setParam(int startindex, int step, float scale) {
	m_startindex = startindex;
	m_step = step;
	m_scale = scale;
	m_counter = startindex-1;
}

bool CSCLoad::checkEnd() { 
	if(m_counter >= m_totalimagesize) m_end = true; 
	return m_end; 
}

