#include <stdio.h>
#include "bn.h"


int main()
{
	bn* a = bn_new();
	bn* b = bn_new();
	bn_init_int(a, 123);
	bn_init_int(b, 0);
	bn_add_to(a, b);
	
	bn_dump(a, stdout);
	bn_delete(a);
	//~ bn_dump(b, stdout);
	bn_delete(b);
	return 0;
}
