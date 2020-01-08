#include "btv.h"

FILE *fin;

main(int argc, char *argv[]){

	FILE		*fout;
	STATS		stats;
	int		length,i,j;
	char 		c;
	int		printflag;
	char 		filename[100];

	c = argv[1][0];
	if((argc < 2) || (fin = fopen(argv[1],"r")) == NULL){
		printf("File open failed\n");
		_exit(0);
	}

	btv_merge_stat(fin,&(stats));

	sprintf(filename,"%s_merged_detail_stat.txt",argv[1]);
	fout = fopen(filename,"w+");
	btv_print_advanced_stats(&(stats), PRINT_DETAILS, fout);
	fclose(fout);
	sprintf(filename,"%s_merged_summary_stat.txt",argv[1]);
	fout = fopen(filename,"w+");
	btv_print_advanced_stats(&(stats), PRINT_SUMMARY, fout);
	fclose(fout);
}
