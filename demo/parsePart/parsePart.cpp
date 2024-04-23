#include "./parsepart.h"
#define ID 1
#define BAOLIU 2
#define UNSIGNED 3
#define DANFENJIE 4
#define SHUANGFENJIE 5
#define ZHUSHI 6
#define SHUZUXIABIAO 7
#define ZIFUCHUANG 8
stack<string> fuhao;								  // fuhao.push("abc");  fuhao.pop(); stack stl
int fuhaoempty;										  // fuhao.empty()
stack<Node **> yufashu;								  // 语法树
stack<Node *> caozuofu;								  // 操作符栈
stack<Node *> caozuoshu;							  // 操作数栈
int lineno;											  // 单词的行号
int now;											  // 第几个token
Node *root;											  // root 语法树根节点
unordered_map<string, unordered_map<string, int> > mp; // ll1分析表
int p_token;										  // token序列总数
Token token[500];									  // 用于存token的数组
symbtable *scope[100];

// token kind
//  状态： 1.标识符 2.保留字 3.无符号数字 4.单分界符 5.双分界 6. 注释 7.数组下标 8.字符状态 18.出错

// node type
//  1 Idk , 2 DecK-IntegerK , 3 DecK-CharK , 4 DecK-ArrayK , 5 DecK-FieldK , 6 PheadK , 7 ProK , 8 TypeK
//  9 VarK , 10 ProcK , 11 stmtk-IfK , 12 stmtk-WhileK , 13 stmtk-ReadK , 14stmtk-WriteK , 15 stmtk-ReturnK , 16 stmtk-AssignK
//  17 stmtk-CallK , 18 DecK , 19 StmLK , 20 ConstK , 21OpK , 22 CmpK , 23 Varik/Expk

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

void wrong(int num, string str, int n)
{ // num 参数表示出错的行号，str 参数表示出错的描述信息，n 参数表示错误类型。
	printf("第%d行%s出错啦\n", num, str.c_str());
	if (n == 0)
	{
		printf("你的非终极符找不到对应的predict集啦o(^_-)O");
	}
	else if (n == 1)
	{
		printf("终极符与当前token不匹配 (･_･;");
	}
	else if (n == 2)
	{
		printf("符号栈空,可你的token还有 （；゜０゜）");
	}
	exit(-1);
}

int match(int now, string temp)
{ /*在调用 match() 函数时，传入的 temp 参数通常会根据当前语法规则或分析阶段的需要来指定。
  比如，当你期望当前 token 是一个标识符时，你会传入 "id" 作为 temp，
  如果期望的是一个无符号数字，你会传入 "intc" 作为 temp。*/
	if (token[now].type == ID)// 标识符
	{ /*对于标识符类型的 token（type == 1）
	  如果 temp 的值是 "id"，表示当前正在期待一个标识符，因此只有当当前 token 的类型为标识符（type == 1）且其字符串值为标识符时，才会返回 1，表示匹配；
	  否则返回 0，表示不匹配。*/
		if (temp == "id")
			return 1;
		else
			return 0;
	}
	else if (token[now].type == UNSIGNED)// 无符号数字
	{ /*当前 token 的类型是无符号数字类型（type == 3）
	  如果 temp 的值是 "intc"，表示当前正在期待一个无符号数字，因此只有当当前 token 的类型为无符号数字（type == 3）时，才会返回 1，表示匹配；
	  否则返回 0，表示不匹配。*/
		if (temp == "intc")
			return 1;
		else
			return 0;
	}
	else /*如果当前 token 的类型既不是标识符也不是无符号数字，那么我们需要直接比较当前 token 的字符串值与 temp 是否相等。
		 如果它们相等，则返回 1，表示匹配；否则返回 0，表示不匹配。*/
		return token[now].str == temp;
}

/*  (1)
	处理声明部分和语句部分的语法树生成时，设置一个语法树栈，存放语法树节点中指向儿子或者兄弟节点的指针的地址。
	在生成当前语法树节点时，如果以后需要对其儿子节点或者兄弟节点赋值，
	则按照处理顺序的逆序将这些儿子节点或者兄弟节点的指针的地址压入语法树栈，
	后面生成它的儿子节点或者兄弟节点时，只需弹栈，并对相应的指针进行赋值，就可以完成所需的语法树节点的链接。
	(2)
	处理表达式时，需要另外设置两个栈，操作数栈和操作符栈，
	遇到操作数压入操作数栈，
	遇到操作符，则进行判断，
	如果当前操作符的优先级高于操作符栈的栈顶操作符，直接压入操作符栈;
	否则，弹出栈顶操作符，并弹出操作数栈顶的两个操作数，生成相应的子树，并对新生成的子树的父节点(操作符节点)进行循环判断。
*/

