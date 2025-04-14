#include <stdio.h>
#include <stdlib.h>

enum TYPE {SQUARE, RECT,CIR};

typedef struct _shape
{
	float params[3];
	enum TYPE type;
}shape;

//void (*fp[3]) (shape* ps) = {&draw_square, &draw_rect, &draw_circile};
//typedef void (*fp)(shape* ps) drawfn;
//drawfn fp[4] = {&draw_square, &draw_rect, &draw_circile};



#if 0
void draw(shape* ps)
{
	switch (ps->type)
	{
		case SQUARE:
		draw_square(ps); 
		break;
		
		case RECT:
		draw_rect(ps);
		break;
		
		default:
		draw_circile(ps);
		break;
	}
}
#endif

void draw_square(shape* ps)
{
	printf("draw_square \n");
}

void draw_rect(shape* ps)
{
	printf("draw_rect \n");
}

void draw_circile(shape* ps)
{
	printf("draw_circile \n");
}

//void (*fp[3]) (shape* ps) = {&draw_square, &draw_rect, &draw_circile};
//void (*fpx) (shape* ps) = &draw_circile;
void (*fpx[2]) (shape* ps) = {&draw_square, &draw_rect};

void draw(shape* ps)
{
	//(fp[ps->type])(ps);
	//(*fp[ps->type])(ps);
	//(*fpx)(ps);
	(fpx[0])(ps);
	
}

int main()
{
	shape* ps; // = NULL;
	ps->type = RECT;
	draw(ps);
	
	return 0;
}