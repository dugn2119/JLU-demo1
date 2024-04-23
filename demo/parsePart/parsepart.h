#include "../include/var.h"
#include "../include/include.h"


Node *parsell();                        // 主要函数
void createLL1Table();                  // 创建分析表
void predict(int pnum);                 // 调用函数
int priosity(Node *t);                  // 判断优先级
void wrong(int num, string str, int n); // 错误处理
int match(int now, string temp);        // 匹配
void predict(int pnum);                 // 预测函数
int priosity(Node *t);                  // 优先级
void printTree(Node *root, int level);  // 打印树
