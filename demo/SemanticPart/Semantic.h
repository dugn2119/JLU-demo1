#include "../include/include.h"
#include "../include/var.h"
#ifndef SEMANTIC_H
#define SEMANTIC_H

#define NAMETYPE 1
#define INTTYPE 2
#define CHARTYPE 3
#define ARRAYTYPE 5
#define RECORDTYPE 6
#define IFSTATMENT 7
#define WHILESTATMENT 12
#define READSTATMENT 13
#define WRITESTATMENT 14
#define RETURNSTATMENT 15
#define ASSINSTMENT 16
#define CALLSTATMENT 17

struct typeir;
struct symbtable;
struct paramtable // 形参类型表
{
    symbtable *entry; // 形参类型表的符号表中的位置
    paramtable *next; // 下一个形参类型表
};

struct fieldchain // 记录域表
{
    string idname;    // 纪录域中的标识符
    typeir *unittype; // 纪录域中的类型，内部表示
    int offset;       // 纪录域中的偏移量
    fieldchain *next; // 纪录域中的下一个域
};

struct attributeir // 标识符
{
    struct typeir *idtype;     // 指向标识符类型的内部表示
    int kind;                  // 0 typekind 1 varkind 2 prockind
    symbtable *link = nullptr; // 如果是过程名，指向过程名的符号表，驻留法

    struct
    {
        string access; // 访问类型
        int level;     // 层次
        int off;       // 偏移量
    } varattr;         // 变量标识符的属性

    struct
    {
        int level;
        paramtable *param; // 参数表
        int code;          // 过程的目标代码地址，在目标代码生成时使用
        int size;          // 过程所需的空间大小
    } procattr; // 过程名标识符的属性
};

struct typeir // 类型内部结构
{
    int size;    // 大小
    string kind; // 指示具体的类型，如值为intTy表示整数类型  intTy, charTy, arrayTy, recordTy, boolTy

    struct // 数组
    {
        struct typeir *indexty;
        struct typeir *elemty; // 元素类型
    } arrayattr;

    fieldchain *body; // 记录类型的域表
};

struct symbtable // 符号表表项
{
    string idname;          // 元素名称
    attributeir attrir;     // 类型
    struct symbtable *next; // 一个一个元素连接  链表一样
    symbtable()
    {
    }
    symbtable(string id, attributeir *attrir)
    {
        this->idname = id;
        this->attrir = *attrir;
    }
};

enum accesskind // 访问类型，直接访问还是间接访问
{
    dir,
    indir
};
//---------------------------------------------------------------------
// 符号表
//---------------------------------------------------------------------
void createtable(); // 创建符号表
void destroytable(); // 销
int searchonetable(string id, int currentlevel, symbtable **entry); // 在当前层查找
int findentry(string id, string flag, symbtable **entry); // 查找
int enter(string id, attributeir *attribp, symbtable **entry); // 登记标识符和属性到符号表
int findfield(string id, fieldchain *head, fieldchain **entry); // 查找域
void printsymbtable(); // 打印符号表


//---------------------------------------------------------------------
typeir *inttype(); // 整数类型
typeir *chartype(); // 字符类型
typeir *arraytype(Node *t); // 数组类型
typeir *typeprocess(Node *t, int num); // 类型分析处理函数,处理语法树的当前节点类型，构造当前类型的内部表示，并将其地址返回给Ptr类型内部表示的地址
typeir *arrayvar(Node *t);
typeir *fieldvar(Node *t);
//---------------------------------------------------------------------
void typedecpart(Node *t); // 类型声明部分



//---------------------------------------------------------------------
void assignstatment(Node *t);
void ifstatment(Node *t);
void whilestatment(Node *t);
void readstatment(Node *t);
void writestatment(Node *t);
void returnstatment(Node *t);
void callstatment(Node *t);
void body(Node *t); // 12
void printsymbtable();
void printsymbtablePro(symbtable *t);
void analyze(Node *currentp);

#endif