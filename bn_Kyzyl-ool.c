#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#ifdef BN
#include "bn.h"
#endif

#define DEFAULT_SIZE 8
#define DYNAMIC_STACK_POISON -111
//~ #define DEBUG
#define _RED_DUMP(the_bn) printf("\033[1;31m"); bn_dump(the_bn, stdout); printf("\033[0m")
#define _GREEN_DUMP(the_bn) printf("\033[1;32m"); bn_dump(the_bn, stdout); printf("\033[0m")
#define _BEEP printf("\a")
#define _BN_ASSERT(the_bn) assert(the_bn); assert(the_bn->body)
#define _NO_BN(the_bn) !the_bn || !the_bn->body
#define _BN_STABILIZE(t) for (int i = 0; i < t->amount_of_allocated_blocks*DEFAULT_SIZE; i++) {if (t->body[i] > 9) { t->body[i+1] += t->body[i] / 10; t->body[i] %= 10; } while (t->body[i] < 0) { t->body[i] += 10; t->body[i+1]--; } } t->bodysize = 0; for (int i = 0; i < t->amount_of_allocated_blocks*DEFAULT_SIZE; i++) { if (t->body[i] != 0) t->bodysize = i + 1; } if (t->bodysize == 0) { t->sign = 0; t->bodysize = 1;}

#define DYNAMIC_STACK_DEFAULT_SIZE 256

typedef char dynamic_stack_t;
typedef struct dynamic_stack
{
	dynamic_stack_t* data;
	int current;
	int amount_of_allocated_blocks;
} dynamic_stack;

typedef enum dynamic_stack_error_codes
{
	DYNAMIC_STACK_OK,
	DYNAMIC_STACK_NO_MEMORY,
	DYNAMIC_STACK_OVERFLOW,
	DYNAMIC_STACK_FULL,
	DYNAMIC_STACK_POP_FROM_EMPTY,
	DYNAMIC_STACK_OK_BUT_ONE_FREE_BLOCK,
	
} dynamic_stack_error_code;


dynamic_stack* dynamic_stack_Construct();
int dynamic_stack_Destroy(dynamic_stack* ds);

int dynamic_stack_Dump(dynamic_stack* ds, FILE* fout, const char* dynamic_stack_name);
int dynamic_stack_check(dynamic_stack* ds);
const char* dynamic_stack_meaning_of_error_code(int error_code);
void dynamic_stack_error_print(int error_code);

int dynamic_stack_Push(dynamic_stack* ds, dynamic_stack_t value);
dynamic_stack_t dynamic_stack_Pop(dynamic_stack* ds);

int is_full(dynamic_stack* ds);
int dynamic_stack_expand(dynamic_stack* ds);
int dynamic_stack_reduce(dynamic_stack* ds);


dynamic_stack* dynamic_stack_Construct()
{
	dynamic_stack* ds = (dynamic_stack* )calloc(1, sizeof(dynamic_stack));
	if (!ds)
		return NULL;
	
	ds->current = 0;
	ds->amount_of_allocated_blocks = 1;
	ds->data = (dynamic_stack_t* )calloc(DYNAMIC_STACK_DEFAULT_SIZE, sizeof(dynamic_stack_t));
	for (int i = 0; i < ds->amount_of_allocated_blocks*DYNAMIC_STACK_DEFAULT_SIZE; i++)
		ds->data[i] = DYNAMIC_STACK_POISON;
	
	return ds;
}

int dynamic_stack_Destroy(dynamic_stack* ds)
{
	#ifdef DEBUG
	assert(ds);
	#endif
	if (ds->data)
		free(ds->data);
	free(ds);

	return DYNAMIC_STACK_OK;
}

int dynamic_stack_Dump(dynamic_stack* ds, FILE* fout, const char* dynamic_stack_name)
{
	fprintf(fout, "Dynamic stack \"%s\"[%s] dump:\n{\n", dynamic_stack_name, dynamic_stack_meaning_of_error_code(dynamic_stack_check(ds)));
	fprintf(fout, "	current: %d\n	blocks: %d\n\n", ds->current, ds->amount_of_allocated_blocks);
	for (int i = 0; i < ds->amount_of_allocated_blocks*DYNAMIC_STACK_DEFAULT_SIZE; i++)
		if (ds->data[i] != DYNAMIC_STACK_POISON)
			fprintf(fout, "	data[%d]:	%d\n", i, ds->data[i]);
		else
			fprintf(fout, "	data[%d]:	DYNAMIC_STACK_POISON\n", i);
	fprintf(fout, "}\n\n");
	
	return 0;
}

int dynamic_stack_Push(dynamic_stack* ds, dynamic_stack_t value)
{
	#ifdef DEBUG
	assert(ds);
	assert(ds->data);
	#endif
	
	ds->data[ds->current] = value;
	ds->current++;
	
	return dynamic_stack_check(ds);
}

dynamic_stack_t dynamic_stack_Pop(dynamic_stack* ds)
{
	#ifdef DEBUG
	assert(ds);
	assert(ds->data);
	#endif
	if (ds->current < 1)
	{
		#ifdef DEBUG
		dynamic_stack_error_print(DYNAMIC_STACK_POP_FROM_EMPTY);
		assert(0);
		#endif
		return DYNAMIC_STACK_POISON;
	}
	ds->current--;
	dynamic_stack_t tmp = ds->data[ds->current];
	ds->data[ds->current] = DYNAMIC_STACK_POISON;
	return tmp;
}

int is_full(dynamic_stack* ds)
{
	
	if (ds->current == ds->amount_of_allocated_blocks*DYNAMIC_STACK_DEFAULT_SIZE)
		return DYNAMIC_STACK_FULL;
	else if (ds->current > ds->amount_of_allocated_blocks*DYNAMIC_STACK_DEFAULT_SIZE)
		return DYNAMIC_STACK_OVERFLOW;
	else if (ds->current < ds->amount_of_allocated_blocks*DYNAMIC_STACK_DEFAULT_SIZE && ds->current >= (ds->amount_of_allocated_blocks - 1)*DYNAMIC_STACK_DEFAULT_SIZE)
		return DYNAMIC_STACK_OK;
	else
		return DYNAMIC_STACK_OK_BUT_ONE_FREE_BLOCK;
}

