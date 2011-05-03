//REV: Jan 2010, saliency map implementation/functions for running it in real time.

#include "env_alloc.h"
#include "env_c_math_ops.h"
#include "env_image.h"
#include "env_image_ops.h"
#include "env_log.h"
#include "env_mt_visual_cortex.h"
#include "env_params.h"
#include "env_pthread_interface.h"
#include "env_stdio_interface.h"
#include "env_visual_cortex.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h> // for atoi(), malloc(), free()
#include <math.h>
#include "envision_lib.h"
#include "opencv/cxcore.h"//this has the IplImage definition in it...
#include <sys/time.h>
//#include <gsl/gsl_randist.h> //for gamma random
//#include <gsl/gsl_rng.h> //gsl random number generator

// INHIB_TIME * 20ms slices
#define INHIB_TIME 25 //120  //*20ms???  //100
#define INHIB_DELAY INHIB_TIME-15//20 //50//INHIB_TIME-10   //let it keep its max for 20 steps... then go inhibit for the difference
//NO! need a better mechanism for inhibition. A constant function is nicest, was using a linear one before...
//i.e. const/time_remain, so 1/100, 1/99, 1/98...1/3, 1/2, 1/1, etc.
//if we just apply a constant weight, then it will be suppressed based on membrane TC, and then 
//will remain fully supressed for INHIB_TIME...
//if winner suppression weight and loser supression weight are equivalent, then they will both be supressed
//same amount for same time... we actually want a hard delay before the constant inhib signal BEGINS, 
//and then have it continue for some time. But, why would that happen? What mechanism could make a hard 
//delay before inhibition begins?
//How about a longer-TC neuron that is up higher-level, that is what sends the signal, but it takes time to 
//reach threshold, and then it suddenly reaches threshold and starts sending? That works. So, pretty much, 
//a step function, with delay of INHIB_DELAY, and extent INHIB_TIME (before that one get's inhibited too...etc?)
//turtles all the way down, lol.

#define SMALL_NUM 0.00001

//gsl_rng* gsl_rand_source; //for uniform random

int x_loc(int, int);
int y_loc(int, int);
int x_y_to_array_loc(int, int, int);

struct WTA_LIF_Neuron 
{
  float Vm;    //membrane potential of neuron
  float Isyn;  //input current to neuron
  //float Inhib; //external inhibition. Normally 0, but follows a "hump" function after this neuron begins to fire
  int inhib_timer;
  
  int mass_points; //for normalizing on the edges.
};

//REV: for saliency map
//int sal_framenum;
int sal_winner_coords[2]; //winner coordinates of *this time-step*
int WTA_winner_coords[2]; //current winner-take-all(WTA) winner coords
int WTAnet_winner; // = -1;
int overt_attn_shift; //= false;

float MASS_NORM; // = MASS_pixels * MASS_pixels * 255;
int MASS_pixels; // = 5;
int MASS_radius;

unsigned char current_sal_color[4]; //REV: r, g, b and char as bool for overt attn shift
//pthread_mutex_t saliencyDataMutex;  //REV: for locking shared data between envision and C_Camera

pthread_mutex_t sal_mutex = PTHREAD_MUTEX_INITIALIZER; //init mutex...

int salmap_w;
int salmap_h;
float* salmap;
struct WTA_LIF_Neuron* WTAnet;

double decay; //30MS tau, assumed 10ms update... (too much?)
double decay_complement;


int INITIALIZEDR = 0;
int INITIALIZEDL = 0;
int FIRSTFRAMER = 1;
int FIRSTFRAMEL = 1;


struct timeval tv;
struct timezone tz;
struct tm *tm;


int x_loc(int array_loc, int width)
{
  return array_loc%width;
}

int y_loc(int array_loc, int width)
{
  return array_loc/width;
}

int x_y_to_array_loc(int x_pos, int y_pos, int width)
{
  return (width*(y_pos)) + x_pos;
}

// ######################################################################
// Thunk to convert from env_size_t to size_t
static void* malloc_thunk(env_size_t n)
{
  return malloc(n);
}

// ######################################################################
struct status_data
{
  int frame_number;
};

//REV: some globals...
struct env_params envp; //REV: hack for left and right eye!
struct env_visual_cortex ivcR; //REV: this is holding each previous img (lowpass5 filtered) for motion
struct env_visual_cortex ivcL;
env_size_t npixels = 0;

//REV: just for now...
int multithreaded=0;



int envision_initL(int width, int height)
{
  
  printf("INITIALIZING...");
  
 
  //allocate memory for saliency map here...(if need dynamic)
  /*WTAnet = malloc( sizeof(struct SalMap_Neuron) * width * height); //er...hope this works.
  //printf("sizeof salmap neuron: %i ", sizeof(struct SalMap_Neuron));
  */
  
  MASS_pixels = width/3; ///4// = 5; 
  MASS_radius = MASS_pixels/2;
  
  //this inits WTA salmap data stored in SalMapData..
  if(salmap == NULL) 
    {
      init_salmap(width, height);
      decay = exp(-1.0); //decay exp(-0.2) ~= .85?.8? @ every time step...
      decay_complement = 1.0 - decay;
      //printf("DECAY SPEED: %f\n", decay);
    }
  WTAnet_winner = -1; // = -1;
  overt_attn_shift = 0; //= false;
  
  //MASS_NORM = MASS_pixels*MASS_pixels*255;// = MASS_pixels * MASS_pixels * 255;
 
 
  // Instantiate our various ModelComponents:
  env_params_set_defaults(&envp);
  
  envp.maxnorm_type = ENV_VCXNORM_MAXNORM;
  envp.scale_bits = 16;
  
  env_assert_set_handler(&env_stdio_assert_handler);
  if (multithreaded)
    {
      env_init_pthread_alloc();
      env_init_pthread_job_server();
    }
  env_allocation_init(&malloc_thunk, &free);
  //REV: Open the { for the unqualified { } section here.
  
  
  env_visual_cortex_init(&ivcL, &envp);  
  printf("FINISHED SALIENCY MAP INITIALIZATION (LEFT EYE)\n");
  return 0;
} //end envision_initL

int envision_initR(int width, int height)
{
  
  printf("INITIALIZING...");
  
 
  //allocate memory for saliency map here...(if need dynamic)
  /*WTAnet = malloc( sizeof(struct SalMap_Neuron) * width * height); //er...hope this works.
  //printf("sizeof salmap neuron: %i ", sizeof(struct SalMap_Neuron));
  */
  
  MASS_pixels = width/3; ///4// = 5; 
  MASS_radius = MASS_pixels/2;
  
  //this inits WTA salmap data stored in SalMapData..
  if(salmap == NULL) 
    {
      init_salmap(width, height);
      decay = exp(-1.0); //decay exp(-0.2) ~= .85?.8? @ every time step...
      decay_complement = 1.0 - decay;
      //printf("DECAY SPEED: %f\n", decay);
    }
  WTAnet_winner = -1; // = -1;
  overt_attn_shift = 0; //= false;
  
  //MASS_NORM = MASS_pixels*MASS_pixels*255;// = MASS_pixels * MASS_pixels * 255;
 
 
  // Instantiate our various ModelComponents:
  env_params_set_defaults(&envp);
  
  envp.maxnorm_type = ENV_VCXNORM_MAXNORM;
  envp.scale_bits = 16;
  
  env_assert_set_handler(&env_stdio_assert_handler);
  if (multithreaded)
    {
      env_init_pthread_alloc();
      env_init_pthread_job_server();
    }
  env_allocation_init(&malloc_thunk, &free);
  //REV: Open the { for the unqualified { } section here.
  
  
  env_visual_cortex_init(&ivcR, &envp);  
  printf("FINISHED SALIENCY MAP INITIALIZATION (RIGHT EYE)\n");
  return 0;
}

