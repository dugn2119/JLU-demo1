#include "./Semantic.h"

int level = 0;				  // 层数
int off = 0;				  // 偏移量
int offtemp = 0;			  // 临时偏移量

//-----------------------------------------------------------------------------------------------
// 符号栈操作
//-----------------------------------------------------------------------------------------------

// 创建一个符号表
void createtable()
{
	level++;
	offtemp = off;
	off = 0;
}

// 撤销一个符号表
void destroytable()
{
	scope[level] = nullptr;
	off = offtemp;
	level = level - 1;
}

// 在当前层查找 currentlevel当前层
int searchonetable(string id, int currentlevel, symbtable **entry)
{//在函数内部， entry 参数是一个指向指针的指针，它指向了一个 symbtable* 类型的指针，即指向符号表节点的指针。通过传递 entry 的地址，可以在函数内部修改它指向的内容，从而实现在函数外部获取函数内部的修改。
	symbtable *head = scope[currentlevel];
	while (head)
	{
		if (head->idname == id)
		{
			*entry = head;
			return 1; // 找到了
		}
		head = head->next;
	}
	return 0; // 没找到
}

// flag = one或者total。如果是 "total"，则表示在所有作用域中查找；如果是 "one"，则表示仅在当前作用域中查找。
int findentry(string id, string flag, symbtable **entry)
{//函数的作用是在符号表中查找指定标识符的条目，并根据需要将指向该条目的指针存储到 entry 所指向的地址中，返回该条目的指针。
	int currentlevel = level;//保存当前所在的作用域层级，初始化为全局变量 level 的值。这个值表示了当前符号表的层级
	// cout << "准备search" << endl;
	// cout << "第" << level << "层" << endl;
	int present = searchonetable(id, currentlevel, entry);//在当前层级（currentlevel）中查找指定标识符的条目。这个函数返回一个表示是否找到的标志，存储在 present 变量中，并根据需要将指向找到的条目的指针存储到 entry 所指向的地址中。
	// cout << "结束search" << endl;
	if (flag == "total")
	{/*如果 flag 参数的值为 "total"，表示需要在所有作用域中查找标识符的条目。
	 此时，通过一个循环来逐层向上查找，直到找到为止。
	 在每一层上调用 searchonetable 函数查找指定标识符的条目，如果找到了，则更新 present 变量。*/
		while (!present && currentlevel > 1)
		{
			currentlevel--;
			present = searchonetable(id, currentlevel, entry);
		}
	}
	return present;
}

// 登记标识符和属性到符号表
int enter(string id, attributeir *attribp, symbtable **entry)
{//函数的作用是向符号表中添加新的标识符和相关属性，并在需要时返回对应的符号表条目指针。
	//	cout << "想找" << id << endl;
	int present = findentry(id, "one", entry);
	// cout << "有无" << present << endl;
	if (!present)
	{
		// cout << attribp->kind << endl;
		symbtable *node = new symbtable(id, attribp);

		// 判断类型，根据 attribp->kind 判断标识符的类型，分别处理不同类型的标识符。
		if (attribp->kind == 0) // typeking 类型标识符
		{
			node->attrir.varattr.level = level;
		}
		else if (attribp->kind == 1) // varkind 变量标识符
		{
			node->attrir.varattr.level = level;
			node->attrir.varattr.off = off; // 偏移量 
		}
		else if (attribp->kind == 2) // prockind 过程标识符
		{
			node->attrir.varattr.level = level;
		}

		// cout << "准备插入" << endl;
		//  insert into the table
		if (scope[level] == nullptr)
		{//如果当前作用域的符号表为空，直接将 node 赋值给 scope[level]
			scope[level] = node;
			node->next = nullptr;
		}
		else
		{////如果不为空，则遍历链表找到最后一个节点，将 node 插入到链表的末尾。
			symbtable *p = scope[level];
			while (p->next)
			{
				p = p->next;
			}
			p->next = node;
			node->next = nullptr;
		}
		*entry = node; // 返回插入的节点到entry
		
		// printsymbtable();
		return 1;
	}
	else
	{//如果发现重复定义的情况，输出错误信息并返回 0，表示添加失败。
		cout << "wrong!!! : " << id << "重复定义" << endl;
		return 0;
	}
}

// 查找域
int findfield(string id, fieldchain *head, fieldchain **entry)
{/*id: 要查找的标识符的名称。
head: 指向记录域表头部的指针。
entry: 一个指向指针的指针，用于在找到匹配的域时将指向该域的指针存储到 entry 所指向的地址中。*/
	while (head)
	{//从链表头部开始，逐个检查每个域的标识符是否与输入的 id 匹配。
		if (head->idname == id)
		{/*通过比较当前域的 idname 和输入的 id 来检查是否找到了匹配的域。
		 如果找到了，则将指向该域的指针存储到 entry 所指向的地址中，并返回 1 表示成功找到。*/
			*entry = head;
			return 1;
		}
		head = head->next;
	}
	return 0;//如果在遍历完整个链表后仍未找到匹配的域，则返回 0 表示未找到。
}

void printsymbtable()
{
	cout << "**********" << endl;
	cout << "打印符号表 (￣▽￣)\n"
		 << endl;
	for (int i = 0; i <= level; i++)
	{//循环遍历符号表 scope 中的每一层（从全局作用域到当前作用域）。
		symbtable *t = scope[i];
		while (t != nullptr)
		{//在循环中，对于每一层，从作用域链表头部开始遍历，依次输出每个符号表条目的信息。
			if (t->attrir.kind == 0)//如果是类型标识符（kind 为 0），则输出标识符名称和类型为 "typekind"，以及所在层级。
				cout << t->idname << " 类型： "
					 << "typekind "
					 << " 层数: " << t->attrir.varattr.level << endl;
			else if (t->attrir.kind == 1)
			{//如果是变量标识符（kind 为 1），则根据是否有类型信息输出不同格式的信息：
				if (t->attrir.idtype != nullptr)//如果有类型信息，则输出类型为 "varkind"，以及所在层级、偏移量、元素类型、大小和访问方式；
					cout << t->idname << " 类型: "
						 << "varkind"
						 << " 层数: " << t->attrir.varattr.level << " 偏移量: " << t->attrir.varattr.off << " 元素类型： " << t->attrir.idtype->kind << " 大小： " << t->attrir.idtype->size << " 访问方式: " << t->attrir.varattr.access << endl;
				else//如果没有类型信息，则只输出标识符名称、类型为 "varkind"，以及所在层级。
					cout << t->idname << " 类型: "
						 << "varkind"
						 << " 层数: " << t->attrir.varattr.level << endl;
			}
			else//如果是过程标识符（kind 为 2），则输出标识符名称、类型为 "prockind"，以及所在层级。
				cout << t->idname << " 类型: "
					 << "prockind"
					 << " 层数: " << t->attrir.varattr.level << endl;


				// 打印过程中的符号表
				if (t->attrir.link != nullptr)
					printsymbtablePro(t);//这里我们调用了一个新的函数 printsymbtablePro，用于打印那些定义在外层的大proc中的小proc
				//这是因为在这个过程内部原本可能存在一些定义在它内部的过程，如procA中定义了一个procB，现在我们要对B删除，但不能真正的删除，因为真删除之后就在找不到procB的符号表了，导致之后无法打印procB的符号表
				//所以我们在假删除procB的时候将其位置挂载到了A中，这样我们就可以在打印A的符号表的同时递归的打印B的符号表了
				
			t = t->next;
		}
	}
	cout << "\n打印符号表结束 (´▽｀)" << endl;
	cout << "**********" << endl;
}
// jbo added
void printTab(int level) {
	/*接受一个整数参数 level，用于表示要打印的层级。
	在函数内部，通过一个循环遍历 level 次，每次输出一串短横线，用于表示层级的缩进。
	这样可以使得打印的符号表在不同层级之间有清晰的分隔线。*/
	for (int i = 0; i < level; ++i) {
		cout << "-------------------";
	}
}

