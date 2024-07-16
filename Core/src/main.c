/*
 * UART_PRINTF示例
 * 主函数使用printf（115200波特率）每秒打印一次数据
 * 数据可在电脑串口软件上观察到
 * Author: zjs
 */
#include "ti_msp_dl_config.h"
#include "oled.h"
#include "key.h"
#include "CLKUpdate.h"

/* 使用printf需要包含以下头文件 */
#include "stdio.h"
#include "string.h"

//===================对printf进行重定向===================
#pragma(__use_no_semihosting)
struct FILE
{
	int handle;
};
FILE __stdout;
void _sys_exit(int x)
{
	x = x;
}
int fputc(int ch, FILE *f)
{
	/* 发送一个数据 */
	DL_UART_Main_transmitData(UART_0_INST, (uint8_t)ch);
	/* 等待数据传输完毕 */
	while (DL_UART_Main_isBusy(UART_0_INST))
		;
	return ch;
}
int fputs(const char *_ptr, register FILE *_fp)
{
	uint16_t i, len;
	len = strlen(_ptr);
	for (i = 0; i < len; i++)
	{
		/* 发送一个数据 */
		DL_UART_Main_transmitData(UART_0_INST, (uint8_t)_ptr[i]);
		/* 等待数据传输完毕 */
		while (DL_UART_Main_isBusy(UART_0_INST))
			;
	}
	return len;
}
//========================================================

void KeyValueCalculate(void);// 这个函数用于进行键值计算
void OLED_Flash(uint32_t PWM1, float PWM1_Duty, uint32_t PWM2, float PWM2_Duty);
// 这个函数用于更新OLED显示，参数是两个PWM通道的频率和占空比。

void OLED_Enter(void);// 这个函数用于进入OLED显示模式
void UpdateFreqDuty(void);// 这个函数用于更新频率和占空比

uint8_t num = 0;

int key_value = 0;
uint8_t key_num[16] = {7, 4, 1, 10, 8, 5, 2, 0, 9, 6, 3, 11, 12, 13, 14, 15};
float key_float[16] = {7, 4, 1, 10, 8, 5, 2, 0, 9, 6, 3, 11, 12, 13, 14, 15};

uint32_t temp_num[7] = {0, 0, 0, 0, 0, 0, 0}; //[3]ch [4]duty[5]enable/disable [6]是否有小数
uint8_t enablechannel[2] = {0, 0};

float temp_float;
uint32_t PWM_Freq[2] = {1, 1};
float PWM_Duty[2] = {50.000, 50.000};
uint32_t PWM1_CLK = 32000000 / 2;
uint32_t PWM2_CLK = 32000000 / 2;

float period0 = 1000;
float dutyCycle0;
uint32_t captureCompareValue1 = 500;
uint32_t loadValue1 = 1000;

float period1 = 1000;
float dutyCycle1;
uint32_t captureCompareValue0 = 500;
uint32_t loadValue0 = 1000;

int main(void)
{
	SYSCFG_DL_init(); // Initialize the device

	/* 初始化OLED屏幕 */
	OLED_Init();
	OLED_Clear();
	OLED_Flash(PWM_Freq[0], PWM_Duty[0], PWM_Freq[1], PWM_Duty[1]);

	DL_TimerG_setClockConfig(PWM_1_INST, (DL_TimerG_ClockConfig *)&gPWM_ClockConfigUpdate1);
	DL_TimerG_startCounter(PWM_0_INST);
	DL_TimerG_startCounter(PWM_1_INST);

	DL_TimerG_startCounter(TIMER_0_INST);
	NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);

	while (1)
	{

		key_value = getKeyValue();

		KeyValueCalculate();

		num++;

		if (key_value != 0)
		{
			delay_ms(100);
			OLED_Enter();
		}

		UpdateFreqDuty();
	}
}

