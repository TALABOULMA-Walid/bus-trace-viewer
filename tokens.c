/* $Id: token.c,v 1.103 1999/12/31 20:39:47 davewang Exp $ */

#include "btv.h"

int file_io_token(char *input){
	size_t length;
	length = strlen(input);
	if(strncmp(input, "FETCH",length) == 0) {
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