// 非终极符 终极符 -> 产生式序号
Node *parsell()
{
	createLL1Table();		 // 创建分析表
	fuhaoempty = true;		 // 标记符号栈是否为空的变量，初始值设置为1是因为在开始时通常符号栈是非空的。
	fuhao.push("Program");	 // 将起始符号压入符号栈，这是语法分析器的入口，从此开始对输入的源代码进行分析。
	root = new Node("root"); // 创建语法树的根节点
	root->type = PROK;		 // 根节点的类型为程序类型
	// 将程序的子节点压入语法树节点栈中，按顺序是TypeK类型声明、PheadK变量声明、ProK过程声明
	yufashu.push(&root->child[2]);
	yufashu.push(&root->child[1]);
	yufashu.push(&root->child[0]); // 入结点栈，后进先出，最左边的child[0]最先出来所以后进
	now = 0;					   // 初始化当前处理的 token 索引为0，表示从源代码的第一个 token 开始分析
	lineno = token[now].linenum;   // 获取当前处理的 token 的行号，用于报错信息的输出

	// 循环直到符号栈为空
	while (!fuhao.empty())
	{
		cout << "**************" << endl;
		string temp = fuhao.top(); // 取出符号栈的栈顶符号
		cout << "current token:=" << now << "   " << temp << "   " << token[now].str << endl;

		// 判断操作符栈是否为空，如果不为空，输出操作符栈顶元素，用于调试和输出信息。
		if (caozuofu.size() == 0)
			cout << "操作符栈为空" << endl;
		else
			cout << "操作符top为" << caozuofu.top()->str[0] << endl;

		/*1.分析栈的栈顶元素是终极符，则看其是否与输入流的头符相匹配，如果匹配成功，去掉栈顶元素,并读下一个单词;若匹配不成功，则报错。
		  2.栈顶是非终极符，则用栈顶和输入流的当前单词去查当前矩阵，如果查得的值是产生式编号，则把对应的产生式右部逆序压入栈中;
			如果查得的值为错误信息，则报错。
		  3.栈己空，输入流不空，这时输入流报错。.
		  4.若栈已空，输入流也空，则语法分析成功。*/
		// 检查当前符号是否为非终结符，如果当前符号 temp 是非终结符，即在 feizhongjifu 中找得到，则进行以下操作：
		if (feizhongjifu.count(temp))
		{
			cout << "token为非终极符" << endl;
			string s = token[now].str;
			/*根据当前token的类型确定预测表中查找的符号类型。
			非终结符的预测表中使用了不同类型的标识符来表示终结符，
			例如 "id" 表示标识符，"intc" 表示整数常量等。*/
			if (token[now].type == ID) // 标识符
				s = "id";
			else if (token[now].type == UNSIGNED) // 无符号数字
				s = "intc";
			// 查找预测表，获取产生式序号
			int pnum = mp[temp][s]; /*根据当前非终结符 temp 和当前 token 的类型 s，
									栈顶非终极符作为列号，Token的词法信息作为行号。从预测表 mp 中获取产生式序号 pnum。
									预测表 mp 存储了非终结符和终结符的组合与相应的产生式序号之间的映射关系。*/
			if (pnum == 0)			// 如果产生式序号为0，表示找不到对应的产生式，报错退出，因为我们合法的有效产生式是从1开始编号的
				wrong(token[now].linenum, token[now].str, 0);
			cout << pnum << endl; // 输出产生式序号，用于调试和分析。
			fuhao.pop();		  // 弹出当前处理的非终结符 temp，因为在这一步已经确定了使用了对应的产生式。
			predict(pnum);		  // 调用预测函数，根据产生式序号 pnum 进行语法分析的下一步操作。这个函数可能涉及到推导产生式右部的符号序列，并将它们压入符号栈，以便进一步分析。
		}
		else // 如果当前符号为终结符
		{
			cout << "token为终极符" << endl;
			// 调用 match() 函数检查当前 token 是否与符号栈顶的符号匹配
			if (match(now, temp)) // 如果匹配
			{
				fuhao.pop();				 // 弹出符号栈顶的符号，因为它已经被匹配掉了。
				now++;						 // 处理下一个 token，即将 now 变量加一，指向下一个 token。
				lineno = token[now].linenum; // 更新行号
			}
			else // 如果当前 token 与符号栈顶的符号不匹配
			{
				wrong(token[now].linenum, token[now].str, 1);
			}
		}
	}

	// 检查是否已经处理完所有的token
	if (token[now].str == ".") // 文件结束符
	{
		cout << "ending !!! (￣▽￣)" << endl;
		return root; // 返回语法树的根节点
	}
	else // 如果没有处理完所有的token，报错退出
		wrong(token[now].linenum, token[now].str, 2);
}

// 创建分析表
/*创建LL(1)分析表。
用二维数组表示LL(1)分析表，初始化二维数组所有元素为0，
据给定的LL(1)文法，对产生式从1编号，共104个产生式。
对于每个产生式，产生式左部的非终极符作为行号，此产生式的Predict集中每个元素分别作为列号，
数组中由此行号，列号对应的元素赋值为这个产生式的编号。*/

/*例如对于第4个产生式:< <DeclarePart>::= TypeDec VarDec ProcDec
它的predict集为: { TYPE, VAR, FUNCTION, BEGIN }
创建LL(1)分析表时，这个产生式部分对应的语句为:
	mp["DeclarePart"]["type"] = 4;
	mp["DeclarePart"]["var"] = 4;
	mp["DeclarePart"]["procedure"] = 4;
	mp["DeclarePart"]["begin"] = 4;*/

