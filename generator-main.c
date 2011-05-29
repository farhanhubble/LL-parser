
#include"generator-common.h"


int main(int argc , char ** argv)
{
	const char 			*grammarFile = "grammar.txt";
	char 				*grammarArray = NULL;
	char				*tokenSet = NULL;
	int 				count;
	int 				retval;
	struct prodList 	**grammarTable = NULL;
	int 				number_of_variables = 0;
	int					error;
	int 				i,j,k;
	char				tmp[100];
	struct parseinfo	parseTable;


	retval = fileread(&grammarArray,grammarFile,&count);

	if(retval)
	{
		printf("\n%s:%d Error reading from file %s.",__FILE__,__LINE__,grammarFile);
		printf("\nWill exit.");
		exit(EXIT_FAILURE);
	}

	printf("\n%d bytes read from file %s",count,grammarFile);
	printf("\nFile contents are:\n");
	printRaw(grammarArray,count);
	printf("\n");

	tokenSet = preProcess(&grammarArray,count);
	assert(tokenSet != NULL);
	assert(grammarArray != NULL);

	grammarTable = makeGrammarTable(grammarArray,&error,&number_of_variables);

	if(error == 1)
	{
		printf("\n%s:%d  Function makeGrammarTable() returned 1",__FILE__,__LINE__);
		printf("\nWill exit now.\n");
		free(grammarArray);
		free(grammarTable);
		exit(EXIT_FAILURE);
	}


	/* Create Token set */

    strsep(&tokenSet,":");

	/* Print token set */
	printf("\n\nTokens(terminal) are\n");

	for(i=0;i<strlen(tokenSet);i++)
	{
		if(tokenSet[i] != ' ')
		{
			for(j=i; 	j<strlen(tokenSet) && tokenSet[j]!= ' ' 	; j++ , i++)
			{
				printf("%c",tokenSet[j]);
			}
			printf("  ");
		}
	}



	/* Calculate FIRST FOLLOW and NULLABLE for the grammar */

	genNullables(grammarTable,number_of_variables,tokenSet);
	genFirst(grammarTable,number_of_variables,tokenSet);
	genFollow(grammarTable,number_of_variables,tokenSet);


	printf("\nTotal number of variables = %d",number_of_variables);
	printf("\n\nGrammar is:");

	for(i=0; i<number_of_variables; i++)
	{


		printf("\n\n\n%s ",grammarTable[i]->head);

		if(grammarTable[i]->nullable == true)
		{
			printf("%-s","   NULLABLE = true;");
		}

		else
		{
			printf("%-s","   NULLABLE = false;");
		}

		sprintf(tmp,"  %s %d; FOLLOW = %s","Number of productions",grammarTable[i]->prodcount,grammarTable[i]->follow);
		printf("%-s",tmp);

		printf("\n|");

		for(j=0;j<grammarTable[i]->prodcount;j++)
		{
			printf("\n|\n|");
			printf("-> %-20s",grammarTable[i]->productions[j]);
			printf("   FIRST = %-20s",grammarTable[i]->first[j]);

		}
	}

	/* Generate Parse Table */

	retval = genParseTable(&parseTable,grammarTable,number_of_variables,tokenSet);

	if(retval )
	{
		printf("\n%s:%d  Error: Function genParseTable() returned 1\n",__FILE__,__LINE__);
		//exit(EXIT_FAILURE);
	}

	/* Print parse table */
	printf("\n\n************************************** PARSE TABLE ******************************************\n");

	printf("%10s","");
	for(i=0;i<parseTable.termcount;i++)
	{
		printf(" %10s ",parseTable.terminals[i]);
	}

	printf("\n\n");

	for(j=0 ; j<parseTable.varcount ; j++)
	{
		printf("%10s",parseTable.variables[j]);

		for(k=0;k<parseTable.termcount;k++)
		{
			printf(" %10s ",parseTable.actions[j][k]);
		}
		printf("\n");
	}

	if(retval)
	{
		system("rm parsetable.txt");
		exit(EXIT_FAILURE);
	}

	/* Write parse table to file */

	retval = save(parseTable);

	if(retval )
	{
		printf("\n%s:%d  save() returned 1.\n Parse table could not be written to parsetable.txt",__FILE__,__LINE__);
	}


	printf("\n\nExiting..");
	printf("\nHAPPY HAPPY JOY JOY\n");
	return 0;

}
