#include "btv.h"

/* given x coordinate, find timestamp */

void init_btv_stats(STATS *this_s){
	int i,j,k,l,m,n;
	this_s->start_no = 0;
	for(i=0;i<STAT_EVENT_TYPE_COUNT;i++){
		for(j=0;j<STAT_EVENT_TYPE_COUNT;j++){
			this_s->sequential[i][j] = NULL;
		}
		this_s->same[i] = NULL;
	}
}


double
btv_find_timestamp(GEOMETRY *geometry,
		int *display_modes,
		TRACE *trace,
		int start_x)
{
	double		timestamp;
	int		offset_x;

	/* units here are ns */

	if(display_modes[SHOW_LABELS]){
		offset_x = geometry -> offset_x;
	} else {
		offset_x = 0;
	}
	timestamp 	= (start_x - offset_x - geometry->origin_x) * geometry->x_axis_scale 
			+ trace->start_time;
	timestamp 	= MAX(timestamp,trace->start_time);
	timestamp 	= MIN(timestamp,trace->end_time);
	return(timestamp);
}

int 
btv_find_event_no(GEOMETRY *geometry,
		TRACE *trace,
		double timestamp)
{
	int		event_count;
	double		start_time;
	double		end_time;
	BUS_Event 	*this_e;
	int		found;
	int		segment_index,event_index;
	int		lower_index,upper_index,temp_index,start_index;

	/* units here are ns */

	event_count 	= trace->event_count;
	start_time 	= trace->start_time;
	end_time	= trace->end_time;
	lower_index 	= 0;
	upper_index 	= event_count - 1;
	found 		= FALSE;

	if(timestamp < start_time){
		return lower_index;
	} 
	if(timestamp > end_time){
		return upper_index;
	} 
	while(!found){
		temp_index 	= (lower_index + upper_index)/2;
		segment_index 	= temp_index / MAX_SEGMENT_SIZE;
		event_index	= temp_index % MAX_SEGMENT_SIZE;
		this_e = &(trace->segment[segment_index]->event[event_index]);

		if(this_e->timestamp == timestamp){
			found = TRUE;
			start_index = lower_index;
		} else if(upper_index - lower_index < 2) {
			found = TRUE;
			start_index = temp_index;
		} else if(this_e->timestamp > timestamp){
			upper_index = temp_index;
		} else {
			lower_index = temp_index;
		}
	}
	return(start_index);
}

void 
btv_gather_basic_stats(STATS *stats, 
		TRACE *trace){
	int i,j;
	int segment_index,event_index;
	int start_no,end_no,temp_no;
	BUS_Event       *this_e;

	end_no 		= stats->end_no;
	start_no	= stats->start_no;
	if(start_no > end_no){	/* start no. has to be lower */
		stats->end_no 	= start_no;
		stats->start_no = end_no;
		end_no 		= stats->end_no;
		start_no	= stats->start_no;
	}

	for(i=0;i<STAT_EVENT_TYPE_COUNT;i++){
		stats->event_count[i] = 0;
	}

	segment_index   = start_no / MAX_SEGMENT_SIZE;
	event_index     = start_no % MAX_SEGMENT_SIZE;
	stats->start_time = trace->segment[segment_index]->event[event_index].timestamp;
	segment_index   = end_no / MAX_SEGMENT_SIZE;
	event_index     = end_no % MAX_SEGMENT_SIZE;
	stats->end_time = trace->segment[segment_index]->event[event_index].timestamp;

	for(i=start_no; i<=end_no;i++){
		segment_index   = i / MAX_SEGMENT_SIZE;
		event_index     = i % MAX_SEGMENT_SIZE;
		this_e 		= &(trace->segment[segment_index]->event[event_index]);
		stats->event_count[this_e->attributes]++;
	}
}

void 
btv_print_basic_stats(STATS *stats)
{

	int i,j;
	char buffer[100];
	printf("size[%d] starttime[%9.1lf] endtime[%9.1lf]\n",
					stats->end_no - stats->start_no + 1,
					stats->start_time,
					stats->end_time);

	for(i = 0 ; i < STAT_EVENT_TYPE_COUNT;i++){
		printf("%s count = [%d]\n",event_name[i],stats->event_count[i]);
	}
}

/* 
 *  Our Objective here is to go through all entries.  stuff them one by one into the queues
 *  and to "execute" stuff from the queues, one at a time.
 *  Keep track of a short history of the idling distances, and when we get an access distance,
 *  we try and figure out if we need an idling distance added for this entry. 
 *  If we do, then add the appropriate amount.
 */