void printsymbtablePro(symbtable *t) {
	//用于打印那些定义在外层的大proc中的小proc，被嵌在printsymbtable函数内部
	// get level
	int level = t->attrir.varattr.level;
	printTab(level);
	cout << "proc : " <<  t->idname << endl;
	symbtable *temp = t->attrir.link;
	while (temp != nullptr) {
		printTab(level);
		if (temp->attrir.kind == 0)
			
			cout << temp->idname << " 类型： "
					<< "typekind "
					<< " 层数: " << temp->attrir.varattr.level << endl;
		else if (temp->attrir.kind == 1)
		{
			if (temp->attrir.idtype != nullptr)
				cout << temp->idname << " 类型: "
						<< "varkind"
						<< " 层数: " << temp->attrir.varattr.level << " 偏移量: " << temp->attrir.varattr.off << " 元素类型： " << temp->attrir.idtype->kind << " 大小： " << temp->attrir.idtype->size << " 访问方式: " << temp->attrir.varattr.access << endl;
			else
				cout << temp->idname << " 类型: "
						<< "varkind"
						<< " 层数: " << temp->attrir.varattr.level << endl;
		}
		else{
			cout << temp->idname << " 类型: "
					<< "prockind"
					<< " 层数: " << temp->attrir.varattr.level << endl;
			// 打印过程中的符号表
			printsymbtablePro(temp);
		}
		temp = temp->next;
	}
}

//-----------------------------------------------------------------------------------------------
// 语义分析实现
//-----------------------------------------------------------------------------------------------

// 初始化基本类型内部表示的三个函数
typeir *inttype() // init int type 2
{
	typeir *t = new typeir();
	t->kind = "intty";//设置 t 的 kind 字段为 "intty"，表示整数类型。
	t->size = 1;//设置 t 的 size 字段为 1，表示整数类型的大小为 1。
	return t;
}
typeir *booltype() // init bool type
{
	typeir *t = new typeir();
	t->kind = "boolty";//设置 t 的 kind 字段为 "boolty"，表示布尔类型。
	t->size = 1;//设置 t 的 size 字段为 1，表示布尔类型的大小为 1。
	return t;
}
typeir *chartype() // init char type 3
{
	typeir *t = new typeir();
	t->kind = "charty";//设置 t 的 kind 字段为 "charty"，表示字符类型。
	t->size = 1;//设置 t 的 size 字段为 1，表示字符类型的大小为 1。
	return t;
}

//-----------------------------------------------------------------------------------------------
// 其他类型处理函数
//-----------------------------------------------------------------------------------------------

// 自定义类型处理函数
typeir *nametype(Node *t) // 1
{/*函数是用来根据自定义类型名称从符号表中获取对应的类型内部表示的函数。
 如果找到了，就返回类型的内部表示；
 如果未找到，就输出错误信息并返回空指针。*/
	symbtable **a = new symbtable *();
	int present;
	present = findentry(t->str[0], "one", a);
	/*t->str[0] 是节点 t 中存储的字符串，表示自定义类型的名称。
	"one" 是传递给 findentry 函数的第二个参数，表示查找模式。在这里，它指定了只在当前层级进行查找。
	a 是一个指向指针的指针，用于存储查找到的符号表条目的地址。
	通过调用 findentry 函数，程序尝试在符号表中查找名为 t->str[0] 的符号表条目。
	如果找到了，则将指向该条目的指针存储到 a 所指向的地址中，并返回 1 表示找到了自定义类型。
	如果未找到，则返回 0 表示未找到对应的条目。*/
	if (present) // 找到了说明这个自定义类型存在
	{//将指向该条目的指针存储到 a 所指向的地址中，并返回 1 表示找到了自定义类型。
		t->entry = *a;//将符号表加入到Node节点中
		printf("\n-----------------------------------------------------%s %d\n", t->entry->idname.c_str(), t->entry->attrir.varattr.level);
		return (*a)->attrir.idtype;
	}
	else // 没找到说明这个自定义类型不存在，报错
	{
		printf("第%d行%s错啦错啦,不是typekind\n", t->num, t->str[0].c_str());
		return nullptr;
	}
}

// 数组内部类型处理函数
typeir *arraytype(Node *t) // 4
{
	if (t->low > t->high)
	{//检查数组的下界 t->low 是否大于上界 t->high，如果是，则说明数组的下标越界，输出错误信息并返回空指针。
		printf("第%d行%s数组下标越界错误\n", t->num, t->str[0].c_str());
		return nullptr;
	}
	typeir *b = new typeir();
	b->kind = "arrayty";//设置 b 的 kind 字段为 "arrayty"，表示这是一个数组类型。
	b->arrayattr.indexty = inttype(); // 将数组的下标类型设置为整数类型，这里调用了 inttype() 函数来创建一个整数类型的内部表示，并将其赋值给 b 的 arrayattr.indexty 字段。
	b->arrayattr.elemty = typeprocess(t, t->type2);//处理数组元素的类型，传递给它节点 t 和 t->type2（数组元素类型的子节点），并将返回的类型内部表示赋值给 b 的 arrayattr.elemty 字段。
	b->size = t->high - t->low + 1;//计算数组的大小，即上界减去下界再加上 1，并将结果赋值给 b 的 size 字段。
	return b;
}

