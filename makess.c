#include "makess.h"

/* makess.c  Make spreadsheet from mase stat file output */

main(int argc, char *argv[]){
	FILE 	*fin;
	FILE 	*fout;
	int 	argc_index;
	int	file_index;
	STATS 	*stat_array[MAX_INPUT_FILE_COUNT];

	file_index = 0;
	mode = NORMAL_MODE;

	if(argc < 2) {
                fprintf(stdout,"Usage: %s <input_file_0> <input_file_1> . . . <input_file_n> -o <output_file>\n",argv[0]);
	} 
	argc_index = 1;
	fin = NULL;
	while(argc_index < argc){
		if(strncmp(argv[argc_index], "-o",2) == 0){
			if((fout = fopen(argv[argc_index+1], "w+")) != NULL){
				print_stats(&(stat_array[0]),fout,file_index);
			} else {
				print_stats(&(stat_array[0]),(FILE *)stderr,file_index);
			}
			argc_index += 2;
		} else if (strncmp(argv[argc_index], "-dram",5) == 0){
			mode = DRAM_MODE;
			argc_index++;
		} else if((fin = fopen(argv[argc_index], "r")) != NULL){
			read_in_stat(&(stat_array[0]),fin,file_index,argv[argc_index]);
			argc_index++;
			file_index++;
		}
	}
}

void read_in_stat(STATS *array[] ,FILE *fin, int file_index, char *filename){
	char temp_buffer[100];
	int index,count;
	int length;

	array[file_index] = (STATS *)calloc(1,sizeof(STATS));
	length = strlen(filename);
	array[file_index]->header = (char *)calloc(length+1,sizeof(char));
	strcpy(array[file_index]->header,filename);
	array[file_index]->total_count = 0.0;
	array[file_index]->count_times_latency = 0.0;
	if(mode == NORMAL_MODE){
		while(fscanf(fin,"%s %d %d",temp_buffer, &index, &count) != EOF){
			if(index >= MAX_STAT_DEPTH){
				array[file_index]->dangling_stat += count;
			} else if (index < 0){
				fprintf(stderr,"WTH? What's this? %s %d %d\n",temp_buffer,index,count);
			} else {
				array[file_index]->delta_stat[index] = count;	
			}
			array[file_index]->total_count += (double) count;
			array[file_index]->count_times_latency += (double) (count * index);
		}
		if(array[file_index]->total_count != 0.0){
			array[file_index]->average_latency = array[file_index]->count_times_latency / array[file_index]->total_count;
		} else {
			array[file_index]->average_latency = 0.0;
		}
	} else if (mode == DRAM_MODE){
		while(fscanf(fin,"%d %d", &index, &count) != EOF){
			if(index >= MAX_STAT_DEPTH){
				array[file_index]->dangling_stat += count;
			} else if (index < 0){
				fprintf(stderr,"WTH? What's this? %d %d\n",index,count);
			} else {
				array[file_index]->delta_stat[index] = count;	
			}
			array[file_index]->total_count += (double) count;
			array[file_index]->count_times_latency += (double) (count * index);
		}
		if(array[file_index]->total_count != 0.0){
			array[file_index]->average_latency = array[file_index]->count_times_latency / array[file_index]->total_count;
		} else {
			array[file_index]->average_latency = 0.0;
		}
	}
	fclose(fin);
}

void print_stats(STATS *array[], FILE *fout, int file_index){
	int i,j;
	int print_flag;

	fprintf(fout,"Latency ");
	for(j=0;j<file_index;j++){
		fprintf(fout,"%s ",array[j]->header);
	}
	fprintf(fout,"\n");
	for(i = 0;i< MAX_STAT_DEPTH;i++){
		print_flag = FALSE;
		for(j=0;j<file_index;j++){
			if(array[j]->delta_stat[i] > 0){
				print_flag = TRUE;
			}
		}
		if(print_flag == TRUE){
			fprintf(fout,"%8d ",i);
			for(j=0;j<file_index;j++){
				fprintf(fout,"%8d ",array[j]->delta_stat[i]);
			}
			fprintf(fout,"\n");
		}
	}
	print_flag = FALSE;
	for(j=0;j<file_index;j++){
		if(array[j]->dangling_stat > 0){
			print_flag = TRUE;
		}
	}
	if(print_flag == TRUE){
		fprintf(fout,">=%8d ",MAX_STAT_DEPTH);
		for(j=0;j<file_index;j++){
			fprintf(fout,"%8d ",array[j]->dangling_stat);
		}
		fprintf(fout,"\n");
	}
	fprintf(fout,"\n         ");
	for(j=0;j<file_index;j++){
		fprintf(fout,"%8.2lf ",array[j]->average_latency);
	}
	fprintf(fout,"\n");
	fclose(fout);
}
