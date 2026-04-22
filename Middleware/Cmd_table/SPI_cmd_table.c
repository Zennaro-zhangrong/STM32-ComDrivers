/**
 * @文件：SPI_cmd_table.c
 * @功能：创建SPI通信协议的命令表，所有命令表以此为模板
 * @作者：张荣
 */


 #include "cmd_engine.h"


//创建指令表指针，数据本体已经放在cdm_engine.c文件了，无需创建实例
 cmd_table *SPI_cmd_table;




/***************************************************************************
 *用户操作区
 * *************************************************************************/

 // SPI命令匿名枚举
enum {
    CMD_NONE, // 空命令/无效命令
    CMD_LFW_Duty,//左前轮车轮占空比设置
    CMD_COUNT
};


const cmd_item SPI_cmd_items [] =
 {
    {CMD_NONE,"空命令",SPICMD_Conectivity_Test},
    {CMD_LFW_Duty,"左前轮占空比",SPICMD_LFwheel_PwmDutycycleSet},
 };


/**
 * @功能：对应命令的处理函数
 * @参数：获取数据的地址
 * @参数：有效数据的长度
 * @返回：无返回
 */

void SPICMD_Conectivity_Test(unsigned char *Pbuff, unsigned char LengthLength)
{
    //
}
void SPICMD_LFwheel_PwmDutycycleSet(unsigned char *Pbuff, unsigned char LengthLength)
{
    //
}



/*********************************用户操作终止区******************************* */






 /**
  * @功能：初始化指令表,将指令表的各个部分绑定为一个整体
  */Count
void SPI_CMD_TABLE_Init(void)
{
    SPI_cmd_table = CMD_Table_Bind(SPI_cmd_items, CMD_COUNT);
}