void 
btv_gather_dram_stats(	TRACE *trace,
			dram_system_t *this_ds,
			STATS *stats){ 
	int 		i,j,k,l,m,n;
	int 		segment_index,event_index;
	int 		start_no,end_no,temp_no,index_no;
	int		queues_full, queues_empty;
	int		chan_id;
	int		rank_id;
	int		bank_id;
	int		access_distance;
	int		access_type;
	int		idling_distance;
	int		max_open_bank_count;
	int		minimum_distance;
	int		ignore_this_transaction;
	BUS_Event	*this_e;
	transaction_t	*this_t;
	transaction_t	*next_t;
	dram_states_t	*this_s;

	minimum_distance = this_ds->config.d_m;
	max_open_bank_count = this_ds->config.max_open_bank_count;

	end_no 		= stats->end_no;
	start_no	= stats->start_no;
	if(start_no > end_no){	/* start no. has to be lower */
		stats->end_no 	= start_no;
		stats->start_no = end_no;
		end_no 		= stats->end_no;
		start_no	= stats->start_no;
	}
	
	this_t = (transaction_t *) calloc(sizeof(transaction_t),1);
	this_s = (dram_states_t *) calloc(sizeof(dram_states_t),1);
	queues_full 	= FALSE;
	queues_empty	= FALSE;
	stats->valid_transaction_count = 0;

	for(index_no=start_no; index_no<=end_no;index_no++){
		segment_index   = index_no / MAX_SEGMENT_SIZE;
		event_index     = index_no % MAX_SEGMENT_SIZE;
		this_e 		= &(trace->segment[segment_index]->event[event_index]);
		ignore_this_transaction = FALSE;
		switch(this_e->attributes){
			case MEM_RD:
			case IO_RD:
			case LOCK_RD:
			case FETCH:
				access_type = MEM_RD;
				stats->valid_transaction_count++;
			break;
			case MEM_WR:
			case IO_WR:
			case LOCK_WR:
				access_type = MEM_WR;
				stats->valid_transaction_count++;
			break;
			case BOFF:
			case INT_ACK:
			case UNKNOWN:
				access_type = BOFF;
				ignore_this_transaction = TRUE;
			break;
			default:
				access_type = MEM_RD;
				stats->valid_transaction_count++;
				/*
				fprintf(stderr,"Not expecting %d default to MEM_RD\n",this_e->attributes);
				*/
			break;
		}
		stats->event_count[access_type]++;
		this_t->type = access_type;
		this_t->addr.virtual_address = this_e->address;
		convert_address(&(this_ds->config), &(this_t->addr));
		/*
		fprintf(stderr,"%d ",index_no);
		print_addresses(&(this_t->addr));
		*/

		if(ignore_this_transaction == TRUE){
			/* Twiddle my thumb */
			/* This guy should be ignored */
		} else if(this_ds->config.transaction_queue_depth == 0){	/* simulate FIFO */
			get_dram_states(this_ds, this_t, this_s);
			if(this_ds->config.row_buffer_management_policy == OPEN_PAGE){		/* open page policy*/
				this_ds->channel[this_t->addr.chan_id].rank[this_t->addr.rank_id].bank[this_t->addr.bank_id].status = OPEN;
				this_ds->channel[this_t->addr.chan_id].rank[this_t->addr.rank_id].bank[this_t->addr.bank_id].row_id = this_t->addr.row_id;
				stats->count[this_ds->last_event_type][this_e->attributes][this_s->chan_state][this_s->rank_state][this_s->bank_state][this_s->row_state]++;
				if(this_s->row_state == CONFLICT){
					access_distance = MIN(this_ds->channel[this_t->addr.chan_id].rank[this_t->addr.rank_id].bank[this_t->addr.bank_id].age, MAX_ACCESS_DISTANCE_RANGE - 1);
					stats->access_distance[access_distance]++;				/* age bucket */
					idling_distance = compute_idling_distance(stats, minimum_distance , access_distance, max_open_bank_count);
					add_idling_distance(stats, idling_distance);
					this_ds->channel[this_t->addr.chan_id].rank[this_t->addr.rank_id].bank[this_t->addr.bank_id].age = 0;
				} else {
					add_idling_distance(stats, 0);
				}
			} else if(this_ds->config.row_buffer_management_policy == CLOSE_PAGE){	/* close page policy*/
				stats->count[this_ds->last_event_type][this_e->attributes][this_s->chan_state][this_s->rank_state][this_s->bank_state][DONTCARE]++;
				access_distance = MIN(this_ds->channel[this_t->addr.chan_id].rank[this_t->addr.rank_id].bank[this_t->addr.bank_id].age, MAX_ACCESS_DISTANCE_RANGE - 1);
				stats->access_distance[access_distance]++;				/* age bucket */
				idling_distance = compute_idling_distance(stats, minimum_distance, access_distance, max_open_bank_count);
				add_idling_distance(stats, idling_distance);
			}

			for(i=0;i<this_ds->config.chan_count;i++){
				for(j=0;j<this_ds->config.rank_count;j++){
					for(k=0;k<this_ds->config.bank_count;k++){
						this_ds->channel[i].rank[j].bank[k].age++;
					}
				}
			}
			this_ds->channel[this_t->addr.chan_id].rank[this_t->addr.rank_id].bank[this_t->addr.bank_id].age 	= 0;
		} else {
			add_transaction(this_ds, this_t);
			queues_full = queue_check(this_ds, IS_QUEUE_FULL);
			while (queues_full) {				/* queues are full, simulate execution */
				next_t = get_next_transaction(this_ds);
				get_dram_states(this_ds, next_t, this_s);
				if(this_ds->config.row_buffer_management_policy == OPEN_PAGE){		/* open page policy*/
					this_ds->channel[next_t->addr.chan_id].rank[next_t->addr.rank_id].bank[next_t->addr.bank_id].status = OPEN;
					this_ds->channel[next_t->addr.chan_id].rank[next_t->addr.rank_id].bank[next_t->addr.bank_id].row_id = next_t->addr.row_id;
					stats->count[this_ds->last_event_type][this_e->attributes][this_s->chan_state][this_s->rank_state][this_s->bank_state][this_s->row_state]++;
					if(this_s->row_state == CONFLICT){
						access_distance = MIN(this_ds->channel[next_t->addr.chan_id].rank[next_t->addr.rank_id].bank[next_t->addr.bank_id].age, MAX_ACCESS_DISTANCE_RANGE - 1);
						stats->access_distance[access_distance]++;				/* age bucket */
						this_ds->channel[next_t->addr.chan_id].rank[next_t->addr.rank_id].bank[next_t->addr.bank_id].age = 0;
						idling_distance = compute_idling_distance(stats, minimum_distance, access_distance, max_open_bank_count);
						add_idling_distance(stats, idling_distance);
					} else {
						add_idling_distance(stats, 0);
					}
				} else if(this_ds->config.row_buffer_management_policy == CLOSE_PAGE){	/* close page policy*/
					stats->count[this_ds->last_event_type][this_e->attributes][this_s->chan_state][this_s->rank_state][this_s->bank_state][DONTCARE]++;
					access_distance = MIN(this_ds->channel[next_t->addr.chan_id].rank[next_t->addr.rank_id].bank[next_t->addr.bank_id].age, MAX_ACCESS_DISTANCE_RANGE - 1);
					idling_distance = compute_idling_distance(stats, minimum_distance, access_distance, max_open_bank_count);
					add_idling_distance(stats, idling_distance);
					stats->access_distance[access_distance]++;				/* age bucket */
				}
	
				for(i=0;i<this_ds->config.chan_count;i++){
					for(j=0;j<this_ds->config.rank_count;j++){
						for(k=0;k<this_ds->config.bank_count;k++){
							this_ds->channel[i].rank[j].bank[k].age++;
						}
					}
				}
				this_ds->channel[next_t->addr.chan_id].rank[next_t->addr.rank_id].bank[next_t->addr.bank_id].age 	= 0;
				remove_transaction(this_ds, next_t);
				queues_full = queue_check(this_ds, IS_QUEUE_FULL);
			}
		}
	}
	queues_empty = queue_check(this_ds, IS_QUEUE_EMPTY);
	next_t = get_next_transaction(this_ds);
	while(next_t != NULL && this_ds->config.transaction_queue_depth != 0){	/* drain the queues */
		get_dram_states(this_ds, next_t, this_s);
		if(this_ds->config.row_buffer_management_policy == OPEN_PAGE){		/* open page policy*/
			this_ds->channel[next_t->addr.chan_id].rank[next_t->addr.rank_id].bank[next_t->addr.bank_id].status = OPEN;
			this_ds->channel[next_t->addr.chan_id].rank[next_t->addr.rank_id].bank[next_t->addr.bank_id].row_id = next_t->addr.row_id;
			stats->count[this_ds->last_event_type][this_e->attributes][this_s->chan_state][this_s->rank_state][this_s->bank_state][this_s->row_state]++;
			if(this_s->row_state == CONFLICT){
				this_ds->channel[next_t->addr.chan_id].rank[next_t->addr.rank_id].bank[next_t->addr.bank_id].age = 0;
			}
		} else if(this_ds->config.row_buffer_management_policy == CLOSE_PAGE){	/* close page policy*/
			stats->count[this_ds->last_event_type][this_e->attributes][this_s->chan_state][this_s->rank_state][this_s->bank_state][DONTCARE]++;
		}
		access_distance = MAX(this_ds->channel[next_t->addr.chan_id].rank[next_t->addr.rank_id].bank[next_t->addr.bank_id].age,
				MAX_ACCESS_DISTANCE_RANGE - 1);
		stats->access_distance[access_distance]++;				/* age bucket */
		idling_distance = compute_idling_distance(stats, minimum_distance, access_distance, max_open_bank_count);
		add_idling_distance(stats, idling_distance);

		for(i=0;i<this_ds->config.chan_count;i++){
			for(j=0;j<this_ds->config.rank_count;j++){
				for(k=0;k<this_ds->config.bank_count;k++){
					this_ds->channel[i].rank[j].bank[k].age++;
				}
			}
		}
		this_ds->channel[next_t->addr.chan_id].rank[next_t->addr.rank_id].bank[next_t->addr.bank_id].age 	= 0;
		remove_transaction(this_ds, next_t);
		next_t = get_next_transaction(this_ds);
	}
}

