#include "key.h"

int h_arr[4] = {KEY_H1_PIN, KEY_H2_PIN, KEY_H3_PIN, KEY_H4_PIN};
int v_arr[4] = {KEY_V1_PIN, KEY_V2_PIN, KEY_V3_PIN, KEY_V4_PIN};

int getKeyValue(void)
{
    int i, j = 0;
    int key_value = 0;
    for (i = 0; i < 4; i++)
    {
        delay_cycles(10);

        DL_GPIO_setPins(KEY_PORT, h_arr[i]);
        DL_GPIO_clearPins(KEY_PORT, h_arr[(i + 1) % 4]);
        DL_GPIO_clearPins(KEY_PORT, h_arr[(i + 2) % 4]);
        DL_GPIO_clearPins(KEY_PORT, h_arr[(i + 3) % 4]);

        delay_cycles(20000);

        for (j = 0; j < 4; j++)
        {
            if (DL_GPIO_readPins(KEY_PORT, v_arr[j]) != 0)
            {
                key_value = j * 4 + i + 1;
            }
            delay_cycles(10);
        }

        //delay_cycles(1000);
    }
		//delay_cycles(CPUCLK_FREQ/1000*200);
    return key_value; // 没有按下，返回0
}