/*
 *WTA net: It should choose the maximum location (either
 * instantaneously, or over a short period of time, i.e. a few
 * frames). That maximum location is the winner (the first neuron to
 * threshold), and becomes activated, inhibiting the rest of the
 * saliency map (by a constant weight or proportional to its current
 * activation? Constant is best as more control. Proportional will
 * create all sorts of dynamics).  However, once activated, it begins
 * to recieve inhibitory input which either increases in strength, or
 * takes time b/c of the time constant of the neuron to fully inhibit
 * it (ideas: sigmoid-shaped inhibitory input function, constant
 * linear input going through time-constant of neuron means a somewhat
 * exponential curve (?), or linear). After some period of time,
 * especially if the input to that area is no longer interesting, a
 * different region will become activated higher than the current
 * winner, and attention will shift to that neuron.  This process will
 * repeat itself. Note: The neurons are LEAKY, so their input has to
 * be enough to keep them activated...so we need inhibitory stuff to
 * increase otherwise we can't guarentee (unless we choose the
 * constant inhib value high enough) that some neuron with
 * super-strong input won't stay forever activated b/c it's able to
 * "beat out" the negative inhib with its positive input.
 *
 */


void resetWTAnet()
{
  //printf("\n\n=============RESETTING WTA NET ===================\n\n");
  pthread_mutex_lock(&sal_mutex);
  WTAnet_winner = -1;
  for(int x=0; x<4; x++)
    {
      current_sal_color[x]=0;
    }
  for(int x=0; x<salmap_w; x++)
    {
      for(int y=0; y<salmap_h; y++)
	{
	  int loc = x_y_to_array_loc(x, y, salmap_w);
	  salmap[loc] = 0;
	  WTAnet[loc].Vm = 0;
	  WTAnet[loc].Isyn = 0;
	  WTAnet[loc].inhib_timer = -1;
	  
	  
	  //what about WTAnet_winner, etc?
	}
    }
  pthread_mutex_unlock(&sal_mutex);
}


float thresh=0.05;

//assume this will be called regularly
void update_WTAnet(unsigned char* win_rgb) //take as argument number of steps/time?
{
  if(salmap != NULL && WTAnet != NULL)
    {
      overt_attn_shift = 0;
      int x,y; 
      
      float max = -100000;
      int max_loc = -1;
      
      //TODO: make it so that the "edge" squares are penalized for being smaller in some way
      //also, is it really right to have the "lapse" between a winner losing threshold and the next one coming?
      //what if it is impossible for a region to pass threshold (there is just not enough input)
      //will that ever exist? Well, it will if the room is entirely too dark...I guess that's realistic.
      
      //only check the areas that are possible (note to self--why not have a falling weight around centre?)
      //to find the max in the current maximum Vm in the WTAnet
      
      pthread_mutex_lock(&sal_mutex);
      int nummaxes=0;
      for(x=0+MASS_radius; x<salmap_w-MASS_radius; x++)
	{
	  for(y=0+MASS_radius; y<salmap_h-MASS_radius; y++)
	    {
	      int loc = x_y_to_array_loc(x, y, salmap_w);
	      if(WTAnet[loc].Vm > max+SMALL_NUM) //max - Vm < 0 ; then Vm is greater
		{
		  max_loc = loc;
		  //printf("Found max: %f > %f)\n", WTAnet[max_loc].Vm, max);
		  nummaxes=1;
		  max = WTAnet[max_loc].Vm;
		}
	      else if(WTAnet[loc].Vm < max+SMALL_NUM && WTAnet[loc].Vm > max-SMALL_NUM) //if its essentially same...
		{
		  ++nummaxes;
		}
	    }
	}
      
      //now with max, go find all maxes (in case there's m ore than one...) and randomly roulette one of them :D
      
      if(nummaxes > 1)
	{
	  int maxwinner=0;//gsl_rng_uniform_int(gsl_rand_source, nummaxes); //generates int [0, nummaxes-1]
	  //printf("FOUND %i maxes (== %f)   chose %i randomly\n", nummaxes, max, maxwinner);
	  nummaxes=0; //use as iter/counter
	
	  for(y=0+MASS_radius; y<salmap_h-MASS_radius; y++)
	    {  
	      for(x=0+MASS_radius; x<salmap_w-MASS_radius; x++)
		{
		  int loc = x_y_to_array_loc(x, y, salmap_w);
		  if(WTAnet[loc].Vm < max+SMALL_NUM && WTAnet[loc].Vm > max-SMALL_NUM)
		    {
		      
		      if(nummaxes == maxwinner)
			{
			  max_loc = loc;
			  //break;
			}
		      
		      ++nummaxes;
		      //printf("1");
		    } else
			{
			  //printf(" ");
			}
		}
	      //printf("\n");
	    }
	  //printf("\n");
	}
      //printf("WTAnet winner %i: %i, %i \n",WTAnet_winner, x_loc(WTAnet_winner, salmap_w), y_loc(WTAnet_winner, salmap_w));
      
      //if max is <= 0...do everything normally...but don't check for thresh
      
      if(/*(max > thresh || max <= 0) &&*/ max_loc != WTAnet_winner) //if, new neuron has passed threshold...
	{
	  //printf("NEW WINNER: %i %i %0.2f\n", x_loc(max_loc, salmap_w), y_loc(max_loc, salmap_w), WTAnet[max_loc].Vm);
	  WTAnet_winner = max_loc;
	  for(x=-MASS_radius; x<=MASS_radius; x++) {
	    for(y=-MASS_radius; y<=MASS_radius; y++) {
	      int xval=x_loc(WTAnet_winner, salmap_w) + x;
	      int yval=y_loc(WTAnet_winner, salmap_w) + y;
	      if(xval >= 0 && xval < salmap_w && yval >=0 && yval < salmap_h) //bounds check
		{ 
		  WTAnet[x_y_to_array_loc(xval, yval, salmap_w)].inhib_timer = INHIB_TIME;
		}
	    }
	  }
	  overt_attn_shift = 1; //yes, we shifted
	}
      
      float inhib_w = 0.2; //should this change depending on the number of neurons? No, since each will get same weight.
      float inhib_signal = 0.3;//0.6 //but, this way, it's same as inhib_weight, and decay will be *as fast* as the
      //other neurons!
      
      for(y=0+MASS_radius; y<salmap_h-MASS_radius; y++)
	{
	  for(x=0+MASS_radius; x<salmap_w-MASS_radius; x++)
	    {
	     
	      int loc = x_y_to_array_loc(x, y, salmap_w);
	      
	      WTAnet[loc].Isyn = salmap[loc]; //adding calculated mass to Isyn (between 1 and 0...)
	      
	      if(WTAnet[loc].Isyn > -SMALL_NUM && WTAnet[loc].Isyn < SMALL_NUM)
		WTAnet[loc].Isyn = 0;
	      
	      if(WTAnet_winner > -1 /*&& (WTAnet[WTAnet_winner].Vm > thresh || WTAnet[WTAnet_winner].Vm <= 0) */
				       && loc != WTAnet_winner) 
		//if winner is above thresh (and exists), inhibit all others
		{
		  WTAnet[loc].Isyn -= inhib_w;
		}
	     
	      if(WTAnet[loc].Isyn > -SMALL_NUM && WTAnet[loc].Isyn < SMALL_NUM)
		WTAnet[loc].Isyn = 0;
	      if(WTAnet[loc].inhib_timer > -1) //if still inhib.., add respression signal..
		{
		  if(WTAnet[loc].inhib_timer < INHIB_DELAY)
		    {
		      //REV: todo: make it so that it SLOWLY decays (linearly?) after inhib...or is offset
		      //of inhibition as sudden as onset? (synapses decay...?)
		      WTAnet[loc].Isyn -= inhib_signal;///(0.25*(WTAnet[loc].inhib_timer+1));
		    }
		  WTAnet[loc].inhib_timer -= 1; //increment timer down regardless..
		}
	      
	      if(WTAnet[loc].Isyn > -SMALL_NUM && WTAnet[loc].Isyn < SMALL_NUM)
		WTAnet[loc].Isyn = 0;
	      
	      //update memb potential
	      WTAnet[loc].Vm = decay * WTAnet[loc].Vm; 
	      if(WTAnet[loc].Vm > -SMALL_NUM && WTAnet[loc].Vm < SMALL_NUM)
		WTAnet[loc].Vm = 0;
	      
	      WTAnet[loc].Vm += decay_complement * WTAnet[loc].Isyn; //wait...Isyn was added in last turn anyways so
	      //we're not 0...
	      
	      //if(overt_attn_shift>0)
	      //{
		  //printf("%i ", WTAnet[loc].inhib_timer);
	      //printf("%04.6f ", WTAnet[loc].Vm);//Isyn //was Vm WTAnet[loc].Isyn
	      //}
	      
	      //printf("%03.2f ", WTAnet[loc].Isyn);//Isyn //was Vm WTAnet[loc].Isyn
	    } //end for all legal x
	  //printf("\n");
	}//end for all legal y
      //printf("\n");
      
      
      //FILL IN win_rgb 0 1 2 3, where 3 is 0 if no overt shift, 1 otherwise
      for(x=0; x<3; x++)
	win_rgb[x] = current_sal_color[x];
      
      win_rgb[3] = overt_attn_shift;
      pthread_mutex_unlock(&sal_mutex);
    }//end if salmap != null
}




