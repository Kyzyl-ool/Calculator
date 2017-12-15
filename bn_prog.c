#include <stdio.h>
#include "bn.h"


int main()
{
	bn* a = bn_new();
	bn* b = bn_new();
	bn_init_int(a, 999);
	bn_init_int(b, 123);
	bn_div_to(a, b);
	
	bn_dump(a, stdout);
	bn_delete(a);
	//~ bn_dump(b, stdout);
	bn_delete(b);
	return 0;
}