void 
btv_print_dram_stats(STATS *stats, int printflag, FILE *fout)
{
	int i,j,k,l,m,n;
	double sum_idling_distances;
	STAT_BUCKET *this_b;
	double total_time,percentage,cumulative_time;
	double total_percentage,cumulative_percentage;
	double trace_event_count;
	
	if(printflag == PRINT_SCHEDULING_TABLE){
		for(i=0;i<STAT_EVENT_TYPE_COUNT;i++){
			stats->event_count[i] = 0;
			for(j=0;j<STAT_EVENT_TYPE_COUNT;j++){
				for(k=0;k<MAX_CHANNEL_STATE;k++){
					for(l=0;l<MAX_RANK_STATE;l++){
						for(m=0;m<MAX_BANK_STATE;m++){
							for(n=0;n<MAX_ROW_STATE;n++){
								if(stats->count[i][j][k][l][m][n] > 0){
									print_event(fout,i);
									print_event(fout,j);
									print_state(fout,k);
									print_state(fout,l);
									print_state(fout,m);
									print_row_state(fout,n);
									fprintf(fout,"%d\n", stats->count[i][j][k][l][m][n]);
								}
							}
						}
					}
				}
			}
		}
	} else if (printflag == PRINT_ACCESS_DISTANCE){
		for(i=0;i<MAX_ACCESS_DISTANCE_RANGE;i++){
			fprintf(fout,"%3d %d\n",i,stats->access_distance[i]);
		}
	} else if (printflag == PRINT_IDLING_DISTANCE){
		for(i=0;i<MAX_ACCESS_DISTANCE_RANGE;i++){
			if(stats->idling_distance[i] != 0){
				fprintf(fout,"%3d %d\n",i,stats->idling_distance[i]);
			}
		}
	} else if (printflag == PRINT_BANDWIDTH){
		trace_event_count = stats->valid_transaction_count;
		sum_idling_distances = 0.0;
		for(i=0;i<MAX_ACCESS_DISTANCE_RANGE;i++){
			sum_idling_distances += (double)(i * stats->idling_distance[i]);
		}
		total_percentage = 100.0 * trace_event_count / (trace_event_count + sum_idling_distances);
		/*
		fprintf(fout,"count[%10.0lf] idle[%10.0lf] percentage[%5.2lf]\n",trace_event_count,sum_idling_distances,total_percentage);
		*/
		fprintf(fout,"%5.2lf ",total_percentage);
	}
}

/* 
 *  Our Objective here is to create buckets of delta_t's, from smallest to 
 *  largest, and keep track of how many times of each occurs.
 */

void 
btv_gather_advanced_stats(STATS *stats, 
		TRACE *trace)
{
	int 		i,j;
	int 		segment_index,event_index;
	int 		start_no,end_no,temp_no;
	int 		last_event_type;
	double 		delta_t_to_same,delta_t_to_last,last_event_timestamp;
	BUS_Event       *this_e;
	int		found;
	STAT_BUCKET	*this_b;
	STAT_BUCKET	*temp_b;
	double		epsilon;

	end_no 		= stats->end_no;
	start_no	= stats->start_no;
	if(start_no > end_no){	/* start no. has to be lower */
		stats->end_no 	= start_no;
		stats->start_no = end_no;
		end_no 		= stats->end_no;
		start_no	= stats->start_no;
	}
	
	if(trace->trace_type == K6_TRACE){
		epsilon = EPSILON;
	} else if(trace->trace_type == TM_TRACE){
		epsilon = 0.1 * EPSILON;
	}

	for(i=0;i<STAT_EVENT_TYPE_COUNT;i++){
		stats->event_count[i] = 0;
		stats->last_time[i] = trace->end_time;		/* init this to the biggest time we can get */
		for(j=0;j<STAT_EVENT_TYPE_COUNT;j++){
			this_b = stats->sequential[i][j];
			while(this_b != NULL){
				temp_b = this_b->next_bucket;
				free(this_b);
				this_b = temp_b;
			}
			stats->sequential[i][j] = NULL;
		}
		this_b = stats->same[i];
		while(this_b != NULL){
			temp_b = this_b->next_bucket;
			free(this_b);
			this_b = temp_b;
		}
		stats->same[i] = NULL;
	}
	last_event_type = UNKNOWN;
	last_event_timestamp = trace->end_time;

	segment_index   = start_no / MAX_SEGMENT_SIZE;
	event_index     = start_no % MAX_SEGMENT_SIZE;
	stats->start_time = trace->segment[segment_index]->event[event_index].timestamp;
	segment_index   = end_no / MAX_SEGMENT_SIZE;
	event_index     = end_no % MAX_SEGMENT_SIZE;
	stats->end_time = trace->segment[segment_index]->event[event_index].timestamp;

	for(i=start_no; i<=end_no;i++){
		segment_index   = i / MAX_SEGMENT_SIZE;
		event_index     = i % MAX_SEGMENT_SIZE;
		this_e 		= &(trace->segment[segment_index]->event[event_index]);
		stats->event_count[this_e->attributes]++;
		delta_t_to_same = this_e->timestamp - stats->last_time[this_e->attributes];
		delta_t_to_last = this_e->timestamp - last_event_timestamp; 

		found = FALSE;
		this_b = stats->sequential[last_event_type][this_e->attributes];
		if(this_b == NULL){				/* Nothing in this type of */
								/* transaction. Add a bucket */		
			this_b = (STAT_BUCKET *)calloc(1,sizeof(STAT_BUCKET));
			stats->sequential[last_event_type][this_e->attributes] = this_b;
			this_b -> next_bucket = NULL;
			this_b -> count = 1;
			this_b -> delta_t = delta_t_to_last; 
			found = TRUE;
		} else if(fabs(this_b->delta_t - delta_t_to_last) <= epsilon){	
								/* If we're within tolerance */
								/* add a count to this bucket */
			this_b->count++;
			found = TRUE;
		} else if(this_b->delta_t > delta_t_to_last){	/* If this delta_t is already */
								/* smaller than the zeroth    */
								/* bucket->delta_t, insert a  */
								/* bucket in front of it      */
			temp_b = (STAT_BUCKET *)calloc(1,sizeof(STAT_BUCKET));
			temp_b -> count = 1;
			temp_b -> delta_t = delta_t_to_last; 
			temp_b -> next_bucket = this_b;
			stats->sequential[last_event_type][this_e->attributes] = temp_b;
			found = TRUE;
		}
		while(!found && (this_b -> next_bucket) != NULL){
			if(fabs(this_b->next_bucket->delta_t - delta_t_to_last) <= epsilon){
				found = TRUE;
				this_b->next_bucket->count++;
			} else if(this_b->next_bucket->delta_t > delta_t_to_last){
				temp_b = (STAT_BUCKET *)calloc(1,sizeof(STAT_BUCKET));
				temp_b -> count = 1;
				temp_b -> delta_t = delta_t_to_last; 
				temp_b -> next_bucket = this_b->next_bucket;
				this_b -> next_bucket = temp_b;
				found = TRUE;
			} else {
				this_b = this_b -> next_bucket;
			}
		}
		if(!found){			/* add a new bucket to the end */
			this_b->next_bucket = (STAT_BUCKET *)calloc(1,sizeof(STAT_BUCKET));
			this_b = this_b -> next_bucket;
			this_b -> next_bucket = NULL;
			this_b -> count = 1;
			this_b -> delta_t = delta_t_to_last; 
		}
		stats->last_time[this_e->attributes] = this_e->timestamp;
		last_event_timestamp = this_e->timestamp;
		last_event_type = this_e -> attributes;
	}
}

