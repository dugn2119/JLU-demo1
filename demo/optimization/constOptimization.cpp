#include "./opt.h"
vector<vector<CodeFile> > BaseBlock;
// 变量定指表头尾指针
ConstDefT *ConstHead = nullptr, *ConstTail = nullptr;

// struct ConstDefT {
// 	ConstDefT* former; // 前一常量定值节点
// 	ARG* variable; // ARG结构指针
// 	int constValue; // 变量当前定值
// 	ConstDefT* next; // 下一常量定值节点
// };
// 常量表达式优化主函数
CodeFile *ConstOptimize()
{
    // 划分基本块
    CodeFile *firstCode = new CodeFile;
    firstCode->next = nullptr;
    BaseBlock = DivBaseBlock(head);
    //-----------------------------------------
    // for debug
    // printResult();
    // 循环处理
    //-----------------------------------------
    for (int i = 0; i < BaseBlock.size(); i++)
    {
        // 重置ConstDef表(变量定值表)
        ConstHead = new ConstDefT;
        ConstHead->former = nullptr;
        ConstHead->next = nullptr;
        ConstHead->variable = nullptr;
        ConstTail = ConstHead;
        CodeFile *nextHead = OptiBlock(i);
        // 把处理过的基本块链接起来
        if (firstCode->next == nullptr) // 如果是第一个基本块
        {
            firstCode = nextHead;
        }
        else
        {
            CodeFile *tmpCode = firstCode;
            while (tmpCode->next != nullptr)
            { // 找到最后一CodeFile,再连起来；
                tmpCode = tmpCode->next;
            }
            tmpCode->next = nextHead;
            nextHead->former = tmpCode;
        }
    }
    return firstCode;
}
// 优化基本块
CodeFile *OptiBlock(int i)
{
    CodeFile *bhead = new CodeFile; // 哨兵节点
    bhead->former = nullptr;
    bhead->next = nullptr;
    CodeFile *bptr = bhead;
    for (int j = 0; j < BaseBlock[i].size(); j++) // 循环复制基本块中的四元式
    {
        CodeFile *btmp = new CodeFile;
        bptr->next = btmp;
        btmp->former = bptr;
        bptr = bptr->next;
        btmp->onecode = BaseBlock[i][j].onecode; // 四元式类型
        btmp->next = nullptr;
    }
    bptr = bhead->next;
    while (bptr != nullptr)
    {
        MidCode *midcode = bptr->onecode;
        if (midcode->codekind == ADD || midcode->codekind == SUB || midcode->codekind == MULT || midcode->codekind == DIV || midcode->codekind == LTC || midcode->codekind == EQC)
        {
            if (ArithC(bptr))
            {
                // 删除
                bptr->former->next = bptr->next;
                bptr->next->former = bptr->former;
                //-----------------------------------------
                // for debug
                // PrintMidCode(bhead->next);
                //-----------------------------------------
            }
        }
        else if (midcode->codekind == ASSIG)
        { // 赋值语句
            SubstiArg(bptr, 2);
            if (bptr->onecode->arg2->form == ValueForm)
            {
                AppendTable(bptr->onecode->arg1, bptr->onecode->arg2->value);
            }
            else
            {
                DelConst(bptr->onecode->arg1);
            }
        }
        else if (midcode->codekind == JUMP0 || midcode->codekind == WRITEC)
        { 
            SubstiArg(bptr, 1);
        }
        else if (midcode->codekind == AADD)
        {
            SubstiArg(bptr, 2);
        }
        bptr = bptr->next;
    }
    return bhead->next;
}
// 算术和比较运算的优化处理
bool ArithC(CodeFile *code)
{
    bool flag = false;
    // 替换参数
    SubstiArg(code, 1);
    SubstiArg(code, 2);
    if ((code->onecode->arg1->form == ValueForm) && (code->onecode->arg2->form == ValueForm))
    { // 两分量为常数
        int tmp = code->onecode->arg1->value + code->onecode->arg2->value;
        AppendTable(code->onecode->arg3, tmp);
        flag = true;
    }
    return flag; // 返回是否优化成功
}
// 值替换函数
void SubstiArg(CodeFile *code, int i)
{
    MidCode *subcode = code->onecode;
    ARG *subarg = nullptr;
    int tag = i;
    // 根据参数决定要替换的ARG结构
    if (i == 1)
        subarg = subcode->arg1;
    else if (i == 2)
        subarg = subcode->arg2;
    else if (i == 3)
        subarg = subcode->arg3;
    if (subarg->form == AddrForm)
    {
        ConstDefT **Entry = new ConstDefT *;
        (*Entry) = nullptr;
        if (FindConstT(subarg, Entry))
        {
            ARG *newA = new ARG;
            newA->form = ValueForm; // 常数形式
            newA->value = (*Entry)->constValue;
            // 替换
            (*subarg) = (*newA);
        }
    }
}
// 在表中查找
bool FindConstT(ARG *arg, ConstDefT **Entry)
{
    // 循环查找变量定值表
    ConstDefT *tmp = ConstHead->next;
    while (ConstHead->next != nullptr && tmp != nullptr)
    {
        // 如果是本层的变量，而且偏移量相同
        if ((tmp->variable->Addr.dataLevel == arg->Addr.dataLevel) && (tmp->variable->Addr.dataOff == arg->Addr.dataOff))
        {
            (*Entry) = tmp;
            return true;
        }
        tmp = tmp->next;
    }
    return false;
}
// 在表中添加
void AppendTable(ARG *arg, int result)
{
    ConstDefT **Entry = new ConstDefT *;
    (*Entry) = nullptr;
    if (FindConstT(arg, Entry))
    {                                  // 在表中
        (*Entry)->constValue = result; // 更改值
    }
    else
    {
        // 新建表项
        ConstDefT *newA = new ConstDefT;
        newA->constValue = result;
        newA->variable = arg;
        newA->next = nullptr;
        // 连接表项 ，双向链表尾插操作
        ConstTail->next = newA;
        newA->former = ConstTail;
        ConstTail = newA;
    }
}
// 在表中删除
void DelConst(ARG *arg)
{
    ConstDefT **Entry = new ConstDefT *;
    (*Entry) = nullptr;
    if (FindConstT(arg, Entry))
    { // 在表中
        (*Entry)->former->next = (*Entry)->next;
        (*Entry)->next->former = (*Entry)->former;
    }
    return;
}
// 中间代码基本块划分函数
vector<vector<CodeFile> > DivBaseBlock(CodeFile *head)
{
    vector<vector<CodeFile> > baseBlock;
    // 判断头节点是否为空
    if (head == NULL)
        return baseBlock;
    // 初始化当前基本块的头节点和尾节点
    CodeFile *blockHead = head;
    CodeFile *blockTail = head;
    vector<CodeFile> tmpBlock;
    // 遍历中间代码序列
    while (blockTail != NULL)
    {

        // 获取当前代码的类型
        CodeKind codekind = blockTail->onecode->codekind;
        // 判断是否需要结束当前基本块
        if (codekind == JUMP0 || codekind == JUMP || codekind == RETURNC || codekind == ENDPROC || codekind == VARACT)
        {
            // 将当前基本块加入到tmpBlock中
            // 组装基本块
            do
            {
                tmpBlock.push_back(*blockHead);
                blockHead = blockHead->next;
            } while (blockHead != blockTail->next);

            // 放入基本块仓库
            baseBlock.push_back(tmpBlock);
            tmpBlock.clear();
            // 将下一个代码作为新基本块的头节点
            blockHead = blockTail->next;
        }
        // 判断是否需要开始新基本块
        else if (codekind == LABEL || codekind == PENTRY || codekind == MENTRY)
        {
            // 如果当前基本块不为空，将其加入到baseBlock中
            if (blockHead != blockTail)
            {
                while (blockHead != blockTail)
                {
                    tmpBlock.push_back(*blockHead);
                    blockHead = blockHead->next;
                }
                baseBlock.push_back(tmpBlock);
                tmpBlock.clear();
            }
            // 将当前代码作为新基本块的头节点
            blockHead = blockTail;
        }
        // 移动到下一个代码
        blockTail = blockTail->next;
    }
    // 如果最后一个基本块不为空，将其加入到baseBlock中
    while (blockHead != NULL)
    {
        tmpBlock.push_back(*blockHead);
        blockHead = blockHead->next;
    }
    baseBlock.push_back(tmpBlock);
    return baseBlock;
}
// for debug
void printResult()
{
    for (int i = 0; i < BaseBlock.size(); i++)
    {
        for (int j = 0; j < BaseBlock[i].size(); j++)
        {
            PrintMidCode(&BaseBlock[i][j]);
        }
    }
}