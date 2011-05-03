#include "indobjtracker.h"

using namespace yarp::os;
using namespace yarp::sig;

IndObjTracker::IndObjTracker(std::string name) : Module(name) {

	std::cout << "Constructing " << getName() << "...";

	trackInLeft.setModule(this);
	trackInRight.setModule(this);

	bella = getName();

//	targetPortLeft.open("/"+bella+"/left/out");
//	targetPortRight.open("/"+bella+"/right/out");

	targetPortLeft.open("/tutorial/ind/target/left/out");
	targetPortRight.open("/tutorial/ind/target/right/out");

	debug = true;

	xL=0.0;
	yL=0.0;
	xR=0.0;
	yR=0.0;

	std::cout<<"finished.\n";

}


IndObjTracker::~IndObjTracker() {

	std::cout<<"\nDestructing " << getName() << "...";


	targetPortLeft.close();

	targetPortRight.close();

	std::cout<<"finished.\n";

}

void IndObjTracker::connecting(){

	std::cout<<"\nConnetting " << getName() << "...";

	printf("\nINIZIO CONNESSIONE");

	Network::connect("/tutorial/ind/target/left/out","/motion/left/in");
	Network::connect("/tutorial/ind/target/right/out","/motion/right/in");

//	Network::connect(("/"+bella+"/left/out").c_str(),"/motion/left/in");
//	Network::connect(("/"+bella+"/right/out").c_str(),"/motion/right/in");
	
	printf("\nFINE CONNESSIONE");
	
	std::cout<<"\nfinished.";

}

void IndObjTracker::disconnecting(){

	std::cout<<"\nDisconnetting " << getName() << "...";

	targetPortLeft.interrupt();
	targetPortRight.interrupt();

	std::cout<<"\nfinished.";

}

void IndObjTracker::execute() {

	TrackData* trackleft = trackInLeft.getBuffer();
	TrackData* trackright = trackInRight.getBuffer();

	if(!trackleft) { std::cerr << getName() << "::ERROR::execute()::trackInLeft is NULL!\n"; return; }
	if(!trackright) { std::cerr << getName() << "::ERROR::execute()::trackInRight is NULL!\n"; return; }

	if(debug) {
		std::cout << getName() << "::initialize()::track\n";
		std::cout << "trackPosLeft = (" << trackleft->winnerPos.x << ", " << trackleft->winnerPos.y << ")\n";
		std::cout << "trackSizeLeft = (" << trackleft->winnerSize.width << ", " << trackleft->winnerSize.height << ")\n";
		std::cout << "trackRectLeft = (" << trackleft->winnerRect.x << ", " << trackleft->winnerRect.y << ", " << 				trackleft->winnerRect.width << ", " << trackleft->winnerRect.height << ")\n";
		std::cout << "imageSizeLeft = (" << trackleft->imageSize.width << ", " << trackleft->imageSize.height << ")\n";
		std::cout << "relLeft = " << trackleft->reliability << "\n";
		std::cout << "trackPosRight = (" << trackright->winnerPos.x << ", " << trackright->winnerPos.y << ")\n";
		std::cout << "trackSizeRight = (" << trackright->winnerSize.width << ", " << trackright->winnerSize.height << ")\n";
		std::cout << "trackRectRight = (" << trackright->winnerRect.x << ", " << trackright->winnerRect.y << ", " << 				trackright->winnerRect.width << ", " << trackright->winnerRect.height << ")\n";
		std::cout << "imageSizeRight = (" << trackright->imageSize.width << ", " << trackright->imageSize.height << ")\n";
		std::cout << "relRight = " << trackright->reliability << "\n";

	}

	m_trackleft.winnerPos.x = cvRound((float)(trackleft->winnerPos.x));
	m_trackleft.winnerPos.y = cvRound((float)(trackleft->winnerPos.y));
	m_trackleft.reliability = trackleft->reliability;

	m_trackright.winnerPos.x = cvRound((float)(trackright->winnerPos.x));
	m_trackright.winnerPos.y = cvRound((float)(trackright->winnerPos.y));
	m_trackright.reliability = trackright->reliability;

	xL = m_trackleft.winnerPos.x;
	yL = m_trackleft.winnerPos.y;
	if(debug){
		std::cout<<"xl : "<< xL <<"\n";
		std::cout<<"yl : "<< yL <<"\n";
	}
	
	xR = m_trackright.winnerPos.x;
	yR = m_trackright.winnerPos.y;
	if(debug){
		std::cout<<"xr : "<< xR <<"\n";
		std::cout<<"yr : "<< yR <<"\n";
	}

	Vector& targetL = targetPortLeft.prepare();
        targetL.resize(3);
        targetL[0] = xL;
        targetL[1] = yL;
        targetL[2] = 1;
        targetPortLeft.write();

	Vector& targetR = targetPortRight.prepare();
        targetR.resize(3);
        targetR[0] = xR;
        targetR[1] = yR;
        targetR[2] = 1;
        targetPortRight.write();

	if(debug) std::cout << getName() << "::execute() complete\n";

}
