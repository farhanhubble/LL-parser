#include"generator-common.h"



/*
 * @param1: Pointer to structure for parse table.
 * @param2: Grammar table
 * @param3: Number of variables in grammar table
 * @param4: Set of tokens.
 *
 */

int genParseTable(struct parseinfo *parseTable, struct prodList **grammarTable , int var_count , char *tokens)
{
	int 		i,j;
	char 		*next;
	int			sofar;
	int			seen;
	char		*more;
	int 		ret;

	parseTable->varcount 	= var_count;
	parseTable->actions 	= NULL;
	parseTable->terminals 	= NULL;
	parseTable->variables 	= (char**)malloc(sizeof(char*) * var_count);

	for(i=0 ; i< var_count ; i++)
	{

		parseTable->variables[i] = grammarTable[i]->head;
	}


	sofar = 0;

	for( i =0 ; ; i++)
	{
		next = getNextSymbol(tokens,&sofar);

		if(next == NULL)
		{
			parseTable->terminals    = addRow(parseTable->terminals,i+1);

			parseTable->terminals[i] = addSymbolToRow(parseTable->terminals[i],"$");
			i++;
			break;

		}


		if( NULL != strstr(next,"epsilon") ) { i -- ;continue ;}	/* Do not add 'epsilon' to the list of terminals */

		parseTable->terminals    = addRow(parseTable->terminals,i+1);


		parseTable->terminals[i] = next;
	}

	parseTable->termcount = i;



	/* Insert action strings into the parse table */

	/* Do memory allocation first */
	parseTable->actions = (char***)malloc(sizeof(char**) * (parseTable->varcount));

	for(j=0 ;j<parseTable->varcount ; j++)
	{
		parseTable->actions[j] = (char**)malloc(sizeof(char*) * parseTable->termcount);
	}

	/* Initialize all actions to NULL */
	for(i=0 ; i< parseTable->varcount ; i++)
	{
		for(j=0 ; j< parseTable->termcount ; j++)
		{
			parseTable->actions[i][j] = NULL;
		}
	}



	/* Do real installation of actions */
		for(i=0; i< var_count ;i++) /* For every variable X */
	{
		for (j=0 ; j<grammarTable[i]->prodcount ; j++)	/* For every production  X -> @ */
		{
			sofar = 0;

			for( ; ;)	/* For every  terminal t in FIRST[@] */
			{
				next = getNextSymbol(grammarTable[i]->first[j] , &sofar);

				if(next == NULL) break;

				else if( strstr(next,"epsilon") != NULL) /* t = 'epsilon' */
				{

					seen = 0;

					for( ; ;)	/* For every terminal s in FOLLOW[X] */
					{
						more = getNextSymbol(grammarTable[i]->follow,&seen);

						if(more == NULL) break;

						/* Install Action X->@ in TABLE[X,s] */

						ret = setAction(parseTable,grammarTable[i]->head,more,grammarTable[i]->productions[j]);

						if(ret) return 1;




					}

				}

				else /* Install X->@ in TABLE[X,t] */
				{

					ret = setAction(parseTable,grammarTable[i]->head,next,grammarTable[i]->productions[j]);

					if(ret) return 1;

				}


			}	/* END: For every  terminal t in FIRST[@] */


		}	/* END: For every production  X -> @ */


	}	/* END: For every variable X */

		return 0;
}



/*
 * @function setAction() : Installs an action (variable -> targetProduction) into the row for
 * 'variable' and column for 'terminal'.
 * @param1 parseTable
 * @param2 a variable
 * @param3 a terminal
 * @param4 the action to add
 */

int setAction(struct parseinfo *parseTable,char* variable, char *terminal,char *target_production )
{
	int i,j;

	for(i=0 ; i<parseTable->varcount ; i++)
	{
		if(strstr (parseTable->variables[i] , variable) != NULL )
		{
			break;
		}
	}

	if( i == parseTable->varcount)  	/* Variable not found */
	{
		printf("\n%s:%d Variable %s not found",__FILE__,__LINE__,variable);
		return 1;
	}

	for(j=0 ; j< parseTable->termcount ; j++)
	{

		if(strstr (parseTable->terminals[j] , terminal) != NULL)
		{
			break;
		}
	}

	if( j== parseTable->termcount) 	/* Terminal not found */
	{
		printf("\n%s:%d  Terminal %s not found",__FILE__,__LINE__,terminal);
		return 1;
	}

	/* Install target production */
	if(parseTable->actions[i][j] == NULL)
	{
		parseTable->actions[i][j] = target_production;
	}

	else
	{
		printf("\n\nAction conflict!");
		printf("\nFor variable \"%s\" token \"%s\"",variable,terminal);
		printf("\nTable already has action \"%s\"",parseTable->actions[i][j]);
		printf("\nCan not install multiply defined entries");
		printf("\nYOUR GRAMMAR IS AMBIGUOUS.\n");
		return 1;
	}

#ifdef DEBUG
	printf("\nInstalling Table[%10s %10s]     = %-20s",variable,terminal,target_production);
#endif

	return 0;

}
