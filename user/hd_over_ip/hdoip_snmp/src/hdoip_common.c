#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>


#include "../../hdoip_daemon/hdoipd_msg.h"
#include "hdoip_common.h"



/* Get and set value generic function
 * Calls the various functions to get/set the desired values
 *
 * @param       Array to identify which function to call and if read-only or not
 * @param       Which snmp mode
 * @param       Parameters from net-snmp
 * @return      error message
 * */

#define CONFIG_FILE "/mnt/config/hdoipd.cfg"

#define DEFAULT_SYSDESCR "Rioxo device (Firmware Version 4.9): Rioxo can be configured as transmitter or receiver of HD video over an IP network."
#define DEFAULT_SYSOBJECTID "1.3.6.1.4.1.8072.3.2.10"
#define DEFAULT_SYSCONTACT "Rudolf Rohr rohr.rudolf@rioxo.ch"
#define DEFAULT_SYSNAME "No name is set to this Rioxo device. The name can be set with an SNMP set command."
#define DEFAULT_SYSLOCATION "No physical location is set to this Rioxo device. The location can be set with an SNMP set command."
#define DEFAULT_IFDESCR1 "eth0"
#define DEFAULT_IFDESCR2 "interface2"


int load_config_file( char*** loadedLines ){
	//returns the loaded lines of the config file
    int lines_allocated = 128;
    int max_line_len = 255;

    /* Allocate lines of text */
    char **lines = (char **)malloc(sizeof(char*)*lines_allocated);
    if (lines==NULL){
    	return 0;
    }

    FILE *fp = fopen(CONFIG_FILE, "r");
    if (fp == NULL){
    	return 0;
    }

    int i;
    for (i=0;1;i++){
        int j;

        /* Have we gone over our line allocation? */
        if (i >= lines_allocated){
            int new_size;
            /* Double our allocation and re-allocate */
            new_size = lines_allocated*2;
            lines = (char **)realloc(lines,sizeof(char*)*new_size);
            if (lines==NULL){
            	fclose(fp);
            	return 0;
            }
            lines_allocated = new_size;
        }

        /* Allocate space for the next line */
        lines[i] = malloc(max_line_len);
        if (lines[i]==NULL){
        	fclose(fp);
        	return 0;
        }
        if (fgets(lines[i],max_line_len-1,fp)==NULL)
            break;

        /* Get rid of CR or LF at end of line */
        for (j=strlen(lines[i])-1;j>=0 && (lines[i][j]=='\n' || lines[i][j]=='\r');j--);

        lines[i][j+1]='\0';
    }
    *loadedLines = lines;
	fclose(fp);
    return i; // i will be the number of lines read
}