void 
btv_print_advanced_stats(STATS *stats, int printflag, FILE *fout)
{
	int i,j,total_event_count;
	int cumulative_count;
	STAT_BUCKET *this_b;
	double total_time,percentage,cumulative_time;
	double total_percentage,cumulative_percentage;
	double trace_event_count;

	trace_event_count = stats->end_no - stats->start_no + 1;

	total_time = stats->end_time - stats->start_time;	
	total_event_count = 0;
	total_percentage = 0.0;

/*
	fprintf(fout,"size[%d] starttime[%lf] endtime[%lf]\n",
					stats->end_no - stats->start_no + 1,
					stats->start_time,
					stats->end_time);
	if(printflag == PRINT_ALL) {
		fprintf(fout,"$1 followed by $2     delta_t      count   %c of time   %c of events\n",
									PERCENT_SIGN,PERCENT_SIGN);
	} else if(printflag == PRINT_SUMMARY) {
		fprintf(fout,"$1 followed by $2  avg delta_t     count   %c of time   %c of events\n",
									PERCENT_SIGN,PERCENT_SIGN);
	}
*/
	for(i = 0 ; i < STAT_EVENT_TYPE_COUNT;i++){
		for(j = 0 ; j < STAT_EVENT_TYPE_COUNT;j++){
			this_b = stats->sequential[j][i];
			cumulative_percentage = 0.0;
			cumulative_count = 0;
			cumulative_time = 0.0;
			while(this_b != NULL){
				total_event_count += this_b->count;
				cumulative_count += this_b->count;
				percentage = (this_b->delta_t * this_b->count * 100.0) /total_time;
				cumulative_time += this_b->delta_t * this_b->count ;
				cumulative_percentage += percentage;
				if(printflag == PRINT_DETAILS) {
/*
					fprintf(fout,"%s -> %s [%9.1lf] [%8d]    [%5.2lf]     [%5.2lf]\n",
						event_name[j],
						event_name[i],
						this_b->delta_t,
						this_b->count,
						percentage,
						100.0 * this_b->count/trace_event_count);
*/
					fprintf(fout,"%s  %s  %8d  %9.1lf    \n",
						event_name[j],
						event_name[i],
						this_b->count,
						this_b->delta_t);
				} else if((i == j) && (cumulative_count > 0) &&  
					(fabs(this_b->delta_t - 10.0) < EPSILON) &&
					(cumulative_count == this_b->count)){
					fprintf(fout,"%s    BURST    %9.1lf   %8d     \n",
						event_name[i],
						this_b->delta_t,
						this_b->count);
					cumulative_count = 0; 
					cumulative_time = 0.0;
					cumulative_percentage = 0.0;
				}
					
				this_b = this_b->next_bucket;
			}
			if(printflag == PRINT_SUMMARY && 
				cumulative_count > 0 && 
				j != UNKNOWN &&
				i != UNKNOWN){
/*
				fprintf(fout,"%s -> %s [%9.1lf] [%8d]    [%5.2lf]     [%5.2lf]\n",
					event_name[j],
					event_name[i],
					cumulative_time/cumulative_count,
					cumulative_count,
					cumulative_percentage,
					100.0 * cumulative_count/trace_event_count);
*/
				fprintf(fout,"%s    %s  %9.1lf   %8d  \n",
					event_name[j],
					event_name[i],
					cumulative_time/cumulative_count,
					cumulative_count);
			}
		}
	}
/*
	fprintf(fout,"Sanity check: total_event_count = %d net percentage = [%5.2lf] \n",
							total_event_count,total_percentage);
	fprintf(fout,"Net percentage should be 0.00 percent\n");
*/
}