// 记录类型处理函数
typeir *recordtype(Node *t) // 5
{
	typeir *c = new typeir();//创建一个新的 typeir 结构体指针 c，用于表示记录类型。
	t = t->child[0];
	fieldchain *head = nullptr;//指向记录类型的域链表的头部
	fieldchain *p = head;//p 用于创建新的域链表节点
	fieldchain *p1 = nullptr;//p1 用于追踪上一个域节点
	int offset = 0;//用于记录域的偏移量，初始化为 0
	while (t) // 循环处理记录中的域
	{
		// cout<<t->idnum<<endl;
		for (int i = 0; i < t->idnum; i++)
		{//遍历节点 t 中存储的每个域的名称
			p = new fieldchain();//为每个域创建一个 fieldchain 结构体节点 p。
			p->idname = t->str[i];//域的名称存储到 p->idname 字段中。
			p->unittype = typeprocess(t, t->type); // 域的类型，内部表示,调用 typeprocess 函数处理域的类型节点 t->type，并将返回的类型内部表示存储到 p->unittype 字段中。
			p->offset = offset;
			offset += p->unittype->size; // 计算当前域的偏移量，并将结果存储到 p->offset 字段中。
			//偏移量为之前处理过的所有域的大小之和。
			// cout<<p->unittype->size<<endl;
			p->next = nullptr;//将新创建的域节点 p 加入到域链表中。
			if (head == nullptr)
			{//如果 head 为空，则将 head 指向当前域节点 p。
				head = p;
			}
			if (p1 != nullptr)
			{//更新 p1 指向新添加的域节点 p。
				p1->next = p;
			}
			p1 = p;
			// cout<<2<<endl;
		}
		t = t->brother;//将 t 移动到下一个兄弟节点，可以继续处理记录类型中的下一个域。
	}
	// cout<<"循环处理结束"<<endl;
	c->body = head;//处理完所有域后，将域链表的头部 head 存储到 c 的 body 字段中，表示记录类型的域链表。
	c->kind = "fieldty";//设置 c 的 kind 字段为 "fieldty"，表示这是一个记录类型。
	return c;//返回指向新创建的 typeir 结构体的指针 c，即记录类型的内部表示。
}

// 类型分析处理函数,处理语法树的当前节点类型，构造当前类型的内部表示，并将其地址返回给Ptr类型内部表示的地址
typeir *typeprocess(Node *t, int num) // 3
{
	// debug
	//  cout << t->str << "在typeprocess处理" << num << "呢" << endl;
	// 根据不同节点类型调用不同的处理函数
	if (num == IDK) // declare type is id 声明的类型是标识符，调用 nametype 函数处理。
		return nametype(t);
	else if (num == ARRAYK) // array 声明的类型是数组类型，调用 arraytype 函数处理。
		return arraytype(t);
	else if (num == FIELDK) // record 声明的类型是记录类型，调用 recordtype 函数处理。
		return recordtype(t);
	else if (num == INTEGERK) // int 声明的类型是整数类型，调用 inttype 函数处理。
		return inttype();
	else if (num == CHARK) // char 声明的类型是字符类型，调用 chartype 函数处理。
		return chartype();
	else // error
	{//num 的值不属于上述任何一种情况，则输出错误信息并返回空指针。
		printf("第%d行有处理不了的类型\n", t->num);
		return nullptr;
	}
}

//-----------------------------------------------------------------------------------------------
//声明部分处理函数
//-----------------------------------------------------------------------------------------------
// 类型声明部分分析处理函数
void typedecpart(Node *t) // 7
{
	// cout << "进来啦" << endl;
	int present = 0;
	attributeir *c = new attributeir();//表示当前处理的类型的属性。
	c->kind = 0;//kind 字段设置为 0，表示类型种类为 typekind。
	while (t)
	{//遍历语法树中的每个节点 t，处理每个节点对应的类型声明。
		// cout << t->idnum << endl;
		// cout << t->str[0] <<endl;
		for (int i = 0; i < t->idnum; i++)
		{//对于每个节点 t，遍历其中的每个标识符 t->str[i]，并逐个处理。
			if (i == 0 && t->type == 1)
			{/*如果 当前节点 t 是一个声明类型的节点（t->type == 1），且当前标识符是第一个标识符（i == 0），则跳过当前标识符的处理。
			 因为它代表的是类型名称本身，不需要额外处理。*/
				continue;
			}
			//否则，调用 enter 函数将当前标识符添加到符号表中，并将返回的指向该标识符在符号表中的指针存储在 b 中。
			symbtable **b = new symbtable *();
			present = enter(t->str[i], c, b);
			/*函数 enter 的目的是将一个新的标识符和相关属性添加到符号表中，并在需要时返回对应的符号表条目指针。
			为了能够在函数外部访问到新添加的符号表条目，使用了指向指针的指针 b。
			在函数内部，将新添加的符号表条目的指针存储在 b 所指向的地址中，这样在函数外部就可以通过 *b 来访问到新添加的符号表条目。*/

			// 加入到节点中
			t->entry = *b;//将当前标识符节点的 entry 字段设置为指向符号表中相应条目的指针，以便后续处理中使用。
			printf("\n-----------------------------------------------------%s %d\n", t->entry->idname.c_str(), t->entry->attrir.varattr.level);

			if (present != 1)
			{//如果标识符已经存在于符号表中（present != 1），则输出重复声明类型的错误信息。
				printf("第%d行%s重复声明类型 o(^_-)O\n", t->num, t->str[i].c_str());
				present = 0;
			}
			else
			{//否则，调用 typeprocess 函数处理当前标识符节点的类型，将得到的类型内部表示的指针存储在符号表条目的 idtype 字段中。
				// cout<<"继续"<<endl;
				(*b)->attrir.idtype = typeprocess(t, t->type);
			}
			// printsymbtable();
		}
		t = t->brother;//继续处理下一个兄弟节点，直到所有节点都处理完毕。
	}
}