//LEFT EYE

//REV: nextpic_frommem: calculates saliency on given input, and draws appropriate pictures on output, 
//given input (assumedly from some other program implementing winner-take-all saliency, etc.)
//we're using global data from SalMapData to share required data/map output with java etc.
//called as quickly as possible in a separate thread (best way to do it?)
int* envision_nextpic_frommemL(const IplImage* ipl_input,  //ipl image as input
			      IplImage* output, int desired_numsalwinners)   //output image we'll write to
{
  
  int* xywinners = malloc(2*sizeof(int)*desired_numsalwinners); //*2 because we are giving 2 points for each.
  
  struct env_dims indims;
  indims.w = ipl_input->width;
  indims.h = ipl_input->height;
  
  //printf("IMAGE SIZE: %i %i \n", ipl_input->width, ipl_input->height);
  //input is ipl_input as const env_rgb_pixel*
  const struct env_rgb_pixel* input = (struct env_rgb_pixel*)ipl_input->imageData;
  
  npixels = indims.w * indims.h;
  sal_winner_coords[0]=-1; sal_winner_coords[1]=-1;
  float width_scale = 320 / indims.w; //this is written for 320...multiply everything hardcoded by scale, right?
  
  //initialize if necessary...note size is pretty manual
  if( !(INITIALIZEDL == 1) ) { envision_initL(indims.w/16, indims.h/16); INITIALIZEDL=1; } 
  
  struct env_image ivcout = env_img_initializer;
  struct env_image intens = env_img_initializer;
  struct env_image color = env_img_initializer;
  struct env_image ori = env_img_initializer;
#ifdef ENV_WITH_DYNAMIC_CHANNELS
  struct env_image flicker = env_img_initializer;
  struct env_image motion = env_img_initializer;
#endif

  struct status_data userdata; //REV: need this?
  //used to have userdata.frame_number = img_num
  
  
  //REV: note this function will call env_visual_cortex_input (the non-multithreaded version) since it's not multithreaded now.
  env_mt_visual_cortex_input(multithreaded,
			     &ivcL, &envp,
			     "visualcortex",
			     input, 0, indims,
			     0, /*&print_chan_status, //REV: want this?*/ //we don't need status_func...if we don't
			     //want to be printing shit.
			     &userdata,
			     &ivcout,
			     &intens, &color,
			     &ori
#ifdef ENV_WITH_DYNAMIC_CHANNELS
			     , &flicker, &motion
#endif
			     );
  
  
  if(FIRSTFRAMEL == 0) //because movement isn't calculated until 2nd frame...only use ivcout for now or it will segfault
    {
      //needs to be inside bc there is nothing to scale first frame (we just do it to build the lpf for motion next time)
      env_visual_cortex_rescale_ranges(
				       &ivcout, &intens, &color, &ori
#ifdef ENV_WITH_DYNAMIC_CHANNELS
				       , &flicker, &motion
#endif
				       );
      
      
      
      //copy ivcout to ipl output image to draw...(I need to interpolate to the image size...)
      char* dataptr = output->imageData;
      int max_loc = -1;
      int max_val = -1;
      int y=0, x=0, z=0, a=0;
      
      
      //calc max_loc here:
      for(x=0; x<salmap_w; x++)
	{
	  for(y=0; y<salmap_h; y++)
	    {
	      int loc = x_y_to_array_loc(x, y, salmap_w);
	      int val = ivcout.pixels[loc];
	      if(val > max_val)
		{
		  max_val = val;
		  max_loc = loc;
		}
	    }
	}
      
      //set external x,y winners so avg color can be calced and passed to java side
      //REV: this is in *SCALED UP* image...(for drawing only..WTAnet is for WTAnet calc too so not scaled)
      sal_winner_coords[0] = x_loc(max_loc, ivcout.dims.w) * 16; //why not *16*16?
      sal_winner_coords[1] = y_loc(max_loc, ivcout.dims.w) * 16; //err should make height scale too
      
      xywinners[0] = sal_winner_coords[0];
      xywinners[1] = sal_winner_coords[1];
      WTA_winner_coords[0] = xywinners[0];
      WTA_winner_coords[1] = xywinners[1];
      
      WTAnet_winner = max_loc; //hope this works?
      
            
      int win_x = WTA_winner_coords[0];//sal_winner_coords[0]; //REV: set to WTAnet when done coding, to use that value... 
      int win_y = WTA_winner_coords[1];//sal_winner_coords[1];
				   
      int c_width = ipl_input->width*3;
      //let's write the picture...1st is FULL SALIENCY, then we have (left-right, up-down, as english)
      //color, motion, (orientation, flicker, intensity)
      int x_scale = ipl_input->width/ivcout.dims.w; //160 / 10 = 16
      //int y_scale = ipl_input->height/ivcout.dims.h; //120 / ?7? = 16
      //printf("DIMSW: %i   DIMSH: %i   X_SCALE: %i   Y_SCALE: %i\n", ivcout.dims.w, ivcout.dims.h, x_scale, y_scale);
      
      
      int startpos_s=0; int startpos_l=0;
      
      int max_loc_l[5];
      for(int x=0; x<5; x++)
	{
	  max_loc_l[x] = -1;
	}
      
      y=0; 
      a=0; 
      z=0; 
      x=0;
      
      
      //REV: write out first two output quarters (top-left, top-right)
      while(y<ivcout.dims.h*ivcout.dims.w)//all same size anyways...just use ivcout as reference
	{
	  
	  for(x=0; x<x_scale/2; x++)
	    {
	      for(a=0; a<3; a++)
		{
		  dataptr[startpos_l + 3*((y - startpos_s) * (x_scale/2) + x)+a] = ivcout.pixels[y];
		}
	    } 
	  if(y==max_loc)
	    {
	      max_loc_l[0] = startpos_l + 3*((y-startpos_s)* (x_scale/2));
	    }
	  if(y==WTAnet_winner) //for drawing longterm attn box
	    {
	      max_loc_l[4] = startpos_l + 3*((y-startpos_s)*(x_scale/2));
	    }
	  
	  for(x=0; x<x_scale/2; x++)
	    {
	      //add c_width/2 to get it halfway across...
	      for(a=0; a<3; a++)
		{
		  if(envp.chan_m_weight > 0)
		    {
		      dataptr[startpos_l + 3*(ipl_input->width/2 + (y - startpos_s) * (x_scale/2) + x)+a] = motion.pixels[y];
		    }
		  else
		    {
		      dataptr[startpos_l + 3*(ipl_input->width/2 + (y - startpos_s) * (x_scale/2) + x)+a] = 255;
		    }
		}
	      if(y==max_loc)
		max_loc_l[1] = startpos_l+3*(ipl_input->width/2 + (y-startpos_s)* (x_scale/2) +x);
	    }
	  //printf("1x\n");
	  y++;
	  if(y%ivcout.dims.w==0)
	    {  
	      //copy the previous like down y_scale times...
	      //add c_width*x_scale to startpos_l
	      startpos_s = y;
	      for(z=1; z<x_scale/2; z++)
		{
		  for(x=c_width-1; x>=0; x--)
		    {
		      dataptr[startpos_l+z*c_width  + x] = dataptr[startpos_l+x];//copy the whole line above down...
		    }
		}
	      startpos_l+=(x_scale/2)*c_width;
	      
	    }
	}
      //printf("DONE!\n\n");
         
      //REV: now do next 2 output quartiles...color and intensity (intens)//orientation ("ori")
      y=0;
      startpos_s = 0;
      while(y<ivcout.dims.h*ivcout.dims.w)
	{
	  for(x=0; x<x_scale/2; x++)
	    {
	      for(a=0; a<3; a++)
		{
		if(envp.chan_c_weight > 0)
		  {
		    dataptr[startpos_l + 3*((y - startpos_s) * x_scale/2 + x) +a] = color.pixels[y];
		  }
		else
		  {
		    dataptr[startpos_l + 3*((y - startpos_s) * x_scale/2 + x) +a] = 100;
		  }
		}
	      if(y==max_loc)
		max_loc_l[2] = startpos_l + 3*((y - startpos_s) * x_scale/2 + x);
	    }
	  for(x=0; x<x_scale/2; x++)
	    {
	      
	      //add c_width/2 to get it halfway across...
	       
	      	      dataptr[startpos_l + c_width/2 + 3*((y - startpos_s) * x_scale/2 + x)+0] = current_sal_color[2]; //value
	     
	      for(a=0; a<3; a++)
		{
		  if(envp.chan_f_weight > 0)
		    {
		      dataptr[startpos_l + c_width/2 + 3*((y - startpos_s) * x_scale/2 + x) + a] = flicker.pixels[y];
		    }
		  else
		    {
		      dataptr[startpos_l + c_width/2 + 3*((y - startpos_s) * x_scale/2 + x) + a] = 0;
		    }
		}
	    }	
	  
	  y++;
	  if(y%ivcout.dims.w==0)
	    {  
	      //copy the previous like down x_scale times...
	      //add c_width*x_scale to startpos_l
	      startpos_s = y;
	      for(z=1; z<x_scale/2; z++)
		{
		  for(x=c_width-1; x>=0; x--)
		    {
		      dataptr[startpos_l+z*c_width  + x] = dataptr[startpos_l+x];//copy the whole line above down...
		    }
		}
	      startpos_l+=x_scale/2*c_width; //2* bc we're making 4 boxes...2x2. So we need to get to right column, =*2
	      
	    }
	}
      
      //draw the yellow squares...
      //from -x*3 to +x*3 (add 3 every time...)
      //   for y=+current+c_width*pixels and -current+c_width*pixels
      
      if(WTAnet_winner > -1) //only if we've chosen a winner already...
	{
	  width_scale = 2; //bc 2 images per width of output image.
	  int max_l_x; int max_l_y; int lt_x; int lt_y;
	  for(z=0; z<1; z++)//for each of the boxes...
	    {
	      //max_l_x = x_loc(max_loc_l[z], c_width);
	      //max_l_y = y_loc(max_loc_l[z], c_width);
	      lt_x = x_loc(max_loc_l[4], c_width);
	      lt_y = y_loc(max_loc_l[4], c_width);
	      max_l_x = lt_x;
	      max_l_y = lt_y; //just make it follow the small one..
	      //draw horizontal "Y" lines...
	      for(x= (-x_scale * (MASS_radius)) / 2;   x < x_scale/2 + (x_scale * (MASS_radius))/2; x++)
		{
		  if( x*3+max_l_x >= 0 && x*3+max_l_x<c_width/2)
		    {
		      y=(-x_scale * (MASS_radius)) / 2 +1;
		      if(y+max_l_y>=0 && y+max_l_y<(ipl_input->height)/2)
			{
			  int loc = x_y_to_array_loc(max_l_x+(x*3),max_l_y+y,c_width);
			  dataptr[loc +0] = 0;
			  dataptr[loc +1] = 255;
			  dataptr[loc +2] = 255;
			}
		      y=x_scale/2 +(x_scale*(MASS_radius))/2;
		      
		      if(y+max_l_y>=0 && y+max_l_y<(ipl_input->height)/2)
			{
			  int loc = x_y_to_array_loc(max_l_x+(x*3),max_l_y+y,c_width);
			  dataptr[loc +0] = 0;
			  dataptr[loc +1] = 255;
			  dataptr[loc +2] = 255;
			}
		    } 
		}
	      
	      //for longterm..
	      for(x=0; x<16/2; x++)
		{
		  y=0;
		  int loc = x_y_to_array_loc(lt_x+(x*3),lt_y+y,c_width);
		  dataptr[loc +0] = 0; //these didnt have +y (wtf?)
		  dataptr[loc +1] = 255;
		  dataptr[loc +2] = 0;
		  
		  y=16/2 -1; //-1 because, as above, see its x<16/2 (not <=) we want all INSIDE the square
		  loc = x_y_to_array_loc(lt_x+(x*3), lt_y+y, c_width);
		  dataptr[loc +0] = 0; //these didnt have +y
		  dataptr[loc +1] = 255;
		  dataptr[loc +2] = 0;
		}
	      
	      for(y=0; y<16/2; y++)
		{
		  x=0;
		  int loc = x_y_to_array_loc(lt_x+(x*3), lt_y+y, c_width);
		  dataptr[loc +0] = 0;
		  dataptr[loc +1] = 255;
		  dataptr[loc +2] = 0;
		  
		  x=16/2 -1; //-1 because, as above, see its y<16/2, we want it all INSIDE the square.
		  loc = x_y_to_array_loc(lt_x+(x*3), lt_y+y, c_width);
		  dataptr[loc +0] = 0;
		  dataptr[loc +1] = 255;
		  dataptr[loc +2] = 0;
		}
	      
	      
	      //draw vertical "X" lines...
	      for(y=(-x_scale * (MASS_radius)) /2 +1; y< (x_scale/2) + (x_scale * (MASS_radius))/2; y++)
		{ 
		  if(y+max_l_y>=0 && y+max_l_y<(ipl_input->height)/2)
		    {
		      x=(-x_scale * (MASS_radius)) /2;
		      
		      if( x*3+max_l_x >= 0 && x*3+max_l_x<c_width/2 )
			{
			  int loc = x_y_to_array_loc(max_l_x+(x*3), max_l_y+y, c_width);
			  dataptr[loc +0] = 0;
			  dataptr[loc +1] = 255;
			  dataptr[loc +2] = 255;
			} 
		      x=x_scale/width_scale + ((MASS_radius) *x_scale)/2-1;
		      
		      if( x*3+max_l_x>= 0 && x*3+max_l_x<c_width/2 )
			{
			  int loc = x_y_to_array_loc(max_l_x+(x*3), max_l_y+y, c_width);
			  dataptr[loc +0] = 0;
			  dataptr[loc +1] = 255;
			  dataptr[loc +2] = 255;
			}
		    }	  
		}
	    }
	  //end for the z
	} //end only if we've chosen a WTAnet winner already 
      env_img_make_empty(&ivcout);
      env_img_make_empty(&intens);
      env_img_make_empty(&color);
      env_img_make_empty(&ori);
#ifdef ENV_WITH_DYNAMIC_CHANNELS
      env_img_make_empty(&flicker);
      env_img_make_empty(&motion);
#endif
    }
  FIRSTFRAMEL=0; //REV: weve been through things once...
  
  
 
  //HMM, can we assume blurriness for infants? Not fully developed... Maybe that's why later on
  //parental (social) cues are better?
  
  
  return xywinners; //REV: for imclever return xywinners
  
}//end nextpic_frommemL


