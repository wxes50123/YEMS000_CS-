/******************************************************************************
* Internal Peripherals Definition for RX72M                                   *
*                                                                             *
*  Copyright (c) 2020-2024, MiSPO Co., Ltd.                                   *
*  All rights reserved.                                                       *
*                                                                             *
* 2020-03-16 Ported to Renesas RX72M                                       FY *
* 2020-05-15 Corrected comments of ICU_IPR050/51 & ICU_IPR114/115/116/117  FY *
* 2022-06-17 Renamed BUS_??? to BSC_???, and added missing BSC definitions FY *
* 2024-11-26 Added EXC_??? macros as exception number to pass to def_inh() FY *
* 2024-12-04 Added ICU_IR???, ICU_FIR~SLIPRCR, WDT_??? & INT_IRQ? etc.     FY *
******************************************************************************/

#ifndef RX72M_H
#define RX72M_H
#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* Input/Output Macros for Peripherals                                       */
/*****************************************************************************/

#define sfr_in(n)       (*(volatile UB __evenaccess *)(n))
#define sfr_out(n,c)    (*(volatile UB __evenaccess *)(n) = (c))
#define sfr_inw(n)      (*(volatile UH __evenaccess *)(n))
#define sfr_outw(n,c)   (*(volatile UH __evenaccess *)(n) = (c))
#define sfr_inl(n)      (*(volatile UW __evenaccess *)(n))
#define sfr_outl(n,c)   (*(volatile UW __evenaccess *)(n) = (c))
#define sfr_set(n,c)    (*(volatile UB __evenaccess *)(n)|= (c))
#define sfr_clr(n,c)    (*(volatile UB __evenaccess *)(n)&=~(c))
#define sfr_setw(n,c)   (*(volatile UH __evenaccess *)(n)|= (c))
#define sfr_clrw(n,c)   (*(volatile UH __evenaccess *)(n)&=~(c))
#define sfr_setl(n,c)   (*(volatile UW __evenaccess *)(n)|= (c))
#define sfr_clrl(n,c)   (*(volatile UW __evenaccess *)(n)&=~(c))

/*****************************************************************************/
/* Operating Modes                                                           */
/*****************************************************************************/

//ver00_0001コメントアウト iodefine.hにも記述があるため
////#define SYSCR0          0x00080006  /* System Control Register 0 */

/*****************************************************************************/
/* Clock Generation Circuit                                                  */
/*****************************************************************************/

#define CGC_SCKCR       0x00080020  /* System Clock Control Register */
#define CGC_MEMWAIT     0x0008101C  /* Memory Wait Cycle Setting Register */
#define CGC_SCKCR2      0x00080024  /* System Clock Control Register 2 */
#define CGC_SCKCR3      0x00080026  /* System Clock Control Register 3 */
#define CGC_PLLCR       0x00080028  /* PLL Control Register */
#define CGC_PLLCR2      0x0008002A  /* PLL Control Register 2 */
#define CGC_BCKCR       0x00080030  /* External Bus Clock Control Register */
#define CGC_MOSCCR      0x00080032  /* Main Clock Oscillator Control Register */
#define CGC_SOSCCR      0x00080033  /* Sub-Clock Oscillator Control Register */
#define CGC_LOCOCR      0x00080034  /* Low-Speed On-Chip Oscillator Control Register */
#define CGC_ILOCOCR     0x00080035  /* IWDT-Dedicated On-Chip Oscillator Control Register */
#define CGC_HOCOCR      0x00080036  /* High-Speed On-Chip Oscillator Control Registger */
#define CGC_HOCOCR2     0x00080037  /* High-Speed On-Chip Oscillator Control Registger 2 */
#define CGC_OSCOVFSR    0x0008003C  /* Oscillation Stability Flag Register */
#define CGC_OSTDCR      0x00080040  /* Oscillation Stop Detection Control Register */
#define CGC_OSTDSR      0x00080041  /* Oscillation Stop Detection Status Register */
#define CGC_MOSCWTCR    0x000800A2  /* Main Clock Oscillator Wait Control Register */
#define CGC_SOSCWTCR    0x000800A3  /* Sub-Clock Oscillator Wait Control Register */
#define CGC_MOFCR       0x0008C293  /* Main Clock Oscillator Forced Oscillation Control Register */
#define CGC_HOCOPCR     0x0008C294  /* High-Speed On-Chip Oscillator Power Supply Control Register */
#define CGC_CKOCR       0x0008003E  /* CLKOUT Output Control Register */
#define CGC_PACKCR      0x00080044  /* Specific-Use Clock Control register */
#define CGC_PPLLCR      0x00080048  /* PPLL Control Register */
#define CGC_PPLLCR2     0x0008004A  /* PPLL Control Register 2 */
#define CGC_PPLLCR3     0x0008004B  /* PPLL Control Register 3 */

/*****************************************************************************/
/* Low Power Consumption                                                     */
/*****************************************************************************/

#define LPC_MSTPCRA     0x00080010  /* Module Stop Control Register A */
#define LPC_MSTPCRB     0x00080014  /* Module Stop Control Register B */
#define LPC_MSTPCRC     0x00080018  /* Module Stop Control Register C */
#define LPC_MSTPCRD     0x0008001C  /* Module Stop Control Register D */

/*****************************************************************************/
/* Register Write Protection Function                                        */
/*****************************************************************************/

#define RWP_PRCR        0x000803FE  /* Protect Register */

/*****************************************************************************/
/* Interrupt Controller (ICUA)                                               */
/*****************************************************************************/

