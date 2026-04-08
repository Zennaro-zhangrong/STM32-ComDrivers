/**
 * @文件：SPI_cmd_table.c
 * @功能：创建SPI通信协议的命令表，所有命令表以此为模板
 * @作者：张荣
 */


 #include "cmd_engine.h"

 const cmd_item SPI_cmd_table [] =
 {
    {0x00,"左前轮占空比",cmd_LFwheel_DutyCycle},
 };



//创建实例化指令表
 cmd_table SPI_cmd_table = {0};

 /**
  * @功能：初始化指令表
  */
void SPI_CMD_TABLE_Init(void)
{
    &SPI_cmd_table = CMD_Table_Bind(SPI_cmd_table, 1);
}

/**
 * @功能：对应命令的处理函数
 * @参数：命令随后的第一个数据
 * @参数：命令随后的第二个数据
 * @返回：无返回
 */
void cmd_LFwheel_DutyCycle(unsigned char data_A, unsigned char data_B)
{
    //
}
