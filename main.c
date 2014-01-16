#include <stdio.h>      // printf
#include <stdlib.h>     // exit
#include <getopt.h>     // getopt_long
#include <signal.h>
#include <math.h>
#include <time.h>
#include "main.h"

void trap(int signal){ execute = 0; }

char* format_time(const char* format_string) {

  char *outstr = malloc(sizeof(char) * 512);

  time_t t;
  struct tm *tmp;

  t = time(NULL);
  tmp = localtime(&t);
  if (tmp == NULL) {
    perror("localtime");
    exit(EXIT_FAILURE);
  }

  if (strftime(outstr, 512, format_string, tmp) == 0) {
    fprintf(stderr, "strftime returned 0");
    exit(EXIT_FAILURE);
  }

  return outstr;
}

void png_exit_on_error(int e) {
  if ( e < 0 ) {
    if ( e == PNG_FILE_ERROR )  printf("File error: PNG_FILE_ERROR\n");
    if ( e == PNG_HEADER_ERROR) printf("File error: PNG_HEADER_ERROR\n");
    if ( e == PNG_IO_ERROR) printf("File error: PNG_IO_ERROR\n");
    if ( e == PNG_EOF_ERROR) printf("File error: PNG_EOF_ERROR\n");
    if ( e == PNG_CRC_ERROR) printf("File error: PNG_CRC_ERROR\n");
    if ( e == PNG_MEMORY_ERROR) printf("File error: PNG_MEMORY_ERROR\n");
    if ( e == PNG_ZLIB_ERROR) printf("File error: PNG_ZLIB_ERROR\n");
    if ( e == PNG_UNKNOWN_FILTER) printf("File error: PNG_UNKNOWN_FILTER\n");
    if ( e == PNG_NOT_SUPPORTED) printf("File error: PNG_NOT_SUPPORTED\n");
    if ( e == PNG_WRONG_ARGUMENTS) printf("File error: PNG_WRONG_ARGUMENTS\n");
    exit(1);
  }
}

int open_png(png_file* file, const char* path) {

  int r;

  r = png_open_file(&file->png_obj, path);
  png_exit_on_error(r);

  file->data = (unsigned char*)malloc(file->png_obj.width*file->png_obj.height*file->png_obj.bpp);
  r = png_get_data(&file->png_obj, file->data);
  png_exit_on_error(r);

  file->is_open = 1;

}

void print_help() {
  printf("Surveill 0.0\n\n");

  printf("\t-h, --help\t\tThis help\n");

  printf("\nInput:\n");
  printf("\t-c, --current\t\tPath to current frame (png)\n");
  printf("\t-p, --previous\t\tPath to save previous frame (used for -d)\n");

  printf("\nOutput:\n");
  printf("\t-o, --out\t\tPath to save current frame to on detected motion\n");
  printf("\t-d, --diff\t\tPath to save cleaned frame to on changed pixels\n");

  printf("\nOptions:\n");
  printf("\t-s, --sensitivity\t\tIntensity treshold, a value between 0 and 255 -s\n");
  printf("\t-e, --passes\t\tPasses, the minimum radius of an object that should be counted as an object (in pixels) -s\n");
  printf("\t-t, --tune\t\tGet a suitable vaiable for -s\n");
  printf("\t-v, --verbose\t\tPrint interesting information along the way\n");
  printf("\t-u, --debug\t\tPrint even more interesting information along the way\n");
}