#define ICU_IR016       0x00087010  /* Interrupt Request Register 016 (BUSERR) */
#define ICU_IR017       0x00087011  /* Interrupt Request Register 017 (GROUPIE0) */
#define ICU_IR018       0x00087012  /* Interrupt Request Register 018 (RAMERR) */
#define ICU_IR021       0x00087015  /* Interrupt Request Register 021 (FIFERR) */
#define ICU_IR023       0x00087017  /* Interrupt Request Register 023 (FRDYI) */
#define ICU_IR026       0x0008701A  /* Interrupt Request Register 026 (SWINT2) */
#define ICU_IR027       0x0008701B  /* Interrupt Request Register 027 (SWINT) */
#define ICU_IR028       0x0008701C  /* Interrupt Request Register 028 (CMI0) */
#define ICU_IR029       0x0008701D  /* Interrupt Request Register 029 (CMI1) */
#define ICU_IR030       0x0008701E  /* Interrupt Request Register 030 (CMWI0) */
#define ICU_IR031       0x0008701F  /* Interrupt Request Register 031 (CMWI1) */
#define ICU_IR034       0x00087022  /* Interrupt Request Register 034 (D0FIFO0) */
#define ICU_IR035       0x00087023  /* Interrupt Request Register 035 (D1FIFO0) */
#define ICU_IR038       0x00087026  /* Interrupt Request Register 038 (SPRI0) */
#define ICU_IR039       0x00087027  /* Interrupt Request Register 039 (SPTI0) */
#define ICU_IR040       0x00087028  /* Interrupt Request Register 040 (SPRI1) */
#define ICU_IR041       0x00087029  /* Interrupt Request Register 041 (SPTI1) */
#define ICU_IR042       0x0008702A  /* Interrupt Request Register 042 (SPRI) */
#define ICU_IR043       0x0008702B  /* Interrupt Request Register 043 (SPTI) */
#define ICU_IR044       0x0008702C  /* Interrupt Request Register 044 (SBFAI) */
#define ICU_IR045       0x0008702D  /* Interrupt Request Register 045 (MBFAI) */
#define ICU_IR046       0x0008702E  /* Interrupt Request Register 046 (SSITXI0) */
#define ICU_IR047       0x0008702F  /* Interrupt Request Register 047 (SSIRXI0) */
#define ICU_IR048       0x00087030  /* Interrupt Request Register 048 (SSIRTI1) */
#define ICU_IR050       0x00087032  /* Interrupt Request Register 050 (RIIC_RXI1) */
#define ICU_IR051       0x00087033  /* Interrupt Request Register 051 (RIIC_TXI1) */
#define ICU_IR052       0x00087034  /* Interrupt Request Register 052 (RIIC_RXI0) */
#define ICU_IR053       0x00087035  /* Interrupt Request Register 053 (RIIC_TXI0) */
#define ICU_IR054       0x00087036  /* Interrupt Request Register 054 (RIIC_RXI2) */
#define ICU_IR055       0x00087037  /* Interrupt Request Register 055 (RIIC_TXI2) */
#define ICU_IR058       0x0008703A  /* Interrupt Request Register 058 (SCI_RXI0) */
#define ICU_IR059       0x0008703B  /* Interrupt Request Register 059 (SCI_TXI0) */
#define ICU_IR060       0x0008703C  /* Interrupt Request Register 060 (SCI_RXI1) */
#define ICU_IR061       0x0008703D  /* Interrupt Request Register 061 (SCI_TXI1) */
#define ICU_IR062       0x0008703E  /* Interrupt Request Register 062 (SCI_RXI2) */
#define ICU_IR063       0x0008703F  /* Interrupt Request Register 063 (SCI_TXI2) */
#define ICU_IR064       0x00087040  /* Interrupt Request Register 064 (IRQ0) */
#define ICU_IR065       0x00087041  /* Interrupt Request Register 065 (IRQ1) */
#define ICU_IR066       0x00087042  /* Interrupt Request Register 066 (IRQ2) */
#define ICU_IR067       0x00087043  /* Interrupt Request Register 067 (IRQ3) */
#define ICU_IR068       0x00087044  /* Interrupt Request Register 068 (IRQ4) */
#define ICU_IR069       0x00087045  /* Interrupt Request Register 069 (IRQ5) */
#define ICU_IR070       0x00087046  /* Interrupt Request Register 070 (IRQ6) */
#define ICU_IR071       0x00087047  /* Interrupt Request Register 071 (IRQ7) */
#define ICU_IR072       0x00087048  /* Interrupt Request Register 072 (IRQ8) */
#define ICU_IR073       0x00087049  /* Interrupt Request Register 073 (IRQ9) */
#define ICU_IR074       0x0008704A  /* Interrupt Request Register 074 (IRQ10) */
#define ICU_IR075       0x0008704B  /* Interrupt Request Register 075 (IRQ11) */
#define ICU_IR076       0x0008704C  /* Interrupt Request Register 076 (IRQ12) */
#define ICU_IR077       0x0008704D  /* Interrupt Request Register 077 (IRQ13) */
#define ICU_IR078       0x0008704E  /* Interrupt Request Register 078 (IRQ14) */
#define ICU_IR079       0x0008704F  /* Interrupt Request Register 079 (IRQ15) */
#define ICU_IR080       0x00087050  /* Interrupt Request Register 080 (SCI_RXI3) */
#define ICU_IR081       0x00087051  /* Interrupt Request Register 081 (SCI_TXI3) */
#define ICU_IR082       0x00087052  /* Interrupt Request Register 082 (SCI_RXI4) */
#define ICU_IR083       0x00087053  /* Interrupt Request Register 083 (SCI_TXI4) */
#define ICU_IR084       0x00087054  /* Interrupt Request Register 084 (SCI_RXI5) */
#define ICU_IR085       0x00087055  /* Interrupt Request Register 085 (SCI_TXI5) */
#define ICU_IR086       0x00087056  /* Interrupt Request Register 086 (SCI_RXI6) */
#define ICU_IR087       0x00087057  /* Interrupt Request Register 087 (SCI_TXI6) */
#define ICU_IR088       0x00087058  /* Interrupt Request Register 088 (LVD1) */
#define ICU_IR089       0x00087059  /* Interrupt Request Register 089 (LVD2) */
#define ICU_IR090       0x0008705A  /* Interrupt Request Register 090 (USBR0) */
#define ICU_IR092       0x0008705C  /* Interrupt Request Register 092 (ALM) */
#define ICU_IR093       0x0008705D  /* Interrupt Request Register 093 (PRD) */
#define ICU_IR095       0x0008705F  /* Interrupt Request Register 095 (IWUNI) */
#define ICU_IR096       0x00087060 /* Interrupt Request Register 096 (WUNI) */
#define ICU_IR097       0x00087061  /* Interrupt Request Register 097 (PCDFI) */
#define ICU_IR098       0x00087062  /* Interrupt Request Register 098 (SCI_RXI7) */
#define ICU_IR099       0x00087063  /* Interrupt Request Register 099 (SCI_TXI7) */
#define ICU_IR100       0x00087064  /* Interrupt Request Register 100 (SCI_RXI8) */
#define ICU_IR101       0x00087065  /* Interrupt Request Register 101 (SCI_TXI8) */
#define ICU_IR102       0x00087066  /* Interrupt Request Register 102 (SCI_RXI9) */
#define ICU_IR103       0x00087067  /* Interrupt Request Register 103 (SCI_TXI9) */
#define ICU_IR104       0x00087068  /* Interrupt Request Register 104 (SCI_RXI10) */
#define ICU_IR105       0x00087069  /* Interrupt Request Register 105 (SCI_TXI10) */
#define ICU_IR106       0x0008706A  /* Interrupt Request Register 106 (GRPBE0) */
#define ICU_IR107       0x0008706B  /* Interrupt Request Register 107 (GRPBL2) */
#define ICU_IR108       0x0008706C  /* Interrupt Request Register 108 (SPRI2) */
#define ICU_IR109       0x0008706D  /* Interrupt Request Register 109 (SPTI2) */
#define ICU_IR110       0x0008706E  /* Interrupt Request Register 110 (GRPBL0) */
#define ICU_IR111       0x0008706F  /* Interrupt Request Register 111 (GRPBL1) */
#define ICU_IR112       0x00087070  /* Interrupt Request Register 112 (GRPAL0) */
#define ICU_IR113       0x00087071  /* Interrupt Request Register 113 (GRPAL1) */
#define ICU_IR114       0x00087072  /* Interrupt Request Register 114 (SCI_RXI11) */
#define ICU_IR115       0x00087073  /* Interrupt Request Register 115 (SCI_TXI11) */
#define ICU_IR116       0x00087074  /* Interrupt Request Register 116 (SCI_RXI12) */
#define ICU_IR117       0x00087075  /* Interrupt Request Register 117 (SCI_TXI12) */
#define ICU_IR120       0x00087078  /* Interrupt Request Register 120 (DMAC0I) */
#define ICU_IR121       0x00087079  /* Interrupt Request Register 121 (DMAC1I) */
#define ICU_IR122       0x0008707A  /* Interrupt Request Register 122 (DMAC2I) */
#define ICU_IR123       0x0008707B  /* Interrupt Request Register 123 (DMAC3I) */
#define ICU_IR124       0x0008707C  /* Interrupt Request Register 124 (DMAC74I) */
#define ICU_IR125       0x0008707D  /* Interrupt Request Register 125 (OSTDI) */
#define ICU_IR126       0x0008707E  /* Interrupt Request Register 126 (EXDMAC0I) */
#define ICU_IR127       0x0008707F  /* Interrupt Request Register 127 (EXDMAC1I) */
#define ICU_IR128       0x00087080  /* Interrupt Request Register 128 (INTB128) */
#define ICU_IR129       0x00087081  /* Interrupt Request Register 129 (INTB129) */
#define ICU_IR130       0x00087082  /* Interrupt Request Register 130 (INTB130) */
#define ICU_IR131       0x00087083  /* Interrupt Request Register 131 (INTB131) */
#define ICU_IR132       0x00087084  /* Interrupt Request Register 132 (INTB132) */
#define ICU_IR133       0x00087085  /* Interrupt Request Register 133 (INTB133) */
#define ICU_IR134       0x00087086  /* Interrupt Request Register 134 (INTB134) */
#define ICU_IR135       0x00087087  /* Interrupt Request Register 135 (INTB135) */
#define ICU_IR136       0x00087088  /* Interrupt Request Register 136 (INTB136) */
#define ICU_IR137       0x00087089  /* Interrupt Request Register 137 (INTB137) */
#define ICU_IR138       0x0008708A  /* Interrupt Request Register 138 (INTB138) */
#define ICU_IR139       0x0008708B  /* Interrupt Request Register 139 (INTB139) */
#define ICU_IR140       0x0008708C  /* Interrupt Request Register 140 (INTB140) */
#define ICU_IR141       0x0008708D  /* Interrupt Request Register 141 (INTB141) */
#define ICU_IR142       0x0008708E  /* Interrupt Request Register 142 (INTB142) */
#define ICU_IR143       0x0008708F  /* Interrupt Request Register 143 (INTB143) */
#define ICU_IR144       0x00087090  /* Interrupt Request Register 144 (INTB144) */
#define ICU_IR145       0x00087091  /* Interrupt Request Register 145 (INTB145) */
#define ICU_IR146       0x00087092  /* Interrupt Request Register 146 (INTB146) */
#define ICU_IR147       0x00087093  /* Interrupt Request Register 147 (INTB147) */
#define ICU_IR148       0x00087094  /* Interrupt Request Register 148 (INTB148) */
#define ICU_IR149       0x00087095  /* Interrupt Request Register 149 (INTB149) */
#define ICU_IR150       0x00087096  /* Interrupt Request Register 150 (INTB150) */
#define ICU_IR151       0x00087097  /* Interrupt Request Register 151 (INTB151) */
#define ICU_IR152       0x00087098  /* Interrupt Request Register 152 (INTB152) */
#define ICU_IR153       0x00087099  /* Interrupt Request Register 153 (INTB153) */
#define ICU_IR154       0x0008709A  /* Interrupt Request Register 154 (INTB154) */
#define ICU_IR155       0x0008709B  /* Interrupt Request Register 155 (INTB155) */
#define ICU_IR156       0x0008709C  /* Interrupt Request Register 156 (INTB156) */
#define ICU_IR157       0x0008709D  /* Interrupt Request Register 157 (INTB157) */
#define ICU_IR158       0x0008709E  /* Interrupt Request Register 158 (INTB158) */
#define ICU_IR159       0x0008709F  /* Interrupt Request Register 159 (INTB159) */
#define ICU_IR160       0x000870A0  /* Interrupt Request Register 160 (INTB160) */
#define ICU_IR161       0x000870A1  /* Interrupt Request Register 161 (INTB161) */
#define ICU_IR162       0x000870A2  /* Interrupt Request Register 162 (INTB162) */
#define ICU_IR163       0x000870A3  /* Interrupt Request Register 163 (INTB163) */
#define ICU_IR164       0x000870A4  /* Interrupt Request Register 164 (INTB164) */
#define ICU_IR165       0x000870A5  /* Interrupt Request Register 165 (INTB165) */
#define ICU_IR166       0x000870A6  /* Interrupt Request Register 166 (INTB166) */
#define ICU_IR167       0x000870A7  /* Interrupt Request Register 167 (INTB167) */
#define ICU_IR168       0x000870A8  /* Interrupt Request Register 168 (INTB168) */
#define ICU_IR169       0x000870A9  /* Interrupt Request Register 169 (INTB169) */
#define ICU_IR170       0x000870AA  /* Interrupt Request Register 170 (INTB170) */
#define ICU_IR171       0x000870AB  /* Interrupt Request Register 171 (INTB171) */
#define ICU_IR172       0x000870AC  /* Interrupt Request Register 172 (INTB172) */
#define ICU_IR173       0x000870AD  /* Interrupt Request Register 173 (INTB173) */
#define ICU_IR174       0x000870AE  /* Interrupt Request Register 174 (INTB174) */
#define ICU_IR175       0x000870AF  /* Interrupt Request Register 175 (INTB175) */
#define ICU_IR176       0x000870B0  /* Interrupt Request Register 176 (INTB176) */
#define ICU_IR177       0x000870B1  /* Interrupt Request Register 177 (INTB177) */
#define ICU_IR178       0x000870B2  /* Interrupt Request Register 178 (INTB178) */
#define ICU_IR179       0x000870B3  /* Interrupt Request Register 179 (INTB179) */
#define ICU_IR180       0x000870B4  /* Interrupt Request Register 180 (INTB180) */
#define ICU_IR181       0x000870B5  /* Interrupt Request Register 181 (INTB181) */
#define ICU_IR182       0x000870B6  /* Interrupt Request Register 182 (INTB182) */
#define ICU_IR183       0x000870B7  /* Interrupt Request Register 183 (INTB183) */
#define ICU_IR184       0x000870B8  /* Interrupt Request Register 184 (INTB184) */
#define ICU_IR185       0x000870B9  /* Interrupt Request Register 185 (INTB185) */
#define ICU_IR186       0x000870BA  /* Interrupt Request Register 186 (INTB186) */
#define ICU_IR187       0x000870BB  /* Interrupt Request Register 187 (INTB187) */
#define ICU_IR188       0x000870BC  /* Interrupt Request Register 188 (INTB188) */
#define ICU_IR189       0x000870BD  /* Interrupt Request Register 189 (INTB189) */
#define ICU_IR190       0x000870BE  /* Interrupt Request Register 190 (INTB190) */
#define ICU_IR191       0x000870BF  /* Interrupt Request Register 191 (INTB191) */
#define ICU_IR192       0x000870C0  /* Interrupt Request Register 192 (INTB192) */
#define ICU_IR193       0x000870C1  /* Interrupt Request Register 193 (INTB193) */
#define ICU_IR194       0x000870C2  /* Interrupt Request Register 194 (INTB194) */
#define ICU_IR195       0x000870C3  /* Interrupt Request Register 195 (INTB195) */
#define ICU_IR196       0x000870C4  /* Interrupt Request Register 196 (INTB196) */
#define ICU_IR197       0x000870C5  /* Interrupt Request Register 197 (INTB197) */
#define ICU_IR198       0x000870C6  /* Interrupt Request Register 198 (INTB198) */
#define ICU_IR199       0x000870C7  /* Interrupt Request Register 199 (INTB199) */
#define ICU_IR200       0x000870C8  /* Interrupt Request Register 200 (INTB200) */
#define ICU_IR201       0x000870C9  /* Interrupt Request Register 201 (INTB201) */
#define ICU_IR202       0x000870CA  /* Interrupt Request Register 202 (INTB202) */
#define ICU_IR203       0x000870CB  /* Interrupt Request Register 203 (INTB203) */
#define ICU_IR204       0x000870CC  /* Interrupt Request Register 204 (INTB204) */
#define ICU_IR205       0x000870CD  /* Interrupt Request Register 205 (INTB205) */
#define ICU_IR206       0x000870CE  /* Interrupt Request Register 206 (INTB206) */
#define ICU_IR207       0x000870CF  /* Interrupt Request Register 207 (INTB207) */
#define ICU_IR208       0x000870D0  /* Interrupt Request Register 208 (INTA208) */
#define ICU_IR209       0x000870D1  /* Interrupt Request Register 209 (INTA209) */
#define ICU_IR210       0x000870D2  /* Interrupt Request Register 210 (INTA210) */
#define ICU_IR211       0x000870D3  /* Interrupt Request Register 211 (INTA211) */
#define ICU_IR212       0x000870D4  /* Interrupt Request Register 212 (INTA212) */
#define ICU_IR213       0x000870D5  /* Interrupt Request Register 213 (INTA213) */
#define ICU_IR214       0x000870D6  /* Interrupt Request Register 214 (INTA214) */
#define ICU_IR215       0x000870D7  /* Interrupt Request Register 215 (INTA215) */
#define ICU_IR216       0x000870D8  /* Interrupt Request Register 216 (INTA216) */
#define ICU_IR217       0x000870D9  /* Interrupt Request Register 217 (INTA217) */
#define ICU_IR218       0x000870DA  /* Interrupt Request Register 218 (INTA218) */
#define ICU_IR219       0x000870DB  /* Interrupt Request Register 219 (INTA219) */
#define ICU_IR220       0x000870DC  /* Interrupt Request Register 220 (INTA220) */
#define ICU_IR221       0x000870DD  /* Interrupt Request Register 221 (INTA221) */
#define ICU_IR222       0x000870DE  /* Interrupt Request Register 222 (INTA222) */
#define ICU_IR223       0x000870DF  /* Interrupt Request Register 223 (INTA223) */
#define ICU_IR224       0x000870E0  /* Interrupt Request Register 224 (INTA224) */
#define ICU_IR225       0x000870E1  /* Interrupt Request Register 225 (INTA225) */
#define ICU_IR226       0x000870E2  /* Interrupt Request Register 226 (INTA226) */
#define ICU_IR227       0x000870E3  /* Interrupt Request Register 227 (INTA227) */
#define ICU_IR228       0x000870E4  /* Interrupt Request Register 228 (INTA228) */
#define ICU_IR229       0x000870E5  /* Interrupt Request Register 229 (INTA229) */
#define ICU_IR230       0x000870E6  /* Interrupt Request Register 230 (INTA230) */
#define ICU_IR231       0x000870E7  /* Interrupt Request Register 231 (INTA231) */
#define ICU_IR232       0x000870E8  /* Interrupt Request Register 232 (INTA232) */
#define ICU_IR233       0x000870E9  /* Interrupt Request Register 233 (INTA233) */
#define ICU_IR234       0x000870EA  /* Interrupt Request Register 234 (INTA234) */
#define ICU_IR235       0x000870EB  /* Interrupt Request Register 235 (INTA235) */
#define ICU_IR236       0x000870EC  /* Interrupt Request Register 236 (INTA236) */
#define ICU_IR237       0x000870ED  /* Interrupt Request Register 237 (INTA237) */
#define ICU_IR238       0x000870EE  /* Interrupt Request Register 238 (INTA238) */
#define ICU_IR239       0x000870EF  /* Interrupt Request Register 239 (INTA239) */
#define ICU_IR240       0x000870F0  /* Interrupt Request Register 240 (INTA240) */
#define ICU_IR241       0x000870F1  /* Interrupt Request Register 241 (INTA241) */
#define ICU_IR242       0x000870F2  /* Interrupt Request Register 242 (INTA242) */
#define ICU_IR243       0x000870F3  /* Interrupt Request Register 243 (INTA243) */
#define ICU_IR244       0x000870F4  /* Interrupt Request Register 244 (INTA244) */
#define ICU_IR245       0x000870F5  /* Interrupt Request Register 245 (INTA245) */
#define ICU_IR246       0x000870F6  /* Interrupt Request Register 246 (INTA246) */
#define ICU_IR247       0x000870F7  /* Interrupt Request Register 247 (INTA247) */
#define ICU_IR248       0x000870F8  /* Interrupt Request Register 248 (INTA248) */
#define ICU_IR249       0x000870F9  /* Interrupt Request Register 249 (INTA249) */
#define ICU_IR250       0x000870FA  /* Interrupt Request Register 250 (INTA250) */
#define ICU_IR251       0x000870FB  /* Interrupt Request Register 251 (INTA251) */
#define ICU_IR252       0x000870FC  /* Interrupt Request Register 252 (INTA252) */
#define ICU_IR253       0x000870FD  /* Interrupt Request Register 253 (INTA253) */
#define ICU_IR254       0x000870FE  /* Interrupt Request Register 254 (INTA254) */
#define ICU_IR255       0x000870FF  /* Interrupt Request Register 255 (INTA255) */

#define ICU_IERm        0x00087200  /* Interrupt Request Enable Register m */

