#ifndef CMD_ENGINE_H_
#define CMD_ENGINE_H_

#ifdef __cplusplus
extern "C"{
#endif

#define CMD_TABLES_MAX  8

// 命令函数指针：无返回值（你说得对，完全不需要）
typedef void (*cmd_func_t)(const unsigned char data_A, unsigned char data_B);

// 命令项（const 只读）
typedef struct {
    unsigned char cmd_code;        // 命令码 0x01~0xFF
    const char *cmd_name;          // 命令名（调试用）
    cmd_func_t cmd_func;           // 执行函数
} cmd_item;

// 命令表
typedef struct {
    unsigned char table_sequence;
    const cmd_item *table_buff;     // 命令数组（const）的地址
    unsigned char item_count;     // 命令数量
} cmd_table;item_buff

// 绑定命令表
cmd_table *CMD_Table_Bind(const cmd_item *item_list, unsigned char item_count);

// 查找并执行命令
unsigned char CMD_engine_RUN(cmd_table *table, unsigned char cmd_code,
                             const unsigned char data_A, unsigned char data_B);

#ifdef __cplusplus
}
#endif

#endif