void parse_options(prog_options* options, int argc, char **argv) {

  // Initialize options config to null for comparison
  options->file_previous = NULL;
  options->file_current = NULL;
  options->file_out = NULL;
  options->file_out_difference = NULL;
  options->tune = 0;
  options->sensitivity = 25;
  options->passes = 5;
  options->verbose = 0;
  options->debug = 0;

  static struct option long_options[] = {
      {"previous", required_argument, 0, 'p'},
      {"current", required_argument, 0, 'c'},
      {"out", required_argument, 0, 'o'},
      {"diff", required_argument, 0, 'd'},
      {"help", no_argument, 0, 'h'},
      {"tune", no_argument, 0, 't'},
      {"sensitivity", required_argument, 0, 's'},
      {"passes", required_argument, 0, 'e'},
      {"verbose", no_argument, 0, 'v'},
      {"debug", no_argument, 0, 'u'},
      {0, 0, 0, 0}
  };
  int option_index = 0, c;
  if (argc > 0) {
    char *old_locale;
    char *saved_locale;
    while ((c = getopt_long(argc, argv, "p:c:o:d:hs:e:tvu", long_options, &option_index)) != -1) {
      switch (c) {
        case 'p':
          options->file_previous = optarg;
          break;
        case 'c':
          options->file_current = optarg;
          break;
        case 'o':
          options->file_out = optarg;
          break;
        case 'd':
          options->file_out_difference = optarg;
          break;
        case 'h':
          print_help();
          exit(0);
          break;
        case 't':
          options->tune = 1;
          break;
        case 'v':
          options->verbose = 1;
          break;
        case 'u':
          options->debug = 1;
          break;
        case 's':
          options->sensitivity = atoi(optarg);
          break;
        case 'e':
          // Temporarily switch locale
          options->passes = atoi(optarg);
          break;
        case '?':
          break;
        default:
          fprintf(stderr, "Unknown argument: 0%o\n",c);
          print_help();
          exit(1);
          break;
      }
    }
  } else {
    fprintf(stderr, "Unknown argument: %s\n",c);
    print_help();
    exit(1);
  }

}

void validate_options(prog_options* options) {

  if ( options->file_current == NULL || ( options->file_previous == NULL && options->file_out_difference != NULL )) {
    fprintf(stderr, "Invalid arguments: You need to specify -c (--current) and -p (--previous)\n");
    exit(1);
  }

  if ( options->file_out == NULL && options->file_out_difference == NULL && !options->tune ) {
    fprintf(stderr, "Info: No output -o (--out) or difference output -d (--diff) specified, no output will be written.\n");
    exit(1);
  }

}

void png_save_file(png_file* file, const char* path) {

  int r;

  png_t ps;
  r = png_open_file_write(&ps, path);
  png_exit_on_error(r);

  // ToDo: User selectable bitrate for output
  r = png_set_data(&ps, file->png_obj.width, file->png_obj.height, 8, file->png_obj.color_type, file->data);
  png_exit_on_error(r);

  // Close
  r = png_close_file(&ps);
  png_exit_on_error(r);

}

