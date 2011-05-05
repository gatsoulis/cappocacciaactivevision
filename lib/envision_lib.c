//REV: Jan 2010, saliency map implementation/functions for running it in real time.
//REV: May 2011: modifications for running on binocular (L eye, R eye), and stuff for im-clever 2011



//########## DEFINES ###########//

//instantaneous map Gaussian LPF SD (in pixels, 1 pixel == 1 unit distance). using salience "mass"
#define GAUSS_LPF_SD 1.0 //0.00001 for original one (i.e. no gaussian combination)
//instantaneous map Gaussian LPF cutoff value (in weight)
#define GAUSS_LPF_CUTOFF 0.05
//define to turn on/off edge correction for gaussian (basically normalize by integral of weights)
#define EDGECORR

#define SC_thresh 0.3 //arbitrary threshold
#define SC_W_mult 10.0


#define L_EYE 0
#define R_EYE 1
//##############################//




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

#include <stdio.h>
#include <stdlib.h> // for atoi(), malloc(), free()
#include <math.h>
#include "envision_lib.h"
#include "opencv/cxcore.h"//this has the IplImage definition in it...
#include <sys/time.h>
#include <gsl/gsl_randist.h> //for gamma random
#include <gsl/gsl_rng.h> //gsl random number generator



float gaussian_val(float, float, float, float);
int x_loc(int, int);
int y_loc(int, int);
int x_y_to_array_loc(int, int, int);
static void* malloc_thunk(env_size_t);

gsl_rng* gsl_rand_source; //for uniform random

//========= SALMAP GAUSSIAN FILTER VARIABLES ============//
float** gauss_filter;
int gauss_filter_radius;
int gauss_filter_w;
int gauss_filter_c;

float** lpf_salmap;
int salmap_w; //width and height of output array (instantaneous salmap)
int salmap_h;

//========== SUPERIOR COLLICULUS VARIABLES ===========//
float** SC_l; //left superior colliculus (long term salmap). These should be neurons, this is just float Vm
float** SC_r; //right superior colliculus (long term salmap)
float SC_memb_t_const=-1; //time constant of decay for membrane potential of superior colliculus neurons
float SC_memb_decay=-1; //analytically solved decay constant for remainder&leakage term of differential equation solution
float SC_memb_decay_compl=-1; //analytically solved decay constant for addition term of differential equation solution

float SC_l_maxval=-1;
float SC_r_maxval=-1;
int SC_l_maxx=-1;
int SC_l_maxy=-1;
int SC_r_maxx=-1;
int SC_r_maxy=-1;

int SC_win_side = -1;//0 for left eye (then look to left max) //1 for right eye (then look to right max)
//or just choose the subset if we get sent a subset...right?


//========== VARIABLES SO WE DONT GET SEGFAULTS FROM LACK OF INITIALIZATION =======//
int INITIALIZED = 0; //initially, nothing is initialized
int FIRSTFRAMER = 1; //initially, it's the first frame for R and L side
int FIRSTFRAMEL = 1;
int divnum=1; //divnum for division from full input image size to output image size





// ######################################################################
struct status_data
{
  int frame_number;
};

//REV: some globals...
struct env_params envp; //REV: hack for left and right eye!
struct env_visual_cortex ivcR; //REV: this is holding each previous img (lowpass5 filtered) for motion (one for each eye)
struct env_visual_cortex ivcL;
env_size_t npixels = 0;

//REV: just for now...
int multithreaded=0;