//LEFT EYE

//REV: nextpic_frommem: calculates saliency on given input, and draws appropriate pictures on output, 
//given input (assumedly from some other program implementing winner-take-all saliency, etc.)
//we're using global data from SalMapData to share required data/map output with java etc.
//called as quickly as possible in a separate thread (best way to do it?)
int* envision_nextpic_frommemR(const IplImage* ipl_input,  //ipl image as input
			      IplImage* output, int desired_numsalwinners)   //output image we'll write to
{
  
  int* xywinners = malloc(2*sizeof(int)*desired_numsalwinners); //*2 because we are giving 2 points for each.
  
  struct env_dims indims;
  indims.w = ipl_input->width;
  indims.h = ipl_input->height;
  
  //printf("IMAGE SIZE: %i %i \n", ipl_input->width, ipl_input->height);
  //input is ipl_input as const env_rgb_pixel*
  const struct env_rgb_pixel* input = (struct env_rgb_pixel*)ipl_input->imageData;
  
  npixels = indims.w * indims.h;
  sal_winner_coords[0]=-1; sal_winner_coords[1]=-1;
  float width_scale = 320 / indims.w; //this is written for 320...multiply everything hardcoded by scale, right?
  
  //initialize if necessary...note size is pretty manual
  if( !(INITIALIZEDR == 1) ) { envision_initR(indims.w/16, indims.h/16); INITIALIZEDR=1; } 
  
  struct env_image ivcout = env_img_initializer;
  struct env_image intens = env_img_initializer;
  struct env_image color = env_img_initializer;
  struct env_image ori = env_img_initializer;
#ifdef ENV_WITH_DYNAMIC_CHANNELS
  struct env_image flicker = env_img_initializer;
  struct env_image motion = env_img_initializer;
#endif

  struct status_data userdata; //REV: need this?
  //used to have userdata.frame_number = img_num
  
  
  //REV: note this function will call env_visual_cortex_input (the non-multithreaded version) since it's not multithreaded now.
  env_mt_visual_cortex_input(multithreaded,
			     &ivcR, &envp,
			     "visualcortex",
			     input, 0, indims,
			     0, /*&print_chan_status, //REV: want this?*/ //we don't need status_func...if we don't
			     //want to be printing shit.
			     &userdata,
			     &ivcout,
			     &intens, &color,
			     &ori
#ifdef ENV_WITH_DYNAMIC_CHANNELS
			     , &flicker, &motion
#endif
			     );
  
  
  if(FIRSTFRAMER == 0) //because movement isn't calculated until 2nd frame...only use ivcout for now or it will segfault
    {
      //needs to be inside bc there is nothing to scale first frame (we just do it to build the lpf for motion next time)
      env_visual_cortex_rescale_ranges(
				       &ivcout, &intens, &color, &ori
#ifdef ENV_WITH_DYNAMIC_CHANNELS
				       , &flicker, &motion
#endif
				       );
      
      
      
      //copy ivcout to ipl output image to draw...(I need to interpolate to the image size...)
      char* dataptr = output->imageData;
      int max_loc = -1;
      int max_val = -1;
      int y=0, x=0, z=0, a=0;
      
      
      //calc max_loc here:
      for(x=0; x<salmap_w; x++)
	{
	  for(y=0; y<salmap_h; y++)
	    {
	      int loc = x_y_to_array_loc(x, y, salmap_w);
	      int val = ivcout.pixels[loc];
	      if(val > max_val)
		{
		  max_val = val;
		  max_loc = loc;
		}
	    }
	}
      
      //set external x,y winners so avg color can be calced and passed to java side
      //REV: this is in *SCALED UP* image...(for drawing only..WTAnet is for WTAnet calc too so not scaled)
      sal_winner_coords[0] = x_loc(max_loc, ivcout.dims.w) * 16; //why not *16*16?
      sal_winner_coords[1] = y_loc(max_loc, ivcout.dims.w) * 16; //err should make height scale too
      
      xywinners[0] = sal_winner_coords[0];
      xywinners[1] = sal_winner_coords[1];
      WTA_winner_coords[0] = xywinners[0];
      WTA_winner_coords[1] = xywinners[1];
      
      WTAnet_winner = max_loc; //hope this works?
      
            
      int win_x = WTA_winner_coords[0];//sal_winner_coords[0]; //REV: set to WTAnet when done coding, to use that value... 
      int win_y = WTA_winner_coords[1];//sal_winner_coords[1];
				   
      int c_width = ipl_input->width*3;
      //let's write the picture...1st is FULL SALIENCY, then we have (left-right, up-down, as english)
      //color, motion, (orientation, flicker, intensity)
      int x_scale = ipl_input->width/ivcout.dims.w; //160 / 10 = 16
      //int y_scale = ipl_input->height/ivcout.dims.h; //120 / ?7? = 16
      //printf("DIMSW: %i   DIMSH: %i   X_SCALE: %i   Y_SCALE: %i\n", ivcout.dims.w, ivcout.dims.h, x_scale, y_scale);
      
      
      int startpos_s=0; int startpos_l=0;
      
      int max_loc_l[5];
      for(int x=0; x<5; x++)
	{
	  max_loc_l[x] = -1;
	}
      
      y=0; 
      a=0; 
      z=0; 
      x=0;
      
      
      //REV: write out first two output quarters (top-left, top-right)
      while(y<ivcout.dims.h*ivcout.dims.w)//all same size anyways...just use ivcout as reference
	{
	  
	  for(x=0; x<x_scale/2; x++)
	    {
	      for(a=0; a<3; a++)
		{
		  dataptr[startpos_l + 3*((y - startpos_s) * (x_scale/2) + x)+a] = ivcout.pixels[y];
		}
	    } 
	  if(y==max_loc)
	    {
	      max_loc_l[0] = startpos_l + 3*((y-startpos_s)* (x_scale/2));
	    }
	  if(y==WTAnet_winner) //for drawing longterm attn box
	    {
	      max_loc_l[4] = startpos_l + 3*((y-startpos_s)*(x_scale/2));
	    }
	  
	  for(x=0; x<x_scale/2; x++)
	    {
	      //add c_width/2 to get it halfway across...
	      for(a=0; a<3; a++)
		{
		  if(envp.chan_m_weight > 0)
		    {
		      dataptr[startpos_l + 3*(ipl_input->width/2 + (y - startpos_s) * (x_scale/2) + x)+a] = motion.pixels[y];
		    }
		  else
		    {
		      dataptr[startpos_l + 3*(ipl_input->width/2 + (y - startpos_s) * (x_scale/2) + x)+a] = 255;
		    }
		}
	      if(y==max_loc)
		max_loc_l[1] = startpos_l+3*(ipl_input->width/2 + (y-startpos_s)* (x_scale/2) +x);
	    }
	  //printf("1x\n");
	  y++;
	  if(y%ivcout.dims.w==0)
	    {  
	      //copy the previous like down y_scale times...
	      //add c_width*x_scale to startpos_l
	      startpos_s = y;
	      for(z=1; z<x_scale/2; z++)
		{
		  for(x=c_width-1; x>=0; x--)
		    {
		      dataptr[startpos_l+z*c_width  + x] = dataptr[startpos_l+x];//copy the whole line above down...
		    }
		}
	      startpos_l+=(x_scale/2)*c_width;
	      
	    }
	}
      //printf("DONE!\n\n");
         
      //REV: now do next 2 output quartiles...color and intensity (intens)//orientation ("ori")
      y=0;
      startpos_s = 0;
      while(y<ivcout.dims.h*ivcout.dims.w)
	{
	  for(x=0; x<x_scale/2; x++)
	    {
	      for(a=0; a<3; a++)
		{
		if(envp.chan_c_weight > 0)
		  {
		    dataptr[startpos_l + 3*((y - startpos_s) * x_scale/2 + x) +a] = color.pixels[y];
		  }
		else
		  {
		    dataptr[startpos_l + 3*((y - startpos_s) * x_scale/2 + x) +a] = 100;
		  }
		}
	      if(y==max_loc)
		max_loc_l[2] = startpos_l + 3*((y - startpos_s) * x_scale/2 + x);
	    }
	  for(x=0; x<x_scale/2; x++)
	    {
	      
	      //add c_width/2 to get it halfway across...
	       
	      	      dataptr[startpos_l + c_width/2 + 3*((y - startpos_s) * x_scale/2 + x)+0] = current_sal_color[2]; //value
	     
	      for(a=0; a<3; a++)
		{
		  if(envp.chan_f_weight > 0)
		    {
		      dataptr[startpos_l + c_width/2 + 3*((y - startpos_s) * x_scale/2 + x) + a] = flicker.pixels[y];
		    }
		  else
		    {
		      dataptr[startpos_l + c_width/2 + 3*((y - startpos_s) * x_scale/2 + x) + a] = 0;
		    }
		}
	    }	
	  
	  y++;
	  if(y%ivcout.dims.w==0)
	    {  
	      //copy the previous like down x_scale times...
	      //add c_width*x_scale to startpos_l
	      startpos_s = y;
	      for(z=1; z<x_scale/2; z++)
		{
		  for(x=c_width-1; x>=0; x--)
		    {
		      dataptr[startpos_l+z*c_width  + x] = dataptr[startpos_l+x];//copy the whole line above down...
		    }
		}
	      startpos_l+=x_scale/2*c_width; //2* bc we're making 4 boxes...2x2. So we need to get to right column, =*2
	      
	    }
	}
      
      //draw the yellow squares...
      //from -x*3 to +x*3 (add 3 every time...)
      //   for y=+current+c_width*pixels and -current+c_width*pixels
      
      if(WTAnet_winner > -1) //only if we've chosen a winner already...
	{
	  width_scale = 2; //bc 2 images per width of output image.
	  int max_l_x; int max_l_y; int lt_x; int lt_y;
	  for(z=0; z<1; z++)//for each of the boxes...
	    {
	      //max_l_x = x_loc(max_loc_l[z], c_width);
	      //max_l_y = y_loc(max_loc_l[z], c_width);
	      lt_x = x_loc(max_loc_l[4], c_width);
	      lt_y = y_loc(max_loc_l[4], c_width);
	      max_l_x = lt_x;
	      max_l_y = lt_y; //just make it follow the small one..
	      //draw horizontal "Y" lines...
	      for(x= (-x_scale * (MASS_radius)) / 2;   x < x_scale/2 + (x_scale * (MASS_radius))/2; x++)
		{
		  if( x*3+max_l_x >= 0 && x*3+max_l_x<c_width/2)
		    {
		      y=(-x_scale * (MASS_radius)) / 2 +1;
		      if(y+max_l_y>=0 && y+max_l_y<(ipl_input->height)/2)
			{
			  int loc = x_y_to_array_loc(max_l_x+(x*3),max_l_y+y,c_width);
			  dataptr[loc +0] = 0;
			  dataptr[loc +1] = 255;
			  dataptr[loc +2] = 255;
			}
		      y=x_scale/2 +(x_scale*(MASS_radius))/2;
		      
		      if(y+max_l_y>=0 && y+max_l_y<(ipl_input->height)/2)
			{
			  int loc = x_y_to_array_loc(max_l_x+(x*3),max_l_y+y,c_width);
			  dataptr[loc +0] = 0;
			  dataptr[loc +1] = 255;
			  dataptr[loc +2] = 255;
			}
		    } 
		}
	      
	      //for longterm..
	      for(x=0; x<16/2; x++)
		{
		  y=0;
		  int loc = x_y_to_array_loc(lt_x+(x*3),lt_y+y,c_width);
		  dataptr[loc +0] = 0; //these didnt have +y (wtf?)
		  dataptr[loc +1] = 255;
		  dataptr[loc +2] = 0;
		  
		  y=16/2 -1; //-1 because, as above, see its x<16/2 (not <=) we want all INSIDE the square
		  loc = x_y_to_array_loc(lt_x+(x*3), lt_y+y, c_width);
		  dataptr[loc +0] = 0; //these didnt have +y
		  dataptr[loc +1] = 255;
		  dataptr[loc +2] = 0;
		}
	      
	      for(y=0; y<16/2; y++)
		{
		  x=0;
		  int loc = x_y_to_array_loc(lt_x+(x*3), lt_y+y, c_width);
		  dataptr[loc +0] = 0;
		  dataptr[loc +1] = 255;
		  dataptr[loc +2] = 0;
		  
		  x=16/2 -1; //-1 because, as above, see its y<16/2, we want it all INSIDE the square.
		  loc = x_y_to_array_loc(lt_x+(x*3), lt_y+y, c_width);
		  dataptr[loc +0] = 0;
		  dataptr[loc +1] = 255;
		  dataptr[loc +2] = 0;
		}
	      
	      
	      //draw vertical "X" lines...
	      for(y=(-x_scale * (MASS_radius)) /2 +1; y< (x_scale/2) + (x_scale * (MASS_radius))/2; y++)
		{ 
		  if(y+max_l_y>=0 && y+max_l_y<(ipl_input->height)/2)
		    {
		      x=(-x_scale * (MASS_radius)) /2;
		      
		      if( x*3+max_l_x >= 0 && x*3+max_l_x<c_width/2 )
			{
			  int loc = x_y_to_array_loc(max_l_x+(x*3), max_l_y+y, c_width);
			  dataptr[loc +0] = 0;
			  dataptr[loc +1] = 255;
			  dataptr[loc +2] = 255;
			} 
		      x=x_scale/width_scale + ((MASS_radius) *x_scale)/2-1;
		      
		      if( x*3+max_l_x>= 0 && x*3+max_l_x<c_width/2 )
			{
			  int loc = x_y_to_array_loc(max_l_x+(x*3), max_l_y+y, c_width);
			  dataptr[loc +0] = 0;
			  dataptr[loc +1] = 255;
			  dataptr[loc +2] = 255;
			}
		    }	  
		}
	    }
	  //end for the z
	} //end only if we've chosen a WTAnet winner already 
      env_img_make_empty(&ivcout);
      env_img_make_empty(&intens);
      env_img_make_empty(&color);
      env_img_make_empty(&ori);
#ifdef ENV_WITH_DYNAMIC_CHANNELS
      env_img_make_empty(&flicker);
      env_img_make_empty(&motion);
#endif
    }
  FIRSTFRAMER=0; //REV: weve been through things once...
  
  
 
  //HMM, can we assume blurriness for infants? Not fully developed... Maybe that's why later on
  //parental (social) cues are better?
  
  
  return xywinners; //REV: for imclever return xywinners
  
}//end nextpic_frommemR

