/*
 * edid_report.h
 *
 *  Created on: May 24, 2011
 *      Author: stth
 */

#ifndef EDID_REPORT_H_
#define EDID_REPORT_H_

#include <stdint.h>

void edid_report_dsc(uint8_t* p);
char* edid_report_str(uint8_t* p);
void edid_report_detailed_timing(uint8_t *p);
void edid_report_dcm(uint8_t *p);
void edid_report_drl(uint8_t* p);
void edid_report_color_point(uint8_t* p);
void edid_report_est3(uint8_t *p);
void edid_report_std_timing(uint16_t p);
void edid_report_cvt_dsc(uint8_t *p);

#endif /* EDID_REPORT_H_ */
