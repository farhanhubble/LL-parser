#include"parser-common.h"



int 			load(struct parseinfo *table);
int 			parse(struct parseinfo parseTable, char *);



int main(int argc, char **argv)
{
	struct parseinfo 		parseTBL;
	int 					ret;
	int						i,j,k;
	char					inputFile[256];
	char 					*inputBuf = NULL;
	int						nbytes;
	FILE 					*fp;


	ret = load(&parseTBL);

	if(ret)
	{
		printf("\n%s:%d  load() returned 1.",__FILE__,__LINE__);
		printf("\nPerhaps the file has been deleted, renamed or moved.\n");
		exit(EXIT_FAILURE);
	}

	/* Print the parsing table */
	printf("\n\n************************************** PARSE TABLE ******************************************\n");

	printf("%10s","");
	for(i=0;i<parseTBL.termcount;i++)
	{
		printf(" %10s ",parseTBL.terminals[i]);
	}

	printf("\n\n");

	for(j=0 ; j<parseTBL.varcount ; j++)
	{
		printf("%10s",parseTBL.variables[j]);

		for(k=0;k<parseTBL.termcount;k++)
		{
			printf(" %10s ",parseTBL.actions[j][k]);
		}
		printf("\n");
	}



	printf("\nEnter name of the file you wish to parse>>");
	scanf("%s",inputFile);
	fp = fopen(inputFile,"r");
	if(!fp)
	{
		printf("\n%s:%d  ",__FILE__,__LINE__);
		perror(inputFile);
		exit(EXIT_FAILURE);
	}

	/*
	 * @TODO: Implement incremental read from input file.
	 * Perhaps through double buffering.
	 */
	fileread(&inputBuf, inputFile , &nbytes );

	for(i=0 ; i< nbytes-1 ; i++)
	{
		if(inputBuf[i] == '\t' || inputBuf[i] == '\n' )
		{
			inputBuf[i] = ' ';
		}

		if(inputBuf[i] == '$')
		{
			inputBuf[i+1] = '\0';
		}
	}





	ret = parse(parseTBL,inputBuf);

	if(ret == 0)
	{
		printf("\nParsing completed successfully.");
	}

	else if(ret == 1)
	{
		printf("\n%s:%d  Function parse() returned %d.Parse could not complete",__FILE__,__LINE__,ret);
		exit(EXIT_FAILURE);
	}


	printf("\n*** HAPPY HAPPY JOY JOY! ***\n");
	exit(EXIT_SUCCESS);
}
