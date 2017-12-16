#include <stdio.h>
#include "bn.h"

int main()
{
	bn* a = bn_new();
	bn* b = bn_new();
	
	bn_init_string(a, "57575765657765");
	bn_init_string(b, "8686876876");
	bn_add_to(a, b);
	
	bn_dump(a, stdout);
	bn_delete(a);
	bn_delete(b);
	return 0;
}
