#ifndef _MOTION_H_
#define _MOTION_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <stdlib.h>

typedef struct t_motion_image motion_image;

typedef struct t_motion_channel {
  motion_image* image;
  unsigned char *data;
} motion_channel;

typedef struct t_motion_image {
  unsigned int width;
  unsigned int height;
  motion_channel red;
  motion_channel green;
  motion_channel blue;
  motion_channel average;
} motion_image;

void free_channel(motion_channel channel);
void free_image(motion_image image);

void filter_noise_reduction(motion_channel* channel, int intensity);
void filter_adjust_intensity(motion_channel* channel, int intensity);
void filter_background_subtraction(motion_channel* dest_channel, motion_channel* ref_channel);
void filter_split_binary(motion_channel* channel, int intensity);
int filter_reduce_blobs(motion_channel* channel, int passes);

void image_from_png_data (motion_image* image, unsigned char* data, int width, int height);
void image_to_png_data(motion_image* image,unsigned char* png_data);

unsigned char get_average_intensity(motion_channel* channel);

#ifdef __cplusplus
}
#endif

#endif
