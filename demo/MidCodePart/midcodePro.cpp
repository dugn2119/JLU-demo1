#include "./mideCode.h"
#define Valparamtype 0
#define Varparamtype 1

int tmp_num;		   // 临时变量个数（编号）
int label_num;		   // 标号个数（编号）
CodeFile *head, *tail; // 中间代码序列头尾指针
const int initOff = 7; // 活动记录的初始偏移量

// 中间代码生成主函数
CodeFile *GenMidCode(Node *t)
{
	tmp_num = 1;
	Node *tmp = t;
	tmp = t->child[1];
	// 找到第一个过程声明
	cout << "GenMidCode" << endl;
	while (tmp && tmp->type != PROCK)
	{
		tmp = tmp->brother;
	}

	while (tmp != NULL)
	{
		GenProcDec(tmp);	// 生成过程声明中间代码
		tmp = tmp->brother; // 下一个过程
	}

	// 生成入口代码
	tmp = t->child[2];
	ARG *arg2 = ARGValue(initOff);
	ARG *arg3 = ARGValue(12); // 偏移量
	CodeFile *c = GenCode(MENTRY, NULL, arg2, arg3);
	tmp_num++; // 活动记录第一个临时变量的偏移
	GenBody(tmp->child[0]);
	return head;
}

// 过程声明中间代码生成函数
void GenProcDec(Node *t)
{
	cout << "GenProcDec" << endl;
	int ProcEny = NewLabel();							   // 生成过程入口
	t->entry->attrir.procattr.code = ProcEny;			   // 过程入口地址，目标代码有用
	ARG *arg1 = ARGValue(ProcEny);						   // 入口
	ARG *arg2 = ARGValue(t->entry->attrir.procattr.level); // 层数
	ARG *arg3 = ARGValue(t->entry->attrir.procattr.code);  // 偏移量

	// 处理过程声明，嵌套过程
	Node *tmp = t->child[1];
	while (tmp && tmp->type != PROCK)
		tmp = tmp->brother;
	if (tmp != NULL)
		tmp = tmp->child[0];
	while (tmp != NULL)
	{
		GenProcDec(tmp);
		tmp = tmp->brother; // 下一个过程
	}
	CodeFile *c = GenCode(PENTRY, arg1, arg2, arg3); // 生成过程入口中间代码
	GenBody(t->child[2]->child[0]);					 // 生成过程体中间代码

	CodeFile *b = GenCode(ENDPROC, NULL, NULL, NULL);
}

// 循环语句中间代码生成函数
void GenWhileS(Node *t)
{
	cout << "GenWhileS" << endl;
	ARG *InLarg = ARGLabel(NewLabel());
	ARG *OutLarg = ARGLabel(NewLabel());
	GenCode(WHILESTART, InLarg, NULL, NULL);
	ARG *Earg = GenExpr((*t).child[0]);
	GenCode(JUMP0, Earg, OutLarg, NULL);
	GenBody((*t).child[1]);
	GenCode(JUMP, InLarg, NULL, NULL);
	GenCode(ENDWHILE, OutLarg, NULL, NULL);
}

// 条件语句中间代码生成函数
void GenIfS(Node *t)
{
	cout << "GenIfS" << endl;
	ARG *ElseLarg = ARGLabel(NewLabel());
	ARG *OutLarg = ARGLabel(NewLabel());
	ARG *Earg = GenExpr((*t).child[0]);
	GenCode(JUMP0, Earg, ElseLarg, NULL);
	GenBody((*t).child[1]);
	GenCode(JUMP, OutLarg, NULL, NULL);
	GenCode(LABEL, ElseLarg, NULL, NULL);
	GenBody((*t).child[2]);
	GenCode(LABEL, OutLarg, NULL, NULL);
}

// 写语句中间代码生成函数
void GenWriteS(Node *t)
{
	cout << "GenWriteS" << endl;
	ARG *Earg = GenExpr((*t).child[0]);
	GenCode(WRITEC, Earg, NULL, NULL);
}

