/*
 * hdcp_drv.h
 *
 * Created on: 28.04.2011
 *     Author: itin
 */

#ifndef HDCP_DRV_H_
#define HDCP_DRV_H_

#include "vsi_drv.h"
#include "vso_drv.h"
#include "eto_drv.h"
#include "eti_drv.h"
#include "asi_drv.h"
#include "aso_drv.h"
#include "adv7441a_drv.h"
#include "adv9889_drv.h"

#include "hoi_msg.h"

// function to compare hdmi and hdcp status
int hdcp_drv_handler(t_eti* h_eti, t_eto* h_eto, t_adv7441a* h_adv7441a, t_adv9889* h_adv9889, t_vsi* h_vsi ,t_vso* h_vso, t_asi* h_asi, t_aso* h_aso, uint32_t* h_drivers, t_queue* event_queue);

#endif /* HDCP_DRV_H_ */