double get_timediff(struct timeval* tv1, struct timeval* tv2) {
  return (double) (tv2->tv_usec - tv1->tv_usec) / 1000000 + \
         (double) (tv2->tv_sec - tv1->tv_sec);
}
int main(int argc, char **argv)
{

  // Handle command line arguments
  prog_options p;
  parse_options(&p,argc,argv);
  validate_options(&p);

  // Return value
  int r;

  // Init library
  r = png_init(0, 0);

  // Read current image
  png_file pf_current;
  png_file pf_previous;

  // Find motion
  if ( p.file_out != NULL ) {

    // Run until signal trap is reached
    signal(SIGINT, &trap);
    execute = 1;
    while(execute){

      if(p.verbose || p.debug) fprintf(stdout,"\nNew round\n");
      if(p.verbose || p.debug) gettimeofday(&tv1_total, NULL);

      // Read current image
      if(p.debug) gettimeofday(&tv1, NULL);
      r = open_png(&pf_current, p.file_current);
      png_exit_on_error(r);

      // Read "previous" image
      if( pf_previous.is_open != 1 ) {
        r = open_png(&pf_previous, p.file_current);
        png_exit_on_error(r);
      }

      // Image instance from png
      motion_image image_current;
      image_from_png_data (&image_current, pf_current.data, pf_current.png_obj.width, pf_current.png_obj.height);
      motion_image image_previous;
      image_from_png_data (&image_previous, pf_previous.data, pf_current.png_obj.width, pf_current.png_obj.height);
      if(p.debug) gettimeofday(&tv2, NULL);
      if(p.debug) printf ("Open file:\t\t%f seconds\n", get_timediff(&tv1,&tv2));

      // Noise reduction
      if(p.debug) gettimeofday(&tv1, NULL);
      filter_noise_reduction (&(image_current.average),1);
      filter_noise_reduction (&(image_previous.average),1);
      if(p.debug) gettimeofday(&tv2, NULL);
      if(p.debug) printf ("Noise reduction:\t%f seconds\n", get_timediff(&tv1,&tv2));

      // Intensity correction
      if(p.debug) gettimeofday(&tv1, NULL);
      int intensity_current = get_average_intensity(&(image_current.average));
      int intensity_previous = get_average_intensity(&(image_previous.average));
      int intensity_difference_before = (intensity_current - intensity_previous) / 2;
      filter_adjust_intensity (&(image_current.average),-intensity_difference_before);
      filter_adjust_intensity (&(image_previous.average),intensity_difference_before);
      if(p.debug) gettimeofday(&tv2, NULL);
      if(p.debug) printf ("Intensity correction:\t%f seconds\n", get_timediff(&tv1,&tv2));

      // Background subtraction
      if(p.debug) gettimeofday(&tv1, NULL);
      filter_background_subtraction(&(image_current.average),&(image_previous.average));
      if(p.debug) gettimeofday(&tv2, NULL);
      if(p.debug) printf ("Background subtraction:\t%f seconds\n", get_timediff(&tv1,&tv2));

      // Binary split
      if(p.debug) gettimeofday(&tv1, NULL);
      filter_split_binary(&(image_current.average),p.sensitivity);
      if(p.debug) gettimeofday(&tv2, NULL);
      if(p.debug) printf ("Binary split:\t\t%f seconds\n", get_timediff(&tv1,&tv2));

      // Blob reduction
      if(p.debug) gettimeofday(&tv1, NULL);
      int pixels_left = filter_reduce_blobs(&(image_current.average),p.passes);
      if(p.debug) gettimeofday(&tv2, NULL);
      if(p.debug) printf ("Blob reduction:\t\t%f seconds\n", get_timediff(&tv1,&tv2));
      if(p.verbose || p.debug) printf ("Pixels left:\t\t%i/10\n", pixels_left);

      if( pixels_left >= 10 ) {
        // Save file
        if(p.debug) gettimeofday(&tv1, NULL);
        // ToDo: Delegate file saving to thread
        char * filename = format_time(p.file_out);
        png_save_file(&pf_current,format_time(p.file_out));
        free(filename);
        if(p.debug) gettimeofday(&tv2, NULL);
        if(p.debug) printf ("File save:\t\t%f seconds\n", get_timediff(&tv1,&tv2));
      }

      // png_close_file(&pf_current.png_obj); // pf_current is closed as pf_previous on signal
      png_close_file(&pf_previous.png_obj);
      free(pf_previous.data);

      pf_previous = pf_current;

      free_image(image_current);
      free_image(image_previous);

      if(p.verbose || p.debug) gettimeofday(&tv2_total, NULL);
      if(p.verbose || p.debug) printf ("Total round time:\t%f seconds\n", get_timediff(&tv1_total,&tv2_total));

    }

    if(p.verbose || p.debug) printf ("\nExecution interrupted, cleaning up...\n");

    r = png_close_file(&pf_previous.png_obj);
    free(pf_previous.data);

    signal(SIGINT, SIG_DFL);
  }

  // Find difference
  if ( p.file_out_difference != NULL ) {
    int change_treshold = 10;
    int x, y;
    for( x = 0 ; x < pf_current.png_obj.width ; x++ ) {
    for( y = 0 ; y < pf_current.png_obj.height ; y++ ) {

    int pixel_idx = (pf_current.png_obj.width*y*4+x*4);

      // Check for changes on pixel level
      int avg_new = (pf_current.data[pixel_idx]+pf_current.data[pixel_idx+1]+pf_current.data[pixel_idx+2])/3;
      int avg_old = (pf_previous.data[pixel_idx]+pf_previous.data[pixel_idx+1]+pf_previous.data[pixel_idx+2])/3;
      int difference = (avg_new - avg_old);
      if(!(
    abs(pf_current.data[pixel_idx]-pf_previous.data[pixel_idx]) > change_treshold ||
    abs(pf_current.data[pixel_idx+1]-pf_previous.data[pixel_idx+1]) > change_treshold ||
    abs(pf_current.data[pixel_idx+2]-pf_previous.data[pixel_idx+2]) > change_treshold ||
    abs(difference) > change_treshold
    ))
    {
      pf_current.data[pixel_idx] = 0;
      pf_current.data[pixel_idx+1] = 0;
      pf_current.data[pixel_idx+2] = 0;
      pf_current.data[pixel_idx+3] = 0;
    }
    }
    }

    // Save difference
    png_save_file(&pf_current,p.file_out_difference);

    // Output file old
    r = png_close_file(&pf_current.png_obj);
    r = png_close_file(&pf_previous.png_obj);
    free(pf_current.data);
    free(pf_previous.data);

  }
  
  return 0;
  
}
