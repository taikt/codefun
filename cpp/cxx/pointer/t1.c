int main( void )
{
	int a = 10;    // assume address of a is 100
	int *p = &a;   // assume address of p is 200

	scanf( "%d", p ); // assume input 20

	printf( "a = %d, &a = %p, p = %p, &p = %p, *p = %d\n", a, &a, p, &p, *p );

	return 0;
}