int envision_init(int w, int h) //just do both the eyes!
{
  printf("INITIALIZING...");
  
  //random stuff init, GSL (gnu scientific library) (for gaussians, etc., for when we have a tie!)
  long rand_seed = time(NULL); //errr...is this always same? :D
  gsl_rand_source = gsl_rng_alloc(gsl_rng_taus);
  gsl_rng_set(gsl_rand_source, rand_seed);
  
   //------ INITIALIZING NORMAL STUFF -----------------------//
  //Instantiate our various ModelComponents:
  env_params_set_defaults(&envp);
  
  envp.maxnorm_type = ENV_VCXNORM_MAXNORM;
  //envp.maxnorm_type = ENV_VCXNORM_NONE;
  
  
  envp.scale_bits = 16; //REV: does this relate to the output level?
  
  env_assert_set_handler(&env_stdio_assert_handler);
  if (multithreaded)
    {
      env_init_pthread_alloc();
      env_init_pthread_job_server();
    }
  env_allocation_init(&malloc_thunk, &free);
    
  //LEFT SIDE
  env_visual_cortex_init(&ivcL, &envp);
  env_visual_cortex_init(&ivcR, &envp);
  printf("FINISHED SALIENCY MAP INITIALIZATION\n\n");
  
  //--------------------- INIT LPF AND SETTING SOME IMPORTNANT W AND H-----------//
  divnum=1; for(int x=0; x<envp.output_map_level; x++) { divnum *= 2; } //just doing 2^output_map_level
  salmap_w = w/divnum;
  salmap_h = h/divnum; //not even...?
  
  lpf_salmap = malloc(sizeof(float*) * salmap_w);
  for(int x=0; x<salmap_w; x++)
    {
      lpf_salmap[x] = malloc(sizeof(float) * salmap_h);
    }

  //----------------------- INITIALIZING GAUSSIAN FILTER ---------------------//
  //init gaussian filter for massing pixels. Note biggest x or y will be at axes.
  gauss_filter_radius = 0;
  
  printf("Calculating 2-dimensional gaussian low-pass-filter\n");
  
  //basically a do-while loop
  float val=gaussian_val((float)gauss_filter_radius, GAUSS_LPF_SD, 0, 1); //sd, mean, scale
  while(val >= GAUSS_LPF_CUTOFF)
    {
      printf("%0.3f ", val);
      ++gauss_filter_radius;
      val= gaussian_val((float)gauss_filter_radius, GAUSS_LPF_SD, 0, 1); //sd, mean, scale
      
    }
  printf(" --  (radius = %i)\n\n", gauss_filter_radius); //note this is the radius i.e. LESS THAN value, we'd do x=0; x<RAD; x++
  gauss_filter_w = gauss_filter_radius*2-1;
  gauss_filter_c = gauss_filter_radius-1;
  
  //SEE: LPF SALMAP INIT IN FIRST PASS THROUGH NEXTFRAME
  
  gauss_filter = malloc(sizeof(float*) * gauss_filter_w);
  for(int x=0; x<gauss_filter_w; x++)
    {
      gauss_filter[x] = malloc(sizeof(float) * gauss_filter_w);
      for(int z=0; z<gauss_filter_w; z++)
	{
	  //dist is x and z, i.e. sqrt (x^2 + z^2)
	  int xdist = x-gauss_filter_c;
	  int zdist = z-gauss_filter_c;
	  float dist = sqrt(xdist*xdist + zdist*zdist);
	  if(dist <= gauss_filter_radius)
	    gauss_filter[x][z] = gaussian_val(dist, GAUSS_LPF_SD, 0, 1);
	  else
	    gauss_filter[x][z] = 0;
	  
	  //gauss_filter[x][z] = sqrt(xdist*xdist + zdist*zdist);
	  printf("%4.3f ", gauss_filter[x][z]);
	}
      printf("\n");
    }
  printf("\n");
  
  printf("FINISHED INITIALIZING GAUSSIAN FILTERS\n");
  
  //----- INITIALIZING SUPERIOR COLLICULUS SIMULATIONS -------//
  SC_memb_t_const = 0.1; //in s, i.e. 500ms
  SC_memb_decay = exp(-SC_memb_t_const); //div DT
  SC_memb_decay_compl = 1 - SC_memb_decay;
  SC_l = malloc(sizeof(float*) * salmap_w); //assume we just want salmap w neurons. note we could just use fixed memory...
  SC_r = malloc(sizeof(float*) * salmap_w);
  for(int x=0; x<salmap_w; x++)
    {
      SC_l[x] = malloc(sizeof(float) * salmap_h);
      SC_r[x] = malloc(sizeof(float) * salmap_h);
      for(int y=0; y<salmap_h; y++)
	{
	  SC_l[x][y] = 0; //set all membrane potentials to 0mV...
	  SC_r[x][y] = 0;
	}
    }
  
  printf("FINISHED INITIALIZING SUPERIOR COLLICULUS MODEL\n");

  
 
  
  
  return 0;
} //end envision_init


