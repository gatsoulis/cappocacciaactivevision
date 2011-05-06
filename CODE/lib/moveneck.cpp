#include "moveneck.h"

using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::dev;

MoveNeck::MoveNeck(std::string name) : Module(name) {





}


MoveNeck::~MoveNeck() {

}

void MoveNeck::execute(){

	Property options;

	IPositionControl *pos;
	IVelocityControl *vel;
	IEncoders *enc;

	options.put("device", "remote_controlboard");
	options.put("local", "/tutorial/motor/client");
	options.put("remote", "/icub/head");

	PolyDriver robotHead(options);

	if (!robotHead.isValid()) {
		printf("Cannot connect to robot head\n");

	}


	robotHead.view(pos);
	robotHead.view(vel);
	robotHead.view(enc);
	if (pos==NULL || vel==NULL || enc==NULL) {
		printf("Cannot get interface to robot head\n");
		robotHead.close();
	}

	int jnts = 0;

	pos->getAxes(&jnts);

	Vector tmp;
	Vector encoders;
	Vector command_position;
	Vector command_velocity;

	tmp.resize(jnts);
	encoders.resize(jnts);
	command_position.resize(jnts);
	command_velocity.resize(jnts);

	int j;
	for (j = 0; j < jnts; j++) {
	tmp[j] = 5.0;
	}
	pos->setRefSpeeds(tmp.data());


	pos->positionMove(0,0);
	pos->positionMove(1,10);
	pos->positionMove(2,0);

	robotHead.close();

}