#define ICU_IER02       0x00087202  /* Interrupt Request Enable Register 02 */
#define ICU_IER03       0x00087203  /* Interrupt Request Enable Register 03 */
#define ICU_IER04       0x00087204  /* Interrupt Request Enable Register 04 */
#define ICU_IER05       0x00087205  /* Interrupt Request Enable Register 05 */
#define ICU_IER06       0x00087206  /* Interrupt Request Enable Register 06 */
#define ICU_IER07       0x00087207  /* Interrupt Request Enable Register 07 */
#define ICU_IER08       0x00087208  /* Interrupt Request Enable Register 08 */
#define ICU_IER09       0x00087209  /* Interrupt Request Enable Register 09 */
#define ICU_IER0B       0x0008720B  /* Interrupt Request Enable Register 0B */
#define ICU_IER0C       0x0008720C  /* Interrupt Request Enable Register 0C */
#define ICU_IER0E       0x0008720E  /* Interrupt Request Enable Register 0E */
#define ICU_IER0F       0x0008720F  /* Interrupt Request Enable Register 0F */
#define ICU_IER10       0x00087210  /* Interrupt Request Enable Register 10 */
#define ICU_IER11       0x00087211  /* Interrupt Request Enable Register 11 */
#define ICU_IER12       0x00087212  /* Interrupt Request Enable Register 12 */
#define ICU_IER13       0x00087213  /* Interrupt Request Enable Register 13 */
#define ICU_IER14       0x00087214  /* Interrupt Request Enable Register 14 */
#define ICU_IER15       0x00087215  /* Interrupt Request Enable Register 15 */
#define ICU_IER16       0x00087216  /* Interrupt Request Enable Register 16 */
#define ICU_IER17       0x00087217  /* Interrupt Request Enable Register 17 */
#define ICU_IER18       0x00087218  /* Interrupt Request Enable Register 18 */
#define ICU_IER19       0x00087219  /* Interrupt Request Enable Register 19 */
#define ICU_IER1A       0x0008721A  /* Interrupt Request Enable Register 1A */
#define ICU_IER1B       0x0008721B  /* Interrupt Request Enable Register 1B */
#define ICU_IER1C       0x0008721C  /* Interrupt Request Enable Register 1C */
#define ICU_IER1D       0x0008721D  /* Interrupt Request Enable Register 1D */
#define ICU_IER1E       0x0008721E  /* Interrupt Request Enable Register 1E */
#define ICU_IER1F       0x0008721F  /* Interrupt Request Enable Register 1F */