void createLL1Table()
{
	mp["Program"]["program"] = 1;
	mp["ProgramHead"]["program"] = 2;
	mp["ProgramName"]["id"] = 3;
	mp["DeclarePart"]["type"] = 4;
	mp["DeclarePart"]["var"] = 4;
	mp["DeclarePart"]["procedure"] = 4;
	mp["DeclarePart"]["begin"] = 4;
	mp["TypeDec"]["var"] = 5;
	mp["TypeDec"]["procedure"] = 5;
	mp["TypeDec"]["begin"] = 5;
	mp["TypeDec"]["type"] = 6;
	mp["TypeDeclaration"]["type"] = 7;
	mp["TypeDecList"]["id"] = 8;
	mp["TypeDecMore"]["var"] = 9;
	mp["TypeDecMore"]["procedure"] = 9;
	mp["TypeDecMore"]["begin"] = 9;
	mp["TypeDecMore"]["id"] = 10;
	mp["TypeId"]["id"] = 11;
	mp["TypeName"]["integer"] = 12;
	mp["TypeName"]["char"] = 12;
	mp["TypeName"]["array"] = 13;
	mp["TypeName"]["record"] = 13;
	mp["TypeName"]["id"] = 14;
	mp["BaseType"]["integer"] = 15;
	mp["BaseType"]["char"] = 16;
	mp["StructureType"]["array"] = 17;
	mp["StructureType"]["record"] = 18;
	mp["ArrayType"]["array"] = 19;
	mp["Low"]["intc"] = 20;
	mp["Top"]["intc"] = 21;
	mp["RecType"]["record"] = 22;
	mp["FieldDecList"]["integer"] = 23;
	mp["FieldDecList"]["char"] = 23;
	mp["FieldDecList"]["array"] = 24;
	mp["FieldDecMore"]["end"] = 25;
	mp["FieldDecMore"]["integer"] = 26;
	mp["FieldDecMore"]["char"] = 26;
	mp["FieldDecMore"]["array"] = 26;
	mp["IdList"]["id"] = 27;
	mp["IdMore"][";"] = 28;
	mp["IdMore"][","] = 29;
	mp["VarDec"]["procedure"] = 30;
	mp["VarDec"]["begin"] = 30;
	mp["VarDec"]["var"] = 31;
	mp["VarDeclaration"]["var"] = 32;
	mp["VarDecList"]["integer"] = 33;
	mp["VarDecList"]["char"] = 33;
	mp["VarDecList"]["array"] = 33;
	mp["VarDecList"]["record"] = 33;
	mp["VarDecList"]["id"] = 33;
	mp["VarDecMore"]["procedure"] = 34;
	mp["VarDecMore"]["begin"] = 34;
	mp["VarDecMore"]["integer"] = 35;
	mp["VarDecMore"]["char"] = 35;
	mp["VarDecMore"]["array"] = 35;
	mp["VarDecMore"]["record"] = 35;
	mp["VarDecMore"]["id"] = 35;
	mp["VarIdList"]["id"] = 36;
	mp["VarIdMore"][";"] = 37;
	mp["VarIdMore"][","] = 38;
	mp["ProcDec"]["begin"] = 39;
	mp["ProcDec"]["procedure"] = 40;
	mp["ProcDeclaration"]["procedure"] = 41;
	mp["ProcDecMore"]["begin"] = 42;
	mp["ProcDecMore"]["procedure"] = 43;
	mp["ProcName"]["id"] = 44;
	mp["ParamList"][")"] = 45;
	mp["ParamList"]["integer"] = 46;
	mp["ParamList"]["char"] = 46;
	mp["ParamList"]["array"] = 46;
	mp["ParamList"]["record"] = 46;
	mp["ParamList"]["id"] = 46;
	mp["ParamList"]["var"] = 46;
	mp["ParamDecList"]["integer"] = 47;
	mp["ParamDecList"]["char"] = 47;
	mp["ParamDecList"]["array"] = 47;
	mp["ParamDecList"]["record"] = 47;
	mp["ParamDecList"]["id"] = 47;
	mp["ParamDecList"]["var"] = 47;
	mp["ParamMore"][")"] = 48; //***
	mp["ParamMore"][";"] = 49;
	mp["Param"]["integer"] = 50;
	mp["Param"]["char"] = 50;
	mp["Param"]["array"] = 50;
	mp["Param"]["record"] = 50;
	mp["Param"]["id"] = 50;
	mp["Param"]["var"] = 51;
	mp["FormList"]["id"] = 52;
	mp["FidMore"][";"] = 53;
	mp["FidMore"][")"] = 53;
	mp["FidMore"][","] = 54;
	mp["ProcDecPart"]["type"] = 55;
	mp["ProcDecPart"]["var"] = 55;
	mp["ProcDecPart"]["procedure"] = 55;
	mp["ProcDecPart"]["begin"] = 55;
	mp["ProcBody"]["begin"] = 56;
	mp["ProgramBody"]["begin"] = 57;
	mp["StmList"]["id"] = 58;
	mp["StmList"]["if"] = 58;
	mp["StmList"]["while"] = 58;
	mp["StmList"]["return"] = 58;
	mp["StmList"]["read"] = 58;
	mp["StmList"]["write"] = 58;
	mp["StmMore"]["else"] = 59;
	mp["StmMore"]["fi"] = 59;
	mp["StmMore"]["end"] = 59;
	mp["StmMore"]["endwh"] = 59;
	mp["StmMore"][";"] = 60;
	mp["Stm"]["if"] = 61;
	mp["Stm"]["while"] = 62;
	mp["Stm"]["read"] = 63;
	mp["Stm"]["write"] = 64;
	mp["Stm"]["return"] = 65;
	mp["Stm"]["id"] = 66;
	mp["AssCall"][":="] = 67;
	mp["AssCall"]["."] = 67; //***
	mp["AssCall"]["["] = 67; //***
	mp["AssCall"]["("] = 68;
	mp["AssignmentRest"]["["] = 69;
	mp["AssignmentRest"]["."] = 69;
	mp["AssignmentRest"][":="] = 69;
	mp["ConditionalStm"]["if"] = 70;
	mp["LoopStm"]["while"] = 71;
	mp["InputStm"]["read"] = 72;
	mp["Invar"]["id"] = 73;
	mp["OutputStm"]["write"] = 74;
	mp["ReturnStm"]["return"] = 75;
	mp["CallStmRest"]["("] = 76;
	mp["ActParamList"][")"] = 77;
	mp["ActParamList"]["("] = 78;
	mp["ActParamList"]["intc"] = 78;
	mp["ActParamList"]["id"] = 78;
	mp["ActParamMore"][")"] = 79;
	mp["ActParamMore"][","] = 80;
	mp["RelExp"]["("] = 81;
	mp["RelExp"]["intc"] = 81;
	mp["RelExp"]["id"] = 81;
	mp["OtherRelE"]["<"] = 82;
	mp["OtherRelE"]["="] = 82;
	mp["Exp"]["("] = 83;
	mp["Exp"]["intc"] = 83;
	mp["Exp"]["id"] = 83;
	mp["OtherTerm"]["<"] = 84;
	mp["OtherTerm"]["="] = 84;
	mp["OtherTerm"]["]"] = 84;
	mp["OtherTerm"]["then"] = 84;
	mp["OtherTerm"]["else"] = 84;
	mp["OtherTerm"]["fi"] = 84;
	mp["OtherTerm"]["do"] = 84;
	mp["OtherTerm"]["endwh"] = 84;
	mp["OtherTerm"][")"] = 84;
	mp["OtherTerm"]["end"] = 84;
	mp["OtherTerm"][";"] = 84;
	mp["OtherTerm"][","] = 84;
	mp["OtherTerm"]["+"] = 85;
	mp["OtherTerm"]["-"] = 85;
	mp["Term"]["("] = 86;
	mp["Term"]["intc"] = 86;
	mp["Term"]["id"] = 86;
	mp["OtherFactor"]["+"] = 87;
	mp["OtherFactor"]["-"] = 87;
	mp["OtherFactor"]["<"] = 87;
	mp["OtherFactor"]["="] = 87;
	mp["OtherFactor"]["]"] = 87;
	mp["OtherFactor"]["then"] = 87;
	mp["OtherFactor"]["else"] = 87;
	mp["OtherFactor"]["fi"] = 87;
	mp["OtherFactor"]["do"] = 87;
	mp["OtherFactor"]["endwh"] = 87;
	mp["OtherFactor"][")"] = 87;
	mp["OtherFactor"]["end"] = 87;
	mp["OtherFactor"][";"] = 87;
	mp["OtherFactor"][","] = 87;
	mp["OtherFactor"]["*"] = 88;
	mp["OtherFactor"]["/"] = 88;
	mp["Factor"]["("] = 89;
	mp["Factor"]["intc"] = 90;
	mp["Factor"]["id"] = 91;
	mp["Variable"]["id"] = 92;
	mp["VariMore"][":="] = 93;
	mp["VariMore"]["*"] = 93;
	mp["VariMore"]["/"] = 93;
	mp["VariMore"]["+"] = 93;
	mp["VariMore"]["-"] = 93;
	mp["VariMore"]["<"] = 93;
	mp["VariMore"]["="] = 93;
	mp["VariMore"]["then"] = 93;
	mp["VariMore"]["else"] = 93;
	mp["VariMore"]["fi"] = 93;
	mp["VariMore"]["do"] = 93;
	mp["VariMore"]["endwh"] = 93;
	mp["VariMore"][")"] = 93;
	mp["VariMore"]["end"] = 93;
	mp["VariMore"][";"] = 93;
	mp["VariMore"][","] = 93;
	mp["VariMore"]["]"] = 93;
	mp["VariMore"]["["] = 94;
	mp["VariMore"]["."] = 95;
	mp["FieldVar"]["id"] = 96;
	mp["FieldVarMore"][":="] = 97;
	mp["FieldVarMore"]["*"] = 97;
	mp["FieldVarMore"]["/"] = 97;
	mp["FieldVarMore"]["+"] = 97;
	mp["FieldVarMore"]["-"] = 97;
	mp["FieldVarMore"]["<"] = 97;
	mp["FieldVarMore"]["="] = 97;
	mp["FieldVarMore"]["then"] = 97;
	mp["FieldVarMore"]["else"] = 97;
	mp["FieldVarMore"]["fi"] = 97;
	mp["FieldVarMore"]["do"] = 97;
	mp["FieldVarMore"]["endwh"] = 97;
	mp["FieldVarMore"][")"] = 97;
	mp["FieldVarMore"]["end"] = 97;
	mp["FieldVarMore"][";"] = 97;
	mp["FieldVarMore"][","] = 97;
	mp["FieldVarMore"]["]"] = 97;
	mp["FieldVarMore"]["["] = 98;
	mp["CmpOp"]["<"] = 99;
	mp["CmpOp"]["="] = 100;
	mp["AddOp"]["+"] = 101;
	mp["AddOp"]["-"] = 102;
	mp["MultOp"]["*"] = 103;
	mp["MultOp"]["/"] = 104;
	return;
}

