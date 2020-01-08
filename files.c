#include "btv.h"

int read_tm_trace(FILE *fin,TRACE *trace){
	int		segment_index;
	int 		index;
	char 		input[MAX_NAME_LENGTH]; 
	int		control;
	double		timestamp;
	double 		multiplier;
	BUS_Event *this_e;
	int		acc_type,dec_addr,port,size,time,dummy;

	index = MAX_SEGMENT_SIZE;
	segment_index = -1;

	while(fscanf(fin, "%d %d %d %d %d %d",
					&acc_type,
					&dec_addr,
					&port,
					&size,
					&time,
					&dummy) != EOF){

		if(index == MAX_SEGMENT_SIZE){
			segment_index++;
			if(segment_index == MAX_SEGMENT_COUNT){
				fprintf(stderr,"read_tm_trace error, MAX_SEGMENT_COUNT exceeded \n");
				return BTV_ERROR;
			}
			if(trace->segment[segment_index] == NULL){
				trace->segment[segment_index] = (TRACE_Segment *) calloc(1,sizeof(TRACE_Segment));	
			}
			index = 0;
		}
		this_e = &(trace->segment[segment_index]->event[index]);

		this_e->timestamp = (double) time;
		if(port == 2 && acc_type == 0) { 	/* DRAM READ */
			this_e->attributes = MEM_RD;  
		} else if (port == 2 && acc_type == 1) { /* DRAM WRITE */
			this_e->attributes = MEM_WR;  
		} else if (port == 1 && acc_type == 0) { /* PCI READ */
			this_e->attributes = IO_RD;  
		} else if (port == 1 && acc_type == 1) { /* PCI WRITE */
			this_e->attributes = IO_WR;  
		} else { /* UNKNOWN */
			this_e->attributes = UNKNOWN;  
		}
		this_e->address = dec_addr;
		index++;
	}

	trace->start_time = trace->segment[0]->event[0].timestamp;
	trace->end_time   = trace->segment[segment_index]->event[index].timestamp;
	trace->event_count   = segment_index * MAX_SEGMENT_SIZE + index;
	trace->segment_count   = segment_index + 1 ;
 
	return BTV_OK;
}

int read_k6_trace(FILE *fin,TRACE *trace){
	int		segment_index;
	int 		index;
	char 		input[MAX_NAME_LENGTH]; 
	int		address;
	int		control;
	int		attributes;
	int		burst_length;
	int 		burst_counter;
	double		timestamp;
	double 		multiplier;
	BUS_Event *this_e;
	BUS_Event *last_e;

	burst_length = 4;	/* Socket 7 cachelines are 32 byte long, burst of 4 */
	burst_counter = 0;

	index = MAX_SEGMENT_SIZE;
	segment_index = -1;
	last_e = NULL;

	while((fscanf(fin,"%s",input) != EOF)){ 

		if(index == MAX_SEGMENT_SIZE){
			segment_index++;
			if(segment_index == MAX_SEGMENT_COUNT){
				fprintf(stderr,"read_k6_trace error, MAX_SEGMENT_COUNT exceeded \n");
				return BTV_ERROR;
			}
			if(trace->segment[segment_index] == NULL){
				trace->segment[segment_index] = (TRACE_Segment *) calloc(1,sizeof(TRACE_Segment));	
			}
			index = 0;
		}

		sscanf(input,"%X",&address);  /* found starting Hex address */
		if(fscanf(fin,"%s",input) == EOF) {
			printf("Unexpected EOF, Please fix input trace file \n");
			return BTV_ERROR;
		} 
		if((control = file_io_token(input)) == UNKNOWN){
			printf("Unknown Token Found [%s]\n",input);
		} 

		/* hack */
		/* K6 traces appear to suggest that these high address MEMWrites are in effect PCI */
		/* writes.  Convert them if MSB of address is 1 */

		if(address & 0x80000000){
			if(control == MEM_RD){
				attributes = IO_RD;
			} else if (control == MEM_WR){
				attributes = IO_WR;
			} else {
				attributes = control;
			}
		} else {
			attributes = control;
		}

		if(fscanf(fin,"%s",input) == EOF) {
			printf("Unexpected EOF, Please fix input trace file \n");
			return BTV_ERROR;
		}
		timestamp = ascii2double(input);
		if(fscanf(fin,"%s",input) == EOF) {
			printf("Unexpected EOF, Please fix input trace file \n");
			return BTV_ERROR;
		}
		timestamp = timestamp * ascii2multiplier(input);

		/* we now have attribute, address and timestamp of this event. */
		/* Do we want to create a new transaction for it? */
		if((last_e != NULL) && 
			(last_e->attributes == attributes) &&
			(((last_e->address ^ address) & 0xFFFFFFE0) == 0) &&
			(burst_counter < burst_length)){
			burst_counter++;
		} else {
			burst_counter = 1;
			this_e = &(trace->segment[segment_index]->event[index]);
			this_e->address = address;
			this_e->attributes = attributes;
			this_e->timestamp = timestamp;
			index++;
			last_e = this_e;
		}
	}

	trace->start_time = trace->segment[0]->event[0].timestamp;
	trace->end_time   = trace->segment[segment_index]->event[index-1].timestamp;
	trace->event_count   = segment_index * MAX_SEGMENT_SIZE + index;
	trace->segment_count   = segment_index + 1 ;
 
	return BTV_OK;
}

