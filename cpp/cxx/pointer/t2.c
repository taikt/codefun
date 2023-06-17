int main( void )
{
	unsigned char aubyData[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A };
	unsigned short int *p; 
	unsigned char *r;

	p = ( unsigned short int * )&aubyData[ 0 ];
	r = ( unsigned char * )&aubyData[ 8 ];

	/* [A] => */ printf( "%08X\n", *( p + 2 ) );
	/* [B] => */ printf( "%08X\n", *( (int *)p + 1 ) );
	/* [C] => */ printf( "%08X\n", ( (int *)p )[ 1 ] );
	/* [D] => */ printf( "%08X\n", *( int * )( (char*)p + 4 ) );
	/* [E] => */ printf( "%08X\n", *( int * )( r - 4 ) );
	/* [F] => */ printf( "%08X\n", *( char *)( ( int * )r - 1 ) );
	/* [G] => */ printf( "%08X\n", *( int * )( ( short int * )r - 2 ) );
  
    return 0;
}
