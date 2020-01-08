#include "btv.h"

/* draw legends */

void 
btv_draw_legends(  Tk_Window tkwin,
		Drawable d,
		Tk_3DBorder *Border_array,
		GC *GC_array,
		GEOMETRY *geometry,
		Tk_Font font,
		int *display_modes,
		int *color,
		TRACE *trace)
{
	GC	gc;
	XPoint	my_array[100];
	int	height;
	char	buffer[100];
	int	i,win_height,win_width;
	int	legend_width,legend_height;
	int	trh_x,trh_y;	/* specifies the top right hand corner of legend in x-coordinates */
	int	allow_draw;
	int 	event_type_count;

	win_width  = geometry->win_width;
	win_height = geometry->win_height;
	trh_x = 0;
	trh_y = 0;
	if(trace->trace_type == K6_TRACE){
		legend_height = 210;
		legend_width = 80;
		event_type_count = EVENT_TYPE_COUNT;
	} else if(trace->trace_type == TM_TRACE){
		legend_height = 210;
		legend_width = 100;
		event_type_count = EVENT_TYPE_COUNT;
	} else if(trace->trace_type == MASE_TRACE){
		legend_height = 70;
		legend_width = 80;
		event_type_count = 3;
	} else if(trace->trace_type == SS_TRACE){
		if(display_modes[COLOR_SCHEME] == COLOR_BY_ACCESS_TYPE){
			legend_height = 100;
		} else if (display_modes[COLOR_SCHEME] == COLOR_BY_THREAD){
			legend_height = 120;
		}
		legend_width = 100;
	} 
	
	/* draw background to cover everything */

	my_array[0].x = win_width - trh_x;
	my_array[0].y = trh_y;
	my_array[1].x = win_width - trh_x;
	my_array[1].y = trh_y + legend_height;
	my_array[2].x = win_width - trh_x - legend_width;
	my_array[2].y = trh_y + legend_height;
	my_array[3].x = win_width - trh_x - legend_width;
	my_array[3].y = trh_y;

	Tk_Fill3DPolygon(tkwin,
		d,
		Border_array[color[BACKGROUND]],
		my_array,
		4,				
		2,			
		TK_RELIEF_SOLID);
	if((trace->trace_type == K6_TRACE) || (trace->trace_type == TM_TRACE) || (trace->trace_type == MASE_TRACE)){
		for(i = 0; i < event_type_count;i++){
			switch(i) {
				case FETCH:
					sprintf(buffer,"FETCH  ");
				break;
				case MEM_RD:
					sprintf(buffer,"MEM_RD ");
				break;
				case MEM_WR:
					sprintf(buffer,"MEM_WR ");
				break;
				case IO_RD:
					sprintf(buffer,"IO_RD  ");
				break;
				case IO_WR:
					sprintf(buffer,"IO_WR  ");
				break;
				case BOFF:
					sprintf(buffer,"BOFF   ");
				break;
				case LOCK_RD:
					sprintf(buffer,"LOCK_RD");
				break;
				case LOCK_WR:
					sprintf(buffer,"LOCK_WR");
				break;
				case INT_ACK:
					sprintf(buffer,"INT_ACK");
				break;
				case UNKNOWN:
					sprintf(buffer,"UNKNOWN");
				break;
			}
			if(display_modes[SHOW_LEGENDS] != SUPRESS_TEXT){
				Tk_DrawChars(Tk_Display(tkwin),
					d,
					GC_array[color[GRID]],
					font,
					buffer,
					strlen(buffer),
					win_width - trh_x - 80,
					(i + 1) * 20 + trh_y);
			}
			my_array[0].x = win_width - trh_x - 90;
			my_array[1].x = win_width - trh_x - 90;
			my_array[2].x = win_width - trh_x - legend_width - 30;
			my_array[3].x = win_width - trh_x - legend_width - 30;
			my_array[0].y = (i + 1) * 20 + trh_y - 10;
			my_array[1].y = (i + 1) * 20 + trh_y;
			my_array[2].y = (i + 1) * 20 + trh_y;
			my_array[3].y = (i + 1) * 20 + trh_y - 10;
			Tk_Fill3DPolygon(tkwin,
				d,
				Border_array[color[i]],
				my_array,
				4,				
				2,			
				TK_RELIEF_RAISED);
		}
	} else if (trace->trace_type == SS_TRACE){
		if(display_modes[SHOW_LEGENDS] != SUPRESS_TEXT){
			if(display_modes[SHOW_THREAD_NUMBER] == ALL_THREADS ){
				sprintf(buffer,"All Threads ");
			} else {
				sprintf(buffer,"Thread %d",display_modes[SHOW_THREAD_NUMBER]);
			}
			Tk_DrawChars(Tk_Display(tkwin),
				d,
				GC_array[color[GRID]],
				font,
				buffer,
				strlen(buffer),
				win_width - trh_x - 100,
				20 + trh_y);
		}
		for(i = 0; i < MAX_THREAD_COUNT;i++){
			if(display_modes[COLOR_SCHEME] == COLOR_BY_ACCESS_TYPE){
				switch(i) {
					case MEM_RD:
						sprintf(buffer,"MEM_RD ");
						allow_draw = TRUE;
					break;
					case MEM_WR:
						sprintf(buffer,"MEM_WR ");
						allow_draw = TRUE;
					break;
					default:
						allow_draw = FALSE;
					break;
				}
			} else if(display_modes[COLOR_SCHEME] == COLOR_BY_THREAD){
				sprintf(buffer,"Thread %d",i);
				allow_draw = TRUE;
			}
			my_array[0].x = win_width - trh_x - 60;
			my_array[1].x = win_width - trh_x - 60;
			my_array[2].x = win_width - trh_x - legend_width;
			my_array[3].x = win_width - trh_x - legend_width;
			my_array[0].y = legend_height - (i + 1) * 20 + trh_y - 10;
			my_array[1].y = legend_height - (i + 1) * 20 + trh_y;
			my_array[2].y = legend_height - (i + 1) * 20 + trh_y;
			my_array[3].y = legend_height - (i + 1) * 20 + trh_y - 10;
			if(allow_draw == TRUE){
				if(display_modes[SHOW_LEGENDS] != SUPRESS_TEXT){
					Tk_DrawChars(Tk_Display(tkwin),
						d,
						GC_array[color[GRID]],
						font,
						buffer,
						strlen(buffer),
						win_width - trh_x - 50,
						legend_height - (i + 1) * 20 + trh_y);
				}
				Tk_Fill3DPolygon(tkwin,
					d,
					Border_array[color[i]],
					my_array,
					4,				
					2,			
					TK_RELIEF_RAISED);
			}
		}
	}
}

