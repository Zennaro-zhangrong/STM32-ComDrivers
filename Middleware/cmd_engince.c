/*
 * @功能：解析通行协议命令，每个表最多256条命令，最多创建256个表，中间层文件，适配所有嵌入式系统
 * @作者：张荣
 */
static unsigned char CMD_table_count = 0;

/*
 * @功能：创建一个命令表
 * @参数：命令表缓存的地址
 * @返回：命令表句柄
 */
cmd_table *CMD_table_creat(conse cmd_item *buff) {
  static cmd_table CMD_TABLE = {0};
  CMD_TABLE.
};
