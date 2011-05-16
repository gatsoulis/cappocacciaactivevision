#include <math.h>
#include <stdio.h>
#include <string>
#include <iostream>

#include <yarp/os/all.h>
#include <yarp/sig/all.h>
#include <yarp/dev/all.h>

#define CTRL_THREAD_PER     0.03        // [s]0.02
#define STATE_ROTATE_HOME   7
#define STATE_WAIT	    6
#define STATE_HOME	    5
#define STATE_RESTART       4
#define STATE_ROTATE_LEFT   3
#define STATE_ROTATE_RIGHT  2
#define STATE_TRACK         1
#define STATE_IDLE          0

//const char targetPortUVL[] = "/tutorial/ind/target/left/out";
//const char targetPortUVR[] = "/tutorial/ind/target/right/out";
const char targetPortUVL[] = "/Attention_Left/out";
const char targetPortUVR[] = "/Attention_Right/out";
const char targetPortUVL2[] = "/cueintegration/left/out";
const char targetPortUVR2[] = "/cueintegration/right/out";

YARP_DECLARE_DEVICES(icubmod)

using namespace std;
using namespace yarp;
using namespace yarp::os;
using namespace yarp::dev;
using namespace yarp::sig;


class ControlThread: public yarp::os::RateThread
{

protected:

	PolyDriver *clientGaze;
	PolyDriver *head;

	IGazeControl *igaze;

	IPositionControl *ipos;
	IVelocityControl *ivel;
	IEncoders        *iencs;

	Vector setpoints;
	Vector tmp;
	Vector encoders;
	Vector commands_position;
	Vector commands_velocity;
	Vector checkpos;

	//int startup_context_id;

	BufferedPort<Bottle> MotionPortLeft;
	BufferedPort<Bottle> MotionPortRight;

	BufferedPort<Bottle> MotionSavePort;

	string TypeL;
	string TypeR;
	string ImageSaved;
	
	double xL;
	double yL;

	double xR;
	double yR;

	double confL;
	double confR;

	double distxL;
	double distyL;

	double distxR;
	double distyR;
	
	double distL;
	double distR;

	/*double adistxL;
	double adistyL;
	
	double adistxR;
	double adistyR;*/

	double x;
	double y;

	int joints;

	Vector  pxl, cl, pxr, cr;

	double velocityFactor;

	bool debug;
	


	
public:
     ControlThread(const double period) : RateThread(int(period*1000.0)) { }
	
	bool moveleft;
	bool moveright;
	bool movehome;
	int state;
	bool start_rot;	
	bool gohome;
	double startroll;
	
	virtual bool threadInit()
	{
		debug = true;

		moveleft=false;
		moveright=false;
		movehome=false;
		start_rot  = true;
		gohome = true;

		//initialize here variables
		printf("ControlThread:starting\n");

		Property optGaze("(device gazecontrollerclient)");
		optGaze.put("remote","/iKinGazeCtrl");
		optGaze.put("local","/AutonomousMotion/gaze_client");

		Property optHead;
		optHead.put("device", "remote_controlboard");
		optHead.put("remote", "/icub/head");
		optHead.put("local", "/AutonomousMotion/local/head");

		// specify the port which will be used
		// when tracking.
		MotionPortLeft.open("/motion/left/in");
		MotionPortRight.open("/motion/right/in");

		MotionSavePort.open("/motion/save/in");
	
		clientGaze = new PolyDriver;
		if (!clientGaze->open(optGaze))
		{
			delete clientGaze;    
			return false;
		}

		// open the view
		clientGaze->view(igaze);

		// latch the controller context in order to preserve
		// it after closing the module
		// the context contains the tracking mode, the neck limits and so on.
		//igaze->storeContext(&startup_context_id);
		head = new PolyDriver;
	        if (!head->open(optHead))
        	{
        		delete head;    
			return false;
        	}

	        // open the view
		head->view(iencs);
		head->view(ivel);
		head->view(ipos);

		// set trajectory time:
		igaze->setNeckTrajTime(1.5);
		igaze->setEyesTrajTime(1.0);

		// put the gaze in tracking mode, so that
		// when the head moves, the gaze controller 
		// will compensate for it
		//igaze->setTrackingMode(true);

		if ( (!iencs) || (!ivel) || (!ipos) )
		{
			printf("Cannot get interface to robot head\n");
			return false;
		}

		joints = 0;
		startroll=0.0;
	
		iencs->getAxes(&joints);
	
		encoders.resize(joints);
		tmp.resize(joints);
		commands_position.resize(joints);
		commands_velocity.resize(joints);
		setpoints.resize(joints);
		checkpos.resize(joints);

		/*int j;
		for (j = 0; j < joints; j++) {
			commands_position[j] = 0.0;
		}

		int i;
		for (i = 0; i < joints; i++) {
			tmp[i] = 5.0;
		}
		ipos->setRefSpeeds(tmp.data());*/
		
		//commands_velocity=10000;
		//ivel->setRefAccelerations(commands.data());

		velocityFactor = 0.5;
		
		pxl.resize(2);
		pxr.resize(2);
		cl.resize(2);
		cr.resize(2);

		cl[0]=160.0;     
		cl[1]=120.0;
		cr[0]=160.0;
		cr[1]=120.0;

		Network::connect("/optical/save/out","/motion/save/in");

		state=STATE_HOME;

		return true;
	}

