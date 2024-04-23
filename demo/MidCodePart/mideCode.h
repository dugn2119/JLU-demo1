#include "../include/include.h"
#include "../include/var.h"
#include "../SemanticPart/Semantic.h"
#ifndef MIDCODE_H
#define MIDCODE_H
enum Form
{
	ValueForm, // 数值类
	LabelForm, // 标号类
	AddrForm   // 地址类
};

enum CodeKind
{
	// 算术运算
	ADD,
	SUB,
	MULT,
	DIV,
	// 关系运算
	EQC,
	LTC,
	// 语句
	READC,
	WRITEC,
	RETURNC,
	ASSIG,
	AADD,
	LABEL,
	JUMP,
	JUMP0,
	CALL,
	VARACT,
	VALACT,
	// 定位
	PENTRY,
	ENDPROC,
	MENTRY,
	// 循环
	WHILESTART,
	ENDWHILE
};
// arg结构
typedef struct ARG
{
	Form form; // ValueForm ,LabelForm ,AddrForm
	int value; // 数值类 在form取值ValueForm 时有效，记录常数的值;
	int label; // 标号类 在form取值LabelForm时有效，记录标号值;
	struct
	{
		string name;	   // 地址类 ，记录变量名;
		int dataLevel;	   // 数据层次 ，记录变量的层次;
		int dataOff;	   // 数据偏移量 ，记录变量的偏移量;
		accesskind access; // dir or indir ，记录变量的访问方式;
	} Addr;				   // 地址类
	//};
	ARG()
	{
	}
} ARG;

// 中间代码结构
struct MidCode
{
	CodeKind codekind;
	ARG *arg1; // 操作分量1
	ARG *arg2; // 操作分量2
	ARG *arg3; // 结果
};

// 中间代码序列，双向链表 former指向前一个，next指向后一个
struct CodeFile
{
	CodeFile *former;
	MidCode *onecode;
	CodeFile *next;
};

extern int tmp_num;			  // 临时变量个数（编号）
extern int label_num;		  // 标号个数（编号）
extern CodeFile *head, *tail; // 中间代码序列头尾指针

// 新建一个临时变量
ARG *NewTemp(accesskind access);
// 新建数值类 ARG 结构
ARG *ARGValue(int value);
// 产生一个新的标号
int NewLabel();
// 新建标号类 ARG 结构
ARG *ARGLabel(int label);
// 创建地址类 ARG 结构
ARG *ARGAddr(string id, int level, int off, accesskind access);
// 输出中间代码
void PrintMidCode(CodeFile *firstCode);
// 对比mode1和mode2的中间代码
void printMidCodeByContrast(CodeFile *head1, CodeFile *head2);
// 生成中间代码
CodeFile *GenCode(CodeKind codekind, ARG *Arg1, ARG *Arg2, ARG *Arg3);
// 中间代码生成主函数
CodeFile *GenMidCode(Node *t);
// 过程声明中间代码生成函数
void GenProcDec(Node *t);
// 语句体中间代码生成函数
void GenBody(Node *t);
// 语句的中间代码生成函数
void GenStatement(Node *t);
// 赋值语句中间代码生成函数
void GenAssignS(Node *t);
// 变量中间代码生成函数
ARG *GenVar(Node *t);
// 数组变量的中间代码生成函数
ARG *GenArray(ARG *Vlarg, Node *t, int low, int size);
// 域成员变量的中间代码生成
ARG *GenField(ARG *V1arg, Node *t, fieldchain *head);
// 表达式的中间代码生成函数
ARG *GenExpr(Node *t);
// 过程调用语句中间代码生成函数
void GenCallS(Node *t);
// 读语句中间代码生成函数
void GenReadS(Node *t);
// 条件语句中间代码生成函数
void GenIfS(Node *t);
// 写语句中间代码生成函数
void GenWriteS(Node *t);
// 循环语句中间代码生成函数
void GenWhileS(Node *t);

#endif