void read_from_config_file(char* parameterName, char** returnValue){
	//find it and read it
	char*** linesPointer = (char***)malloc(sizeof(char**));
	int nol = load_config_file(linesPointer);
	char** lines = *linesPointer;
	int i;
	int parameterNameLen = strlen(parameterName);
	for (i = 0; i < nol; i ++){
		int lineLen = strlen(lines[i]);
		int j;
		if (parameterNameLen < lineLen){
			int found = 1;
			for (j =0; j < parameterNameLen; j ++){
				if (parameterName[j]!=lines[i][j])
					found = 0;
			}
			if (found){
				int k = 0;
				while ((k<lineLen)&&(lines[i][k]!='=')) k++;
				*returnValue = (char*)malloc(sizeof(char)*(lineLen+1-k));
				strcpy(*returnValue,&lines[i][k+1]);
				//clean
				int ci;
				for (ci = 0; ci < nol; ci++){
					free(lines[ci]);
				}
				free(lines);
				return;
			}
		}
	}
	int ci;
	for (ci = 0; ci < nol; ci++){
		free(lines[ci]);
	}
	free(lines);

	// if could not find it add it with default value and return the default value
    FILE *fp = fopen(CONFIG_FILE, "a");
    if (fp){
		if (strcmp(parameterName,"sysDescr")==0){
			fprintf(fp,"sysDescr=%s\n", DEFAULT_SYSDESCR);
			*returnValue = (char*)malloc(sizeof(char)*(strlen(DEFAULT_SYSDESCR)+1));
			strcpy(*returnValue,DEFAULT_SYSDESCR);
		}else if (strcmp(parameterName,"sysObjectId")==0){
			fprintf(fp,"sysObjectId=%s\n",DEFAULT_SYSOBJECTID);
			*returnValue = (char*)malloc(sizeof(char)*(strlen(DEFAULT_SYSOBJECTID)+1));
			strcpy(*returnValue,DEFAULT_SYSOBJECTID);
		}else if (strcmp(parameterName,"sysContact")==0){
			fprintf(fp,"sysContact=%s\n",DEFAULT_SYSCONTACT);
			*returnValue = (char*)malloc(sizeof(char)*(strlen(DEFAULT_SYSCONTACT)+1));
			strcpy(*returnValue,DEFAULT_SYSCONTACT);
		}else if (strcmp(parameterName,"sysName")==0){
			fprintf(fp,"sysName=%s\n",DEFAULT_SYSNAME);
			*returnValue = (char*)malloc(sizeof(char)*(strlen(DEFAULT_SYSNAME)+1));
			strcpy(*returnValue,DEFAULT_SYSNAME);
		}else if (strcmp(parameterName,"sysLocation")==0){
			fprintf(fp,"sysLocation=%s\n",DEFAULT_SYSLOCATION);
			*returnValue = (char*)malloc(sizeof(char)*(strlen(DEFAULT_SYSLOCATION)+1));
			strcpy(*returnValue,DEFAULT_SYSLOCATION);
		}else if (strcmp(parameterName,"ifDescr1")==0){
			fprintf(fp,"ifDescr1=%s\n",DEFAULT_IFDESCR1);
			*returnValue = (char*)malloc(sizeof(char)*(strlen(DEFAULT_IFDESCR1)+1));
			strcpy(*returnValue,DEFAULT_IFDESCR1);
		}else if (strcmp(parameterName,"ifDescr2")==0){
			fprintf(fp,"ifDescr2=%s\n",DEFAULT_IFDESCR2);
			*returnValue = (char*)malloc(sizeof(char)*(strlen(DEFAULT_IFDESCR2)+1));
			strcpy(*returnValue,DEFAULT_IFDESCR2);
		}

		fclose(fp);
    }
}

//return 1 if parameter exists in the config file otherwise 0
int check_param_config_file(char* parameterName){
	//find it and read it
	char*** linesPointer = (char***)malloc(sizeof(char**));
	int nol = load_config_file(linesPointer);
	char** lines = *linesPointer;
	int i;
	int parameterNameLen = strlen(parameterName);
	for (i = 0; i < nol; i ++){
		int lineLen = strlen(lines[i]);
		int j;
		if (parameterNameLen < lineLen){
			int found = 1;
			for (j =0; j < parameterNameLen; j ++){
				if (parameterName[j]!=lines[i][j])
					found = 0;
			}
			if (found){
				//clean
				int ci;
				for (ci = 0; ci < nol; ci++){
					free(lines[ci]);
				}
				free(lines);
				return 1;
			}
		}
	}
	//clean
	int ci;
	for (ci = 0; ci < nol; ci++){
		free(lines[ci]);
	}
	free(lines);
	return 0;
}

