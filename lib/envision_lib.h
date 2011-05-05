//Header file for envision_lib...
//Richard Veale 15 may 2010
#include <opencv/cxcore.h>
#include "env_image.h"

#ifdef  __cplusplus
extern "C" {
#endif
  int envision_nextframe(const IplImage* ipl_input, IplImage* ipl_output, int eye_side);
#ifdef  __cplusplus
}
#endif

#ifdef  __cplusplus
extern "C" {
#endif
  int SC_naive_competition(IplImage* ipl_outputL, IplImage* ipl_outputR);
#ifdef  __cplusplus
}
#endif

#ifdef  __cplusplus
extern "C" {
#endif
  int envision_cleanup(void);
#ifdef  __cplusplus
}
#endif


//rev; old guy
#ifdef  __cplusplus
extern "C" {
#endif
  int* envision_nextpic_frommem(const IplImage* ipl_input, IplImage* output, int desired_numsalwinners, int eye_side);
#ifdef  __cplusplus
}
#endif
