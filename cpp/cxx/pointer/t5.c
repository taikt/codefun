int main( void )
{
	char a[7], b[100];

	strcpy( a, "abc" );
	strcpy( b, "abc" );

	memcpy( a + 4, "def", 3 );
	memcpy( b + 4, "dfe", 3 );

	if ( 0 == strcmp( a, b )  )
		printf( "Strcmp Equal\n" );
	else
		printf( "Strcmp Not-Equal\n" );

	if ( 0 == memcmp( a, b, 5 ) )
		printf( "Memcmp Equal\n" );
	else
		printf( "Memcmp Not-Equal\n" );

	return 0;
}
