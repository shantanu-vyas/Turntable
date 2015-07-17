#ifndef TURNTABLE_H
#define TURNTABLE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __APPLE__
#  ifdef _WIN32
#    include <windows.h>
#  endif
#  include <GL/glut.h>
#else
#  include <GLUT/glut.h>
#endif
#include <AR/ar.h>
#include <AR/gsub.h>
#include <AR/video.h>
#include <AR/arMulti.h>

#include <cv.hpp>
#include <thread>

using namespace cv;
using namespace std;

#define                 CPARA_NAME       "Data/camera_para.dat"
#define                 CONFIG_NAME      "Data/multi/marker.dat"

ARHandle               *arHandle;
AR3DHandle             *ar3DHandle;
ARGViewportHandle      *vp;
ARMultiMarkerInfoT     *config;
ARMultiMarkerInfoT     *original;

int                     robustFlag = 0;
int                     counter;
ARParamLT              *gCparamLT = NULL;
int first = 1;
ARUint8 *currentFrame;

static void init(int argc, char *argv[]);
static  void cleanup(void);
static  void mainLoop(void);
static  void draw( ARdouble trans1[3][4], ARdouble trans2[3][4], int mode );
static  void keyEvent( unsigned char key, int x, int y);
static  void updateMat(cv::Mat input);
static  void setup();


int main(int argc, char *argv[])
{

  glutInit(&argc,argv);
  init(argc, argv);

  argSetDispFunc( mainLoop, 1 );
  argSetKeyFunc( keyEvent );
  counter = 0;
  arVideoCapStart();
  arUtilTimerReset();
  argMainLoop();
}

static void keyEvent( unsigned char key, int x, int y)
{
  int     debug;
  int     thresh = 120; //120 works nice in lighting of office

  /* quit if the ESC key is pressed */
  if( key == 0x1b ) {
    //ARLOG("*** %f (frame/sec)\n", (double)counter/arUtilTimer());
    cleanup();
    exit(0);
  }
  if (key == 't')
    {
      first = 1;
    }
  if( key == 'd' ) {
    arGetDebugMode( arHandle, &debug );
    debug = 1 - debug;
    arSetDebugMode( arHandle, debug );
  }

  if( key == '1' ) {
    arGetDebugMode( arHandle, &debug );
    if( debug ) {
      arGetLabelingThresh( arHandle, &thresh );
      thresh -= 5;
      if( thresh < 0 ) thresh = 0;
      arSetLabelingThresh( arHandle, thresh );
      //ARLOG("thresh = %d\n", thresh);
    }
  }
  if( key == '2' ) {
    arGetDebugMode( arHandle, &debug );
    if( debug ) {
      arGetLabelingThresh( arHandle, &thresh );
      thresh += 5;
      if( thresh > 255 ) thresh = 255;
      arSetLabelingThresh( arHandle, thresh );
      //ARLOG("thresh = %d\n", thresh);
    }
  }

  if( key == ' ' ) {
    robustFlag = 1 - robustFlag;
    if( robustFlag ) ARLOG("Robust estimation mode.\n");
    else             ARLOG("Normal estimation mode.\n");
  }}


//static void updateMat(cv::Mat input)
//{
// input.convertTo(input,CV_8U);
// currentFrame = input.data;

//}

