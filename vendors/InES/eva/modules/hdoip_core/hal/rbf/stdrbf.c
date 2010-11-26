#include "stdrbf.h"

/** Initialize t_rbf_dsc structure
 *
 * @param dsc pointer to the descriptor structure
 * @param start_addr start address of ringbuffer
 * @param size size in bytes of ringbuffer
 * @return error code (0=success)
 */
int rbf_init_dsc(t_rbf_dsc* dsc, uint32_t start_addr, size_t size) 
{
	start_addr = start_addr >> 2;
	size = (size >> 2) - 1;
    dsc->start = start_addr;
    dsc->stop = start_addr + size - 1;
    dsc->read = start_addr;
    dsc->write = start_addr;
	return 0;
}

/** print the descriptor values to the console
 *
 * @param p pointer to the acb register
 * @param o offset to the acb descriptor start register
 */ 
void rbf_report_dsc(void* p, uint32_t o)
{
    REPORT(INFO, "START : 0x%08x\n", RBF_GET_REG(p, o+RBF_OFF_START));
    REPORT(INFO, "STOP  : 0x%08x\n", RBF_GET_REG(p, o+RBF_OFF_STOP));
    REPORT(INFO, "WRITE : 0x%08x\n", RBF_GET_REG(p, o+RBF_OFF_WRITE));
    REPORT(INFO, "READ  : 0x%08x\n", RBF_GET_REG(p, o+RBF_OFF_READ));
}

/** Set start, stop, write and read register to the values in the descriptor structure
 *
 * @param p pointer to the acb registers
 * @param o offset to the descriptor start register
 * @param dsc pointer to the descriptor structur
 */
void rbf_set_dsc(void* p, uint32_t o, t_rbf_dsc* dsc) 
{
    RBF_SET_REG(p, o+RBF_OFF_START, dsc->start);
    RBF_SET_REG(p, o+RBF_OFF_STOP,  dsc->stop);
    RBF_SET_REG(p, o+RBF_OFF_WRITE, dsc->write);
    RBF_SET_REG(p, o+RBF_OFF_READ,  dsc->read);
}

/**
 * 
 * @param p pointer to the acb register
 * @param o offset to the descriptor start register
 * @param dsc pointer to the descriptor structure
 */
void rbf_get_dsc(void* p, uint32_t o, t_rbf_dsc* dsc) 
{
    dsc->start = RBF_GET_REG(p, o+RBF_OFF_START);
    dsc->stop = RBF_GET_REG(p, o+RBF_OFF_STOP);
    dsc->read = RBF_GET_REG(p, o+RBF_OFF_READ);
    dsc->write = RBF_GET_REG(p, o+RBF_OFF_WRITE);
}

/** Set write register to the value in the descriptor structure
 *
 * @param p pointer to the acb registers
 * @param o offset to the descriptor start register
 * @param dsc pointer to the descriptor structur
 */

void rbf_set_dsc_w(void* p, uint32_t o, t_rbf_dsc* dsc) 
{
    RBF_SET_REG(p, o+RBF_OFF_WRITE, dsc->write);
}

/** Set start, stop and write register to the values in the descriptor structure
 *
 * @param p pointer to the acb registers
 * @param o offset to the descriptor start register
 * @param dsc pointer to the descriptor structur
 */
void rbf_set_dsc_wc(void* p, uint32_t o, t_rbf_dsc* dsc) 
{
    RBF_SET_REG(p, o+RBF_OFF_START, dsc->start);
    RBF_SET_REG(p, o+RBF_OFF_STOP,  dsc->stop);
    RBF_SET_REG(p, o+RBF_OFF_WRITE, dsc->write);

}

/** Set read register to the value in the descriptor structure
 *
 * @param p pointer to the acb registers
 * @param o offset to the descriptor start register
 * @param dsc pointer to the descriptor structur
 */
void rbf_set_dsc_r(void* p, uint32_t o, t_rbf_dsc* dsc) 
{
    RBF_SET_REG(p, o+RBF_OFF_READ, dsc->read);
}

/** Set start, stopi and read register to the values in the descriptor structure
 *
 * @param p pointer to the acb registers
 * @param o offset to the descriptor start register
 * @param dsc pointer to the descriptor structur
 */

void rbf_set_dsc_rc(void* p, uint32_t o, t_rbf_dsc* dsc) 
{
    RBF_SET_REG(p, o+RBF_OFF_START, dsc->start);
    RBF_SET_REG(p, o+RBF_OFF_STOP,  dsc->stop);
    RBF_SET_REG(p, o+RBF_OFF_READ,  dsc->read);
}