void KeyValueCalculate(void)
{
	if (key_value > 0 && key_value <= 16)
	{
		if (key_num[key_value - 1] >= 0 && key_num[key_value - 1] <= 9)
		{
			if (temp_num[6] == 0)
			{
				// 首先检查是否整数部分已经达到了最大可能的长度（8位数字），
        // 如果是，则停止处理并返回，避免溢出。
				if (temp_num[0] >= 10000000)
				{
					return;
				}
				// 如果整数部分未达到最大长度，将当前键值对应的数字添加到整数部分。
        // 这里使用乘以10然后加上的方式来构建整数。
				temp_num[0] = temp_num[0] * 10 + key_num[key_value - 1];
				temp_num[2]++;
				// 更新浮点数表示，以保持与整数部分的一致性。
				temp_float = temp_num[0];
			}
			// 如果整数部分已满，开始处理小数部分。
        // 初始化浮点数因子为1，这将用于计算小数点后的位数。
			else
			{
				float FloatFactor = 1;
				// 增加小数部分的位数计数。
				temp_num[2]++;			
				for (int x = 0; x < temp_num[2]; x++)
				{
					FloatFactor = FloatFactor * 10; // 计算小数点后需要移动的小数位数。
				}
         // 根据计算出的小数位数，将当前键值对应的浮点数添加到总和中。
				if (temp_num[2] >= 1)
				{
					temp_float = temp_float + key_float[key_value - 1] / FloatFactor;
				}
			}
		}

		else if (key_num[key_value - 1] == 15)
		{
			temp_num[1] = temp_num[0];
			if (temp_num[6] == 0)
			{
				if (temp_num[3] != 0 && temp_num[0] != 0)
				{
					if (temp_num[4] == 2)
					{
						PWM_Duty[temp_num[3] - 1] = temp_num[1];
					}
					else
					{
						PWM_Freq[temp_num[3] - 1] = temp_num[1];
					}
				}
				else if (temp_num[3] == 0 && temp_num[0] != 0)
				{
					if (temp_num[4] == 2)
					{
						PWM_Duty[0] = temp_num[1];
					}
					else
					{
						PWM_Freq[0] = temp_num[1];
					}
				}
				// OLED_Flash(PWM_Freq[0], PWM_Duty[0], PWM_Freq[1], PWM_Duty[1]);
			}
			else
			{
				if (temp_num[3] != 0 && temp_num[0] != 0)
				{
					if (temp_num[4] == 2)
					{
						PWM_Duty[temp_num[3] - 1] = temp_float;
					}
					else
					{
						PWM_Freq[temp_num[3] - 1] = temp_float;
					}
				}
				else if (temp_num[3] == 0 && temp_num[0] != 0)
				{
					if (temp_num[4] == 2)
					{
						PWM_Duty[0] = temp_float;
					}
					else
					{
						PWM_Freq[0] = temp_float;
					}
				}
			}
			temp_num[0] = 0;
			temp_num[1] = 0;
			temp_num[2] = 0;
			temp_num[3] = 0;
			temp_num[4] = 0;
			temp_num[6] = 0;
			OLED_Flash(PWM_Freq[0], PWM_Duty[0], PWM_Freq[1], PWM_Duty[1]);

		} // confirm

		else if (key_num[key_value - 1] == 10)
		{
			temp_num[0] = temp_num[0] / 10;
			if (temp_num[2] != 0)
			{
				temp_num[2]--;
				OLED_Flash(PWM_Freq[0], PWM_Duty[0], PWM_Freq[1], PWM_Duty[1]);
				OLED_ShowNum(2, 3, (unsigned int)temp_num[0], temp_num[2], 16);
			}
		}

		else if (key_num[key_value - 1] == 12)
		{
			if (temp_num[3] == 1)
			{
				temp_num[3] = 2;
			}
			else
			{
				temp_num[3] = 1;
			} // temp_num[3] 1==CH1 2==CH2
			OLED_Flash(PWM_Freq[0], PWM_Duty[0], PWM_Freq[1], PWM_Duty[1]);
		}
		else if (key_num[key_value - 1] == 13)
		{
			if (temp_num[4] == 1)
			{
				temp_num[4] = 2;
			}
			else
			{
				temp_num[4] = 1;
			} // temp_num[4] 1==Freq 2==Duty
			OLED_Flash(PWM_Freq[0], PWM_Duty[0], PWM_Freq[1], PWM_Duty[1]);
		}
		else if (key_num[key_value - 1] == 14)
		{
			if (temp_num[3] == 1)
			{
				if (enablechannel[0] == 0)
				{
					enablechannel[0] = 1;
				}
				else
				{
					enablechannel[0] = 0;
				}
			}

			else if (temp_num[3] == 2)
			{
				if (enablechannel[1] == 0)
				{
					enablechannel[1] = 1;
				}
				else
				{
					enablechannel[1] = 0;
				}
			}

			OLED_Clear();
			OLED_Flash(PWM_Freq[0], PWM_Duty[0], PWM_Freq[1], PWM_Duty[1]);
		}
		else
		{
			// temp_num[0] = 0;
			temp_num[6] = 1;
			temp_num[2] = 0;

			OLED_Flash(PWM_Freq[0], PWM_Duty[0], PWM_Freq[1], PWM_Duty[1]);
		}
		/*printf("key_num[key_value - 1]=%d\n", key_num[key_value - 1]);
		printf("temp_num[0]=%d\n", temp_num[0]);
		printf("temp_num[1]=%d\n", temp_num[1]);
		printf("temp_num[2]=%d\n", temp_num[2]);
		printf("temp_num[3]=%d\n", temp_num[3]);
		printf("temp_num[4]=%d\n", temp_num[4]);
		printf("temp_num[5]=%d\n", temp_num[5]);
		printf("\nPWM_Freq[0]=%dHz\n", PWM_Freq[0]);
		printf("PWM_Duty[0]=%d%%\n", (unsigned int)PWM_Duty[0]);
		printf("PWM_Freq[1]=%dHz\n", PWM_Freq[1]);
		printf("PWM_Duty[1]=%d%%\n", (unsigned int)PWM_Duty[1]);*/
	}
}