// 变量声明部分分析处理函数
void vardeclist(Node *t) // 8
{
	// printf("\n\n8开始\n");
	int present = 0;
	attributeir *c = new attributeir(); // 创建新的标识符，表示变量类型
	c->kind = 1;						// 变量类型 vartype  kind 成员设置为 1，表示这是一个变量类型。
	while (t)
	{//遍历语法树中的每个节点 t，处理每个节点对应的类型声明。
		for (int i = 0; i < t->idnum; i++)
		{//对于每个节点 t，遍历其中的每个标识符 t->str[i]，并逐个处理。
			if (i == 0 && t->type == 1)
			{/*如果 当前节点 t 是一个声明类型的节点（t->type == 1），且当前标识符是第一个标识符（i == 0），则跳过当前标识符的处理。
			 因为它代表的是类型名称本身，不需要额外处理。*/
				continue;
			}
			// 找找有没有重复定义
			symbtable **b = new symbtable *();
			present = enter(t->str[i], c, b);//调用 enter 函数，将当前标识符、变量类型 c 和指向 symbtable 指针的指针 b 作为参数传递给 enter 函数，以将标识符添加到符号表中。
			if (present != 1)
			{//如果 enter 函数返回值不为 1，则表示当前标识符已经被定义过，输出重复定义的错误信息。
				// cout<<t->str<<endl;
				printf("第%d行%s重复定义 o(^_-)O\n", t->num, t->str[i].c_str());
				present = 0;
			}
			else // 没有重复定义。将当前标识符的类型信息通过 typeprocess 函数处理，并将处理结果存储到符号表中。
			{
				(*b)->attrir.idtype = typeprocess(t, t->type); // 处理 t->type
				(*b)->attrir.varattr.access = "dir";		   // 直接访问 当前标识符的访问方式为直接访问。
				(*b)->attrir.varattr.level = level;			   // 层次 当前标识符的层次为当前层次 level。
				(*b)->attrir.varattr.off = off;				   // 偏移量 偏移量为当前偏移量 off。
				// cout<<1;
				if ((*b)->attrir.idtype)//如果标识符的类型信息存在，则将当前偏移量 off 增加该类型信息的大小。
					off += (*b)->attrir.idtype->size;
				
				// 将当前符号表条目的指针存储到节点 t 的 entry 成员中，以便在后续处理中可以访问到该符号表条目。
				t->entry = *b;//
				// debug
				printf("\n-----------------------------------------------------%s %d\n", t->entry->idname.c_str(), t->entry->attrir.varattr.level);
			}
		}
		t = t->brother;//遍历下一个兄弟节点，重复上述步骤，直到遍历完所有节点。
	}
	// printf("8结束\n\n");
}

// 形式参数部分分析处理函数
paramtable *paradeclist(Node *t) // 11
{
	Node *p;
	paramtable *head = nullptr;
	if (t->child[0])
	{//如果参数表的第一个子节点存在（即参数列表不为空），则进入处理过程
		p = t->child[0];//获取参数表的第一个子节点 p。
		//处理参数表
		vardeclist(p);//处理参数表中的变量声明部分，将参数添加到符号表中。
		symbtable *temp = scope[level];//获取当前层次的符号表头部指针 temp。
		paramtable *current = nullptr;
		paramtable *last = nullptr;
		// paramtable* head=new paramtable();
		while (temp)
		{//遍历符号表中的每个条目 temp：
			current = new paramtable();
			if (!head)
			{//如果 head 为空（即链表还未创建），则将 head 和 last 都指向当前新创建的节点 current。
				head = current;
				last = current;
			}
			current->entry = temp;/*temp 是一个指向符号表中当前条目的指针。
									current->entry 表示当前节点 current 的 entry 成员，即指向符号表条目的指针,即形参类型表的符号表中的位置
									current->entry = temp 将符号表条目指针 temp 赋值给当前节点的 entry 成员，从而使当前节点与符号表中的对应条目建立联系。*/
			current->next = nullptr;//将当前节点的 next 指针置为 nullptr，因为当前节点暂时是链表中的最后一个节点。
			if (last != current)
			{//如果 last 指向的节点不是当前节点 current，说明链表中已经有节点存在，需要将当前节点连接到链表的尾部：
				last->next = current;//将上一个节点 last 的 next 指针指向当前节点 current，实现链表的连接。
				last = current;//更新 last 指针为当前节点 current，因为当前节点成为了链表中的最后一个节点。
			}
			temp = temp->next;//temp 指针指向符号表中的下一个条目，即准备处理下一个符号表条目。
		}
		// destroytable();
		return head;
	}//这样，循环结束后，所有的符号表条目都被转换为了 paramtable 类型的节点，并且这些节点被连接成一个链表。
	else
	{//参数表的第一个子节点不存在（即参数列表为空），直接返回 nullptr。
		// destroytable();
		return nullptr;
	}
}

// 声明头部分析处理函数
symbtable *headprocess(Node *t) // 10
{
	attributeir *c = new attributeir();
	c->kind = 2;//类型设为2，表示这是一个过程或函数的声明。
	symbtable **b = new symbtable *();
	// printf("准备走enter\n");
	enter(t->str[0], c, b);//将过程名和属性信息注册到符号表中。这样，enter 函数会创建一个新的符号表条目，表示这个过程或函数，并将其地址存储在指针 b 指向的位置。

	// symbtable 加入到Node节点中
	t->entry = *b;
	//debug
	printf("\n-----------------------------------------------------%s %d\n", t->entry->idname.c_str(), t->entry->attrir.varattr.level);

	// printf("出来enter\n");
	createtable(); //一个新的过程，创建一个新的符号表，为新的过程或函数的参数列表准备空间。
	//(*b)->attrir.procattr.level = level;
	// cout<<(*b)->attrir.varattr.level<<endl;
	// printsymbtable();
	paramtable *paramt = new paramtable;
	(*b)->attrir.procattr.param = paradeclist(t); //新层过程名形参标识符的属性

	return *b;//函数返回新注册到符号表中的过程或函数的符号表条目的地址。
}

// 过程声明部分分析处理函数
void procdecpart(Node *t) // 9
{
	if (t == nullptr)
		return;
	symbtable **entry = new symbtable *();//用于存储新的过程的符号表条目。
	// printf("准备走headprocess\n");
	*entry = headprocess(t); // 处理过程声明的头部部分，并返回新注册到符号表中的过程的符号表条目的地址。
	// printf("走headprocess结束\n");
	Node *temp = t;//暂存当前节点地址
	// cout << t->child[0]->str << endl;
	t = t->child[1]; // 将节点 t 移动到过程体的位置，准备处理过程体部分。
	while (t)
	{//遍历过程体中的声明内容。
		if (t->type == TYPEK) // 类型声明部分 TypeK
		{
			typedecpart(t->child[0]);
		}
		else if (t->type == VARK) // 变量声明部分 VarK
		{
			// cout << "我在" << t->str << "!!!" << endl;
			vardeclist(t->child[0]);
		}
		else // 过程声明部分 ProcK
		{
			procdecpart(t);
		}

		// 转到下一个兄弟节点，即下一个过程继续处理
		t = t->brother;
	}
	t = temp; //将节点 t 恢复到之前保存的地址，准备处理过程体的语句部分
	// cout << "准备走Body" << endl;
	body(t->child[2]->child[0]);//调用了 body 函数，用于处理过程体中的语句部分。

	//销毁本层符号表之前，将本层符号表加入到过程名标识符中
	(*entry)->attrir.link = scope[level];// 加入到Node节点中
	//销毁本层符号表
	destroytable();
}

