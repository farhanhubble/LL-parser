

#include"generator-common.h"

/* Reads contents of file into character buffer */

int fileread(char **ptrDest,const char *fileName,int *bytesRead)
{
	int nbytes;		/* Total number of bytes read from file */
	int nread;		/* Bytes read by one call to fread() */
	int count;		/* Number of times memory was allocated */
	int offset;		/* Offset from start of buffer */
	const int BUF_STEP_SZ = 256;
	char *oldbuffer;
	char *buffer = NULL;
	FILE *fp;

	assert(buffer == NULL);

	fp = fopen(fileName,"rb");	/* rb mode for compatibility with non POSIX systems */

	if(!fp)
	{
		perror(fileName);
		return 1;
	}

	count = 0;
	nbytes = 0;

	for(; ; count++)
	{
		if(feof(fp))
		{
			fclose(fp);
			break;
		}

		oldbuffer = buffer;		/* store pointer to memory allocated so far */
		buffer = (char*)realloc(buffer,(count+1)*BUF_STEP_SZ);		/* Increase size of buffer */

		if(!buffer)
		{
			printf("\nError: Unable to allocate memory.");
			free(oldbuffer);
			fclose(fp);
			return 1;
		}

		offset = count*BUF_STEP_SZ;
		nread = fread(buffer+offset,sizeof(char),BUF_STEP_SZ,fp);

		if(nread == 0)
		{
			if(ferror(fp))
			{
				perror(fileName);
				fclose(fp);
				free(buffer);
				return 1;
			}

		}

		nbytes += nread;


	}

	*bytesRead = nbytes;
	*ptrDest = buffer;
	return 0;
}


/* function for printing a byte array */
/* Array = a;
 * Number of bytes to be printed  = nbytes
 */

void printRaw(char *a,int nbytes)
{
	int i ;

	assert(a != NULL);

	for(i=0;i<nbytes;i++)
	{
		printf("%c",*a);
		a++;
	}

}



/* Adds a new row to a 2d array of characters
 * Arg1 = A pointer to a two dimensional array.
 * Arg2 = New row number
 */
char **addRow(char **arrayPtr,int rowNum)
{
	arrayPtr = (char**)realloc(arrayPtr,rowNum*sizeof(char*));

	return arrayPtr;
}



/* Add a new string to an array of characters
 * Arg1 = array;
 * Arg2 = String to append;
 */

char *addSymbolToRow(char *array,const char *str)
{
	int newlen;


	if(array == NULL)
	{
		newlen = strlen(str) + 2;
		array = (char*)realloc(array,newlen);
		strcpy(array,str);
		array = strcat(array," ");
	}

	else
	{
		newlen = (strlen(array) + strlen(str) +2);

		array = (char*)realloc(array,newlen);
		if(array == NULL)
		{
			printf("\n%s:%d  Error during memory allocation",__FILE__,__LINE__);
			return array;
		}
		//array = strcat(array,str);
		array = strcat(array," ");
		array = strcat(array,str);


	}

	return array;
}


/* Increment number of rows in an array of type struct prodList
 * Arg1 = array
 * Arg2 = new number of rows;
 */

struct prodList **incrementRows(struct prodList **a,int updated_row_count)
{
	a = (struct prodList **)realloc(a , sizeof(struct prodList*) * updated_row_count);
	return a;
}



/* Eat newlines and tabs.
 * Put a trailing NULL character
 */
char *preProcess(char **a,int sz)
{
	char *tokens;

	/* Make the array NULL terminated */

	int i;
	for(i=sz-1; i>=1 ;i--)
	{ /* Skip trailing white spaces */
		if((*a)[i] == ';') break;
	}
	(*a)[++i] = '\0';		/* Overwrite first trailing white space with a NULL */

	for(i=0;i<strlen(*a);i++)
	{
		if((*a)[i] == '\t' || (*a)[i] == '\n')
		{
			(*a)[i] = ' ';
		}
	}

	tokens = strsep(a,"%");

	return tokens;

}










/*
 * Tests if  a string is nullable
 * Arg1 =  A production body.
 * Arg2 = Grammar table.
 * Arg3 = Number of variables in the grammar table
 * Arg4 = Array of all tokens.
 */
_Bool isNullable(char *body,struct prodList **table,int no_of_variables,char *tokens)
{

	int i;
	int lastindex = 0;
	_Bool retval = true;
	int j;
	char *next;

	if( strncmp(body,"epsilon",7) == 0)	/* If body contains nothing but epsilon */
	{
		return true;
	}

	for(i=0;	;i++)
	{
		next = getNextSymbol(body,&lastindex);

		if( next == NULL) break;

		if(isToken(next,tokens))	/* If symbol is a token, body can not be nullable */
		{
			retval = false;
			break;
		}

		else	/* If symbol is a variable */
		{
			j = indexOf(next,table,no_of_variables);	/* Index of this variable in the grammar table */
			assert(j>=0);
			if(table[j]->nullable == false)
			{

				retval = false;
				break;
			}
		}
	}


	return retval;
}


/*
 * Returns next symbol from an array holding a production body.
 * A
 */

