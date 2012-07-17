#ifndef VIO_DRV_STRUCT_H_
#define VIO_DRV_STRUCT_H_

/** Handle
 */
typedef struct {
    bool            active;
    uint32_t		config;			        //!< config mode
    
    void*           p_vio;                  //!< pointer to vio hardware registers
    void*           p_adv;                  //!< pointer to adv212 direct registers
    t_si598*        si598;                  //!< pointer to si598 struct

    t_adv212        adv;                    //!< ADV212 configuration
    t_vio_pll       pll;                    //!< PLL configuration
    t_video_timing  timing;                 //!< Video timing
    t_video_format  format_in;              //!< Video format input
    t_video_format  format_proc;            //!< Video format internal processing
    t_video_format  format_out;                 //!< Video format output
    t_osd			osd;			        //!< OSD
    
    uint64_t        mean_1;                 //!< Mean of image offset (used for clock control) (32.32 fractional)
    uint64_t        mean_2;                 //!< "
    uint32_t        mean_cnt;               //!< count of captured values to calculate mean
    bool            clock_control_active;   //!< true indicates if clock control is active

    uint32_t        bandwidth;              //!< when encoding this target bandwidth is used
    uint32_t        chroma;                 //!< percent of bandwidth used by chroma

    uint32_t        hw_cfg_old;
} t_vio;

#endif /*VIO_DRV_STRUCT_H_*/
