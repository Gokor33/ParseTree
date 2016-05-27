#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tokeniser.h"
#include "nodes.h"

int symb;

extern printSymb();
extern char* showSymb(int);
extern int yylex(void);
extern FILE* yyin;

extern void prettytree(NODE*,int);
extern char * yytext;

int mainFunction = 0;

lex()
{
   //printSymb();
   symb = yylex();
   //return NULL;
}

NODE * newInt(int v)
{  NODE * n;
   n = (NODE *)malloc(sizeof(NODE));
   n->tag = NUMBER;
   n->f.value = v;
   return n;
}


NODE * newId(char * i)
{  NODE * n;
   char * cur = strdup(i);
   n = (NODE *)malloc(sizeof(NODE));
   n->tag = NAME;
   n->f.name = cur;
   return n;
}

NODE * newNode(int tag)
{  NODE * n;
   n = (NODE *)malloc(sizeof(NODE));
   n->tag = tag;
   n->f.b.n1 = NULL;
   n->f.b.n2 = NULL;
   return n;
}

NODE * name()
{
	return newNode(yytext);
}

NODE * number()
{
	return newNode(atoi(yytext));
}

showTree(NODE * tree,int depth)
{  int i;
   if(tree==NULL)
    return;
   for(i=0;i<depth;i++)
    putchar('-');
   switch(tree->tag)
   {  case NAME: printf("%s\n",tree->f.name);
               return;
      case NUMBER: printf("%d\n",tree->f.value);
               return;
      default: printf("%s\n",showSymb(tree->tag));
               showTree(tree->f.b.n1,depth+1);
               showTree(tree->f.b.n2,depth+1);
   }
}

error(char * rule,char * message)
{  printf("%s: found %s\n",rule,showSymb(symb));
   printf("%s: %s\n",rule,message);
   exit(0);
}


NODE * program()
{  extern NODE * functions();
   NODE * p;
   p = newNode(SEMI);
   if(symb==SEMI)
    p->f.b.n1 = functions();
   return p;
}

NODE * functions()
{  extern NODE * function();
   NODE * c;
   c = function();
   if(symb==SEMI)
   {  NODE * c1;
      c1 = c;
      c = newNode(SEMI);
      c->f.b.n1 = c1;
      lex();
      c->f.b.n2 = functions();
   }
   return c;
}

NODE * function()
{
    extern NODE * name();
    extern NODE * args();
    extern NODE * arg();
    extern NODE * defs();
    extern NODE * commands();

    NODE * f = newNode(FUNCTION);
    lex();
    //printf("-%s of %d-",yytext,symb);
    if(symb==MAIN){ // if: its a main function
        if(mainFunction == 0)
            mainFunction = 1;
        else
            error("functions","Main function already exists\n");
        f->f.b.n1 = name();
        lex();
        if(symb!=LBRA)
            error("function","'(' expected after main\n");
        lex();
        if(symb!=RBRA)
            error("function","')' expected after 'main('\n");
        lex();
        switch(symb){
            case IS:
                f->f.b.n2 = newNode(IS);
                lex();
                f->f.b.n2->f.b.n1 = defs();
                if(symb!=TBEGIN)
                    error("function","begin expected in a function\n");
                f->f.b.n2->f.b.n2 = newNode(TBEGIN);
                lex();
                f->f.b.n2->f.b.n2->f.b.n1 = commands();
                break;
            case TBEGIN:
                f->f.b.n2 = newNode(TBEGIN);
                lex();
                f->f.b.n2->f.b.n1 = commands();
                break;
            default: error("function","is or begin expected\n");
        }
        lex();
        if(symb!=MAIN)
        error("function","main expected after end\n");
    }else{ // else: its a normal function
        if(symb==BOP) // Make sure function is not named plus,minus,etc.
            error("function","function name not allowed\n");
        if(symb!=NAME)
            error("function","name expected\n");
        f->f.b.n1 = name();
        lex();
        if(symb!=LBRA)
            error("function","'(' expected after function name\n");
        lex();
        f->f.b.n2 = newNode(RBRA);
        if(symb!=RBRA){
            f->f.b.n2->f.b.n1 = args(); // even if there are no arguments, this will raise an error if RBRA is not there
            if(symb!=RBRA)
                error("function","')' expected after arguments\n");
        }
        lex();
        if(symb!=RETURNS)
            error("function","function must have a return\n");
        f->f.b.n2->f.b.n2 = newNode(RETURNS);
        lex();
        if(symb!=LBRA)
            error("function","'(' expected after returns\n");
        lex();
        f->f.b.n2->f.b.n2->f.b.n1 = arg();
        if(symb!=RBRA)
            error("function","')' expected after args\n");
        lex();
        switch(symb){
            case IS:
                f->f.b.n2->f.b.n2->f.b.n2 = newNode(IS);
                lex();
                f->f.b.n2->f.b.n2->f.b.n2->f.b.n1 = defs();
                if(symb!=TBEGIN)
                    error("function","begin expected in a function\n");
                f->f.b.n2->f.b.n2->f.b.n2->f.b.n2 = newNode(TBEGIN);
                lex();
                f->f.b.n2->f.b.n2->f.b.n2->f.b.n2->f.b.n1 = commands();
                break;
            case TBEGIN:
                f->f.b.n2->f.b.n2->f.b.n2 = newNode(TBEGIN);
                lex();
                f->f.b.n2->f.b.n2->f.b.n2->f.b.n1 = commands();
                break;
            default: error("function","begin or is expected in a function\n");
        }
        lex();
        if(symb!=NAME)
        error("function","name expected after end\n");
    }
    char * startName = f->f.b.n1->f.name;
    char * endName = yytext;
    if(strcmp(startName,endName))
        error("function","start name doesn't match end name\n");
    return f;
}

