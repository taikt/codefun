int main()
{
	double a[] = { 0.1, 0.2, 0.3, 0.4, 0.5 };
	void *vp = a; 

	vp = ( double * )vp + 3;

	printf ( "%.1lf\n", *( double * )vp  + 1 );

	return 0;
}