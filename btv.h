#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<string.h>
#include 	<sys/time.h>
#include 	<math.h>
#include 	<tk.h>
#define 	MIN(a,b) 		((a) < (b) ? (a) : (b))
#define 	MAX(a,b) 		((a) > (b) ? (a) : (b))
#define 	ABS(a) 			((a) > 0 ? (a) : (-a))

#define 	DEF_XSCROLL_COMMAND        ""
#define 	DEF_YSCROLL_COMMAND        ""

#define 	INT_LIMIT		2147483647
#define 	MAX_NAME_LENGTH		100
#define 	MAX_SEGMENT_SIZE	100000
#define		MAX_SEGMENT_COUNT	1000
#define 	EPSILON			1.1
#define 	MAX_THREAD_COUNT	4

#define		BTV_OK			1
#define		BTV_ERROR		-1
#define		TRUE			1
#define		FALSE			0
#define		INVALID			-1

#define		IDLE			-1
#define		ACTIVE			4

#define		PRINT_SUMMARY		1
#define		PRINT_DETAILS		2
#define		PRINT_ALL		3
#define		DRAM_ANALYSIS		4
#define		OPEN_PAGE_ANALYSIS	5
#define		PRINT_IDLING_DISTANCE	6
#define		PRINT_SCHEDULING_TABLE	7
#define		PRINT_ACCESS_DISTANCE	8
#define		PRINT_BANDWIDTH		9

#define		PERCENT_SIGN		37

#define		DISPLAY_MODE_COUNT	7
#define		MAIN_MODE		0
#define		SHOW_LABELS		1
#define		SHOW_LEGENDS		2
#define 	SHOW_THREAD_NUMBER	3
#define		HASH_MODE		4
#define		COLOR_SCHEME		5
#define		SHOW_BW_LIMIT		6

#define		ALL_THREADS		-1

#define		HASH_TO_ZERO		0
#define		FLOAT			1

#define		HISTOGRAM_MODE		0
#define		INTERARRIVAL_MODE	1

#define		COLOR_BY_ACCESS_TYPE	0
#define		COLOR_BY_THREAD		1

#define 	SUPRESS_TEXT		2

#define 	TIME_STAMP		0
#define		EVENT			1
#define		INTERARRIVAL_TIME	2
#define		ADDRESS			3
#define		ADDRESS_DIFFERENTIAL	4
#define		HISTOGRAM_VIEW		5

#define		NO_TRACE		0
#define		K6_TRACE		1
#define		TM_TRACE		2
#define  	SS_TRACE		3
#define		MASE_TRACE		4

#define 	CQ_FCFS  		0
#define 	CQ_IFETCH 		1
#define 	SQ_LFF 			2
#define 	SQ_RR 			3

#define 	BTV_GRAY_PALLET_SIZE	100

#define 	COLOR_MODE_COUNT	20
#define		EVENT_TYPE_COUNT	9
#define		STAT_EVENT_TYPE_COUNT	2
	
#define 	MEM_RD			0
#define 	MEM_WR			1
#define 	FETCH			2
#define 	IO_RD			3
#define 	IO_WR			4
#define 	BOFF			5
#define 	LOCK_RD			6
#define 	LOCK_WR			7
#define 	INT_ACK			8

#define		HIGHLIGHT		10
#define		FOREGROUND		11
#define		BACKGROUND		12
#define		GRID			13

#define		PICOSECOND		1
#define		NANOSECOND		2
#define		MICROSECOND		3
#define		MILLISECOND		4
#define		SECOND			5

#define	BTV_COLOR_PALLET_SIZE		16
#define BTV_Black                       0
#define BTV_DarkBlue                    1
#define BTV_Blue                        2
#define BTV_Cyan                        3
#define BTV_DarkGreen                   4
#define BTV_Green                       5
#define BTV_Yellow                      6
#define BTV_Orange                      7
#define BTV_Red                         8
#define BTV_Pink                        9
#define BTV_Magenta                     10
#define BTV_Purple                      11
#define BTV_Brown                       12
#define BTV_Gray                        13
#define BTV_LightGray                   14
#define BTV_White                       15