void process1()
{
	fuhao.push("ProgramBody");
	fuhao.push("DeclarePart");
	fuhao.push("ProgramHead");
}

Node *currentP;
void process2()
{
	fuhao.push("ProgramName");
	fuhao.push("program");
	Node **t = yufashu.top();
	yufashu.pop();
	currentP = new Node("");
	*t = currentP;
	currentP->type = 6;
}

void process3()
{
	fuhao.push("id");
	currentP->str[0] = token[now].str;
	currentP->num = token[now].linenum;
	currentP->idnum++;
}

void process4()
{
	fuhao.push("ProcDec");
	fuhao.push("VarDec");
	fuhao.push("TypeDec");
}

void process5() {}

void process6()
{
	fuhao.push("TypeDeclaration");
}

void process7()
{
	fuhao.push("TypeDecList");
	fuhao.push("type");
	Node **t = yufashu.top();
	yufashu.pop();
	currentP = new Node("");
	*t = currentP;
	currentP->type = 8;
	yufashu.push(&((*t)->brother));
	yufashu.push(&((*t)->child[0]));
}

void process8()
{
	fuhao.push("TypeDecMore");
	fuhao.push(";");
	fuhao.push("TypeName");
	fuhao.push("=");
	fuhao.push("TypeId");

	Node **t = yufashu.top();
	yufashu.pop();
	// cout<<"8******"<<endl;
	// cout<<*t<<endl;
	currentP = new Node("");
	currentP->type = 18;
	*t = currentP;
	yufashu.push(&((*t)->brother));
}

void process9()
{
	yufashu.pop();
}

void process10()
{
	fuhao.push("TypeDecList");
}

void process11()
{
	fuhao.push("id");
	currentP->str[0] = token[now].str;
	currentP->num = token[now].linenum;
	currentP->idnum++;
}

int *temp;
void process12()
{
	fuhao.push("BaseType");
	temp = &currentP->type;
}

void process13()
{
	fuhao.push("StructureType");
}

void process14()
{
	fuhao.push("id");
	currentP->type = 1;
	// cout<<token[now].str<<endl;
	currentP->str[0] = token[now].str;
	currentP->num = token[now].linenum;
	currentP->idnum++;
}

void process15()
{
	fuhao.push("integer");
	// cout<<currentP->type<<endl;
	*temp = 2;
}

void process16()
{
	fuhao.push("char");
	*temp = 3;
}

void process17()
{
	fuhao.push("ArrayType");
}

void process18()
{
	fuhao.push("RecType");
}

void process19()
{
	fuhao.push("BaseType");
	fuhao.push("of");
	fuhao.push("]");
	fuhao.push("Top");
	fuhao.push("..");
	fuhao.push("Low");
	fuhao.push("[");
	fuhao.push("array");
	currentP->type = 4;
	temp = &currentP->type2;
}

void process20()
{
	fuhao.push("intc");
	currentP->low = stoi(token[now].str);
}

void process21()
{
	fuhao.push("intc");
	currentP->high = stoi(token[now].str);
}

Node *saveP;
void process22()
{
	fuhao.push("end");
	fuhao.push("FieldDecList");
	fuhao.push("record");
	// cout<<temp;
	currentP->type = 5;
	saveP = currentP;
	// cout<<currentP->child[0]<<endl;
	yufashu.push(&currentP->child[0]);
}

void process23()
{
	fuhao.push("FieldDecMore");
	fuhao.push(";");
	fuhao.push("IdList");
	fuhao.push("BaseType");
	Node **t = yufashu.top();
	yufashu.pop();
	currentP = new Node("");
	temp = &currentP->type;
	*t = currentP;
	yufashu.push(&(currentP->brother));
}

void process24()
{
	fuhao.push("FieldDecMore");
	fuhao.push(";");
	fuhao.push("Idlist");
	fuhao.push("ArrayType");
	currentP = new Node("");
	currentP->type = 18;
	Node **t = yufashu.top();
	yufashu.pop();
	*t = currentP;
	temp = &currentP->type;
	yufashu.push(&((*t)->child[0]));
}

void process25()
{
	// cout << currentP->str[0] << " " << currentP->type << endl;
	yufashu.pop();
	currentP = saveP;
}

void process26()
{
	fuhao.push("FieldDecList");
}

void process27()
{
	fuhao.push("IdMore");
	fuhao.push("id");
	// cout<<token[now].str<<endl;
	currentP->str[currentP->idnum] = token[now].str;
	currentP->num = token[now].linenum;
	currentP->idnum++;
}

void process28()
{
}

void process29()
{
	fuhao.push("IdList");
	fuhao.push(",");
}

void process30()
{
}

void process31()
{
	fuhao.push("VarDeclaration");
}

void process32()
{
	// cout<<"lala"<<endl;
	fuhao.push("VarDecList");
	fuhao.push("var");
	currentP = new Node("");
	currentP->type = 9;
	currentP->exp = "vark";
	Node **t = yufashu.top();
	yufashu.pop();
	*t = currentP;
	yufashu.push(&((*t)->brother));
	yufashu.push(&((*t)->child[0]));
}

void process33()
{
	fuhao.push("VarDecMore");
	fuhao.push(";");
	fuhao.push("VarIdList");
	fuhao.push("TypeName");
	currentP = new Node("");
	currentP->type = 18;
	Node **t = yufashu.top();
	yufashu.pop();
	*t = currentP;
	yufashu.push(&((*t)->brother));
}

void process34()
{
	yufashu.pop();
}

void process35()
{
	fuhao.push("VarDecList");
}

