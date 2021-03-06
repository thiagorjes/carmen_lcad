#ifndef DRAW_CAR_H_
#define DRAW_CAR_H_

#include <carmen/carmen.h>
#include <carmen/glm.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/glu.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct CarDrawer CarDrawer;

CarDrawer* createCarDrawer(int argc, char** argv);
void destroyCarDrawer(CarDrawer* carDrawer);

void draw_car(CarDrawer* carDrawer);
void draw_car_at_pose(CarDrawer* carDrawer, carmen_pose_3D_t position);

#ifdef __cplusplus
}
#endif

#endif