int envision_cleanup(void)
{
  struct env_alloc_stats stats;
  env_allocation_get_stats(&stats);
  env_stdio_print_alloc_stats(&stats, npixels ? npixels : 1);
  env_visual_cortex_destroy(&ivcL);
  env_visual_cortex_destroy(&ivcR);
  
  //REV: } close the unqualified { } section here... (important? for threading?)
  env_allocation_cleanup();
  
  return 0;
}

void init_salmap(int w, int h)
{
  printf("%i %i\n", w, h);
  printf("INITIALIZING LONG-TERM SALIENCY MAP\n");
  //saliencyDataMutex = PTHREAD_MUTEX_INITIALIZER; //init mutex...
  salmap_w = w;
  salmap_h = h;
  salmap = malloc(sizeof(unsigned int) * w * h);
  //salmap = new unsigned char[w*h];
  unsigned int num_bytes = sizeof(struct WTA_LIF_Neuron) * w * h;
  printf("ALLOCATING: %i\n", num_bytes);
  WTAnet = malloc(num_bytes);
  //WTAnet = new struct WTA_LIF_Neuron[w*h];
  for(int x=0; x<w*h; x++)  { salmap[x] = (unsigned char)0; WTAnet[x].Vm = 0; WTAnet[x].Isyn = 0;  
    WTAnet[x].inhib_timer= -1; WTAnet[x].mass_points = 0;}
  int xv, yv;
  int full_square = (MASS_pixels+1) * (MASS_pixels+1);
  for(int y=0; y<salmap_h; y++)
    {
      for(int x=0; x<salmap_w; x++)
	{
	  for(int dx=-MASS_radius; dx<=MASS_radius; dx++)
	    {
	      for(int dy=-MASS_radius; dy<=MASS_radius; dy++)
		{
		  xv = x+dx;
		  yv = y+dy;
		  if(xv >= 0 && xv<salmap_w && yv >=0 && yv<salmap_h)
		    ++WTAnet[x_y_to_array_loc(x, y, salmap_w)].mass_points;
		}
	    }
	  
	  int diff = full_square - WTAnet[x_y_to_array_loc(x,y,salmap_w)].mass_points;
	  WTAnet[x_y_to_array_loc(x,y,salmap_w)].mass_points *= (255+diff); //*255 bc each pixel has max val of 255
	  printf("%2i ", WTAnet[x_y_to_array_loc(x, y, salmap_w)].mass_points);
	}
      printf("\n");
    }
  printf("\n");

  
  //make random option:
  
  long rand_seed = time(NULL); //errr...is this always same? :D
  printf("TIME %i\n", rand_seed);
  //gsl
  // gsl_rand_source = gsl_rng_alloc(gsl_rng_taus);
  //gsl_rng_set(gsl_rand_source, rand_seed);
  
  /*for(int x=0; x<100; x++)
    {
      int asdf = gsl_rng_uniform_int(gsl_rand_source, 100);
      printf("random number (1,100)!: %i\n", asdf);
    }
  */

}

