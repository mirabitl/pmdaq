#pragma once
#define FEBV2_1
static uint32_t gFebVersion=1;
/*!
/def m_encode(t,fpga,ch,side,strip,prc) encode time,fpga,side,stripand petiroc channel in 1 uint64_t 

/def m_traw(m) retrieve traw from encoded
/def m_fpga(m) retrieve fpga from encoded
/def m_side(m) retrieve side from encoded
/def m_strip(m) retrieve strip from encoded
/def m_pr_channel(m) retrieve PETIROC channel from encoded
/def m_tcor(t,tbc0) returns time corrected t-t0 value in ns
*/
#define m_encode(t,fpga,ch,side,strip,prc) ((uint64_t) t +( (uint64_t) (fpga&0x3)<<32)+( (uint64_t) (ch&0x3F)<<34)+( (uint64_t) (side&0x1)<<40)+( (uint64_t) (strip&0x3F)<<41)+( (uint64_t) (prc&0x1F)<<47))

/*!
/def c_side(ch) Side of the channel 1 direct, 0 return
*/
#define m_traw(m) (m&0XFFFFFFFF)
#define m_fpga(m) ((m>>32)&0X3)
#define m_channel(m) ((m>>34)&0X3F)
#define m_side(m) ((m>>40)&0X1)
#define m_strip(m) ((m>>41)&0X3F)
#define m_pr_channel(m) ((m>>47)&0X1F)
#define m_tcor(t,tbc0) (t-tbc0)*2.5/256

// Side 0 LR 1 HR
//#define c_side(ch) ((ch>15)?ch%2:(ch+1)%2)

static uint32_t v2ch2p[32]={1,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,30,28,26,24,22,20,18,16,14,12,10,8,6,4,2,1};
#define c_side(ch) ((gFebVersion==1)?((ch>15)?(ch+1)%2:(ch+1)%2):((ch>15)?0:1))

#define c_local_strip(ch) ((gFebVersion==1)?((ch>15)?((ch-16)/2):16-(ch/2+1)):((ch>15)?(ch-16):15-ch))
#define c_strip(fpga,ch) ((gFebVersion==1)?(fpga*16+c_local_strip(ch)):(fpga*16+c_local_strip(ch)))
#define c_petiroc(ch) ((gFebVersion==1)?((ch>15)?(ch-16)*2:ch*2):(v2ch2p[ch]))