int read_ss_trace(FILE *fin,TRACE *trace){
	int		segment_index;
	int 		index;
	char 		input[MAX_NAME_LENGTH]; 
	int		thread_id;
	SS_BUS_Event 	*this_e;
	char		c;
	size_t 		length;
	int		time_index;
	int 		i;

	/*  
	 *  Disabled for now.

	index = MAX_SEGMENT_SIZE;
	segment_index = -1;
	if(fscanf(fin,"%s",input) == EOF){ 
		printf("Unexpected EOF, Please fix input trace file \n");
		return BTV_ERROR;
	} 	
	c = input[0];
        length = strlen(input);

	trace->trace_type = SS_TRACE;
        if ((c == 'C') && (strncmp(input, "CQ_FCFS", length) == 0)) {
		trace->policy = CQ_FCFS;
        } else if ((c == 'S') && (strncmp(input, "SQ_RR", length) == 0)) {
		trace->policy = SQ_RR;
        } else if ((c == 'S') && (strncmp(input, "SQ_LFF", length) == 0)) {
		trace->policy = SQ_LFF;
	} else {
		trace->policy = -1;
		printf("Unexpected EOF, Please fix input trace file \n");
		return BTV_ERROR;
	}

	while((fscanf(fin,"%s",input) != EOF)){ 
		c = input[0];
        	length = strlen(input);
        	if (strncmp(input, "time", length) != 0) {
			printf("Unexpected Error, expecting time, but read [%s]\n",input);
			return BTV_ERROR;
		} 
		if(index == MAX_SEGMENT_SIZE){
			segment_index++;
			if(segment_index == MAX_SEGMENT_COUNT){
				fprintf(stderr,"read_ss_trace error, MAX_SEGMENT_COUNT exceeded \n");
				return BTV_ERROR;
			}
			if(trace->segment[segment_index] == NULL){
				trace->ss_segment[segment_index] = (SS_TRACE_Segment *) calloc(1,sizeof(SS_TRACE_Segment));	
			}
			index = 0;
		}

		if(fscanf(fin,"%d",&time_index) == EOF){ 
			printf("Unexpected EOF, Please fix input trace file \n");
			return BTV_ERROR;
		}
		index = time_index % MAX_SEGMENT_SIZE;

		for(i = 0; i< MAX_THREAD_COUNT; i++){
			this_e = &(trace->ss_segment[segment_index]->event[i][index]);
			if(fscanf(fin,"%d",&thread_id) == EOF) {
				printf("Unexpected EOF, Please fix input trace file \n");
				return BTV_ERROR;
			} else if (thread_id != i){
				printf("Unexpected thread_id. Expecting %d, but got %d instead \n",
					i,thread_id);
				return BTV_ERROR;
			}
			if(fscanf(fin,"%d",&(this_e->mem_load_latency_sum)) == EOF) {
				printf("Unexpected EOF, Please fix input trace file \n");
				return BTV_ERROR;
			}
			if(fscanf(fin,"%d",&(this_e->mem_load_count)) == EOF) {
				printf("Unexpected EOF, Please fix input trace file \n");
				return BTV_ERROR;
			}
			if(fscanf(fin,"%d",&(this_e->mem_store_count)) == EOF) {
				printf("Unexpected EOF, Please fix input trace file \n");
				return BTV_ERROR;
			}
		}
	}

	*/

	trace->start_time = 0.0;
	trace->end_time   = 1000000.0 * time_index;
	trace->event_count   = time_index;
	trace->segment_count = segment_index + 1 ;
 
	return BTV_OK;
}