/* main loop */
static void mainLoop(void)
{
  ARUint8         *dataPtr; //video frame
  ARMarkerInfo    *marker_info;
  int             marker_num;
  int             imageProcMode;
  int             debugMode;
  double          err;
  int             i;

  //  dataPtr = currentFrame;

  if( (dataPtr = (ARUint8 *)arVideoGetImage()) == NULL ) {
   arUtilSleep(2);
   return;
    }


  if( counter == 100 ) {
    //ARLOG("*** %f (frame/sec)\n", (double)counter/arUtilTimer());
    arUtilTimerReset();
    counter = 0;
  }
  counter++;

  /* detect the markers in the video frame */
  if( arDetectMarker(arHandle, dataPtr) < 0 ) {
    cleanup();
    exit(0);
  }
  marker_num = arGetMarkerNum( arHandle );
  marker_info =  arGetMarker( arHandle );

  argDrawMode2D(vp);
  arGetDebugMode( arHandle, &debugMode );
  if( debugMode == 0 ) {
    argDrawImage( dataPtr );
  }
  else {
    arGetImageProcMode(arHandle, &imageProcMode);
    if( imageProcMode == AR_IMAGE_PROC_FRAME_IMAGE ) {
      argDrawImage( arHandle->labelInfo.bwImage );
    }
    else {
      argDrawImageHalf( arHandle->labelInfo.bwImage );
    }
    glColor3f( 1.0f, 0.0f, 0.0f );
    glLineWidth( 5.0f);
    for( i = 0; i < marker_num; i++ ) {
      argDrawSquareByIdealPos( marker_info[i].vertex );
    }
    glLineWidth( 1.0f );
  }

  if( robustFlag ) {
    err = arGetTransMatMultiSquareRobust( ar3DHandle, marker_info, marker_num, config);
  }
  else {
    err = arGetTransMatMultiSquare( ar3DHandle, marker_info, marker_num, config);
  }
  if( config->prevF == 0 ) {
    argSwapBuffers();
    return;
  }
  //ARLOGd("err = %f\n", err);

  argDrawMode3D(vp);
  glClearDepth( 1.0 );
  glClear(GL_DEPTH_BUFFER_BIT);
  for( i = 0; i < config->marker_num; i++ ) {

    //draw( config->trans, config->marker[i].trans, 0 );
    if( config->marker[i].visible >= 0 ) draw( config->trans, config->marker[i].trans, 0 );
    else        draw( config->trans, config->marker[i].trans, 1 );
  }

  /*for each marker if its visible find transformation back to original marker, print */
  int counterer = 0;

  //fflush(stdout);
  /*
   * should press button to initialize, it will save the first frame
   * it should then compare each new frame to that one
   * from there it save the transformation matrix
   * once the transformation matrix is applied it should
   * run icp on the 2 meshes
   */


  for(int i = 0; i < config->marker_num; i++)
    {
      fflush(stdout);

      //when it sees at least 1 for the first time save that configuration
      if (first == 1 && config->marker[i].visible >1)
        {
          original = config;
          first = 0;
          printf("%s","called \n");
          double trans[3][4];

          for (int p = 0; p < 3; p++)
            {
              for (int t = 0; t < 4; t++)
                {
                  trans[p][t] = original->marker[i].trans[p][t];
                }
            }

          for (int p = 0; p < 3; p++)
            {
              for (int t = 0; t < 4; t++)
                {
                  printf("%8.4f ", trans[p][t]);
                }
              printf("%s ", "\n");
            }

          double target_trans[3][4];


        }

      counterer++;
    }

  argSwapBuffers();
}

static void init(int argc, char *argv[])
{
  ARParam         cparam;
  ARGViewport     viewport;
  ARPattHandle   *arPattHandle;
  char            vconf[512];
  char            configName[512];
  int             xsize, ysize;
  AR_PIXEL_FORMAT pixFormat;
  int             i;

  configName[0] = '\0';
  vconf[0] = '\0';

  char *confi = "v4l2src device=/dev/video0 ! capsfilter caps=video/x-raw-rgb,bpp=24 ! identity name=artoolkit ! fakesink";
  for( i = 1; i < argc; i++ ) {
    if( strncmp(argv[i], "-config=", 8) == 0 ) {
      strcpy(configName, &argv[i][8]);
    }
    else {
      if( confi[0] != '\0' ) strcat(confi, " ");
      strcat(confi, argv[i]);
    }
  }
  if( configName[0] == '\0' ) strcpy(configName, CONFIG_NAME);

  /* open the video path */
  if( arVideoOpen( confi ) < 0 ) exit(0);
  /* find the size of the window */
  if( arVideoGetSize(&xsize, &ysize) < 0 ) exit(0);
  ARLOGi("Image size (x,y) = (%d,%d)\n", xsize, ysize);
  if( (pixFormat=arVideoGetPixelFormat()) < 0 ) exit(0);

  /* set the initial camera parameters */
  if( arParamLoad(CPARA_NAME, 1, &cparam) < 0 ) {
    ARLOGe("Camera parameter load error !!\n");
    exit(0);
  }
  arParamChangeSize( &cparam, xsize, ysize, &cparam );
  ARLOG("*** Camera Parameter ***\n");
  arParamDisp( &cparam );
  if ((gCparamLT = arParamLTCreate(&cparam, AR_PARAM_LT_DEFAULT_OFFSET)) == NULL) {
    ARLOGe("Error: arParamLTCreate.\n");
    exit(-1);
  }

  if( (arHandle=arCreateHandle(gCparamLT)) == NULL ) {
    ARLOGe("Error: arCreateHandle.\n");
    exit(0);
  }
  if( arSetPixelFormat(arHandle, pixFormat) < 0 ) {
    ARLOGe("Error: arSetPixelFormat.\n");
    exit(0);
  }

  if( (ar3DHandle=ar3DCreateHandle(&cparam)) == NULL ) {
    ARLOGe("Error: ar3DCreateHandle.\n");
    exit(0);
  }

  if( (arPattHandle=arPattCreateHandle()) == NULL ) {
    ARLOGe("Error: arPattCreateHandle.\n");
    exit(0);
  }
  arPattAttach( arHandle, arPattHandle );

  if( (config = arMultiReadConfigFile(configName, arPattHandle)) == NULL ) {
    ARLOGe("config data load error !!\n");
    exit(0);
  }
  if( config->patt_type == AR_MULTI_PATTERN_DETECTION_MODE_TEMPLATE ) {
    arSetPatternDetectionMode( arHandle, AR_TEMPLATE_MATCHING_COLOR );
  } else if( config->patt_type == AR_MULTI_PATTERN_DETECTION_MODE_MATRIX ) {
    arSetPatternDetectionMode( arHandle, AR_MATRIX_CODE_DETECTION );
  } else {
    arSetPatternDetectionMode( arHandle, AR_TEMPLATE_MATCHING_COLOR_AND_MATRIX );
  }

  /* open the graphics window */
  viewport.sx = 0;
  viewport.sy = 0;
  viewport.xsize = xsize;
  viewport.ysize = ysize;
  if( (vp=argCreateViewport(&viewport)) == NULL ) exit(0);
  argViewportSetCparam( vp, &cparam );
  argViewportSetPixFormat( vp, pixFormat );
}

