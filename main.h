#ifndef _MAIN_H_
#define _MAIN_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <sys/time.h>
#include "pnglite.h"
#include "motion.h"

typedef struct t_prog_options {
	
	const char* file_current;
	const char* file_previous;
	const char* file_out;
	const char* file_out_difference;

    int tune;
    int sensitivity;
	int passes;
	int verbose;

} prog_options;

typedef struct t_png_file {
	png_t png_obj;
	unsigned char* data;
	int is_open;
} png_file;

struct timeval  tv1, tv2, tv1_total, tv2_total;

static volatile int execute;

#ifdef __cplusplus
}
#endif

#endif