int dynamic_stack_check(dynamic_stack* ds)
{
	switch (is_full(ds))
	{
		case DYNAMIC_STACK_FULL:
		{
			dynamic_stack_expand(ds);
			break;
		}
		case DYNAMIC_STACK_OK:
		{
			break;
		}
		case DYNAMIC_STACK_OVERFLOW:
		{
			#ifdef DEBUG
			dynamic_stack_error_print(DYNAMIC_STACK_OVERFLOW);
			assert(0);
			#endif
			return DYNAMIC_STACK_OVERFLOW;
			break;
		}
		case DYNAMIC_STACK_OK_BUT_ONE_FREE_BLOCK:
		{
			dynamic_stack_reduce(ds);
			break;
		}
		default: assert(0);
	}
	
	return DYNAMIC_STACK_OK;
}

int dynamic_stack_expand(dynamic_stack* ds)
{
	dynamic_stack_t* tmp = (dynamic_stack_t* )malloc(ds->amount_of_allocated_blocks*DYNAMIC_STACK_DEFAULT_SIZE*sizeof(dynamic_stack_t));
	for (int i = 0; i < ds->amount_of_allocated_blocks*DYNAMIC_STACK_DEFAULT_SIZE; i++)
		tmp[i] = ds->data[i];
	free(ds->data);
	ds->amount_of_allocated_blocks++;
	ds->data = (dynamic_stack_t* )malloc(ds->amount_of_allocated_blocks*DYNAMIC_STACK_DEFAULT_SIZE*sizeof(dynamic_stack_t));
	for (int i = 0; i < (ds->amount_of_allocated_blocks - 1)*DYNAMIC_STACK_DEFAULT_SIZE; i++)
		ds->data[i] = tmp[i];
	for (int i = (ds->amount_of_allocated_blocks - 1)*DYNAMIC_STACK_DEFAULT_SIZE; i < ds->amount_of_allocated_blocks*DYNAMIC_STACK_DEFAULT_SIZE; i++)
		ds->data[i] = DYNAMIC_STACK_POISON;
	return dynamic_stack_check(ds);
}

const char* dynamic_stack_meaning_of_error_code(int error_code)
{
	switch (error_code)
	{
		#define _RET_CODE(code) case code: return #code
		
		_RET_CODE(DYNAMIC_STACK_OK);
		_RET_CODE(DYNAMIC_STACK_NO_MEMORY);
		_RET_CODE(DYNAMIC_STACK_OVERFLOW);
		_RET_CODE(DYNAMIC_STACK_POP_FROM_EMPTY);
		_RET_CODE(DYNAMIC_STACK_FULL);
		_RET_CODE(DYNAMIC_STACK_OK_BUT_ONE_FREE_BLOCK);
		default: return "DYNAMIC_STACK_UNDEFINED_ERROR";
		
		#undef _RET_CODE
	}
}

void dynamic_stack_error_print(int error_code)
{
	printf("%s\n", dynamic_stack_meaning_of_error_code(error_code));
}

int dynamic_stack_reduce(dynamic_stack* ds)
{
	#ifdef DEBUG
	assert(ds);
	assert(ds->data);
	#endif
	dynamic_stack_t* tmp = (dynamic_stack_t* )malloc((ds->amount_of_allocated_blocks - 1)*DYNAMIC_STACK_DEFAULT_SIZE*sizeof(dynamic_stack_t));
	for (int i = 0; i < (ds->amount_of_allocated_blocks - 1)*DYNAMIC_STACK_DEFAULT_SIZE; i++)
		tmp[i] = ds->data[i];
	
	free(ds->data);
	ds->amount_of_allocated_blocks--;
	ds->data = tmp;
	
	return dynamic_stack_check(ds);
}

typedef char body_t;
typedef struct bn_s
{
	body_t* body;
	int bodysize;
	char sign;
	int amount_of_allocated_blocks;
} bn;
enum bn_codes {
   BN_OK = 0,
   BN_NULL_OBJECT,
   BN_NO_MEMORY,
   BN_DIVIDE_BY_ZERO,
   BN_BODY_OVERFLOW,
   BN_BODY_FULL,
   BN_BODY_HAS_EMPTY_BLOCKS
   
};

void bn_expand(bn* t)
{
	t->amount_of_allocated_blocks++;
	body_t* tmp = (body_t* )malloc(t->amount_of_allocated_blocks*DEFAULT_SIZE*sizeof(body_t));
	for (int i = 0; i < (t->amount_of_allocated_blocks - 1)*DEFAULT_SIZE; i++)
		tmp[i] = t->body[i];
	for (int i = (t->amount_of_allocated_blocks - 1)*DEFAULT_SIZE; i < t->amount_of_allocated_blocks*DEFAULT_SIZE; i++)
		tmp[i] = 0;
	free(t->body);
	t->body = tmp;
}

void bn_reduce(bn* t)
{
	//~ t->amount_of_allocated_blocks--;
	//~ body_t* tmp = (body_t* )malloc(t->amount_of_allocated_blocks*DEFAULT_SIZE*sizeof(body_t));
	//~ for (int i = 0; i < t->amount_of_allocated_blocks*DEFAULT_SIZE; i++)
		//~ tmp[i] = t->body[i];
	//~ free(t->body);
	//~ t->body = tmp;
	
	if ((int)(trunc(t->bodysize / DEFAULT_SIZE) + 1) < t->amount_of_allocated_blocks)
	{
		t->amount_of_allocated_blocks = (int)(trunc(t->bodysize / DEFAULT_SIZE) + 1);
		body_t* tmp = (body_t* )calloc(t->amount_of_allocated_blocks*DEFAULT_SIZE, sizeof(body_t));
		for (int i = 0; i < t->bodysize; i++)
			tmp[i] = t->body[i];
		free(t->body);
		t->body = tmp;
	}
}

