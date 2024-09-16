
#ifndef __LOX_H__
#define __LOX_H__ 1

#include <u.h>
#include <libc.h>
#include "myth.h"


/* The following are offsets in 0x7F00 page
 */

#define ARG 0xF7 /*Current argument string offset*/
#define POS 0xF8 /*Output text position offset*/
#define VTP 0xF9 /*VOCAB top page*/
#define VTO 0xFA /*VOCAB top offset*/
#define DESTP 0xFB /*Destination address page*/
#define DESTO 0xFC /*Destination address offs*/
#define SRCP 0xFD /*Source address page*/
#define SRCO 0xFE /*Source address offs*/
#define ECODE 0xFF /*Exit Code*/


/* Application specific opcode
*/

#define END 0x81 /*0x80 + 16 * Nx + xM // scrounge NM*/


/* Attached virtual SMEM (synchronous memory) device:
*/

#define SH2_SMEMA0 2<<4 /* SMEM address bit latch 0-7 */
#define SH3_SMEMA1 3<<4 /* SMEM address bit latch 8-15 */
#define SH4_SMEMA2 4<<4 /* SMEM address bit latch 16-23 */
#define SL2_SMEMOE 2    /* SMEM data byte output enable */


#endif
