#include "./include/include.h"
#include "./include/var.h"
#include "./parsePart/parsepart.h"
#include "./SemanticPart/Semantic.h" 
#include "./MidCodePart/mideCode.h"
#include "./optimization/opt.h"

const int numOfBaoLiu = 21; // 21个保留字;
string baoliu[25] = {"char", "int", "var", "program", "integer", "procedure", "array", "begin", "while", "if",
					 "then", "else", "endwh", "end", "read", "of", "record", "fi", "return", "write", "type"};

set<string> feizhongjifu{
	"Program", "ProgramHead", "ProgramName",
	"DeclarePart", "TypeDec", "TypeDeclaration", "TypeDecList",
	"TypeDecMore", "TypeId", "TypeName", "BaseType", "StructureType", "ArrayType",
	"Low", "Top", "RecType", "FieldDecList", "FieldDecMore", "IdList", "IdMore",
	"VarDec", "VarDeclaration", "VarDecList", "VarDecMore", "VarIdList", "VarIdMore", "ProcDec", "ProcDeclaration",
	"ProcDecMore", "ProcName", "ParamList", "ParamDecList", "ParamMore", "Param",
	"FormList", "FidMore", "ProcDecPart", "ProcBody", "ProgramBody", "StmList", "StmMore",
	"Stm", "AssCall", "AssignmentRest", "ConditionalStm", "LoopStm", "InputStm",
	"Invar", "OutputStm", "ReturnStm", "CallStmRest", "ActParamList", "ActParamMore",
	"RelExp", "OtherRelE", "Exp", "OtherTerm", "Term", "OtherFactor", "Factor",
	"Variable", "VariMore", "FieldVar", "FieldVarMore", "CmpOp", "AddOp", "MultOp"};

//from parsePart.cpp printTree 
//token kind
// 状态： 1.标识符 2.保留字 3.无符号数字 4.单分界符 5.双分界 6. 注释 7.数组下标 8.字符状态 18.出错

//node type
// 1 DecK Idk , 2 IntegerK , 3 CharK , 4 ArrayK , 5 FieldK , 6 PheadK , 7 ProK , 8 TypeK
// 9 VarK , 10 ProcK , 11 IfK , 12 WhileK , 13 ReadK , 14 WriteK , 15 ReturnK , 16 AssignK
// 17 CallK , 18 DecK , 19 StmLK , 20 ConstK , 21 OpK , 22 CmpK , 23 Varik/Expk

/*用到的类型
ProK 7 过程类型
PheadK 6 程序头部类型
Procdek 8 过程声明类型
TypeK 9 类型类型
Deck 18 表达式类型
StmlK 19 语句类型
Vark 23 表达式类型

Expk -> constk | opk | vark
stmtk -> ifk | whilek | readk | writek | returnk | assignk | callk
decK ->  Arrayk | CharK | IntegerK | IdK | RecordK  
*/

// 1. 语法分析
// 2. 语义分析
// 3. 生成目标代码
int main(void)
{
	FILE *fp = fopen("token.txt", "r");
	if (fp == NULL)
	{
		cout << "文件打开失败" << endl;
		return 0;
	}
	fscanf(fp, "%d", &p_token);
	cout << p_token << endl;
	for (int i = 0; i < p_token; i++)
	{
		fscanf(fp, "%d%d%s", &token[i].linenum, &token[i].type, token[i].s);
		token[i].str = token[i].s;

		// for debug
		//cout<<token[i].linenum << ' ' << token[i].type << ' ' << token[i].str << endl;
	} // 读取token序列

	cout << "开始语法分析" << endl;
	parsell();			// 语法分析，创建语法树
	printTree(root, 0); // 打印语法树
	cout << "语法分析结束" << endl;

	printf("\n\n\n***********\n");
	printf("错误信息区\n");
	for (int i = 0; i < 100; i++)
	{
		scope[i] = nullptr;
	}

	cout << "开始语义分析" << endl;
	analyze(root); // 语义分析
	printf("错误信息区结束\n");
	printf("***********\n\n\n");

	printf("\n\n\n***********\n");
	cout << "打印符号表(´▽｀)" << endl;
	printsymbtable();
	cout << "语义分析结束 ( ´▽｀)" << endl;
	//exit(0);
	
	// 生成中间代码
	printf("\n\n\n***********\n");
	cout << "开始生成中间代码 凸^-^凸" << endl;
	GenMidCode(root); // 生成中间代码
	cout << "生成中间代码结束 (^з^)-☆" << endl;
	printf("***********\n\n\n");

	// 打印中间代码
	cout << "打印中间代码 (´▽｀)" << endl;
	PrintMidCode(head);
	cout << "打印中间代码结束 ( ´▽｀)" << endl;

	// 优化
	printf("\n\n\n***********\n");
	cout << "开始优化 o(^▽^)o" << endl;
	CodeFile* optCodeFile = ConstOptimize();
	cout << "优化结束" << endl;
	printf("***********\n\n\n");

	// 打印中间代码
	//PrintMidCode(optCodeFile);
	cout << "打印优化后的中间代码 (´▽｀)" << endl;
	PrintMidCode(optCodeFile);
	return 0;
}