void delete_salmap(void) //call this in feature extractor destructor?
{
  if(salmap != NULL)
    {
      //delete [] salmap;
      //delete [] WTAnet;
      
      free(salmap); 
      free(WTAnet);
      /*int x=0;
      for(x=0; x<salmap_w * salmap_h; x++)
	{
      free(salmap[x]); 
      free(WTAnet[x]);
	}
      */
      
    }
}


/*
//REV: this automatically takes input images (in color...) and outputs greyscale images.
//REV: we might want to change this to deal totally in memory.
//REV: the greyscale images we can handle just fine (we know dimensions and max value...)
//REV: on the other hand color images may not be so simple (if we need them?)
//REV: also, if a lot of the functions are written to use files...we'll be in trouble...
int envision_nextpic(int img_num, char* instem, char* outstem)
{
  struct env_dims indims;
  struct env_rgb_pixel* input = 0;
  
  char fname[256];
  snprintf(fname, sizeof(fname),
	   "%s%06d.pnm", instem, img_num);
  
  //we need to put filename into fname...
  
  input = env_stdio_parse_rgb(fname, &indims); //parses the image here...writes it to env_rgb_pixel input.
  //this is actually the image in memory! good. So, we should be able to take a memory image as argument.
  
  npixels = indims.w * indims.h; //REV: number of pixels...times 3 should be actual byte size of img.


  //REV: PNM image is just abstraction of ppm, pgm, pbm, each of which
  //can be either ascii or binary. 
  //P1, P2, P3 are ascii (b/w, grey, color) respectively
  //P4, P5, P6 are binary (b/w, grey, color) respectively
  //so, it's like a union of those...
  //b/w I assume is bits (1 or 0), grey is bytes/integers (for value)
  //color is R-G-B triples (either bytes/integers depending on range).
  
  struct env_image ivcout = env_img_initializer;
  struct env_image intens = env_img_initializer;
  struct env_image color = env_img_initializer;
  struct env_image ori = env_img_initializer;
#ifdef ENV_WITH_DYNAMIC_CHANNELS
  struct env_image flicker = env_img_initializer;
  struct env_image motion = env_img_initializer;
#endif

  struct status_data userdata;
  userdata.frame_number = img_num; //used to be = c (where c iterates through all input frames first to last)
  //REV: but, we're just always going to give it the same picture...(unless it needs current + previous...)
  //REV: it doesnt seem to...vcx holds required previous img lowpass.

  //REV: why is it always sending 0 for prev_colimg? Well, we dont need for mtn or fliker so OK i guess..
  //REV: added bias (as fraction...[0-1], where 1 is normal...)
  
  envp.red_bias=0;
  envp.yellow_bias=0;
  envp.green_bias = 0;
  envp.blue_bias = 1.0;
  //I can bias here by changing envp.
   env_mt_visual_cortex_input(multithreaded,
			     &ivc, &envp,
			     "visualcortex",
			     input, 0, indims,
			     &print_chan_status,
			     &userdata,
			     &ivcout,
			     &intens, &color,
			     &ori
#ifdef ENV_WITH_DYNAMIC_CHANNELS
			     , &flicker, &motion
#endif
			     );
  
  //env_deallocate(input);
  input = 0;
  
  env_visual_cortex_rescale_ranges(
				   &ivcout, &intens, &color, &ori
#ifdef ENV_WITH_DYNAMIC_CHANNELS
				   , &flicker, &motion
#endif
				   );

  //REV: output the max point of the color channel...
  int* src = env_img_pixels(&color);
  int max = -1;
  int max_loc = -1;

  
  for(int x=0; x<env_img_size(&color); x++)
    {
      //REV: to print out the image in ascii...
      //printf("%i ", src[x]);
      //if(x%salmap_w == salmap_w-1)
      //  printf("\n");
      
      if(src[x] >= max)
	{
	  max = src[x];
	  max_loc = x;
	}
    }
  int y_loc = max_loc/salmap_w;
  int x_loc = max_loc%salmap_w;
  printf("Image dims: (%i, %i)   Max location: %i    (%i,%i)\n", salmap_w, salmap_h, max_loc, x_loc, y_loc);
  
  env_stdio_write_gray(&ivcout, outstem, "vcx", img_num);
  
  env_stdio_write_gray(&intens, outstem, "intens", img_num);
  env_stdio_write_gray(&color, outstem, "color", img_num);
  env_stdio_write_gray(&ori, outstem, "ori", img_num);
#ifdef ENV_WITH_DYNAMIC_CHANNELS
  env_stdio_write_gray(&flicker, outstem, "flicker", img_num);
  env_stdio_write_gray(&motion, outstem, "motion", img_num);
#endif
  
  env_img_make_empty(&ivcout);
  env_img_make_empty(&intens);
  env_img_make_empty(&color);
  env_img_make_empty(&ori);
#ifdef ENV_WITH_DYNAMIC_CHANNELS
  env_img_make_empty(&flicker);
  env_img_make_empty(&motion);
#endif
  
  return 0;
}


//REV: added color bias...
int envision_nextpic_bias(int img_num, char* instem, char* outstem, float red_bias, float green_bias, float blue_bias, float yellow_bias)
{
  struct env_dims indims;
  struct env_rgb_pixel* input = 0;
  
  char fname[256];
  snprintf(fname, sizeof(fname),
	   "%s%06d.pnm", instem, img_num);
  
  //we need to put filename into fname...
  
  input = env_stdio_parse_rgb(fname, &indims);
  
  npixels = indims.w * indims.h;

  struct env_image ivcout = env_img_initializer;
  struct env_image intens = env_img_initializer;
  struct env_image color = env_img_initializer;
  struct env_image ori = env_img_initializer;
#ifdef ENV_WITH_DYNAMIC_CHANNELS
  struct env_image flicker = env_img_initializer;
  struct env_image motion = env_img_initializer;
#endif

  struct status_data userdata;
  userdata.frame_number = img_num; //used to be = c (where c iterates through all input frames first to last)
  //REV: but, we're just always going to give it the same picture...(unless it needs current + previous...)
  //REV: it doesnt seem to...vcx holds required previous img lowpass.

  //REV: why is it always sending 0 for prev_colimg? Well, we dont need for mtn or fliker so OK i guess..
  //REV: added bias (as fraction...[0-1], where 1 is normal...)
  //float red_bias=0, green_bias=1, blue_bias=1, yellow_bias=1;
  env_mt_visual_cortex_input_bias(multithreaded,
			     &ivc, &envp,
			     "visualcortex",
			     input, 0, indims,
			     &print_chan_status,
			     &userdata,
			     &ivcout,
			     &intens, &color, red_bias, green_bias, blue_bias, yellow_bias,
			     &ori
#ifdef ENV_WITH_DYNAMIC_CHANNELS
			     , &flicker, &motion
#endif
			     );
  
  //env_deallocate(input);
  input = 0;
  
  env_visual_cortex_rescale_ranges(
				   &ivcout, &intens, &color, &ori
#ifdef ENV_WITH_DYNAMIC_CHANNELS
				   , &flicker, &motion
#endif
				   );
  
  env_stdio_write_gray(&ivcout, outstem, "vcx", img_num);
  
  env_stdio_write_gray(&intens, outstem, "intens", img_num);
  env_stdio_write_gray(&color, outstem, "color", img_num);
  env_stdio_write_gray(&ori, outstem, "ori", img_num);
#ifdef ENV_WITH_DYNAMIC_CHANNELS
  env_stdio_write_gray(&flicker, outstem, "flicker", img_num);
  env_stdio_write_gray(&motion, outstem, "motion", img_num);
#endif
  
  env_img_make_empty(&ivcout);
  env_img_make_empty(&intens);
  env_img_make_empty(&color);
  env_img_make_empty(&ori);
#ifdef ENV_WITH_DYNAMIC_CHANNELS
  env_img_make_empty(&flicker);
  env_img_make_empty(&motion);
#endif
  
  return 0;
}
*/