/* mem system # defines */

#define tick_t 				long long 

#define MAX_CPU_FREQUENCY		1000000		/* one teraherz */
#define MIN_CPU_FREQUENCY		10		/* ten MHz */
#define MAX_DRAM_FREQUENCY		1000000		/* one teraherz */
#define MIN_DRAM_FREQUENCY		10		/* ten MHz */
#define MIN_CHANNEL_WIDTH		2		/* narrowest allowable DRAM channel assumed to be 2 bytes */
#define MAX_CHANNEL_COUNT		4		/* max number of logical memory channels */
#define	MAX_RANK_COUNT			32		/* Number of ranks on a channel. A rank is a device-bank */
#define	MAX_BANK_COUNT			32		/* number of banks per rank (chip) */
#define MIN_CACHE_LINE_SIZE		32		/* min cacheline length assumed to be 32 bytes */
#define MAX_TRANSACTION_QUEUE_DEPTH	8		/* each bank has a transaction queue in front of it */

#define	UNKNOWN				-1
#define SDRAM				0
#define	DDRSDRAM			1
#define	DRDRAM				2
#define DDR2				3

/* This section defines the physical address mapping policy */

#define	OPEN_PAGE_BASELINE		0
#define	CLOSE_PAGE_BASELINE		1
#define	OPEN_PAGE_EXPANDABLE		2
#define	CLOSE_PAGE_EXPANDABLE		3
#define	OPEN_PAGE_PERMUTE		4
#define	CLOSE_PAGE_PERMUTE		5
#define	OPEN_PAGE_EXPANDABLE_PERMUTE	6
#define	CLOSE_PAGE_EXPANDABLE_PERMUTE	7

/* This section defines the virtual address mapping policy */
#define VA_EQUATE			0			/* physical == virtual */
#define VA_RANDOM			1

#define STRICT_ORDER			0
#define ROUND_ROBIN			1
#define FULL_REORDER			1

#define NO_REFRESH			0
#define BANK_CONCURRENT			1
#define BANK_STAGGERED_HIDDEN		2

/* This section defines the row buffer management policy */

#define OPEN_PAGE               	0
#define CLOSE_PAGE              	1

/* This section used for statistics gathering */

#define	GATHER_BUS_STAT			0
#define	GATHER_BANK_HIT_STAT		1
#define	GATHER_BANK_CONFLICT_STAT	2
#define	GATHER_CAS_PER_RAS_STAT		3
#define GATHER_REQUEST_LOCALITY_STAT	4
#define	GATHER_BUS_TRACE		5
#define CMS_MAX_LOCALITY_RANGE		257             /* 256 + 1 */

/* end of mem system # defines */

/* file io #defines */

#define 	EOL 	10
#define 	CR 	13
#define 	SPACE	32
#define		TAB	9

/* file io #defines */

/* stat collection # defines */

#define		MAX_CHANNEL_STATE	2		/* same or different */
#define		MAX_RANK_STATE		3		/* same, different, or don't care */
#define		MAX_BANK_STATE		3		/* same, different, or don't care */
#define		MAX_ROW_STATE		4		/* open, closed, conflict or don't care */
#define		SAME			0	
#define		DIFFERENT		1
#define		DONTCARE		2
#define		OPEN			0
#define		CLOSED			1
#define		CONFLICT		3
#define		MAX_ACCESS_DISTANCE_RANGE	33	/* larger than 33, and we don't care */
#define		IS_QUEUE_FULL		0
#define		IS_QUEUE_EMPTY		1

/* end of stat collection #defines */

typedef struct BUS_Event {   /* 6 DWord per event */
	int 	attributes;	/* read/write/Fetch type stuff.  Not overloaded with other stuff */
	int	address;	/* assume to be <= 32 bits for now */
	double	timestamp;	/* time stamp will now have a large dynamic range, but only 53 bit precision */
} BUS_Event;

