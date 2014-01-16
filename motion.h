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
void image_to_png_data(motion_image image,unsigned char* png_data);

unsigned char get_average_intensity(motion_channel* channel);

/*
        // Smudge the new intensity channel
        for( x = 1 ; x < pf_current.png_obj.width - 1; x++ ) {
        for( y = 1 ; y < pf_current.png_obj.height - 1; y++ ) {

           int pixel_idx = (pf_current.png_obj.width*y*4+x*4);
           int cur_corrected = (pf_current.data[pixel_idx+3] - avg_intensity_diff);
           int prev_corrected = (pf_previous.data[pixel_idx+3] + avg_intensity_diff);

           cur_corrected = (cur_corrected > 255) ? 255 : cur_corrected;
           prev_corrected = (prev_corrected > 255) ? 255 : prev_corrected;
           cur_corrected = (cur_corrected < 0) ? 0 : cur_corrected;
           prev_corrected = (prev_corrected < 0) ? 0 : prev_corrected;

           if( abs( cur_corrected - prev_corrected ) > p.sensitivity ) {
               pixels_changed+=1;
           }

           pf_current.data[pixel_idx+3] = 255;
        }
        }

        pixels_changed /= (pf_current.png_obj.width*pf_current.png_obj.height/100);

        printf("Percent of changed interesting pixels is: %f\n",pixels_changed);


}*/

#ifdef __cplusplus
}
#endif

#endif