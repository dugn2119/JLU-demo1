#include "include.h"
#ifndef VAR_H
#define VAR_H

#define IDK 1
#define INTEGERK 2
#define CHARK 3
#define ARRAYK 4
#define FIELDK 5
#define PHEADK 6
#define PROK 7
#define TYPEK 8
#define VARK 9
#define PROCK 10
#define IFK 11
#define WHILEK 12
#define READK 13
#define WRITEK 14
#define RETURNK 15
#define ASSIGNK 16
#define CALLK 17
#define DECK 18
#define STMLK 19
#define CONSTK 20
#define OPK 21
#define CMPK 22
#define VARIK 23
#define EXPK 24


struct symbtable; // 符号表

struct Token // token序列
{
	int linenum; // 行号
	int type;	 // 类别
	char s[100]; // 信息
	string str;	 // 信息
};

typedef struct Node
{

	Node *child[3]; // 子节点
	Node *brother;	// sibling
	int num;		// 第几行 lineno

	int idnum;		// 几个数 记录一个节点中的标志符的个数
	string str[10]; // 字符串数组,存储标识符

	// 1 DecK Idk , 2 IntegerK , 3 CharK , 4 ArrayK , 5 FieldK , 6 PheadK , 7 ProK , 8 TypeK
	// 9 VarK , 10 ProcK , 11 IfK , 12 WhileK , 13 ReadK , 14 WriteK , 15 ReturnK , 16 AssignK
	// 17 CallK , 18 DecK , 19 StmLK , 20 ConstK , 21 OpK , 22 CmpK , 23 Varik
	int type; // 类型
	int low;  // 数组下界
	int high; // 数组上界

	int type1;	// 参数类型  1值参 2
	int type2;	// 数组元素类型
	string exp; // 类型 constk opk vark/分别为常量、操作符、变量

	symbtable *entry = nullptr; // 符号表中的位置

	Node(string s)
	{ // 构造函数
		// 这里要赋值
		for (int i = 0; i < 10; i++)
		{
			str[i] = "";
		}
		str[0] = s;
		brother = nullptr;
		child[0] = nullptr;
		child[1] = nullptr;
		child[2] = nullptr;
		idnum = 0;
		type = 0;
		num = 0;
	}
} Node; // 这里写树节点信息

extern string baoliu[25];
extern set<string> feizhongjifu;                            // 非终极符
extern stack<string> fuhao;                                  // fuhao.push("abc");  fuhao.pop(); stack stl
extern int fuhaoempty;                                       // fuhao.empty()
extern stack<Node **> yufashu;                               // 语法树
extern stack<Node *> caozuofu;                               // 操作符栈
extern stack<Node *> caozuoshu;                              // 操作数栈
extern int lineno;                                           // 单词的行号
extern int now;                                              // 第几个token
extern Node *root;                                           // root 语法树根节点
extern unordered_map<string, unordered_map<string, int> > mp; // ll1分析表
extern int p_token;                                          // token序列总数 61
extern Token token[500];                                     // 用于存token的数组
extern symbtable *scope[100];                                // 符号表
#endif

