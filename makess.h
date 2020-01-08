/*
 * makess.h - header file for makess.c
 * makess.c - make spreadsheet from stat files
 *
 * Copyright (c) 2002 by David Wang and Bruce Jacob
 * David Wang, Bruce Jacob
 * Systems and Computer Architecture Lab
 * Dept of Electrical & Computer Engineering
 * University of Maryland, College Park
 * All Rights Reserved
 *
 * This software is distributed with *ABSOLUTELY NO SUPPORT* and
 * *NO WARRANTY*.  Permission is given to modify this code
 * as long as this notice is not removed.
 *
 * Send feedback to David Wang davewang@wam.umd.edu
 *               or Bruce Jacob blj@eng.umd.edu
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#define MAKESS_H     1

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#define 	TRUE			1
#define		FALSE 			0
#define		NORMAL_MODE		0
#define		DRAM_MODE		1

#define 	MAX_STAT_DEPTH		1000000
#define		MAX_INPUT_FILE_COUNT	100		/* no more than 100 input files */

typedef struct STATS {
	char	*header;				/* name of the input file */
	int 	delta_stat[MAX_STAT_DEPTH];
	int	dangling_stat;				/* read stuff into the array, whatever doesn't fit gets accumulated here. */
	double	total_count;
	double	count_times_latency;
	double 	average_latency;
} STATS;

void read_in_stat(STATS **, FILE *,int ,char *);
void print_stats(STATS **, FILE *,int);

int mode;