void process36()
{
	fuhao.push("IdMore");
	fuhao.push("id");
	// cout<<token[now].str<<endl;
	currentP->str[currentP->idnum] = token[now].str;
	currentP->num = token[now].linenum;
	currentP->idnum++;
}

void process37()
{
}

void process38()
{
	fuhao.push("VarIdList");
	fuhao.push(",");
}

void process39()
{
}

void process40()
{
	fuhao.push("ProcDeclaration");
}

void process41()
{
	// cout<<"allaal"<<endl;
	fuhao.push("ProcDecMore");
	fuhao.push("ProcBody");
	fuhao.push("ProcDecPart");
	fuhao.push(";");
	fuhao.push(")");
	fuhao.push("ParamList");
	fuhao.push("(");
	fuhao.push("ProcName");
	fuhao.push("procedure");
	currentP = new Node("");
	currentP->type = 10;
	Node **t = yufashu.top();
	yufashu.pop();
	*t = currentP;
	yufashu.push(&((*t)->brother));
	cout << &((*t)->child[2]) << endl;
	cout << &((*t)->child[1]) << endl;
	cout << &((*t)->child[0]) << endl;
	yufashu.push(&((*t)->child[2]));
	yufashu.push(&((*t)->child[1]));
	yufashu.push(&((*t)->child[0]));
}

void process42()
{
}

void process43()
{
	fuhao.push("ProcDeclaration");
}

void process44()
{
	fuhao.push("id");
	currentP->str[0] = token[now].str;
	currentP->num = token[now].linenum;
	currentP->idnum++;
}

void process45()
{
	yufashu.pop();
}

void process46()
{
	fuhao.push("ParamDecList");
}

void process47()
{
	fuhao.push("ParamMore");
	fuhao.push("Param");
}

void process48()
{
	yufashu.pop();
	saveP = currentP;
	temp = &currentP->type;
}

void process49()
{
	fuhao.push("ParamDecList");
	fuhao.push(";");
}

void process50()
{
	cout << "50renne?" << endl;
	fuhao.push("FormList");
	fuhao.push("TypeName");
	currentP = new Node("");
	currentP->type1 = 18;
	Node **t = yufashu.top();
	cout << t << endl;
	yufashu.pop();
	*t = currentP;
	yufashu.push(&((*t)->brother));
}

void process51()
{
	fuhao.push("FormList");
	fuhao.push("TypeName");
	fuhao.push("var");
	currentP = new Node("");
	currentP->type = 18;
	currentP->type1 = 1; // 变参
	Node **t = yufashu.top();
	yufashu.pop();
	*t = currentP;
	yufashu.push(&((*t)->brother));
}

void process52()
{
	fuhao.push("FidMore");
	fuhao.push("id");
	currentP->str[currentP->idnum] = token[now].str;
	currentP->num = token[now].linenum;
	currentP->idnum++;
}

void process53()
{
}

void process54()
{
	fuhao.push("FormList");
	fuhao.push(",");
}

void process55()
{
	fuhao.push("DeclarePart");
}

void process56()
{
	fuhao.push("ProgramBody");
}

void process57()
{
	fuhao.push("end");
	fuhao.push("StmList");
	fuhao.push("begin");
	yufashu.pop();
	currentP = new Node("");
	currentP->type = 19;
	Node **t = yufashu.top();
	cout << t << endl;
	yufashu.pop();
	*t = currentP;
	yufashu.push(&((*t)->child[0]));
}

void process58()
{
	fuhao.push("StmMore");
	fuhao.push("Stm");
}

void process59()
{
	yufashu.pop();
}

void process60()
{
	fuhao.push("StmList");
	fuhao.push(";");
}

void process61()
{
	fuhao.push("ConditionalStm");
	currentP = new Node("");
	currentP->type = 11;
	Node **t = yufashu.top();
	yufashu.pop();
	*t = currentP;
	yufashu.push(&((*t)->brother));
}

void process62()
{
	fuhao.push("LoopStm");
	currentP = new Node("");
	currentP->type = 12;
	Node **t = yufashu.top();
	yufashu.pop();
	*t = currentP;
	yufashu.push(&((*t)->brother));
}

void process63()
{
	fuhao.push("InputStm");
	currentP = new Node("");
	currentP->type = 13;
	Node **t = yufashu.top();
	yufashu.pop();
	*t = currentP;
	yufashu.push(&((*t)->brother));
}

void process64()
{
	fuhao.push("OutputStm");
	currentP = new Node("");
	currentP->type = 14;
	Node **t = yufashu.top();
	yufashu.pop();
	*t = currentP;
	yufashu.push(&((*t)->brother));
}

void process65()
{
	fuhao.push("ReturnStm");
	currentP = new Node("");
	currentP->type = 15;
	Node **t = yufashu.top();
	yufashu.pop();
	*t = currentP;
	yufashu.push(&((*t)->brother));
}

void process66()
{
	fuhao.push("AssCall");
	fuhao.push("id");
	currentP = new Node("");
	currentP->type = 16;
	Node *t = new Node("");
	t->str[0] = token[now].str;
	t->num = token[now].linenum;
	currentP->child[0] = t;
	currentP->str[0] = token[now].str;
	currentP->num = token[now].linenum;
	// cout<<token[now].str<<"已经被处理"<<endl;
	t->exp = "vark";
	t->idnum++;
	// cout<<currentP->child.size()<<endl;
	Node **t1 = yufashu.top();
	yufashu.pop();
	*t1 = currentP;
	yufashu.push(&((*t1)->brother));
}

void process67()
{
	fuhao.push("AssignmentRest");
	currentP->type = 16;
}

void process68()
{
	fuhao.push("CallStmRest");
	// 标志符变量  调用的
	currentP->type = 17;
	currentP->exp = "opk";
}

void process69()
{
	fuhao.push("Exp");
	fuhao.push(":=");
	fuhao.push("VariMore");
	yufashu.push(&currentP->child[1]);
	currentP = currentP->child[0];
	Node *t = new Node("end");
	t->exp = "opk";
	caozuofu.push(t);
}

void process70()
{
	fuhao.push("fi");
	fuhao.push("StmList");
	fuhao.push("else");
	fuhao.push("StmList");
	fuhao.push("then");
	fuhao.push("RelExp");
	fuhao.push("if");
	yufashu.push(&currentP->child[2]);
	yufashu.push(&currentP->child[1]);
	yufashu.push(&currentP->child[0]);
}

void process71()
{
	fuhao.push("endwh");
	fuhao.push("StmList");
	fuhao.push("do");
	fuhao.push("RelExp");
	fuhao.push("while");
	yufashu.push(&currentP->child[1]);
	yufashu.push(&currentP->child[0]);
}

void process72()
{
	fuhao.push(")");
	fuhao.push("Invar");
	fuhao.push("(");
	fuhao.push("read");
}

