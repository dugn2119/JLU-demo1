#include "./mideCode.h"

// 新建一个临时变量，返回一个ARG结构
ARG *NewTemp(accesskind access)
{
	ARG *tmp = new ARG();
	tmp->form = AddrForm;
	tmp->Addr.name = to_string(tmp_num);
	tmp->Addr.dataLevel = -1;
	tmp->Addr.dataOff = tmp_num;
	tmp->Addr.access = access;
	tmp_num++;
	return tmp;
}

// 新建数值类 ARG 结构
ARG *ARGValue(int value)
{
	cout << "ARGValue" << endl;
	ARG *tmp = new ARG();
	tmp->form = ValueForm;
	tmp->value = value;
	return tmp;
}

// 产生一个新的标号， 标号值Label加1，并返回新的标号
int NewLabel()
{
	cout << "NewLabel" << endl;
	label_num++;
	return label_num;
}

// 新建标号类 ARG 结构
ARG *ARGLabel(int label)
{
	cout << "ARGLabel" << endl;
	ARG *tmp = new ARG();
	tmp->form = LabelForm;
	tmp->label = label;
	return tmp;
}

// 创建地址类 ARG 结构
ARG *ARGAddr(string id, int level, int off, accesskind access)
{ // 修改string
	cout << "ARGAddr" << endl;
	ARG *tmp = new ARG();
	tmp->form = AddrForm;
	tmp->Addr.name = id;
	tmp->Addr.dataLevel = level;
	tmp->Addr.dataOff = off;
	tmp->Addr.access = access;
	return tmp;
}

// 生成中间代码，加入到中间代码链表中
CodeFile *GenCode(CodeKind codekind, ARG *Arg1, ARG *Arg2, ARG *Arg3)
{
	cout << "GenCode" << endl;
	CodeFile *c = new CodeFile(); // 中间代码链表
	if (head == NULL)
	{
		head = tail = c;
		c->former = c->next = NULL;
	}
	else
	{
		c->former = tail;
		tail->next = c;
		c->next = NULL;
		tail = c;
	}
	c->onecode = new MidCode();
	c->onecode->codekind = codekind;
	c->onecode->arg1 = Arg1;
	c->onecode->arg2 = Arg2;
	c->onecode->arg3 = Arg3;
	return c;
}
// 打印四元式核心部分；
void PrintCodeCore(CodeFile *firstCode,  string codes[22], int index)
{
	if (!firstCode) return;
	cout << index << " ";
	cout << "(";
	if (firstCode->onecode->codekind >= 0 && firstCode->onecode->codekind <= 21)
	{ // 输出中间代码
		cout << codes[firstCode->onecode->codekind];
	}
	else
	{
		cout << "error";
	}

	cout << ", ";
	// 输出arg1,arg2,arg3
	if (firstCode->onecode->arg1 != NULL)
	{
		switch (firstCode->onecode->arg1->form)
		{
		case ValueForm: // 数值类
			cout << firstCode->onecode->arg1->value;
			break;
		case LabelForm: // 标号类
			cout << firstCode->onecode->arg1->label;
			break;
		case AddrForm: //
			if (firstCode->onecode->arg1->Addr.name[0] >= '0' && firstCode->onecode->arg1->Addr.name[0] <= '9')
				cout << "temp" << firstCode->onecode->arg1->Addr.name;
			else
			{
				cout << firstCode->onecode->arg1->Addr.name;
			}
			break;
		}
	}
	else
	{
		cout << "_";
	}
	cout << ", ";

	if (firstCode->onecode->arg2 != NULL)
	{
		switch (firstCode->onecode->arg2->form)
		{
		case ValueForm:
			cout << firstCode->onecode->arg2->value;
			break;
		case LabelForm:
			cout << firstCode->onecode->arg2->label;
			break;
		case AddrForm:
			if (firstCode->onecode->arg2->Addr.name[0] >= '0' && firstCode->onecode->arg2->Addr.name[0] <= '9')
				cout << "temp" << firstCode->onecode->arg2->Addr.name;
			else
			{
				cout << firstCode->onecode->arg2->Addr.name;
			}
			break;
		}
	}
	else
	{
		cout << "_";
	}
	cout << ", ";

	if (firstCode->onecode->arg3 != NULL)
	{
		switch (firstCode->onecode->arg3->form)
		{
		case ValueForm:
			cout << firstCode->onecode->arg3->value;
			break;
		case LabelForm:
			cout << firstCode->onecode->arg3->label;
			break;
		case AddrForm:
			if (firstCode->onecode->arg3->Addr.name[0] >= '0' && firstCode->onecode->arg3->Addr.name[0] <= '9')
				cout << "temp" << firstCode->onecode->arg3->Addr.name;
			else
			{
				cout << firstCode->onecode->arg3->Addr.name;
			}
		}
	}
	else
	{
		cout << "_";
	}
	cout << ")" << endl;
}
// 输出中间代码
void PrintMidCode(CodeFile *firstCode)
{
	int index = 0;
	cout << "PrintMidCode" << endl;
	string codes[22] = {"ADD", "SUB", "MULT", "DIV", "EQC", "LTC", "READC", "WRITEC", "RETURNC", "ASSIG", "AADD", "LABEL", "JUMP", "JUMP0", "CALL", "VARACT", "VALACT", "PENTRY", "ENDPROC", "MENTRY", "WHILESTART", "ENDWHILE"};
	while (firstCode)
	{
		PrintCodeCore(firstCode,codes, index++);
		firstCode = firstCode->next;
	}
}

//对比输出中间代码
void printMidCodeByContrast(CodeFile *head1, CodeFile *head2)
{
	int index = 0;
	cout << "PrintMidCode after opt(l) and before opt(R)" << endl;
	string codes[22] = {"ADD", "SUB", "MULT", "DIV", "EQC", "LTC", "READC", "WRITEC", "RETURNC", "ASSIG", "AADD", "LABEL", "JUMP", "JUMP0", "CALL", "VARACT", "VALACT", "PENTRY", "ENDPROC", "MENTRY", "WHILESTART", "ENDWHILE"};
	while (head1 || head2)
	{
		PrintCodeCore(head1, codes, index);
		printf("                                  ");
		PrintCodeCore(head2, codes, index);
		if (head1) head1 = head1->next;
		if (head2) head2 = head2->next;
		index++;
	}
}