void UpdateFreqDuty(void)
{

	if (PWM_Freq[0] > 500)
	{
		DL_TimerG_setClockConfig(PWM_0_INST, (DL_TimerG_ClockConfig *)&gPWM_ClockConfigUpdate1);
		PWM1_CLK = 32000000;
	}
	else if (PWM_Freq[0] > 180 && PWM_Freq[0] <= 500)
	{
		DL_TimerG_setClockConfig(PWM_0_INST, (DL_TimerG_ClockConfig *)&gPWM_ClockConfigUpdate2);
		PWM1_CLK = 10666667;
	}
	else if (PWM_Freq[0] > 65 && PWM_Freq[0] <= 180)
	{
		DL_TimerG_setClockConfig(PWM_0_INST, (DL_TimerG_ClockConfig *)&gPWM_ClockConfigUpdate3);
		PWM1_CLK = 4000000;
	}
	else if (PWM_Freq[0] > 32 && PWM_Freq[0] <= 65)
	{
		DL_TimerG_setClockConfig(PWM_0_INST, (DL_TimerG_ClockConfig *)&gPWM_ClockConfigUpdate4);
		PWM1_CLK = 1333333;
	}
	else if (PWM_Freq[0] > 10 && PWM_Freq[0] <= 32)
	{
		DL_TimerG_setClockConfig(PWM_0_INST, (DL_TimerG_ClockConfig *)&gPWM_ClockConfigUpdate5);
		PWM1_CLK = 500000;
	}
	else if (PWM_Freq[0] > 4 && PWM_Freq[0] <= 10)
	{
		DL_TimerG_setClockConfig(PWM_0_INST, (DL_TimerG_ClockConfig *)&gPWM_ClockConfigUpdate6);
		PWM1_CLK = 250000;
	}
	else //(PWM_Freq[0]<=3)
	{
		DL_TimerG_setClockConfig(PWM_0_INST, (DL_TimerG_ClockConfig *)&gPWM_ClockConfigUpdate7);
		PWM1_CLK = 62500;
	}

	// 计算周期和占空比，使用浮点运算以保持精度
	period0 = (float)PWM1_CLK / PWM_Freq[0];
	dutyCycle0 = period0 * (1 - PWM_Duty[0] / (float)100);

	// 将结果转换为整数类型，以便于传递给硬件接口
	if (enablechannel[0] == 0)
	{
		captureCompareValue0 = (uint32_t)period0;
		loadValue0 = (uint32_t)dutyCycle0;
		// printf("captureCompareValue=%d\n", (unsigned int)captureCompareValue0);
	}
	else
	{
		captureCompareValue0 = 1;
		loadValue0 = 2;
	}
	// 设置捕获比较值和载入值
	DL_TimerG_setLoadValue(PWM_0_INST, captureCompareValue0);
	DL_TimerG_setCaptureCompareValue(PWM_0_INST, loadValue0, DL_TIMERG_CAPTURE_COMPARE_0_INDEX);

	if (PWM_Freq[1] > 500)
	{
		DL_TimerG_setClockConfig(PWM_1_INST, (DL_TimerG_ClockConfig *)&gPWM_ClockConfigUpdate1);
		PWM2_CLK = 32000000;
	}
	else if (PWM_Freq[1] > 180 && PWM_Freq[1] <= 500)
	{
		DL_TimerG_setClockConfig(PWM_1_INST, (DL_TimerG_ClockConfig *)&gPWM_ClockConfigUpdate2);
		PWM2_CLK = 10666667;
	}
	else if (PWM_Freq[1] > 65 && PWM_Freq[1] <= 180)
	{
		DL_TimerG_setClockConfig(PWM_1_INST, (DL_TimerG_ClockConfig *)&gPWM_ClockConfigUpdate3);
		PWM2_CLK = 4000000;
	}
	else if (PWM_Freq[1] > 32 && PWM_Freq[1] <= 65)
	{
		DL_TimerG_setClockConfig(PWM_1_INST, (DL_TimerG_ClockConfig *)&gPWM_ClockConfigUpdate4);
		PWM2_CLK = 1333333;
	}
	else if (PWM_Freq[1] > 10 && PWM_Freq[1] <= 32)
	{
		DL_TimerG_setClockConfig(PWM_1_INST, (DL_TimerG_ClockConfig *)&gPWM_ClockConfigUpdate5);
		PWM2_CLK = 500000;
	}
	else if (PWM_Freq[1] > 4 && PWM_Freq[1] <= 10)
	{
		DL_TimerG_setClockConfig(PWM_1_INST, (DL_TimerG_ClockConfig *)&gPWM_ClockConfigUpdate6);
		PWM2_CLK = 250000;
	}
	else //(PWM_Freq[1]<=3)
	{
		DL_TimerG_setClockConfig(PWM_1_INST, (DL_TimerG_ClockConfig *)&gPWM_ClockConfigUpdate7);
		PWM2_CLK = 62500 ;
	}

	// 计算周期和占空比，使用浮点运算以保持精度
	
		period1 = (float)PWM2_CLK / PWM_Freq[1];
		dutyCycle1 = period1 * (1 - PWM_Duty[1] / (float)100);
	
	

	// 将结果转换为整数类型，以便于传递给硬件接口
	if (enablechannel[1] == 0)
	{
		captureCompareValue1 = (uint32_t)period1;
	loadValue1 = (uint32_t)dutyCycle1;
	}
	// printf("captureCompareValue1=%d\n", (unsigned int)captureCompareValue1);
else
	{
		captureCompareValue1 = 1;
		loadValue1 = 2;
	}
	// 设置捕获比较值和载入值
	DL_TimerG_setLoadValue(PWM_1_INST, captureCompareValue1);									 // 频率
	DL_TimerG_setCaptureCompareValue(PWM_1_INST, loadValue1, DL_TIMERG_CAPTURE_COMPARE_0_INDEX); // 占空比
}