void rewrite_config_file(char* parameterName, char* newValue){
	//find it and rewrite it if it exists
	char*** linesPointer = (char***)malloc(sizeof(char**));
	int nol = load_config_file(linesPointer);
	char** lines = *linesPointer;
	int i;
	int parameterNameLen = strlen(parameterName);
	int exist = 0;
	for (i = 0; i < nol; i ++){
		int lineLen = strlen(lines[i]);
		int j;
		if (parameterNameLen < lineLen){
			int found = 1;
			for (j =0; j < parameterNameLen; j ++){
				if (parameterName[j]!=lines[i][j])
					found = 0;
			}
			if (found){
				free(lines[i]);
				lines[i]=(char*)malloc(sizeof(char)*(parameterNameLen+strlen(newValue)+5));
				sprintf(lines[i],"%s=%s",parameterName,newValue);
				exist = 1;
				break;
			}
		}
	}
	//add if it does not exist
	if (!exist){
		sprintf(lines[nol],"%s=%s",parameterName,newValue);
		nol ++;
	}
	//rewrite the config file
    FILE *fp = fopen(CONFIG_FILE, "w+");
	if (fp){
		int lineNumber=0;
		for (lineNumber = 0; lineNumber < nol; lineNumber++){
			fprintf(fp,"%s\n",lines[lineNumber]);
		}
		fclose(fp);
	}
	//free memory
	int ci;
	for (ci = 0; ci < nol; ci++){
		free(lines[ci]);
	}
	free(lines);
}