/* draw labels */

void 
btv_draw_labels(  Tk_Window tkwin,
		Drawable d,
		Tk_3DBorder *Border_array,
		GC *GC_array,
		GEOMETRY *geometry,
		Tk_Font font,
		int *display_modes,
		int *color,
		TRACE *trace)
{
	XPoint	vertical_line[2],horizontal_line[2],my_array[100];
	double	timestamp;
	double	start_time;
	int	height;
	float	fp_height;
	char	buffer[100];
	int	i,win_height,win_width;

	/* units are in nanoseconds */

	start_time 	= (geometry->origin_x * -1.0 * geometry->x_axis_scale + trace->start_time );

	win_width  = geometry->win_width;
	win_height = geometry->win_height;

	horizontal_line[0].x = geometry->offset_x - 2 ;
	horizontal_line[0].y = geometry->win_height - geometry->offset_y + 2;
	horizontal_line[1].x = geometry->win_width - 1;
	horizontal_line[1].y = horizontal_line[0].y;
	/* BUG ? */
	/*
	fprintf(stderr,"%d %d\n",horizontal_line[0].x,horizontal_line[1].x);
	Tk_Draw3DPolygon(tkwin,
		d,
		Border_array[color[GRID]],
		horizontal_line,
		2,		
		1,	
		TK_RELIEF_SOLID);
		*/
	XDrawLine(Tk_Display(tkwin),
			d,
			GC_array[color[GRID]],
			horizontal_line[0].x,
			horizontal_line[0].y,
			horizontal_line[1].x,
			horizontal_line[1].y);
	vertical_line[0].x = geometry->offset_x - 2;
	vertical_line[0].y = 0;
	vertical_line[1].x = vertical_line[0].x;
	vertical_line[1].y = horizontal_line[1].y;
	Tk_Draw3DPolygon(tkwin,
		d,
		Border_array[color[GRID]],
		vertical_line,
		2,				
		1,			
		TK_RELIEF_SOLID);
	if((trace->trace_type == SS_TRACE) && (display_modes[SHOW_BW_LIMIT])){
		horizontal_line[0].x = geometry->offset_x - 2 ;
		horizontal_line[0].y = win_height - 25000.0/geometry->y_axis_scale 
					- geometry->origin_y - geometry->offset_y;
		horizontal_line[1].x = geometry->win_width;
		horizontal_line[1].y = horizontal_line[0].y;
		Tk_Draw3DPolygon(tkwin,
			d,
			Border_array[color[GRID]],
			horizontal_line,
			2,		
			1,	
			TK_RELIEF_SOLID);
	}
	if(trace->trace_type == K6_TRACE){
		sprintf(buffer,"x scale - %d us per pixel width",geometry->x_axis_scale/1000);
	} else if(trace->trace_type == MASE_TRACE){
		sprintf(buffer,"x scale - %d CPU ticks per pixel width",geometry->x_axis_scale);
	} else if(trace->trace_type == TM_TRACE){
		sprintf(buffer,"x scale - %d CPU ticks per pixel width",geometry->x_axis_scale);
	} else if(trace->trace_type == SS_TRACE){
		sprintf(buffer,"x scale - %d us per pixel width",geometry->x_axis_scale/1000);
	}
	if(display_modes[SHOW_LABELS] != SUPRESS_TEXT){
		Tk_DrawChars(Tk_Display(tkwin),
			d,
			GC_array[color[GRID]],
			font,
			buffer,
			strlen(buffer),
			80,
			geometry->win_height - 20);
	}
	sprintf(buffer,"Time");
	if((display_modes[SHOW_LABELS] != SUPRESS_TEXT) && (geometry->win_width > 500)){
		Tk_DrawChars(Tk_Display(tkwin),
			d,
			GC_array[color[GRID]],
			font,
			buffer,
			strlen(buffer),
			geometry->win_width - 120,
			geometry->win_height - 15);
		XDrawLine(Tk_Display(tkwin),
			d,
			GC_array[color[GRID]],
			geometry->win_width - 80,
			win_height - 20,
			geometry->win_width - 20,
			win_height - 20);
		my_array[0].x =  geometry->win_width - 12;
		my_array[0].y =  win_height - 20 ;
		my_array[1].x =  geometry->win_width - 20;
		my_array[1].y =  win_height - 24;
		my_array[2].x =  geometry->win_width - 20;
		my_array[2].y =  win_height - 16;
		Tk_Fill3DPolygon(tkwin,
			d,
			Border_array[color[GRID]],
			my_array,
			3,
			1,	
			TK_RELIEF_SOLID);
	}
	if(geometry->y_axis_scale == 1){
		sprintf(buffer,"y scale - %d transaction per pixel height",geometry->y_axis_scale);
	} else {
		sprintf(buffer,"y scale - %d transactions per pixel height",geometry->y_axis_scale);
	}
	if(display_modes[SHOW_LABELS] != SUPRESS_TEXT){
		Tk_DrawChars(Tk_Display(tkwin),
			d,
			GC_array[color[GRID]],
			font,
			buffer,
			strlen(buffer),
			80,
			geometry->win_height - 5);
	}

	for(i= geometry->offset_y;i<win_height;i+=100){	/* every 100 pixels, give a indicator */
		if(trace->trace_type == SS_TRACE){
			fp_height =  (i - geometry->origin_y - geometry->offset_y) * 
					geometry->y_axis_scale * 0.064;
			sprintf(buffer,"%5.1f",fp_height);
		} else {
			height =  (i - geometry->origin_y - geometry->offset_y) * (geometry->y_axis_scale) ;
			sprintf(buffer,"%5d",height);
		}
		if(display_modes[SHOW_LABELS] != SUPRESS_TEXT){	
			Tk_DrawChars(Tk_Display(tkwin),
				d,
				GC_array[color[GRID]],
				font,
				buffer,
				strlen(buffer),
				2,
				geometry->win_height - i + 5);
			if(trace->trace_type == SS_TRACE ){
				sprintf(buffer,"MB/s");
				Tk_DrawChars(Tk_Display(tkwin),
					d,
					GC_array[color[GRID]],
					font,
					buffer,
					strlen(buffer),
					5,
					geometry->win_height - i + 17);
			}
		}
		my_array[0].x =  59;
		my_array[0].y =  win_height - i ;
		my_array[1].x =  52;
		my_array[1].y =  win_height - i+2;
		my_array[2].x =  52;
		my_array[2].y =  win_height - i-2;
		Tk_Fill3DPolygon(tkwin,
			d,
			Border_array[color[GRID]],
			my_array,
			3,
			1,	
			TK_RELIEF_SOLID);
	}
	for(i=geometry->offset_x;i<win_width;i+=100){	/* every 100 pixels, give a indicator */
		if((trace->trace_type == K6_TRACE) || (trace->trace_type == SS_TRACE)){
			timestamp =  (i - geometry->offset_x) * geometry->x_axis_scale + start_time;
			if(timestamp < 1000.0){
				sprintf(buffer,"%8.2f ns",timestamp);
			} else if(timestamp < 1000000.0) {
				sprintf(buffer,"%8.2f us",timestamp*0.001);
			} else if(timestamp < 1000000000.0) {
				sprintf(buffer,"%8.2f ms",timestamp*0.000001);
			} else {
				sprintf(buffer,"%8.3f s",timestamp*0.000000001);
			}
		} else if((trace->trace_type == TM_TRACE) || (trace->trace_type == MASE_TRACE)){
			timestamp =  (double) (i - geometry->offset_x) * (double) geometry->x_axis_scale + start_time;
			sprintf(buffer,"%8.2e",timestamp);
		}
		if(display_modes[SHOW_LABELS] != SUPRESS_TEXT){
			Tk_DrawChars(Tk_Display(tkwin),
				d,
				GC_array[color[GRID]],
				font,
				buffer,
				strlen(buffer),
				i - 20,
				geometry->win_height - 39);
		}
		my_array[0].x =  i ;
		my_array[0].y =  geometry->win_height - 59;
		my_array[1].x =  i+2;
		my_array[1].y =  geometry->win_height - 52;
		my_array[2].x =  i-2;
		my_array[2].y =  geometry->win_height - 52;
		Tk_Fill3DPolygon(tkwin,
			d,
			Border_array[color[GRID]],
			my_array,
			3,		
			1,	
			TK_RELIEF_SOLID);
	}
}