// 读语句中间代码生成函数
void GenReadS(Node *t)
{
	cout << "GenReadS" << endl;
	// PrintMidCode(head);
	symbtable *s = (*t).entry;
	ARG *Varg = nullptr;
	if (s->attrir.varattr.access == "indir")
	{
		Varg = ARGAddr(t->str[0], s->attrir.varattr.level, s->attrir.varattr.off, accesskind::dir);
	}
	else
	{
		Varg = ARGAddr(t->str[0], s->attrir.varattr.level, s->attrir.varattr.off, accesskind::indir);
	}
	//-------------------------------------
	// debug
	// cout << Varg->Addr.name << " "
	// 	 << Varg->form << " "
	// 	 << Varg->Addr.dataLevel << " "
	// 	 << Varg->Addr.dataOff << " "
	// 	 << Varg->Addr.access << endl;
	//-------------------------------------
	GenCode(READC, Varg, NULL, NULL);
}

// 过程调用语句中间代码生成函数
void GenCallS(Node *t)
{
	cout << "GenCallS" << endl;
	int EntryAddr = t->entry->attrir.procattr.code;	 // 过程入口地址
	paramtable *p = t->entry->attrir.procattr.param; // 参数表(行参)
	Node *tmp = t->child[0];						 // 实参表
	//-------------------------------------
	// for debug
	// print info of tmp
	// cout << tmp->str[0] << " "
	// 	 << tmp->type << " "
	// 	 << tmp->type1 << " "
	// 	 << tmp->exp << endl;
	//-------------------------------------

	while (tmp != NULL)
	{
		ARG *Earg = GenExpr(tmp); // 实参
		ARG *Rarg = NULL;		  // 参数
		Rarg = ARGValue(p->entry->attrir.varattr.off);
		if (tmp->type1 == Valparamtype)
			GenCode(VALACT, Earg, Rarg, NULL);
		else
			GenCode(VARACT, Earg, Rarg, NULL);
		p = p->next;		// 下一个参数
		tmp = tmp->brother; // 下一个参数
	}

	ARG *labelarg = ARGLabel(EntryAddr); // 入口
	ARG *pOff = ARGValue(1);			 // 过程偏移量
	GenCode(CALL, labelarg, NULL, pOff); //????????????????
}

// 表达式的中间代码生成函数
ARG *GenExpr(Node *t)
{
	cout << "GenExpr" << endl;
	ARG *arg = NULL;
	if (!t)
		return NULL;
	if ((*t).type == CONSTK || (*t).type == VARK || (*t).type == OPK || (*t).type == IDK)
	{
		if ((*t).exp == "vark" || (*t).exp == "") // vark
		{
			arg = GenVar(t);
		}
		else if ((*t).exp == "constk")
		{
			arg = ARGValue(atoi((*t).str[0].c_str()));
		}
		else if ((*t).exp == "opk")
		{
			//-------------------------------------
			// debug
			// cout << "---opk---" << endl;
			// cout << (*t).child[0]->str[0] << " "
			// 	 << (*t).child[0]->type << " "
			// 	 << (*t).child[0]->exp << endl;
			//-------------------------------------
			ARG *Larg = GenExpr((*t).child[0]);

			CodeKind op;
			string opp = (*t).str[0]; // ADD, SUB, MULT, DIV,EQC, LTC,
			//-------------------------------------
			// for debug
			// printf("opp: %s\n", opp.c_str());
			//-------------------------------------
			if (opp == "+")
				op = ADD;
			else if (opp == "-")
				op = SUB;
			else if (opp == "*")
				op = MULT;
			else if (opp == "/")
				op = DIV;
			else if (opp == "=")
				op = EQC;
			else if (opp == "<")
				op = LTC;
			ARG *Rarg = GenExpr((*t).child[1]);	   // 右操作数，递归处理
			ARG *temp1 = NewTemp(accesskind::dir); // 生成临时变量
			GenCode(op, Larg, Rarg, temp1);
			arg = temp1;
		}
	}
	return arg;
}

