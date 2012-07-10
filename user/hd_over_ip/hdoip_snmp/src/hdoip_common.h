#ifndef HDOIP_COMMON_H
#define HDOIP_COMMON_H

#define FIFO_NODEC       "/tmp/hdoipd0.cmd"
#define FIFO_NODER       "/tmp/hdoipd0.rsp"

// The function to get/set parameters
#define HOIC_GET_R_ONLY  0
#define HOIC_GET_SET     1
#define HOIC_SW          2
#define HOIC_REMOTE_UP   3

/* Command definitions */
typedef struct {
    int         arg_cnt;  // choose the function
    uint32_t    id;       // Id: only used for the different commands
    char*       p_name;   // The name of the parameter to get/set (not used in commands)
} t_snmp_array;

int getset_value_generic(t_snmp_array* arr, int mode, netsnmp_request_info *requests);

#endif /* HDOIP_COMMON_H */
