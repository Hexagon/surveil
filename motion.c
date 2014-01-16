#include <stdio.h>
#include <string.h>
#include "motion.h"


void free_channel(motion_channel channel) {
  free(channel.data);
}

void free_image(motion_image image) {
  free_channel(image.red);
  free_channel(image.green);
  free_channel(image.blue);
  free_channel(image.average);
}

/* data is a pointer to an unsigned char array with rgbargbargba... */
void image_from_png_data (motion_image* image, unsigned char* data, int width, int height) {

  // Store width and height
  image->width = width;
  image->height = width;

  // Prepare the channels
  motion_channel red;
  motion_channel green;
  motion_channel blue;
  motion_channel average;

  // Pre-calculate data length
  unsigned int channel_len = width*height;
  unsigned int data_len = channel_len*4;

  // Allocate memory
  red.data = (unsigned char*)malloc(channel_len);
  green.data = (unsigned char*)malloc(channel_len);
  blue.data = (unsigned char*)malloc(channel_len);
  average.data = (unsigned char*)malloc(channel_len);

  // Deep copy the image data
  unsigned int pixel_idx,pixel_mult;
  for( pixel_idx = 0 ; pixel_idx < channel_len; pixel_idx++ ) {
    pixel_mult = pixel_idx*4;
    red.data[pixel_idx]     = data[pixel_mult  ];
    green.data[pixel_idx]   = data[pixel_mult+1];
    red.data[pixel_idx]     = data[pixel_mult+2];
    average.data[pixel_idx] = (data[pixel_mult  ]+data[pixel_mult+1]+data[pixel_mult+2])/3;
  }

  // Populate instance of motion_image
  image->width = width;
  image->height = height;

  image->red = red;
  image->green = green;
  image->blue = blue;
  image->average = average;

  image->red.image = image;
  image->green.image = image;
  image->blue.image = image;
  image->average.image = image;

}

void image_to_png_data(motion_image* image,unsigned char* png_data) {
  int data_len = image->width*image->height,x,pixel_mult;
  for( x = 0 ; x < data_len ; x++ ) {
    pixel_mult = x*4;
    png_data[pixel_mult  ] = image->average.data[x];
    png_data[pixel_mult+1] = image->average.data[x];
    png_data[pixel_mult+2] = image->average.data[x];
    png_data[pixel_mult+3] = 255;
  }
}

/* intensity tells the filter how big radius will be used for noise reduction, in pixels. */
void filter_noise_reduction(motion_channel* channel, int intensity) {

  // Allocate memory for new channel
  motion_channel new_channel;
  new_channel.image = channel->image;

  new_channel.data = (unsigned char*)malloc(new_channel.image->width*new_channel.image->height);

  // Apply filter
  unsigned int x,y,pixel_idx,temp_value,row,counter;
  for( y = 0 ; y < new_channel.image->height ; y++ ) {
    row = (channel->image)->width*y;
    for( x = 0 ; x < new_channel.image->width ; x++ ) {
      pixel_idx = row+x;
      temp_value = 0;
      counter = 1;
      temp_value += channel->data[pixel_idx];
      if(x < new_channel.image->width -1 ) {
        temp_value += channel->data[pixel_idx+1];
        counter++;
      }
      if(x > 0 ) {
        temp_value += channel->data[pixel_idx-1];
        counter++;
      }
      if(y > 0 ) {
        temp_value += channel->data[(channel->image)->width*(y-1)+x];
        counter++;
      }
      if(y < new_channel.image->height -1 ) {
        temp_value += channel->data[(channel->image)->width*(y+1)+x];
        counter++;
      }
      new_channel.data[pixel_idx] = temp_value/counter;
    }
  }

  // Remove the old channel and replace it with the filtered one
  free(channel->data);
  channel->data = new_channel.data;

}

unsigned char get_average_intensity(motion_channel* channel) {

  // Store intensity in alpha channel
  motion_image *image = channel->image;
  unsigned int pixel_idx;
  unsigned int data_len = image->width*image->height;
  unsigned int total_intensity=0;
  for( pixel_idx = 0 ; pixel_idx < data_len; pixel_idx++ ) {
    total_intensity += channel->data[pixel_idx];
  }
  return total_intensity / data_len;

}

