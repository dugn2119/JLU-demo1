#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
using namespace std;

struct Token // token序列
{
	int linenum; // 行号
	int type;	 // 类别
	char s[100]; // 信息
}; // 定义Token结构体

struct queue
{
	void insert(char ch); // 向队列尾插入一个字符
	void ini();			  // 队列初始化函数
	void delet();		  // 删除队列头元素
	char view_head();	  // 查看队列头元素
	bool empty();		  // 判断队列是否为空
	void clear();		  // 清空队列
	int count;			  // 对队列里元素计数
	int rear;			  // 队尾
	int head;			  // 对头
	char a[200];
}; // 使用队列来处理字符串

// queue 结构里的函数声明
void queue::clear()
{
	count = head = rear = 0;
} // 清空队列

void queue::ini()
{
	count = head = rear = 0;
} // 初始化队列

void queue::delet()
{
	if (count <= 0)
		printf("队列里已经没有字符可以删除!\n");
	else
	{
		head++;
		if (head >= 200)
			head = head % 200;
		count--;
	}
} // 删除队列中的头元素

char queue::view_head()
{
	if (count > 0)
		return a[head];
} // 得到头元素

bool queue::empty()
{
	if (count <= 0)
		return true;
	else
		return false;
} // 看队列是否为空

void queue::insert(char ch)
{
	if (count >= 200)
		printf("字符过长!!!");
	else
	{
		a[rear] = ch;
		rear++;
		if (rear >= 200)
			rear = rear % 200;

		count++;
	}
}

const int numOfBaoLiu = 22; // 21个保留字;
char baoliu[25][10] = {"char", "int", "var", "program", "integer", "procedure", "array", "begin", "while", "if",
					   "then", "else", "endwh", "end", "read", "of", "record", "fi", "return", "write", "type", "do"};

bool judge(char *str) // 在标识符判断是不是保留字
{
	bool flag = false;
	for (int i = 0; i < numOfBaoLiu; i++)
	{
		if (strcmp(baoliu[i], str) == 0)
		{
			flag = true;
			break;
		}
	}
	return flag;
}

