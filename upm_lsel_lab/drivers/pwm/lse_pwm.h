//
//  lse_pwm.h
//  
//
//  Created by Pedro Malagon on 08/03/18.
//
//

#ifndef LSE_PWM_H
#define LSE_PWM_H

#define BCM2709_PERI_BASE	0x3F000000
#define GPIO_BASE		(BCM2709_PERI_BASE + 0x200000)
#define PWM_BASE		(BCM2709_PERI_BASE + 0x20C000)
#define CLOCK_BASE		(BCM2709_PERI_BASE + 0x101000)

#define MAIN_CLK        19200000
#define PWM_CLK         160000
#define PWM_MAX_FREQ    (PWM_CLK/10)
#define DIVISOR         (MAIN_CLK/PWM_CLK)

#define SET_GPIO_ALT(g,a) \
    gpio_reg->GPFSEL[g/10] = (gpio_reg->GPFSEL[g/10] & (~(0b111<<((g%10)*3)))) | (a << ((g%10)*3))

struct PWMRegisters
{
    uint32_t CTL;
    uint32_t STA;
    uint32_t DMAC;
    uint32_t Reserved;
    uint32_t RNG1;
    uint32_t DAT1;
    uint32_t FIF1;
    uint32_t Reserved2;
    uint32_t RNG2;
    uint32_t DAT2;
};

struct GpioRegisters
{
    uint32_t GPFSEL[6];
    uint32_t Reserved1;
    uint32_t GPSET[2];
    uint32_t Reserved2;
    uint32_t GPCLR[2];
    uint32_t Reserved3;
    uint32_t GPLEV[2];
};

struct ClkRegisters
{
    uint32_t GNRICCTL;
    uint32_t GNRICDIV;
    uint32_t VPUCTL;
    uint32_t VPUDIV;
    uint32_t SYSCTL;
    uint32_t SYSDIV;
    uint32_t PERIACTL;
    uint32_t PERIADIV;
    uint32_t PERIICTL;
    uint32_t PERIIDIV;
    uint32_t H264CTL;
    uint32_t H264DIV;
    uint32_t ISPCTL;
    uint32_t ISPDIV;
    uint32_t V3DCTL;
    uint32_t V3DDIV;
    uint32_t CAM0CTL;
    uint32_t CAM0DIV;
    uint32_t CAM1CTL;
    uint32_t CAM1DIV;
    uint32_t CCP2CTL;
    uint32_t CCP2DIV;
    uint32_t DSI0ECTL;
    uint32_t DSI0EDIV;
    uint32_t DSI0PCTL;
    uint32_t DSI0PDIV;
    uint32_t DPICTL;
    uint32_t DPIDIV;
    uint32_t GP0CTL;
    uint32_t GP0DIV;
    uint32_t GP1CTL;
    uint32_t GP1DIV;
    uint32_t GP2CTL;
    uint32_t GP2DIV;
    uint32_t HSMCTL;
    uint32_t HSMDIV;
    uint32_t OTPCTL;
    uint32_t OTPDIV;
    uint32_t PCMCTL;
    uint32_t PCMDIV;
    uint32_t PWMCTL;
    uint32_t PWMDIV;
    uint32_t SLIMCTL;
    uint32_t SLIMDIV;
    uint32_t SMICTL;
    uint32_t SMIDIV;
    uint32_t TCNTCTL;
    uint32_t TCNTCNT;
    uint32_t TECCTL;
    uint32_t TECDIV;
    uint32_t TD0CTL;
    uint32_t TD0DIV;
    uint32_t TD1CTL;
    uint32_t TD1DIV;
    uint32_t TSENSCTL;
    uint32_t TSENSDIV;
    uint32_t TIMERCTL;
    uint32_t TIMERDIV;
    uint32_t UARTCTL;
    uint32_t UARTDIV;
    uint32_t VECCTL;
    uint32_t VECDIV;
    uint32_t OSCCOUNT;
    uint32_t PLLA;
    uint32_t PLLC;
    uint32_t PLLD;
    uint32_t PLLH;
    uint32_t LOCK;
    uint32_t EVENT;
    uint32_t INTEN;
    uint32_t DSI0HSCK;
    uint32_t CKSM;
    uint32_t OSCFREQI;
    uint32_t OSCFREQF;
    uint32_t PLLTCTL;
    uint32_t PLLTCNT0;
    uint32_t PLLTCNT1;
    uint32_t PLLTCNT2;
    uint32_t PLLTCNT3;
    uint32_t TDCLKEN;
    uint32_t BURSTCTL;
    uint32_t BURSTCNT;
    uint32_t DSI1ECTL;
    uint32_t DSI1EDIV;
    uint32_t DSI1PCTL;
    uint32_t DSI1PDIV;
    uint32_t DFTCTL;
    uint32_t DFTDIV;
    uint32_t PLLB;
    uint32_t PULSECTL;
    uint32_t PULSEDIV;
    uint32_t SDCCTL;
    uint32_t SDCDIV;
    uint32_t ARMCTL;
    uint32_t ARMDIV;
    uint32_t AVEOCTL;
    uint32_t AVEODIV;
    uint32_t EMMCCTL;
    uint32_t EMMCDIV;
};


#endif /* LSE_PWM_H */