#define ICU_IPR000      0x00087300  /* Interrupt Source Priority Register 000 (BUSERR/GROUPIE0/RAMERR) */
#define ICU_IPR001      0x00087301  /* Interrupt Source Priority Register 001 (FIFERR) */
#define ICU_IPR002      0x00087302  /* Interrupt Source Priority Register 002 (FRDYI) */
#define ICU_IPR003      0x00087303  /* Interrupt Source Priority Register 003 (SWINT2/SWINT) */
#define ICU_IPR004      0x00087304  /* Interrupt Source Priority Register 004 (CMI0) */
#define ICU_IPR005      0x00087305  /* Interrupt Source Priority Register 005 (CMI1) */
#define ICU_IPR006      0x00087306  /* Interrupt Source Priority Register 006 (CMWI0) */
#define ICU_IPR007      0x00087307  /* Interrupt Source Priority Register 007 (CMWI1) */
#define ICU_IPR034      0x00087322  /* Interrupt Source Priority Register 034 (D0FIFO0) */
#define ICU_IPR035      0x00087323  /* Interrupt Source Priority Register 035 (D1FIFO0) */
#define ICU_IPR038      0x00087326  /* Interrupt Source Priority Register 038 (SPRI0) */
#define ICU_IPR039      0x00087327  /* Interrupt Source Priority Register 039 (SPTI0) */
#define ICU_IPR040      0x00087328  /* Interrupt Source Priority Register 040 (SPRI1) */
#define ICU_IPR041      0x00087329  /* Interrupt Source Priority Register 041 (SPTI1) */
#define ICU_IPR042      0x0008732A  /* Interrupt Source Priority Register 042 (SPRI) */
#define ICU_IPR043      0x0008732B  /* Interrupt Source Priority Register 043 (SPTI) */
#define ICU_IPR044      0x0008732C  /* Interrupt Source Priority Register 044 (SBFAI) */
#define ICU_IPR045      0x0008732D  /* Interrupt Source Priority Register 045 (MBFAI) */
#define ICU_IPR046      0x0008732E  /* Interrupt Source Priority Register 046 (SSITXI0) */
#define ICU_IPR047      0x0008732F  /* Interrupt Source Priority Register 047 (SSIRXI0) */
#define ICU_IPR048      0x00087330  /* Interrupt Source Priority Register 048 (SSIRTI1) */
#define ICU_IPR050      0x00087332  /* Interrupt Source Priority Register 050 (RIIC1:RXI1) */
#define ICU_IPR051      0x00087333  /* Interrupt Source Priority Register 051 (RIIC1:TXI1) */
#define ICU_IPR052      0x00087334  /* Interrupt Source Priority Register 052 (RIIC0:RXI0) */
#define ICU_IPR053      0x00087335  /* Interrupt Source Priority Register 053 (RIIC0:TXI0) */
#define ICU_IPR054      0x00087336  /* Interrupt Source Priority Register 054 (RIIC2:RXI2) */
#define ICU_IPR055      0x00087337  /* Interrupt Source Priority Register 055 (RIIC2:TXI2) */
#define ICU_IPR058      0x0008733A  /* Interrupt Source Priority Register 058 (SCI0:RXI0) */
#define ICU_IPR059      0x0008733B  /* Interrupt Source Priority Register 059 (SCI0:TXI0) */
#define ICU_IPR060      0x0008733C  /* Interrupt Source Priority Register 060 (SCI1:RXI1) */
#define ICU_IPR061      0x0008733D  /* Interrupt Source Priority Register 061 (SCI1:TXI1) */
#define ICU_IPR062      0x0008733E  /* Interrupt Source Priority Register 062 (SCI2:RXI2) */
#define ICU_IPR063      0x0008733F  /* Interrupt Source Priority Register 063 (SCI2:RXI2) */
#define ICU_IPR064      0x00087340  /* Interrupt Source Priority Register 064 (IRQ0) */
#define ICU_IPR065      0x00087341  /* Interrupt Source Priority Register 065 (IRQ1) */
#define ICU_IPR066      0x00087342  /* Interrupt Source Priority Register 066 (IRQ2) */
#define ICU_IPR067      0x00087343  /* Interrupt Source Priority Register 067 (IRQ3) */
#define ICU_IPR068      0x00087344  /* Interrupt Source Priority Register 068 (IRQ4) */
#define ICU_IPR069      0x00087345  /* Interrupt Source Priority Register 069 (IRQ5) */
#define ICU_IPR070      0x00087346  /* Interrupt Source Priority Register 070 (IRQ6) */
#define ICU_IPR071      0x00087347  /* Interrupt Source Priority Register 071 (IRQ7) */
#define ICU_IPR072      0x00087348  /* Interrupt Source Priority Register 072 (IRQ8) */
#define ICU_IPR073      0x00087349  /* Interrupt Source Priority Register 073 (IRQ9) */
#define ICU_IPR074      0x0008734A  /* Interrupt Source Priority Register 074 (IRQ10) */
#define ICU_IPR075      0x0008734B  /* Interrupt Source Priority Register 075 (IRQ11) */
#define ICU_IPR076      0x0008734C  /* Interrupt Source Priority Register 076 (IRQ12) */
#define ICU_IPR077      0x0008734D  /* Interrupt Source Priority Register 077 (IRQ13) */
#define ICU_IPR078      0x0008734E  /* Interrupt Source Priority Register 078 (IRQ14) */
#define ICU_IPR079      0x0008734F  /* Interrupt Source Priority Register 079 (IRQ15) */
#define ICU_IPR080      0x00087350  /* Interrupt Source Priority Register 080 (SCI3:RXI3) */
#define ICU_IPR081      0x00087351  /* Interrupt Source Priority Register 081 (SCI3:TXI3) */
#define ICU_IPR082      0x00087352  /* Interrupt Source Priority Register 082 (SCI4:RXI4) */
#define ICU_IPR083      0x00087353  /* Interrupt Source Priority Register 083 (SCI4:TXI4) */
#define ICU_IPR084      0x00087354  /* Interrupt Source Priority Register 084 (SCI5:RXI5) */
#define ICU_IPR085      0x00087355  /* Interrupt Source Priority Register 085 (SCI5:TXI5) */
#define ICU_IPR086      0x00087356  /* Interrupt Source Priority Register 086 (SCI6:RXI6) */
#define ICU_IPR087      0x00087357  /* Interrupt Source Priority Register 087 (SCI6:TXI6) */
#define ICU_IPR088      0x00087358  /* Interrupt Source Priority Register 088 (LVD1) */
#define ICU_IPR089      0x00087359  /* Interrupt Source Priority Register 089 (LVD2) */
#define ICU_IPR090      0x0008735A  /* Interrupt Source Priority Register 090 (USBR0) */
#define ICU_IPR092      0x0008735C  /* Interrupt Source Priority Register 092 (ALM) */
#define ICU_IPR093      0x0008735D  /* Interrupt Source Priority Register 093 (PRD) */
#define ICU_IPR095      0x0008735F  /* Interrupt Source Priority Register 095 (IWUNI) */
#define ICU_IPR096      0x00087360  /* Interrupt Source Priority Register 096 (WUNI) */
#define ICU_IPR097      0x00087361  /* Interrupt Source Priority Register 097 (PCDFI) */
#define ICU_IPR098      0x00087362  /* Interrupt Source Priority Register 098 (SCI7:RXI7) */
#define ICU_IPR099      0x00087363  /* Interrupt Source Priority Register 099 (SCI7:TXI7) */
#define ICU_IPR100      0x00087364  /* Interrupt Source Priority Register 100 (SCI8:RXI8) */
#define ICU_IPR101      0x00087365  /* Interrupt Source Priority Register 101 (SCI8:TXI8) */
#define ICU_IPR102      0x00087366  /* Interrupt Source Priority Register 102 (SCI9:RXI9) */
#define ICU_IPR103      0x00087367  /* Interrupt Source Priority Register 103 (SCI9:TXI9) */
#define ICU_IPR104      0x00087368  /* Interrupt Source Priority Register 104 (SCI10:RXI10) */
#define ICU_IPR105      0x00087369  /* Interrupt Source Priority Register 105 (SCI10:TXI10) */
#define ICU_IPR106      0x0008736A  /* Interrupt Source Priority Register 106 (GROUPBE0) */
#define ICU_IPR107      0x0008736B  /* Interrupt Source Priority Register 107 (GROUPBL2) */
#define ICU_IPR108      0x0008736C  /* Interrupt Source Priority Register 108 (SPRI2) */
#define ICU_IPR109      0x0008736D  /* Interrupt Source Priority Register 109 (SPTI2) */
#define ICU_IPR110      0x0008736E  /* Interrupt Source Priority Register 110 (GROUPBL0) */
#define ICU_IPR111      0x0008736F  /* Interrupt Source Priority Register 111 (GROUPBL1) */
#define ICU_IPR112      0x00087370  /* Interrupt Source Priority Register 112 (GROUPAL0) */
#define ICU_IPR113      0x00087371  /* Interrupt Source Priority Register 113 (GROUPAL1) */
#define ICU_IPR114      0x00087372  /* Interrupt Source Priority Register 114 (SCI11:RXI11) */
#define ICU_IPR115      0x00087373  /* Interrupt Source Priority Register 115 (SCI11:TXI11) */
#define ICU_IPR116      0x00087374  /* Interrupt Source Priority Register 116 (SCI12:RXI12) */
#define ICU_IPR117      0x00087375  /* Interrupt Source Priority Register 117 (SCI12:TXI12) */
#define ICU_IPR120      0x00087378  /* Interrupt Source Priority Register 120 (DMAC0I) */
#define ICU_IPR121      0x00087379  /* Interrupt Source Priority Register 121 (DMAC1I) */
#define ICU_IPR122      0x0008737A  /* Interrupt Source Priority Register 122 (DMAC2I) */
#define ICU_IPR123      0x0008737B  /* Interrupt Source Priority Register 123 (DMAC3I) */
#define ICU_IPR124      0x0008737C  /* Interrupt Source Priority Register 124 (DMAC74I) */
#define ICU_IPR125      0x0008737D  /* Interrupt Source Priority Register 125 (OSTDI) */
#define ICU_IPR126      0x0008737E  /* Interrupt Source Priority Register 126 (EXDMAC0I) */
#define ICU_IPR127      0x0008737F  /* Interrupt Source Priority Register 127 (EXDMAC1I) */
#define ICU_IPR128      0x00087380  /* Interrupt Source Priority Register 128 (INTB128) */
#define ICU_IPR129      0x00087381  /* Interrupt Source Priority Register 129 (INTB129) */
#define ICU_IPR130      0x00087382  /* Interrupt Source Priority Register 130 (INTB130) */
#define ICU_IPR131      0x00087383  /* Interrupt Source Priority Register 131 (INTB131) */
#define ICU_IPR132      0x00087384  /* Interrupt Source Priority Register 132 (INTB132) */
#define ICU_IPR133      0x00087385  /* Interrupt Source Priority Register 133 (INTB133) */
#define ICU_IPR134      0x00087386  /* Interrupt Source Priority Register 134 (INTB134) */
#define ICU_IPR135      0x00087387  /* Interrupt Source Priority Register 135 (INTB135) */
#define ICU_IPR136      0x00087388  /* Interrupt Source Priority Register 136 (INTB136) */
#define ICU_IPR137      0x00087389  /* Interrupt Source Priority Register 137 (INTB137) */
#define ICU_IPR138      0x0008738A  /* Interrupt Source Priority Register 138 (INTB138) */
#define ICU_IPR139      0x0008738B  /* Interrupt Source Priority Register 139 (INTB139) */
#define ICU_IPR140      0x0008738C  /* Interrupt Source Priority Register 140 (INTB140) */
#define ICU_IPR141      0x0008738D  /* Interrupt Source Priority Register 141 (INTB141) */
#define ICU_IPR142      0x0008738E  /* Interrupt Source Priority Register 142 (INTB142) */
#define ICU_IPR143      0x0008738F  /* Interrupt Source Priority Register 143 (INTB143) */
#define ICU_IPR144      0x00087390  /* Interrupt Source Priority Register 144 (INTB144) */
#define ICU_IPR145      0x00087391  /* Interrupt Source Priority Register 145 (INTB145) */
#define ICU_IPR146      0x00087392  /* Interrupt Source Priority Register 146 (INTB146) */
#define ICU_IPR147      0x00087393  /* Interrupt Source Priority Register 147 (INTB147) */
#define ICU_IPR148      0x00087394  /* Interrupt Source Priority Register 148 (INTB148) */
#define ICU_IPR149      0x00087395  /* Interrupt Source Priority Register 149 (INTB149) */
#define ICU_IPR150      0x00087396  /* Interrupt Source Priority Register 150 (INTB150) */
#define ICU_IPR151      0x00087397  /* Interrupt Source Priority Register 151 (INTB151) */
#define ICU_IPR152      0x00087398  /* Interrupt Source Priority Register 152 (INTB152) */
#define ICU_IPR153      0x00087399  /* Interrupt Source Priority Register 153 (INTB153) */
#define ICU_IPR154      0x0008739A  /* Interrupt Source Priority Register 154 (INTB154) */
#define ICU_IPR155      0x0008739B  /* Interrupt Source Priority Register 155 (INTB155) */
#define ICU_IPR156      0x0008739C  /* Interrupt Source Priority Register 156 (INTB156) */
#define ICU_IPR157      0x0008739D  /* Interrupt Source Priority Register 157 (INTB157) */
#define ICU_IPR158      0x0008739E  /* Interrupt Source Priority Register 158 (INTB158) */
#define ICU_IPR159      0x0008739F  /* Interrupt Source Priority Register 159 (INTB159) */
#define ICU_IPR160      0x000873A0  /* Interrupt Source Priority Register 160 (INTB160) */
#define ICU_IPR161      0x000873A1  /* Interrupt Source Priority Register 161 (INTB161) */
#define ICU_IPR162      0x000873A2  /* Interrupt Source Priority Register 162 (INTB162) */
#define ICU_IPR163      0x000873A3  /* Interrupt Source Priority Register 163 (INTB163) */
#define ICU_IPR164      0x000873A4  /* Interrupt Source Priority Register 164 (INTB164) */
#define ICU_IPR165      0x000873A5  /* Interrupt Source Priority Register 165 (INTB165) */
#define ICU_IPR166      0x000873A6  /* Interrupt Source Priority Register 166 (INTB166) */
#define ICU_IPR167      0x000873A7  /* Interrupt Source Priority Register 167 (INTB167) */
#define ICU_IPR168      0x000873A8  /* Interrupt Source Priority Register 168 (INTB168) */
#define ICU_IPR169      0x000873A9  /* Interrupt Source Priority Register 169 (INTB169) */
#define ICU_IPR170      0x000873AA  /* Interrupt Source Priority Register 170 (INTB170) */
#define ICU_IPR171      0x000873AB  /* Interrupt Source Priority Register 171 (INTB171) */
#define ICU_IPR172      0x000873AC  /* Interrupt Source Priority Register 172 (INTB172) */
#define ICU_IPR173      0x000873AD  /* Interrupt Source Priority Register 173 (INTB173) */
#define ICU_IPR174      0x000873AE  /* Interrupt Source Priority Register 174 (INTB174) */
#define ICU_IPR175      0x000873AF  /* Interrupt Source Priority Register 175 (INTB175) */
#define ICU_IPR176      0x000873B0  /* Interrupt Source Priority Register 176 (INTB176) */
#define ICU_IPR177      0x000873B1  /* Interrupt Source Priority Register 177 (INTB177) */
#define ICU_IPR178      0x000873B2  /* Interrupt Source Priority Register 178 (INTB178) */
#define ICU_IPR179      0x000873B3  /* Interrupt Source Priority Register 179 (INTB179) */
#define ICU_IPR180      0x000873B4  /* Interrupt Source Priority Register 180 (INTB180) */
#define ICU_IPR181      0x000873B5  /* Interrupt Source Priority Register 181 (INTB181) */
#define ICU_IPR182      0x000873B6  /* Interrupt Source Priority Register 182 (INTB182) */
#define ICU_IPR183      0x000873B7  /* Interrupt Source Priority Register 183 (INTB183) */
#define ICU_IPR184      0x000873B8  /* Interrupt Source Priority Register 184 (INTB184) */
#define ICU_IPR185      0x000873B9  /* Interrupt Source Priority Register 185 (INTB185) */
#define ICU_IPR186      0x000873BA  /* Interrupt Source Priority Register 186 (INTB186) */
#define ICU_IPR187      0x000873BB  /* Interrupt Source Priority Register 187 (INTB187) */
#define ICU_IPR188      0x000873BC  /* Interrupt Source Priority Register 188 (INTB188) */
#define ICU_IPR189      0x000873BD  /* Interrupt Source Priority Register 189 (INTB189) */
#define ICU_IPR190      0x000873BE  /* Interrupt Source Priority Register 190 (INTB190) */
#define ICU_IPR191      0x000873BF  /* Interrupt Source Priority Register 191 (INTB191) */
#define ICU_IPR192      0x000873C0  /* Interrupt Source Priority Register 192 (INTB192) */
#define ICU_IPR193      0x000873C1  /* Interrupt Source Priority Register 193 (INTB193) */
#define ICU_IPR194      0x000873C2  /* Interrupt Source Priority Register 194 (INTB194) */
#define ICU_IPR195      0x000873C3  /* Interrupt Source Priority Register 195 (INTB195) */
#define ICU_IPR196      0x000873C4  /* Interrupt Source Priority Register 196 (INTB196) */
#define ICU_IPR197      0x000873C5  /* Interrupt Source Priority Register 197 (INTB197) */
#define ICU_IPR198      0x000873C6  /* Interrupt Source Priority Register 198 (INTB198) */
#define ICU_IPR199      0x000873C7  /* Interrupt Source Priority Register 199 (INTB199) */
#define ICU_IPR200      0x000873C8  /* Interrupt Source Priority Register 200 (INTB200) */
#define ICU_IPR201      0x000873C9  /* Interrupt Source Priority Register 201 (INTB201) */
#define ICU_IPR202      0x000873CA  /* Interrupt Source Priority Register 202 (INTB202) */
#define ICU_IPR203      0x000873CB  /* Interrupt Source Priority Register 203 (INTB203) */
#define ICU_IPR204      0x000873CC  /* Interrupt Source Priority Register 204 (INTB204) */
#define ICU_IPR205      0x000873CD  /* Interrupt Source Priority Register 205 (INTB205) */
#define ICU_IPR206      0x000873CE  /* Interrupt Source Priority Register 206 (INTB206) */
#define ICU_IPR207      0x000873CF  /* Interrupt Source Priority Register 207 (INTB207) */
#define ICU_IPR208      0x000873D0  /* Interrupt Source Priority Register 208 (INTA208) */
#define ICU_IPR209      0x000873D1  /* Interrupt Source Priority Register 209 (INTA209) */
#define ICU_IPR210      0x000873D2  /* Interrupt Source Priority Register 210 (INTA210) */
#define ICU_IPR211      0x000873D3  /* Interrupt Source Priority Register 211 (INTA211) */
#define ICU_IPR212      0x000873D4  /* Interrupt Source Priority Register 212 (INTA212) */
#define ICU_IPR213      0x000873D5  /* Interrupt Source Priority Register 213 (INTA213) */
#define ICU_IPR214      0x000873D6  /* Interrupt Source Priority Register 214 (INTA214) */
#define ICU_IPR215      0x000873D7  /* Interrupt Source Priority Register 215 (INTA215) */
#define ICU_IPR216      0x000873D8  /* Interrupt Source Priority Register 216 (INTA216) */
#define ICU_IPR217      0x000873D9  /* Interrupt Source Priority Register 217 (INTA217) */
#define ICU_IPR218      0x000873DA  /* Interrupt Source Priority Register 218 (INTA218) */
#define ICU_IPR219      0x000873DB  /* Interrupt Source Priority Register 219 (INTA219) */
#define ICU_IPR220      0x000873DC  /* Interrupt Source Priority Register 220 (INTA220) */
#define ICU_IPR221      0x000873DD  /* Interrupt Source Priority Register 221 (INTA221) */
#define ICU_IPR222      0x000873DE  /* Interrupt Source Priority Register 222 (INTA222) */
#define ICU_IPR223      0x000873DF  /* Interrupt Source Priority Register 223 (INTA223) */
#define ICU_IPR224      0x000873E0  /* Interrupt Source Priority Register 224 (INTA224) */
#define ICU_IPR225      0x000873E1  /* Interrupt Source Priority Register 225 (INTA225) */
#define ICU_IPR226      0x000873E2  /* Interrupt Source Priority Register 226 (INTA226) */
#define ICU_IPR227      0x000873E3  /* Interrupt Source Priority Register 227 (INTA227) */
#define ICU_IPR228      0x000873E4  /* Interrupt Source Priority Register 228 (INTA228) */
#define ICU_IPR229      0x000873E5  /* Interrupt Source Priority Register 229 (INTA229) */
#define ICU_IPR230      0x000873E6  /* Interrupt Source Priority Register 230 (INTA230) */
#define ICU_IPR231      0x000873E7  /* Interrupt Source Priority Register 231 (INTA231) */
#define ICU_IPR232      0x000873E8  /* Interrupt Source Priority Register 232 (INTA232) */
#define ICU_IPR233      0x000873E9  /* Interrupt Source Priority Register 233 (INTA233) */
#define ICU_IPR234      0x000873EA  /* Interrupt Source Priority Register 234 (INTA234) */
#define ICU_IPR235      0x000873EB  /* Interrupt Source Priority Register 235 (INTA235) */
#define ICU_IPR236      0x000873EC  /* Interrupt Source Priority Register 236 (INTA236) */
#define ICU_IPR237      0x000873ED  /* Interrupt Source Priority Register 237 (INTA237) */
#define ICU_IPR238      0x000873EE  /* Interrupt Source Priority Register 238 (INTA238) */
#define ICU_IPR239      0x000873EF  /* Interrupt Source Priority Register 239 (INTA239) */
#define ICU_IPR240      0x000873F0  /* Interrupt Source Priority Register 240 (INTA240) */
#define ICU_IPR241      0x000873F1  /* Interrupt Source Priority Register 241 (INTA241) */
#define ICU_IPR242      0x000873F2  /* Interrupt Source Priority Register 242 (INTA242) */
#define ICU_IPR243      0x000873F3  /* Interrupt Source Priority Register 243 (INTA243) */
#define ICU_IPR244      0x000873F4  /* Interrupt Source Priority Register 244 (INTA244) */
#define ICU_IPR245      0x000873F5  /* Interrupt Source Priority Register 245 (INTA245) */
#define ICU_IPR246      0x000873F6  /* Interrupt Source Priority Register 246 (INTA246) */
#define ICU_IPR247      0x000873F7  /* Interrupt Source Priority Register 247 (INTA247) */
#define ICU_IPR248      0x000873F8  /* Interrupt Source Priority Register 248 (INTA248) */
#define ICU_IPR249      0x000873F9  /* Interrupt Source Priority Register 249 (INTA249) */
#define ICU_IPR250      0x000873FA  /* Interrupt Source Priority Register 250 (INTA250) */
#define ICU_IPR251      0x000873FB  /* Interrupt Source Priority Register 251 (INTA251) */
#define ICU_IPR252      0x000873FC  /* Interrupt Source Priority Register 252 (INTA252) */
#define ICU_IPR253      0x000873FD  /* Interrupt Source Priority Register 253 (INTA253) */
#define ICU_IPR254      0x000873FE  /* Interrupt Source Priority Register 254 (INTA254) */
#define ICU_IPR255      0x000873FF  /* Interrupt Source Priority Register 255 (INTA255) */

#define ICU_FIR         0x000872F0  /* Fast Interrupt Set Register */
#define ICU_SWINTR      0x000872E0  /* Software Interrupt Generation Register */
#define ICU_SWINT2R     0x000872E1  /* Software Interrupt 2 Generation Register */
#define ICU_DTCER(n)  (0x87100+(n)) /* DTC Transfer Request Enable Register n */
#define ICU_DMRSR0      0x00087400  /* DMAC trigger Select Register 0 */
#define ICU_DMRSR1      0x00087404  /* DMAC trigger Select Register 1 */
#define ICU_DMRSR2      0x00087408  /* DMAC trigger Select Register 2 */
#define ICU_DMRSR3      0x0008740C  /* DMAC trigger Select Register 3 */
#define ICU_DMRSR4      0x00087410  /* DMAC trigger Select Register 4 */
#define ICU_DMRSR5      0x00087414  /* DMAC trigger Select Register 5 */
#define ICU_DMRSR6      0x00087418  /* DMAC trigger Select Register 6 */
#define ICU_DMRSR7      0x0008741C  /* DMAC trigger Select Register 7 */

#define ICU_IRQCR0      0x00087500  /* IRQ Control Register 0 */
#define ICU_IRQCR1      0x00087501  /* IRQ Control Register 1 */
#define ICU_IRQCR2      0x00087502  /* IRQ Control Register 2 */
#define ICU_IRQCR3      0x00087503  /* IRQ Control Register 3 */
#define ICU_IRQCR4      0x00087504  /* IRQ Control Register 4 */
#define ICU_IRQCR5      0x00087505  /* IRQ Control Register 5 */
#define ICU_IRQCR6      0x00087506  /* IRQ Control Register 6 */
#define ICU_IRQCR7      0x00087507  /* IRQ Control Register 7 */
#define ICU_IRQCR8      0x00087508  /* IRQ Control Register 8 */
#define ICU_IRQCR9      0x00087509  /* IRQ Control Register 9 */
#define ICU_IRQCR10     0x0008750A  /* IRQ Control Register 10 */
#define ICU_IRQCR11     0x0008750B  /* IRQ Control Register 11 */
#define ICU_IRQCR12     0x0008750C  /* IRQ Control Register 12 */
#define ICU_IRQCR13     0x0008750D  /* IRQ Control Register 13 */
#define ICU_IRQCR14     0x0008750E  /* IRQ Control Register 14 */
#define ICU_IRQCR15     0x0008750F  /* IRQ Control Register 15 */