	virtual void afterStart(bool s)
	{
		if (s)
		fprintf(stdout,"Thread started successfully\n");
		else
		fprintf(stdout,"Thread did not start\n");        
	}

	virtual void run()
	{	

		if (state==STATE_TRACK)
		{
			//retrieve the target position
			Bottle *targetIndL=NULL;
			Bottle *targetIndR=NULL;
			targetIndL = MotionPortLeft.read();//.read(false)
			targetIndR = MotionPortRight.read();//.read(false)
			if (targetIndL != NULL && targetIndL->size()==4 && targetIndR != NULL && targetIndR->size()==4)
			{

				xL = targetIndL->get(0).asDouble();
				yL = targetIndL->get(1).asDouble();
				confL = targetIndL->get(2).asDouble();
				TypeL = targetIndL->get(3).asString();				

				xR = targetIndR->get(0).asDouble();
				yR = targetIndR->get(1).asDouble();
				confR = targetIndR->get(2).asDouble();
				TypeR = targetIndR->get(3).asString();

				if(confL == 0 || confR == 0)
				{

					igaze->lookAtStereoPixels(cl,cr); 		

				}		
				else
				{
	
					distxL = xL - 160;
					distyL = yL - 120;

					distxR = xR - 160;
					distyR = yR - 120;

					x = (distxL + distxR)/2;
					y = (distyL + distyR)/2;

					distL = sqrt(distxL*distxL+distyL*distyL);
					distR =	sqrt(distxR*distxR+distyR*distyR);				

					if((distL>25.0 || distR>25.0) && (TypeL == "LookAtStereoPixel" && TypeR == "LookAtStereoPixel"))
					//if(TypeL == "LookAtStereoPixel" && TypeR == "LookAtStereoPixel")
					{

						if (TypeL == "LookAtStereoPixel" && TypeR == "LookAtStereoPixel")
							printf("\nLOOK AT STEREO PIXEL\n");
						else if (TypeL == "LookAtLocation" && TypeR == "LookAtLocation")
							printf("\nLOOK AT LOCATION\n");
						else 
							printf("\nLEFT AND RIGHT NOT THE SAME\n");

						printf("STATE_TRACK MOVE HEAD DISTANCE>25\n");
						printf("distanceL = %f\ndistanceR = %f\n",distL,distR);
						printf("distxL = %f, distxR = %f\n", distxL, distxR);
						printf("distyL = %f, distyR = %f\n", distyL, distyR);


						pxl[0]=cl[0]+velocityFactor*distxL;
						pxl[1]=cl[1]+velocityFactor*distyL;
						pxr[0]=cr[0]+velocityFactor*distxR;
						pxr[1]=cr[1]+velocityFactor*distyR;
			
						igaze->lookAtStereoPixels(pxl,pxr);
						//igaze->waitMotionDone();


					}
					
					confL = 0;
					confR = 0;

					while (confL == 0 || confR ==0)
					{

						targetIndL = MotionPortLeft.read();
						targetIndR = MotionPortRight.read();

						xL = targetIndL->get(0).asDouble();
						yL = targetIndL->get(1).asDouble();
						confL = targetIndL->get(2).asDouble();
						TypeL = targetIndL->get(3).asString();


						xR = targetIndR->get(0).asDouble();
						yR = targetIndR->get(1).asDouble();
						confR = targetIndR->get(2).asDouble();
						TypeR = targetIndR->get(3).asString();

						if (confL !=0 && confR !=0)
							break;	

						igaze->lookAtStereoPixels(cl,cr);  
						igaze->waitMotionDone();

					}	

					
					distxL = xL - 160;
					distyL = yL - 120;

					distxR = xR - 160;
					distyR = yR - 120;

					x = (distxL + distxR)/2;
					y = (distyL + distyR)/2;

					distL = sqrt(distxL*distxL+distyL*distyL);
					distR =	sqrt(distxR*distxR+distyR*distyR);


					if((distL<=25.0 && distR<=25.0) || (TypeL == "LookAtLocation" && TypeR == "LookAtLocation"))
					//else if(TypeL == "LookAtLocation" && TypeR == "LookAtLocation")
					{
						if (TypeL == "LookAtStereoPixel" && TypeR == "LookAtStereoPixel")
							printf("\nLOOK AT STEREO PIXEL\n");
						else if (TypeL == "LookAtLocation" && TypeR == "LookAtLocation")
							printf("\nLOOK AT LOCATION\n");
						else 
							printf("\nLEFT AND RIGHT NOT THE SAME\n");

						printf("STATE_TRACK MOVE ONLY EYES DISTANCE <= 25\n");						
						printf("distanceL = %f\ndistanceR = %f\n", distL, distR);

						double vx=0;
						double vy=0;
						double vz=0;

						if(fabs(distxL)>=4.1 || fabs(distxR)>=4.1 )
						{
							printf("STATE_TRACK MOVING EYES\n");
							printf("distxL = %f, distxR = %f\n", distxL, distxR);
							printf("distyL = %f, distyR = %f\n", distyL, distyR);

							if((distxL>=0 && distxR>=0) || ( distxL<=0 && distxR<=0))
							{

								if(distxL>=0.0){
									vx=1*log(fabs(x)+1)*4;
									printf("smooth pursuit x right\n");
									printf("vx = %f\n",vx);
								}
								else{
									vx=-1*log(fabs(x)+1)*4;
									printf("smooth pursuit x left\n");
									printf("vx = %f\n",vx);
								}

							}
							if((distyL>=0.0 && distyR>=0.0) ||( distyL<=0.0 && distyR<=0.0))
							{
								if(distyL>=0.0){
									vy = -log(fabs(y)+1)*4;
									printf("smooth pursuit y down\n");
									printf("vy = %f\n",vy);
								}
								else{
									vy = log(fabs(y)+1)*4;
									printf("smooth pursuit y up\n");
									printf("vy = %f\n",vy);
								}
							}
							if((distyL>=0 && distyR<=0))
							{
							
								if(fabs(distyL)>=fabs(distyR))
								
									vy = 1.0;//(log(fabs(distyL)+1))*4;
								
								else
									vy = -1.0;//(log(fabs(distyR)+1))*4;		

							}
							if((distyL<=0 && distyR>=0))
							{
							
								if(fabs(distyL)>=fabs(distyR))
								
									vy = -1.0;//(log(fabs(distyL)+1))*4;
								
								else
									vy = 1.0;//(log(fabs(distyR)+1))*4;		

							}
							if(distxL>=0.0 && distxR <= 0.0)
							{
								vz=(log(fabs(x)+1))*4;
								printf("diverging\n");
								printf("vz = %f\n",vz);

							}
							if(distxL<=0.0 && distxR>=0.0)
							{
								vz=-(log(fabs(x)+1))*4;
								printf("converging\n");
								printf("vz = %f\n",vz);

							}



						}
						else 
						{
							printf("STATE_TRACK DO NOTHING\n");
							printf("distxL = %f, distxR = %f\n", distxL, distxR);
							printf("distyL = %f, distyR = %f\n", distyL, distyR);


											
						}

						// prepare command
						for (int i=0; i<joints; i++) 
						{
					  		setpoints[i] = 0;
						}

						setpoints[3] = vy;
						setpoints[4] = vx;
						setpoints[5] = vz;


						ivel->velocityMove(setpoints.data());

						confL = 0;
						confR = 0;
			
						while (confL == 0 || confR ==0)
						{
							targetIndL = MotionPortLeft.read();
							targetIndR = MotionPortRight.read();

							xL = targetIndL->get(0).asDouble();
							yL = targetIndL->get(1).asDouble();
							confL = targetIndL->get(2).asDouble();
							TypeL = targetIndL->get(3).asString();


							xR = targetIndR->get(0).asDouble();
							yR = targetIndR->get(1).asDouble();
							confR = targetIndR->get(2).asDouble();
							TypeR = targetIndR->get(3).asString();

							if (confL !=0 && confR !=0)
								break;	

							setpoints[3] = 0;
							setpoints[4] = 0;
							setpoints[5] = 0;
	
							ivel->velocityMove(setpoints.data());

						}

						distxL = xL - 160;
						distyL = yL - 120;

						distxR = xR - 160;
						distyR = yR - 120;

						x = (distxL + distxR)/2;
						y = (distyL + distyR)/2;
						
						distL = sqrt(distxL*distxL+distyL*distyL);
						distR =	sqrt(distxR*distxR+distyR*distyR);

						if(fabs(distxL)<4.1 && fabs(distxR)<4.1 )
						{
							if(start_rot == true && (TypeL == "LookAtLocation" && TypeR == "LookAtLocation"))
							{
								iencs->getEncoder(1,&startroll);
								printf("\nPosition joint 1 (neck roll) = %f\n",startroll);
								start_rot = false;								
								state = STATE_ROTATE_LEFT;
							}
						}

					}
			
				}

			}
			else
				printf("\nInput not good!\n");

		}

		if(state==STATE_ROTATE_RIGHT)
		{
			moveleft=false;
			moveright=true;
			movehome=false;
			/*igaze->setNeckTrajTime(6.5);
			igaze->setEyesTrajTime(5.0);
			Vector fp(3);
			igaze->getFixationPoint(fp); 
			igaze->lookAtFixationPoint(fp);
			igaze->setTrackingMode(true);
			igaze->blockNeckRoll(20.0);	
			igaze->waitMotionDone();
			fprintf(stdout,"\nmotion done roll blocked at 20°");*/
			state=STATE_TRACK;
		}
		if(state==STATE_ROTATE_LEFT)
		{
			moveright=false;
			moveleft=true;
			movehome=false;
			/*igaze->setNeckTrajTime(6.5);
			igaze->setEyesTrajTime(5.0);
			Vector fp(3);
			igaze->getFixationPoint(fp); 
			igaze->lookAtFixationPoint(fp);
			igaze->setTrackingMode(true);
			igaze->blockNeckRoll(-20.0);	
			igaze->waitMotionDone();
			fprintf(stdout,"\nmotion done roll blocked at -20°");*/
			state=STATE_TRACK;
		}
		if(state==STATE_RESTART)
		{
			moveright=false;
			moveleft=false;
			movehome=false;
			start_rot = true;	
			state=STATE_TRACK;
		}
		if(state==STATE_HOME)
		{
			gohome=true;
			moveleft=false;
			moveright=false;
			movehome=false;

		}
		if(state==STATE_WAIT)
		{
			printf("\nWaiting Buffer...\n");
			moveleft=false;
			moveright=false;
			movehome=false;
			Bottle *savein=NULL;
			savein = MotionSavePort.read();
			ImageSaved = savein->get(0).asString();
			if (ImageSaved == "ImageSaved")
			{
				printf("Buffer Complete\niCub can finally go home!\n");
				state=STATE_HOME;
			}
		}
		if(state==STATE_ROTATE_HOME)
		{
			moveleft=false;
			moveright=false;
			movehome=true;
			/*igaze->setNeckTrajTime(6.5);
			igaze->setEyesTrajTime(5.0);
			Vector fp(3);
			igaze->getFixationPoint(fp); 
			igaze->lookAtFixationPoint(fp);
			igaze->setTrackingMode(true);
			igaze->blockNeckRoll(20.0);	
			igaze->waitMotionDone();
			fprintf(stdout,"\nmotion done roll blocked at 20°");*/
			state=STATE_TRACK;
		}


	}