void process73()
{
	fuhao.push("id");
	currentP->str[0] = token[now].str;
	currentP->num = token[now].linenum;
	currentP->idnum++;
}

void process74()
{
	fuhao.push(")");
	fuhao.push("Exp");
	fuhao.push("(");
	fuhao.push("write");
	yufashu.push(&currentP->child[0]);
	Node *t = new Node("end");
	t->exp = "opk";
	caozuofu.push(t);
}

void process75()
{
	fuhao.push(")");
	fuhao.push("Exp");
	fuhao.push("(");
	fuhao.push("return");
}

void process76()
{
	fuhao.push(")");
	fuhao.push("ActParamList");
	fuhao.push("(");
	yufashu.push(&currentP->child[0]);
}

void process77()
{
	yufashu.pop();
}

void process78()
{
	fuhao.push("ActParamMore");
	fuhao.push("Exp");
	Node *t = new Node("end");
	t->exp = "opk";
	caozuofu.push(t);
}

void process79()
{
}

void process80()
{
	fuhao.push("ActParamList");
	fuhao.push(",");

	yufashu.push(&currentP->brother);
}

int getExpResult = 1;

void process81()
{
	fuhao.push("OtherRelE");
	fuhao.push("Exp");
	Node *t = new Node("end");
	t->exp = "opk";
	caozuofu.push(t);
	getExpResult = 0;
}

void process82()
{
	fuhao.push("Exp");
	fuhao.push("CmpOp");
	currentP = new Node("");
	currentP->str[0] = token[now].str;
	currentP->num = token[now].linenum;
	currentP->type = OPK;
	currentP->exp = "opk";
	// cout << caozuofu.size() << endl;
	// cout << caozuofu.top()->str[0] << endl;
	while (priosity(caozuofu.top()) >= priosity(currentP))
	{
		Node *t = caozuofu.top();
		caozuofu.pop();
		Node *Rnum = caozuoshu.top(); // 右操作数
		caozuoshu.pop();
		Node *Lnum = caozuoshu.top(); // 左操作数
		caozuoshu.pop();
		t->child[0] = Lnum;
		t->child[1] = Rnum;
		caozuoshu.push(t);
	}
	caozuofu.push(currentP);
	getExpResult = 1;
}

void process83()
{
	fuhao.push("OtherTerm");
	fuhao.push("Term");
}
int expflag = 0;
int getExpResult2 = 0;
void process84()
{
	if (token[now].str == ")" && expflag != 0)
	{
		// cout << 12 << endl;
		// cout << caozuofu.size()<<endl;
		while (caozuofu.top()->str[0] != "(")
		{
			Node *t = caozuofu.top();
			caozuofu.pop();
			Node *Rnum = caozuoshu.top();
			caozuoshu.pop();
			Node *Lnum = caozuoshu.top();
			caozuoshu.pop();
			t->child[0] = Lnum;
			t->child[1] = Rnum;
			caozuoshu.push(t);
		}

		caozuofu.pop();
		expflag--;
	}
	else
	{
		// cout << 2 << endl;
		// cout << caozuofu.size() << endl;
		if (getExpResult || getExpResult2)
		{
			while (caozuofu.top()->str[0] != "end")
			{
				// cout << 3 << endl;
				// cout << caozuofu.top()->str << endl;
				// cout << caozuoshu.size() << endl;
				Node *t = caozuofu.top();
				caozuofu.pop();
				Node *Rnum = caozuoshu.top();
				caozuoshu.pop();
				Node *Lnum = caozuoshu.top();
				caozuoshu.pop();
				t->child[0] = Lnum;
				t->child[1] = Rnum;
				caozuoshu.push(t);
				// cout<<"3结束"<<endl;
			}
			caozuofu.pop();
			currentP = caozuoshu.top();
			// currentP->type=
			caozuoshu.pop();
			Node **t = yufashu.top();
			yufashu.pop();
			*t = currentP;
			getExpResult2 = 0;
		}
	}
}

void process85()
{
	fuhao.push("Exp");
	fuhao.push("AddOp");
	currentP = new Node("");
	currentP->exp = "opk";
	currentP->type = OPK;
	currentP->str[0] = token[now].str;
	currentP->num = token[now].linenum;
	while (priosity(caozuofu.top()) >= priosity(currentP))
	{
		Node *t = caozuofu.top();
		caozuofu.pop();
		Node *Rnum = caozuoshu.top();
		caozuoshu.pop();
		Node *Lnum = caozuoshu.top();
		caozuoshu.pop();
		t->child[0] = Lnum;
		t->child[1] = Rnum;
		caozuoshu.push(t);
	}
	caozuofu.push(currentP);
}

void process86()
{
	fuhao.push("OtherFactor");
	fuhao.push("Factor");
}

void process87()
{
}

void process88()
{
	fuhao.push("Term");
	fuhao.push("MultOp");
	currentP = new Node("");
	currentP->type = 21;
	currentP->exp = "opk";
	currentP->str[0] = token[now].str;
	currentP->num = token[now].linenum;
	while (priosity(caozuofu.top()) >= priosity(currentP))
	{
		Node *t = caozuofu.top();
		caozuofu.pop();
		Node *Rnum = caozuoshu.top();
		caozuoshu.pop();
		Node *Lnum = caozuoshu.top();
		caozuoshu.pop();
		t->child[0] = Lnum;
		t->child[1] = Rnum;
		caozuoshu.push(t);
	}
	caozuofu.push(currentP);
}

void process89()
{
	fuhao.push(")");
	fuhao.push("Exp");
	fuhao.push("(");
	currentP = new Node("");
	currentP->str[0] = "(";
	currentP->exp = "opk";
	caozuofu.push(currentP);
	expflag++;
}

void process90()
{
	fuhao.push("intc");
	currentP = new Node("");
	currentP->type = 20;
	currentP->exp = "constk";
	currentP->str[0] = token[now].str;
	currentP->num = token[now].linenum;
	caozuoshu.push(currentP);
}

void process91()
{
	fuhao.push("Variable");
}

void process92()
{
	fuhao.push("VariMore");
	fuhao.push("id");
	currentP = new Node("");
	currentP->str[0] = token[now].str;
	currentP->num = token[now].linenum;
	currentP->idnum++;
	currentP->type = 23;
	caozuoshu.push(currentP);
}

void process93()
{
	currentP->type = 1;
}

void process94()
{
	fuhao.push("]");
	fuhao.push("Exp");
	fuhao.push("[");
	currentP->type = 4;
	yufashu.push(&currentP->child[0]);
	Node *t = new Node("end");
	t->exp = "opk";
	caozuofu.push(t);
	getExpResult2 = 1;
}

void process95()
{
	fuhao.push("FieldVar");
	fuhao.push(".");
	currentP->type = 5;
	yufashu.push(&currentP->child[0]);
}