//-----------------------------------------------------------------------------------------------
// 语句部分处理函数
//-----------------------------------------------------------------------------------------------
//语句序列分析处理函数,根据语法树节点中的kind 项判断应该转向处理哪个语句类型函数
void statement(Node *t) // 13
{
	// cout << "轮到" << t->str << "是类型" << t->type << endl;
	if (t->type == IFK)//if条件语句
	{
		// cout<<"开始if\n"<<endl;
		ifstatment(t);
	}
	else if (t->type == WHILEK)//循环语句
	{
		whilestatment(t);
	}
	else if (t->type == ASSIGNK)//赋值语句
	{
		assignstatment(t);
		// cout<<t->str[0]<<endl;
	}
	else if (t->type == READK)//输入语句
	{
		// cout << "开始读入\n" << endl;
		readstatment(t);
	}
	else if (t->type == WRITEK)//输出语句
	{
		// cout << "进入" << endl;
		writestatment(t);
	}
	else if (t->type == CALLK)//过程调用语句
	{
		callstatment(t);
	}
	else if (t->type == RETURNK)//返回语句
	{
		returnstatment(t);
	}
	else
	{//如果节点类型不匹配上述任何一种情况，输出错误信息，表示语句类型不明。
		cout << "第" << t->num << "行"
			 << "对应的语句类型不明 (･_･;" << endl;
		// printf("%d\n",t->type);
	}
}

//执行体部分分析处理函数,循环处理语旬序列
void body(Node *t) // 12
{//循环处理语句序列，即对执行体中的每一个语句节点，依次调用 statement 函数进行分析处理。
	//int cnt = 0;
	while (t)
	{
		// cout << "第" << cnt++ << "次" << endl;
		statement(t);//对当前节点 t 调用 statement 函数，进行语句的分析处理。
		t = t->brother;//将当前节点指针移动到下一个兄弟节点，即执行体中的下一个语句节点。
	}
}

// 表达式处理
typeir *expr(Node *t, accesskind *ekind) // 14 //函数接受两个参数，一个是指向当前表达式节点的指针 t，另一个是指向访问类型的指针 ekind，表示该表达式的访问类型。
{
	// cout << "expr:" << t->str<<" "<<t->exp<<endl;
	if (t->exp == "constk")
	{//如果当前节点表示的是常量，即常数节点。
		*ekind = dir;//设置访问类型为直接访问。
		return inttype();//返回一个指向整数类型的内部表示的指针。
	}
	else if (t->exp == "opk")
	{//如果当前节点表示的是运算符节点。
		accesskind *e = new accesskind;//声明一个新的访问类型指针 e。
		typeir *a = expr(t->child[0], e);//当前节点的左子树递归调用 expr 函数，得到左表达式的类型。
		// cout<<a->kind<<endl;
		typeir *b = expr(t->child[1], e);//当前节点的右边子树递归调用 expr 函数，得到右表达式的类型。
		// cout<<b->kind<<endl;
		if (a->kind == b->kind)
		{//如果左右表达式的类型相同，说明是合法的运算符组合。
			*ekind = dir;//设置访问类型为直接访问。
			if (t->str[0] == "-" || t->str[0] == "+" || t->str[0] == "*" || t->str[0] == "/")
				return inttype();//对于常见的加、减、乘、除运算，通常会返回整数类型的结果，因此在这里返回指向整数类型的内部表示的指针。
			else
				return booltype(); //对于其他类型的运算符，比如逻辑运算符或比较运算符，其结果通常是布尔类型，因此在这里返回指向布尔类型的内部表示的指针。
		}
		else
		{//如果左右表达式的类型不同，表示运算符左右不匹配，输出错误信息。
			cout << "第" << t->num << "行"
				 << "运算符左右不匹配" << endl;
			return nullptr;
		}
	}
	else//变量处理部分的算法
	{// 如果当前节点表示的是变量节点或标识符。
		symbtable **entry = new symbtable *();//声明一个指向符号表条目的指针 entry。
		bool flag = findentry(t->str[0], "total", entry);//调用 findentry 函数查找当前标识符在符号表中的位置
		if (flag == false)
		{//如果找不到则输出错误信息。
			cout << "第" << t->num << "行" << t->str << "找不到" << endl;
			return nullptr;
		}

		t->entry = *entry;//将当前节点指向的符号表条目存储到节点中
		printf("\n-----------------------------------------------------%s %d\n", t->entry->idname.c_str(), t->entry->attrir.varattr.level);

		if (t->child[0] == nullptr)//直接变量，如果当前节点没有子节点，则表示是直接变量
		{
			*ekind = indir;//设置访问类型为间接访问
			return (*entry)->attrir.idtype;//并返回该变量的类型。
		}
		else
		{//如果当前节点有子节点，则表示是数组或域：
			// cout<<"我是数组或域"<<endl;
			if ((*entry)->attrir.idtype->kind == "arrayty")
			{//如果当前标识符的类型是数组，则调用 arrayvar 函数处理数组变量。
				return arrayvar(t);
			}
			else if ((*entry)->attrir.idtype->kind == "fieldty")
			{//如果当前标识符的类型是记录，则调用 fieldvar 函数处理记录域。
				return fieldvar(t);
			}
		}
	}
	// printsymbtable();
	return nullptr;//最后，如果没有匹配到任何情况，则返回空指针。
}