int read_mase_trace(FILE *fin,TRACE *trace){
	int		segment_index;
	int 		index;
	char 		input[MAX_NAME_LENGTH]; 
	int		address;
	int		control;
	double		timestamp;
	double 		multiplier;
	BUS_Event *this_e;

	index = MAX_SEGMENT_SIZE;
	segment_index = -1;

	while((fscanf(fin,"%s",input) != EOF)){ 

		if(index == MAX_SEGMENT_SIZE){
			segment_index++;
			if(segment_index == MAX_SEGMENT_COUNT){
				fprintf(stderr,"read_mase_trace error, MAX_SEGMENT_COUNT exceeded \n");
				return BTV_ERROR;
			}
			if(trace->segment[segment_index] == NULL){
				trace->segment[segment_index] = (TRACE_Segment *) calloc(1,sizeof(TRACE_Segment));
			}
			index = 0;
		}
		this_e = &(trace->segment[segment_index]->event[index]);

		sscanf(input,"%X",&(this_e->address));  /* found starting Hex address */
		if(fscanf(fin,"%s",input) == EOF) {
			printf("Unexpected EOF, Please fix input trace file \n");
			/*
			return BTV_ERROR;
			*/
		} 
		if((control = file_io_token(input)) == UNKNOWN){
			printf("Unknown Token Found [%s]\n",input);
		} 

		/* hack */
		/* K6 traces appear to suggest that these high address MEMWrites are in effect PCI */
		/* writes.  Convert them if MSB of address is 1 */
		/* Use same set of thing for Mase */
	
		if(this_e->address & 0x80000000){
			if(control == MEM_RD){
				this_e->attributes = IO_RD;
			} else if (control == MEM_WR){
				this_e->attributes = IO_WR;
			} else {
				this_e->attributes = control;
			}
		} else {
			this_e->attributes = control;
		}

		if(fscanf(fin,"%s",input) == EOF) {
			printf("Unexpected EOF, Please fix input trace file \n");
			/*
			return BTV_ERROR;
			*/
		}
		sscanf(input,"%lf",&timestamp);
		this_e->timestamp = timestamp;
		index++;
	}

	trace->start_time = trace->segment[0]->event[0].timestamp;
	trace->end_time   = trace->segment[segment_index]->event[index-1].timestamp;
	trace->event_count   = segment_index * MAX_SEGMENT_SIZE + index;
	trace->segment_count   = segment_index + 1 ;
 
	return BTV_OK;
}

void print_trace(TRACE *trace){
	BUS_Event *this_e;
	int 		i;
	int		segment_index,event_index;
	int		event_count;

	event_count = trace->event_count;

	for(i=0;i<event_count;i++){
		segment_index = i / MAX_SEGMENT_SIZE;
		event_index = i % MAX_SEGMENT_SIZE;
		this_e = &(trace->segment[segment_index]->event[event_index]);
		printf("%X %d %12lf\n",this_e->address,this_e->attributes,this_e->timestamp);
	}
}

int	is_valid_hex_number(char *input){
	int result;
	int length,i;

	result = TRUE;

	length = strlen(input);

	if(i>8){
		return FALSE;
	}

	for(i=0;i<length;i++){ /* 0-9 or A-F */
		if((input[i]> 47 && input[i] < 58) || (input[i] > 64 && input[i] < 71)){
			result = TRUE;
		} else {
			result = FALSE;
			return result;
		}
	}
	return result;
}

