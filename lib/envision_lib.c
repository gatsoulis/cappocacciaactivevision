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

#include <stdio.h>
#include <stdlib.h> // for atoi(), malloc(), free()
#include <math.h>
#include "envision_lib.h"
#include "opencv/cxcore.h"//this has the IplImage definition in it...
#include <sys/time.h>
//#include <gsl/gsl_randist.h> //for gamma random
//#include <gsl/gsl_rng.h> //gsl random number generator

#define SMALL_NUM 0.00001

//gsl_rng* gsl_rand_source; //for uniform random

int x_loc(int, int);
int y_loc(int, int);
int x_y_to_array_loc(int, int, int);

//REV: for saliency map
//int sal_framenum;
int sal_winner_coords[2]; //winner coordinates of *this time-step*
int WTA_winner_coords[2]; //current winner-take-all(WTA) winner coords
int WTAnet_winner; // = -1;

int salmap_w;
int salmap_h;

int INITIALIZEDR = 0; //initially, neither R not L side is initialized
int INITIALIZEDL = 0;
int FIRSTFRAMER = 1; //initially, it's the first frame for R and L side
int FIRSTFRAMEL = 1;

int MASS_pixels;
int MASS_radius;


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


int envision_init(int width, int height, int eye_side)
{
  
  printf("INITIALIZING...");
  
  salmap_w = width;
  salmap_h = height;
  
  MASS_pixels = width/3; ///4// = 5; 
  MASS_radius = MASS_pixels/2;
    
  // Instantiate our various ModelComponents:
  env_params_set_defaults(&envp);
  
  //envp.maxnorm_type = ENV_VCXNORM_MAXNORM;
  envp.maxnorm_type = ENV_VCXNORM_NONE;
  envp.scale_bits = 16;
  
  env_assert_set_handler(&env_stdio_assert_handler);
  if (multithreaded)
    {
      env_init_pthread_alloc();
      env_init_pthread_job_server();
    }
  env_allocation_init(&malloc_thunk, &free);
  //REV: Open the { for the unqualified { } section here.
  
  if(eye_side == 0)
    {
      env_visual_cortex_init(&ivcL, &envp);  
      printf("FINISHED SALIENCY MAP INITIALIZATION (LEFT EYE)\n");
    }
  else if(eye_side == 1)
    {
      env_visual_cortex_init(&ivcR, &envp);  
      printf("FINISHED SALIENCY MAP INITIALIZATION (RIGHT EYE)\n");
    }
  else
    {
      printf("ERROR: eye side is not 0 or 1! envision_init requires eye_side==0 (left) or eye_side==1 (right)\n\n");
      exit(1);
    }
  
  
  
  return 0;
}

//LEFT EYE

//REV: nextpic_frommem: calculates saliency on given input, and draws appropriate pictures on output, 
//given input (assumedly from some other program implementing winner-take-all saliency, etc.)
//we're using global data from SalMapData to share required data/map output with java etc.
//called as quickly as possible in a separate thread (best way to do it?)
int* envision_nextpic_frommem(const IplImage* ipl_input,  //ipl image as input
			       IplImage* output, int desired_numsalwinners, int eye_side)   //output image we'll write to
{
  int* xywinners = malloc(2* sizeof(int) * desired_numsalwinners); //*2 because we are giving 2 points for each.
  int* xywinners1d = malloc( sizeof(int) * desired_numsalwinners); //whatever, ints when we could use chars, who cares.

  for(int zz=0; zz<desired_numsalwinners; zz++)
    {
      xywinners1d[zz] = -666;
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
  sal_winner_coords[0]=-1; sal_winner_coords[1]=-1;
  float width_scale = 320 / indims.w; //this is written for 320...multiply everything hardcoded by scale, right?
  
  //initialize if necessary...note size is pretty manual
  if(eye_side == 0 && !(INITIALIZEDL == 1))
    {
      envision_init(indims.w/16, indims.h/16, 0); 
      INITIALIZEDL=1;
    }
  else if(eye_side == 1 && !(INITIALIZEDR == 1))
    {
      envision_init(indims.w/16, indims.h/16, 1); 
      INITIALIZEDR=1;
    }
  if(eye_side != 1 && eye_side != 0)
    {
      printf("ERROR: nextpic_frommem: eye_side is not 0 or 1: (left or right) -- You only have 2 eyes don't give it a diff number!\n\n");
      exit(1);
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
	       	  
      for(int zz=0; zz<desired_numsalwinners; zz++)
	{ 
	  max_val = -1;
	  max_loc = -1;
	  
	  //calc max_loc here:
	  for(int x=0; x<salmap_w; x++)
	    {
	      for(int y=0; y<salmap_h; y++)
		{
		  int exists=0;
		  int loc = x_y_to_array_loc(x, y, salmap_w);
		  int val = ivcout.pixels[loc];
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
	    }
	  
	  xywinners1d[zz] = max_loc;
	  xywinners[zz*2] = x_loc(max_loc, ivcout.dims.w); //this is scaled up...?
	  xywinners[zz*2+1] = y_loc(max_loc, ivcout.dims.w);
	}
            
      
      //set external x,y winners so avg color can be calced and passed to java side
      //REV: this is in *SCALED UP* image...(for drawing only..WTAnet is for WTAnet calc too so not scaled)
      
      WTAnet_winner = xywinners1d[0]; //hope this works?
      
      
      int win_x = xywinners[0];
      int win_y = xywinners[1];
				   
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
	      for(int x=0; x<16/2; x++)
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
		  int x=0;
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
  else
    {
      printf("WTF eye_side isn't 1 or 0!");
      exit(1);
    }
  
 
  //HMM, can we assume blurriness for infants? Not fully developed... Maybe that's why later on
  //parental (social) cues are better?
  
  
  return xywinners; //REV: for imclever return xywinners
  
}//end nextpic_frommem

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
