/*
 * @功能：解析通行协议命令，每个表最多256条命令，最多创建256个表，中间层文件，适配所有嵌入式系统
 * @作者：张荣
 */


 #include "cmd_engine.h"
#include "../DriverLayer/Serial_port_log.h"


static cmd_table cmd_book[CMD_TABLES_MAX] = {0};//用于存放所有表
volatile static unsigned char cmd_table_count = 0;

/**
 * @功能：绑定一张const命令表
 * @参数：命令表的地址
 * @参数：命令表的总命令数量
 * @返回：命令表的句柄地址，命令表满的话返回NULL
 */
cmd_table *CMD_Table_Bind(const cmd_item *item_buff, unsigned char item_count)
{
    if(cmd_table_count >= CMD_TABLES_MAX) {
        log_printf("命令表已满\r\n");
        return NULL;
    }

    cmd_table *tab = &cmd_book[cmd_table_count];
    tab->table_sequence = cmd_table_count;
    tab->table_buff = item_buff;
    tab->item_count = item_count;

    cmd_table_count++;
    return tab;
}




/**
 * @功能：执行命令对应的处理函数
 * @参数：要执行的命令表句柄地址
 * @参数：要解析的指令（0~255）
 * @参数：函数指针的输入参数,必须是数据内容的地址（整帧命令去掉帧头，帧尾，指令代码，长度和校验和）
 * @参数：函数指针的输入参数，代表数据内容的长度
 * @返回：0：执行失败，1：执行成功
 */
unsigned char CMD_engine_RUN(cmd_table *table, unsigned char cmd_code,
                      const unsigned char* Pbuff, unsigned char Length)
{
    if(table == NULL || table->table_buff == NULL) {
      log_printf("%s[%d]:命令解析失败,地址未找到!\r\n",__FILE__,__LINE__);
      return 0;
    }
    // 遍历查表
    for(unsigned char i = 0; i < table->item_count; i++) {
        const cmd_item *item = &table->table_buff[i];

        if(item->cmd_code == cmd_code) {
            if(item->cmd_func != NULL) {
                item->cmd_func(Pbuff, Length);
            }
            return 1;
        }
    }
    log_printf("此表[%d]未找到命令: 0x%02X\r\n",table->table_sequence, cmd_code);
    return 0;
}