NODE * args()
{  extern NODE * arg();
   NODE * c;
   c = arg();
   if(symb==COMMA)
   {  NODE * c1;
      c1 = c;
      c = newNode(COMMA);
      c->f.b.n1 = c1;
      lex();
      c->f.b.n2 = args();
   }
   return c;
}

NODE * arg()
{  extern NODE * name();
   NODE * t;
   t = name();
   if(symb!=NAME)
   {
      error("arg","NAME expected");
   }
   lex();
   if(symb!=COLLON)
   {
	error("arg",": expected");
   }
   lex();
   if(symb!=NUMBER)
   {
	error("arg","NUMBER expected");
   }
   lex();
   return t;
}

NODE * defs()
{  extern NODE * def();
   NODE * c;
   c = def();
   if(symb==SEMI)
   {  NODE * c1;
      c1 = c;
      c = newNode(SEMI);
      c->f.b.n1 = c1;
      lex();
      c->f.b.n2 = defs();
   }
   return c;
}

NODE * def()
{  extern NODE * name();
   extern NODE * type();

   NODE * t = newNode(COLLON);

   if(symb==NAME)
   {
      t->f.b.n1 = name();
      lex();
   }

   if(symb==COLLON)
   {
      lex();
      t->f.b.n2 = type();
   }
   return t;
}

NODE * type()
{  extern NODE * number();
   switch(symb)
   {  case NUMBER: lex();
                   return NUMBER;
      case ARRAY: lex();
                   return NUMBER;
   }
}

NODE * commands()
{  extern NODE * command();
   NODE * c;
   c = command();
   if(symb==SEMI)
   {  NODE * c1;
      c1 = c;
      c = newNode(SEMI);
      c->f.b.n1 = c1;
      lex();
      c->f.b.n2 = commands();
   }
   return c;
}

NODE * command()
{
    extern NODE * ifComm();
    extern NODE * whileComm();
    extern NODE * name();
    extern NODE * expr();
    extern NODE * assign();
    extern NODE * read();
    extern NODE * write();
    NODE * c;

    switch(symb){
        case IF: return ifComm();
        case WHILE: return whileComm();
        case READ: return read();
        lex();
        if(symb!=NAME)
            error("command","NAME expected after READ");
        c = name();
        lex();
        return c;
        case WRITE: return write();
        lex();
        return expr();
        case NAME: return assign();
        default: error("command","Invalid command\n");
    }
}

NODE * repeatComm()
{  extern NODE * condexp();
   NODE *c;
   NODE *r;
   c = command();
   if (symb != UNTIL)
    error("repeat","UNTIL expected\n");
   r = newNode(REPEAT);
   r->f.b.n1 = c;
   lex();
   r->f.b.n2 = condexp();
   return r;
}

