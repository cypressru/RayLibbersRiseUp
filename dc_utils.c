#include <kos.h>
#include <stdio.h>
#include "dc_utils.h"
//#include "dc_vmu.h"
/* ============================================================ */

void
dc_lcd_clear ()
{
  uint8 screen[48 * 32];
  
  dc_lcd_icon_clear (screen, 0);
  dc_lcd_icon_flip (screen);
}

/* ------------------------------------------------------------ */
int
load_bmp (uint16 *raw, const char *filename)
{
  FILE *fp = NULL;
  uint8 bmp[320 * 240 * 3];
  uint8 *p;
  int i;
  
  fp = fopen(filename, "r");
  if (!fp) goto error;
  
  fseek(fp, 54, SEEK_SET);
  if (fread(bmp, sizeof(bmp), 1, fp) != 1) 
    goto error;
  
  fclose(fp);
  
  p = bmp;
  for (i = 0; i < 320 * 240; ++i)
  {
    uint16 r, g, b;
    
    b = *p++ * 32 / 256;
    g = *p++ * 64 / 256;
    r = *p++ * 32 / 256;
    
    *raw++ = (r << 11) | (g << 5) | (b << 0);
  }
  
  return 0;
  
error:
  if (fp) fclose(fp);
  memset (raw, 0, 320 * 240 * 2);
  return -1;
}

/* ------------------------------------------------------------ */
int
load_bmp_vmu (uint8 *raw, const char *filename)
{
  FILE *fp = NULL;

  // 48x32 bit image:
  // 48x32 = 1536 bits = 192 bytes
  // With padding to ensure each line starts on a 4-byte boundary:
  // 64x32 = 2048 bits = 256 bytes

  uint8 bmp[256]; 
  int i, j, k;
  
  fp = fopen(filename, "r");
  if (!fp) goto error;
  
  // Skip the BMP header and the palette.
  fseek(fp, 128, SEEK_SET);

  if (fread(bmp, sizeof(bmp), 1, fp) != 1) 
    goto error;
  
  fclose(fp);
  
  for (i = 0; i < 32; ++i)
  {
    for (j = 0; j < 6; ++j)
    {
      uint8 const val = bmp[i * 8 + j + 2];

      uint8 flipped_byte = 0;
      for (k = 0; k < 4; ++k)
      {
        int const shift_amt = 2 * k + 1;
        int const mask_shift_amt = 3 - k;
        flipped_byte |= (val >> shift_amt) & (1u << mask_shift_amt);
        flipped_byte |= (val << shift_amt) & (128u >> mask_shift_amt);
      }

      raw [i * 6 + (5 - j)] = flipped_byte;
    }
  }
  
  return 0;
  
error:
  if (fp) fclose(fp);
  memset (raw, 0, 192);
  return -1;
}