	void threadRelease()
	{
		printf("ControlThread:stopping the robot\n");

		MotionPortLeft.interrupt();
		MotionPortLeft.close();

		MotionPortRight.interrupt();
		MotionPortRight.close();
		
		MotionSavePort.interrupt();
		MotionSavePort.close();

	        // we require an immediate stop
		// before closing the client for safety reason
		// (anyway it's already done internally in the
		// destructor)
		igaze->stopControl();
		ivel->stop();
		ipos->stop();

		// it's a good rule to restore the controller
		// context as it was before opening the module
		//igaze->restoreContext(startup_context_id);


		delete head;
		delete clientGaze;
		
		printf("\nDone, goodbye from ControlThread\n");
	}

	void setMode(int mode)
	{
		state=mode;
	}

	BufferedPort<Bottle>* getMotionPortLeft(void)
	{
		return &MotionPortLeft;
	}
	BufferedPort<Bottle>* getMotionPortRight(void)
	{
		return &MotionPortRight;
	}

};


class AutoMovModule:public RFModule
{
	
	ControlThread *thr;
protected:

	PolyDriver *neck;

	IPositionControl *pos;
	IVelocityControl *vel;
	IEncoders        *encs;

	Vector ntmp;
	Vector nencoders;
	Vector ncommands_position;
	Vector ncommands_velocity;