void process96()
{
	fuhao.push("FieldVarMore");
	fuhao.push("id");
	currentP = new Node("");
	currentP->type = 5;
	currentP->str[0] = token[now].str;
	currentP->num = token[now].linenum;
	Node **t = yufashu.top();
	yufashu.pop();
	*t = currentP;
}

void process97()
{
	currentP->type = 1;
}

void process98()
{
	fuhao.push("]");
	fuhao.push("Exp");
	fuhao.push("[");
	currentP->exp = "opk";
	currentP->type = 4;
	yufashu.push(&currentP->child[0]);
	Node *t = new Node("end");
	caozuofu.push(t);
	getExpResult2 = 1;
}

void process99()
{
	fuhao.push("<");
}

void process100()
{
	fuhao.push(":=");
}

void process101()
{
	fuhao.push("+");
}

void process102()
{
	fuhao.push("-");
}

void process103()
{
	fuhao.push("*");
}

void process104()
{
	fuhao.push("/");
}

void predict(int pnum) // 调用函数
{
	switch (pnum)
	{
	case 1:
		process1();
		break;
	case 2:
		process2();
		break;
	case 3:
		process3();
		break;
	case 4:
		process4();
		break;
	case 5:
		process5();
		break;
	case 6:
		process6();
		break;
	case 7:
		process7();
		break;
	case 8:
		process8();
		break;
	case 9:
		process9();
		break;
	case 10:
		process10();
		break;
	case 11:
		process11();
		break;
	case 12:
		process12();
		break;
	case 13:
		process13();
		break;
	case 14:
		process14();
		break;
	case 15:
		process15();
		break;
	case 16:
		process16();
		break;
	case 17:
		process17();
		break;
	case 18:
		process18();
		break;
	case 19:
		process19();
		break;
	case 20:
		process20();
		break;
	case 21:
		process21();
		break;
	case 22:
		process22();
		break;
	case 23:
		process23();
		break;
	case 24:
		process24();
		break;
	case 25:
		process25();
		break;
	case 26:
		process26();
		break;
	case 27:
		process27();
		break;
	case 28:
		process28();
		break;
	case 29:
		process29();
		break;
	case 30:
		process30();
		break;
	case 31:
		process31();
		break;
	case 32:
		process32();
		break;
	case 33:
		process33();
		break;
	case 34:
		process34();
		break;
	case 35:
		process35();
		break;
	case 36:
		process36();
		break;
	case 37:
		process37();
		break;
	case 38:
		process38();
		break;
	case 39:
		process39();
		break;
	case 40:
		process40();
		break;
	case 41:
		process41();
		break;
	case 42:
		process42();
		break;
	case 43:
		process43();
		break;
	case 44:
		process44();
		break;
	case 45:
		process45();
		break;
	case 46:
		process46();
		break;
	case 47:
		process47();
		break;
	case 48:
		process48();
		break;
	case 49:
		process49();
		break;
	case 50:
		process50();
		break;
	case 51:
		process51();
		break;
	case 52:
		process52();
		break;
	case 53:
		process53();
		break;
	case 54:
		process54();
		break;
	case 55:
		process55();
		break;
	case 56:
		process56();
		break;
	case 57:
		process57();
		break;
	case 58:
		process58();
		break;
	case 59:
		process59();
		break;
	case 60:
		process60();
		break;
	case 61:
		process61();
		break;
	case 62:
		process62();
		break;
	case 63:
		process63();
		break;
	case 64:
		process64();
		break;
	case 65:
		process65();
		break;
	case 66:
		process66();
		break;
	case 67:
		process67();
		break;
	case 68:
		process68();
		break;
	case 69:
		process69();
		break;
	case 70:
		process70();
		break;
	case 71:
		process71();
		break;
	case 72:
		process72();
		break;
	case 73:
		process73();
		break;
	case 74:
		process74();
		break;
	case 75:
		process75();
		break;
	case 76:
		process76();
		break;
	case 77:
		process77();
		break;
	case 78:
		process78();
		break;
	case 79:
		process79();
		break;
	case 80:
		process80();
		break;
	case 81:
		process81();
		break;
	case 82:
		process82();
		break;
	case 83:
		process83();
		break;
	case 84:
		process84();
		break;
	case 85:
		process85();
		break;
	case 86:
		process86();
		break;
	case 87:
		process87();
		break;
	case 88:
		process88();
		break;
	case 89:
		process89();
		break;
	case 90:
		process90();
		break;
	case 91:
		process91();
		break;
	case 92:
		process92();
		break;
	case 93:
		process93();
		break;
	case 94:
		process94();
		break;
	case 95:
		process95();
		break;
	case 96:
		process96();
		break;
	case 97:
		process97();
		break;
	case 98:
		process98();
		break;
	case 99:
		process99();
		break;
	case 100:
		process100();
		break;
	case 101:
		process101();
		break;
	case 102:
		process102();
		break;
	case 103:
		process103();
		break;
	case 104:
		process104();
		break;
	default:;
		// wrong(token[now].linenum,token[now].str);这里没有在输出错误信息是因为我们在145行左右那里已经在算出来的生成式序号为0的时候输出一次错误信息了
	}
}

/*参数为操作符，类型为单词的词法类型。
对于给定的操作符，此函数返回操作符的优先级，返回类型是整数，规定返回值越大所给操作符的优先级越高。
优先级由高到低排序为:
乘法运算符 > 加法运算符 > 关系运算符 > 栈底标识END*/
int priosity(Node *t) // 判断优先级
{
	string op = t->str[0]; // 将节点 t 中的字符串值的第一个字符赋值给字符串变量 op，以便于后续比较操作符。
	if (op == "end")	   // 如果操作符是 "end"，则返回 -1，表示该操作符不参与优先级比较，因为 "end" 通常表示表达式的结束。
		return -1;
	else if (op == "(")
		return 0;
	else if (op == "<" || op == "=")
		return 1;
	else if (op == "+" || op == "-")
		return 2;
	else if (op == "*" || op == "/")
		return 3;
	else
		return -1; // 出错
}

