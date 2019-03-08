

Server端解析
struct node *resolve(char *buf)


Client修改发送值

/*
int thread_count;
Main线程每生成一个发送线程，count+1,每个线程退出时count-1
*/
bool used[10];加信号量
Main创建线程时遍历used找空位，有空创建线程
线程退出修改used

共享区gps_signal sig[10]

标识flag[]，默认为0（加上信号量）
发送线程每一次生成随机数据之前检查flag，为1就不修改直接发送

主线程修改时置flag为1

Server端



Huffman
先计算各字符权重
构造huffman树
遍历一下得到hashtable
压缩字符串
hashtable、压缩内容一并发送