/* cleanup function called when program exits */
static void cleanup(void )
{
  arParamLTFree(&gCparamLT);
  arVideoCapStop();
  arVideoClose();
  argCleanup();
}

static void draw( ARdouble trans1[3][4], ARdouble trans2[3][4], int mode )
{
  ARdouble  gl_para[16];
  GLfloat   light_position[]  = {100.0f, -200.0f, 200.0f, 0.0f};
  GLfloat   light_ambi[]      = {0.1f, 0.1f, 0.1f, 0.0f};
  GLfloat   light_color[]     = {1.0f, 1.0f, 1.0f, 0.0f};
  GLfloat   mat_flash[]       = {1.0f, 1.0f, 1.0f, 0.0f};
  GLfloat   mat_flash_shiny[] = {50.0f};
  GLfloat   mat_diffuse[]     = {0.0f, 0.0f, 1.0f, 1.0f};
  GLfloat   mat_diffuse1[]    = {1.0f, 0.0f, 0.0f, 1.0f};
  int       debugMode;

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  /* load the camera transformation matrix */
  glMatrixMode(GL_MODELVIEW);
  argConvGlpara(trans1, gl_para);
#ifdef ARDOUBLE_IS_FLOAT
  glLoadMatrixf( gl_para );
#else
  glLoadMatrixd( gl_para );
#endif
  argConvGlpara(trans2, gl_para);
#ifdef ARDOUBLE_IS_FLOAT
  glMultMatrixf( gl_para );
#else
  glMultMatrixd( gl_para );
#endif

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambi);
  glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_color);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_color);
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash);
  glMaterialfv(GL_FRONT, GL_SHININESS, mat_flash_shiny);
  if( mode == 0 ) {
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_diffuse);
  }
  else {
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse1);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_diffuse1);
  }
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  /* glPushMatrix(); */
  /*     glRotatef( 90.0, 0.0, 1.0, 0.0 ); */
  /*     glColor4f( 1.0, 0.0, 0.0, 1.0 ); */
  /*     glutSolidCone(5.0, 100.0, 20, 24); */
  /* glPopMatrix(); */

  /* glPushMatrix(); */
  /*     glRotatef( -90.0, 1.0, 0.0, 0.0 ); */
  /*     glColor4f( 0.0, 1.0, 0.0, 1.0 ); */
  /*     glutSolidCone(5.0, 100.0, 20, 24); */
  /* glPopMatrix(); */

  /* glPushMatrix(); */
  /*     glRotatef( 00.0, 0.0, 0.0, 1.0 ); */
  /*     glColor4f( 0.0, 0.0, 1.0, 1.0 ); */
  /*     glutSolidCone(5.0, 100.0, 20, 24); */
  /* glPopMatrix(); */


  glTranslatef( 0.0f, 0.0f, 20.0f );
  arGetDebugMode( arHandle, &debugMode );
  if( debugMode == 0 ) glutSolidCube(40.0);
  else                glutWireCube(40.0);
  glPopMatrix();




  glDisable( GL_LIGHT0 );
  glDisable( GL_LIGHTING );
  glDisable( GL_DEPTH_TEST );
}
#endif
