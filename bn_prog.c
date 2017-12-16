#include <stdio.h>
#include <assert.h>
#include "bn.h"

int main()
{
	bn* a = bn_new();
	bn* b = bn_new();
	bn_init_string(a, "9991234587163254713624");
	bn_init_string(b, "2");
	if (bn_pow_to(a, 999) != BN_OK) assert(0);
	
	bn_dump(a, stdout);
	bn_delete(a);
	bn_delete(b);
	return 0;
}
