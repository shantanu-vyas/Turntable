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


class TurnTable
{
public:
  TurnTable();
  static void init(int argc, char *argv[]);
  static void cleanup(void);
  static void mainLoop(void);
  static void draw( ARdouble trans1[3][4], ARdouble trans2[3][4], int mode );
  static void keyEvent( unsigned char key, int x, int y);
  //static void updateMat(cv::Mat input);
  static void setup();
};
#endif