void convert_address(dram_system_configuration_t *this_c, addresses_t     *this_a){
	unsigned int input_a;
	unsigned int temp_a, temp_b, temp_c, xor_bank_id;
	int	chan_count, rank_count, bank_count, col_count, row_count;
	int	chan_addr_depth, rank_addr_depth, bank_addr_depth, row_addr_depth, col_addr_depth; 
	int	cacheline_size, cacheline_offset;

	chan_count = this_c->chan_count;
	rank_count = this_c->rank_count;
	bank_count = this_c->bank_count;
	row_count  = this_c->row_count;
	col_count  = this_c->col_count;
	cacheline_size = this_c->cacheline_size;
	chan_addr_depth = intlog2(chan_count);
	rank_addr_depth = intlog2(rank_count);
	bank_addr_depth = intlog2(bank_count);
	row_addr_depth  = intlog2(row_count);
	col_addr_depth  = intlog2(col_count);
	cacheline_offset= intlog2(cacheline_size);
	/*
	fprintf(stderr,"%d %d %d %d %d %d\n",chan_addr_depth,rank_addr_depth,bank_addr_depth,row_addr_depth,col_addr_depth,cacheline_offset);
	*/

	this_a->physical_address = this_a->virtual_address;	/* for now, equate va to pa */
	input_a = this_a->physical_address;
	input_a = input_a >> cacheline_offset;			/* strip off cacheline offset.  We don't care */

	if(this_c->pa_mapping_policy == OPEN_PAGE_BASELINE){		/* r:l:b:n:k:z */
		/* 
		 * |<-------------------------------->|<------>|<------>|<---------------->|<----------------->|<----------->|
		 *                           row id     Rank id  Bank id    column id         Channel id          Byte Address 
		 *                                                          DRAM page size/   log2(chan. count)   within cacheline
		 *                                                          Bus Width         used if chan. > 1   
		 *     
		 *               As applied to system (1 chan) using 8 1 Gbit DDR2 DRAM chips:
		 *               16384 rows * 8 banks * 1024 columns * 8 bytes per column.
		 *               16384 rows * 8 banks * 128 cachelines per column * 64 bytes per cacheline
		 *		 1 channel and 1 ranks = 1 GB in system.
		 *
		 *    31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
		 *          |<------------------------------------->| |<---->| |<---------------->|  |<------------>|
		 *                      row id                           bank         Col id             64 byte 
		 *                      (512 rows)                        id         1024*8/64          cacheline
		 *                      							         offset
		 */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> chan_addr_depth;
		temp_a  = input_a << chan_addr_depth;
		this_a->chan_id = temp_a ^ temp_b;     		/* strip out the channel address */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> col_addr_depth;
		temp_a  = input_a << col_addr_depth;
		this_a->col_id = temp_a ^ temp_b;     		/* strip out the column address */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> bank_addr_depth;
		temp_a  = input_a << bank_addr_depth;
		this_a->bank_id = temp_a ^ temp_b;		/* this should strip out the bank address */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> rank_addr_depth;
		temp_a  = input_a << rank_addr_depth;
		this_a->rank_id = temp_a ^ temp_b;		/* this should strip out the rank address */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> row_addr_depth;
		temp_a  = input_a << row_addr_depth;
		this_a->row_id = temp_a ^ temp_b;		/* this should strip out the row address */

	} else if(this_c->pa_mapping_policy == CLOSE_PAGE_BASELINE){		/* r:n:l:b:k:z */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> chan_addr_depth;
		temp_a  = input_a << chan_addr_depth;
		this_a->chan_id = temp_a ^ temp_b;     		/* strip out the channel address */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> bank_addr_depth;
		temp_a  = input_a << bank_addr_depth;
		this_a->bank_id = temp_a ^ temp_b;		/* this should strip out the bank address */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> rank_addr_depth;
		temp_a  = input_a << rank_addr_depth;
		this_a->rank_id = temp_a ^ temp_b;		/* this should strip out the rank address */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> col_addr_depth;
		temp_a  = input_a << col_addr_depth;
		this_a->col_id = temp_a ^ temp_b;     		/* strip out the column address */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> row_addr_depth;
		temp_a  = input_a << row_addr_depth;
		this_a->row_id = temp_a ^ temp_b;		/* this should strip out the row address */

	} else if(this_c->pa_mapping_policy == OPEN_PAGE_EXPANDABLE){		/* k:l:r:b:n:z */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> col_addr_depth;
		temp_a  = input_a << col_addr_depth;
		this_a->col_id = temp_a ^ temp_b;     		/* strip out the column address */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> bank_addr_depth;
		temp_a  = input_a << bank_addr_depth;
		this_a->bank_id = temp_a ^ temp_b;		/* this should strip out the bank address */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> row_addr_depth;
		temp_a  = input_a << row_addr_depth;
		this_a->row_id = temp_a ^ temp_b;		/* this should strip out the row address */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> rank_addr_depth;
		temp_a  = input_a << rank_addr_depth;
		this_a->rank_id = temp_a ^ temp_b;		/* this should strip out the rank address */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> chan_addr_depth;
		temp_a  = input_a << chan_addr_depth;
		this_a->chan_id = temp_a ^ temp_b;     		/* strip out the channel address */

	} else if(this_c->pa_mapping_policy == CLOSE_PAGE_EXPANDABLE){		/* k:l:r:n:b:z */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> bank_addr_depth;
		temp_a  = input_a << bank_addr_depth;
		this_a->bank_id = temp_a ^ temp_b;		/* this should strip out the bank address */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> col_addr_depth;
		temp_a  = input_a << col_addr_depth;
		this_a->col_id = temp_a ^ temp_b;     		/* strip out the column address */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> row_addr_depth;
		temp_a  = input_a << row_addr_depth;
		this_a->row_id = temp_a ^ temp_b;		/* this should strip out the row address */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> rank_addr_depth;
		temp_a  = input_a << rank_addr_depth;
		this_a->rank_id = temp_a ^ temp_b;		/* this should strip out the rank address */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> chan_addr_depth;
		temp_a  = input_a << chan_addr_depth;
		this_a->chan_id = temp_a ^ temp_b;     		/* strip out the channel address */

	} else if(this_c->pa_mapping_policy == OPEN_PAGE_PERMUTE){			/* r:l:b^l[b-1:0]:n:k:z */
		
		temp_b = input_a;				/* save away original address */
		input_a = input_a >> chan_addr_depth;
		temp_a  = input_a << chan_addr_depth;
		this_a->chan_id = temp_a ^ temp_b;     		/* strip out the channel address */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> col_addr_depth;
		temp_a  = input_a << col_addr_depth;
		this_a->col_id = temp_a ^ temp_b;     		/* strip out the column address */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> bank_addr_depth;
		temp_a  = input_a << bank_addr_depth;
		this_a->bank_id = temp_a ^ temp_b;		/* this should strip out the bank address */

		temp_b = input_a >> bank_addr_depth;
		temp_c = temp_b << bank_addr_depth;
		xor_bank_id = input_a ^ temp_c;			/* this strips out the address bits to be XOR'ed against bank_id */
		this_a->bank_id = this_a->bank_id ^ xor_bank_id; /* permute and get new bank_id */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> rank_addr_depth;
		temp_a  = input_a << rank_addr_depth;
		this_a->rank_id = temp_a ^ temp_b;		/* this should strip out the rank address */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> row_addr_depth;
		temp_a  = input_a << row_addr_depth;
		this_a->row_id = temp_a ^ temp_b;		/* this should strip out the row address */

	} else if(this_c->pa_mapping_policy == CLOSE_PAGE_PERMUTE){		/* r:n:l:b^l[b-1:0]:k:z */
		
		temp_b = input_a;				/* save away original address */
		input_a = input_a >> chan_addr_depth;
		temp_a  = input_a << chan_addr_depth;
		this_a->chan_id = temp_a ^ temp_b;     		/* strip out the channel address */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> bank_addr_depth;
		temp_a  = input_a << bank_addr_depth;
		this_a->bank_id = temp_a ^ temp_b;		/* this should strip out the bank address */

		temp_b = input_a >> bank_addr_depth;
		temp_c = temp_b << bank_addr_depth;
		xor_bank_id = input_a ^ temp_c;			/* this strips out the address bits to be XOR'ed against bank_id */
		this_a->bank_id = this_a->bank_id ^ xor_bank_id; /* permute and get new bank_id */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> rank_addr_depth;
		temp_a  = input_a << rank_addr_depth;
		this_a->rank_id = temp_a ^ temp_b;		/* this should strip out the rank address */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> col_addr_depth;
		temp_a  = input_a << col_addr_depth;
		this_a->col_id = temp_a ^ temp_b;     		/* strip out the column address */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> row_addr_depth;
		temp_a  = input_a << row_addr_depth;
		this_a->row_id = temp_a ^ temp_b;		/* this should strip out the row address */

	} else if(this_c->pa_mapping_policy == OPEN_PAGE_EXPANDABLE_PERMUTE){	/* k:l:r:b^r[b-1:0]:n:z */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> col_addr_depth;
		temp_a  = input_a << col_addr_depth;
		this_a->col_id = temp_a ^ temp_b;     		/* strip out the column address */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> bank_addr_depth;
		temp_a  = input_a << bank_addr_depth;
		this_a->bank_id = temp_a ^ temp_b;		/* this should strip out the bank address */

		temp_b = input_a >> bank_addr_depth;
		temp_c = temp_b << bank_addr_depth;
		xor_bank_id = input_a ^ temp_c;			/* this strips out the address bits to be XOR'ed against bank_id */
		this_a->bank_id = this_a->bank_id ^ xor_bank_id; /* permute and get new bank_id */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> row_addr_depth;
		temp_a  = input_a << row_addr_depth;
		this_a->row_id = temp_a ^ temp_b;		/* this should strip out the row address */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> rank_addr_depth;
		temp_a  = input_a << rank_addr_depth;
		this_a->rank_id = temp_a ^ temp_b;		/* this should strip out the rank address */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> chan_addr_depth;
		temp_a  = input_a << chan_addr_depth;
		this_a->chan_id = temp_a ^ temp_b;     		/* strip out the channel address */

	} else if(this_c->pa_mapping_policy == CLOSE_PAGE_EXPANDABLE_PERMUTE){	/* k:l:r:n:b^n[b-1:0]:z */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> bank_addr_depth;
		temp_a  = input_a << bank_addr_depth;
		this_a->bank_id = temp_a ^ temp_b;		/* this should strip out the bank address */

		temp_b = input_a >> bank_addr_depth;
		temp_c = temp_b << bank_addr_depth;
		xor_bank_id = input_a ^ temp_c;			/* this strips out the address bits to be XOR'ed against bank_id */
		this_a->bank_id = this_a->bank_id ^ xor_bank_id; /* permute and get new bank_id */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> col_addr_depth;
		temp_a  = input_a << col_addr_depth;
		this_a->col_id = temp_a ^ temp_b;     		/* strip out the column address */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> row_addr_depth;
		temp_a  = input_a << row_addr_depth;
		this_a->row_id = temp_a ^ temp_b;		/* this should strip out the row address */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> rank_addr_depth;
		temp_a  = input_a << rank_addr_depth;
		this_a->rank_id = temp_a ^ temp_b;		/* this should strip out the rank address */

		temp_b = input_a;				/* save away original address */
		input_a = input_a >> chan_addr_depth;
		temp_a  = input_a << chan_addr_depth;
		this_a->chan_id = temp_a ^ temp_b;     		/* strip out the channel address */

	}
}