// 域成员变量的中间代码生成,处理域成员变量的中间代码生成,V1arg为变量的地址，head为域链表
ARG *GenField(ARG *V1arg, Node *t, fieldchain *head)
{
	cout << "GenField" << endl;
	ARG *offArg = ARGAddr(head->idname, 1, 1, accesskind::dir); // 域的偏移量
	ARG *temp1 = NewTemp(accesskind::dir);						// 生成临时变量
	GenCode(AADD, V1arg, offArg, temp1);
	ARG *FieldV = NULL;
	ARG *Field = NULL;
	if (head->unittype->kind == "arrayTy")
		FieldV = GenArray(temp1, t, t->low, t->high - t->low + 1);
	else
		FieldV = temp1;
	return FieldV;//返回域变量的ARG结构指针Fieldv
}

// 数组变量的中间代码生成函数（处理下标表达式）
ARG *GenArray(ARG *Vlarg, Node *t, int low, int size)
{
	cout << "GenArray" << endl;
	ARG *Earg = GenExpr((*t).child[0]);
	ARG *lowArg = ARGValue(low);
	ARG *sizeArg = ARGValue(size);
	ARG *temp1 = NewTemp(accesskind::dir);
	ARG *temp2 = NewTemp(accesskind::dir);
	ARG *temp3 = NewTemp(accesskind::dir);
	GenCode(SUB, Earg, lowArg, temp1);
	GenCode(MULT, temp1, sizeArg, temp2);
	GenCode(AADD, Vlarg, temp2, temp3);
	return temp3;
}

// 变量中间代码生成函数
ARG *GenVar(Node *t)
{
	cout << "GenVar" << endl;
	// PrintMidCode(head);
	string id = t->str[0];
	// cout << id << endl;
	if (t->entry == NULL)
	{
		cout << "error" << endl;
		exit(0);
	}
	accesskind acc = (accesskind)(t->entry->attrir.varattr.access != "dir"); // 访问方式																	 // cout << acc << endl
	int lev = t->entry->attrir.varattr.level;								 // 层次
	int off = t->entry->attrir.varattr.off;									 // 偏移量
	ARG *Vlarg = ARGAddr(id, lev, off, acc);
	ARG *Varg = NULL;

	if (t->type == IDK) // idk
	{
		Varg = Vlarg;
	}
	else if (t->type == ARRAYK) // arrayk
	{
		int _size = t->entry->attrir.idtype->arrayattr.elemty->size; // ？？？？？
		int low = t->low;											 // 下界
		Varg = GenArray(Vlarg, t->child[0], low, _size);
	}
	else if (t->type == 5)
	{
		fieldchain *ff = t->entry->attrir.idtype->body; // 是否是t
		Varg = GenField(Vlarg, t->child[0], ff);
	}
	return Varg;
}

// 赋值语句中间代码生成函数
void GenAssignS(Node *t)
{
	cout << "GenAssignS" << endl;
	ARG *Larg = GenVar((*t).child[0]);
	ARG *Rarg = GenExpr((*t).child[1]);
	//-------------------------------------
	// for debug
	// cout << Larg->Addr.name << " "
	// 	 << Larg->form << " "
	// 	 << Larg->Addr.dataLevel << " "
	// 	 << Larg->Addr.dataOff << " "
	// 	 << Larg->Addr.access << endl;
	//-------------------------------------
	GenCode(ASSIG, Larg, Rarg, NULL);
}
// 语句的中间代码生成函数(错误情况不会处理)
void GenStatement(Node *t)
{
	cout << "GenStatement" << endl;
	if ((*t).type == ASSIGNK)
		GenAssignS(t);
	else if ((*t).type == CALLK)
		GenCallS(t);
	else if ((*t).type == READK)
		GenReadS(t);
	else if ((*t).type == WRITEK)
		GenWriteS(t);
	else if ((*t).type == IFK)
		GenIfS(t);
	else if ((*t).type == WHILEK)
		GenWhileS(t);
	else if ((*t).type == RETURNK)
		GenCode(RETURNC, NULL, NULL, NULL);
	else
	{
		// error
	}
}

// 语句体中间代码生成函数
void GenBody(Node *t)
{
	cout << "GenBody" << endl;
	Node *tmp = t;
	while (tmp)
	{
		GenStatement(tmp);
		tmp = (*tmp).brother;
	}
}