void 
btv_draw_histogram(  Tk_Window tkwin,
		Drawable d,
		Tk_3DBorder *Border_array,
		GC *GC_array,
		GEOMETRY *geometry,
		Tk_Font font,
		int *display_modes,
		int *color,
		TRACE *trace)
{
	XPoint		my_array[100];
	GC              gc;
	int		i,j;
	int		win_width,win_height,x_start,y_start,y_end;
	int		end_drawing;
	int		event_count;
	int		segment_index,event_index;
	double		last_time;
	double		timestamp;
	double		start_time;
	double		bucket_endtime;
	BUS_Event 	*this_e;
	SS_BUS_Event	*ss_this_e;
	SS_BUS_Event	*ss_next_e;
	int		bucket_height[COLOR_MODE_COUNT];
	int		aggregate_height;
	int		origin_x,origin_y;
	int		offset_x,offset_y;
	int		found;
	int		win_height_minus_offset_y;
	int		lower_index,upper_index,temp_index,start_index,pixel_index;
	int		pix_x_axis_scale;
	int		color_index;
	int		line_length;

	/* units here are ns */
	pix_x_axis_scale = 1000000 / geometry->x_axis_scale;
	origin_x 	= geometry->origin_x;
	origin_y 	= geometry->origin_y;
	start_time 	= origin_x * -1.0 * geometry->x_axis_scale + trace->start_time;
	geometry->first_time = start_time;
	win_height 	= geometry->win_height;
	win_width 	= geometry->win_width;
	event_count 	= trace->event_count;
	aggregate_height = 0;

	if(display_modes[SHOW_LABELS]){
		offset_x = geometry -> offset_x;
		offset_y = geometry -> offset_y;
	} else {
		offset_x = 0;
		offset_y = 0;
	}

	for(i = 0; i < COLOR_MODE_COUNT;i++){
		bucket_height[i] = 0;
	}

	found = FALSE;
	lower_index 	= 0;
	upper_index 	= event_count - 1;

	if(trace->trace_type != SS_TRACE){
		while(!found){
			temp_index 	= (lower_index + upper_index)/2;
			segment_index 	= temp_index / MAX_SEGMENT_SIZE;
			event_index	= temp_index % MAX_SEGMENT_SIZE;
			this_e = &(trace->segment[segment_index]->event[event_index]);
	
			if((upper_index - lower_index) < 3){
				found = TRUE;
				start_index = lower_index;
			} else if(this_e->timestamp == start_time){
				found = TRUE;
				start_index = temp_index;
			} else if(this_e->timestamp > start_time){
				upper_index = temp_index;
			} else {
				lower_index = temp_index;
			}
		}
	} else {	/* sstrace */
		if(start_time <0.0){
			start_index = 0;
		} else if (start_time >= trace->end_time ) {
			start_index = event_count - 1;
		} else {
			start_index = (start_time/trace->end_time) * (event_count - 1);
		}
	}

	x_start = offset_x;
	win_height_minus_offset_y = win_height - offset_y;
	bucket_endtime = start_time + geometry->x_axis_scale;
	end_drawing = FALSE;

	if(trace->trace_type != SS_TRACE){
		for(i=start_index; i<event_count && !end_drawing; i++){
			segment_index 	= i / MAX_SEGMENT_SIZE;
			event_index	= i % MAX_SEGMENT_SIZE;
			this_e = &(trace->segment[segment_index]->event[event_index]);
			if(this_e->timestamp > bucket_endtime){	
				for(j = 0; j < EVENT_TYPE_COUNT;j++){
					if(color[j] != color[BACKGROUND]) {
						gc = GC_array[color[j]];
						/* This routine overdraws the labels if the histograms are allowed to
						 * float in the y axis.  Not a problem if y axis is locked down
						 */
	
						line_length = bucket_height[j] / geometry->y_axis_scale;
						aggregate_height +=line_length;
						if( x_start < win_width && x_start > offset_x) {
							y_start = win_height - aggregate_height - origin_y - offset_y;
							y_end = y_start + line_length;
							XDrawLine(Tk_Display(tkwin),
									d,
									gc,
									x_start,
									y_start,
									x_start,
									y_end);
						}
						bucket_height[j] = 0;

						/* 
						 * This routine does not overdraw the labels, since it draws point by point, 
						 * Each point could be checked Individually.
						 */
					       /*	
						while(bucket_height[j] > 0){
							bucket_height[j] = bucket_height[j] - geometry->y_axis_scale;
							aggregate_height++; 
							y_start = win_height - aggregate_height - origin_y - offset_y;
							if(	y_start < win_height_minus_offset_y && 
								y_start > 0 &&
								x_start < win_width &&
								x_start > offset_x) {
								XDrawPoint(Tk_Display(tkwin),
									d,
									gc,
									x_start,
									y_start);
							}
						}
						bucket_height[j] = 0;
						*/
					}
				}
				bucket_endtime += geometry->x_axis_scale;
				x_start++;
				if(x_start > win_width){
					end_drawing = TRUE;
				}
				aggregate_height = 0;
			} 
			bucket_height[this_e->attributes]++; 
		}
	} else {
		for(pixel_index=0; pixel_index < win_width && !end_drawing; pixel_index+= pix_x_axis_scale){
			bucket_height[0] = 0;
			segment_index   = (pixel_index/pix_x_axis_scale + start_index) / MAX_SEGMENT_SIZE;
                        event_index     = (pixel_index/pix_x_axis_scale + start_index) % MAX_SEGMENT_SIZE;
			if((segment_index*MAX_SEGMENT_SIZE + event_index) >= trace->event_count ){
				end_drawing = TRUE;
			}
			/*
			for(j=0;j<MAX_THREAD_COUNT && !end_drawing;j++){
				ss_this_e = &(trace->ss_segment[segment_index]->event[j][event_index]);
				my_array[0].x =  offset_x + pixel_index;
				my_array[0].y =  MIN(win_height_minus_offset_y - origin_y - bucket_height[0],
							win_height_minus_offset_y);
				my_array[1].x =  offset_x + pixel_index + pix_x_axis_scale;
				my_array[1].y =  MIN(win_height_minus_offset_y - origin_y - bucket_height[0],
							win_height_minus_offset_y);
				my_array[2].x =  offset_x + pixel_index + pix_x_axis_scale;
				my_array[2].y =  MIN(win_height_minus_offset_y - origin_y - bucket_height[0] -
							ss_this_e->mem_load_count/geometry->y_axis_scale,
							win_height_minus_offset_y);
				my_array[3].x =  offset_x + pixel_index;
				my_array[3].y =  MIN(win_height_minus_offset_y - origin_y - bucket_height[0] -
							ss_this_e->mem_load_count/geometry->y_axis_scale,
							win_height_minus_offset_y);
				if(display_modes[COLOR_SCHEME] == COLOR_BY_ACCESS_TYPE){
					color_index = MEM_RD;
				} else {
					color_index = j;
				}
				if((display_modes[SHOW_THREAD_NUMBER] == ALL_THREADS) ||
					(display_modes[SHOW_THREAD_NUMBER] == j)){
					Tk_Fill3DPolygon(tkwin,
						d,
						Border_array[color[color_index]],
						my_array,
						4,
						1,	
						TK_RELIEF_SOLID);
					bucket_height[0] += 
						(ss_this_e->mem_load_count / geometry->y_axis_scale);
				}
				my_array[0].x =  offset_x + pixel_index;
				my_array[0].y =  MIN(win_height_minus_offset_y - origin_y - bucket_height[0],
							win_height_minus_offset_y);
				my_array[1].x =  offset_x + pixel_index + pix_x_axis_scale;
				my_array[1].y =  MIN(win_height_minus_offset_y - origin_y - bucket_height[0],
							win_height_minus_offset_y);
				my_array[2].x =  offset_x + pixel_index + pix_x_axis_scale;
				my_array[2].y =  MIN(win_height_minus_offset_y - origin_y - bucket_height[0] -
							ss_this_e->mem_store_count/geometry->y_axis_scale,
							win_height_minus_offset_y);
				my_array[3].x =  offset_x + pixel_index;
				my_array[3].y =  MIN(win_height_minus_offset_y - origin_y - bucket_height[0] -
							ss_this_e->mem_store_count/geometry->y_axis_scale,
							win_height_minus_offset_y);
				if(display_modes[COLOR_SCHEME] == COLOR_BY_ACCESS_TYPE){
					color_index = MEM_WR;
				} else {
					color_index = j;
				}
				if((display_modes[SHOW_THREAD_NUMBER] == ALL_THREADS) ||
					(display_modes[SHOW_THREAD_NUMBER] == j)){
					Tk_Fill3DPolygon(tkwin,
						d,
						Border_array[color[color_index]],
						my_array,
						4,
						1,	
						TK_RELIEF_SOLID);
				}
				if((display_modes[SHOW_THREAD_NUMBER] == ALL_THREADS) ||
					(display_modes[HASH_MODE] == FLOAT )){
					bucket_height[0] += 
						(ss_this_e->mem_store_count / geometry->y_axis_scale);
				}
			}
			*/
		}
	}
}