void print_addresses(	addresses_t     *this_a){
	fprintf(stderr,"virtual[0x%X] physical[0x%X] chan[%X] rank[%X] bank[%X] row[%X] col[%X]\n",
				this_a->virtual_address,
				this_a->physical_address,
				this_a->chan_id,
				this_a->rank_id,
				this_a->bank_id,
				this_a->row_id,
				this_a->col_id);
}
int intlog2(int input){
	int result ;
	result = INVALID;
	if(input == 1){
		result = 0;
	} else if (input == 2){
		result = 1;
	} else if (input == 4){
		result = 2;
	} else if (input == 8){
		result = 3;
	} else if (input == 16){
		result = 4;
	} else if (input == 32){
		result = 5;
	} else if (input == 64){
		result = 6;
	} else if (input == 128){
		result = 7;
	} else if (input == 256){
		result = 8;
	} else if (input == 512){
		result = 9;
	} else if (input == 1024){
		result = 10;
	} else if (input == 2048){
		result = 11;
	} else if (input == 4096){
		result = 12;
	} else if (input == 8192){
		result = 13;
	} else if (input == 16384){
		result = 14;
	} else if (input == 32768){
		result = 15;
	} else if (input == 65536){
		result = 16;
	} else if (input == 65536*2){
		result = 17;
	} else if (input == 65536*4){
		result = 18;
	} else if (input == 65536*8){
		result = 19;
	} else if (input == 65536*16){
		result = 20;
	} else if (input == 65536*32){
		result = 21;
	} else if (input == 65536*64){
		result = 22;
	} else if (input == 65536*128){
		result = 23;
	} else if (input == 65536*256){
		result = 24;
	} else if (input == 65536*512){
		result = 25;
	} else if (input == 65536*1024){
		result = 26;
	} else if (input == 65536*2048){
		result = 27;
	} else if (input == 65536*4096){
		result = 28;
	} else if (input == 65536*8192){
		result = 29;
	} else if (input == 65536*16384){
		result = 30;
	} else {
		fprintf(stderr,"Error, %d is not a nice power of 2\n",input);
		_exit(2);
	}
	return result;
}

/*
 * Init DRAM system
 */

void init_dram_system(dram_system_t *this_ds){
	int i,j,k;
	init_dram_system_configuration(&(this_ds->config));
	for(i=0;i<MAX_CHANNEL_COUNT;i++){
		init_dram_controller(&(this_ds->channel[i]));
		for(j=0;j<MAX_RANK_COUNT;j++){
			for(k=0;k<MAX_BANK_COUNT;k++){
				init_transaction_queue(&(this_ds->transaction_queue[i][j][k]));
			}
		}
	}
	this_ds->last_event_type = MEM_RD;
	this_ds->last_chan_id = 0;
	this_ds->last_rank_id = 0;
	this_ds->last_bank_id = 0;
}

void init_dram_system_configuration(dram_system_configuration_t *this_c){

	this_c->data_rate			= 1000; 		/* 1 Gbit per sec */
	this_c->row_buffer_management_policy	= CLOSE_PAGE;
	this_c->va_mapping_policy		= VA_EQUATE;
	this_c->pa_mapping_policy		= CLOSE_PAGE_BASELINE;
	this_c->command_ordering_policy		= STRICT_ORDER;
	this_c->transaction_ordering_policy	= STRICT_ORDER;
	this_c->refresh_policy			= NO_REFRESH;

	this_c->chan_count			= 1;
	this_c->rank_count			= 1;
	this_c->bank_count			= 16;
	this_c->row_count			= 16384;
	this_c->cachelines_per_row		= INVALID;		/* not used now, deliberately set to INVALID */
	this_c->col_count			= 1024;
	this_c->col_size			= INVALID;
	this_c->row_size			= INVALID;
	this_c->cacheline_size			= 64;
	this_c->TLB_pagesize			= 4096;
	this_c->transaction_queue_depth		= 4;

	/* all timing parameters in terms of cycles */

	this_c->t_ras				= 36;		/* 45 ns @ 1.25ns per cycle = 36 cycles */
	this_c->t_rcd				= 8;
	this_c->t_cas				= 8;
	this_c->t_rp				= 10;		/* 12 ns @ 1.25ns per cycle = 9.6 cycles */
	this_c->t_rc				= 46;		/* 57 ns @ 1.25ns per cycle = 45.6 cycles */
	this_c->t_cwd				= 6;
	this_c->t_al				= 14;
	this_c->t_rl				= 16;
	this_c->t_dqs				= 2;
	this_c->t_burst				= 8;
	this_c->d_m 				= 6;
	this_c->max_open_bank_count		= 16;		/* no limit */
}