#define ICU_IRQFLTE0    0x00087520  /* IRQ Pin Digital Filter Enable Register 0 */
#define ICU_IRQFLTE1    0x00087521  /* IRQ Pin Digital Filter Enable Register 1 */
#define ICU_IRQFLTC0    0x00087528  /* IRQ Pin Digital Filter Setting Register 0 */
#define ICU_IRQFLTC1    0x0008752A  /* IRQ Pin Digital Filter Setting Register 1 */

#define ICU_NMISR       0x00087580  /* Non-Maskable Interrupt Status Register */
#define ICU_NMIER       0x00087581  /* Non-Maskable Interrupt Enable Register */
#define ICU_NMICLR      0x00087582  /* Non-Maskable Interrupt Status Clear Register */
#define ICU_NMICR       0x00087583  /* NMI Pin Interrupt Control Register */
#define ICU_EXNMISR     0x00087584  /* Expanded Non-Maskable Interrupt Status Register */
#define ICU_EXNMIER     0x00087585  /* Expanded Non-Maskable Interrupt Enable Register */
#define ICU_EXNMICLR    0x00087586  /* Expanded Non-Maskable Interrupt Status Clear Register */
#define ICU_NMIFLTE     0x00087590  /* NMI Pin Digital Filter Enable Register */
#define ICU_NMIFLTC     0x00087594  /* NMI Pin Digital Filter Setting Register */

#define ICU_GRPIE0      0x000875B0  /* Group IE0 Interrupt Source Register */
#define ICU_GRPBE0      0x00087600  /* Group BE0 Interrupt Source Register */
#define ICU_GRPBL0      0x00087630  /* Group BL0 Interrupt Source Register */
#define ICU_GRPBL1      0x00087634  /* Group BL1 Interrupt Source Register */
#define ICU_GRPBL2      0x00087638  /* Group BL2 Interrupt Source Register */
#define ICU_GRPAL0      0x00087830  /* Group AL0 Interrupt Source Register */
#define ICU_GRPAL1      0x00087834  /* Group AL1 Interrupt Source Register */

#define ICU_GENIE0      0x000875B4  /* Group IE0 Interrupt Enable Register */
#define ICU_GENBE0      0x00087640  /* Group BE0 Interrupt Enable Register */
#define ICU_GENBL0      0x00087670  /* Group BL0 Interrupt Enable Register */
#define ICU_GENBL1      0x00087674  /* Group BL1 Interrupt Enable Register */
#define ICU_GENBL2      0x00087678  /* Group BL2 Interrupt Enable Register */
#define ICU_GENAL0      0x00087870  /* Group AL0 Interrupt Enable Register */
#define ICU_GENAL1      0x00087874  /* Group AL1 Interrupt Enable Register */

#define ICU_GCRIE0      0x000875B8  /* Group IE0 Interrupt Clear Register */
#define ICU_GCRBE0      0x00087680  /* Group BE0 Interrupt Clear Register */

#define ICU_PIBR0       0x00087700  /* Software Configurable Interrupt B Request Register 0 */
#define ICU_PIBR1       0x00087701  /* Software Configurable Interrupt B Request Register 1 */
#define ICU_PIBR2       0x00087702  /* Software Configurable Interrupt B Request Register 2 */
#define ICU_PIBR3       0x00087703  /* Software Configurable Interrupt B Request Register 3 */
#define ICU_PIBR4       0x00087704  /* Software Configurable Interrupt B Request Register 4 */
#define ICU_PIBR5       0x00087705  /* Software Configurable Interrupt B Request Register 5 */
#define ICU_PIBR6       0x00087706  /* Software Configurable Interrupt B Request Register 6 */
#define ICU_PIBR7       0x00087707  /* Software Configurable Interrupt B Request Register 7 */
#define ICU_PIBR8       0x00087708  /* Software Configurable Interrupt B Request Register 8 */
#define ICU_PIBR9       0x00087709  /* Software Configurable Interrupt B Request Register 9 */
#define ICU_PIBRA       0x0008770A  /* Software Configurable Interrupt B Request Register A */
#define ICU_PIBRB       0x0008770B  /* Software Configurable Interrupt B Request Register B */

#define ICU_PIAR0       0x00087900  /* Software Configurable Interrupt A Request Register 0 */
#define ICU_PIAR1       0x00087901  /* Software Configurable Interrupt A Request Register 1 */
#define ICU_PIAR2       0x00087902  /* Software Configurable Interrupt A Request Register 2 */
#define ICU_PIAR3       0x00087903  /* Software Configurable Interrupt A Request Register 3 */
#define ICU_PIAR4       0x00087904  /* Software Configurable Interrupt A Request Register 4 */
#define ICU_PIAR5       0x00087905  /* Software Configurable Interrupt A Request Register 5 */
#define ICU_PIAR6       0x00087906  /* Software Configurable Interrupt A Request Register 6 */
#define ICU_PIAR7       0x00087907  /* Software Configurable Interrupt A Request Register 7 */
#define ICU_PIAR8       0x00087908  /* Software Configurable Interrupt A Request Register 8 */
#define ICU_PIAR9       0x00087909  /* Software Configurable Interrupt A Request Register 9 */
#define ICU_PIARA       0x0008790A  /* Software Configurable Interrupt A Request Register A */
#define ICU_PIARB       0x0008790B  /* Software Configurable Interrupt A Request Register B */
#define ICU_PIARC       0x0008790C  /* Software Configurable Interrupt A Request Register C */

#define ICU_SLIBXR(n) (0x87700+(n)) /* Software Configurable Interrupt B Source Select Register Xn */
#define ICU_SLIBR(n)  (0x87700+(n)) /* Software Configurable Interrupt B Source Select Register n */
#define ICU_SLIAR(n)  (0x87900+(n)) /* Software Configurable Interrupt A Source Select Register n */
#define ICU_SELEXDR     0x00087A01  /* EXDMAC Trigger Select Register */
#define ICU_SLIPRCR     0x00087A00  /* Software Configurable Interrupt Source Select Register Write Protect Register */

/*****************************************************************************/
/* Bus Controller                                                            */
/*****************************************************************************/

#define BSC_CS0CR       0x00083802  /* CS0 Control Register */
#define BSC_CS1CR       0x00083812  /* CS1 Control Register */
#define BSC_CS2CR       0x00083822  /* CS2 Control Register */
#define BSC_CS3CR       0x00083832  /* CS3 Control Register */
#define BSC_CS4CR       0x00083842  /* CS4 Control Register */
#define BSC_CS5CR       0x00083852  /* CS5 Control Register */
#define BSC_CS6CR       0x00083862  /* CS6 Control Register */
#define BSC_CS7CR       0x00083872  /* CS7 Control Register */
#define BSC_CS0REC      0x0008380A  /* CS0 Recovery Cycle Register */
#define BSC_CS1REC      0x0008381A  /* CS1 Recovery Cycle Register */
#define BSC_CS2REC      0x0008382A  /* CS2 Recovery Cycle Register */
#define BSC_CS3REC      0x0008383A  /* CS3 Recovery Cycle Register */
#define BSC_CS4REC      0x0008384A  /* CS4 Recovery Cycle Register */
#define BSC_CS5REC      0x0008385A  /* CS5 Recovery Cycle Register */
#define BSC_CS6REC      0x0008386A  /* CS6 Recovery Cycle Register */
#define BSC_CS7REC      0x0008387A  /* CS7 Recovery Cycle Register */
#define BSC_CSRECEN     0x00083880  /* CS Recovery Cycle Insertion Enable Register */
#define BSC_CS0MOD      0x00083002  /* CS0 Mode Register */
#define BSC_CS1MOD      0x00083012  /* CS1 Mode Register */
#define BSC_CS2MOD      0x00083022  /* CS2 Mode Register */
#define BSC_CS3MOD      0x00083032  /* CS3 Mode Register */
#define BSC_CS4MOD      0x00083042  /* CS4 Mode Register */
#define BSC_CS5MOD      0x00083052  /* CS5 Mode Register */
#define BSC_CS6MOD      0x00083062  /* CS6 Mode Register */
#define BSC_CS7MOD      0x00083072  /* CS7 Mode Register */
#define BSC_CS0WCR1     0x00083004  /* CS0 Wait Control Register 1 */
#define BSC_CS1WCR1     0x00083014  /* CS1 Wait Control Register 1 */
#define BSC_CS2WCR1     0x00083024  /* CS2 Wait Control Register 1 */
#define BSC_CS3WCR1     0x00083034  /* CS3 Wait Control Register 1 */
#define BSC_CS4WCR1     0x00083044  /* CS4 Wait Control Register 1 */
#define BSC_CS5WCR1     0x00083054  /* CS5 Wait Control Register 1 */
#define BSC_CS6WCR1     0x00083064  /* CS6 Wait Control Register 1 */
#define BSC_CS7WCR1     0x00083074  /* CS7 Wait Control Register 1 */
#define BSC_CS0WCR2     0x00083008  /* CS0 Wait Control Register 2 */
#define BSC_CS1WCR2     0x00083018  /* CS1 Wait Control Register 2 */
#define BSC_CS2WCR2     0x00083028  /* CS2 Wait Control Register 2 */
#define BSC_CS3WCR2     0x00083038  /* CS3 Wait Control Register 2 */
#define BSC_CS4WCR2     0x00083048  /* CS4 Wait Control Register 2 */
#define BSC_CS5WCR2     0x00083058  /* CS5 Wait Control Register 2 */
#define BSC_CS6WCR2     0x00083068  /* CS6 Wait Control Register 2 */
#define BSC_CS7WCR2     0x00083078  /* CS7 Wait Control Register 2 */
#define BSC_SDCCR       0x00083C00  /* SDC Control Register */
#define BSC_SDCMOD      0x00083C01  /* SDC Mode Register */
#define BSC_SDAMOD      0x00083C02  /* SDRAM Access Mode Register */
#define BSC_SDSELF      0x00083C10  /* SDRAM Self-Refresh Control Register */
#define BSC_SDRFCR      0x00083C14  /* SDRAM Refresh Control Register */
#define BSC_SDRFEN      0x00083C16  /* SDRAM Auto-Refresh Control Register */
#define BSC_SDICR       0x00083C20  /* SDRAM Initialization Sequence Control Register */
#define BSC_SDIR        0x00083C24  /* SDRAM Initialization Register */
#define BSC_SDADR       0x00083C40  /* SDRAM Address Register */
#define BSC_SDTR        0x00083C44  /* SDRAM Timing Register */
#define BSC_SDMOD       0x00083C48  /* SDRAM Mode Register */
#define BSC_SDSR        0x00083C50  /* SDRAM Status Register */
#define BSC_BERCLR      0x00081300  /* Bus Error Status Clear Register */
#define BSC_BEREN       0x00081304  /* Bus Error Monitoring Enable Register */
#define BSC_BERSR1      0x00081308  /* Bus Error Status Register 1 */
#define BSC_BERSR2      0x0008130A  /* Bus Error Status Register 2 */
#define BSC_BUSPRI      0x00081310  /* Bus Priority Control Register */
#define BSC_EBMAPCR     0x000C5800  /* Extended Bus Master Priority Control Register */

/*****************************************************************************/
/* I/O Ports                                                                 */
/*****************************************************************************/

#define P0_PDR          0x0008C000  /* Port Direction Register */
#define P0_PODR         0x0008C020  /* Port Output Data Register */
#define P0_PIDR         0x0008C040  /* Port Input Register */
#define P0_PMR          0x0008C060  /* Port Mode Register */
#define P0_ODR0         0x0008C080  /* Open Drain Control Register 0 */
#define P0_ODR1         0x0008C081  /* Open Drain Control Register 1 */
#define P0_PCR          0x0008C0C0  /* Pull-Up Resistor Control Register */
#define P0_DSCR         0x0008C0E0  /* Drive Capacity Control Register */
#define P0_DSCR2        0x0008C128  /* Drive Capacity Control Register 2 */

#define P1_PDR          0x0008C001
#define P1_PODR         0x0008C021
#define P1_PIDR         0x0008C041
#define P1_PMR          0x0008C061
#define P1_ODR0         0x0008C082
#define P1_ODR1         0x0008C083
#define P1_PCR          0x0008C0C1
#define P1_DSCR         0x0008C0E1
#define P1_DSCR2        0x0008C129

#define P2_PDR          0x0008C002
#define P2_PODR         0x0008C022
#define P2_PIDR         0x0008C042
#define P2_PMR          0x0008C062
#define P2_ODR0         0x0008C084
#define P2_ODR1         0x0008C085
#define P2_PCR          0x0008C0C2
#define P2_DSCR         0x0008C0E2
#define P2_DSCR2        0x0008C12A

#define P3_PDR          0x0008C003
#define P3_PODR         0x0008C023
#define P3_PIDR         0x0008C043
#define P3_PMR          0x0008C063
#define P3_ODR0         0x0008C086
#define P3_ODR1         0x0008C087
#define P3_PCR          0x0008C0C3
#define P3_DSCR2        0x0008C12B

#define P4_PDR          0x0008C004
#define P4_PODR         0x0008C024
#define P4_PIDR         0x0008C044
#define P4_PMR          0x0008C064
#define P4_ODR0         0x0008C088
#define P4_ODR1         0x0008C089
#define P4_PCR          0x0008C0C4