typedef struct TRACE_Segment {  /* 2.4 MB per 100000 event */   
	BUS_Event 	event[MAX_SEGMENT_SIZE];
} TRACE_Segment ;

typedef struct SS_BUS_Event { /* 3 DWords per event */
	int	mem_load_latency_sum;
	int	mem_load_count;
	int	mem_store_count;
} SS_BUS_Event;

typedef struct SS_TRACE_Segment {  /* 1.6 MB per 100000 event */   
	SS_BUS_Event 	event[MAX_THREAD_COUNT][MAX_SEGMENT_SIZE];
} SS_TRACE_Segment ;

typedef struct TRACE {
	int		trace_type;	/* what type of trace is this ? */
	int		policy;		/* scheduling policy */
	double		start_time;
	double 		end_time;
	TRACE_Segment 	*segment[MAX_SEGMENT_COUNT];
	int		segment_count;
	int		event_count;	
	/*						
	SS_TRACE_Segment *ss_segment[MAX_SEGMENT_COUNT];	disabled for now  
	*/
} TRACE;

typedef struct GEOMETRY {
	int	origin_x;
	int	origin_y;
	int	win_height;
	int	win_width;
	int	offset_x;
	int	offset_y;
	int	mark_x;
	int 	mark_y;
	int	x_axis_scale;
	int	y_axis_scale;
	double	first_time;
	int	first_event;
} GEOMETRY;

typedef struct STAT_BUCKET {
	int			count;
	double			delta_t;
	struct	STAT_BUCKET 	*next_bucket;
} STAT_BUCKET;

typedef struct STATS {
	int		start_no;
	int		end_no;
	double		start_time;
	double		end_time;
	int		valid_transaction_count;
	int		event_count[STAT_EVENT_TYPE_COUNT];
	double		last_time[EVENT_TYPE_COUNT];
	STAT_BUCKET	*sequential[STAT_EVENT_TYPE_COUNT][STAT_EVENT_TYPE_COUNT];
	STAT_BUCKET	*same[STAT_EVENT_TYPE_COUNT];
	int		count[STAT_EVENT_TYPE_COUNT][STAT_EVENT_TYPE_COUNT][MAX_CHANNEL_STATE][MAX_RANK_STATE][MAX_BANK_STATE][MAX_ROW_STATE];
	int		access_distance[MAX_ACCESS_DISTANCE_RANGE];
	int		idling_distance[MAX_ACCESS_DISTANCE_RANGE];
	int		idling_distances_history[MAX_ACCESS_DISTANCE_RANGE];   	/* for j - 1 to j - MAX_ACCESS_DISTANCE_RANGE -1 */
									 	/* keep track of access_distance history */
} STATS;

static char	*event_name[] = {
 "MEM_RD ",                /* 1 */
 "MEM_WR ",                /* 2 */
 "FETCH  ",                /* 0 */
 "IO_RD  ",                /* 3 */
 "IO_WR  ",                /* 4 */
 "BOFF   ",                /* 5 */
 "LOCK_RD",                /* 6 */
 "LOCK_WR",                /* 7 */
 "INT_ACK",                /* 8 */
 "UNKNOWN"                 /* 9 */
};

static char	*color_map[] = {
 "black",                       /* 0 */
 "MidnightBlue",                /* 1 */
 "blue",                        /* 2 */
 "cyan",                        /* 3 */
 "darkgreen",                   /* 4 */
 "green",                       /* 5 */
 "yellow",                      /* 6 */
 "orange",                      /* 7 */
 "red",                         /* 8 */
 "pink",                        /* 9 */
 "magenta",                     /* 10 */
 "purple",                      /* 11 */
 "brown",                       /* 12 */
 "gray",                        /* 13 */
 "lightgray",                   /* 14 */
 "white",                        /* 15 */
"red1", /* 460 */ 
"red2", /* 12 */ 
"firebrick", /* 218 */ 
"brown4", /* 39 */ 
"DarkRed" /* 387 */ 
"green1", /* 251 */ 
"green2", /* 453 */ 
"green3", /* 205 */ 
"green4", /* 160 */ 
"DarkGreen" /* 17 */ 
"blue1", /* 19 */ 
"MediumBlue", /* 373 */ 
"NavyBlue", /* 81 */ 
"DarkBlue", /* 125 */ 
"MidnightBlue" /* 495 */
};