	int jnts;
	double joint1;


public:
	AutoMovModule(){}
	

	Port cmdPort;
	Port handlerPort;

	/* 
	* Configure function. Receive a previously initialized
	* resource finder object. Use it to configure your module.
	* Open port and attach it to message handler.
	*/
	bool configure(ResourceFinder &rf)
	{
	
		Time::turboBoost();

		Property optNeck;
		optNeck.put("device", "remote_controlboard");
		optNeck.put("remote", "/icub/head");
		optNeck.put("local", "/AutonomousMotion/local/neck");

		neck = new PolyDriver;
	        if (!neck->open(optNeck))
        	{
        		delete neck;    
			return false;
        	}

	        // open the view
		neck->view(encs);
		neck->view(vel);
		neck->view(pos);

		if ( (!encs) || (!vel) || (!pos) )
		{
			printf("Cannot get interface to robot head\n");
			return false;
		}

		jnts = 0;
	
		encs->getAxes(&jnts);
	
		nencoders.resize(jnts);
		ntmp.resize(jnts);
		ncommands_position.resize(jnts);
		ncommands_velocity.resize(jnts);


		int j;
		for (j = 0; j < jnts; j++) {
			ntmp[j] = 1.0;
		}
		pos->setRefSpeeds(ntmp.data());
	
		//commands_velocity=10000;
		//ivel->setRefAccelerations(commands.data());


		thr = new ControlThread(CTRL_THREAD_PER);
		if (!thr->start())
		{
			delete thr;
			return false;
		}



		handlerPort.open("/Autonomous/motion/command:i");
		//opening the port to send commands to the 
		cmdPort.open("/Autonomous/motioin/command:o");

		attach(handlerPort);
		attachTerminal(); 


		return true;
	}