void OLED_Enter(void)
{
	if (temp_num[4] == 1)
	{
		OLED_ShowString(2 + 2 * 8 + 4, 1, " Freq ");
	}
	else if (temp_num[4] == 2)
	{
		OLED_ShowString(2 + 2 * 8 + 4, 1, " Duty ");
	}
	if (temp_num[3] == 1)
	{
		OLED_ShowString(2, 1, "CH1");
	}
	else if (temp_num[3] == 2)
	{
		OLED_ShowString(2, 1, "CH2");
	}
	// OLED_ShowNum(2, 3, (unsigned int)temp_num[0], temp_num[2], 16);
	if (temp_num[0] != 0)
	{
		if (temp_num[6] == 0)
		{
			OLED_ShowNum(2, 3, (unsigned int)temp_num[0], 8, 16);
		}
		else
		{
			OLED_Showdecimal(2, 3, temp_float, 5, 3);
		}
	}
}

void OLED_Flash(uint32_t PWM1, float PWM1_Duty, uint32_t PWM2, float PWM2_Duty)
{
	OLED_Clear();
	OLED_ShowString8(0, -1, "CH1");
	if (enablechannel[0] == 1)
	{
		OLED_ShowString8(3 * 6 + 2, -1, "OFF");
	}
	else
	{
		OLED_ShowString8(3 * 6 + 2, -1, "ON");
	}
	OLED_ShowString8(2 * 6, 0, "Freq");
	OLED_ShowString8(2 * 6, 1, "Duty");

	OLED_ShowNum(7 * 6 + 2, 0, PWM1, 5, 16);
	OLED_ShowString8(7 * 6 + 2 + 5 * 8, 0, "Hz");
	OLED_Showdecimal(7 * 6 - 2, 2, PWM1_Duty, 2, 3);
	OLED_ShowString(7 * 6 + 5 * 8 + 2, 2, "%");
	if (PWM1 == 1000000)
	{
		OLED_ShowString(7 * 6 + 3 * 8 + 2, 0, "1M");
	}
	if (PWM1 == 100000)
	{
		OLED_ShowString(7 * 6 + 3 * 8 + 2, 0, "100k");
	}

	OLED_ShowString8(0, -1 + 4, "CH2");
	if (enablechannel[1] == 1)
	{
		OLED_ShowString8(3 * 6 + 2, -1 + 4, "OFF");
	}
	else
	{
		OLED_ShowString8(3 * 6 + 2, -1 + 4, "ON");
	}
	OLED_ShowString8(2 * 6, 0 + 4, "Freq");
	OLED_ShowString8(2 * 6, 1 + 4, "Duty");

	OLED_ShowNum(7 * 6 + 2, 0 + 4, PWM2, 5, 16);
	OLED_ShowString8(7 * 6 + 2 + 5 * 8, 0 + 4, "Hz");
	OLED_Showdecimal(7 * 6 - 2, 2 + 4, PWM2_Duty, 2, 3);
	OLED_ShowString(7 * 6 + 5 * 8 + 2, 2 + 4, "%");
	if (PWM2 == 1000000)
	{
		OLED_ShowString(7 * 6 + 3 * 8 + 2, 4, "1M");
	}
	if (PWM2 == 100000)
	{
		OLED_ShowString(7 * 6 + 1 * 8 + 2, 4, "100k");
	}

	OLED_ShowString8(12 * 8 + 6, -1, "MENU");
	OLED_ShowString8(13 * 8 - 2, 0, "CH/2");
	OLED_ShowString8(12 * 8 + 6, 1, "----");
	OLED_ShowString8(13 * 8, 2, "F/D");
	OLED_ShowString8(12 * 8 + 6, 3, "----");
	OLED_ShowString8(13 * 8 - 4, 4, "Enab");
	OLED_ShowString8(12 * 8 + 6, 5, "----");
	OLED_ShowString8(13 * 8 + 4, 6, "OK");

	if (temp_num[3] == 1)
	{

		OLED_ShowString8(13 * 8 - 2, 0, " CH1");
	}
	else if (temp_num[3] == 2)
	{

		OLED_ShowString8(13 * 8 - 2, 0, " CH2");
	}
	if (temp_num[4] == 1)
	{
		OLED_ShowString8(13 * 8 - 3, 2, "Freq");
	}
	else if (temp_num[4] == 2)
	{
		OLED_ShowString8(13 * 8 - 3, 2, "Duty");
	}
	/*if(temp_num[5]==0)
			{
						OLED_ShowString8(13*8-4,4,"Enab");
			}
	else if(temp_num[5]==1)
			{
					OLED_ShowString8(13*8-4,4,"Dise");
			}	*/
	/*
	if(temp_num[3]==0){
	OLED_ShowString(13*8,0,"CH1");}
	else{
		OLED_ShowString(13*8,0,"CH2");}
	if(temp_num[4]==0){
	OLED_ShowString(12*8,2,"Freq");}
	else{
		OLED_ShowString(12*8,2,"Duty");}
	*/
}