static char	*gray_map[] = {
"gray0", /* 239 */ 
"gray1", /* 198 */ 
"gray2", /* 192 */ 
"gray3", /* 306 */ 
"gray4", /* 46 */ 
"gray5", /* 419 */ 
"gray6", /* 178 */ 
"gray7", /* 367 */ 
"gray8", /* 551 */ 
"gray9", /* 64 */ 
"gray10", /* 89 */ 
"gray11", /* 240 */ 
"gray12", /* 193 */ 
"gray13", /* 263 */ 
"gray14", /* 150 */ 
"gray15", /* 241 */ 
"gray16", /* 196 */ 
"gray17", /* 265 */ 
"gray18", /* 151 */ 
"gray19", /* 242 */ 
"gray20", /* 267 */ 
"gray21", /* 153 */ 
"gray22", /* 133 */ 
"gray23", /* 454 */ 
"gray24", /* 394 */ 
"gray25", /* 413 */ 
"gray26", /* 426 */ 
"gray27", /* 270 */ 
"gray28", /* 246 */ 
"gray29", /* 552 */ 
"gray30", /* 520 */ 
"gray31", /* 521 */ 
"gray32", /* 305 */ 
"gray33", /* 518 */ 
"gray34", /* 307 */ 
"gray35", /* 308 */ 
"gray36", /* 365 */ 
"gray37", /* 364 */ 
"gray38", /* 45 */ 
"gray39", /* 309 */ 
"gray40", /* 27 */ 
"gray41", /* 523 */ 
"gray42", /* 519 */ 
"gray43", /* 507 */ 
"gray44", /* 5 */ 
"gray45", /* 508 */ 
"gray46", /* 21 */ 
"gray47", /* 310 */ 
"gray48", /* 509 */ 
"gray49", /* 389 */ 
"gray50", /* 440 */ 
"gray51", /* 116 */ 
"gray52", /* 522 */ 
"gray53", /* 114 */ 
"gray54", /* 407 */ 
"gray55", /* 311 */ 
"gray56", /* 93 */ 
"gray57", /* 62 */ 
"gray58", /* 411 */ 
"gray59", /* 16 */ 
"gray60", /* 6 */ 
"gray61", /* 180 */ 
"gray62", /* 532 */ 
"gray63", /* 506 */ 
"gray64", /* 425 */ 
"gray65", /* 59 */ 
"gray66", /* 381 */ 
"gray67", /* 313 */ 
"gray68", /* 42 */ 
"gray69", /* 88 */ 
"gray70", /* 312 */ 
"gray71", /* 439 */ 
"gray72", /* 319 */ 
"gray73", /* 320 */ 
"gray74", /* 314 */ 
"gray75", /* 391 */ 
"gray76", /* 386 */ 
"gray77", /* 433 */ 
"gray78", /* 502 */ 
"gray79", /* 223 */ 
"gray80", /* 183 */ 
"gray81", /* 315 */ 
"gray82", /* 475 */ 
"gray83", /* 456 */ 
"gray84", /* 316 */ 
"gray85", /* 318 */ 
"gray86", /* 410 */ 
"gray87", /* 452 */ 
"gray88", /* 489 */ 
"gray89", /* 501 */ 
"gray90", /* 162 */ 
"gray91", /* 530 */ 
"gray92", /* 317 */ 
"gray93", /* 450 */ 
"gray94", /* 490 */ 
"gray95", /* 377 */ 
"gray96", /* 468 */ 
"gray97", /* 247 */ 
"gray98", /* 135 */ 
"gray99", /* 197 */ 
"gray100"
};