	bool respond(const Bottle& command, Bottle& reply) 
	{
		fprintf(stderr, "Got something, echo is on\n");	
		if (command.get(0).asString()=="quit")
		{	
			//thr->threadRelease();
			//close();
			return 0;
		}
		else if (command.get(0).asString()=="track")
		{	
			fprintf(stderr, "Switching to tracking mode\n");
			thr->setMode(STATE_TRACK);
			/*Contact ctcPortL = yarp::os::NetworkBase::queryName(targetPortUVL);
			Contact ctcPortR = yarp::os::NetworkBase::queryName(targetPortUVR);
			if (ctcPortL.isValid() && ctcPortR.isValid())
			{
				yarp::os::NetworkBase::connect(targetPortUVL, thr->getMotionPortLeft()->getName());
				yarp::os::NetworkBase::connect(targetPortUVR, thr->getMotionPortRight()->getName());
			}
			else
				fprintf(stderr, "The track cannot be performed since no tracker is available\n");*/
			return true;
		}
		else if (command.get(0).asString()=="rotateright")
		{	
			fprintf(stderr, "Rotate head\n");
			thr->setMode(STATE_ROTATE_RIGHT);
/*			Contact ctcPort = yarp::os::NetworkBase::queryName(targetPortUV);
			if (ctcPort.isValid())
			{
				fprintf(stderr, "Connecting the uv->3D port...");
				yarp::os::NetworkBase::connect(targetPortUV, uv2TableInputPort, "mcast");
				fprintf(stderr, "ok!\n");
				//grasp the object on the table
				graspOnTable();
				fprintf(stderr, "Disconnecting the uv->3D port...");
				yarp::os::NetworkBase::disconnect(targetPortUV, uv2TableInputPort);
				fprintf(stderr, "ok!\n");
			}
			else
				fprintf(stderr, "The table mode cannot be performed since no tracker is available\n");
*/	
			return true;
		}
		else if (command.get(0).asString()=="rotateleft")
		{	
			fprintf(stderr, "Rotate head\n");
			thr->setMode(STATE_ROTATE_LEFT);
			return true;
		}
		else if (command.get(0).asString()=="restart")
		{	
			fprintf(stderr, "Restart\n");
			thr->setMode(STATE_RESTART);
			return true;
		}
		else if (command.get(0).asString()=="idle")
		{	
			fprintf(stderr, "Idle\n");
			thr->setMode(STATE_IDLE);
			return true;
		}
		else if (command.get(0).asString()=="home")
		{	
			fprintf(stderr, "Going home position\n");
			thr->setMode(STATE_HOME);
			return true;
		}
		else if (command.get(0).asString()=="rotateright")
		{	
			fprintf(stderr, "Rotate head\n");
			thr->setMode(STATE_ROTATE_HOME);
			return true;
		}
		else	
		{					
			reply=command;
			return true;
		}	
	}