// 数组 域处理
typeir *arrayvar(Node *t) // 15
{
	// cout<<"数组处理"<<t->str[0]<<endl;
	symbtable **entry = new symbtable *();
	bool flag = findentry(t->str[0], "total", entry);//调用 findentry 函数查找当前标识符在符号表中的位置。
	// cout<<flag;
	if (!flag)
	{//如果找不到当前标识符对应的符号表条目，则输出错误信息，并返回空指针。
		printf("第%d行数组%s没声明\n", t->num, t->str[0].c_str());
		return nullptr;
	}
	else
	{//如果找到了符号表条目：
		t->entry = *entry;//将当前节点指向的符号表条目存储到节点中，并输出该条目的信息。
		printf("\n-----------------------------------------------------%s %d\n", t->entry->idname.c_str(), t->entry->attrir.varattr.level);
		// cout<<"数组处理"<<t->str[0]<<endl;
		// cout<<*entry<<t->str[0]<<endl;
		// cout<<(*entry)->attrir.idtype<<endl;
		if ((*entry)->attrir.idtype == nullptr)
		{//如果当前标识符的类型为空（即 idtype 为 nullptr），表示该标识符未被声明为数组类型，返回空指针。
			// cout<<"数组处理"<<endl;
			return nullptr;
		}
		else if ((*entry)->attrir.idtype->kind != "arrayty")
		{//如果当前标识符的类型不是数组类型，输出错误信息，并返回空指针。
			printf("第%d行%s不是数组\n", t->num, t->str[0].c_str());
			return nullptr;
		}
		else
		{//如果当前标识符的类型是数组类型：返回数组类型的元素类型。
			return (*entry)->attrir.idtype->arrayattr.elemty;
		}
	}
}

typeir *fieldvar(Node *t) // 16
{
	symbtable **entry = new symbtable *();
	bool flag = findentry(t->str[0], "total", entry);//调用 findentry 函数查找当前标识符在符号表中的位置。
	// cout<<t->str[0]<<endl;
	if (!flag)
	{//找不到当前标识符对应的符号表条目，则输出错误信息，并返回空指针。
		printf("第%d行记录%s没声明\n", t->num, t->str[0].c_str());
		return nullptr;
	}
	else
	{
		if ((*entry)->attrir.idtype->kind != "fieldty")
			printf("第%d行%s不是记录\n", t->num, t->str[0].c_str());
		else
		{//如果找到了符号表条目：
			//寻找域变量
			fieldchain *c = (*entry)->attrir.idtype->body;
			while (c)
			{//使用指针 c 遍历记录类型的域链表 ((*entry)->attrir.idtype->body)。
				if (c->idname == t->child[0]->str[0])//检查当前域的标识符是否与节点 t 中的标识符匹配。
				{//匹配成功，表示找到了对应的域变量，此时返回该域的类型 c->unittype。
					// cout<<"找到了"<<endl;
					return c->unittype;
				}
				c = c->next;//匹配失败，前往下一个
			}
			//遍历完整个域链表都未找到匹配的域变量，说明记录类型中不存在节点 t 中指定的域，输出错误信息
			printf("第%d行记录%s中找不到变量 o(^_-)O\n", t->num, t->str[0].c_str());
			return nullptr;
		}
	}

	t->entry = *entry;//更新节点中的符号表条目:将当前节点指向的符号表条目存储到节点中，并输出该条目的信息。
	printf("\n-----------------------------------------------------%s %d\n", t->entry->idname.c_str(), t->entry->attrir.varattr.level);

	return nullptr;
}

//-----------------------------------------------------------------------------------------------
// 具体语句处理函数
//处理赋值语句
void assignstatment(Node *t) // 17
{
	// cout << "进入assign" << endl;
	//从节点 t 的子节点中分别获取左右两边的表达式，存储在 child1 和 child2 中。
	Node *child1 = t->child[0]; 				// 左边
	// cout<<"1:"<<child1->str[0]<<endl;
	Node *child2 = t->child[1];					 // 右边
	// cout<<"2:"<<child2->str[0]<<endl;
	if (child1->child[0] == nullptr)
	{//如果左边表达式没有子节点（即不是数组或域），则表示左边是一个变量标识符。
		// cout<<1<<child1->str[0]<<endl;
		symbtable **entry = new symbtable *();
		bool flag = findentry(child1->str[0], "total", entry);//在符号表中查找对应的条目。
		if (!flag)
		{//如果找不到对应的条目，则输出错误信息，并返回。
			printf("\n第%d行标识符%s不存在 o(^_-)O\n", t->child[0]->num, t->child[0]->str[0].c_str());
			return;
		}
		if ((*entry)->attrir.kind != 1)
		{//如果找到了条目，但该条目不是变量类型，则输出错误信息，并返回。
			cout << t->str[0] << "(" << t->num << "行)";
			printf("不是变量! （；゜０゜） （；゜０゜） （；゜０゜）\n");
			return;
		}
		child1->entry = *entry;//将当前节点指向的符号表条目存储到节点中，并输出该条目的信息。
		printf("\n-----------------------------------------------------%s %d\n", child1->entry->idname.c_str(), child1->entry->attrir.varattr.level);

	}
	else
	{
		//如果左右两边的表达式是数组或域，使用 arrayvar 或 fieldvar 函数处理，获取对应的类型信息。
		typeir *t1;
		if (child1->type == 4)
		{//左边是数组
			// cout<<"14"<<endl;
			t1 = arrayvar(child1);
		}
		else if (child1->type == 5)
		{//左边是域
			// cout<<"15"<<endl;
			t1 = fieldvar(child1);
		}
		// cout<<"lalala"<<endl;
		typeir *t2;
		if (child2->type == 4)
		{//右边是数组
			// cout<<"24"<<endl;
			t2 = arrayvar(child2);
		}
		else if (child2->type == 5)
		{//右边是域
			// cout<<"25"<<endl;
			t2 = fieldvar(child2);
		}
		else
		{//如果左右两边的表达式不是数组或域，则调用 expr 函数处理，获取表达式的类型信息。
			accesskind *tt = new accesskind();
			t2 = expr(child2, tt);
		}
		// cout<<t1->kind<<" "<<t2->kind<<endl;
		if (!t1 || !t2)
			/*检查了左右两边的表达式是否为空，如果有一边为空，则直接返回，
			因为在此情况下无法进行后续的类型匹配和处理，
			返回可以避免在空指针的情况下进行后续操作，避免程序崩溃或产生未定义行为。*/
			return;
		if (t1->kind != t2->kind)
		{//如果左右两边表达式的类型不匹配，则输出错误信息，并返回。
			cout << "第" << t->num << "行";
			printf("赋值两边类型不匹配\n");
			return;
		}
		else
		{//上述情况外的其他难以预料的情况
			// cout<<"结束了"<<endl;
			return;
		}
	}
	//如果左右两边的表达式不是数组或域，进入这个分支。
	//t3 和 t4 是用来记录赋值语句左右两边的表达式的访问类型的。
	accesskind *t3 = new accesskind;
	accesskind *t4 = new accesskind;
	// cout<<"新的一次"<<endl;
	// cout<<"11"<<child1->str[0]<<endl;
	// cout<<"22"<<child2->str[0]<<endl;
	typeir *t1 = expr(child1, t3);//对赋值语句的左边 child1 进行表达式分析处理，并将结果存储在 t1 中；同时将左边表达式的访问类型记录在 t3 中
	typeir *t2 = expr(child2, t4);//同理，分析赋值号右边
	// cout<<"这里"<<endl;
	if (!t1 || !t2)
	{//检查了左右两边的表达式是否为空，如果有一边为空，则直接返回
		return;
	}
	if (t1->kind != t2->kind)
	{//如果左右两边表达式的类型不同，则输出错误信息，并返回
		// cout<<t1->kind<<" "<<t2->kind<<endl;
		cout << "第" << t->num << "行";
		printf("赋值两边类型不匹配 (･_･;\n");
		return;
	}
	
	// cout << "退出assign" << endl;
}

