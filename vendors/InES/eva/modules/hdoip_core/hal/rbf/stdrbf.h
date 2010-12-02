#ifndef STDRBF_H_
#define STDRBF_H_

#include "std.h"

#define RBF_OFF_START       (0x0)
#define RBF_OFF_STOP        (0x4)
#define RBF_OFF_WRITE       (0x8)
#define RBF_OFF_READ        (0xC)

typedef struct {
    uint32_t    start;
    uint32_t    stop;
    uint32_t    write;
    uint32_t    read;
} t_rbf_dsc;

#define RBF_GET_REG(p, o)		(HOI_RD32((p), (o)) << 2)
#define RBF_SET_REG(p, o, v)	(HOI_WR32((p), (o), ((v) >> 2)))

static inline void rbf_dsc(t_rbf_dsc* dsc, void* b, size_t s)
{
    dsc->start = b;
    dsc->stop = OFFSET(b, s - 4);
    dsc->write = b;
    dsc->read = b;
}

int rbf_init_dsc(t_rbf_dsc* dsc, uint32_t start_addr, size_t size);
void rbf_report_dsc(void* p, uint32_t o);
void rbf_set_dsc(void* p, uint32_t o, t_rbf_dsc* dsc);
void rbf_get_dsc(void* p, uint32_t o, t_rbf_dsc* dsc);
void rbf_set_dsc_w(void* p, uint32_t o, t_rbf_dsc* dsc);
void rbf_set_dsc_wc(void* p, uint32_t o, t_rbf_dsc* dsc);
void rbf_set_dsc_r(void* p, uint32_t o, t_rbf_dsc* dsc);
void rbf_set_dsc_rc(void* p, uint32_t o, t_rbf_dsc* dsc);

#endif /*STDRBF_H_*/
