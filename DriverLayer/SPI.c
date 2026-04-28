/*
 * SPI.c
 *
 *	此文件的SPI配置为中断全双工从模式，CS引脚为双边沿触发外部中断模式
 *
 *	此文件涵盖36字节的【单缓冲发送】和【双缓冲接收】功能，与Hardware.h中的结构体高度耦合
 *
 *	接收结构为 0xCMD,0xLEN,0xDATA[2:35]，LEN表示1帧的总字节数，范围为3到36
 *
 *  Created on: 2026年3月30日
 *      Author: zr186
 */


#include "Hardware.h"

extern SPI_HandleTypeDef hspi1;

SPI_Data_Buffer SPI_Recive_Buffer_1 = {0};
SPI_Data_Buffer SPI_Recive_Buffer_2 = {0};
SPI_Data_Buffer SPI_Transmit_Buffer = {0};

typedef enum{
	BUFFER_EMPTY,//缓冲区空，可用
	BUFFER_FULL,//当前缓冲区等待处理
	BUFFER_READ,//当前缓冲区正在被读取
	BUFFER_WRITE,//当前缓冲区正在被写入
	BUFFER_END
}SPI_BUFFER_STATUS;

/*
 * 功能：调用HAL库完成发送缓冲区的内容，此内容应当提前填充好
 * 输入：无
 * 输出：发送完成返回HAL_OK,发送失败返回HAL_ERROR
 */
static HAL_StatusTypeDef SPI_Transmit_Data(void){
	if(SPI_Transmit_Buffer.Data_Len == 0) return HAL_ERROR;//原因是发送无效数据
	HAL_StatusTypeDef state = HAL_SPI_Transmit_IT(&hspi1,
								SPI_Transmit_Buffer.Data_Buffer,
								SPI_Transmit_Buffer.Data_Len);
	if(state != HAL_OK){
		UART_Printf("SPI send data error,the data content is:\r\n");
		for(int i = 0; i < 36; i++){
			UART_Printf("%02X ",SPI_Transmit_Buffer.Data_Buffer[i]);
		}
		UART_Printf("\r\n");
	}
	return state;
}



/*
 * 功能：判断最新接收的一个字节应该写入哪个接收缓冲区的哪个位置，
 * 		接收结构为 0xCMD,0xLEN,0xDATA[2:35]。
 * 输入：无
 * 返回：可以写入的物理内存地址
 */
static uint8_t* SPI_GetNextReceiveAddr(void){
	//判断写入缓冲区
	SPI_Data_Buffer * Buffer;
	if(SPI_Recive_Buffer_1.Buffer_Status == BUFFER_WRITE){
		Buffer = &SPI_Recive_Buffer_1;
	}else if(SPI_Recive_Buffer_2.Buffer_Status == BUFFER_WRITE){
		Buffer = &SPI_Recive_Buffer_2;
	}else if(SPI_Recive_Buffer_1.Buffer_Status == BUFFER_EMPTY){
		Buffer = &SPI_Recive_Buffer_1;
	}else if(SPI_Recive_Buffer_2.Buffer_Status == BUFFER_EMPTY){
		Buffer = &SPI_Recive_Buffer_2;
	}else return NULL;

	//判断写入地址
	uint8_t Cursor = Buffer->Buffer_Cursor;
	if(Cursor >= 36) return NULL;
	Buffer->Buffer_Cursor++;

	//判断帧尾
	if(Buffer->Data_Buffer[1] < 3 || Buffer->Data_Buffer[1] > 36){
		return NULL;
	}else if(Buffer->Buffer_Cursor == Buffer->Data_Buffer[1]){
		Buffer->Buffer_Status = BUFFER_FULL;
		Buffer->Buffer_Cursor = 0;
	}

	//发送地址
	return &(Buffer->Data_Buffer[Cursor]);
}




/*
 * @功能：SPI1的接收中断回调函数，务必放在HAL库的
 * HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)函数中
 *
 * @输入：SPI句柄
 *
 * @输出：无
 */
static void SPI1_RxCallback(SPI_HandleTypeDef *hspi)
{
    if(hspi == &hspi1){
    	uint8_t *nextAddr = SPI_GetNextReceiveAddr();
		if (nextAddr != NULL) {
			HAL_SPI_Receive_IT(&hspi1, nextAddr, 1);   // 启动下一个字节接收
		} else {
			UART_Printf("Receive error!Possible reasons:[1] Data too long, buffer overflow.[2] Incorrect format sent by the sender\r\n");
		}
    }
}



void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi){
	SPI1_RxCallback(hspi);
}