/***********************************************************************
 Here are the data structures for the memory system.
 ***********************************************************************/

typedef struct addresses_t {
	unsigned int	virtual_address;	
	unsigned int	physical_address;
	int		chan_id;		/* logical channel id */
	int		rank_id;		/* device id */
	int		bank_id;
	int		row_id;
	int		col_id;			/* column address */
} addresses_t;

typedef struct bank_t {
	int		status;			/* IDLE, ACTIVE */
	int		row_id;			/* if the bank is open to a certain row, what is the row id? */
	int		age;			/* when was it accessed last? */
} bank_t;

typedef struct rank_t {
	bank_t		bank[MAX_BANK_COUNT];
} rank_t;

typedef struct transaction_t {
	int		event_no;
	int		status;
	tick_t		arrival_time;		/* time when first seen by the memory controller in DRAM clock ticks */
	tick_t		completion_time;	/* time when entire transaction has completed in DRAM clock ticks */
	int		type;			/* command type */
	addresses_t	addr;
} transaction_t;

typedef struct transaction_queue_t {
	int		transaction_count;
	transaction_t 	entry[MAX_TRANSACTION_QUEUE_DEPTH];	/* FIFO queue inside of controller */
} transaction_queue_t;

typedef struct dram_controller_t {
	int		id;
	rank_t 		rank[MAX_RANK_COUNT];  		/* device in RDRAM system, device-bank in SDRAM system */
	int		refresh_row_index;		/* the row index to be refreshed */
	tick_t		last_refresh_cycle;		/* tells me when last refresh was done */
}dram_controller_t;

typedef struct dram_states_t {
	int             chan_state;
	int		rank_state;
	int		bank_state;
	int		row_state;
} dram_states_t;

/* 
 *	Contains all of the configuration information.  
 */

typedef struct dram_system_configuration_t {
	
	int		dram_type;
	int		data_rate;
        int             row_buffer_management_policy;   /* which row buffer management policy? OPEN PAGE, CLOSE PAGE, etc */
	int		va_mapping_policy;		/* equate or random ? */
	int		pa_mapping_policy;		/* which address mapping policy for Physical to memory? */
	int		command_ordering_policy;	/* strict or not strict? */
	int		transaction_ordering_policy;	/* strict or round robin? */
	int		refresh_policy;			/*  */

	int		chan_count;			/* How many logical channels (memory controllers) are there ? */
	int 		rank_count;			/* How many ranks are there per channel ? */
	int		bank_count;			/* How many banks per device? */
	int		row_count;			/* how many rows per bank? */
	int		cachelines_per_row;

	int		col_count;			/* Hwo many columns per row? */
	int		col_size;
	int		row_size;			/* how many bytes per row? (across all devices in single rank) */
	int		cacheline_size;			/* probably 64 */
	int		TLB_pagesize;
	int		transaction_queue_depth;	/* how deep? per bank */
							/* we also ignore t_pp (min prec to prec of any bank) and t_rr (min ras to ras).   */
	int		t_ras;				/* interval between ACT and PRECHARGE to same bank */
	int		t_rcd;				/* RAS to CAS delay of same bank */
	int		t_cas;				/* delay between start of CAS command and start of data burst */
	int		t_rp;				/* interval between PRECHARGE and ACT to same bank */
							/* t_rc is simply t_ras + t_rp */
	int		t_rc;
	int		t_cwd;				/* delay between end of CAS Write command and start of data packet */

	int		t_al;				/* additive latency = t_rcd - 2 (ddr half cycles) */
	int		t_rl;				/* read latency  = t_al + t_cas */

	int		t_dqs;				/* rank hand off penalty. 0 for SDRAM, 2 for DDR, and 0 for DRDRAM */
	int		t_burst;			/* */
	int		d_m;				/* minimum distance. equal to t_RC/t_burst -1 */
	int		max_open_bank_count;		/* for things like tFAW */

} dram_system_configuration_t;

/*
 *  	DRAM System now has the configuration component, the transaction queue and the dram controller
 */