#define P5_PDR          0x0008C005
#define P5_PODR         0x0008C025
#define P5_PIDR         0x0008C045
#define P5_PMR          0x0008C065
#define P5_ODR0         0x0008C08A
#define P5_ODR1         0x0008C08B
#define P5_PCR          0x0008C0C5
#define P5_DSCR         0x0008C0E5
#define P5_DSCR2        0x0008C12D

#define P6_PDR          0x0008C006
#define P6_PODR         0x0008C026
#define P6_PIDR         0x0008C046
#define P6_PMR          0x0008C066
#define P6_ODR0         0x0008C08C
#define P6_ODR1         0x0008C08D
#define P6_PCR          0x0008C0C6
#define P6_DSCR2        0x0008C12E

#define P7_PDR          0x0008C007
#define P7_PODR         0x0008C027
#define P7_PIDR         0x0008C047
#define P7_PMR          0x0008C067
#define P7_ODR0         0x0008C08E
#define P7_ODR1         0x0008C08F
#define P7_PCR          0x0008C0C7
#define P7_DSCR         0x0008C0E7
#define P7_DSCR2        0x0008C12F

#define P8_PDR          0x0008C008
#define P8_PODR         0x0008C028
#define P8_PIDR         0x0008C048
#define P8_PMR          0x0008C068
#define P8_ODR0         0x0008C090
#define P8_ODR1         0x0008C091
#define P8_PCR          0x0008C0C8
#define P8_DSCR         0x0008C0E8
#define P8_DSCR2        0x0008C130

#define P9_PDR          0x0008C009
#define P9_PODR         0x0008C029
#define P9_PIDR         0x0008C049
#define P9_PMR          0x0008C069
#define P9_ODR0         0x0008C092
#define P9_ODR1         0x0008C093
#define P9_PCR          0x0008C0C9
#define P9_DSCR         0x0008C0E9
#define P9_DSCR2        0x0008C131

#define PA_PDR          0x0008C00A
#define PA_PODR         0x0008C02A
#define PA_PIDR         0x0008C04A
#define PA_PMR          0x0008C06A
#define PA_ODR0         0x0008C094
#define PA_ODR1         0x0008C095
#define PA_PCR          0x0008C0CA
#define PA_DSCR         0x0008C0EA
#define PA_DSCR2        0x0008C132

#define PB_PDR          0x0008C00B
#define PB_PODR         0x0008C02B
#define PB_PIDR         0x0008C04B
#define PB_PMR          0x0008C06B
#define PB_ODR0         0x0008C096
#define PB_ODR1         0x0008C097
#define PB_PCR          0x0008C0CB
#define PB_DSCR         0x0008C0EB
#define PB_DSCR2        0x0008C133

#define PC_PDR          0x0008C00C
#define PC_PODR         0x0008C02C
#define PC_PIDR         0x0008C04C
#define PC_PMR          0x0008C06C
#define PC_ODR0         0x0008C098
#define PC_ODR1         0x0008C099
#define PC_PCR          0x0008C0CC
#define PC_DSCR         0x0008C0EC
#define PC_DSCR2        0x0008C134

#define PD_PDR          0x0008C00D
#define PD_PODR         0x0008C02D
#define PD_PIDR         0x0008C04D
#define PD_PMR          0x0008C06D
#define PD_ODR0         0x0008C09A
#define PD_ODR1         0x0008C09B
#define PD_PCR          0x0008C0CD
#define PD_DSCR         0x0008C0ED
#define PD_DSCR2        0x0008C135

#define PE_PDR          0x0008C00E
#define PE_PODR         0x0008C02E
#define PE_PIDR         0x0008C04E
#define PE_PMR          0x0008C06E
#define PE_ODR0         0x0008C09C
#define PE_ODR1         0x0008C09D
#define PE_PCR          0x0008C0CE
#define PE_DSCR         0x0008C0EE
#define PE_DSCR2        0x0008C136

#define PF_PDR          0x0008C00F
#define PF_PODR         0x0008C02F
#define PF_PIDR         0x0008C04F
#define PF_PMR          0x0008C06F
#define PF_ODR0         0x0008C09E
#define PF_ODR1         0x0008C09F
#define PF_PCR          0x0008C0CF

#define PG_PDR          0x0008C010
#define PG_PODR         0x0008C030
#define PG_PIDR         0x0008C050
#define PG_PMR          0x0008C070
#define PG_ODR0         0x0008C0A0
#define PG_ODR1         0x0008C0A1
#define PG_PCR          0x0008C0D0
#define PG_DSCR         0x0008C0F0
#define PG_DSCR2        0x0008C138

#define PH_PDR          0x0008C011
#define PH_PODR         0x0008C031
#define PH_PIDR         0x0008C051
#define PH_PMR          0x0008C071
#define PH_ODR0         0x0008C0A2
#define PH_ODR1         0x0008C0A3
#define PH_PCR          0x0008C0D1
#define PH_DSCR         0x0008C0F1
#define PH_DSCR2        0x0008C139

#define PJ_PDR          0x0008C012
#define PJ_PODR         0x0008C032
#define PJ_PIDR         0x0008C052
#define PJ_PMR          0x0008C072
#define PJ_ODR0         0x0008C0A4
#define PJ_ODR1         0x0008C0A5
#define PJ_PCR          0x0008C0D2
#define PJ_DSCR         0x0008C0F2
#define PJ_DSCR2        0x0008C13A

#define PK_PDR          0x0008C013
#define PK_PODR         0x0008C033
#define PK_PIDR         0x0008C053
#define PK_PMR          0x0008C073
#define PK_ODR0         0x0008C0A6
#define PK_ODR1         0x0008C0A7
#define PK_PCR          0x0008C0D3
#define PK_DSCR         0x0008C0F3
#define PK_DSCR2        0x0008C13B

#define PL_PDR          0x0008C014
#define PL_PODR         0x0008C034
#define PL_PIDR         0x0008C054
#define PL_PMR          0x0008C074
#define PL_ODR0         0x0008C0A8
#define PL_ODR1         0x0008C0A9
#define PL_PCR          0x0008C0D4
#define PL_DSCR         0x0008C0F4
#define PL_DSCR2        0x0008C13C

#define PM_PDR          0x0008C015
#define PM_PODR         0x0008C035
#define PM_PIDR         0x0008C055
#define PM_PMR          0x0008C075
#define PM_ODR0         0x0008C0AA
#define PM_ODR1         0x0008C0AB
#define PM_PCR          0x0008C0D5
#define PM_DSCR         0x0008C0F5
#define PM_DSCR2        0x0008C13D

#define PN_PDR          0x0008C016
#define PN_PODR         0x0008C036
#define PN_PIDR         0x0008C056
#define PN_PMR          0x0008C076
#define PN_ODR0         0x0008C0AC
#define PN_ODR1         0x0008C0AD
#define PN_PCR          0x0008C0D6
#define PN_DSCR         0x0008C0F6
#define PN_DSCR2        0x0008C13E

#define PQ_PDR          0x0008C017
#define PQ_PODR         0x0008C037
#define PQ_PIDR         0x0008C057
#define PQ_PMR          0x0008C077
#define PQ_ODR0         0x0008C0AE
#define PQ_ODR1         0x0008C0AF
#define PQ_PCR          0x0008C0D7
#define PQ_DSCR         0x0008C0F7
#define PQ_DSCR2        0x0008C13F

/*****************************************************************************/
/* Multi-Function Pin Controller (MPC)                                       */
/*****************************************************************************/

#define MPC_PWPR        0x0008C11F  /* Write-Protect Register */
#define PWPR_PFSWE      (1 << 6)    /* PFS Register Write Enable */
#define PWPR_B0WI       (1 << 7)    /* PFSWE Bit Write Disable */

#define MPC_P00PFS      0x0008C140  /* P0n Pin Function Control Register */
#define MPC_P01PFS      0x0008C141
#define MPC_P02PFS      0x0008C142
#define MPC_P03PFS      0x0008C143
#define MPC_P05PFS      0x0008C145
#define MPC_P07PFS      0x0008C147

#define MPC_P10PFS      0x0008C148  /* P1n Pin Function Control Register */
#define MPC_P11PFS      0x0008C149
#define MPC_P12PFS      0x0008C14A
#define MPC_P13PFS      0x0008C14B
#define MPC_P14PFS      0x0008C14C
#define MPC_P15PFS      0x0008C14D
#define MPC_P16PFS      0x0008C14E
#define MPC_P17PFS      0x0008C14F

#define MPC_P20PFS      0x0008C150  /* P2n Pin Function Control Register */
#define MPC_P21PFS      0x0008C151
#define MPC_P22PFS      0x0008C152
#define MPC_P23PFS      0x0008C153
#define MPC_P24PFS      0x0008C154
#define MPC_P25PFS      0x0008C155
#define MPC_P26PFS      0x0008C156
#define MPC_P27PFS      0x0008C157

#define MPC_P30PFS      0x0008C158  /* P3n Pin Function Control Register */
#define MPC_P31PFS      0x0008C159
#define MPC_P32PFS      0x0008C15A
#define MPC_P33PFS      0x0008C15B
#define MPC_P34PFS      0x0008C15C

#define MPC_P40PFS      0x0008C160  /* P4n Pin Function Control Register */
#define MPC_P41PFS      0x0008C161
#define MPC_P42PFS      0x0008C162
#define MPC_P43PFS      0x0008C163
#define MPC_P44PFS      0x0008C164
#define MPC_P45PFS      0x0008C165
#define MPC_P46PFS      0x0008C166
#define MPC_P47PFS      0x0008C167

#define MPC_P50PFS      0x0008C168  /* P5n Pin Function Control Register */
#define MPC_P51PFS      0x0008C169
#define MPC_P52PFS      0x0008C16A
#define MPC_P54PFS      0x0008C16C
#define MPC_P55PFS      0x0008C16D
#define MPC_P56PFS      0x0008C16E
#define MPC_P57PFS      0x0008C16F

#define MPC_P60PFS      0x0008C170  /* P6n Pin Function Control Register */
#define MPC_P61PFS      0x0008C171
#define MPC_P62PFS      0x0008C172
#define MPC_P63PFS      0x0008C173
#define MPC_P64PFS      0x0008C174
#define MPC_P66PFS      0x0008C176
#define MPC_P67PFS      0x0008C177

#define MPC_P70PFS      0x0008C178  /* P7n Pin Function Control Register */
#define MPC_P71PFS      0x0008C179
#define MPC_P72PFS      0x0008C17A
#define MPC_P73PFS      0x0008C17B
#define MPC_P74PFS      0x0008C17C
#define MPC_P75PFS      0x0008C17D
#define MPC_P76PFS      0x0008C17E
#define MPC_P77PFS      0x0008C17F

#define MPC_P80PFS      0x0008C180  /* P8n Pin Function Control Register */
#define MPC_P81PFS      0x0008C181
#define MPC_P82PFS      0x0008C182
#define MPC_P83PFS      0x0008C183
#define MPC_P84PFS      0x0008C184
#define MPC_P85PFS      0x0008C185
#define MPC_P86PFS      0x0008C186
#define MPC_P87PFS      0x0008C187

#define MPC_P90PFS      0x0008C188  /* P9n Pin Function Control Register */
#define MPC_P91PFS      0x0008C189
#define MPC_P92PFS      0x0008C18A
#define MPC_P93PFS      0x0008C18B
#define MPC_P94PFS      0x0008C18C
#define MPC_P95PFS      0x0008C18D
#define MPC_P96PFS      0x0008C18E
#define MPC_P97PFS      0x0008C18F

#define MPC_PA0PFS      0x0008C190  /* PAn Pin Function Control Register */
#define MPC_PA1PFS      0x0008C191
#define MPC_PA2PFS      0x0008C192
#define MPC_PA3PFS      0x0008C193
#define MPC_PA4PFS      0x0008C194
#define MPC_PA5PFS      0x0008C195
#define MPC_PA6PFS      0x0008C196
#define MPC_PA7PFS      0x0008C197

#define MPC_PB0PFS      0x0008C198  /* PBn Pin Function Control Register */
#define MPC_PB1PFS      0x0008C199
#define MPC_PB2PFS      0x0008C19A
#define MPC_PB3PFS      0x0008C19B
#define MPC_PB4PFS      0x0008C19C
#define MPC_PB5PFS      0x0008C19D
#define MPC_PB6PFS      0x0008C19E
#define MPC_PB7PFS      0x0008C19F

#define MPC_PC0PFS      0x0008C1A0  /* PCn Pin Function Control Register */
#define MPC_PC1PFS      0x0008C1A1
#define MPC_PC2PFS      0x0008C1A2
#define MPC_PC3PFS      0x0008C1A3
#define MPC_PC4PFS      0x0008C1A4
#define MPC_PC5PFS      0x0008C1A5
#define MPC_PC6PFS      0x0008C1A6
#define MPC_PC7PFS      0x0008C1A7

#define MPC_PD0PFS      0x0008C1A8  /* PDn Pin Function Control Register */
#define MPC_PD1PFS      0x0008C1A9
#define MPC_PD2PFS      0x0008C1AA
#define MPC_PD3PFS      0x0008C1AB
#define MPC_PD4PFS      0x0008C1AC
#define MPC_PD5PFS      0x0008C1AD
#define MPC_PD6PFS      0x0008C1AE
#define MPC_PD7PFS      0x0008C1AF

#define MPC_PE0PFS      0x0008C1B0  /* PEn Pin Function Control Register */
#define MPC_PE1PFS      0x0008C1B1
#define MPC_PE2PFS      0x0008C1B2
#define MPC_PE3PFS      0x0008C1B3
#define MPC_PE4PFS      0x0008C1B4
#define MPC_PE5PFS      0x0008C1B5
#define MPC_PE6PFS      0x0008C1B6
#define MPC_PE7PFS      0x0008C1B7