//处理if语句
void ifstatment(Node *t) // 19
{
	Node *t3 = t->child[0];//if条件 获取 if 语句中的条件部分
	typeir *t2;//声明一个指针用于存储条件部分的表达式类型。
	accesskind *t1 = new accesskind();//声明一个指针用于存储条件部分的访问类型。
	// cout<<t3->type<<" "<<t3->exp<<endl;
	// if(t->exp=="")
	// cout<<"为空";
	// exit(0);
	// cout<<t->exp<<endl;
	t2 = expr(t3, t1);//调用 expr 函数对条件部分进行表达式分析处理，并将结果存储在 t2 中，同时将条件部分的访问类型记录在 t1 中。
	if (t2 == nullptr)
	{//条件表达式的分析结果为空，说明条件表达式存在错误，直接返回。
		// printf("if错误\n");
		return;
	}
	if (t2->kind != "boolty")
	{//条件表达式的类型不是布尔类型，说明条件表达式不符合语法，报错并返回。
		printf("第%d行if错误, noi boolty !!! (^_−)−☆\n", t->num);
		return;
	}
	// cout<<t2->kind<<"!!!"<<endl;
	Node *t4 = t->child[1];// then 语句 获取 if 语句中的 then 分支。
	while (t4)
	{//遍历 then 分支中的语句序列，逐个处理每个语句。
		statement(t4);//根据语法树节点中的kind 项判断应该转向处理哪个语句类型函数
		t4 = t4->brother;//逐个处理每个语句
	}
	Node *t5 = t->child[2]; // else 语句 获取 if 语句中的 else 分支。
	while (t5)
	{//遍历 else 分支中的语句序列，逐个处理每个语句。
		statement(t5);
		t5 = t5->brother;
	}
}


void callstatment(Node *t) //18
{
	symbtable **entry = new symbtable *();//声明一个符号表指针的指针，用于存储函数名对应的符号表条目。
	bool flag = findentry(t->str[0], "total", entry);/*在符号表中查找读取目标。
													 t->str[0] 表示读取目标的名称。
													 "total" 表示搜索整个符号表（全局作用域）。*/
	// cout<<t->str<<endl;
	if (!flag)
	{//未找到函数名对应的符号表条目，则输出错误信息并返回。
		printf("call找不到函数");
		cout << t->str[0] << "(" << t->num << "行)" << endl;
		return;
	}
	else if ((*entry)->attrir.kind != 2)
	{//检查找到的符号表条目是否是过程名。过程名的属性 kind 应该为 2，如果不是，则输出错误信息并返回。
		cout << t->str[0] << "(" << t->num << "行)";
		printf("不是过程名\n");
		return;
	}

	//将找到的符号表条目赋值给语法树节点 t 的 entry 字段，以便后续引用。
	t->entry = *entry;
	printf("\n-----------------------------------------------------%s %d\n", t->entry->idname.c_str(), t->entry->attrir.varattr.level);

	//参数个数匹配
	paramtable *tt = (*entry)->attrir.procattr.param;//获取函数对应的参数表。
	int cnt = 0;//参数个数
	while (tt)
	{//遍历参数表，计算参数的个数，并将结果存储在 cnt 中。
		cnt++;
		tt = tt->next;
	}
	// cout<<cnt<<endl;
	int cnt1 = 0;//cnt1 用于记录函数调用语句中实际传入的参数个数。
	Node *t1 = t->child[0];//获取函数调用语句中参数列表的起始节点
	while (t1)
	{//遍历参数列表，计算实际传入的参数个数，并将结果存储在 cnt1 中。
		cnt1++;
		t1 = t1->brother;
	}
	if (cnt1 != cnt)
	{//检查实际传入的参数个数是否与函数声明的参数个数相匹配，如果不匹配，则输出错误信息并返回。
		cout << "第" << t->num << "行" << t->str[0] << "过程参数个数不匹配 (･_･;" << endl;
		return;
	}
	t1 = t->child[0];//重新将 t1 指向函数调用语句中的参数列表的起始节点。
	tt = (*entry)->attrir.procattr.param;//重新将 tt 指向被调用函数在符号表中的条目中存储的参数表。。
	int cnt2 = 0;//初始化计数器 cnt2，用于记录当前正在检查的参数的位置。
	while (t1)
	{//开始循环，遍历函数调用语句中的参数列表。
		cnt2++;//每次循环迭代，递增计数器 cnt2，表示正在检查的参数位置。
		accesskind *ac = new accesskind();//创建一个新的访问类型指针 ac，用于存储参数表达式的访问类型。
		typeir *a = expr(t1, ac);//调用 expr 函数来解析当前参数节点 t1，并将结果存储在 a 中。
		typeir *b = tt->entry->attrir.idtype;//获取被调用函数参数表中当前位置对应的参数类型，并将其存储在 b 中。
		if (!a || !b)/*检查是否有参数的类型信息为空。
					 如果其中一个参数的类型信息为空，则表示参数解析出错，直接跳过该参数的比较。*/
			continue;
		else
		{
			if (a->kind != b->kind)
			{/*比较实际传递的参数类型 a 与被调用函数声明的参数类型 b 是否相同。
			 如果类型不匹配，则输出错误信息，指示用户在函数调用中哪个位置的参数类型不匹配，并返回。*/
				cout << "第" << t->num << "行" << t->str[0] << "过程第" << cnt2 << "个参数不匹配" << endl;
				return;
			}
		}
		t1 = t1->brother;//移动到下一个参数节点，准备比较下一个参数。
		tt = tt->next;//移动到被调用函数参数表中的下一个参数，准备比较下一个参数的类型。
	}
}