//======================= ENVISION NEXTPIC FROMMEM ===============================//

int envision_nextframe(const IplImage* ipl_input,  //ipl image as input
		       IplImage* ipl_output, int eye_side)   //output image we'll write to (use ->imageData to write)
{
    
  struct env_dims indims;
  indims.w = ipl_input->width;
  indims.h = ipl_input->height;
  
  //input is ipl_input as const env_rgb_pixel*
  const struct env_rgb_pixel* input = (struct env_rgb_pixel*)ipl_input->imageData;
  
  npixels = indims.w * indims.h;
    
  //initialize if necessary...note size is pretty manual. We really want ivcout.dims.w etc.
  if(!(INITIALIZED == 1))
    {
      envision_init(indims.w, indims.h);
      
      
      INITIALIZED=1;
    }
    
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
  
  if(eye_side == L_EYE)
    {
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
  
    } //end if eyeside == 0
  else if(eye_side == R_EYE)
    {  
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
    } //end if eyeside == R_EYE
  
  
  //--------------(things to do only after first frame)------------//
  if((eye_side == L_EYE && FIRSTFRAMEL == 0) || (eye_side == R_EYE && FIRSTFRAMER == 0)) 
    {
      //REV: still not sure that passing over rescale step is OK, but trying it anyways.
      //values are between 0 and INT_MAX (highest no guarantee might
      //be low), instead of 0, CHAR_MAX (highest guarneteed charmax)
      /*
      env_visual_cortex_rescale_ranges(&ivcout, &intens, &color, &ori
#ifdef ENV_WITH_DYNAMIC_CHANNELS
				       , &flicker, &motion
#endif
				       );
      */
      
      //------------ Do stuff in here -------------//
      
      //CALCULATE LPF SALMAP FOR THIS INPUT (no timecourse? Whatever. Make gaussian sufficiently small and we get basically nothing)      
      for(int y=0; y<salmap_h; y++)
	{
	  for(int x=0; x<salmap_w; x++)
	    {
	      lpf_salmap[x][y]=0;
#ifdef EDGECORR
	      float integral=0;
#endif
	      for(int dx=0; dx<gauss_filter_w; dx++)
		{
		  for(int dy=0; dy<gauss_filter_w; dy++)
		    {
		      //dx, dy is location in gauss_filter
		      //but actual location is x+(dx-gauss_filter_c). I.e. first is x + (0-4), i.e. x-4. Last is x+(8-4) = x+4
		      int tx= x+(dx-gauss_filter_c); // ("true" x, true y, etc.)
		      int ty= y+(dy-gauss_filter_c);
		      //printf("Calc true xy: (%2i, %2i)\n", tx, ty);
		      if(tx >= 0 && tx < salmap_w && ty >= 0 && ty < salmap_h) //only if legal point
			{
#ifdef EDGECORR
			  integral += gauss_filter[dx][dy];
#endif
			  int loc = x_y_to_array_loc(tx, ty, salmap_w);
			  float val = (float)ivcout.pixels[loc] / (float)INT_MAX;
			  lpf_salmap[x][y] += gauss_filter[dx][dy] * val;
			}
		    }
		}
	      
	      //int val = ivcout.pixels[x_y_to_array_loc(x, y, salmap_w)];
	      //printf("%0.3f ", (float)val/(float)INT_MAX);
	      
#ifdef EDGECORR
	      lpf_salmap[x][y] /= integral;
#endif
	      //printf("%0.3f ", lpf_salmap[x][y]);
	      //printf("%0.3f ", integral);
	      
	    }
	  //printf("\n");
	}
      //printf("\n");
      
      
      
      //we now have stored as floats in lpf_salmap[x][y] the lpf version of the salmap.
      //Now, use this to update the appropriate long-term saliency map's individual neurons
      
      
      //REV: TODO UPDATE LONG-TERM SALIENCY MAP HERE
      //SC_l and SC_r using SC_memb_decay and SC_memb_decay_compl. Do I need to calculate Isyn better?
      //feed current lpf_salmap[x][y] as input into each SC_l[x][y] neuron

      //note, we could've done the gaussian at THIS level as well (i.e. a weight array into each neuron that samples
      //from the surround of the next level down). It's all going to sum anyways, just doing it pre-transmission is good, since linear syn.
      
      if(eye_side == L_EYE)
	SC_l_maxval = -1;
      else if(eye_side == R_EYE)
	SC_r_maxval = -1;
      
      for(int x=0; x<salmap_w; x++)
	{
	  for(int y=0; y<salmap_h; y++)
	    {
	      
	      float Isyn = lpf_salmap[x][y] * SC_W_mult;
	      //printf("Adding: %0.2f (decay: %0.2f   compl: %0.2f)\n", Isyn, SC_memb_decay, SC_memb_decay_compl);
	      
	      if(eye_side == L_EYE)
		{
		  
		  SC_l[x][y] = (SC_l[x][y] * SC_memb_decay) + (SC_memb_decay_compl * (0+Isyn)); //0 is Ibg
		  if(SC_l[x][y] > SC_l_maxval)
		    {
		      SC_l_maxval = SC_l[x][y];
		      SC_l_maxx= x;
		      SC_l_maxy= y;
		      
		    }
		  
		}
	      else if(eye_side == R_EYE)
		{
		  SC_r[x][y] = (SC_r[x][y] * SC_memb_decay) + (SC_memb_decay_compl * (0+Isyn)); //0 is Ibg
		  if(SC_r[x][y] > SC_r_maxval)
		    {
		      SC_r_maxval = SC_r[x][y];
		      SC_r_maxx= x;
		      SC_r_maxy= y;
		    }
		}
	    }
	}
      
      
      //DRAW SHIT! (note there needs to be an update step or something...? This one could draw the other 3 boxes, 
      //but what we're interested in is drawing the main one right? That could be in another function...?)
      //need some marker to tell which side (L or R) the current winning max-thing is?
      //(diff function?)
      
      // cleaning up memory (deallocating the different salmaps channels & ivcout!) for next run-through
      env_img_make_empty(&ivcout);
      env_img_make_empty(&intens);
      env_img_make_empty(&color);
      env_img_make_empty(&ori);
#ifdef ENV_WITH_DYNAMIC_CHANNELS
      env_img_make_empty(&flicker);
      env_img_make_empty(&motion);
#endif
    } //end if eye_side==L_EYE && FIRSTFRAMEL == 0
  
  //REV: weve been through things once, set FIRSTFRAMEL/R to false
  if(eye_side == L_EYE)       FIRSTFRAMEL=0; 
  else if(eye_side == R_EYE)  FIRSTFRAMER=0;
  
  return 0; //success?
} //end nextpic_frommem