#define MPC_PF0PFS      0x0008C1B8  /* PFn Pin Function Control Register */
#define MPC_PF1PFS      0x0008C1B9
#define MPC_PF2PFS      0x0008C1BA
#define MPC_PF5PFS      0x0008C1BD

#define MPC_PG0PFS      0x0008C1C0  /* PGn Pin Function Control Register */
#define MPC_PG1PFS      0x0008C1C1
#define MPC_PG2PFS      0x0008C1C2
#define MPC_PG3PFS      0x0008C1C3
#define MPC_PG4PFS      0x0008C1C4
#define MPC_PG5PFS      0x0008C1C5
#define MPC_PG6PFS      0x0008C1C6
#define MPC_PG7PFS      0x0008C1C7

#define MPC_PH0PFS      0x0008C1C8  /* PHn Pin Function Control Register */
#define MPC_PH1PFS      0x0008C1C9
#define MPC_PH2PFS      0x0008C1CA
#define MPC_PH3PFS      0x0008C1CB
#define MPC_PH4PFS      0x0008C1CC
#define MPC_PH5PFS      0x0008C1CD
#define MPC_PH6PFS      0x0008C1CE
#define MPC_PH7PFS      0x0008C1CF

#define MPC_PJ0PFS      0x0008C1D0  /* PJn Pin Function Control Register */
#define MPC_PJ1PFS      0x0008C1D1
#define MPC_PJ2PFS      0x0008C1D2
#define MPC_PJ3PFS      0x0008C1D3
#define MPC_PJ5PFS      0x0008C1D5

#define MPC_PK0PFS      0x0008C1D8  /* PKn Pin Function Control Register */
#define MPC_PK1PFS      0x0008C1D9
#define MPC_PK2PFS      0x0008C1DA
#define MPC_PK3PFS      0x0008C1DB
#define MPC_PK4PFS      0x0008C1DC
#define MPC_PK5PFS      0x0008C1DD
#define MPC_PK6PFS      0x0008C1DE
#define MPC_PK7PFS      0x0008C1DF

#define MPC_PL0PFS      0x0008C1E0  /* PLn Pin Function Control Register */
#define MPC_PL1PFS      0x0008C1E1
#define MPC_PL2PFS      0x0008C1E2
#define MPC_PL3PFS      0x0008C1E3
#define MPC_PL4PFS      0x0008C1E4
#define MPC_PL5PFS      0x0008C1E5
#define MPC_PL6PFS      0x0008C1E6
#define MPC_PL7PFS      0x0008C1E7

#define MPC_PM0PFS      0x0008C1E8  /* PMn Pin Function Control Register */
#define MPC_PM1PFS      0x0008C1E9
#define MPC_PM2PFS      0x0008C1EA
#define MPC_PM3PFS      0x0008C1EB
#define MPC_PM4PFS      0x0008C1EC
#define MPC_PM5PFS      0x0008C1ED
#define MPC_PM6PFS      0x0008C1EE
#define MPC_PM7PFS      0x0008C1EF

#define MPC_PN0PFS      0x0008C1F0  /* PNn Pin Function Control Register */
#define MPC_PN1PFS      0x0008C1F1
#define MPC_PN2PFS      0x0008C1F2
#define MPC_PN3PFS      0x0008C1F3
#define MPC_PN4PFS      0x0008C1F4
#define MPC_PN5PFS      0x0008C1F5

#define MPC_PQ0PFS      0x0008C1F8  /* PMn Pin Function Control Register */
#define MPC_PQ1PFS      0x0008C1F9
#define MPC_PQ2PFS      0x0008C1FA
#define MPC_PQ3PFS      0x0008C1FB
#define MPC_PQ4PFS      0x0008C1FC
#define MPC_PQ5PFS      0x0008C1FD
#define MPC_PQ6PFS      0x0008C1FE
#define MPC_PQ7PFS      0x0008C1FF

#define MPC_PFCSE       0x0008C100  /* CS Output Enable Register */
#define MPC_PFCSS0      0x0008C102  /* CS Output Pin Select Register 0 */
#define MPC_PFCSS1      0x0008C103  /* CS Output Pin Select Register 1 */
#define MPC_PFAOE0      0x0008C104  /* Address Output Enable Register 0 */
#define MPC_PFAOE1      0x0008C105  /* Address Output Enable Register 1 */
#define MPC_PFBCR0      0x0008C106  /* External Bus Control Register 0 */
#define MPC_PFBCR1      0x0008C107  /* External Bus Control Register 1 */
#define MPC_PFBCR2      0x0008C108  /* External Bus Control Register 2 */
#define MPC_PFBCR3      0x0008C109  /* External Bus Control Register 3 */
#define MPC_PFENET      0x0008C10E  /* Ethernet Control Register */

/*****************************************************************************/
/* Compare Match Timer (CMT)                                                 */
/*****************************************************************************/

#define CMT_CMSTR0      0x00088000  /* Compare Match Timer Start Register 0 */
#define CMT_CMSTR1      0x00088010  /* Compare Match Timer Start Register 1 */

#define CMT0_CMCR       0x00088002  /* Compare Match Timer Control Register */
#define CMT0_CMCNT      0x00088004  /* Compare Match Timer Counter */
#define CMT0_CMCOR      0x00088006  /* Compare Match Timer Constant Register */

#define CMT1_CMCR       0x00088008
#define CMT1_CMCNT      0x0008800A
#define CMT1_CMCOR      0x0008800C

#define CMT2_CMCR       0x00088012
#define CMT2_CMCNT      0x00088014
#define CMT2_CMCOR      0x00088016

#define CMT3_CMCR       0x00088018
#define CMT3_CMCNT      0x0008801A
#define CMT3_CMCOR      0x0008801C

#define CMCR_CMIE       (1 << 6)    /* Compare Match Interrupt Enable */

/*****************************************************************************/
/* Watchdog Timer (WDTA)                                                     */
/*****************************************************************************/

#define WDT_WDTRR       0x00088020  /* WDT Refresh Register */
#define WDT_WDTCR       0x00088022  /* WDT Control Register */
#define WDT_WDTSR       0x00088024  /* WDT Status Register */
#define WDT_WDTRCR      0x00088026  /* WDT Reset Control Register */

/*****************************************************************************/
/* Independent Watchdog Timer (IWDTa)                                        */
/*****************************************************************************/

#define IWDT_IWDTRR     0x00088030  /* IWDT Refresh Register */
#define IWDT_IWDTCR     0x00088032  /* IWDT Control Register */
#define IWDT_IWDTSR     0x00088034  /* IWDT Status Register */
#define IWDT_IWDTRCR    0x00088036  /* IWDT Reset Control Register */
#define IWDT_IWDTCSTPR  0x00088038  /* IWDT Count Stop Control Register */

/*****************************************************************************/
/* Ethernet Controller (ETHERC)                                              */
/*****************************************************************************/

#define ETHERC0_ECMR        0x000C0100  /* ETHERC Mode Register */
#define ETHERC0_ECSR        0x000C0110  /* ETHERC Status Register */
#define ETHERC0_ECSIPR      0x000C0118  /* ETHERC Interrupt Permission Register */
#define ETHERC0_RFLR        0x000C0108  /* Receive Frame Length Register */
#define ETHERC0_PIR         0x000C0120  /* PHY Interface Register */
#define ETHERC0_MAHR        0x000C01C0  /* MAC Address High Register */
#define ETHERC0_MALR        0x000C01C8  /* MAC Address Low Register */
#define ETHERC0_PSR         0x000C0128  /* PHY Status Register */
#define ETHERC0_TROCR       0x000C01D0  /* Transmit Retry Over Counter Register */
#define ETHERC0_CDCR        0x000C01D4  /* Delayed Collision Detect Counter Register */
#define ETHERC0_LCCR        0x000C01D8  /* Lost Carrier Counter Register */
#define ETHERC0_CNDCR       0x000C01DC  /* Carrier Not Detect Counter Register */
#define ETHERC0_CEFCR       0x000C01E4  /* CRC Error Frame Receive Counter Register */
#define ETHERC0_FRECR       0x000C01E8  /* Frame Receive Error Counter Register */
#define ETHERC0_TSFRCR      0x000C01EC  /* Too-Short Frame Receive Counter Register */
#define ETHERC0_TLFRCR      0x000C01F0  /* Too-Long Frame Receive Counter Register */
#define ETHERC0_RFCR        0x000C01F4  /* Residual-Bit Frame Receive Counter Register */
#define ETHERC0_MAFCR       0x000C01F8  /* Multicast Address Frame Receive Counter Register */
#define ETHERC0_IPGR        0x000C0150  /* IPG Register */
#define ETHERC0_APR         0x000C0154  /* Automatic PAUSE Frame Register */
#define ETHERC0_MPR         0x000C0158  /* Manual PAUSE Frame Register */
#define ETHERC0_TPAUSER     0x000C0164  /* Retransmit Count Register */
#define ETHERC0_RDMLR       0x000C0140  /* Random Number Generation Counter Upper Limit Setting Register */
#define ETHERC0_RFCF        0x000C0160  /* PAUSE Frame Receive Counter Register */
#define ETHERC0_TPAUSECR    0x000C0168  /* PAUSE Frame Retransmit Counter Register */
#define ETHERC0_BCFRR       0x000C016C  /* Broadcast Frame Receive Count Setting Register */

#define ETHERC1_ECMR        0x000C0300
#define ETHERC1_ECSR        0x000C0310
#define ETHERC1_ECSIPR      0x000C0318
#define ETHERC1_RFLR        0x000C0308
#define ETHERC1_PIR         0x000C0320
#define ETHERC1_MAHR        0x000C03C0
#define ETHERC1_MALR        0x000C03C8
#define ETHERC1_PSR         0x000C0328
#define ETHERC1_TROCR       0x000C03D0
#define ETHERC1_CDCR        0x000C03D4
#define ETHERC1_LCCR        0x000C03D8
#define ETHERC1_CNDCR       0x000C03DC
#define ETHERC1_CEFCR       0x000C03E4
#define ETHERC1_FRECR       0x000C03E8
#define ETHERC1_TSFRCR      0x000C03EC
#define ETHERC1_TLFRCR      0x000C03F0
#define ETHERC1_RFCR        0x000C03F4
#define ETHERC1_MAFCR       0x000C03F8
#define ETHERC1_IPGR        0x000C0350
#define ETHERC1_APR         0x000C0354
#define ETHERC1_MPR         0x000C0358
#define ETHERC1_TPAUSER     0x000C0364
#define ETHERC1_RDMLR       0x000C0340
#define ETHERC1_RFCF        0x000C0360
#define ETHERC1_TPAUSECR    0x000C0368
#define ETHERC1_BCFRR       0x000C036C

/*****************************************************************************/
/* Ethernet Controller Direct Memory Access Controller (EDMAC)               */
/*****************************************************************************/

#define EDMAC0_EDMR     0x000C0000  /* EDMAC Mode Register */
#define EDMAC0_EDTRR    0x000C0008  /* EDMAC Transmit Request Register */
#define EDMAC0_EDRRR    0x000C0010  /* EDMAC Receive Request Register */
#define EDMAC0_TDLAR    0x000C0018  /* Transmit Descriptor List Start Address Register */
#define EDMAC0_RDLAR    0x000C0020  /* Receive Descriptor List Start Address Register */
#define EDMAC0_EESR     0x000C0028  /* ETHERC/EDMAC Status Register */
#define EDMAC0_EESIPR   0x000C0030  /* ETHERC/EDMAC Status Interrupt Permission Register */
#define EDMAC0_TRSCER   0x000C0038  /* Transmit/Receive Status Copy Enable Register */
#define EDMAC0_RMFCR    0x000C0040  /* Receive Missed-Frame Counter Register */
#define EDMAC0_TFTR     0x000C0048  /* Transmit FIFO Threshold Register */
#define EDMAC0_FDR      0x000C0050  /* FIFO Depth Register */
#define EDMAC0_RMCR     0x000C0058  /* Receiving Method Control Register */
#define EDMAC0_TFUCR    0x000C0064  /* Transmit FIFO Underrun Counter */
#define EDMAC0_RFOCR    0x000C0068  /* Receive FIFO Overflow Counter */
#define EDMAC0_RBWAR    0x000C00C8  /* Receive Buffer Write Address Register */
#define EDMAC0_RDFAR    0x000C00CC  /* Receive Descriptor Fetch Address Register */
#define EDMAC0_TBRAR    0x000C00D4  /* Transmit Buffer Read Address Register */
#define EDMAC0_TDFAR    0x000C00D8  /* Transmit Descriptor Fetch Address Register */
#define EDMAC0_FCFTR    0x000C0070  /* Flow Control Start FIFO Threshold Setting Register */
#define EDMAC0_RPADIR   0x000C0078  /* Receive Data Padding Insert Register */
#define EDMAC0_TRIMD    0x000C007C  /* Transmit Interrupt Setting Register */
#define EDMAC0_IOSR     0x000C006C  /* Independent Output Signal Setting Register */

#define EDMAC1_EDMR     0x000C0200
#define EDMAC1_EDTRR    0x000C0208
#define EDMAC1_EDRRR    0x000C0210
#define EDMAC1_TDLAR    0x000C0218
#define EDMAC1_RDLAR    0x000C0220
#define EDMAC1_EESR     0x000C0228
#define EDMAC1_EESIPR   0x000C0230
#define EDMAC1_TRSCER   0x000C0238
#define EDMAC1_RMFCR    0x000C0240
#define EDMAC1_TFTR     0x000C0248
#define EDMAC1_FDR      0x000C0250
#define EDMAC1_RMCR     0x000C0258
#define EDMAC1_TFUCR    0x000C0264
#define EDMAC1_RFOCR    0x000C0268
#define EDMAC1_RBWAR    0x000C02C8
#define EDMAC1_RDFAR    0x000C02CC
#define EDMAC1_TBRAR    0x000C02D4
#define EDMAC1_TDFAR    0x000C02D8
#define EDMAC1_FCFTR    0x000C0270
#define EDMAC1_RPADIR   0x000C0278
#define EDMAC1_TRIMD    0x000C027C
#define EDMAC1_IOSR     0x000C026C