int bn_size_status(bn* t)
{
	int allocsize = t->amount_of_allocated_blocks*DEFAULT_SIZE;
	if (t->bodysize == allocsize)
		return BN_BODY_FULL;
	else if (t->bodysize > allocsize)
		return BN_BODY_OVERFLOW;
	else if (t->bodysize < (allocsize) && t->bodysize >= (allocsize - DEFAULT_SIZE))
		return BN_OK;
	else
		return BN_BODY_HAS_EMPTY_BLOCKS;
}


int bn_check(bn* t)
{
	switch (bn_size_status(t))
	{
		case BN_BODY_FULL:
		{
			bn_expand(t);
			break;
		}
		case BN_BODY_HAS_EMPTY_BLOCKS:
		{
			bn_reduce(t);
			break;
		}
		case BN_BODY_OVERFLOW:
		{
			#ifdef DEBUG
			assert(0);
			#endif
			return BN_BODY_OVERFLOW;
		}
		default: break;
	}
	return BN_OK;
}


const char* bn_meaning_of_error_code(int error_code)
{
	switch (error_code)
	{
		#define _RET_CODE(code) case code: return #code
		
		_RET_CODE(BN_OK);
		_RET_CODE(BN_DIVIDE_BY_ZERO);
		_RET_CODE(BN_NO_MEMORY);
		_RET_CODE(BN_NULL_OBJECT);
		_RET_CODE(BN_BODY_FULL);
		_RET_CODE(BN_BODY_HAS_EMPTY_BLOCKS);
		_RET_CODE(BN_BODY_OVERFLOW);
		default: return "BN_UNDEFINED_ERROR_CODE";
		
		#undef _RET_CODE
	}
}

void bn_print_error(int error_code)
{
	printf("%s\n", bn_meaning_of_error_code(error_code));
}


bn* bn_new()
{
	bn* new_bn = (bn* )calloc(1, sizeof(bn));
	if (!new_bn)
		return NULL;
	new_bn->body = (body_t* )calloc(DEFAULT_SIZE, sizeof(body_t));
	if (!new_bn->body)
	{
		free(new_bn);
		return NULL;
	}
	new_bn->sign = 0;
	new_bn->bodysize = 1;
	new_bn->amount_of_allocated_blocks = 1;
	
	return new_bn;
}

bn *bn_init(bn const *orig) // Создать копию существующего BN
{
	#ifdef DEBUG
	_BN_ASSERT(orig);
	#endif
	if (_NO_BN(orig))
		return NULL;
	
	bn* new_bn = (bn* )calloc(1, sizeof(bn));
	if (!new_bn)
		return NULL;
	new_bn->amount_of_allocated_blocks = orig->amount_of_allocated_blocks;
	new_bn->bodysize = orig->bodysize;
	new_bn->sign = orig->sign;
	new_bn->body = (body_t* )calloc(new_bn->amount_of_allocated_blocks*DEFAULT_SIZE, sizeof(body_t));
	if (!new_bn->body)
	{
		free(new_bn);
		return NULL;
	}
	for (int i = 0; i < new_bn->amount_of_allocated_blocks*DEFAULT_SIZE; i++)
		new_bn->body[i] = orig->body[i];
	
	return new_bn;
}

// Инициализировать значение BN десятичным представлением строки
int bn_init_string(bn *t, const char *init_string)
{
	#ifdef DEBUG
	_BN_ASSERT(t);
	assert(init_string);
	#endif
	if (_NO_BN(t))
		return BN_NULL_OBJECT;
	
	const char* number = NULL;
	//Считать все цифры из строки и записать в соответствющие разряды
	if (init_string[0] == '-')
	{
		number = &init_string[1];
		t->sign = -1;
	}
	else
	{
		number = init_string;
		t->sign = 1;
	}
	int i = 0;
	while (number[i] != '\0') i++;
	t->bodysize = i;
	t->amount_of_allocated_blocks = trunc(t->bodysize / DEFAULT_SIZE) + 1;
	if (t->body)
		free(t->body);
	
	t->body = (body_t* )calloc(t->amount_of_allocated_blocks*DEFAULT_SIZE, sizeof(body_t));
	if (!t->body)
	{
		return BN_NO_MEMORY;
	}
	for (i = 0; i < t->bodysize; i++) t->body[t->bodysize - 1 - i] = number[i] - '0';
	
	return BN_OK;
}

// Инициализировать значение BN заданным целым числом
int bn_init_int(bn *t, int init_int)
{
	#ifdef DEBUG
	_BN_ASSERT(t);
	#endif
	if (_NO_BN(t))
		return BN_NULL_OBJECT;
	//Брать остаток от деления на 10 и прибавлять к соответствующему разряду
	
	//присвоить t
	
	if (init_int == 0)
	{
		t->body = (body_t* )calloc(DEFAULT_SIZE, sizeof(body_t));
		t->sign = 0;
		t->amount_of_allocated_blocks = 1;
		t->bodysize = 1;
		return BN_OK;
	}
	
	int x = init_int;
	if (x < 0)
	{
		x = -x;
		t->sign = -1;
	}
	else if (x > 0)
		t->sign = 1;
	else
		t->sign = 0;
	
	int i = 0;
	while (x > 0)
	{
		i++;
		x /= 10;
	}
	t->bodysize = i;
	t->amount_of_allocated_blocks = trunc(i / DEFAULT_SIZE) + 1;
	if (t->body)
		free(t->body);
	
	t->body = (body_t* )calloc(t->amount_of_allocated_blocks*DEFAULT_SIZE, sizeof(body_t));
	if (!t->body)
		return BN_NO_MEMORY;
	x = init_int*t->sign;
	for (int i = 0; i < t->bodysize; i++)
	{
		t->body[i] = x % 10;
		x /= 10;
	}
	
	return BN_OK;
}

