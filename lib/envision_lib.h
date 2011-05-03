//Header file for envision_lib...
//Richard Veale 15 may 2010
#include <opencv/cxcore.h>
#include "env_image.h"


void init_salmap(int w, int h);

#ifdef  __cplusplus
extern "C" {
#endif
  void delete_salmap(void);
#ifdef  __cplusplus
}
#endif


#ifdef  __cplusplus
extern "C" {
#endif
  void update_WTAnet(unsigned char* win_rgb);
#ifdef  __cplusplus
}
#endif

#ifdef  __cplusplus
extern "C" {
#endif
  int envision_initL(int width, int height);
#ifdef  __cplusplus
}
#endif

#ifdef  __cplusplus
extern "C" {
#endif
  int envision_initR(int width, int height);
#ifdef  __cplusplus
}
#endif

#ifdef  __cplusplus
extern "C" {
#endif
  int envision_nextpic(int img_num, char* instem, char* outstem);
#ifdef  __cplusplus
}
#endif

#ifdef  __cplusplus
extern "C" {
#endif
  int env_tester(int blah);
#ifdef  __cplusplus
}
#endif

#ifdef  __cplusplus
extern "C" {
#endif
  int* envision_nextpic_frommemL(const IplImage* ipl_input, IplImage* output, int desired_numsalwinners);
#ifdef  __cplusplus
}
#endif

#ifdef  __cplusplus
extern "C" {
#endif
  int* envision_nextpic_frommemR(const IplImage* ipl_input, IplImage* output, int desired_numsalwinners);
#ifdef  __cplusplus
}
#endif

#ifdef  __cplusplus
extern "C" {
#endif
 const  struct env_rgb_pixel* cv_img_to_envrgbpix(const IplImage* ipl_input);
#ifdef  __cplusplus
}
#endif

/*
#ifdef  __cplusplus
extern "C" {
#endif
  int envision_nextpic_bias(int img_num, char* instem, char* outstem, 
			    float red_bias, float green_bias, float blue_bias,
			    float yellow_bias);
#ifdef  __cplusplus
}
#endif
*/

#ifdef  __cplusplus
extern "C" {
#endif
  int envision_cleanup(void);
#ifdef  __cplusplus
}
#endif

#ifdef  __cplusplus
extern "C" {
#endif
  void resetWTAnet();
#ifdef  __cplusplus
}
#endif
