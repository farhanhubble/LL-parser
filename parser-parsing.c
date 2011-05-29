
#ifndef __AUXILLARY_C
#define __AUXILLARY_C 1
#include "parser-common.h"

#endif /* __AUXILLARY_H */

char 			**push(char** , char * , int *);
char 			**pop(char**  , int * );
_Bool 			isTerminal(char *, struct parseinfo  , int );
char 			*getAction(struct parseinfo table, char *variable , char *token);


/*
 * Load parse table from file.
 * @Param1: Structure to load into.
 */

int load(struct parseinfo *table)
{
	FILE 		*fp;
	int 		nvar;
	int 		nterm;
	int 		i,j,len;
	char 		*tmp;

	fp = fopen("parsetable.txt","r");

	if(!fp)
	{
		perror("parsetable.txt");
		return 1;
	}

	fscanf(fp,"%d",&nvar);
	fgetc(fp);	/* This is to eat newline */

	assert(nvar > 0);

	printf("\n%d Variables detected\n",nvar);

	table->varcount = nvar;

	table->variables = (char**)malloc(sizeof(char*) * nvar);

	assert(table->variables != NULL);

	/* Read symbols each on a new line from file */
	/* Read symbols into dynamic buffer of length 256.
	 * This puts a limit of 255 characters on the symbol length.
	 *
	 */

	for(i=0 ; i< nvar ; i++)
	{
		tmp = (char*)malloc(sizeof(char) * 256);
		fgets(tmp,256,fp);
		table->variables[i] = strdup(tmp);
		free(tmp);

		if(table->variables[i] == NULL )
		{
			fclose(fp);
			return 1;
		}


		len = strlen(table->variables[i]);
		table->variables[i][len-1] = '\0';
	}


	fscanf(fp,"%d",&nterm);
	fgetc(fp);

	assert(nterm > 0);

	table->termcount = nterm;
	table->terminals = (char**)malloc(sizeof(char*) * nterm);

	assert(table->terminals != NULL);

	for(i = 0 ; i< nterm ; i++)
	{
		tmp = (char*)malloc(sizeof(char) * 256);
		fgets(tmp,256,fp);
		table->terminals[i] = strdup(tmp);
		free(tmp);

		if(table->terminals[i] == NULL)
		{
			fclose(fp);
			return 1;
		}

		len = strlen(table->terminals[i]);
		table->terminals[i][len-1] = '\0';

	}

	table->actions = (char***)malloc(sizeof(char**) * nvar);

	assert(table->actions != NULL);

	for(i=0 ; i< nvar ; i++)
	{
		table->actions[i] = (char**) malloc(sizeof(char*) * nterm);
		assert(table->actions[i] != NULL);
	}

	for(i=0 ; i< nvar ;i++)
	{
		for(j=0;j<nterm ;j++)
		{
			tmp = (char*)malloc(sizeof(char) * 4096);
			fgets(tmp,4095,fp);
			table->actions[i][j] = strdup(tmp);
			free(tmp);
			for(int z=0;z<strlen(table->actions[i][j]) ; z++)
			{
				if(table->actions[i][j][z] == '\n')	table->actions[i][j][z] = ' ';
			}

			len = strlen(table->actions[i][j]);
			table->actions[i][j][len-1] = '\0';


		}

	}

	fclose(fp);

	return 0;


}