void getTokenList()
{
	Token token[500]; // 储存token序列；
	int p_token = 0;  // token序列的位置
	FILE *fp;		  // 从文件中读字符
	if ((fp = fopen("source.txt", "r")) == NULL)
		printf("文件打开失败!\n");

	char ch = fgetc(fp); // 读取第一个字符
	int start = 0;		 // 设置初始状态为0，总共14+1(程序终止)种状态
	int type = 0;		 // 定义单词类别
	int line = 1;		 // 记录行数,初始行号为1
	queue store;		 // 定义储存队列
	store.ini();
	queue linshi; // 临时队列
	linshi.ini();
	linshi.insert(ch); // 加入第一个字符

	while (start != 14)
	{
		if (start == 0) // 初始状态,判断处于哪一条支线上
		{
			char c = linshi.view_head();

			// 标识符状态
			if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) 
			{
				start = 1;
				store.insert(c);
				linshi.delet();
				ch = fgetc(fp);
				if (ch == EOF) // 文件终止
				{
					start = 14;
					type = 1;
				}
				else
				{
					linshi.insert(ch); // 插入
				}
			}

			// 数字状态
			else if (c >= '0' && c <= '9')
			{ 
				start = 2;
				store.insert(c);
				linshi.delet();
				ch = fgetc(fp);
				if (ch == EOF) // 文件终止
				{
					start = 14;
					type = 3;
				}
				else
				{
					linshi.insert(ch); // 插入
				} // 单分界符号
			}

			
			else if (c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')' || c == ';' || c == '[' || c == ']' || c == '=' || c == '<' || c == ',')
			{//op  + - * / ( ) ; [ ] = < ,
				start = 3;
				store.insert(c);
				linshi.delet();
				ch = fgetc(fp);
				if (ch == EOF) // 文件终止
				{
					start = 14;
					type = 4;
				}
				else
				{
					linshi.insert(ch); // 插入
				}
			}
			
			else if (c == ':')
			{ // 赋值符号
				start = 4;
				store.insert(c);
				linshi.delet();
				ch = fgetc(fp);
				if (ch == EOF) // 文件终止
				{
					start = 14;
					type = 18;
				}
				else
				{
					linshi.insert(ch); // 插入
				}
			}
			else if (c == '{')
			{ // 注释符号
				start = 6;
				linshi.delet();
				ch = fgetc(fp);
				if (ch == EOF) // 文件终止
				{
					start = 14;
				}
				else
				{
					linshi.insert(ch); // 插入
				}
			}
			else if (c == '.')
			{ // 程序终止或者数组下标
				start = 7;
				type = 4;
				store.insert(c);
				linshi.delet();
				ch = fgetc(fp);
				if (ch == EOF) // 文件终止
				{
					start = 14;
				}
				else
				{
					linshi.insert(ch); // 插入
				}
			}
			else if (c == ' ' || c == '\n' || c == '\t')
			{
				start = 13; // 中止状态，已经读取下一个字符
				if (c == '\n')
					line++; // 行数加一
				linshi.delet();
				ch = fgetc(fp);
				if (ch == EOF) // 文件终止
				{
					start = 14;
				}
				else
				{
					linshi.insert(ch); // 插入
				}
			}
			else if (c == '\'')//
			{
				start = 9;
				store.insert(c);
				linshi.delet();
				ch = fgetc(fp);
				if (ch == EOF) // 文件终止
				{
					start = 14;
				}
				else
				{
					linshi.insert(ch); // 插入
				}
			}
			else // 出错
			{
				//printf("Error exists in %d\n", line);
				linshi.delet();
				start = 13;
				ch = fgetc(fp);
				if (ch == EOF) // 文件终止
				{
					start = 14;
				}
				else
				{
					linshi.insert(ch); // 插入
				}
			}
		}
		if (start == 13)
		{ // 中止状态
			start = 0;
			if (!store.empty()) // 队列不为空，要储存token序列
			{
				token[p_token].linenum = line; // 行数
				token[p_token].type = type;
				for (int i = 0; i < store.count; i++)
				{
					int q = store.head;
					int m = q + i;
					if (m >= 200)
						m %= 200;
					token[p_token].s[i] = store.a[m];
				}
				token[p_token].s[store.count++] = '\0';
				if (type == 1)
				{
					if (judge(token[p_token].s))
						token[p_token].type = 2;
				}
				type = token[p_token].type;
				p_token++;
				store.clear(); // 清除
			}
		}
		else
		{ // 其他状态
			char c = linshi.view_head();
			switch (start)
			{
			case 1:
				if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))
				{
					store.insert(c);
					linshi.delet();
					ch = fgetc(fp);
					if (ch == EOF) // 文件终止
					{
						start = 14;
					}
					else
					{
						linshi.insert(ch); // 插入
					}
				}
				else
				{
					start = 13;
					type = 1; // 标识符
				}
				break;
			case 2:
				if (c >= '0' && c <= '9') // 数字
				{
					store.insert(c);
					linshi.delet();
					ch = fgetc(fp);
					if (ch == EOF) // 文件终止
					{
						start = 14;
					}
					else
					{
						linshi.insert(ch); // 插入
					}
				}
				else
				{
					start = 13;
					type = 3; // 无符号数字
				}
				break;
			case 3:
				start = 13;
				type = 4; // 单符号
				break;
			case 4:
				if (c == '=')
				{
					store.insert(c);
					linshi.delet();
					start = 13;
					type = 5; // 赋值
					ch = fgetc(fp);
					if (ch == EOF) // 文件终止
					{
						start = 14;
					}
					else
					{
						linshi.insert(ch); // 插入
					}
				}
				else
				{
					printf("4 Error exists in %d\n", line);
					start = 13;
					type = 18; // 错误状态
				}
				break;
			case 6:
				if (c != '}')
				{
					linshi.delet();
					if (c == '\n')
						line++;
					ch = fgetc(fp);
					if (ch == EOF) // 文件终止
					{
						start = 14;
					}
					else
					{
						linshi.insert(ch); // 插入
					}
				}
				else
				{
					type = 6;
					start = 13;
					linshi.delet();
					ch = fgetc(fp);
					if (ch == EOF) // 文件终止
					{
						start = 14;
					}
					else
					{
						linshi.insert(ch); // 插入
					}
				}
				break;
			case 7:
				if (c != '.')
				{
					start = 13; // 相当于单符号
					type = 4;
					break;
				}
				else
				{
					start = 8;
					linshi.delet();
					store.insert(c);
					ch = fgetc(fp);
					if (ch == EOF) // 文件终止
					{
						start = 14;
					}
					else
					{
						linshi.insert(ch); // 插入
					}
				}
				break;
			case 8:
				start = 13;
				type = 7;
				break;
			case 9:
				if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))
				{
					start = 10;
					store.insert(c);
					linshi.delet();
					ch = fgetc(fp);
					if (ch == EOF) // 文件终止
					{
						start = 14;
					}
					else
					{
						linshi.insert(ch); // 插入
					}
				}
				else
				{
					printf("9 Error exists in %d\n", line);
					start = 13;
					type = 18; // 错误状态
				}
				break;
			case 10:
				if (c == '\'')
				{
					start = 13;
					type = 8;
					store.insert(c);
					linshi.delet();
					ch = fgetc(fp);
					if (ch == EOF) // 文件终止
					{
						start = 14;
					}
					else
					{
						linshi.insert(ch); // 插入
					}
				}
				else
				{
					printf("10 Error exists in %d\n", line);
					start = 13;
					type = 18; // 错误状态
				}
			}
		}
	}
	if (!store.empty()) // 队列不为空，要储存token序列
	{
		token[p_token].linenum = line; // 行数
		token[p_token].type = type;
		for (int i = 0; i < store.count; i++)
		{ //
			int q = store.head;
			int m = q + i;
			if (m >= 200)
				m %= 200;
			token[p_token].s[i] = store.a[m];
		}
		token[p_token].s[store.count++] = '\0';
		// if(type==1)//判断保留字
		// {
		if (judge(token[p_token].s))
			token[p_token].type = 2;
		// }
		p_token++;
	}
	store.clear();
	FILE *fpp = fopen("token.txt", "w");
	fprintf(fpp, "%d\n", p_token);
	for (int i = 0; i < p_token; i++)
	{
		// cout<<token[i].linenum<<' '<<token[i].type<<' '<<token[i].s<<endl;
		fprintf(fpp, "%d    %d    %s\n", token[i].linenum, token[i].type, token[i].s);
	}
}
// 状态： 1.标识符 2.保留字 3.无符号数字 4.单分界符 5.双分界 6. 注释 7.数组下标 8.字符状态 18.出错
int main(void)
{
	getTokenList(); // 执行gettokenlist函数
	return 0;
}