/* hand coded ascii to double routine.  */

double ascii2double(char *input){
	double 	result,decimal_multiplier;
	int  	decimal_flag,i,length;

	length = strlen(input);

	result = 0.0;
	decimal_multiplier = 1.0;
	decimal_flag = FALSE;

	for(i=0;i<length;i++){
		if(input[i] < 58 && input[i] > 47){
			if(!decimal_flag){
				result = result * 10 + (input[i] - 48);
			} else {
				decimal_multiplier *= 0.1;
				result = result + (input[i]-48) * decimal_multiplier;
			}
		} else if(input[i] == 46){  /* this is the decimal, set decimal_flag */
			decimal_flag = TRUE;
		} else if(input[i] == 44){  /* This is the comma, do nothing */

		} else if(input[i] == 58){ /*  This is the Colon denotes minute */
			result = result * 60;
		} else {
			printf("unrecognized character within number[%s]\n",input);
		}
	}
	return result;
}

double ascii2multiplier(char *input){
	double 	result;
	int  	length;
	int 	token;

	token = file_io_token(input);
	
	switch(token){
		case PICOSECOND:
			result = 0.001;
		break;
		case NANOSECOND:
			result = 1.0;
		break;
		case MICROSECOND:
			result = 1000.0;
		break;
		case MILLISECOND:
			result = 1000000.0;
		break;
		case SECOND:
			result = 1000000000.0;
		break;
		case UNKNOWN:
		default:
			result = 0.0;
		break;
	}
	return result;
}

void btv_merge_stat(FILE *fin,STATS *stats)
{
	char 		input0[MAX_NAME_LENGTH]; 
	char 		input1[MAX_NAME_LENGTH]; 
	double		timestamp;
	double		delta_t_to_same,delta_t_to_last,last_event_timestamp;
	int		i,j,count,found; 
	BUS_Event *this_e;
	STAT_BUCKET	*this_b;
	STAT_BUCKET	*temp_b;
	double 		epsilon;
	int		this_event_type,last_event_type;

	epsilon = EPSILON * 0.1;

	for(i=0;i<STAT_EVENT_TYPE_COUNT;i++){
		stats->event_count[i] = 0;
		stats->last_time[i] = 0.0;
		for(j=0;j<STAT_EVENT_TYPE_COUNT;j++){
			stats->sequential[i][j] = NULL;
		}
		this_b = stats->same[i];
	}

	last_event_timestamp = 1000000000.0;	/* this can be weeded out easily */

	while(fscanf(fin, "%s %s %d %lf",
					&input0,
					&input1,
					&count,
					&delta_t_to_last) != EOF){
		last_event_type = file_io_token(input0);
		this_event_type = file_io_token(input1);
		found = FALSE;
		this_b = stats->sequential[last_event_type][this_event_type];

		if(this_b == NULL){                             /* Nothing in this type of */
								/* transaction. Add a bucket */
			this_b = (STAT_BUCKET *)calloc(1,sizeof(STAT_BUCKET));
			stats->sequential[last_event_type][this_event_type] = this_b;
			this_b -> next_bucket = NULL;
			this_b -> count = count;
			this_b -> delta_t = delta_t_to_last;
			found = TRUE;
		} else if(fabs(this_b->delta_t - delta_t_to_last) <= epsilon){
								/* If we're within tolerance */
								/* add a count to this bucket */
			this_b->count += count;
			found = TRUE;
		} else if(this_b->delta_t > delta_t_to_last){   /* If this delta_t is already */
								/* smaller than the zeroth    */
								/* bucket->delta_t, insert a  */
								/* bucket in front of it      */
			temp_b = (STAT_BUCKET *)calloc(1,sizeof(STAT_BUCKET));
			temp_b -> count = count;
			temp_b -> delta_t = delta_t_to_last;
			temp_b -> next_bucket = this_b;
			stats->sequential[last_event_type][this_event_type] = temp_b;
			found = TRUE;
		}
		while(!found && (this_b -> next_bucket) != NULL){
			if(fabs(this_b->next_bucket->delta_t - delta_t_to_last) <= epsilon){
				found = TRUE;
				this_b->next_bucket->count += count;
			} else if(this_b->next_bucket->delta_t > delta_t_to_last){
				temp_b = (STAT_BUCKET *)calloc(1,sizeof(STAT_BUCKET));
				temp_b -> count = count;
				temp_b -> delta_t = delta_t_to_last;
				temp_b -> next_bucket = this_b->next_bucket;
				this_b -> next_bucket = temp_b;
				found = TRUE;
			} else {
				this_b = this_b -> next_bucket;
			}
		}
		if(!found){                     /* add a new bucket to the end */
			this_b->next_bucket = (STAT_BUCKET *)calloc(1,sizeof(STAT_BUCKET));
			this_b = this_b -> next_bucket;
			this_b -> next_bucket = NULL;
			this_b -> count = count;
			this_b -> delta_t = delta_t_to_last;
		}
	}
}