int getset_value_generic(t_snmp_array* arr, int mode, netsnmp_request_info *requests){

    int fd = -1, fdr = -1, ret;
    char* s;

    // open pipe to hdoipd and check if it was successful
    fd = open(FIFO_NODEC, O_RDWR, 0600);
    if(fd == -1) {
        snmp_log(LOG_ERR, "Failed to open (fd) %s: %s \n", FIFO_NODEC, strerror(errno));
        return -1;
    }
    
    fdr = open(FIFO_NODER, O_RDWR, 0600);
    if(fdr == -1) {
        snmp_log(LOG_ERR, "Failed to open (fdr) %s: %s \n", FIFO_NODEC, strerror(errno));
        close(fd); //close the previous pipe
        return -1;
    }
    

    // check if read only parameter
    if (arr->arg_cnt == 0){
        switch(mode) {
            case MODE_GET:     
            	if (strcmp(arr->p_name,"sysDescr")==0){
            		char** value = (char**)malloc(sizeof(char*));
            		read_from_config_file("sysDescr",value);
            		s = (char*)malloc(sizeof(char)*(strlen(*value)+1));
            		strcpy(s,*value);
            		free(*value);
            		free(value);
            	}else if (strcmp(arr->p_name,"sysObjectId")==0){
            		char** value = (char**)malloc(sizeof(char*));
            		read_from_config_file("sysObjectId",value);
            		s = (char*)malloc(sizeof(char)*(strlen(*value)+1));
            		strcpy(s,*value);
            		free(*value);
            		free(value);
            	}else if (strcmp(arr->p_name,"ifNumber")==0){
            		if (check_param_config_file("system-mac2")==1){
            			s = (char*)malloc(2*sizeof(char));
            			sprintf(s,"2");
            		}else{
            			s = (char*)malloc(2*sizeof(char));
            			sprintf(s,"1");
            		}
            	}else if (strcmp(arr->p_name,"ifIndex1")==0){
        			s = (char*)malloc(2*sizeof(char));
        			sprintf(s,"1");
            	}else if (strcmp(arr->p_name,"ifIndex2")==0){
            		s = (char*)malloc(2*sizeof(char));
    				sprintf(s,"2");
            	}else if (strcmp(arr->p_name,"ifPhysAddress1")==0){
            		char** value = (char**)malloc(sizeof(char*));
            		read_from_config_file("system-mac",value);
            		s = (char*)malloc(sizeof(char)*(strlen(*value)+1));
            		strcpy(s,*value);
            		free(*value);
            		free(value);
            	}else if (strcmp(arr->p_name,"ifPhysAddress2")==0){
            		char** value = (char**)malloc(sizeof(char*));
            		read_from_config_file("system-mac2",value);
            		s = (char*)malloc(sizeof(char)*(strlen(*value)+1));
            		strcpy(s,*value);
            		free(*value);
            		free(value);
            	}else if (strcmp(arr->p_name,"ipAdEntAddr")==0){
            		char** value = (char**)malloc(sizeof(char*));
            		read_from_config_file("system-ip",value);
            		s = (char*)malloc(sizeof(char)*(strlen(*value)+1));
            		strcpy(s,*value);
            		free(*value);
            		free(value);
            	}else if (strcmp(arr->p_name,"ipAdEntNetMask")==0){
            		char** value = (char**)malloc(sizeof(char*));
            		read_from_config_file("system-subnet",value);
            		s = (char*)malloc(sizeof(char)*(strlen(*value)+1));
            		strcpy(s,*value);
            		free(*value);
            		free(value);
            	}else if (strcmp(arr->p_name,"ipAdEntBcastAddr")==0){
            		s = (char*)malloc(sizeof(char)*2);
            		strcpy(s,"1");
            	}else if (strcmp(arr->p_name,"ifDescr1")==0){
            		char** value = (char**)malloc(sizeof(char*));
            		read_from_config_file("ifDescr1",value);
            		s = (char*)malloc(sizeof(char)*(strlen(*value)+1));
            		strcpy(s,*value);
            		free(*value);
            		free(value);
            	}else if (strcmp(arr->p_name,"ifDescr2")==0){
            		char** value = (char**)malloc(sizeof(char*));
            		read_from_config_file("ifDescr2",value);
            		s = (char*)malloc(sizeof(char)*(strlen(*value)+1));
            		strcpy(s,*value);
            		free(*value);
            		free(value);
            	}else if (strcmp(arr->p_name,"sysUpTime")==0){
            		s = (char*)malloc(60);

            		strcpy(s,"unknown");

            		FILE* fddd;
            	    fddd = fopen("/tmp/sysuptime.txt", "r");
            	    if(fddd) {
            	    	long long sysctime;
            	    	fscanf(fddd, "%lld",&sysctime);

            	    	struct timeval tv;
            	    	gettimeofday(&tv, 0);
            	    	long long time_mil = (tv.tv_sec)*1000LL + (tv.tv_usec)/1000;
            	    	long long now_in_hundredth = time_mil / 10;

            	    	long long delta = (now_in_hundredth-sysctime);

            	    	long long hours = delta / 360000;
            	    	long long minutes = (delta - hours*360000)/6000;
            	    	long long seconds = (delta - hours*360000 - minutes * 6000)/100;

            	    	sprintf(s,"%lld (%lld h. %lld min. %lld sec.)",(now_in_hundredth-sysctime),hours,minutes,seconds);

            			fclose(fddd);
            	    }
            	}else{
            		s = hoic_get_param(fd, fdr, arr->p_name);
            	}
            	snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, (u_char *)s, strlen((char *)s));
                //free(s);
                break;
            default:
                // we should never get here, so this is a really bad error
                snmp_log(LOG_ERR, "unknown mode (%d) in handle_systemState\n", mode );
                close(fd);
                close(fdr);
                return -1;
        }
    }else{
    	int isString = 1;

    // parameter is read and writable
        switch(mode){
            // if a value is read, only MODE_GET is executed
            case MODE_GET:     
            	if (strcmp(arr->p_name,"sysContact")==0){
            		char** value = (char**)malloc(sizeof(char*));
            		read_from_config_file("sysContact",value);
            		s = (char*)malloc(strlen(*value)+1);
            		strcpy(s,*value);
					hoic_set_param(fd, arr->p_name, *value);
            		free(*value);
            		free(value);
			//isString=0;
            	}else if (strcmp(arr->p_name,"sysName")==0){
            		char** value = (char**)malloc(sizeof(char*));
            		read_from_config_file("sysName",value);
            		s = (char*)malloc(strlen(*value)+1);
					hoic_set_param(fd, arr->p_name, *value);
            		strcpy(s,*value);
            		free(*value);
            		free(value);

			//isString=0;

            	}else if (strcmp(arr->p_name,"sysLocation")==0){
            		char** value = (char**)malloc(sizeof(char*));
            		read_from_config_file("sysLocation",value);
            		s = (char*)malloc(strlen(*value)+1);
            		strcpy(s,*value);
					hoic_set_param(fd, arr->p_name, *value);
            		free(*value);
            		free(value);
			//isString=0;
            	}if (strcmp(arr->p_name,"ipDefaultTTL")==0){
			
            		char** value = (char**)malloc(sizeof(char*));
            		read_from_config_file("eth-ttl",value);

            		s = (char*)malloc(strlen(*value)+1);
            		strcpy(s,*value);
            		free(*value);
            		free(value);
            		isString = 1;
            	}else{
            		s = hoic_get_param(fd, fdr, arr->p_name);
            		//free(s);
            	}
            	if (isString){
            		if (strlen(s)<1){
            			requests->requestvb->val.string[0]=0;
            		}
            		snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, (u_char *)s, strlen((char *)s));
            	}
            	break;
            // if a value is set, the following modes are executed
            case MODE_SET_RESERVE1:
                //ret = netsnmp_check_vb_type(requests->requestvb, ASN_OCTET_STR);
                //if ( ret != SNMP_ERR_NOERROR ) {
                   // netsnmp_set_request_error(reqinfo, requests, ret );
                //}
                break;
            case MODE_SET_RESERVE2:
                break;
            case MODE_SET_FREE:
                break;
            case MODE_SET_ACTION:

            	if ((strcmp(arr->p_name,"sysContact")==0)||(strcmp(arr->p_name,"sysName")==0)||(strcmp(arr->p_name,"sysLocation")==0)){
					hoic_set_param(fd, arr->p_name, requests->requestvb->val.string);
            		rewrite_config_file(arr->p_name,requests->requestvb->val.string);
            	}else if (strcmp(arr->p_name,"ipDefaultTTL")==0){
            		char* value = (char*)malloc(40*sizeof(char));
            		if (requests && requests->requestvb && requests->requestvb->val.integer){
            			sprintf(value,"%d", *(requests->requestvb->val.integer));
            			
            			rewrite_config_file("eth-ttl",value);
            		}
					hoic_set_param(fd, "eth-ttl", value);

					free(value);
            		isString = 0;
            	}else if ((strcmp(arr->p_name,"system-ip")==0)||(strcmp(arr->p_name,"system-subnet")==0)||(strcmp(arr->p_name,"system-gateway")==0)){
            		//change hex ip to human readable format
            		char* value = (char*)malloc(15*sizeof(char));
            		char* temp =  requests->requestvb->val.string;

            			sprintf(value,"%d.%d.%d.%d", (unsigned char)temp[0],(unsigned char)temp[1],(unsigned char)temp[2],(unsigned char)temp[3]);


					hoic_set_param(fd, arr->p_name, value);

					isString = 0;
            	}

            	// Select the right function
				switch(arr->arg_cnt){
					case 1:
						//Call function 1: hoic_set_param
						if (isString){
							//if (strlen(requests->requestvb->val.string)>1)
		            		if (strlen(requests->requestvb->val.string)<1){
		            			requests->requestvb->val.string[0]=0;
		            		}
							hoic_set_param(fd, arr->p_name, requests->requestvb->val.string);

						}else{

						}
					break;
					case 2:
						//Call function 2: hoic_sw (used for commands)
						if (isString)
							hoic_sw(fd, arr->id);
						break;
					case 3:
						//Call function 3: hoic_remote_update (used for remote update command only)
					   //(arr->fnc_ptr)(fd, requests->requestvb->val.string);
						if (isString)
							hoic_remote_update(fd, requests->requestvb->val.string);
						break;
					default:
						break;
				}
                break;
            case MODE_SET_COMMIT:
                break;
            case MODE_SET_UNDO:
                break;
            default:
                // we should never get here, so this is a really bad error
                snmp_log(LOG_ERR, "unknown mode (%d) in handle_systemState\n", mode );
                close(fd);
                close(fdr);
                return -1;
        }
    }
    close(fd);
    close(fdr);

    return 0;
}