// Уничтожить BN (освободить память)
int bn_delete(bn *t)
{
	#ifdef DEBUG
	_BN_ASSERT(t);
	#endif
	if (_NO_BN(t))
		return BN_NULL_OBJECT;
	//освобдить body
	//освободить само число
	if (t->body)
		free(t->body);
	free(t);
	
	return BN_OK;
}

// Если левое меньше, вернуть <0 если равны, вернуть 0 иначе  >0
int bn_cmp(bn const *left, bn const *right)
{
	#ifdef DEBUG
	_BN_ASSERT(left);
	_BN_ASSERT(right);
	#endif
	if (_NO_BN(left) || _NO_BN(right))
		return BN_NULL_OBJECT;
	//сравнение знаков
		//если одного знака
			//если положитнльный
				//больше тот, у кого модуль больше
			//иначе
				//больше тот, у кого модуль меньше
		//иначе
			//больше тот, у кого знак положительный
	
	if (left->sign*right->sign == 1)
	{
		if (left->sign > 0)
		{
			if (left->bodysize > right->bodysize)
			{
				return 1;
			}
			else if (left->bodysize < right->bodysize)
			{
				return -1;
			}
			else
			{
				for (int i = 0; i < left->bodysize; i++)
				{
					if (left->body[left->bodysize - 1 - i] > right->body[right->bodysize - 1 - i])
					{
						return 1;
					}
					else if (left->body[left->bodysize - 1 - i] < right->body[right->bodysize - 1 - i])
					{
						return -1;
					}
				}
				return 0;
			}
		}
		else
		{
			if (left->bodysize > right->bodysize)
			{
				return -1;
			}
			else if (left->bodysize < right->bodysize)
			{
				return 1;
			}
			else
			{
				//сравнение поразрядно
				for (int i = 0; i < left->bodysize; i++)
				{
					if (left->body[left->bodysize - 1 - i] > right->body[right->bodysize - 1 - i])
					{
						return -1;
					}
					else if (left->body[left->bodysize - 1 - i] < right->body[right->bodysize - 1 - i])
					{
						return 1;
					}
				}
				return 0;
			}
		}
	}
	else if (left->sign*right->sign == -1)
	{
		if (left->sign > 0)
		{
			return 1;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		if (left->sign > 0)
		{
			return 1;
		}
		else if (left->sign < 0)
		{
			return -1;
		}
		else
		{
			if (right->sign > 0)
			{
				return -1;
			}
			else if (right->sign < 0)
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}
	}
	
	assert(0);
}

int bn_neg(bn *t) // Изменить знак на противоположный
{
	#ifdef DEBUG
	_BN_ASSERT(t);
	#endif
	if (_NO_BN(t))
		return BN_NULL_OBJECT;
	
	t->sign *= -1;
	
	return BN_OK;
}

int bn_abs(bn *t) // Взять модуль
{
	#ifdef DEBUG
	_BN_ASSERT(t);
	#endif
	if (_NO_BN(t))
		return BN_NULL_OBJECT;
	if (t->sign < 0)
	{
		t->sign = 1;
	}
	return BN_OK;
}

int bn_sign(bn const *t) //-1 если t<0; 0 если t = 0, 1 если t>0
{
	#ifdef DEBUG
	_BN_ASSERT(t);
	#endif
	if (_NO_BN(t))
		return BN_NULL_OBJECT;
	
	return t->sign;
}


void bn_dump(bn* b, FILE* f)
{
	#ifdef DEBUG
	_BN_ASSERT(b);
	#endif
	fprintf(f, "bn's [%p] dump.\n{\nbodysize: %d\nbody: ", b, b->bodysize);
	
	for (int i = 0; i < b->amount_of_allocated_blocks*DEFAULT_SIZE; i++)
		fprintf(f, "%d", b->body[b->amount_of_allocated_blocks*DEFAULT_SIZE - 1 - i]);
		
	fprintf(f,"\namount of allocated blocks: %d\nsign: %d\n}\n", b->amount_of_allocated_blocks, b->sign);
}

// Операции, аналогичные +=, -=, *=, /=, %=
int bn_add_to(bn *t, bn const *right)
{
	#ifdef DEBUG
	_BN_ASSERT(t);
	_BN_ASSERT(right);
	if (bn_check(t) != BN_OK) assert(0);
	#endif
	if (_NO_BN(t) || _NO_BN(right))
		return BN_NULL_OBJECT;
	//предварительное расширение буфера. Если размер буфера right больше, чем t
		//Расширить до расмера right + 1
	//Расширить до размера t + 1
	if (right->amount_of_allocated_blocks > t->amount_of_allocated_blocks)
	{
		int amount_of_blocks = right->amount_of_allocated_blocks + 1;
		body_t* tmp = (body_t* )calloc(amount_of_blocks*DEFAULT_SIZE, sizeof(body_t));
		if (!tmp)
			return BN_NO_MEMORY;
		for (int i = 0; i < t->bodysize; i++) tmp[i] = t->body[i];
		
		free(t->body);
		t->body = tmp;
		t->amount_of_allocated_blocks = amount_of_blocks;
	}
	else
	{
		int amount_of_blocks = t->amount_of_allocated_blocks + 1;
		
		body_t* tmp = (body_t* )calloc(amount_of_blocks*DEFAULT_SIZE, sizeof(body_t));
		if (!tmp)
			return BN_NO_MEMORY;
		for (int i = 0; i < t->bodysize; i++) tmp[i] = t->body[i];
		
		free(t->body);
		t->body = tmp;
		t->amount_of_allocated_blocks = amount_of_blocks;
	}
	
	
	
		//если знаки разные:
			//результат – разность модулей, причем отнимать от большего меншее.
				//отнимать поразрядно
				//стабилизировать
			//знак - знак того числа, чей модуль был больше
		//если знаки одинаковые
			//результат - сумма модулей
				//прибавлять поразрядно
				//стабилизировать
			//знак остается тем же
	
	if (t->sign * right->sign == 1)
	{
		//простая поразрядная сумма
		for (int i = 0; i < right->bodysize; i++) t->body[i] += right->body[i];
	}
	else if (t->sign * right->sign == -1)
	{
		//если модуль right больше модуля t
			//сохранить в tmp t-body
			//скопировать right->body в t->body
			//отнимать поразрядно от t tmp
			//free(tmp);
		//если модуль right меньше модуля t
			//отнимать поразряжно от t right
		//иначе
			//ноль
		
		bn* abs_t = bn_init(t);
		if (!abs_t)
			return BN_NO_MEMORY;
		bn* abs_right = bn_init(right);
		if (!abs_right)
			return BN_NO_MEMORY;
		
		bn_abs(abs_t);
		bn_abs(abs_right);
		switch (bn_cmp(abs_t, abs_right))
		{
			case 1:
			{
				for (int i = 0; i < right->bodysize; i++)
					t->body[i] -= right->body[i];
				break;
			}
			case -1:
			{
				t->sign = right->sign;
				body_t* tmp = (body_t* )calloc(t->amount_of_allocated_blocks*DEFAULT_SIZE, sizeof(body_t));
				if (!tmp)
					{
						bn_delete(abs_t);
						bn_delete(abs_right);
						return BN_NO_MEMORY;
					}
				for (int i = 0; i < t->bodysize; i++)
					tmp[i] = t->body[i];
				for (int i = 0; i < right->bodysize; i++)
					t->body[i] = right->body[i];
				for (int i = 0; i < t->bodysize; i++)
					t->body[i] -= tmp[i];
				free(tmp);
				break;
			}
			case 0:
			{
				for (int i = 0; i < t->amount_of_allocated_blocks*DEFAULT_SIZE; i++)
					t->body[i] = 0;
				t->sign = 0;
				t->bodysize = 1;
				//~ free(t->body);
				//~ t->amount_of_allocated_blocks = 1;
				//~ t->bodysize = 1;
				//~ t->sign = 0;
				//~ t->body = (body_t* )calloc(DEFAULT_SIZE, sizeof(body_t));
				
				bn_delete(abs_t);
				bn_delete(abs_right);
				return BN_OK;
			}
			default: assert(0);
		}
		bn_delete(abs_t);
		bn_delete(abs_right);
	}
	else if (t->sign == 0)
	{
		t->sign = right->sign;
		for (int i = 0; i < right->bodysize; i++)
			t->body[i] = right->body[i];
	}
	_BN_STABILIZE(t);
	
	//если можно обойтись меньшей памятью
		//выделить нужную память
		//записать туда t->body
		//осводобить t->body
		//обновить t->body
		//изменить amount_of...
	
	
	//~ if ((int)(trunc(t->bodysize / DEFAULT_SIZE) + 1) < t->amount_of_allocated_blocks)
	//~ {
		//~ t->amount_of_allocated_blocks = (int)(trunc(t->bodysize / DEFAULT_SIZE) + 1);
		//~ body_t* tmp = (body_t* )calloc(t->amount_of_allocated_blocks*DEFAULT_SIZE, sizeof(body_t));
		//~ if (!tmp)
			//~ return BN_NO_MEMORY;
		//~ for (int i = 0; i < t->bodysize; i++)
			//~ tmp[i] = t->body[i];
		//~ free(t->body);
		//~ t->body = tmp;
	//~ }
	return bn_check(t);
}

int bn_sub_to(bn *t, bn const *right)
{
	#ifdef DEBUG
	_BN_ASSERT(t);
	_BN_ASSERT(right);
	#endif
	if (_NO_BN(t) || _NO_BN(right))
		return BN_NULL_OBJECT;
	//аналгично bn_add_to, но right имеет противоположный знак
	bn_neg(t);
	bn_add_to(t, right);
	bn_neg(t);
	
	return BN_OK;
}

int bn_mul_to(bn *t, bn const *right)
{
	#ifdef DEBUG
	if (bn_check(t) != BN_OK)
	{
		bn_print_error(bn_check(t));
		assert(0);
	}
	#endif
	#ifdef DEBUG
	_BN_ASSERT(t);
	_BN_ASSERT(right);
	#endif
	if (_NO_BN(t) || _NO_BN(right))
		return BN_NULL_OBJECT;
	//предварительное расширение буфера
		//размер буфера t = buf_t + buf_right + 1
		//собственно расширение буфера
	
	//умножение столбиком (по старой схеме)
	
	int sign_result = t->sign * right->sign;
	if (t->body[0] == 1 && t->bodysize == 1)
	{
		free(t->body);
		t->body = (body_t* )calloc(right->amount_of_allocated_blocks*DEFAULT_SIZE, sizeof(body_t));
		for (int i = 0; i < right->bodysize; i++) t->body[i] = right->body[i];
		t->bodysize = right->bodysize;
		t->amount_of_allocated_blocks = right->amount_of_allocated_blocks;
		t->sign = sign_result;
	}
	else if (right->body[0] == 1 && right->bodysize == 1)
	{
		t->sign = sign_result;
	}
	else
	{
		int amount_of_blocks = t->amount_of_allocated_blocks + right->amount_of_allocated_blocks + 1;
		body_t* mul_result = (body_t* )calloc(amount_of_blocks*DEFAULT_SIZE, sizeof(body_t));
		if (!mul_result)
			return BN_NO_MEMORY;

		for (int i = 0; i < right->bodysize; i++)
		{
			for (int j = 0; j < t->bodysize; j++)
			{
				mul_result[i+j] += t->body[j]*right->body[i];
				//~ printf("i = %d\n", i);
				//~ printf("j = %d\n\n", j);
			}
		}
		t->sign = sign_result;
		free(t->body);
		t->body = mul_result;
		t->amount_of_allocated_blocks = amount_of_blocks;
	}
	
	_BN_STABILIZE(t);
	
	//~ #ifdef DEBUG
	//~ printf("t->bodysize:	%d\nallocated:	%d\n\n", t->bodysize, t->amount_of_allocated_blocks*DEFAULT_SIZE);
	//~ assert(0);
	//~ #endif
	
	//~ if ((int)(trunc(t->bodysize / DEFAULT_SIZE) + 1) < t->amount_of_allocated_blocks)
	//~ {
		//~ t->amount_of_allocated_blocks = (int)(trunc(t->bodysize / DEFAULT_SIZE) + 1);
		//~ body_t* tmp = (body_t* )calloc(t->amount_of_allocated_blocks*DEFAULT_SIZE, sizeof(body_t));
		//~ if (!tmp)
			//~ return BN_NO_MEMORY;
		//~ for (int i = 0; i < t->bodysize; i++)
			//~ tmp[i] = t->body[i];
		//~ free(t->body);
		//~ t->body = tmp;
	//~ }
	
	return bn_check(t);
}

int bn_div_to(bn *t, bn const *right) //ЕСТЬ ПОДОЗРЕНИЯ, ЧТО ДЕЛЕНИЕ ПРИ ОТРИЦАТЕЛЬНЫЗ ЧИСТАХ НЕ ВСЕГДА РАБАОТЕТ
{
	#ifdef DEBUG
	_BN_ASSERT(t);
	_BN_ASSERT(right);
	#endif
	if (_NO_BN(t) || _NO_BN(right))
		return BN_NULL_OBJECT;
	//Деление столбиком
	
	if (right->sign == 0)
	{
		return BN_DIVIDE_BY_ZERO;
	}
	
	int sign_result = t->sign * right->sign;
	if (right->bodysize > t->bodysize)
	{
		free(t->body);
		t->amount_of_allocated_blocks = 1;
		t->sign = 0;
		t->bodysize = 1;
		t->body = (body_t* )calloc(DEFAULT_SIZE, sizeof(body_t));
		if (!t)
			return BN_NO_MEMORY;
	}
	else
	{
		bn* abs_t = bn_init(t);
		bn* abs_right = bn_init(right);
		if (!abs_t || !abs_right)
			return BN_NO_MEMORY;
			
		bn_abs(abs_t);
		bn_abs(abs_right);
		bn* ten = bn_new();
		bn* one = bn_new();
		if (!ten || !one)
			return BN_NO_MEMORY;
		bn_init_int(ten, 10);
		bn_init_int(one, 1);
		switch (bn_cmp(abs_t, abs_right))
		{
			case -1:
			{
				free(t->body);
				t->body = (body_t* )calloc(DEFAULT_SIZE, sizeof(body_t));
				if (!t->body)
					return BN_NO_MEMORY;
				t->bodysize = 1;
				t->sign = 0;
				t->amount_of_allocated_blocks = 1;
				
				break;
			}
			case 0:
			{
				free(t->body);
				t->body = (body_t* )calloc(DEFAULT_SIZE, sizeof(body_t));
				if (!t->body)
					return BN_NO_MEMORY;
				t->body[0] = 1;
				t->bodysize = 1;
				t->amount_of_allocated_blocks = 1;
				t->sign = sign_result;
				break;
			}
			case 1:
			{
				bn* x = bn_new(); //отсеченная часть
				if (!x)
					return BN_NO_MEMORY;
				free(x->body);
				x->body = (body_t* )calloc(abs_right->amount_of_allocated_blocks*DEFAULT_SIZE, sizeof(body_t));
				x->sign = 1;
				
				for (int i = 0; i < right->bodysize; i++)
					x->body[right->bodysize - 1 - i] = t->body[t->bodysize - 1 - i];
				
				x->bodysize = right->bodysize;
				if (bn_cmp(x, abs_right) == -1)
				{
					bn_mul_to(x, ten);
					x->body[0] = t->body[t->bodysize - 1 - right->bodysize];
				}
				
				
				
				dynamic_stack* st = dynamic_stack_Construct();
				for (int i = 0; i < t->bodysize - x->bodysize; i++)
					dynamic_stack_Push(st, t->body[i]);
				
				dynamic_stack* s = dynamic_stack_Construct();
				
				while (1)
				{
					bn* premul = bn_new();
					if (!premul)
						return BN_NO_MEMORY;
					int digit = 0;
					while (bn_cmp(premul, x) == -1)
					{
						bn_add_to(premul, abs_right);
						digit++;
					}
					if (bn_cmp(premul, x) == 1)
					{
						bn_sub_to(premul, abs_right);
						digit--;
					}
					dynamic_stack_Push(s, digit);
					
					//~ printf("x:\n");
					//~ _GREEN_DUMP(x);
					//~ printf("premul:\n");
					//~ _GREEN_DUMP(premul);
					bn_sub_to(x, premul);
					//~ _RED_DUMP(x);
					
					#ifdef DEBUG
					if (bn_cmp(x, abs_right) == 1)
					{
						printf("РЕЗУЛЬТАТ ВЫЧИТАНИЯ БОЛЬШЕ ДЕЛИТЕЛЯ!\n");
						_RED_DUMP(x);
						_GREEN_DUMP(abs_right);
						printf("premul:\n");
						_RED_DUMP(premul);
						assert(0);
					}
					#endif
					
					
					
					bn_delete(premul);
					if (st->current)
					{
						//~ bn_mul_to(x, ten);
						
						
						bn_check(x);
						if (x->sign != 0)
						{
							for (int i = 0; i < x->bodysize; i++)
								x->body[x->bodysize - i] = x->body[x->bodysize - 1 - i];
							x->body[0] = dynamic_stack_Pop(st);
							x->bodysize++;
						}
						else
						{
							x->body[0] = dynamic_stack_Pop(st);
							x->sign = 1;
						}
						bn_check(x);
						
						//~ bn* tmp = bn_new();
						//~ bn_init_int(tmp, dynamic_stack_Pop(st));
						//~ bn_add_to(x, tmp);
						//~ bn_delete(tmp);
					}
					else
					{
						break;
						//конец деления
					}
				}
				
				free(t->body);
				t->amount_of_allocated_blocks = (int)trunc(s->current / DEFAULT_SIZE) + 1;
				t->body = (body_t* )calloc(t->amount_of_allocated_blocks*DEFAULT_SIZE, sizeof(body_t));
				if (!t->body)
					return BN_NO_MEMORY;
				t->bodysize = s->current;
				t->sign = sign_result;
				int i = 0;
				while (s->current > 0)
				{
					t->body[i] = dynamic_stack_Pop(s);
					i++;
				}
				
				dynamic_stack_Destroy(st);
				dynamic_stack_Destroy(s);
				bn_delete(x);
				break;
			}
			default: assert(0);
		}
		bn_delete(abs_t);
		bn_delete(abs_right);
		bn_delete(ten);
		bn_delete(one);
	}
	
	_BN_STABILIZE(t);
	//~ if ((int)(trunc(t->bodysize / DEFAULT_SIZE) + 1) < t->amount_of_allocated_blocks)
	//~ {
		//~ t->amount_of_allocated_blocks = (int)(trunc(t->bodysize / DEFAULT_SIZE) + 1);
		//~ body_t* tmp = (body_t* )calloc(t->amount_of_allocated_blocks*DEFAULT_SIZE, sizeof(body_t));
		//~ if (!tmp)
			//~ return BN_NO_MEMORY;
		//~ for (int i = 0; i < t->bodysize; i++)
			//~ tmp[i] = t->body[i];
		//~ free(t->body);
		//~ t->body = tmp;
	//~ }
	
	return bn_check(t);
};

int bn_mod_to(bn *t, bn const *right)
{
	#ifdef DEBUG
	_BN_ASSERT(t);
	_BN_ASSERT(right);
	#endif
	if (_NO_BN(t) || _NO_BN(right))
		return BN_NULL_OBJECT;
	//испльзуя предыдущую функцию, вычислить остаток от деления
	bn* tmp = bn_init(t);
	if (!tmp)
		return BN_NO_MEMORY;
	bn_div_to(tmp, right);
	bn_mul_to(tmp, right);
	bn_sub_to(t, tmp);
	
	bn_delete(tmp);
	return bn_check(t);
}

// Возвести число в степень degree
int bn_pow_to(bn *t, int degree)
{
	#ifdef DEBUG
	_BN_ASSERT(t);
	#endif
	//~ if (_NO_BN(t))
		//~ return BN_NULL_OBJECT;
	
	//~ bn* t_copy = bn_init(t);
	//~ for (int i = 2; i <= degree; i++)
	//~ {
		//~ bn_mul_to(t, t_copy);
	//~ }
	//~ return BN_OK;
	
	//Бинарное возведение в степень
	bn* result = bn_new();
	bn_init_int(result, 1);
	int n = degree;
	
	while (n)
	{
		if (n % 2 == 1)
		{
			bn_mul_to(result, t);
			n--;
		}
		else
		{
			bn* t_copy = bn_init(t);
			bn_mul_to(t, t_copy);
			bn_delete(t_copy);
			n /= 2;
		}
	}
	
	free(t->body);
	_BN_STABILIZE(result);
	t->body = (body_t* )calloc(result->amount_of_allocated_blocks*DEFAULT_SIZE, sizeof(body_t));
	t->bodysize = result->bodysize;
	t->amount_of_allocated_blocks = result->amount_of_allocated_blocks;
	for (int i = 0; i < result->bodysize; i++) t->body[i]= result->body[i];
	t->sign = result->sign;
	bn_delete(result);
	
	return bn_check(t);
}



// Инициализировать значение BN представлением строки
// в системе счисления radix
int bn_init_string_radix(bn *t, const char *init_string, int radix)
{
	#ifdef DEBUG
	_BN_ASSERT(t);
	#endif
	if (_NO_BN(t))
		return BN_NULL_OBJECT;
	
	free(t->body);
	t->body = (body_t* )calloc(DEFAULT_SIZE, sizeof(body_t));
	t->amount_of_allocated_blocks = 1;
	t->sign = 0;
	t->bodysize = 1;
	//читать посимвольно
	//опредеить значение каждого символа
	//прибавить полученное значение инициализируемому числу
	// и так со всеми симвлоами
	
	const char* beginning = init_string;
	char sign_result = 0;
	if (init_string[0] == '-' && init_string[1] != '0')
	{
		sign_result = -1;
		beginning = &init_string[1];
	}
	else if (init_string[0] != '0')
	{
		sign_result = 1;
	}
	
	
	int i = 0;
	for (i = 0; beginning[i] != '\0'; i++);
	int n = i;
	
	for (i = 0; i < n; i++)
	{
		bn* rn = bn_new();
		bn_init_int(rn, radix);
		bn_pow_to(rn, i);
		bn* p = bn_new();
		#define x beginning[n - 1 - i]
		if (x >= '0' && x <= '9')
		{
			bn_init_int(p, x - '0');
		}
		else if (x >= 'A' && x<='Z')
		{
			bn_init_int(p, x - 'A' + 10);
		}
		else if (x >= 'a' && x <= 'z')
		{
			bn_init_int(p, x - 'a' + 10);
		}
		else
			assert(0);
		
		
		bn_mul_to(rn, p);
		bn_add_to(t, rn);
		#undef x
		bn_delete(p);
		bn_delete(rn);
	}
	
	t->sign = sign_result;
	
	//присвоить t
	return BN_OK;
}

// Аналоги операций x = l+r (l-r, l*r, l/r, l%r)
bn* bn_add(bn const *left, bn const *right)
{
	#ifdef DEBUG
	_BN_ASSERT(left);
	_BN_ASSERT(right);
	#endif
	if (_NO_BN(left) || _NO_BN(right))
		return NULL;
	
	bn* result = bn_new();
	if (!result)
		return NULL;
	
	bn_add_to(result, left);
	bn_add_to(result, right);
	return result;
}

bn* bn_sub(bn const *left, bn const *right)
{
	#ifdef DEBUG
	_BN_ASSERT(left);
	_BN_ASSERT(right);
	#endif
	if (_NO_BN(left) || _NO_BN(right))
		return NULL;
	bn* result = bn_new();
	if (!result)
		return NULL;
	bn_add_to(result, left);
	bn_sub_to(result, right);
	return result;
}

bn* bn_mul(bn const *left, bn const *right)
{
	#ifdef DEBUG
	_BN_ASSERT(left);
	_BN_ASSERT(right);
	#endif
	if (_NO_BN(left) || _NO_BN(right))
		return NULL;
	bn* result = bn_init(left);
	if (!result)
		return NULL;
	bn_mul_to(result, right);
	return result;
}

bn* bn_div(bn const *left, bn const *right)
{
	#ifdef DEBUG
	_BN_ASSERT(left);
	_BN_ASSERT(right);
	#endif
	if (_NO_BN(left) || _NO_BN(right))
		return NULL;
	bn* result = bn_init(left);
	if (!result)
		return NULL;
	bn_div_to(result, right);
	return result;
}

bn* bn_mod(bn const *left, bn const *right)
{
	#ifdef DEBUG
	_BN_ASSERT(left);
	_BN_ASSERT(right);
	#endif
	if (_NO_BN(left) || _NO_BN(right))
		return NULL;
	
	bn* result = bn_init(left);
	if (!result)
		return NULL;
	bn_mod_to(result, right);
	return result;
}

// Выдать представление BN в системе счисления radix в виде строки
// Строку после использования потребуется удалить.
const char *bn_to_string(bn const *t, int radix)
{
	#ifdef DEBUG
	_BN_ASSERT(t);
	#endif
	if (_NO_BN(t))
		return NULL;
	
	//пока текущее число не ноль
		//взять остаток от деления t на radix
		//класть в стек
		//делить текущее число нацело на radix
	
	bn* bn_radix = bn_new();
	bn_init_int(bn_radix, radix);
	
	bn* digit = NULL;
	
	bn* p = bn_init(t);

	dynamic_stack* s = dynamic_stack_Construct();
	
	int i = 0;
	while (p->sign != 0)
	{
		digit = bn_mod(p, bn_radix);
		int d = 0;
		int ten = 1;
		for (int i = 0; i < digit->bodysize; i++)
		{
			d += digit->body[i]*ten;
			ten *= 10;
		}
		dynamic_stack_Push(s, d);
		
		bn_div_to(p, bn_radix);
		bn_delete(digit);
		i++;
		//~ printf("i = %d\n", i);
	}

	char* outs = (char* )malloc((s->current + 1)*sizeof(char));
	int n = s->current;
	outs[n] = '\0';
	for (int i = 0; s->current; i++)
		outs[i] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[(int)dynamic_stack_Pop(s)];
	
	dynamic_stack_Destroy(s);
	bn_delete(bn_radix);
	bn_delete(p);
	
	
	return outs;
}



// Извлечь корень степени reciprocal из BN (бонусная функция)
int bn_root_to(bn *t, int reciprocal)
{
	#ifdef DEBUG
	_BN_ASSERT(t);
	#endif
	if (_NO_BN(t))
		return BN_NULL_OBJECT;
	
	
	//base = 10
	//i = 1
	//while (base^rec < t)
		//base *= 10
	
	if (t->sign == -1 && reciprocal % 2 == 0)
		return BN_DIVIDE_BY_ZERO;
	
	bn* ten = bn_new();
	bn_init_int(ten, 10);
	
	bn* base = bn_new();
	bn_init_int(base, 10);
	
	int i = 0;
	bn* base_powered = bn_init(base);
	bn_pow_to(base_powered, reciprocal);
	while (bn_cmp(base_powered, t) != 1)
	{
		i++;
		bn_mul_to(base, ten);
		bn_delete(base_powered);
		base_powered = bn_init(base);
		bn_pow_to(base_powered, reciprocal);
	}
	bn_delete(base_powered);
	int n = i + 1;
	
	dynamic_stack* s = dynamic_stack_Construct();
	
	bn* bn_olds = bn_new();
	for (int i = 0; i < n; i++)
	{
		bn* ten_powered = bn_init(ten);
		bn_pow_to(ten_powered, n - 1 - i);
		
		bn* bn_N_reced = bn_init(ten_powered);
		bn_add_to(bn_N_reced, bn_olds);
		bn_pow_to(bn_N_reced, reciprocal);
		
		bn* bn_N = bn_init(ten_powered);
		bn* bn_N_pred = NULL;
		
		int j = 1;
		while (j < 10 && bn_cmp(bn_N_reced, t) != 1)
		{
			j++;
			bn_delete(bn_N_reced);
			bn_N_reced = bn_init(ten_powered);
			bn* bn_j = bn_new();
			bn_init_int(bn_j, j);
			bn_mul_to(bn_N_reced, bn_j);
			bn_delete(bn_N_pred);
			bn_N_pred = bn_init(bn_N);
			bn_delete(bn_N);
			bn_N = bn_init(bn_N_reced);
			bn_delete(bn_j);
			bn_add_to(bn_N_reced, bn_olds);
			bn_pow_to(bn_N_reced, reciprocal);
		}
		dynamic_stack_Push(s, j - 1);
		bn_delete(ten_powered);
		
		if (bn_N_pred)
			bn_add_to(bn_olds, bn_N_pred);
		bn_delete(bn_N);
		bn_delete(bn_N_reced);
	}
	bn_delete(bn_olds);
	
	
	free(t->body);
	t->bodysize = n;
	t->amount_of_allocated_blocks = trunc(t->bodysize / DEFAULT_SIZE) + 1;
	t->body = (body_t* )calloc(t->amount_of_allocated_blocks*DEFAULT_SIZE, sizeof(body_t));
	int k = 0;
	while (s->current) t->body[k++] = dynamic_stack_Pop(s);
	
	bn_delete(ten);
	bn_delete(base);
	dynamic_stack_Destroy(s);
	return BN_OK;
}