void read_dram_config_from_file(FILE *fin, dram_system_configuration_t *this_c){
	char 	c;
	char 	input_string[256];
	int	input_int;
	int	dram_config_token;
	size_t length;

	while ((c = fgetc(fin)) != EOF){
		if((c != EOL) && (c != CR) && (c != SPACE) && (c != TAB)){
			fscanf(fin,"%s",&input_string[1]);
			input_string[0] = c;
		} else {
			fscanf(fin,"%s",&input_string[0]);
		}
		dram_config_token = file_io_token(&input_string[0]);
		switch(dram_config_token){
			case dram_type_token:
				fscanf(fin,"%s",&input_string[0]);
				if (!strncmp(input_string, "sdram", 5)){
					this_c->dram_type = SDRAM;	
				} else if (!strncmp(input_string, "ddrsdram", 8)){
					this_c->dram_type = DDRSDRAM;	
				} else if (!strncmp(input_string, "drdram", 6)){
					this_c->dram_type = DRDRAM;	
				} else if (!strncmp(input_string, "ddr2", 4)){
					this_c->dram_type = DDR2;	
				} else {
					this_c->dram_type = SDRAM;	
				}
			break;
			case data_rate_token:				/* aka memory_frequency: units is MBPS */
				fscanf(fin,"%d",&input_int);
				this_c->data_rate = input_int;	
			break;
			case row_buffer_management_policy_token:
				fscanf(fin,"%s",&input_string[0]);
				if (!strncmp(input_string, "open_page", 9)){
					this_c->row_buffer_management_policy = OPEN_PAGE;
				} else if (!strncmp(input_string, "close_page", 10)){
					this_c->row_buffer_management_policy = CLOSE_PAGE;
				} else {
					fprintf(stderr,"\n\n\n\nExpecting buffer management policy, found [%s] instead\n\n\n",input_string);
					this_c->row_buffer_management_policy = OPEN_PAGE;
				}
			break;
			case VA_mapping_policy_token:
				fscanf(fin,"%s",&input_string[0]);
				if (!strncmp(input_string, "equate", 6)){
					this_c->va_mapping_policy = VA_EQUATE;
				} else if (!strncmp(input_string, "random", 6)){
					this_c->va_mapping_policy = VA_RANDOM;
				}
			break;
			case PA_mapping_policy_token:
				fscanf(fin,"%s",&input_string[0]);
				if (!strncmp(input_string, "close_page_expandable_permute", 28)){
					this_c->pa_mapping_policy = CLOSE_PAGE_EXPANDABLE_PERMUTE;
				} else if (!strncmp(input_string, "open_page_expandable_permute", 27)){
					this_c->pa_mapping_policy = OPEN_PAGE_EXPANDABLE_PERMUTE;
				} else if (!strncmp(input_string, "close_page_permute", 18)){
					this_c->pa_mapping_policy = CLOSE_PAGE_PERMUTE;
				} else if (!strncmp(input_string, "open_page_permute", 17)){
					this_c->pa_mapping_policy = OPEN_PAGE_PERMUTE;
				} else if (!strncmp(input_string, "close_page_expandable", 21)){
					this_c->pa_mapping_policy = CLOSE_PAGE_EXPANDABLE;
				} else if (!strncmp(input_string, "open_page_expandable", 20)){
					this_c->pa_mapping_policy = OPEN_PAGE_EXPANDABLE;
				} else if (!strncmp(input_string, "close_page_baseline", 20)){
					this_c->pa_mapping_policy = CLOSE_PAGE_BASELINE;
				} else if (!strncmp(input_string, "open_page_baseline", 19)){
					this_c->pa_mapping_policy = OPEN_PAGE_BASELINE;
				} else {
					fprintf(stderr,"\n\n\n\nExpecting mapping policy, found [%s] instead\n\n\n",input_string);
					this_c->pa_mapping_policy = OPEN_PAGE_BASELINE;
				}
			break;
			case command_ordering_policy_token:
				fscanf(fin,"%s",&input_string[0]);
				if (!strncmp(input_string, "strict_order", 12)){
					this_c->command_ordering_policy = STRICT_ORDER;
				} else if (!strncmp(input_string, "full_reorder", 12)){
					this_c->command_ordering_policy = FULL_REORDER;
				}
			break;
			case transaction_ordering_policy_token:
				fscanf(fin,"%s",&input_string[0]);
				if (!strncmp(input_string, "strict_order", 12)){
					this_c->transaction_ordering_policy = STRICT_ORDER;
				} else if (!strncmp(input_string, "round_robin", 11)){
					this_c->transaction_ordering_policy = ROUND_ROBIN;
				}
			break;
			case refresh_policy_token:
				fscanf(fin,"%s",&input_string[0]);
				if (!strncmp(input_string, "no_refresh", 10)){
					this_c->refresh_policy = NO_REFRESH;
				} else if (!strncmp(input_string, "bank_concurrent", 15)){
					this_c->refresh_policy = BANK_CONCURRENT;
				} else if (!strncmp(input_string, "bank_staggered", 14)){
					this_c->refresh_policy = BANK_STAGGERED_HIDDEN;
				}
			break;
			case channel_count_token:
				fscanf(fin,"%d",&input_int);
				this_c->chan_count = input_int;
			break;
			case rank_count_token:
				fscanf(fin,"%d",&input_int);
				this_c->rank_count = input_int;
			break;
			case bank_count_token:
				fscanf(fin,"%d",&input_int);
				this_c->bank_count = input_int;
			break;
			case row_count_token:
				fscanf(fin,"%d",&input_int);
				this_c->row_count = input_int;
			break;
			case col_count_token:
				fscanf(fin,"%d",&input_int);
				this_c->col_count = input_int;
			break;
			case col_size_token:
				fscanf(fin,"%d",&input_int);
				this_c->col_size = input_int;
			break;
			case row_size_token:
				fscanf(fin,"%d",&input_int);
				this_c->row_size = input_int;
			break;
			case cacheline_size_token:
				fscanf(fin,"%d",&input_int);
				this_c->cacheline_size = input_int;
			break;
			case TLB_pagesize_token:
				fscanf(fin,"%d",&input_int);
				this_c->TLB_pagesize = input_int;
			break;
			case transaction_queue_depth_token:
				fscanf(fin,"%d",&input_int);
				this_c->transaction_queue_depth = input_int;
			break;
			
			case t_ras_token:
				fscanf(fin,"%d",&input_int);
				this_c->t_ras = input_int;
			break;
			case t_rcd_token:
				fscanf(fin,"%d",&input_int);
				this_c->t_rcd = input_int;
			break;
			case t_cas_token:
				fscanf(fin,"%d",&input_int);
				this_c->t_cas = input_int;
			break;
			case t_rp_token:
				fscanf(fin,"%d",&input_int);
				this_c->t_rp = input_int;
			break;
			case t_rc_token:
				fscanf(fin,"%d",&input_int);
				this_c->t_rc = input_int;
			break;
			case t_cwd_token:
				fscanf(fin,"%d",&input_int);
				this_c->t_cwd = input_int;
			break;
			case t_al_token:
				fscanf(fin,"%d",&input_int);
				this_c->t_al = input_int;
			break;
			case t_rl_token:
				fscanf(fin,"%d",&input_int);
				this_c->t_rl = input_int;
			break;
			case t_dqs_token:
				fscanf(fin,"%d",&input_int);
				this_c->t_dqs = input_int;
			break;
			case t_burst_token:
				fscanf(fin,"%d",&input_int);
				this_c->t_burst = input_int;
			break;
			case comment_token:
				while (((c = fgetc(fin)) != EOL) && (c != EOF)){
					/*comment, to be ignored */
				}
			break;
			default:
			case UNKNOWN:
				fprintf(stderr,"Unknown Token [%s]\n",input_string);
			break;
		}
	}
}