//==================== SC_NAIVE_WINNER =====================//
//compares SC_l_maxval to SC_r_maxval (req. maxval > 1) and gives a winner. Draws?
int SC_naive_competition(IplImage* ipl_outputL, IplImage* ipl_outputR)
{
  float maxval=-1;
  if(SC_l_maxval > maxval)
    {
      maxval = SC_l_maxval;
      if(maxval > SC_thresh)
	SC_win_side = L_EYE;
    }
  if(SC_r_maxval > maxval)
    {
      maxval = SC_r_maxval;
      if(maxval>SC_thresh)
	SC_win_side = R_EYE;
    }
  //else, maxval just stays -1, and winside remains -1 (i.e. no winner (yet))
  //we might want a THRESHOLD for winning too! (the other way of doing things)
  if(SC_win_side == L_EYE)
    printf("WIN: %0.2f (%i %i)\n", SC_l_maxval, SC_l_maxx, SC_l_maxy);
  else if(SC_win_side == R_EYE)
    printf("WIN: %0.2f (%i %i)\n", SC_r_maxval, SC_r_maxx, SC_r_maxy);
  else
    printf("WIN: NONE\n");
  
  for(int y=0; y<salmap_h; y++)
    {
      for(int x=0; x<salmap_w; x++)
	{
	  printf("%0.2f ", SC_l[x][y]);
	}
      
      printf("     ");
      for(int x=0; x<salmap_w; x++)
	{
	  printf("%0.2f ", SC_r[x][y]);
	    }
      printf("\n");
    }
  printf("\n\n");
  
  //either way, draw the shizzle! //first, just draw the two!
  //just draw at original size! (do L first, then R)
  //problem is that the LIF neurons have no "cap" for their Vm, since they could explode from lots of input
  //(unless we force them to hit a cap and stay there...?). But, we can't always just scale (though we could do first-to threshold!)
  //so just do everything that's at threshold as "white" and at continuous levels beneath it.
  
  //so, 0 is 0.0, 255 is SC_thresh (and above).    --> (val/thresh) (cap to 1) then mult by 255.
  //Also, if what-you-call it is on this side (& above thresh, but that's implicit), draw a (yellow) circle!
  
  int width = ipl_outputL->width;
  int height = ipl_outputL->height;
  char* img_output_ptr = ipl_outputL->imageData;
  float** state_ptr = SC_l;
  
  float CAP=SC_thresh;
  //find max first...lame
  for(int x=0; x<salmap_w; x++)
    {
      for(int y=0; y<salmap_h; y++)
	{
	  if(state_ptr[x][y] > CAP)
	    CAP = state_ptr[x][y];
	}
    }
  
  //for each state_ptr[x][y], go and draw a 16x16 square in output (starting at x*16, y*16), offset by proper amount.
  for(int x=0; x<salmap_w; x++) //16 is hard coded lewl
    {
      for(int y=0; y<salmap_h; y++)
	{
	  float val = state_ptr[x][y] / CAP; //calc ratio
	  if(val > 1) printf("SOMETHING IS BROKEN IT HSOULDNT BIG BIGGER\n");
	  val *= 255; //put it in right range
	  int intval = (int)val; //this SHOULD be between 0, 255... 
	  char charval = (char)intval; //(converting directly might do something fishy with floating point representation)
	  for(int dy=0; dy<16; dy++)
	    {
	      for(int dx=0; dx<16; dx++)
		{
		  
		  int corrx = x*16 + dx;
		  int corry = y*16 + dy;
		  //printf("Writing to: %i %i\n", corrx, corry);
		  int offsetloc = x_y_to_array_loc(corrx, corry, width)*3;
		  img_output_ptr[offsetloc+0] = charval;
		  img_output_ptr[offsetloc+1] = charval;
		  img_output_ptr[offsetloc+2] = charval;
		}
	    }
	}
    }
  
  if(SC_win_side == L_EYE)
    {
      //draw circle
      //SC_l_maxx, SC_l_maxy 
      //draw circle
      //SC_r_maxx, SC_r_maxy
      int realx = SC_l_maxx*16 + 8; //8 to put in mid of winner square
      int realy = SC_l_maxy*16 + 8;
      //draw a circle around that. Circle radius is gauss_filter_radius
      int realrad = gauss_filter_radius*16;
      for(int y= realy - realrad; y<realy+realrad; y++)
	{
	  for(int x=realx -realrad; x<realx+realrad; x++)
	    {
	      float dist = sqrt((x-realx) * (x-realx)+ (y-realy)*(y-realy));
	      if(dist <= realrad && dist >= realrad-1 && y >= 0 && y<height && x>=0 && x<width)
		{
		  int offsetloc = x_y_to_array_loc(x, y, width)*3;
		  img_output_ptr[offsetloc+0] = 0;
		  img_output_ptr[offsetloc+1] = 255;
		  img_output_ptr[offsetloc+2] = 255;
		}
	  
	    }
	}
    }
  
  
  //RIGHT SIDE
  img_output_ptr = ipl_outputR->imageData;
  state_ptr = SC_r;
   
  CAP=SC_thresh;
  //find max first...lame
  for(int x=0; x<salmap_w; x++)
    {
      for(int y=0; y<salmap_h; y++)
	{
	  if(state_ptr[x][y] > CAP)
	    CAP = state_ptr[x][y];
	}
    }
  
  //for each state_ptr[x][y], go and draw a 16x16 square in output (starting at x*16, y*16), offset by proper amount.
  for(int x=0; x<salmap_w; x++) //16 is hard coded lewl
    {
      for(int y=0; y<salmap_h; y++)
	{
	  float val = state_ptr[x][y] / CAP; //calc ratio
	  if(val > 1) printf("SOMETHING IS BROKEN IT SHOULDNT BE GREATHER THAN 1 RIGHT\n"); //cap it
	  val *= 255; //put it in right range
	  int intval = (int)val; //this SHOULD be between 0, 255... 
	  char charval = (char)intval; //(converting directly might do something fishy with floating point representation)
	  for(int dx=0; dx<16; dx++)
	    {
	      for(int dy=0; dy<16; dy++)
		{
		  int corrx = x*16 + dx;
		  int corry = y*16 + dy;
		  int offsetloc = x_y_to_array_loc(corrx, corry, width)*3;
		  img_output_ptr[offsetloc+0] = charval;
		  img_output_ptr[offsetloc+1] = charval;
		  img_output_ptr[offsetloc+2] = charval;
		}
	    }
	}
    }//end for all x in salmap_w

  if(SC_win_side == R_EYE)
    {
      //draw circle
      //SC_r_maxx, SC_r_maxy
      int realx = SC_r_maxx*16 + 8; //8 to put it in middle of winner square
      int realy = SC_r_maxy*16 + 8;
      //draw a circle around that. Circle radius is gauss_filter_radius
      int realrad = gauss_filter_radius*16;
      for(int y= realy - realrad; y<realy+realrad; y++)
	{
	  for(int x=realx -realrad; x<realx+realrad; x++)
	    {
	      float dist = sqrt((x-realx) * (x-realx) + (y-realy)*(y-realy));
	      if(dist <= realrad && dist >= realrad-1 && y >= 0 && y<height && x>=0 && x<width)
		{
		  int offsetloc = x_y_to_array_loc(x, y, width)*3;
		  img_output_ptr[offsetloc+0] = 0;
		  img_output_ptr[offsetloc+1] = 255;
		  img_output_ptr[offsetloc+2] = 255;
		}
	  
	    }
	}
      
    }
  
  
  
  //DRAW A CIRCLE!
  
}



