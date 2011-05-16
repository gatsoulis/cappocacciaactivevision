/*
 * sendinput.cpp
 *
 *  Created on: Mar 2, 2011
 *      Author: gabriele
 */

#include "sendinput.h"

using namespace yarp::os;
using namespace yarp::sig;

SendInput::SendInput(std::string name) : Module(name) {

	std::cout << "\nConstructing " << getName() << "...";

	ciao = getName();
//	OutPortLeft.open("/left/out");
//	OutPortRight.open("/right/out");
	OutPortLeft.open(("/"+ciao+"/left/out").c_str());
	OutPortRight.open(("/"+ciao+"/right/out").c_str());
	PointInPortLeft.open(("/"+ciao+"/left/in").c_str());
	PointInPortRight.open(("/"+ciao+"/right/in").c_str());

	debug = false;

	std::cout<<"\nfinished.";



}

SendInput::~SendInput() {

	std::cout<<"\nDestructing " << getName() << "...";

	OutPortLeft.close();
	OutPortRight.close();
	PointInPortLeft.close();
	PointInPortRight.close();
	std::cout<<"\nfinished.";

}

void SendInput::connecting(){

	std::cout<<"\nConnecting " << getName() << "...";

//	Network::connect("/left/out","/motion/left/in");
//	Network::connect("/right/out","/motion/right/in");

	Network::connect(("/"+ciao+"/left/out").c_str(),"/motion/left/in");
	Network::connect(("/"+ciao+"/right/out").c_str(),"/motion/right/in");
	


	std::cout<<"\nfinished.";

}

void SendInput::disconnecting(){

	std::cout<<"\nDisconnecting " << getName() << "...";

	OutPortLeft.interrupt();
	OutPortRight.interrupt();


	std::cout<<"\nfinished.";

}

void SendInput::execute(){

	PointInYarpL = PointInPortLeft.read();
	PointInYarpR = PointInPortRight.read();

	int a = (*PointInYarpL)[0];
	int b = (*PointInYarpL)[1];
	int c = (*PointInYarpL)[2];

	int d = (*PointInYarpR)[0];
	int e = (*PointInYarpR)[1];
	int f = (*PointInYarpR)[2];

	printf("\ndalla porta sinistra yarp : %d", a);
	printf("\ndalla porta sinistra yarp : %d", b);
	printf("\ndalla porta sinistra yarp : %d\n", c);

	printf("\ndalla porta destra yarp : %d", d);
	printf("\ndalla porta destra yarp : %d", e);
	printf("\ndalla porta destra yarp : %d\n", f);


/*	if(!pointLeft) 
		std::cout << "\n" << getName() << "ERROR::execute()::point left NULL";

	if(!pointRight) 
		std::cout << "\n" << getName() << "ERROR::execute()::point right NULL";

	if(!reliabilityLeft) 
		std::cout << "\n" << getName() << "ERROR::execute()::point left NULL";

	if(!reliabilityRight) 
		std::cout << "\n" << getName() << "ERROR::execute()::point right NULL";
*/

	Vector& targetL = OutPortLeft.prepare();
	targetL.resize(3);
	targetL[0] = a;
	targetL[1] = b;
	targetL[2] = c;

	OutPortLeft.write();

	Vector& targetR = OutPortRight.prepare();
	targetR.resize(3);
	targetR[0] = d;
	targetR[1] = e;
	targetR[2] = f;

	OutPortRight.write();



}

