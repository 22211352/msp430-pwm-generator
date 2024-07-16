#include "ti_msp_dl_config.h"
#ifndef _CLKUPDATE_H_
#define _CLKUPDATE_H_

/*static const DL_TimerG_ClockConfig gPWM_ClockConfigUpdate1 = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_3,
    .prescale = 0U
};//1MHz-163Hz    10666667

static const DL_TimerG_ClockConfig gPWM_ClockConfigUpdate2 = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale = 18U 
};//168-26Hz  1684210

static const DL_TimerG_ClockConfig gPWM_ClockConfigUpdate3 = {
    .clockSel = DL_TIMER_CLOCK_MFCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_8,
    .prescale = 1U
};//25Hz-4Hz 250000

static const DL_TimerG_ClockConfig gPWM_ClockConfigUpdate4 = {
    .clockSel = DL_TIMER_CLOCK_LFCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale = 0U
};//3Hz-0.5Hz 32768
*/

static const DL_TimerG_ClockConfig gPWM_ClockConfigUpdate1 = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale = 0U
};//32000000 1.6k-488.28   500

static const DL_TimerG_ClockConfig gPWM_ClockConfigUpdate2 = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_3,
    .prescale = 0U
};//10666667 533-162     500-180

static const DL_TimerG_ClockConfig gPWM_ClockConfigUpdate3 = {
    .clockSel = DL_TIMER_CLOCK_MFCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale = 0U
};//4000000 200-61.04  180-65

static const DL_TimerG_ClockConfig gPWM_ClockConfigUpdate4 = {
    .clockSel = DL_TIMER_CLOCK_MFCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_3,
    .prescale = 0U
};//1333333 66-21   65-23

static const DL_TimerG_ClockConfig gPWM_ClockConfigUpdate5 = {
    .clockSel = DL_TIMER_CLOCK_MFCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_8,
    .prescale = 0U
};//500000 25-8   23-10

static const DL_TimerG_ClockConfig gPWM_ClockConfigUpdate6 = {
    .clockSel = DL_TIMER_CLOCK_MFCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_8,
    .prescale = 1U
};//250000 12-4  10-4

static const DL_TimerG_ClockConfig gPWM_ClockConfigUpdate7 = {
    .clockSel = DL_TIMER_CLOCK_MFCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_8,
    .prescale = 7U
}; //62500 3-1



#endif