//===================== ENVISION CLEANUP =================//
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
}//end envision cleanup








//===================== OLD VERSION =====================//

int WTAnet_winner = -1;

int MASS_pixels;
int MASS_radius;

//REV: nextpic_frommem: calculates saliency on given input, and draws appropriate pictures on output, 
//given input (assumedly from some other program implementing winner-take-all saliency, etc.)
int* envision_nextpic_frommem(const IplImage* ipl_input,  //ipl image as input
			      IplImage* output, int desired_numsalwinners, int eye_side)   //output image we'll write to
{
  int* xywinners = malloc(2* sizeof(int) * desired_numsalwinners); //*2 because we are giving 2 points for each.
  int* xywinners1d = malloc( sizeof(int) * desired_numsalwinners); //whatever, ints when we could use chars, who cares.

  for(int zz=0; zz<desired_numsalwinners; zz++)
    {
      xywinners1d[zz] = -1;
      xywinners[zz*2] = -1;
      xywinners[zz*2+1] = -1;
    }
  
  struct env_dims indims;
  indims.w = ipl_input->width;
  indims.h = ipl_input->height;
  
  //printf("IMAGE SIZE: %i %i \n", ipl_input->width, ipl_input->height);
  //input is ipl_input as const env_rgb_pixel*
  const struct env_rgb_pixel* input = (struct env_rgb_pixel*)ipl_input->imageData;
  
  npixels = indims.w * indims.h;
  float width_scale = 320 / indims.w; //this is written for 320...multiply everything hardcoded by scale, right?
  
  //initialize if necessary...note size is pretty manual. We really want ivcout.dims.w etc.
  if(!(INITIALIZED == 1))
    {
      envision_init(indims.w, indims.h);
      divnum=1;
      for(int x=0; x<envp.output_map_level; x++)
	{
	  divnum *= 2; //just doing 2^output_map_level
	}
      
      salmap_w = indims.w/divnum;
      salmap_h = indims.h/divnum;
      
      
      //REV: these are for drawing the box now, nothing more...
      MASS_pixels = (indims.w/divnum)/3; //this will be ivcout.dims.w...?
      MASS_radius = MASS_pixels/2;
      
      INITIALIZED=1;
    }
    
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
  
  if(eye_side == 0)
    {
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
  
    } //end if eyeside == 0
  else if(eye_side == 1)
    {  
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
  
    } //end if eyeside == 1
  if((eye_side == 0 && FIRSTFRAMEL == 0) || (eye_side==1 && FIRSTFRAMER == 0)) 
    //because movement isn't calculated until 2nd frame...only use ivcout for now or it will segfault
    {
      //REV: this scales to between 0 and 255 ...from min and max of the other one? Because remember, there could be negatives 
      //(which is OK, just part of the "relative scale". So, that won't mess anything up, right...?
      /*env_visual_cortex_rescale_ranges(
				       &ivcout, &intens, &color, &ori
#ifdef ENV_WITH_DYNAMIC_CHANNELS
				       , &flicker, &motion
#endif
				       );
      */
      
      //copy ivcout to ipl output image to draw...(I need to interpolate to the image size...)
      char* dataptr = output->imageData;
     
      int max_loc = -1;
      int max_val = -1;
	       	  
      for(int zz=0; zz<desired_numsalwinners; zz++)
	{ 
	  max_val = -1;
	  max_loc = -1;
	  
	  //calc max_loc here (stupid, local, instantaneous)
	  for(int y=0; y<ivcout.dims.h;y++)
	    {
	      for(int x=0; x<ivcout.dims.w; x++)
		{
		  int exists=0;
		  int loc = x_y_to_array_loc(x, y, salmap_w);
		  int val = ivcout.pixels[loc];
		  if(zz==0)
		    {
		      printf("%0.3f ", (float)val/(float)INT_MAX);
		      //printf("%3i ", val);
		    }
		  //check to make sure it's none of the others...
		  for(int ee=0; ee<zz; ee++)
		    {
		      
		      if(xywinners1d[ee] == loc)
			{
			  exists=1;
			  
			}
		    }
		  
		  if(val > max_val && exists==0)
		    {
		      max_val = val;
		      max_loc = loc;
		    }
		}
	      if(zz==0)
		{
		  printf("\n");
		}
	    }
	  if(zz==0)
	    printf("\n\n");
	  
	  xywinners1d[zz] = max_loc;
	  xywinners[zz*2] = x_loc(max_loc, ivcout.dims.w); //this is scaled up...?
	  xywinners[zz*2+1] = y_loc(max_loc, ivcout.dims.w);
	}
            
      
      //set external x,y winners so avg color can be calced and passed to java side
      //REV: this is in *SCALED UP* image...(for drawing only..WTAnet is for WTAnet calc too so not scaled)
      
      WTAnet_winner = xywinners1d[0]; //hope this works?
      
      
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
      
      int y=0;
      //REV: write out first two output quarters (top-left, top-right)
      while(y<ivcout.dims.h*ivcout.dims.w)//all same size anyways...just use ivcout as reference
	{
	  
	  for(int x=0; x<x_scale/2; x++)
	    {
	      for(int a=0; a<3; a++)
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
	  
	  for(int x=0; x<x_scale/2; x++)
	    {
	      //add c_width/2 to get it halfway across...
	      for(int a=0; a<3; a++)
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
	      for(int z=1; z<x_scale/2; z++)
		{
		  for(int x=c_width-1; x>=0; x--)
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
	  for(int x=0; x<x_scale/2; x++)
	    {
	      for(int a=0; a<3; a++)
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
	  for(int x=0; x<x_scale/2; x++)
	    {
	      
	      //add c_width/2 to get it halfway across...
	       
	      //dataptr[startpos_l + c_width/2 + 3*((y - startpos_s) * x_scale/2 + x)+0] = current_sal_color[2]; //value
	     
	      for(int a=0; a<3; a++)
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
	      for(int z=1; z<x_scale/2; z++)
		{
		  for(int x=c_width-1; x>=0; x--)
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
      y=0; //err?
      if(WTAnet_winner > -1) //only if we've chosen a winner already...
	{
	  width_scale = 2; //bc 2 images per width of output image.
	  int max_l_x; int max_l_y; int lt_x; int lt_y;
	  for(int z=0; z<1; z++)//for each of the boxes...
	    {
	      //max_l_x = x_loc(max_loc_l[z], c_width);
	      //max_l_y = y_loc(max_loc_l[z], c_width);
	      lt_x = x_loc(max_loc_l[4], c_width);
	      lt_y = y_loc(max_loc_l[4], c_width);
	      max_l_x = lt_x;
	      max_l_y = lt_y; //just make it follow the small one..
	      //draw horizontal "Y" lines...
	      for(int x= (-x_scale * (MASS_radius)) / 2;   x < x_scale/2 + (x_scale * (MASS_radius))/2; x++)
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
	      for(int x=0; x<divnum/2; x++)
		{
		  y=0;
		  int loc = x_y_to_array_loc(lt_x+(x*3),lt_y+y,c_width);
		  dataptr[loc +0] = 0; //these didnt have +y (wtf?)
		  dataptr[loc +1] = 255;
		  dataptr[loc +2] = 0;
		  
		  y=divnum/2 -1; //-1 because, as above, see its x<16/2 (not <=) we want all INSIDE the square
		  loc = x_y_to_array_loc(lt_x+(x*3), lt_y+y, c_width);
		  dataptr[loc +0] = 0; //these didnt have +y
		  dataptr[loc +1] = 255;
		  dataptr[loc +2] = 0;
		}
	      
	      for(y=0; y<divnum/2; y++)
		{
		  int x=0;
		  int loc = x_y_to_array_loc(lt_x+(x*3), lt_y+y, c_width);
		  dataptr[loc +0] = 0;
		  dataptr[loc +1] = 255;
		  dataptr[loc +2] = 0;
		  
		  x=divnum/2 -1; //-1 because, as above, see its y<16/2, we want it all INSIDE the square.
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
		      int x=(-x_scale * (MASS_radius)) /2;
		      
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
    } //end if eye_side==0 && FIRSTFRAMEL == 0
  if(eye_side == 0)
    FIRSTFRAMEL=0; //REV: weve been through things once...
  else if(eye_side == 1)
    FIRSTFRAMER=0;
  
  return xywinners; //REV: for imclever return xywinners
  
} //end nextpic_frommem

float gaussian_val(float x_input, float sd, float mean, float scale)
{
  return scale*exp(-(((x_input-mean) * (x_input-mean))/(2*sd*sd)));
}

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
