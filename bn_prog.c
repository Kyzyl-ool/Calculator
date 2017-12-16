#include <stdio.h>
#include <assert.h>
#include "bn.h"

int main()
{
	bn* a = bn_new();
	bn* b = bn_new();
	bn_init_string(a, "108098230981029380192830918230981230981203981029380198230918230918230981203981029380129830192830192830918230912830918203981029381029380129380129830129830128301298301298301928309128309182309182309128301928309128309128301298312038102938");
	bn_init_string(b, "1209812309812309812039810239810293801928301923801928301982301982301298301983019283091283091823091283019823019283091823098123098123098123091283018230192830129831");
	//~ bn_init_string(a, "1135874612387451212312312487631987468376451812673458123762");
	//~ bn_init_string(b, "3182653123321231234823465234765");
	if (bn_pow_to(a, 2) != BN_OK) assert(0);
	
	bn_dump(a, stdout);
	bn_delete(a);
	bn_delete(b);
	return 0;
}
