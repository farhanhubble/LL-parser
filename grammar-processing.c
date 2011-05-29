#include"generator-common.h"



/*
 * Creates a table for grammar productions.
 * Arg1 = Char array of grammar
 * Arg2 =
 */

struct prodList **makeGrammarTable(char *grammar, int *errornum,int *nvar)
{
	struct prodList **table = NULL;					/* An array of structures for holding the entire grammar */
	char **productions 	;					/* Holds all productions of a variable */


	char *prodTok 		= NULL;				/* Token containing all production for one variable */
	char *bodyTok 		= NULL;				/* Token containing bodies of all productions for one variable*/
	char *headTok		= NULL;				/* Token containing head of a set of productions*/
	char *perProdTok    = NULL;				/* Token containing body of one productions */
	char *symbolTok 	= NULL;				/* Token containing all symbols in body of one production */

	char *strToTokenize = NULL;

	const char *prodDelim 			= ";";
	const char *bodyDelim 			= ":";
	const char *perProdDelim		= "|";
	const char *symbolDelim			= " ";

	struct prodList *NEW;

	int nvariables = 0;
	int nprod;
	int i;
	int j;
	char *saveptr1 ,*saveptr2=NULL,*saveptr3=NULL,*saveptr4=NULL;


	assert(table == NULL);
	assert(*nvar == 0);


	for( i=0 , strToTokenize = grammar	; ;	i++ , strToTokenize = NULL)
	{

		prodTok = __strtok_r(strToTokenize,prodDelim,&saveptr1);

		if(prodTok == NULL) break;
		nvariables++;

		headTok = __strtok_r(prodTok,bodyDelim,&saveptr2);
		bodyTok = __strtok_r(NULL,bodyDelim,&saveptr2);

		productions = NULL;
		nprod = 0;			/* Initialize number of productions discovered for variable headTok to 0. */

		for(j=0 ; ;  bodyTok = NULL , j++)
		{
			perProdTok = __strtok_r(bodyTok,perProdDelim,&saveptr3);
			if(perProdTok == NULL) break;

			productions = addRow(productions,j+1);	/* Add a new row */
			productions[j] = NULL;					/* Initialize new row pointer to NULL */
			nprod++;

			if(productions == NULL)
			{
			printf("\n%s:%d Could not create new row.",__FILE__,__LINE__);
				*errornum = 1;
				return NULL;
			}


			for( ;  ; perProdTok = NULL)
			{

				symbolTok = __strtok_r(perProdTok,symbolDelim,&saveptr4);
				if(symbolTok == NULL) break;

				symbolTok = clean(symbolTok); /* Removes trailing white spaces */
				productions[j] = addSymbolToRow(productions[j],symbolTok);
			}


		}

		NEW = (struct prodList*)malloc(sizeof(struct prodList));



		/* cleaning is used for removing leading white spaces, that were ignored by strtor_r() */
		NEW->head = clean(headTok);

		NEW->productions = productions;
		NEW->prodcount = nprod;

		table = incrementRows(table,i+1);

		table[i] = NEW;

	}


	*nvar = nvariables;
	*errornum = 0;
	return table;

}



/* Generate nullable */
int genNullables(struct prodList **table,int var_count,char *tokenSet)
{
	int i,j;
	_Bool changed ;
	char *head = NULL;
	int prodcount;
	char *body;

	/* Find all nullable variables */

#ifdef DEBUG
	int runs = 0;
#endif

	for(i=0;i<var_count;i++)	/* For every variable X */
	{
		table[i]->nullable = false;	/* Initialize nullable[X] to false */

	}

	changed = true;

	for(;	changed == true ;)	/* Repeat until changed becomes false */
	{
		changed=false;

#ifdef DEBUG
		runs++;
#endif

#ifdef DEBUG
		printf("\n***************ITERATION = %d***************",runs);
#endif

		for(i=0;i<var_count;i++)			/* For every variable X */
		{
			head = table[i]->head;
			prodcount = table[i]->prodcount;

#ifdef DEBUG
			printf("\nNow examining variable %s",head);
#endif

			for(j=0;j<prodcount;j++)		/* For every production of  variable X */
			{

				body = table[i]->productions[j];

#ifdef DEBUG
				printf("\nproduction %s",body);
#endif

				if(isNullable(body,table,var_count,tokenSet))	/* body is nullable */
				{
#ifdef DEBUG
					printf("\nProduction body %s is nullable",body);
#endif
					if(table[i]->nullable == false) /* But X is not marked nullable */
					{
						table[i]->nullable = true;	/* Mark X as nullable */
						changed = true;

#ifdef DEBUG
						printf("\nVariable %s was set as not-nullable",head);
						printf(".Updated as nullable");
#endif
					}

#ifdef DEBUG
					else printf("\nVariable %s is already set as nullable",head);
#endif

				}
#ifdef DEBUG

				else printf("\nProduction body %s is not nullable. Nothing to do.",body);
#endif


			}


		}


	}


	return 0;
}



/*
 * Generate FIRST set  for all symbols
 * Arg1 = Grammar table
 * Arg2 = Number of variables in  grammar
 * Arg3 = List of tokens
 */
