void fun( char **p )
{
	char *t;
	t = ( p += sizeof(  *( short ** )p )  )[ -1 ];
	printf ( "%s\n", &0[ t ] );
}

int main( void )
{
	char *argv[] = { "ab", "cd", "ef", "gh", "ij", "kl", "mn", "op", "qr" };
	char **p = argv;
	fun ( ( p + 1, p ) );

	return 0;
}