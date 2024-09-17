
#ifndef __LOX_H__
#define __LOX_H__ 1

/* Definitions and helper functions for LOX vm.
   Sonne 8 micro-controller Rev. Myth/LOX
   Author: mim@ok-schalter.de (Michael/Dosflange@github)
    */

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


/* Application specific opcodes
*/

#define END 0x81 /*0x80 + 16 * Nx + xM // scrounge NM*/


/* Attached virtual devices:
*/

#define SH0_NULL      0    /* NULL device for SH */        
#define SH1_PARLE     1<<4 /* CPU parallel port latch enable */
#define SH2_SMEMA0LE  2<<4 /* SMEM address bit latch 0-7 */
#define SH3_SMEMA1LE  3<<4 /* SMEM address bit latch 8-15 */
#define SH4_SMEMA2LE  4<<4 /* SMEM address bit latch 16-23 */

#define SL0_NULL      0    /* NULL device for SL */
#define SL1_PAROE     1    /* CPU parallel port output enable */
#define SL2_SMEMOE    2    /* SMEM data byte output enable */
#define SL3_SMEMWE    3    /* SMEM data byte write enable */


#endif