void printTree(Node *root, int level)
{ // 递归函数，用于打印语法树的节点信息。它接受两个参数：指向语法树根节点的指针 root 和当前节点所在的层级 level。

	unordered_map<int, string> mp;
	// 23种type固定对应的打印结果
	//  1 DecK Idk , 2 IntegerK , 3 CharK , 4 ArrayK , 5 FieldK , 6 PheadK , 7 ProK , 8 TypeK
	//  9 VarK , 10 ProcK , 11 IfK , 12 WhileK , 13 ReadK , 14 WriteK , 15 ReturnK , 16 AssignK
	//  17 CallK , 18 DecK , 19 StmLK , 20 ConstK , 21 OpK , 22 CmpK , 23 Varik
	mp[0] = "Bingo";
	mp[1] = "ExpK Idk"; // 声明的类型
	mp[2] = "DecK IntegerK";
	mp[3] = "DecK CharK";
	mp[4] = "DecK ArrayK";
	mp[5] = "DecK FieldK";
	mp[6] = "PheadK";
	mp[7] = "ProK";
	mp[8] = "TypeK";
	mp[9] = " VarK";
	mp[10] = "ProcK";
	mp[11] = "stmtk IfK";
	mp[12] = "stmtk WhileK";
	mp[13] = "stmtk ReadK";
	mp[14] = "stmtk WriteK";
	mp[15] = "stmtk ReturnK";
	mp[16] = "stmtk AssignK";
	mp[17] = "stmtk CallK";
	mp[18] = "DecK";
	mp[19] = "StmLK";
	mp[20] = "ExpK ConstK";
	mp[21] = "ExpK OpK";
	mp[22] = "CmpK";
	mp[23] = "Varik";
	if (root)
	{
		// 打印一定数量的空格，以表示当前节点所在的层级。
		for (int i = 0; i < level; i++)
			printf("   ");

		/*检查节点的 type1 属性是否为1。
		根据前面的定义，type1 表示参数类型，其中1表示值参。
		因此，如果节点的参数类型为值参，即 type1 为1，则打印 "VarK "。*/
		if (root->type1 == 1)
			cout << "VarK ";

		/*首先检查节点的 type 属性是否既不是18也不是4。
		18表示 DecK 节点，4表示 ArrayK 节点。

		1.DecK 节点（节点类型18）：这些节点表示声明类型，如整型、字符型、数组型、记录型等。
		在语法树中，它们的类型由 type 属性指定。因此，当节点的 type 属性为18时，我们需要根据该属性的值确定节点的实际类型，并打印相应的描述信息。
		2.ArrayK 节点（节点类型4）：这些节点表示数组类型。
		除了节点的 type 属性外，它们还具有一个额外的属性 type2，用于表示数组元素的类型。
		因此，当节点的 type 属性为4时，我们不仅需要打印数组的类型信息，还需要打印数组元素的类型信息。
		如果 type2 不为0，则表示数组的元素类型已经被指定。因此，在打印节点类型信息时，我们需要同时考虑节点的 type 和 type2 属性，并打印相应的描述信息。*/

		// 如果节点既不是 DecK 类型也不是 ArrayK 类型，则从映射表 mp 中根据节点的 type 属性获取对应的节点类型描述信息并打印。
		if (root->type != 18 && root->type != 4)
			cout << mp[root->type] << " ";

		/*如果节点的 type 属性为18或4，则会进入 else 分支。
		在这个分支中，首先检查节点的 type2 属性是否为0。
		type2 表示数组内的元素类型。*/
		else
		{
			if (root->type2 != 0) // 如果 type2 不为0，表示数组的元素类型已经被指定。在打印节点类型信息时，我们需要同时考虑节点的 type 和 type2 属性，并打印相应的描述信息。
				cout << mp[root->type] + " " << mp[root->type2] << " ";
			else // 否则，只打印节点的 type 属性对应的节点类型描述信息。
				cout << mp[root->type] << " ";
		}

		// 声明类型
		if (root->type == 1)
		{ /*type == 1时，即当节点的类型为 IdK 时，表示这是一个标识符节点，通常用于表示变量名或其他命名实体。
		  此时，需要根据节点中的标识符个数和具体情况进行打印处理。*/
			if (root->idnum > 2)
			{ /*检查节点中的标识符个数是否大于 2。
			  如果大于 2，则意味着节点中包含多个标识符，可能是用逗号分隔的形式。
			  在这种情况下，使用 for 循环遍历每个标识符，
			  并根据标识符的位置选择性地添加空格和逗号，以正确地格式化输出标识符。*/
				// idnum是标识符的个数
				for (int i = 0; i < root->idnum; i++)
				{
					if (i == 0)
						cout << root->str[0];
					else if (i == 1)
						cout << " " << root->str[1];
					else
						cout << "," << root->str[i];
				}
			}
			else /*如果标识符个数不大于 2，则进入 else 块。
				   在这种情况下，直接打印第一个和第二个标识符，中间用空格分隔。
				   这是因为如果只有两个标识符节点可能只包含一个标识符和一个附加信息，例如变量的类型。*/
				cout << root->str[0] << " " << root->str[1];
		}

		else if (root->type != 16)
		{ /*检查节点的类型是否不是 AssignK，即不是赋值语句类型。
		  这是因为赋值语句的标识符和表达式部分需要特殊处理，不应该在此处打印。
		  如果节点不是赋值语句类型，则进入条件语句块中进行处理。*/
			if (root->idnum > 1)
			{ // 对于非声明的其他类型的语句，通常只包含一个标识符，因此只需要检查标识符的个数是否大于 1，如果是，则逐个打印并用逗号分隔。
				for (int i = 0; i < root->idnum; i++)
				{
					if (i == 0)
						cout << root->str[0];
					else
						cout << "," << root->str[i];
				}
			}
			else
				cout << root->str[0];
		}

		// 数组类型
		// 如果当前节点是数组类型节点且下界和上界不为0，则打印下界和上界信息；否则直接换行。
		if (root->type == 4 && !(root->low == 0 && root->high == 0))
		{
			cout << " of ";
			cout << "low=" << root->low << " high=" << root->high << endl;
		}
		else
		{
			cout << endl;
		}

		// 递归打印
		// 深度优先地遍历整个语法树，从根节点开始，逐层打印节点信息，直到叶子节点。
		for (int i = 0; i < 3; i++)
		{							  // for 循环遍历当前节点的子节点数组 root->child，数组长度为 3，因为每个节点最多有三个子节点。
			Node *t = root->child[i]; // t指向各个子节点
			// cout<<t<<endl;
			// cout<<level;
			if (!t) // 对于每个子节点 t，首先检查是否为空。如果子节点为空，即指针为 nullptr，则跳过当前循环，继续处理下一个子节点。
				continue;
			cout << i << " "; // 如果子节点不为空，则先打印节点的索引 i（从0开始），然后调用 printTree 函数递归地打印该子节点的信息，同时将层级 level 加 1。
			printTree(t, level + 1);
			/*在处理完当前子节点后，通过 while 循环遍历该子节点的兄弟节点链表。
			如果兄弟节点存在（即 t->brother 不为空），则依次打印连字符 -，以及兄弟节点的信息，并递归地调用 printTree 函数打印兄弟节点的子节点信息。*/
			while (t->brother)
			{
				cout << "-"
					 << " ";
				printTree(t->brother, level + 1);
				t = t->brother;
			}
		}
	}
}