void filter_adjust_intensity(motion_channel* channel, int intensity) {

  // Allocate memory for new channel
  motion_channel new_channel;
  new_channel.image = channel->image;
  new_channel.data = malloc(new_channel.image->width*new_channel.image->height);

  // Apply filter
  int temp_value;
  unsigned int data_len = new_channel.image->width*new_channel.image->height,pixel_idx;
  for( pixel_idx = 0; pixel_idx < data_len; pixel_idx++ ) {
    temp_value = channel->data[pixel_idx]+intensity;
    if( temp_value > 255 ) temp_value = 255;
    if( temp_value < 0 ) temp_value = 0;
    new_channel.data[pixel_idx] = temp_value;
  }

  // Remove the old channel and replace it with the filtered one
  free(channel->data);
  channel->data = new_channel.data;

}

void filter_background_subtraction(motion_channel* dest_channel, motion_channel* ref_channel) {

  // Allocate memory for new channel
  motion_channel new_channel;
  new_channel.image = dest_channel->image;

  new_channel.data = malloc(new_channel.image->width*new_channel.image->height);

  // Apply filter
  unsigned int data_len = new_channel.image->width*new_channel.image->height,pixel_idx;
  for( pixel_idx = 0; pixel_idx < data_len; pixel_idx++ ) {
    new_channel.data[pixel_idx] = abs((int)dest_channel->data[pixel_idx]-(int)ref_channel->data[pixel_idx]);
  }

  // Remove the old channel and replace it with the filtered one
  free(dest_channel->data);
  dest_channel->data = new_channel.data;
}

void filter_split_binary(motion_channel* channel, int intensity) {
  unsigned int data_len = (channel->image)->width*(channel->image)->height,pixel_idx;
  for( pixel_idx = 0; pixel_idx < data_len; pixel_idx++ ) {
    if(channel->data[pixel_idx]<intensity) {
      channel->data[pixel_idx] = 0;  
    } else {
      channel->data[pixel_idx] = 255;
    }
  }
}

// Give passes a positive number to run a pre-defined number of passes, -1 runs until no white pixels are left and return number of passes
int filter_reduce_blobs(motion_channel* channel, int passes) {
  int pixels_left=1,pass_counter=0;

  while(passes>0 || (passes==-1 && pixels_left>0) ) {
    pixels_left=0;

    // Allocate memory for new channel
    motion_channel new_channel;
    new_channel.image = channel->image;
    new_channel.data = malloc(new_channel.image->width*new_channel.image->height);

    // 1. Transfer data
    //memcpy ( channel->data, new_channel.data, new_channel.image->width*new_channel.image->height );
    unsigned int x,y,pixel_idx,whites,row;

    for( y = 0 ; y < new_channel.image->height ; y++ ) {
      row = (channel->image)->width*y;
      for( x = 0 ; x < new_channel.image->width ; x++ ) {
        pixel_idx = row+x;
        new_channel.data[pixel_idx] = channel->data[pixel_idx];
        if( channel->data[pixel_idx]==255 ) pixels_left++;
      }
    }

    // 2. Expand black
    if( pixels_left > 0 ) {
      for( y = 1 ; y < new_channel.image->height -1 ; y++ ) {
        row = (channel->image)->width*y;
        for( x = 1 ; x < new_channel.image->width -1 ; x++ ) {
          pixel_idx=row+x;
          if(channel->data[pixel_idx]==0) {
            new_channel.data[pixel_idx+1] = 0;
            new_channel.data[pixel_idx-1] = 0;
            new_channel.data[(channel->image)->width*(y-1)+x] = 0;
            new_channel.data[(channel->image)->width*(y+1)+x] = 0;
          }
        }
      }
    }

    // Remove the old channel and replace it with the filtered one
    free(channel->data);
    channel->data = new_channel.data;

    passes--;
    pass_counter++;
  }
  return pixels_left;
}
