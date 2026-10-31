#ifndef VAST_CAMERA_H
#define VAST_CAMERA_H

#include "vast_types.h"
#include "vast_math_linear_algebra.h"

/* #############################################################################
 * # [SECTION] Basic Camera
 * #############################################################################
 */
typedef struct vast_camera
{
  vast_vec3 position;
  vast_vec3 forward;
  vast_vec3 up;
  vast_vec3 right;
  vast_vec3 worldUp;

  f32 yaw;
  f32 pitch;
  f32 fov;
  f32 roll;

} vast_camera;

VAST_API void vast_camera_update(vast_camera *cam)
{
  f32 yawRad = VAST_DEG_TO_RAD(cam->yaw);
  f32 pitchRad = VAST_DEG_TO_RAD(cam->pitch);
  f32 rollRad = VAST_DEG_TO_RAD(cam->roll);

  f32 cy = vast_cosf(yawRad);
  f32 sy = vast_sinf(yawRad);

  f32 cp = vast_cosf(pitchRad);
  f32 sp = vast_sinf(pitchRad);

  f32 cr = vast_cosf(rollRad);
  f32 sr = vast_sinf(rollRad);

  vast_vec3 right;
  vast_vec3 up;

  cam->forward.x = cy * cp;
  cam->forward.y = sp;
  cam->forward.z = sy * cp;

  right.x = -sy;
  right.y = 0.0f;
  right.z = cy;

  up.x = -cy * sp;
  up.y = cp;
  up.z = -sy * sp;

  /* Apply roll */
  cam->right.x = right.x * cr - up.x * sr;
  cam->right.y = right.y * cr - up.y * sr;
  cam->right.z = right.z * cr - up.z * sr;

  cam->up.x = right.x * sr + up.x * cr;
  cam->up.y = right.y * sr + up.y * cr;
  cam->up.z = right.z * sr + up.z * cr;
}

VAST_API VAST_INLINE vast_camera vast_camera_init(void)
{
  vast_camera cam = {0};

  cam.position = vast_vec3_zero;
  cam.forward = vast_vec3_init(0.0f, 0.0f, -1.0f);
  cam.up = vast_vec3_init(0.0f, 1.0f, 0.0f);
  cam.worldUp = cam.up;
  cam.fov = 90.0f;
  cam.yaw = -90.0f;

  vast_camera_update(&cam);

  return cam;
}

#endif /* VAST_CAMERA_H */