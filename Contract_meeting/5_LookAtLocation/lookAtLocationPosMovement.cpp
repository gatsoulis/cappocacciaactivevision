#include <stdio.h>
#include <yarp/os/all.h>
#include <yarp/sig/all.h>
#include <yarp/dev/all.h>
using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::dev;
int main() {
  Network yarp; // set up yarp
  BufferedPort<Vector> targetPort;
  targetPort.open("/tutorial/target/in");
  Network::connect("/tutorial/target/out","/tutorial/target/in");

  Property options;
  options.put("device", "remote_controlboard");
  options.put("local", "/tutorial/motor/client");
  options.put("remote", "/icub/head");
  PolyDriver robotHead(options);
  if (!robotHead.isValid()) {
    printf("Cannot connect to robot head\n");
    return 1;
  }
  IPositionControl *pos;
  IVelocityControl *vel;
  IEncoders *enc;
  robotHead.view(pos);
  robotHead.view(vel);
  robotHead.view(enc);
  if (pos==NULL || vel==NULL || enc==NULL) {
    printf("Cannot get interface to robot head\n");
    robotHead.close();
    return 1;
  }
  int jnts = 0;
  pos->getAxes(&jnts);
  Vector setpoints;
  setpoints.resize(jnts);
int cnt=0;bool temp;int cnt2=0;int joint=0;
  while (1) { // repeat forever
    Vector *target = targetPort.read();  // read a target
    if (target!=NULL) { // check we actually got something
      // printf("We got a vector containing");
       for (int i=0; i<target->size(); i++) {
         //printf(" %g", (*target)[i]);
       }
       printf("\n");

       double x = (*target)[0];
       double y = (*target)[1];
       double conf = (*target)[2];

       x -= 320/2;
       y -= 240/2;

       double vx = x*0.1;
       double vy = -y*0.1;

       // prepare command
 if(cnt>2)
temp =false;

else if(cnt<-2)
temp =true;
if (temp ==true)
cnt++;
else if (temp==false)
cnt--;
       for (int i=0; i<jnts; i++) {
         setpoints[i] = 0.0;//(double)cnt;
       }

	printf("enter joint number \n");

	if(cnt2>10)
{cnt2==0;
printf("new joint \n");
joint++; if (joint>=5) joint=0;
}
else
cnt2++;

	
	
	
	setpoints[0] = (double) cnt;

       /*if (conf>0.5) {
         setpoints[3] = vy;
         setpoints[4] = vx;
       } else {
         setpoints[3] = 0;
         setpoints[4] = 0;
       }*/
             for (int i=0; i<jnts; i++) {
       printf("%f ", setpoints[i]);
         }
         printf("\n");
       //vel->velocityMove(setpoints.data());

       pos->positionMove(setpoints.data());
       getchar();
    }
  }
  return 0;
}
