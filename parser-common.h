/*
 * common.h
 *
 *  Created on: Oct 23, 2010
 *      Author: farhanhubble
 */




#ifndef COMMON_H_
#define COMMON_H_ 1


#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<assert.h>
#include<stdbool.h>



#endif /* COMMON_H_ */

struct prodList{	/* Structure for storing all productions of a symbol */
	char *head;
	char **productions;
	char **first;
	char *follow;
	_Bool nullable;
	int prodcount;

};

struct parseinfo{
	int  varcount;
	int	 termcount;
	char **variables;
	char **terminals;
	char ***actions;
};





int 			fileread(char**,const char*,int *);

void 			printRaw(char *,int);

struct prodList **makeGrammarTable(char *, int *, int*);

char			**addRow(char **,int );

char 			*addSymbolToRow(char *,const char *);

struct prodList **incrementRows(struct prodList**,int );

char 			*preProcess(char **,int );

int 			genNullables(struct prodList**,int,char*);

int 			genFirst(struct prodList**,int,char*);

int 			genFollow(struct prodList**,int,char*);

_Bool 			isToken(char *,char *);

_Bool 			isNullable(char *,struct prodList**,int ,char *);

char 			*getNextSymbol(char *,int *);

_Bool 			isToken(char *,char *);

int 			indexOf(char *,struct prodList**,int );

char 			*addToSet(char *, char * , int *);

char 			*doUnion(char *, char *,int *);

int 			genParseTable(struct parseinfo *, struct prodList ** , int  , char *);

int 			setAction(struct parseinfo *,char* , char *,char * );

int 			save(struct parseinfo );

char 			*removeFromSet(char* ,char *,int *);

char			*clean(char *);