int file_io_token(char *input){
	size_t length;
	length = strlen(input);
	if(strncmp(input, "//",2) == 0) {
		return comment_token;
	} else if (strncmp(input, "type",length) == 0) {
		return dram_type_token;
	} else if (strncmp(input, "datarate",length) == 0) {
		return data_rate_token;
	} else if (strncmp(input, "row_buffer_policy",length) == 0) {
		return row_buffer_management_policy_token;
	} else if (strncmp(input, "VA_mapping_policy",length) == 0) {
		return VA_mapping_policy_token;
	} else if (strncmp(input, "PA_mapping_policy",length) == 0) {
		return PA_mapping_policy_token;
	} else if (strncmp(input, "transaction_ordering_policy",length) == 0) {
		return transaction_ordering_policy_token;
	} else if (strncmp(input, "command_ordering_policy",length) == 0) {
		return command_ordering_policy_token;
	} else if (strncmp(input, "refresh_policy",length) == 0) {
		return refresh_policy_token;
	} else if (strncmp(input, "channel_count",length) == 0) {
		return channel_count_token;
	} else if (strncmp(input, "rank_count",length) == 0) {
		return rank_count_token;
	} else if (strncmp(input, "bank_count",length) == 0) {
		return bank_count_token;
	} else if (strncmp(input, "row_count",length) == 0) {
		return row_count_token;
	} else if (strncmp(input, "col_count",length) == 0) {
		return col_count_token;
	} else if (strncmp(input, "col_size",length) == 0) {
		return col_size_token;
	} else if (strncmp(input, "row_size",length) == 0) {
		return row_size_token;
	} else if (strncmp(input, "cacheline_size",length) == 0) {
		return cacheline_size_token;
	} else if (strncmp(input, "TLB_pagesize",length) == 0) {
		return TLB_pagesize_token;
	} else if (strncmp(input, "transaction_queue_depth",length) == 0) {
		return transaction_queue_depth_token;
	} else if (strncmp(input, "t_ras",length) == 0) {
		return t_ras_token;
	} else if (strncmp(input, "t_rcd",5) == 0) {
		return t_rcd_token;
	} else if (strncmp(input, "t_cas",length) == 0) {
		return t_cas_token;
	} else if (strncmp(input, "t_rp",length) == 0) {
		return t_rp_token;
	} else if (strncmp(input, "t_rc",4) == 0) {
		return t_rc_token;
	} else if (strncmp(input, "t_cwd",length) == 0) {
		return t_cwd_token;
	} else if (strncmp(input, "t_al",length) == 0) {
		return t_al_token;
	} else if (strncmp(input, "t_rl",length) == 0) {
		return t_rl_token;
	} else if (strncmp(input, "t_dqs",length) == 0) {
		return t_dqs_token;
	} else if (strncmp(input, "t_burst",length) == 0) {
		return t_burst_token;
	} else if(strncmp(input, "FETCH",length) == 0) {
		return FETCH;
	} else if (strncmp(input, "IFETCH",length) == 0) {
		return FETCH;
	} else if (strncmp(input, "P_FETCH",length) == 0) {
		return FETCH;
	} else if (strncmp(input, "P_LOCK_RD",length) == 0) {
		return LOCK_RD;                    
	} else if (strncmp(input, "P_LOCK_WR",length) == 0) {
		return LOCK_WR;                    
	} else if (strncmp(input, "LOCK_RD",length) == 0) {
		return LOCK_RD;                    
	} else if (strncmp(input, "LOCK_WR",length) == 0) {
		return LOCK_WR;                    
	} else if (strncmp(input, "MEM_RD",length) == 0) {
		return MEM_RD;                    
	} else if (strncmp(input, "WRITE",length) == 0) {
		return MEM_WR;                    
	} else if (strncmp(input, "MEM_WR",length) == 0) {
		return MEM_WR;                    
	} else if (strncmp(input, "READ",length) == 0) {
		return MEM_RD;                    
	} else if (strncmp(input, "P_MEM_RD",length) == 0) {
		return MEM_RD;                    
	} else if (strncmp(input, "P_MEM_WR",length) == 0) {
		return MEM_WR;                    
	} else if (strncmp(input, "P_I/O_RD",length) == 0) {
		return IO_RD;                    
	} else if (strncmp(input, "P_I/O_WR",length) == 0) {
		return IO_WR;                    
	} else if (strncmp(input, "IO_RD",length) == 0) {
		return IO_RD;                    
	} else if (strncmp(input, "I/O_RD",length) == 0) {
		return IO_RD;                    
	} else if (strncmp(input, "IO_WR",length) == 0) {
		return IO_WR;                    
	} else if (strncmp(input, "I/O_WR",length) == 0) {
		return IO_WR;                    
	} else if (strncmp(input, "P_INT_ACK",length) == 0) {
		return INT_ACK;                    
	} else if (strncmp(input, "INT_ACK",length) == 0) {
		return INT_ACK;                    
	} else if (strncmp(input, "BOFF",length) == 0) {
		return BOFF;                    
	} else if (strncmp(input, "ps",length) == 0) {
		return PICOSECOND;                    
	} else if (strncmp(input, "ns",length) == 0) {
		return NANOSECOND;                    
	} else if (strncmp(input, "us",length) == 0) {
		return MICROSECOND;                    
	} else if (strncmp(input, "ms",length) == 0) {
		return MILLISECOND;                    
	} else if (strncmp(input, "s",length) == 0) {
		return SECOND;                    
	} else {
		printf("Unknown %s\n",input);
		return UNKNOWN;
	}
}

