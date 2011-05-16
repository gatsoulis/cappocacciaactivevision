
#ifndef IndObjTracker_H
#define IndObjTracker_H

#include "module.h"
#include "inputconnector.h"
#include "outputconnector.h"
#include "csctype.h"

#include <iostream> 
#include <string>
#include <vector>

#include <yarp/os/all.h>
#include <yarp/sig/all.h>

	class IndObjTracker : public Module  {

	public: 
		IndObjTracker(std::string name = "IndObjTracker");
		~IndObjTracker();

		void execute();

		void connecting();

		void disconnecting();

		InputConnector<TrackData*> trackInLeft;
		InputConnector<TrackData*> trackInRight;

		bool debug;
		double xL,yL,xR,yR;

		yarp::os::BufferedPort<yarp::sig::Vector> targetPortLeft;
		yarp::os::BufferedPort<yarp::sig::Vector> targetPortRight;

	private:

		TrackData m_trackleft;
		TrackData m_trackright;

		std::string bella;

};

#endif