/*****************************************************************************/
/* Serial Communications Interface (SCIg, SCIh)                              */
/*****************************************************************************/

#define SCI0_SMR        0x0008A000  /* Serial Mode Register */
#define SCI0_BRR        0x0008A001  /* Bit Rate Register */
#define SCI0_SCR        0x0008A002  /* Serial Control Register */
#define SCI0_TDR        0x0008A003  /* Transmit Data Register */
#define SCI0_SSR        0x0008A004  /* Serial Status Register */
#define SCI0_RDR        0x0008A005  /* Receive Data Register */
#define SCI0_SCMR       0x0008A006  /* Smart Card Mode Register */
#define SCI0_SEMR       0x0008A007  /* Serial Extended Mode Register */
#define SCI0_SPMR       0x0008A00D  /* SPI Mode Register */

#define SCI1_SMR        0x0008A020
#define SCI1_BRR        0x0008A021
#define SCI1_SCR        0x0008A022
#define SCI1_TDR        0x0008A023
#define SCI1_SSR        0x0008A024
#define SCI1_RDR        0x0008A025
#define SCI1_SCMR       0x0008A026
#define SCI1_SEMR       0x0008A027
#define SCI1_SPMR       0x0008A02D

#define SCI2_SMR        0x0008A040
#define SCI2_BRR        0x0008A041
#define SCI2_SCR        0x0008A042
#define SCI2_TDR        0x0008A043
#define SCI2_SSR        0x0008A044
#define SCI2_RDR        0x0008A045
#define SCI2_SCMR       0x0008A046
#define SCI2_SEMR       0x0008A047
#define SCI2_SPMR       0x0008A04D

#define SCI3_SMR        0x0008A060
#define SCI3_BRR        0x0008A061
#define SCI3_SCR        0x0008A062
#define SCI3_TDR        0x0008A063
#define SCI3_SSR        0x0008A064
#define SCI3_RDR        0x0008A065
#define SCI3_SCMR       0x0008A066
#define SCI3_SEMR       0x0008A067
#define SCI3_SPMR       0x0008A06D

#define SCI4_SMR        0x0008A080
#define SCI4_BRR        0x0008A081
#define SCI4_SCR        0x0008A082
#define SCI4_TDR        0x0008A083
#define SCI4_SSR        0x0008A084
#define SCI4_RDR        0x0008A085
#define SCI4_SCMR       0x0008A086
#define SCI4_SEMR       0x0008A087
#define SCI4_SPMR       0x0008A08D

#define SCI5_SMR        0x0008A0A0
#define SCI5_BRR        0x0008A0A1
#define SCI5_SCR        0x0008A0A2
#define SCI5_TDR        0x0008A0A3
#define SCI5_SSR        0x0008A0A4
#define SCI5_RDR        0x0008A0A5
#define SCI5_SCMR       0x0008A0A6
#define SCI5_SEMR       0x0008A0A7
#define SCI5_SPMR       0x0008A0AD

#define SCI6_SMR        0x0008A0C0
#define SCI6_BRR        0x0008A0C1
#define SCI6_SCR        0x0008A0C2
#define SCI6_TDR        0x0008A0C3
#define SCI6_SSR        0x0008A0C4
#define SCI6_RDR        0x0008A0C5
#define SCI6_SCMR       0x0008A0C6
#define SCI6_SEMR       0x0008A0C7
#define SCI6_SPMR       0x0008A0CD

#define SCI7_SMR        0x000D00E0
#define SCI7_BRR        0x000D00E1
#define SCI7_SCR        0x000D00E2
#define SCI7_TDR        0x000D00E3
#define SCI7_SSR        0x000D00E4
#define SCI7_RDR        0x000D00E5
#define SCI7_SCMR       0x000D00E6
#define SCI7_SEMR       0x000D00E7
#define SCI7_SPMR       0x0008A0ED

#define SCI8_SMR        0x000D0000
#define SCI8_BRR        0x000D0001
#define SCI8_SCR        0x000D0002
#define SCI8_TDR        0x000D0003
#define SCI8_SSR        0x000D0004
#define SCI8_RDR        0x000D0005
#define SCI8_SCMR       0x000D0006
#define SCI8_SEMR       0x000D0007
#define SCI8_SPMR       0x000D000D

#define SCI9_SMR        0x000D0020
#define SCI9_BRR        0x000D0021
#define SCI9_SCR        0x000D0022
#define SCI9_TDR        0x000D0023
#define SCI9_SSR        0x000D0024
#define SCI9_RDR        0x000D0025
#define SCI9_SCMR       0x000D0026
#define SCI9_SEMR       0x000D0027
#define SCI9_SPMR       0x000D002D

#define SCI10_SMR       0x000D0040
#define SCI10_BRR       0x000D0041
#define SCI10_SCR       0x000D0042
#define SCI10_TDR       0x000D0043
#define SCI10_SSR       0x000D0044
#define SCI10_RDR       0x000D0045
#define SCI10_SCMR      0x000D0046
#define SCI10_SEMR      0x000D0047
#define SCI10_SPMR      0x000D004D

#define SCI11_SMR       0x000D0060
#define SCI11_BRR       0x000D0061
#define SCI11_SCR       0x000D0062
#define SCI11_TDR       0x000D0063
#define SCI11_SSR       0x000D0064
#define SCI11_RDR       0x000D0065
#define SCI11_SCMR      0x000D0066
#define SCI11_SEMR      0x000D0067
#define SCI11_SPMR      0x000D006D

#define SCI12_SMR       0x0008B300
#define SCI12_BRR       0x0008B301
#define SCI12_SCR       0x0008B302
#define SCI12_TDR       0x0008B303
#define SCI12_SSR       0x0008B304
#define SCI12_RDR       0x0008B305
#define SCI12_SCMR      0x0008B306
#define SCI12_SEMR      0x0008B307
#define SCI12_SPMR      0x0008B30D

/*****************************************************************************/
/* Interrupt Vector Numbers (Software Configurable Interrupts not Included)  */
/*****************************************************************************/

#define INT_BUSERR      16
#define INT_GROUPIE0    17
#define INT_RAMERR      18
#define INT_FIFERR      21
#define INT_FRDYI       23
#define INT_SWINT2      26
#define INT_SWINT       27
#define INT_CMI0        28
#define INT_CMI1        29
#define INT_CMWI0       30
#define INT_CMWI1       31
#define INT_D0FIFO0     34
#define INT_D1FIFO0     35
#define INT_SPRI0       38
#define INT_SPTI0       39
#define INT_SPRI1       40
#define INT_SPTI1       41
#define INT_SPRI        42
#define INT_SPTI        43
#define INT_SBFAI       44
#define INT_MBFAI       45
#define INT_SSITXI0     46
#define INT_SSIRXI0     47
#define INT_SSIRTI1     48
#define INT_RIIC_RXI1   50
#define INT_RIIC_TXI1   51
#define INT_RIIC_RXI0   52
#define INT_RIIC_TXI0   53
#define INT_RIIC_RXI2   54
#define INT_RIIC_TXI2   55
#define INT_SCI_RXI0    58
#define INT_SCI_TXI0    59
#define INT_SCI_RXI1    60
#define INT_SCI_TXI1    61
#define INT_SCI_RXI2    62
#define INT_SCI_TXI2    63
#define INT_IRQ0        64
#define INT_IRQ1        65
#define INT_IRQ2        66
#define INT_IRQ3        67
#define INT_IRQ4        68
#define INT_IRQ5        69
#define INT_IRQ6        70
#define INT_IRQ7        71
#define INT_IRQ8        72
#define INT_IRQ9        73
#define INT_IRQ10       74
#define INT_IRQ11       75
#define INT_IRQ12       76
#define INT_IRQ13       77
#define INT_IRQ14       78
#define INT_IRQ15       79
#define INT_SCI_RXI3    80
#define INT_SCI_TXI3    81
#define INT_SCI_RXI4    82
#define INT_SCI_TXI4    83
#define INT_SCI_RXI5    84
#define INT_SCI_TXI5    85
#define INT_SCI_RXI6    86
#define INT_SCI_TXI6    87
#define INT_LVD1        88
#define INT_LVD2        89
#define INT_USBR0       90
#define INT_ALM         92
#define INT_PRD         93
#define INT_IWUNI       95
#define INT_WUNI        96
#define INT_PCDFI       97
#define INT_SCI_RXI7    98
#define INT_SCI_TXI7    99
#define INT_SCI_RXI8    100
#define INT_SCI_TXI8    101
#define INT_SCI_RXI9    102
#define INT_SCI_TXI9    103
#define INT_SCI_RXI10   104
#define INT_SCI_TXI10   105
#define INT_GRPBE0      106
#define INT_GRPBL2      107
#define INT_SPRI2       108
#define INT_SPTI2       109
#define INT_GRPBL0      110
#define INT_GRPBL1      111
#define INT_GRPAL0      112
#define INT_GRPAL1      113
#define INT_SCI_RXI11   114
#define INT_SCI_TXI11   115
#define INT_SCI_RXI12   116
#define INT_SCI_TXI12   117
#define INT_DMAC0I      120
#define INT_DMAC1I      121
#define INT_DMAC2I      122
#define INT_DMAC3I      123
#define INT_DMAC74I     124
#define INT_OSTDI       125
#define INT_EXDMAC0I    126
#define INT_EXDMAC1I    127
#define INT_INTB128     128
#define INT_INTB129     129
#define INT_INTB130     130
#define INT_INTB131     131
#define INT_INTB132     132
#define INT_INTB133     133
#define INT_INTB134     134
#define INT_INTB135     135
#define INT_INTB136     136
#define INT_INTB137     137
#define INT_INTB138     138
#define INT_INTB139     139
#define INT_INTB140     140
#define INT_INTB141     141
#define INT_INTB142     142
#define INT_INTB143     143
#define INT_INTB144     144
#define INT_INTB145     145
#define INT_INTB146     146
#define INT_INTB147     147
#define INT_INTB148     148
#define INT_INTB149     149
#define INT_INTB150     150
#define INT_INTB151     151
#define INT_INTB152     152
#define INT_INTB153     153
#define INT_INTB154     154
#define INT_INTB155     155
#define INT_INTB156     156
#define INT_INTB157     157
#define INT_INTB158     158
#define INT_INTB159     159
#define INT_INTB160     160
#define INT_INTB161     161
#define INT_INTB162     162
#define INT_INTB163     163
#define INT_INTB164     164
#define INT_INTB165     165
#define INT_INTB166     166
#define INT_INTB167     167
#define INT_INTB168     168
#define INT_INTB169     169
#define INT_INTB170     170
#define INT_INTB171     171
#define INT_INTB172     172
#define INT_INTB173     173
#define INT_INTB174     174
#define INT_INTB175     175
#define INT_INTB176     176
#define INT_INTB177     177
#define INT_INTB178     178
#define INT_INTB179     179
#define INT_INTB180     180
#define INT_INTB181     181
#define INT_INTB182     182
#define INT_INTB183     183
#define INT_INTB184     184
#define INT_INTB185     185
#define INT_INTB186     186
#define INT_INTB187     187
#define INT_INTB188     188
#define INT_INTB189     189
#define INT_INTB190     190
#define INT_INTB191     191
#define INT_INTB192     192
#define INT_INTB193     193
#define INT_INTB194     194
#define INT_INTB195     195
#define INT_INTB196     196
#define INT_INTB197     197
#define INT_INTB198     198
#define INT_INTB199     199
#define INT_INTB200     200
#define INT_INTB201     201
#define INT_INTB202     202
#define INT_INTB203     203
#define INT_INTB204     204
#define INT_INTB205     205
#define INT_INTB206     206
#define INT_INTB207     207
#define INT_INTA208     208
#define INT_INTA209     209
#define INT_INTA210     210
#define INT_INTA211     211
#define INT_INTA212     212
#define INT_INTA213     213
#define INT_INTA214     214
#define INT_INTA215     215
#define INT_INTA216     216
#define INT_INTA217     217
#define INT_INTA218     218
#define INT_INTA219     219
#define INT_INTA220     220
#define INT_INTA221     221
#define INT_INTA222     222
#define INT_INTA223     223
#define INT_INTA224     224
#define INT_INTA225     225
#define INT_INTA226     226
#define INT_INTA227     227
#define INT_INTA228     228
#define INT_INTA229     229
#define INT_INTA230     230
#define INT_INTA231     231
#define INT_INTA232     232
#define INT_INTA233     233
#define INT_INTA234     234
#define INT_INTA235     235
#define INT_INTA236     236
#define INT_INTA237     237
#define INT_INTA238     238
#define INT_INTA239     239
#define INT_INTA240     240
#define INT_INTA241     241
#define INT_INTA242     242
#define INT_INTA243     243
#define INT_INTA244     244
#define INT_INTA245     245
#define INT_INTA246     246
#define INT_INTA247     247
#define INT_INTA248     248
#define INT_INTA249     249
#define INT_INTA250     250
#define INT_INTA251     251
#define INT_INTA252     252
#define INT_INTA253     253
#define INT_INTA254     254
#define INT_INTA255     255

/*****************************************************************************/
/* Exception Numbers (NORTi Unique)                                          */
/*****************************************************************************/

#define EXC_PRIV_INST   ((INHNO)-11)
#define EXC_ACCESS      ((INHNO)-10)
#define EXC_UNDEF_INST  ((INHNO)-8)
#define EXC_ADDRESS     ((INHNO)-7)
#define EXC_FLOAT_POINT ((INHNO)-6)
#define EXC_NMI         ((INHNO)-1)

#ifdef __cplusplus
}
#endif
#endif /* RX72M_H */
