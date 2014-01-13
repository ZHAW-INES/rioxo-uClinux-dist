#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

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
                s = hoic_get_param(fd, fdr, arr->p_name);
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

    // parameter is read and writable
        switch(mode){
            // if a value is read, only MODE_GET is executed
            case MODE_GET:     
                s = hoic_get_param(fd, fdr, arr->p_name);
                snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, (u_char *)s, strlen((char *)s));
                //free(s);
                break;
            // if a value is set, the following modes are executed
            case MODE_SET_RESERVE1:
                ret = netsnmp_check_vb_type(requests->requestvb, ASN_OCTET_STR);
                if ( ret != SNMP_ERR_NOERROR ) {
                   // netsnmp_set_request_error(reqinfo, requests, ret );
                }
                break;
            case MODE_SET_RESERVE2:
                break;
            case MODE_SET_FREE:
                break;
            case MODE_SET_ACTION:
            // Select the right function
            switch(arr->arg_cnt){
                case 1:
                    snmp_log(LOG_ERR, "set (%s)\n", requests->requestvb->val.string );
                	//Call function 1: hoic_set_param
                    hoic_set_param(fd, arr->p_name, requests->requestvb->val.string);
                break;
                case 2:
                    //Call function 2: hoic_sw (used for commands)
                	//snmp_log(LOG_ERR, "here (%s) \n", requests->requestvb->val.string );
                    hoic_sw(fd, arr->id);
                    break;
                case 3:
                    //Call function 3: hoic_remote_update (used for remote update command only)
                   //(arr->fnc_ptr)(fd, requests->requestvb->val.string);
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
