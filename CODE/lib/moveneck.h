/*
 * DrawAtt.h
 *
 *  Created on: Mar 1, 2011
 *      Author: gabriele
 */

#ifndef MOVENECK_H_
#define MOVENECK_H_

#include "cv.h"
#include "highgui.h"
#include "cvaux.h"

#include "module.h"
#include "inputconnector.h"
#include "outputconnector.h"
#include "cvimage.h"
#include "csctype.h"

#include <yarp/os/all.h>
#include <yarp/sig/all.h>
#include <yarp/dev/all.h>

#include "camera.h"

#include <iostream>
#include <stdio.h>
#include <string>
#include <math.h>

using namespace std;

class MoveNeck :public Module{
public:
	MoveNeck(std::string name="Moveneck");
	~MoveNeck();

	void execute();



private:

	/*yarp::os::Property options;

	yarp::dev::IPositionControl *pos;
	yarp::dev::IVelocityControl *vel;
	yarp::dev::IEncoders *enc;*/
	

};

#endif /*MOVENECK_H_ */