int btvColor_token(char *mycolor){
        if(strncmp(mycolor, "MidnightBlue",8) == 0) {
                return BTV_DarkBlue;
        } else if (strncmp(mycolor, "blue",4) == 0) {
                return BTV_Blue;                    
        } else if (strncmp(mycolor, "cyan",4) == 0) {
                return BTV_Cyan;                    
        } else if (strncmp(mycolor, "darkgreen",9) == 0) {    
                return BTV_DarkGreen;                       
        } else if (strncmp(mycolor, "green",5) == 0) {        
                return BTV_Green;                          
        } else if (strncmp(mycolor, "yellow",6) == 0) {    
                return BTV_Yellow;                       
        } else if (strncmp(mycolor, "orange",6) == 0) {  
                return BTV_Orange;                     
        } else if (strncmp(mycolor, "red",3) == 0) {   
                return BTV_Red;                      
        } else if (strncmp(mycolor, "pink",4) == 0) {
                return BTV_Pink;                     
        } else if (strncmp(mycolor, "magenta",7) == 0) {
                return BTV_Magenta;                    
        } else if (strncmp(mycolor, "purple",6) == 0) { 
                return BTV_Purple;                    
        } else if (strncmp(mycolor, "brown",5) == 0) { 
                return BTV_Brown;                     
        } else if (strncmp(mycolor, "gray",4) == 0) { 
                return BTV_Gray;
        } else if (strncmp(mycolor, "lightgray",9) == 0) {
                return BTV_LightGray;
        } else if (strncmp(mycolor, "white",5) == 0) {  
                return BTV_White;
        } else { 
                return BTV_Black;
        }
} 