void TIMER_0_INST_IRQHandler(void)
{
	if(enablechannel[0]==0)
	{
		printf("\nCH1 Enable\n");
	}
	else
	{
		printf("\nCH1 Disable\n");
	}
	printf("CH1_Freq=%dHz\n", PWM_Freq[0]);
	printf("CH1_Duty[0]=%d%%\n", (unsigned int)PWM_Duty[0]);
	if(enablechannel[1]==0)
	{
		printf("CH2 Enable\n");
	}
	else
	{
		printf("CH2 Disable\n");
	}
	printf("CH2_Freq=%dHz\n", PWM_Freq[1]);
	printf("CH2_Duty=%d%%\n", (unsigned int)PWM_Duty[1]);
	// printf("%d or %d\n",enablechannel[0],enablechannel[1]);
	/*		printf("CH1\n");
		   printf("PWM1_CLK=%d\n", PWM1_CLK);

		printf("loadValue0=%d\n",loadValue0);
		//printf("loadValue0=%f\n",period0);
		//printf("captureCompareValue0=%f\n", dutyCycle1);
		printf("captureCompareValue0=%d\n", captureCompareValue0);
		printf("CH2\n");
		   printf("PWM2_CLK=%d\n", PWM2_CLK);

		printf("loadValue1=%d\n",loadValue1);
		//printf("loadValue1=%f\n",period1);
		//printf("captureCompareValue1=%f\n", dutyCycle1);
		printf("captureCompareValue1=%d\n", captureCompareValue1);
		*/
}
