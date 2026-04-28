#ifndef CMD_ENGINE_H_
#define CMD_ENGINE_H_

#ifdef __cplusplus
extern "C"{
#endif

/*********************************************************************
 * @用户配置
 *********************************************************************/
#define CMD_TABLES_MAX  8   //命令表的最大数量，一般有几个数据通道便设为几







/***************************************************************************
 *用户操作区结束
 * *************************************************************************/





// 命令函数指针
//Length代表有效数据的长度，不包括帧头帧尾校验位和载荷长度
//Pbuff代表有数据第一个元素的地址，有效数据必须按正确顺序放在一块数组中
typedef void (*cmd_func_t)(unsigned char Length, unsigned char *Pbuff);




// 命令项
typedef struct {
    unsigned char cmd_code;        // 命令码 0x01~0xFF
    cmd_func_t cmd_func;           // 执行函数
} cmd_item;




// 命令表
typedef struct {
    char table_sequence;//此表的序号
    cmd_item *table_buff;     // 命令数组（const）的地址
    unsigned char item_count;     // 命令数量
} cmd_table;





// 绑定命令表
cmd_table *CMD_Table_Bind(const cmd_item *table_buff, unsigned char item_count);



// 查找并执行命令
unsigned char CMD_engine_RUN(cmd_table *table, unsigned char cmd_code,
                            unsigned char data_length, unsigned char *data_Pbuffer);


                             
#ifdef __cplusplus
}
#endif

#endif