int genFirst(struct prodList** table,int var_count,char* tokenSet)
{
	int i,j,k,l;
	_Bool changed ;
	char *head = NULL;
	int prodcount;
	char *body;
	char *next;
	int test = 0;


	for(i=0;i<var_count;i++)	/* For every variable X */
	{
		table[i]->first = (char**) malloc(sizeof(char) * (var_count));

		for(j=0 ; j< table[i]->prodcount ; j++) /* For every production of X */
		{
			table[i]->first[j] = NULL;	/* Initialize the FIRST set of this production to empty */
		}

	}

	changed = true;

	for(; changed == true ; )	/* Repeat until changed becomes false */
	{
		changed = false;

		for(i=0 ; i < var_count ; i++)	/* For every variable X */
		{
			head = table[i]->head;
			prodcount = table[i]->prodcount;

			for(j=0 ; j< prodcount ;j++)	/* For every production of X */
			{
				body = table[i]->productions[j] ;

				int lastindex = 0;

				for( ; ;)	/* For every symbol in the body of this production of X */
				{
					next = getNextSymbol(body,&lastindex);

					if(next == NULL) break;

					else if(isToken(next,tokenSet))	/* If symbol is a terminal */
					{
						table[i]->first[j] = doUnion(table[i]->first[j],next,&test);	/* Add terminal to FIRST[X} */

						if(test)
						{
							changed = true;
						}
						break;	/* Do nothing more for this production body */

					}

					else	/* If symbol is a variable Y */
					{
						k = indexOf(next,table,var_count);

						assert (k >= 0);

						for(l=0 ; l< table[k]->prodcount ; l++) /* Add FISRT[Y] for every production of Y
																	to FIRST[X] */
						{
							table[i]->first[j] = doUnion(table[i]->first[j] , table[k]->first[l] , &test);

							if(test) changed = true;
						}

						if(table[k]->nullable == false) break; /* If Y is not nullable */
																/* Done with this production of X */

					}


				} /* for( ; ;) */


			}	/* for(j=0 ; j< prodcount ;j++) */


		}	/* for(i=0 ; i < var_count ; i++) */


	}	/* for(; changed == true ; ) */

	return 0;
}



/*
 * Generate follow set for for each variable;
 * Arg1 = grammar table;
 * Arg2 = number of variables;
 * Arg3 = Set of token strings;
 */
int genFollow(struct prodList** table,int var_count,char* tokenSet)
{
	int 	i,j,k,l,a;
	_Bool 	changed ;
	char 	*head = NULL;
	int 	prodcount;
	char	*body;
	char 	*next;
	char 	*lookright;
	int 	test = 0;
	char 	*tmp;

	changed = true;

	table[0]->follow = doUnion(table[0]->follow ,"$",&test);	/* Add end of input marker
														   to FOLLOW of start symbol */

	for( ; changed == true ;) /* As long as 'changed' stays true */
	{
		changed = false;

		for(i=0 ; i< var_count ; i++) /* For every variable X */
		{
			head = table[i]->head;
			prodcount = table[i]->prodcount;

			for(j=0 ; j< prodcount ;j++)	/* For every production of variable X */
			{
				body = table[i]->productions[j] ;

				int lastseen = 0;

				for( ; ;)	/* For every symbol in this production */
				{

					next = getNextSymbol(body,&lastseen);

#ifdef DEBUG
					printf("\nNow examining symbol %s in production %s",next,body);
#endif

					if(next == NULL) break; /* Done processing this production */


					else if(isToken(next,tokenSet))	/* If this symbol is a terminal */
					{
						continue;	/* Do nothing for terminals */
									/* FOLLOW[terminal] = terminal */
					}

					else /* Symbol is a variable Y*/
					{
						int tmplastseen = lastseen;	/* Do not change original last-seen */

						k = indexOf(next,table,var_count);	/* Find Y in the grammar table */

						for( ; ;) /* For every symbol Z to the right of Y in this body */
						{

							lookright = getNextSymbol(body,&tmplastseen); /* Get symbol Z to the right of Y
																in this production */

							if(lookright == NULL) /* If we have reached the end of this production of  X */
							{
								/* Add FOLLOW[X] to FOLLOW[Y] */
								table[k]->follow = doUnion(table[k]->follow,table[i]->follow,&test);

								if(test)
								{
									changed = true;
								}

								break;
							}


							else if(isToken(lookright,tokenSet)) /* Symbol Z is a terminal */
							{
								table[k]->follow = doUnion(table[k]->follow,lookright,&test);

								if(test)
								{
									changed = true;
								}

								break;	/* No symbols after this will contribute to FOLLOW[Y] */

							}

							else /* Symbol Z is a variable */
							{
								l = indexOf(lookright,table,var_count); /* Find Z in the grammar table */

								for(a=0 ; a<table[l]->prodcount ; a++)	/* For every production of Z */
								{
									/** If first set for this production of Z has an epsilon,
									 *  strip off the epsilon.
									 */

									tmp = removeFromSet(table[l]->first[a],"epsilon",NULL);

									table[k]->follow = doUnion(table[k]->follow,tmp,&test);

									if(test)
									{
										changed = true;
									}

								}

								/** If variable Z is not nullable do not look right of Z. */

								if(table[l]->nullable == false) break;
							}


						}	 /* END: For every symbol Z to the right of Y in this body */


					}	/* END : Else symbol is a Variable Y */


				}	/* END: For every symbol in this production */


			}	/* END: For every production of variable X */


		}	/* END: For every variable X */


	}	/* END: As long as 'changed' stays true */

	return 0;
}

