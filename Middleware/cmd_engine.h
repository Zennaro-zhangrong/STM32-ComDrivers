#ifndef CMD_ENGINE_H_
#define CMD_ENGINE_H_

#ifdef __cplusplus
extern "C"{
#endif

typedef int (*cmd_func_t)(const unsigned char data, unsigned char len);

typedef struct{
  unsigned char cmd_item_sequence;
  const char cmd_item_name;
  cmd_func_t cmd_function;
}cmd_item;



typedef struct{
  unsigned char cmd_table_sequence;
  cmd_item * cmd_item_buff;
}cmd_table;



#ifdef __cplusplus
}
#endif

#endif