void init_transaction_queue(transaction_queue_t *this_t){
	int i;
	/* initialize entries in the transaction queue */
	this_t->transaction_count		= 0;
	for(i=0;i<MAX_TRANSACTION_QUEUE_DEPTH;i++){
		this_t->entry[i].status 	= INVALID;
	}
}

void init_dram_controller(dram_controller_t *this_dc){
	int i,j;
	rank_t *this_r;

	/* initialize all ranks */
	for(i=0;i<MAX_RANK_COUNT;i++){
		this_r = &(this_dc->rank[i]);
		for(j=0;j<MAX_BANK_COUNT;j++){
			this_r->bank[j].status 			= IDLE;
			this_r->bank[j].row_id 			= INVALID;
			this_r->bank[j].age 			= 0;
		}
	}
	this_dc->refresh_row_index 		= 0;
	this_dc->last_refresh_cycle 		= 0;
}

void print_dram_system_state(dram_system_t *this_ds){
	rank_t *this_r;
	int i,j,k,chan_count,rank_count,bank_count;

	chan_count 	= this_ds->config.chan_count;
	rank_count 	= this_ds->config.rank_count;
	bank_count 	= this_ds->config.bank_count;

	for(i=0;i<chan_count;i++){
		for(j=0;j<rank_count;j++){
			this_r = &(this_ds->channel[i].rank[j]);
			for(k=0;k<bank_count;k++){
				fprintf(stderr,"controller[%2d] rank [%2d] bank[%2x] status[", i, j, k);
				print_status(this_r->bank[k].status);
				fprintf(stderr,"] row_id[0x%8x]\n", this_r->bank[k].row_id); 
			}
		}
	}
}


void print_status(int status){
        switch(status) {
		case IDLE:
			fprintf(stderr,"IDLE");
		break;
		case ACTIVE:
			fprintf(stderr,"ACT ");
		break;
		default:
			fprintf(stderr,"UNKN");
		break;
	}
}

void    init_dram_stats(STATS *stats){
	int i,j,k,l,m,n;
	STAT_BUCKET *this_b;
	STAT_BUCKET *temp_b;

	for(i=0;i<STAT_EVENT_TYPE_COUNT;i++){
		stats->event_count[i] = 0;
		for(j=0;j<STAT_EVENT_TYPE_COUNT;j++){
			for(k=0;k<MAX_CHANNEL_STATE;k++){
				for(l=0;l<MAX_RANK_STATE;l++){
					for(m=0;m<MAX_BANK_STATE;m++){
						for(n=0;n<MAX_ROW_STATE;n++){
							stats->count[i][j][k][l][m][n] = 0;
						}
					}
				}
			}
		}
	}
	for(i=0;i<MAX_ACCESS_DISTANCE_RANGE;i++){
		stats->access_distance[i] = 0;
		stats->idling_distance[i] = 0;
		stats->idling_distances_history[i] = 0;
	}
}

int compute_idling_distance(STATS *stats, int minimum_distance, int request_distance, int max_open_bank_count){
	int j;
	int sum_in_min_distance_range_previous_idling_distances;
	int sum_in_tFAW_distance_range_previous_idling_distances;
	int required_idling_distance;
	int additional_idling_distance;		/* due to bank restrictions */
	int total_distance;
	int request_distance_range;
	int min_idling_distance;
	int min_idling_distance_range;
	int total_idling_distance;

	min_idling_distance = MAX(0, minimum_distance + 1 - max_open_bank_count);	/* every tRC, we need to idle this much  D_i-min*/ 
	min_idling_distance_range = MAX(0, minimum_distance - min_idling_distance);

	sum_in_min_distance_range_previous_idling_distances = 0;
	sum_in_tFAW_distance_range_previous_idling_distances = 0;
	request_distance_range = MIN(request_distance,MAX_ACCESS_DISTANCE_RANGE);

	for(j = 0; j < request_distance_range; j++){
		sum_in_min_distance_range_previous_idling_distances += stats->idling_distances_history[j];
	}
	for(j = 0; j < min_idling_distance_range; j++){
		sum_in_tFAW_distance_range_previous_idling_distances += stats->idling_distances_history[j];
	}
	required_idling_distance = MAX(0,minimum_distance - (request_distance + sum_in_min_distance_range_previous_idling_distances));
	sum_in_tFAW_distance_range_previous_idling_distances += required_idling_distance;
	additional_idling_distance = MAX(0, min_idling_distance - sum_in_tFAW_distance_range_previous_idling_distances);
	total_idling_distance = required_idling_distance + additional_idling_distance;
	/*
	if(additional_idling_distance > 0){
		fprintf(stderr,"min[%d] max_open[%d] req[%d] idle[%d] add_idle[%d] total_idle[%d]\n",
			minimum_distance,
			max_open_bank_count,
			request_distance,
			idling_distance,
			additional_idling_distance,
			total_idling_distance);
	}
	*/
	return total_idling_distance;
}

void add_idling_distance(STATS *stats, int distance){
	int i;
	for(i=MAX_ACCESS_DISTANCE_RANGE-1;i>0;i--){
		stats->idling_distances_history[i] = stats->idling_distances_history[i-1];
	}
	stats->idling_distance[distance]++;			/* increment the idling distance count */
	stats->idling_distances_history[0] = distance;
}

void add_transaction(dram_system_t *this_ds, transaction_t *this_t){
	transaction_queue_t *this_q;
	this_q = &(this_ds->transaction_queue[this_t->addr.chan_id][this_t->addr.rank_id][this_t->addr.bank_id]);

	this_q->entry[this_q->transaction_count].status 		= ACTIVE;
	this_q->entry[this_q->transaction_count].type 			= this_t->type;
	this_q->entry[this_q->transaction_count].event_no 		= this_t->event_no;
	this_q->entry[this_q->transaction_count].addr.virtual_address	= this_t->addr.virtual_address;
	this_q->entry[this_q->transaction_count].addr.physical_address	= this_t->addr.physical_address;
	this_q->entry[this_q->transaction_count].addr.chan_id		= this_t->addr.chan_id;
	this_q->entry[this_q->transaction_count].addr.rank_id		= this_t->addr.rank_id;
	this_q->entry[this_q->transaction_count].addr.bank_id		= this_t->addr.bank_id;
	this_q->entry[this_q->transaction_count].addr.row_id		= this_t->addr.row_id;
	this_q->entry[this_q->transaction_count].addr.col_id		= this_t->addr.col_id;
	this_q->transaction_count++;
}

