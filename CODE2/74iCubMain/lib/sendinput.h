/*
 * sendinput.h
 *
 *  Created on: Mar 1, 2011
 *      Author: gabriele
 */

#ifndef SENDINPUT_H_
#define SENDINPUT_H_

#include <yarp/os/all.h>
#include <yarp/sig/all.h>

#include "cv.h"
#include "highgui.h"
#include "cvaux.h"

#include "module.h"
#include "inputconnector.h"
#include "outputconnector.h"
#include "cvimage.h"
#include "csctype.h"

#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>


using namespace std;



class SendInput :public Module{
public:
	SendInput(std::string name="SendInputToMotionModule");
	~SendInput();

	void execute();

	void connecting();

	void disconnecting();

	yarp::os::BufferedPort<yarp::sig::Vector> OutPortLeft;
	yarp::os::BufferedPort<yarp::sig::Vector> OutPortRight;

	yarp::os::BufferedPort<yarp::sig::Vector> PointInPortLeft;
	yarp::os::BufferedPort<yarp::sig::Vector> PointInPortRight;


private:

	std::string ciao;
	bool debug;
	
protected:

	yarp::sig::Vector *PointInYarpL;
	yarp::sig::Vector *PointInYarpR;

};

#endif /* SENDINPUT_H_ */