	/*
	* Close function, to perform cleanup.
	*/
	virtual bool close()
	{

		encs->getEncoders(nencoders.data());

		for(int i =0; i<jnts;i++)
		{
			while(nencoders[i]>0.3 || nencoders[i]<-0.3)
			{
				if (nencoders[i]>0.3)
					vel->velocityMove(i,-3);
				else if(nencoders[i]<-0.3)
					vel->velocityMove(i,3);
				encs->getEncoders(nencoders.data());
			}
		}

		printf("\niCub is at home :)\nGoing to STATE_IDLE\n");

		thr->stop();
		printf("Thread stopped\n");
		delete thr;
		printf("Thread deleted\n");

		pos->stop();
		printf("Position control stopped\n");
		delete neck;
		printf("Neck deleted\n");

		cmdPort.interrupt();
		cmdPort.close();
		handlerPort.interrupt();
		handlerPort.close();

		return true;
	}

	virtual double getPeriod()    { return 0.01;}//1.0;  }
	virtual bool   updateModule() 
	{ 
		//NEW
		if(thr->gohome==true)
		{
			encs->getEncoders(nencoders.data());
			if     (nencoders[0]<0.3 && nencoders[0]>-0.3 && 
				nencoders[1]<0.3 && nencoders[1]>-0.3 &&
				nencoders[2]<0.3 && nencoders[2]>-0.3 &&
				nencoders[3]<0.3 && nencoders[3]>-0.3 &&
				nencoders[4]<0.3 && nencoders[4]>-0.3)
			{
				thr->state=STATE_IDLE;
				thr->gohome=false;
				printf("\niCub is at home :)\nGoing to STATE_IDLE\n");
			}
			for(int i =0; i<jnts;i++)
			{
				if (nencoders[i]>0.3)
					vel->velocityMove(i,-3);
				else if(nencoders[i]<-0.3)
					vel->velocityMove(i,3);
			}

		}

		if(thr->moveleft==true)
		{		
			encs->getEncoder(1,&joint1);
			if(joint1 >= -19.8)
			{
				vel->velocityMove(1,-1.0);
				encs->getEncoder(1,&joint1);
			}
			else
			{
				thr->state=STATE_ROTATE_RIGHT;
			}
		}
		if(thr->moveright==true)
		{
			encs->getEncoder(1,&joint1);
			if(joint1 <= 19.8)
			{
				vel->velocityMove(1,1.0);
				encs->getEncoder(1,&joint1);
			}
			else
			{
				thr->state=STATE_ROTATE_HOME;
			}	
		}
		if(thr->movehome==true)
		{
			encs->getEncoder(1,&joint1);
			if(joint1 <= (thr->startroll + 0.2) && joint1 >= (thr->startroll -0.2) )
			{	
				thr->state=STATE_WAIT;
			}
			else
			{
				if (joint1 > (thr->startroll + 0.3))
					vel->velocityMove(1,-1.0);
				else if (joint1 < (thr->startroll - 0.3))
					vel->velocityMove(1,1.0);
			}	
		}
		return true;
	 
	 }
};


int main(int argc, char *argv[])
{
	putenv("GABBO_ROOT=/media/Dati/Testing/GaborMatching");

	ResourceFinder rf;
	rf.setVerbose(true);
	rf.setDefaultConfigFile("motion.ini");
	rf.configure("GABBO_ROOT",argc,argv);

	Network yarp;

	if (!yarp.checkNetwork())
	{
		printf("No yarp network, quitting\n");
		return false;
	}
	
	// we need to initialize the drivers list 
	YARP_REGISTER_DEVICES(icubmod)

	AutoMovModule mod;

	return mod.runModule(rf);


}




