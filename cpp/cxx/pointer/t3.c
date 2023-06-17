int main( void )
{
	char cVal = 'A'; char *p = &cVal;
	const char * const *q = ( const char * const * )p;
	const char * const *r = ( const char * const * )p;
	char * const * const s = ( char * const * const )r;
	char * const * const t = ( char * const * const )r;

	r = q;		 /* A */
	*q = p;		 /* B */
	**q = 'B';	 /* C */
	t = s;		 /* D */
    
    return 0;
}