//for testing purposes...delete/comment out later...
/*int main(int argc, const char** argv)
{
   if (argc != 5 && argc != 6)
        {
                fprintf(stderr,
                        "usage: %s instem outstem firstframe lastframe ?multi-threaded?\n",
                        argv[0]);
                return 1;
        }
 const char* instem = argv[1];
 const char* outstem = argv[2];
 const int first = atoi(argv[3]);
 const int last = atoi(argv[4]);
 //const int multithreaded = argc < 6 ? 0 : atoi(argv[5]);

 envision_init();
 for(int x=first; x<last; x++)
   {
     envision_nextpic(x, instem, outstem);
   }
 envision_cleanup();
 //cout << "DONE!" << endl;
 return 0;
 
}
*/


     /*
      //===============================================================//
      //do recursive WTA suppression using a 2d separable mexican-hat filter... [...-1, 1, 1, 1, -1...]
      //note: we're not attenuating the edges...
      for(x=0; x<ivcout.dims.w; x++)
	{
	  for(y=0; y<ivcout.dims.h; y++)
	    {
	      tmpa[x][y]= ivcout.pixels[y*ivcout.dims.w + x];
	    }
	}
      for(x=0; x<ivcout.dims.w; x++)
	{
	  for(y=0; y<ivcout.dims.h; y++)
	    {
	      //x,y is center pixel...subtract its value from tmp[all others except surround]
	      for(z=0; z<ivcout.dims.w; z++)
		{
		  for(a=0; a<ivcout.dims.h; a++)
		    {
		      if(!(z == x-1 || z==x || z == x+1) && !(a==y-1 || a==y || a==y+1))
			{
			  tmpa[z][a] -= ivcout.pixels[y*ivcout.dims.w+x];
			}
		    }
		}
	    }
	}
      int tmpmax=10000000;
      int tmpmin= -10000000;
      for(x=0; x<ivcout.dims.w; x++)
	{
	  for(y=0; y<ivcout.dims.h; y++)
	    {
	      if( tmpmax > tmpa[x][y] )
		tmpmax = tmpa[x][y];
	      if( tmpmin < tmpa[x][y] )
		tmpmin = tmpa[x][y];
	    }
	}
      tmpmin -= tmpmax;
      
      
      for(x=0; x<ivcout.dims.w; x++)
	{
	  for(y=0; y<ivcout.dims.h; y++)
	    {
	      tmpa[x][y] = ((tmpa[x][y]-tmpmax)/(tmpmin)) * 254;
	      if(tmpa[x][y]>0)
		max_loc = y*ivcout.dims.w + x;
	    }
	}
	// END filtered WTA
      */
