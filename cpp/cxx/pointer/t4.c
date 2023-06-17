int main( void )
{
	char *a[] = { "socket", "connect", "read", "listen" };
	char **p = a;

	printf ( "1=> %s ", *( p + 3 ) );
	printf ( "2=> %s ", p[ 1 ] );
	printf ( "3=> %c ", *( *( p + 2 ) + 1 ) );
	printf ( "4=> %c\n", p[ 0 ][ 3 ] );

	return 0;
}