/* removes the zeroth entry, then moves everything down */

void remove_transaction(dram_system_t *this_ds, transaction_t *this_t){
	int i;
	int found;
	transaction_queue_t *this_q;

	this_ds->last_event_type = this_t->type;
	this_ds->last_chan_id = this_t->addr.chan_id;
	this_ds->last_rank_id = this_t->addr.rank_id;
	this_ds->last_bank_id = this_t->addr.bank_id;
	found = FALSE;
	this_q = &(this_ds->transaction_queue[this_t->addr.chan_id][this_t->addr.rank_id][this_t->addr.bank_id]);
	for(i=0;i<this_q->transaction_count;i++){
		if((this_q->entry[i].addr.virtual_address == this_t->addr.virtual_address) && (this_q->entry[i].type == this_t->type)){
			found = TRUE;
		}
		if(found == TRUE){
			this_q->entry[i].status 		= this_q->entry[i+1].status;
			this_q->entry[i].type 			= this_q->entry[i+1].type;
			this_q->entry[i].addr.virtual_address	= this_q->entry[i+1].addr.virtual_address;
			this_q->entry[i].addr.physical_address	= this_q->entry[i+1].addr.physical_address;
			this_q->entry[i].addr.chan_id		= this_q->entry[i+1].addr.chan_id;
			this_q->entry[i].addr.rank_id		= this_q->entry[i+1].addr.rank_id;
			this_q->entry[i].addr.bank_id		= this_q->entry[i+1].addr.bank_id;
			this_q->entry[i].addr.row_id		= this_q->entry[i+1].addr.row_id;
			this_q->entry[i].addr.col_id		= this_q->entry[i+1].addr.col_id;
		}
	}
	this_q->transaction_count--;
}

transaction_t *get_next_transaction(dram_system_t *this_ds){
	int chan_id;
	int rank_id;
	int bank_id;
	int i,j,k,l;
	int found;
	transaction_t *next_t;
	found = FALSE;
	chan_id = this_ds->last_chan_id;
	rank_id = this_ds->last_rank_id;
	bank_id = this_ds->last_bank_id;
	for(i=0;i<MAX_CHANNEL_COUNT;i++){
		for(j=0;j<MAX_RANK_COUNT;j++){
			for(k=0;k<MAX_BANK_COUNT;k++){
				bank_id = (bank_id + 1) % this_ds->config.bank_count;
				if(this_ds->transaction_queue[chan_id][rank_id][bank_id].transaction_count > 0){
					if(this_ds->config.row_buffer_management_policy == OPEN_PAGE){ /* open */
						for(l=0;l<this_ds->transaction_queue[chan_id][rank_id][bank_id].transaction_count;l++){
							if(this_ds->transaction_queue[chan_id][rank_id][bank_id].entry[l].addr.row_id ==
								this_ds->channel[chan_id].rank[rank_id].bank[bank_id].row_id){
								return &(this_ds->transaction_queue[chan_id][rank_id][bank_id].entry[l]);
							}
						}
						return &(this_ds->transaction_queue[chan_id][rank_id][bank_id].entry[0]);
					} else if (this_ds->config.row_buffer_management_policy == CLOSE_PAGE) { /* close page policy */
						return &(this_ds->transaction_queue[chan_id][rank_id][bank_id].entry[0]);
					}
				}
			}
			rank_id = (rank_id + 1) % this_ds->config.rank_count;
		}
		chan_id = (chan_id + 1) % this_ds->config.chan_count;
	}
	return NULL;
}

/* if any queue has reached saturation, the queues are said to be full */

int queue_check(dram_system_t *this_ds, int check_type){
	int i,j,k;
	
	if(check_type == IS_QUEUE_FULL){
		for(i=0;i<MAX_CHANNEL_COUNT;i++){
			for(j=0;j<MAX_RANK_COUNT;j++){
				for(k=0;k<MAX_BANK_COUNT;k++){
					if(this_ds->transaction_queue[i][j][k].transaction_count >= this_ds->config.transaction_queue_depth){
						return TRUE;
					}	
				}
			}
		}
		return FALSE;
	} else if (check_type == IS_QUEUE_EMPTY){
		for(i=0;i<MAX_CHANNEL_COUNT;i++){
			for(j=0;j<MAX_RANK_COUNT;j++){
				for(k=0;k<MAX_BANK_COUNT;k++){
					if(this_ds->transaction_queue[i][j][k].transaction_count > 0){
						return FALSE;
					}	
				}
			}
		}
		return TRUE;
	}
	return FALSE;
}

void get_dram_states(dram_system_t *this_ds, transaction_t *this_t, dram_states_t *this_s){
	if(this_t->addr.chan_id != this_ds->last_chan_id){
		this_s->chan_state 	= DIFFERENT;
		this_s->rank_state	= DONTCARE;
		this_s->bank_state	= DONTCARE;
		this_s->row_state	= DONTCARE;
	} else {
		this_s->chan_state = SAME;
		if(this_t->addr.rank_id != this_ds->last_rank_id){
			this_s->rank_state 	= DIFFERENT;
			this_s->bank_state	= DONTCARE;
		} else {
			this_s->rank_state = SAME;
			if(this_t->addr.bank_id != this_ds->last_bank_id){
				this_s->bank_state	= DIFFERENT;
			} else {
				this_s->bank_state	= SAME;
			}
		}
		if(this_ds->channel[this_t->addr.chan_id].rank[this_t->addr.rank_id].bank[this_t->addr.bank_id].status == OPEN){
			if(this_t->addr.row_id != this_ds->channel[this_t->addr.chan_id].rank[this_t->addr.rank_id].bank[this_t->addr.bank_id].row_id){
				this_s->row_state = CONFLICT;
			} else {
				this_s->row_state = OPEN;
			}
		} else {
			this_s->row_state = CLOSED;
		}
	}
}

void print_event(FILE *fout, int event){
        switch(event) {
		case MEM_RD:
			fprintf(fout,"RD ");
		break;
		case MEM_WR:
			fprintf(fout,"WR ");
		break;
		default:
			fprintf(fout,"?? ");
		break;
	}
}

void print_state(FILE *fout, int state){
        switch(state) {
		case SAME:
			fprintf(fout,"S ");
		break;
		case DIFFERENT:
			fprintf(fout,"D ");
		break;
		case DONTCARE:
			fprintf(fout,"X ");
		break;
		default:
			fprintf(fout,"? ");
		break;
	}
}

void print_row_state(FILE *fout, int state){
        switch(state) {
		case OPEN:
			fprintf(fout,"O ");
		break;
		case CLOSED:
			fprintf(fout,"C ");
		break;
		case CONFLICT:
			fprintf(fout,"K ");
		break;
		case DONTCARE:
			fprintf(fout,"X ");
		break;
		default:
			fprintf(fout,"? ");
		break;
	}
}


