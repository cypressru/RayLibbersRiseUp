#ifndef _DC_UTILS_H_
#define _DC_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#if 0
}
#endif

#include "types.h"
/* ------------------------------------------------------------ */
/* LCD handling */

void dc_lcd_icon_clear (uint8 *screen, int is_black);
void dc_lcd_icon_flip(const uint8 *screen);
void dc_lcd_clear();
/* BMP handling */
int load_bmp (uint16 *raw, const char *filename);
int load_bmp_vmu (uint8 *raw, const char *filename);



#ifdef __cplusplus
}
#endif

#endif