typedef struct dram_system_t {
	dram_system_configuration_t 	config;
	transaction_queue_t     	transaction_queue[MAX_CHANNEL_COUNT][MAX_RANK_COUNT][MAX_BANK_COUNT];
	dram_controller_t       	channel[MAX_CHANNEL_COUNT];
	int				last_event_type;
	int				last_chan_id;
	int				last_rank_id;
	int				last_bank_id;
} dram_system_t;

enum dram_config_token_t {
	dram_type_token,
	data_rate_token,
	row_buffer_management_policy_token,
	VA_mapping_policy_token,
	PA_mapping_policy_token,
	transaction_ordering_policy_token,
	command_ordering_policy_token,
	refresh_policy_token,
	channel_count_token,
	rank_count_token,
	bank_count_token,
	row_count_token,
	col_count_token,
	col_size_token,
	row_size_token,
	cacheline_size_token,
	TLB_pagesize_token,
	transaction_queue_depth_token,

	t_ras_token,
	t_rcd_token,
	t_cas_token,
	t_rp_token,
	t_rc_token,
	t_cwd_token,
	t_al_token,
	t_rl_token,
	t_dqs_token,
	t_burst_token,
	comment_token,
};

/* function calls used in file IO */

int     is_valid_hex_number(char *);
double  ascii2double(char *);
double  ascii2multiplier(char *);
int     read_mase_trace(FILE *,TRACE *);
int     read_k6_trace(FILE *,TRACE *);
int     read_tm_trace(FILE *,TRACE *);
int     read_ss_trace(FILE *,TRACE *);
void    print_trace(TRACE *);
void 	btv_merge_stat(FILE *,STATS *);
int 	file_io_token(char *);
int	intlog2(int);
void 	init_btv_stats(STATS *);
void 	init_dram_system(dram_system_t *);
void 	init_dram_system_configuration(dram_system_configuration_t *);
void 	init_transaction_queue(transaction_queue_t *);
void 	init_dram_controller(dram_controller_t *);
void 	convert_address(dram_system_configuration_t *, addresses_t *);
void	init_dram_stats(STATS *);
void	reset_idling_distances(TRACE *, STATS *);
void	add_idling_distance(STATS *, int );
int	compute_idling_distance(STATS *, int, int, int);

/* Gather statistics */

/*given xcoordinate, find timestamp */
double btv_find_timestamp(GEOMETRY *, int *, TRACE *,int);

/* given timestamp, find closest event_no at or below it */
int btv_find_event_no(GEOMETRY *, TRACE *, double);

void btv_gather_basic_stats(STATS *, TRACE *);
void btv_print_basic_stats(STATS *);
void btv_gather_advanced_stats(STATS *, TRACE *);
void btv_print_advanced_stats(STATS *, int, FILE *);
void btv_gather_dram_stats(TRACE *, dram_system_t *, STATS *);	
void btv_print_dram_stats(STATS *, int, FILE *);

/* Drawing functions */

void btv_draw_legends(Tk_Window, Drawable, Tk_3DBorder *, GC *, GEOMETRY *, Tk_Font, int *, int *,TRACE *);
void btv_draw_labels(Tk_Window, Drawable, Tk_3DBorder *, GC *, GEOMETRY *, Tk_Font, int *, int *,TRACE *);
void btv_draw_histogram(Tk_Window, Drawable, Tk_3DBorder *, GC *, GEOMETRY *, Tk_Font, int *, int *,TRACE *);

void print_status(int);
void print_addresses(addresses_t *);

/* add transaction */
void add_transaction(dram_system_t *,transaction_t *);
void remove_transaction(dram_system_t *,transaction_t *);
transaction_t *get_next_transaction(dram_system_t * );
int  queue_check(dram_system_t *, int);
void get_dram_states(dram_system_t *, transaction_t *, dram_states_t *);
void print_event(FILE *, int );
void print_state(FILE *, int );
void print_row_state(FILE *, int );
