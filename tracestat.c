#include "btv.h"

FILE *trace_file_ptr;
FILE *spd_file_ptr;

main(int argc, char *argv[]){

	FILE		*fout;
	STATS		stats;
	TRACE		trace;
	dram_system_t	dram_system;
	int		length,i,j;
	char 		c;
	int		analysisflag;
	char		filename[256];
	char		outfile[256];
	int		argc_index;
	int		bank_count;
	int		min_bank_count;
	int		max_bank_count;
	int		bank_count_interval;
	int 		ratio;
	int		min_ratio;
	int		max_ratio;
	int		max_open;
	int		min_max_open;
	int		max_max_open;
	int		max_open_interval;
	int		queue_depth;
	int		min_queue_depth;
	int		max_queue_depth;
	int		queue_depth_interval;

	/* set defaults */
	
	argc_index 	= 1;
	analysisflag 	= PRINT_SUMMARY;
	trace.trace_type = UNKNOWN;
	trace_file_ptr	= NULL;
	spd_file_ptr 	= NULL;
	fout		= stdout;
	filename[0]	= '\0';
	min_bank_count	= 16;
	max_bank_count	= 16;
	bank_count_interval = 8;
	min_ratio	= 8;
	max_ratio	= 20;
	min_max_open	= 8;
	max_max_open	= 16;
	max_open_interval= 4;
	min_queue_depth	= 4;
	max_queue_depth = 4;
	queue_depth_interval= 2;
	
	
	if((argc < 2) || (strncmp(argv[1], "-help",5) == 0)){
		fprintf(stdout,"Usage: %s -options optionswitch\n",argv[0]);
		fprintf(stdout,"-trace_type [k6|mase|tm]\n");
		fprintf(stdout,"-trace_file TRACE_FILENAME\n");
		fprintf(stdout,"-dram:spd_input SPD_FILENAME\n");
		fprintf(stdout,"-output_file OUTPUT_FILENAME\n");
		fprintf(stdout,"-analysis_type [PRINT_ALL|PRINT_SUMMARY|PRINT_DETAILS|DRAM_ANALYSIS|OPEN_PAGE_ANALYSIS]\n");
		fprintf(stdout,"-debug \n");
		_exit(0);
	}

	while(argc_index < argc){
		length = strlen(argv[argc_index]);
		if(strncmp(argv[argc_index], "-trace_type",length) == 0) {
			length = strlen(argv[argc_index+1]);
			if(strncmp(argv[argc_index+1], "mase",length) == 0) {
				trace.trace_type = MASE_TRACE;
			} else if(strncmp(argv[argc_index+1], "k6",length) == 0) {
				trace.trace_type = K6_TRACE;
			} else {
				fprintf(stdout,"Unknown trace type [%s]\n",argv[argc_index+1]);
			}
			argc_index += 2;
		} else if(strncmp(argv[argc_index], "-trace_file",length) == 0) {
			length = strlen(argv[argc_index+1]);
			if((trace_file_ptr=fopen(argv[argc_index+1],"r"))==NULL){
				printf("Error in opening tracefile %s\n",argv[argc_index+1]);
				_exit(3);
			}
			length = strlen(argv[argc_index+1]);
			for(i=0;i<length-4;i++){
				filename[i] = argv[argc_index+1][i];
			}
			filename[length-4] = '\0';
			argc_index += 2;
		} else if(strncmp(argv[argc_index], "-dram:spd_input",length) == 0) {
			length = strlen(argv[argc_index+1]);
			if((spd_file_ptr=fopen(argv[argc_index+1],"r"))==NULL){
				printf("Error in opening spd file %s\n",argv[argc_index+1]);
				_exit(3);
			}
			fprintf(stderr,"Not currently enabled.  Talk to me. David.\n");
			_exit(0);
			argc_index += 2;
		} else if(strncmp(argv[argc_index], "-output_file",length) == 0) {
			sprintf(filename,"%s",argv[argc_index+1]);
			argc_index += 2;
		} else if(strncmp(argv[argc_index], "-analysis_type",length) == 0) {
			if((strncmp(argv[argc_index+1], "printall", 8)== 0)) {
				analysisflag = PRINT_ALL;
			} else if((strncmp(argv[argc_index+1], "printsummary", 10)== 0)) {
				analysisflag = PRINT_SUMMARY;
			} else if((strncmp(argv[argc_index+1], "printdetails", 10)== 0)) {
				analysisflag = PRINT_DETAILS;
			} else if((strncmp(argv[argc_index+1], "dram", 4)== 0)) {
				analysisflag = DRAM_ANALYSIS;
			} else if((strncmp(argv[argc_index+1], "open_page", 9)== 0)) {	/* open page analysis */
				analysisflag = OPEN_PAGE_ANALYSIS;
			}
			argc_index += 2;
		}
	}

	if((trace.trace_type == K6_TRACE) && (read_k6_trace(trace_file_ptr,&(trace)) != BTV_OK)){
		printf("K6 Trace file open failed\n");
		_exit(0);
 	} else if((trace.trace_type == MASE_TRACE) && (read_mase_trace(trace_file_ptr,&(trace)) != BTV_OK)){
		printf("Mase Trace file open failed\n");
		_exit(0);
	}
	printf("size[%d] starttime[%lf] endtime[%lf]\n",
		trace.event_count,
		trace.start_time,
		trace.end_time);

	init_btv_stats(&(stats));
	stats.end_no = trace.event_count - 1;

	init_dram_system(&(dram_system));

	if(spd_file_ptr != NULL){
		read_dram_config_from_file(spd_file_ptr, &(dram_system.config));
	}

	if(analysisflag == PRINT_ALL){
		btv_gather_advanced_stats(&(stats), &(trace));
		sprintf(outfile,"%s.detail_stat",filename);
		if((fout=fopen(outfile,"w+"))==NULL){
			printf("Error in opening %s for output\n",filename);
			_exit(3);
		}
		btv_print_advanced_stats(&(stats), PRINT_DETAILS, fout);
		fclose(fout);
		sprintf(outfile,"%s.summary_stat",filename);
		if((fout=fopen(outfile,"w+"))==NULL){
			printf("Error in opening %s for output\n",filename);
			_exit(3);
		}
		btv_print_advanced_stats(&(stats), PRINT_SUMMARY, fout);
		fclose(fout);
	} else if(analysisflag == PRINT_DETAILS){
		btv_gather_advanced_stats(&(stats), &(trace));
		btv_print_advanced_stats(&(stats), analysisflag, fout);
		fclose(fout);
	} else if(analysisflag == PRINT_SUMMARY){
		btv_gather_advanced_stats(&(stats), &(trace));
		btv_print_advanced_stats(&(stats), analysisflag, fout);
		fclose(fout);
	} else if(analysisflag == DRAM_ANALYSIS){
		sprintf(outfile,"%s.txt",filename);
		if((fout=fopen(outfile,"w+"))==NULL){
			printf("Error in opening %s for output\n",outfile);
			_exit(3);
		}
		init_dram_system(&(dram_system));
		dram_system.config.row_buffer_management_policy	= CLOSE_PAGE;
		dram_system.config.pa_mapping_policy  		= CLOSE_PAGE_BASELINE;
		for(bank_count = min_bank_count; bank_count <= max_bank_count;bank_count+=bank_count_interval){
			dram_system.config.bank_count			= bank_count;
			for(queue_depth = min_queue_depth; queue_depth <= max_queue_depth;queue_depth+=queue_depth_interval){
				dram_system.config.transaction_queue_depth	= queue_depth;
				for(max_open = min_max_open; max_open <= max_max_open;max_open+=max_open_interval){
					dram_system.config.max_open_bank_count		= max_open;
					for(ratio = min_ratio; ratio <= max_ratio; ratio+=2){
						init_dram_stats(&(stats));
						dram_system.config.d_m 			= ratio - 1;
						btv_gather_dram_stats(&(trace),&(dram_system), &(stats));
						fprintf(fout,"%d ", dram_system.config.d_m);
						btv_print_dram_stats(&(stats), PRINT_BANDWIDTH, fout);
						fprintf(fout,"%2d %d %d\n", 
								dram_system.config.bank_count,
								dram_system.config.max_open_bank_count,
								dram_system.config.transaction_queue_depth);
					
					}
				}
			}
		}
		fclose(fout);
	} else if(analysisflag == OPEN_PAGE_ANALYSIS){
		sprintf(outfile,"%s.txt",filename);
		if((fout=fopen(outfile,"w+"))==NULL){
			printf("Error in opening %s for output\n",outfile);
			_exit(3);
		}
		init_dram_system(&(dram_system));
		dram_system.config.row_buffer_management_policy	= OPEN_PAGE;
		dram_system.config.pa_mapping_policy  		= OPEN_PAGE_BASELINE;
		for(bank_count = min_bank_count; bank_count <= max_bank_count;bank_count+=bank_count_interval){
			dram_system.config.bank_count			= bank_count;
			for(queue_depth = min_queue_depth; queue_depth <= max_queue_depth;queue_depth+=queue_depth_interval){
				dram_system.config.transaction_queue_depth	= queue_depth;
				for(max_open = min_max_open; max_open <= max_max_open;max_open+=max_open_interval){
					dram_system.config.max_open_bank_count		= max_open;
					for(ratio = min_ratio; ratio <= max_ratio; ratio+=2){
						init_dram_stats(&(stats));
						dram_system.config.d_m 			= ratio - 1;
						btv_gather_dram_stats(&(trace),&(dram_system), &(stats));
						fprintf(fout,"%d ", dram_system.config.d_m);
						btv_print_dram_stats(&(stats), PRINT_BANDWIDTH, fout);
						fprintf(fout,"%2d %d %d\n", 
								dram_system.config.bank_count,
								dram_system.config.max_open_bank_count,
								dram_system.config.transaction_queue_depth);
					
					}
				}
			}
		}
		fclose(fout);
	} 
}