char *getNextSymbol(char *body,int *lastindex)
{
	int i,j;
	char *retval = NULL;
	int len;

	if(body[*lastindex] == '\0')	/* No more symbols to extract ? Return NULL */
	{
		return retval;
	}

	for (i = *lastindex ;  i<strlen(body) ; i++)
	{
		if(body[i] != ' ') break;
	}

	for(j=i+1; j<strlen(body) ;j++)
	{
		if(body[j] == ' ') break;

	}

	len = j-i+1;

	retval = (char*)malloc(sizeof(char) * (len + 1));
	strncpy(retval,body+i,len);
	retval[len] = '\0';

	(*lastindex) = j;

	if(strcmp (retval,"") == 0) return NULL;	/* Never return  an empty string */
	return retval;

}


_Bool isToken(char *sym,char *toks)
{
	if( strstr(toks,sym) == NULL )
	{

		return false;
	}


	return true;
}



int indexOf(char *variable,struct prodList **table,int tablelen)
{
	int i;

	for(i=0;i<tablelen;i++)
	{
		if(	strstr(table[i]->head,variable) != NULL)
		{

#ifdef DEBUG
			printf("\nindexOf(%s) returned %d",variable,i);
#endif

			return i;

		}
	}


	return -1;
}


/*
 * Add a new element (string) to a space separated set of strings if it not already in the set.
 * Arg1 = set
 * Arg2 = element to add
 * Arg3 = Used to indicate if the set changed as a result of this call
 */
char *addToSet(char *set, char * element, int *changed)
{
	char *retval = set;

#ifdef DEBUG
	printf("\nset = %s ; element = %s ; ",set,element);
#endif


	if(set == NULL) /*  strstr() can not handle NULL as first argument.*/
	{
		if(element == NULL)
		{
			*changed = 0;
		}

		else
		{
			retval =  addSymbolToRow(set,element);
			*changed = 1;
		}

	}

	else
	{
		if(element == NULL)
		{
			*changed = 0;
		}

		else
		{
			if(strstr(set,element) == NULL)
			{
				retval = addSymbolToRow(set,element);
				*changed = 1;
			}

			else
			{
				*changed = 0;
			}
		}
	}

#ifdef DEBUG
	printf("returning %s",retval);
#endif

	return retval;
}



/* Union of two sets(strings).
 * Each set has members in the form of substrings separated by white spaces.
 *
 * Arg1 = String1
 * Arg2 = String2
 */


char *doUnion(char *set1, char *set2,int *changed)
{
	char *more;
	char *ret = set1;
	int marker = 0;
	int tmp;



	if(set2 == NULL)
	{
		*changed = 0;
		return set1;
	}

	for( ; ;)
	{
		more = getNextSymbol(set2,&marker);

		if(more == NULL) break;

		ret = addToSet(ret,more,&tmp);
		*changed = tmp;

	}

	return ret;

}




char *removeFromSet(char* set,char *element,int *setchanged)
{
	char *ret = set;
	int changes;
	int index = -1;
	char *before = NULL ,*after = NULL;
	int i;

	//printf("\n[ Set = %s ] [element = %s ]",set,element);
	/* If set is no removal is required */
	if(set == NULL)
	{
		changes = 0;

	}


	else
	{
		/* If element to be removed is empty, do nothing */
		if(element == NULL)
		{
			changes = 0;

		}

		else
		{
			/* Find the offset of element in set */
			int j = strlen(set)-strlen(element)+1;
			for(i=0 ; i < j ; i++)
			{
//
				if(strncmp(set+i,element,strlen(element)) == 0)
				{
					index = i;
					break;

				}
			}

			/* If element not found in set */
			if(index == -1) changes = 0;

			/* If element found in set, extract subsets before and after of set and merge them*/
			/* Index is the offset at which element was found */
			else
			{
				before = strndup(set,index);
				after =  strdup(set+index+strlen(element));

				ret = before;
				ret = strcat(ret,after);
				changes = 1;

			}


		}


	}

	if(setchanged) *setchanged = changes;
	//printf("Returning %-s",ret);
	return ret;

}

/*
 * Writes Parse Table to a file
 * @PARAM: Parse Table Structure.
 * @TODO: Use XML instead of proprietary format when saving to file.
 */


int save(struct parseinfo parseTable)
{
	int 	i;
	int 	j;
	FILE 	*fp;

	fp = fopen("parsetable.txt","w+");

	if(!fp)
	{
		perror("parsetable.txt");
		return 1;
	}

	fprintf(fp,"%d\n",parseTable.varcount);


	for(i=0 ; i<parseTable.varcount ; i++)
	{
		fprintf(fp,"%s\n",parseTable.variables[i]);
	}

	fprintf(fp,"%d\n",parseTable.termcount);

	for(i=0 ; i<parseTable.termcount ; i++)
	{
		fprintf(fp,"%s\n",parseTable.terminals[i]);
	}

	for(i=0 ; i<parseTable.varcount ; i++)
	{
		for(j=0 ; j<parseTable.termcount ; j++)
		{
			if(parseTable.actions[i][j] != NULL)
			{
				fprintf(fp,"%s\n",parseTable.actions[i][j]);
			}

			else fprintf(fp,"%s\n","(null)");
		}
	}

	fclose(fp);

	return 0;
}


char *clean(char *dirty)
{
	int tmp = 0;
	return(getNextSymbol(dirty,&tmp));
}