void returnstatment(Node *t)
{
}

//处理循环语句
void whilestatment(Node *t) // 20
{
	Node *t3 = t->child[0];//获取 while 循环语句中的条件部分。
	typeir *t2;//声明一个类型信息指针 t2，用于存储条件表达式的类型信息。
	accesskind *t1 = new accesskind();//创建一个新的访问类型指针 t1，用于存储条件表达式的访问类型。
	t2 = expr(t3, t1);//调用 expr 函数解析条件部分的表达式，并将结果存储在 t2 中。t1 用于传递访问类型信息。
	if (t2 == nullptr)
	{//条件表达式的解析结果为空，说明条件表达式解析出错，直接返回。
		// printf("while错误\n");
		return;
	}
	if (t2->kind != "boolty")
	{//条件表达式的类型不是布尔类型，说明条件表达式的结果不符合 while 循环的要求，输出错误信息并返回。
		printf("第%d行while错误\n", t->num);
		return;
	}
	Node *t4 = t->child[1];//获取 while 循环语句中的循环体部分。
	//int cnt = 0;//声明一个计数器 cnt，用于记录循环执行的次数，调试使用。
	// 循环处理语句序列
	while (t4)
	{//循环遍历处理循环体内的语句序列。条件 t4 表示只要当前语句节点不为空，就会一直执行循环体内的操作。
		statement(t4);//调用 statement 函数处理当前语句节点 t4，执行循环体内的语句。
		// cout<<cnt++<<endl;
		t4 = t4->brother;//移动到下一个兄弟节点，准备处理下一个循环体内的语句。
		// cout<<cnt++<<endl;
	}
}

//处理读取语句
void readstatment(Node *t) // 21
{
	// cout<<1<<endl;
	symbtable **entry = new symbtable *();// 声明一个指向符号表指针的指针 entry，用于存储读取目标在符号表中的条目。
	bool flag = findentry(t->str[0], "total", entry);/*在符号表中查找读取目标。
													 t->str[0] 表示读取目标的名称。
													 "total" 表示搜索整个符号表（全局作用域）。*/
	//检查是否找到了读取目标。即读取目标是否声明。
	if (!flag)
	{//未找到，输出错误信息，指出读取目标未定义。
		printf("read找不到要读的");
		cout << t->str[0] << "(" << t->num << "行)" << endl;
	}
	else if ((*entry)->attrir.kind != 1)
	{//检查读取目标的种类是否为变量。如果不是变量，输出错误信息，指出读取目标不是一个变量。
		cout << t->str[0] << "(" << t->num << "行)";
		printf("不是变量!\n");
	}
	t->entry = *entry;//将找到的符号表条目存储在当前节点 t 的 entry 属性中，方便后续使用。
	//输出读取目标的名称和作用域级别信息。
	printf("\n-----------------------------------------------------%s %d\n", t->entry->idname.c_str(), t->entry->attrir.varattr.level);
}

//处理写入语句
void writestatment(Node *t) // 22
{
	// cout<<"write了"<<endl;
	typeir *t2;//声明一个指向类型信息的指针 t2，用于存储表达式的类型信息。
	accesskind *t1 = new accesskind();//声明一个访问类型的指针 t1，用于存储表达式的访问类型。
	// cout<<"write:"<<t->child[0]->str[0]<<endl;
	t2 = expr(t->child[0], t1);//调用 expr 函数计算表达式 t->child[0] 的类型信息，并将结果存储在 t2 中。同时，将表达式的访问类型存储在 t1 中。
	if(t2 && t2->kind == "boolty")
	{/*检查表达式的类型是否为布尔类型。
	如果是布尔类型，说明写入的内容是一个布尔表达式，输出错误信息，指出写入语句中不能包含布尔类型的表达式。*/
		printf("第%d行write错误 （；゜０゜）\n", t->num);
	}
}

//语义分析总程序，分析语法树并执行相应的语义分析操作
void analyze(Node *currentp) // 1 接受一个指向语法树节点的指针 currentp 作为参数。
{
	// cout << "1" << " " << currentp->str << endl;
	Node *p = currentp->child[1];//将当前节点的第二个子节点赋值给 p，即获取主程序块的节点。
	/*当前节点的第二个子节点被赋值给了变量 p，
	这是因为在语法树的结构中，通常第一个子节点是程序块的标识符或者一些修饰符，
	而第二个子节点则是程序块的实际内容，也就是主程序块。*/
	// cout<<p->str[0]<<endl;
	createtable();//调用 createtable 函数创建符号表。
	// cout << "建表完成***" << endl;
	// cout<<p->str<<endl;
	while (p)
	{//循环遍历主程序块中的子节点。
		// cout << "回到主函数" << endl;
		// cout << p->str[0] << "11 " << p->type << endl;
		if (p->type == TYPEK)
		{//如果当前节点是类型声明部分的节点，则调用 typedecpart 函数进行类型声明的处理。
			// cout<<"type"<<endl;
			typedecpart(p->child[0]);
		}
		else if (p->type == VARK)
		{//如果当前节点是变量声明部分的节点，则调用 vardeclist 函数进行变量声明的处理。
			// cout<<"var"<<endl;
			vardeclist(p->child[0]);
		}
		else
		{//对于其他类型的节点，假定为过程声明部分的节点，然后调用 procdecpart 函数进行过程声明的处理。
			// cout << "我准备去走" << p->child[0]->str[0] << endl;
			procdecpart(p);
		}
		p = p->brother;//将当前节点移动到下一个兄弟节点，以便继续处理。
	}
	// cout << "到主函数函数体了" << endl;
	/*在这种上下文中，通常将语法树的结构设计成这样的形式，其中：
	第一个子节点可能是程序中的全局变量或类型定义部分。
	第二个子节点可能是程序中的全局过程或函数定义部分。
	第三个子节点通常是主程序块，包含了程序的主函数体或主执行部分。
	因此，将 currentp 移动到主程序块的函数体部分，可以让分析器开始处理程序的主要执行逻辑，即主程序块内的语句序列。这种设计有助于在语法树中将主要执行部分与全局定义部分区分开来，使代码的组织结构更清晰。*/
	currentp = currentp->child[2]->child[0];//将 currentp 移动到主程序块的函数体部分。
	body(currentp);//调用 body 函数处理主程序块的函数体部分，即执行主程序块中的语句序列。
	//destroytable();
}

// 第四部分结束