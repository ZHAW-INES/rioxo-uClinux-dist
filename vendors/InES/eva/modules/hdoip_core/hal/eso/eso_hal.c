
#include "eso_hal.h"


/** Initialize ethernet stream out (CFG- and DMA-Register)
 * 
 * @param p hardware pointer
 */
bool eso_init(void* p) 
{  
    // DMA CFG register 
    eso_set_burst_sz(p, ESO_BURST_SIZE); 
    eso_set_fifo_th(p, ESO_FIFO_TRESHOLD);
        
    // Set run bit and statistic reset (priorization disabled)
    HOI_WR32(p, ESO_OFF_CFG, ESO_CFG_RUN | ESO_CFG_STAT_RESET);
    
    return true;
}

/** Initialize needed ringbuffer pointer
 * 
 * @param p             hardware pointer
 * @param cpu_buffer    start address of cpu ringbuffer
 * @param cpu_size      cpu ringbuffer size in bytes
 * @param vid_buffer    start address of video ringbuffer
 * @param vid_size      video ringbuffer size in bytes
 * @param aud_buffer    start address of audio ringbuffer
 * @param aud_size      audio ringbuffer size in bytes
 * @return success (true)
 */
bool eso_init_rbf(void* p, void* cpu_buffer, size_t cpu_size,
                           void* vid_buffer, size_t vid_size,
                           void* aud_buffer, size_t aud_size)
{
    if((cpu_size > ESO_MTU) && (vid_size > ESO_MTU) && (aud_size > ESO_MTU)) {
        HOI_WR32(p, ESO_OFF_CPU_START,  cpu_buffer);
        HOI_WR32(p, ESO_OFF_CPU_STOP,   cpu_buffer+cpu_size-ESO_MTU);
        HOI_WR32(p, ESO_OFF_CPU_READ,   cpu_buffer);
        
        HOI_WR32(p, ESO_OFF_VID_START,  vid_buffer);
        HOI_WR32(p, ESO_OFF_VID_STOP,   vid_buffer+vid_size-ESO_MTU);
        HOI_WR32(p, ESO_OFF_VID_READ,   vid_buffer);
        
        HOI_WR32(p, ESO_OFF_AUD_START,  aud_buffer);
        HOI_WR32(p, ESO_OFF_AUD_STOP,   aud_buffer+aud_size-ESO_MTU);
        HOI_WR32(p, ESO_OFF_AUD_READ,   aud_buffer);
        return true;
    } else {
        return false;
    }
}


/** plots (printf) all pointer of the ringbuffer (audio, video, cpu)
 * 
 * @param p hardware pointer
 */
void eso_report(void* p) 
{
    printf(" ethernet stream out (eso) report:\n");
    printf(" CPU ringbuffer: \n");
    hoi_report_rbf(p,ESO_OFF_CPU_START);
    printf(" Audio ringbuffer: \n");
    hoi_report_rbf(p,ESO_OFF_AUD_START);
    printf(" Video ringbuffer: \n");
    hoi_report_rbf(p,ESO_OFF_VID_START);
} 

/** Interprete status register and returns a string pointer
 * 
 * @param p hardware pointer
 * @return intepreted status register string
 */
char* eso_get_str(void* p) 
{
    char* str = "";
    uint32_t status = HOI_RD32(p, ESO_OFF_STATUS);
    
    // Idle flag
    str = "[0] : ";
    strcat(str, (status & ESO_STATUS_IDLE ? "Idle\n" : "Not idle\n"));
    
    // Error flag
    strcat(str,"[1] : ");
    strcat(str,(status & ESO_STATUS_ERR ? "Error occured\n" : "No error\n"));
    
    if(status & ESO_STATUS_ERR) {
        strcat(str,"error occured\n");
        strcat(str,(status & ESO_STATUS_SIZE_ERR1 ? "[2] : frame size = 0\n" : "[2] : ok\n"));       
        strcat(str,(status & ESO_STATUS_SIZE_ERR2 ? "[3] : frame size > MTU\n" : "[3] : ok\n"));    
        
    } else {
        strcat(str,"no error\n");
    }
    
    return str;
}

/** Sets the DMA burst size in words (32 Bits)
 * 
 * @param p hardware pointer
 * @param v burst size 
 */
void eso_set_burst_sz(void* p, uint32_t v) 
{
    uint32_t temp; 
    temp = v << ESO_DMA_BURST_SIZE_INDEX;
    HOI_WR32(p, ESO_OFF_CFG_DMA, temp&ESO_DMA_BURST_SIZE_MASK);  
}

/** Returns the DMA burst size in words (32 Bits)
 * 
 * @param p hardware pointer
 */
uint32_t eso_get_burst_sz(void* p)
{
    uint32_t temp;
    temp = HOI_RD32(p, ESO_OFF_CFG_DMA) & ESO_DMA_BURST_SIZE_MASK;
    return (temp >> ESO_DMA_BURST_SIZE_INDEX);
}

/** Sets the DMA fifo full threshold in words (32 Bits)
 * 
 * @param p hardware pointer
 * @param v threshold
 */
void eso_set_fifo_th(void* p, uint32_t v) 
{
    uint32_t temp; 
    temp = v << ESO_DMA_FIFO_TRESH_INDEX;
    HOI_WR32(p, ESO_OFF_CFG_DMA, temp&ESO_DMA_FIFO_TRESH_MASK);  
}

/** Returns the DMA fifo full threshold in words (32 Bits)
 * 
 * @param p hardware pointer
 * @return fifo full threshold
 */
uint32_t eso_get_fifo_th(void* p)
{
    uint32_t temp;
    temp = HOI_RD32(p, ESO_OFF_CFG_DMA) & ESO_DMA_FIFO_TRESH_MASK;
    return (temp >> ESO_DMA_FIFO_TRESH_INDEX);
}
