#include <stdio.h>
#include "sema.h"
#include "myparser.h"

extern int linenum;		//在lex源文件中定义的,当前记号所在行
extern char linebuf[];	//lex中定义
extern int errors;		//yacc中定义

static char info[1024];		//缓存错误信息

//输出语义警告
static void warn()
{
	printf("line %d:<语义错误>%s\n%s\n",linenum+1,info,linebuf);
	errors++;
}

//将编码的类型转化为字符串的形式
static char* findType(int type)
{
	char *s;
	switch(type)
	{
	case INT: s="INT"; break;
	case TBOOL: s="TBOOL"; break;
	case ARRAY: s="ARRAY"; break;
	case NORETURN: s="无返回值"; break;
	default: s="**"; printf("类型:%d尚未支持\n",type);
	}
	return s;
}

//将编码的操作类型转化为字符串的形式
static char* findOp(int type)
{
	char *s;
	switch(type)
	{
	case NEG: s="取负"; break;
	case '!': s="取非"; break;
	case EQUAL: s="判等"; break;
	case AND: s="与"; break;
	case OR: s="或"; break;
	case '<': s="<"; break;
	case '>': s=">"; break;

	case WHILE: s="while循环"; break;
	case IFU: s="不完全匹配的条件选择"; break;
	case IFF: s="完全匹配的条件选择"; break;
	}
	return s;
}

void verifySema(LTree node)
{
	LTree c1,c2;
	int t;

	switch(node->type)
	{
	case MAIN: case MULTI2:	case PRINT:	//这种树不检查,语法通过了语义就没问题
		break;

	case WHILE: case IFF: case IFU:		//这种树检查第一棵子树是不是布尔型的
		c1=node->chi;
		if(c1->returnType!=TBOOL)
		{
			sprintf(info,"%s类型不能作为%s语句的判定条件",findType(c1->returnType),findOp(node->type));
			errors++;
			printf("<语义错误>%s\n",info);
		}
		break;

	//赋值类型检查
	case '=':
		c1=node->chi;
		c2=c1->bro;
		if(c1->returnType!=c2->returnType)	//除了new其他运算中数组出现的格式都是id[I]
		{									//(id都被当做变量而非数组来检索,如果是数组就会报告找不到变量)
			sprintf(info,"%s类型不能赋值为%s类型",findType(c1->returnType),findType(c2->returnType));
			warn();
		}
		node->returnType=c1->returnType;
		break;

	//整数双元运算检查
	case '+': case '-': case '*': case '/': case '<': case '>':
		c1=node->chi;
		c2=c1->bro;
		if(c1->returnType!=INT || c2->returnType!=INT)
		{
			sprintf(info,"%c运算不能应用于%s类型与%s类型",node->type,findType(c1->returnType),findType(c2->returnType));
			warn();
		}
		node->returnType=(node->type=='<' || node->type=='>') ? TBOOL : INT;
		break;

	//单元运算检查
	case NEG: case '!':
		t=node->type==NEG ? INT : TBOOL;
		c1=node->chi;
		if(c1->returnType!=t)
		{
			sprintf(info,"运算不能应用于%s类型",findOp(node->type),findType(c1->returnType));
			warn();
		}
		node->returnType=t;
		break;

	//布尔运算检查
	case OR: case AND:
		c1=node->chi;
		c2=c1->bro;
		if(c1->returnType!=TBOOL || c2->returnType!=TBOOL)
		{
			sprintf(info,"%s运算不能应用于%s类型与%s类型",findOp(node->type),findType(c1->returnType),findType(c2->returnType));
			warn();
		}
		node->returnType=TBOOL;		//为了精确定位错误,一般节点的返回值还是赋值为正确的
		break;
	
	//判等运算对于INT,INT和TBOOL,TBOOL组合都是正确的
	case EQUAL:
		c1=node->chi;
		c2=c1->bro;
		if(c1->returnType!=c2->returnType)
		{
			sprintf(info,"%s运算不能应用于%s类型与%s类型",findOp(node->type),findType(c1->returnType),findType(c2->returnType));
			warn();
		}
		node->returnType=TBOOL;		//为了精确定位错误,一般节点的返回值还是赋值为正确的
		break;
	}
}