int parse(struct parseinfo parseTable, char *input)
{
	int 	seen = 0;
	char 	**stack = NULL;
	int		top;
	char 	*nexttoken ;
	char 	*topSymbol;
	char    *action;
	char 	*productionSymbol;

	printf("%-30s %20s %20s","Stack (top)","Input token","Action");
	/* Initialize Stack with input endmarker*/
	stack = push(stack,"$",&top);
	if(stack == NULL)
	{
		printf("\n%s:%d  Error pushing string onto stack.",__FILE__,__LINE__);
		return 1;
	}

	/* Push start variable on top */
	stack = push(stack,parseTable.variables[0],&top);
	if(stack == NULL)
	{
		printf("\n%s:%d  Error pushing string onto stack.",__FILE__,__LINE__);
		return 1;
	}


	/* Keep parsing */
	for( nexttoken = clean(getNextSymbol(input,&seen))	 ;	 ;	)
	{

		printf("\n%-30s %20s",stack[top],nexttoken);

		/* If no more input terminals are left raise error */
		if(nexttoken == NULL)
		{
			printf("\nInput string exhausted. No end marker found.");
			printf("\nParsing failed");
			return 1;
		}




		/* If input symbol is a valid token */
		else if(isTerminal(nexttoken,parseTable,parseTable.termcount))
		{

			topSymbol = clean(stack[top]);

			/* If top of the stack is a terminal */
			if(isTerminal(topSymbol,parseTable,parseTable.termcount))
			{

				/* If input token and top of stack terminal  match */

				if(strstr(nexttoken,topSymbol) != NULL)
				{
					/* If token = terminal = end of input markers, job's done*/

					if(strstr("$",topSymbol) != NULL)
					{
						return 0;
					}

					/* Else pop the terminal off the stack and read in next input symbol*/
					pop(stack,&top);
					nexttoken = clean(getNextSymbol(input,&seen));
				}

				/* Input token and top-of-stack do not match error*/
				else
				{
					printf("\nError top of stack terminal = %s , input token %s",topSymbol,nexttoken);
					return 1;
				}
			}

			/* If top of the stack is a variable, find action from the table.
			 * If no action is found either the
			 */

			else
			{
				action = getAction(parseTable,topSymbol,nexttoken);
				printf(" %20s",action);

				if(action == NULL)
				{
					printf("\n%s:%d  Fatal error.Function getAction() returned NULL.",__FILE__,__LINE__);
					return 1;
				}


				/* If action returned is the string '(null)' , time to do error recovery.
				 * @TODO: Insert error recovery routine below.
				 */
				else if(strstr(action,"(null)") != NULL)
				{
					printf("\nError unexpected token %s",nexttoken);
					return 1;
				}

				/*
				 * If the action string is 'epsilon' we need to apply a null production here.
				 * Just pop off the terminal on top of the stack.
				 */

				else if(strstr(action,"epsilon") != NULL)
				{
					pop(stack,&top);
				}


				/*
				 * Any other action.
				 */

				else
				{
					/* Pop stack variable */
					pop(stack,&top);

					/* Stack for reversing the order of symbols in this production */
					char **auxstack = NULL;
					int  auxtop ;

					int sofar = 0;
					/* Push all symbols in this action onto an auxiliary stack */
					for( ; ;)
					{
						productionSymbol = getNextSymbol(action,&sofar);
						//printf("\nProduction symbol is %s",productionSymbol);
						if(productionSymbol == NULL) break;
						//printf("\nPushing %s onto auxiliary stack",productionSymbol);
						auxstack = push(auxstack, productionSymbol ,&auxtop);
						//printf("\nTop of auxiliary stack is %s",auxstack[auxtop]);
					}

					/* Push symbols from auxiliary stack to parsing stack and free the auxiliary stack*/

					for(int i= auxtop; i>=0  ; i--)
					{
						stack = push(stack, auxstack[i] ,&top);

					}

					free(auxstack);

				}

			}


		}



		else
		{
			printf("\nUnknown input %s is neither terminal nor end-marker",nexttoken);
			break;
		}

	}
	return 1;
}

char **push(char** stack , char *value , int *top )
{
	char **oldptr;

	/*
	 * If empty array initialize top to 0. Otherwise increment top. Add a new row and copy 'value' pointer.
	 *
	 */

	if(stack == NULL)
	{
		*top = 0;

	}

	else
	{
		(*top)++;
	}

	oldptr = stack;
	stack = addRow(stack,(*top)+1);

	if(stack == NULL) free(oldptr);

	else
	{
		stack[*top] = value;
	}

	return stack;

}


char **pop(char** stack , int *top )
{
	char **oldptr;
	assert(top >= 0);

	/* Do not free up 'value' pointer. It is used by the parse table */
	/* Just reduce the size of the stack one notch */

	oldptr = stack;
	stack = (char**)malloc(sizeof(char*) * (*top));

	if(stack == NULL) free(oldptr);

	(*top)--;

	if(*top == -1) free(stack);

	return stack;
}


_Bool isTerminal(char *symbol, struct parseinfo parseTable , int tableSZ)
{
	int 	i;
	char 	*tmp;

	for(i=0;i<tableSZ;i++)
	{
		//printf("\nNow comparing input token %s with terminal %s",symbol,parseTable.terminals[i]);

		symbol = clean(symbol);
		tmp    = clean(parseTable.terminals[i]);
		if(strstr( tmp,symbol   ) != NULL)
		{
		//	printf("\nMatch found.");
			return true;
		}
		//printf("\t Diff = %d",strcmp(tmp , symbol  ));
	}

	//printf("\nMatch not found.");
	return false;
}


char *getAction(struct parseinfo table, char *variable , char *token)
{
	int i,j;

	for(i=0 ; i< table.varcount ; i++)
	{
		if( strstr(clean(table.variables[i]) , clean(variable)) != NULL)
		{
			for(j = 0 ; j< table.termcount ; j++)
			{

				if(strstr(table.terminals[j],token ) != NULL)
				{
					return table.actions[i][j];
				}
			}

			printf("\n%s:%d  Warning terminal %s not found.",__FILE__,__LINE__,token);
			return NULL;
		}
	}

	/* If we reach here neither the terminal nor the variable was found */
	printf("\n%s:%d  Warning terminal %s not found.",__FILE__,__LINE__,token);
	return NULL;
}