NODE * assign()
{  extern NODE * expr();
   extern NODE * name();

   NODE * a = newNode(ASSIGN);
   NODE * i = newId(yytext);
   lex();
   if(symb==NAME)
   {
	a->f.b.n1 = name();
        lex();
        if(symb!=LSBRACK)
        {
		error("assign","LSBRACK expected\n");
                a->f.b.n1->f.b.n2 = expr();
	        lex();
                if(symb!=RSBRACK)
		{
			error("assign","rSBRACK expected\n");
			lex();
			  if(symb!=ASSIGN)
    				error("assign",":= expected\n");
   				a = newNode(ASSIGN);
   				a->f.b.n1 = i;
   				lex();
   				a->f.b.n2 = expr();
   				return a;
		}
        }

   }
   if(symb!=ASSIGN)
    error("assign",":= expected\n");
   a = newNode(ASSIGN);
   a->f.b.n1 = i;
   lex();
   a->f.b.n2 = expr();
   return a;
}

NODE * ifComm()
{  extern NODE * condexpr();
   NODE * c;
   NODE * t;
   c = newNode(IF);
   c->f.b.n1 = condexpr();
   if(symb!=THEN)
    error("if","THEN expected\n");
   lex();
   t = command();
   if(symb==ELSE)
   {  lex();
      c->f.b.n2 = newNode(ELSE);
      c->f.b.n2->f.b.n1 = t;
      c->f.b.n2->f.b.n2 = command();
   }
   else
    c->f.b.n2 = t;
   if(symb!=END)
        error("if","end expected\n");
    lex();
    if(symb!=IF)
        error("if","end if expected\n");
    lex();
   return c;
}


NODE * whileComm()
{  extern NODE * condexpr();
   NODE * w;
   NODE * c;
   c = condexpr();
   if(symb!=LOOP)
    error("while","LOOP expected\n");
   lex();
   w = newNode(WHILE);
   w->f.b.n1 = c;
   w->f.b.n2 = command();
   if(symb!=END)
    error("while","END expected\n");
   lex();
   if(symb!=LOOP)
    error("while","LOOP expected\n");
   lex();
   return w;
}

NODE * forComm()
{  extern NODE * condexp();
   NODE * w;
   NODE * c;
   if(symb!=WHILE)
   {
    error("for","WHILE expected\n");
   }
   c = condexp();
   lex();
   if(symb!=THEN)
    error("for","THEN expected\n");
   lex();
   w = newNode(FOR);
   w->f.b.n1 = c;
   w->f.b.n2 = command();
   return w;
}

NODE * condexpr()
{
    extern NODE * name();
    extern NODE * exprs();

    NODE * c = newNode(LBRA);
    lex();
    if(symb!=BOP)
        error("condexpr","BOP expected \n");
    c->f.b.n1 = name();
    lex();
    if(symb!=LBRA)
        error("condexpr","( expected after BOP\n");
    lex();
    c->f.b.n2 = exprs();
    lex();
    return c;
}

NODE * expr()
{
    extern NODE * exprs();
    extern NODE * name();
    NODE * e;
    NODE * ename;

    switch(symb){
        case NAME:
            e = name();
            lex();
            switch(symb){
                case LBRA:
                    ename = e;
                    e = newNode(LBRA);
                    e->f.b.n1 = ename;
                    lex();
                    e->f.b.n2 = exprs();
                    printf("-%s-",yytext);
                    if(symb!=RBRA)
                        error("expr",") expected after exprs");
                    lex();
                    return e;
                case LSBRACK:
                    e->tag = RSBRACK;
                    lex();
                    e->f.b.n2 = expr();
                    if(symb!=RSBRACK)
                        error("expr","[ expected after [expr\n");
                    lex();
                    return e;
                case COMMA:
                case RBRA:
                case SEMI: return e;
                default: error("expr","invalid expr\n");
            }
        case NUMBER:
            lex();
            return number();
        default: error("expr","name or number expected\n");
    }
}

NODE * exprs()
{  extern NODE * expr();
   NODE * c;
   c = expr();
   if(symb==COMMA)
   {  NODE * c1;
      c1 = c;
      c = newNode(SEMI);
      c->f.b.n1 = c1;
      lex();
      c->f.b.n2 = exprs();
   }
   return c;
}

NODE * read()
{  NODE * b;
   b = newNode(READ);
   if(symb!=NAME)
    error("read","END expected\n");
   b->f.b.n1 = name();
   lex();
   return b;
}

NODE * write()
{  NODE * b;
   b = newNode(WRITE);
   if(symb!=NAME)
    error("write","EXPR expected\n");
   b->f.b.n1 = expr();
   lex();
   return b;
}

main (int c,char ** argv)
{

   if((yyin=fopen(argv[1],"r"))==NULL){
      printf("can't open %s\n",argv[1]);
      exit(0);
   }

   symb = yylex();
   NODE* ast = program();
   showTree(ast,1);
   fclose(yyin);

}
