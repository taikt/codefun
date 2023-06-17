int main( void )
{
	typedef short int ( SPTR )[ 2 ];
	SPTR *p, *q; 
	char a2D[ 5 ][ 3 ] = { { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 }, { 10, 11, 12 }, { 13, 14, 15 } };
	
	p = q = ( short int (*)[2] )a2D[1];

	printf ( "%04hX,%04hX\n", ( *++p )[ 1 ],  ( q[1] )[2] );

	return 0;
}