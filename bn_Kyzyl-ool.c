#define DEFAULT_SIZE 8
#define DEBUG

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

/*		EXPAND_BODY(the_bn, amount_of_blocks)
		
		int amount_of_blocks = 2;
		bn* the_bn = t;
		
		body_t* tmp = (body_t* )calloc(amount_of_blocks*DEFAULT_SIZE, sizeof(body_t));
		for (int i = 0; i < the_bn->bodysize; i++) tmp[i] = the_bn->body[i]; 
		if (the_bn->body) free(the_bn->body);
		the_bn->body = tmp;
		the_bn->amount_of_allocated_blocks = amount_of_blocks;
*/

#define _RED_DUMP(the_bn) printf("\033[1;31m"); bn_dump(the_bn, stdout); printf("\033[0m")
#define _GREEN_DUMP(the_bn) printf("\033[1;32m"); bn_dump(the_bn, stdout); printf("\033[0m")
#define _BEEP printf("\a")
#define _BN_ASSERT(the_bn) assert(the_bn); assert(the_bn->body)
#define _NO_BN(the_bn) !the_bn || !the_bn->body

//~ #ifdef BN
#include "bn.h"
//~ #endif

#define POISON 999999999999999.9999999999999
#define KANAR 771

typedef double stack_elem;

enum stack_error_codes
{
	STACK_OK = 0,
	STACK_TAIL_IS_NOT_POISON,
	STACK_ELEMENT_IS_POISON, 
	STACK_HASHSUM_ERROR,
	ERROR_POP_FROM_EMPTY_STACK, 
	ERROR_PUSH_TO_FULL_STACK,
	STACK_CURRENT_MORE_THAN_LENGTH,
	STACK_GET_FROM_INVALID_INDEX,
};
typedef enum stack_error_codes stack_error_code;

typedef struct stack
{
	int kanar1;
	int current;
	int amount_of_elements;
	stack_elem* elements;
	int hashsum;
	int kanar2;
	
}stack;

stack* 		stack_Construct(int amount_of_elements);
int 		stack_Destroy(stack* s);
int 		stack_Push(stack* s, stack_elem value);
stack_elem	stack_Pop(stack* s);
int			stack_calc_hashsum(stack* s);
stack_error_code	stack_check(stack* s);
char*		stack_error_message(stack_error_code error_code);
void		stack_help(stack_error_code error_code);
void		stack_print_dump(stack* s);
int  		terminate_message(int error);
stack_elem	stack_Get(stack* s, int n);
void 		stack_dump(stack* s, const char* dump_name);

stack* stack_Construct(int amount_of_elements)
{
	#ifdef DEBUG
	assert(amount_of_elements);
	#endif
	stack* s = (stack* )calloc(1, sizeof(stack));
	s->elements = (stack_elem* )calloc(amount_of_elements, sizeof(stack_elem));
	s->kanar1 = KANAR;
	s->kanar2 = KANAR;
	s->current = 0;
	s->amount_of_elements = amount_of_elements;
	s->hashsum = s->kanar1 + s->kanar2 + amount_of_elements;
	int i = 0;
	for (i = 0; i < amount_of_elements; i++) s->elements[i] = POISON;
	
	#ifdef DEBUG
	assert(!stack_check(s));
	#endif
	return s;
}

int stack_Destroy(stack* s)
{
	#ifdef DEBUG
	assert(s);
	#endif
	int error_code = stack_check(s);
	for(int i = 0; i < s->current; i++) s->elements[i] = POISON;
	free(s->elements);
	free(s);
	
	return error_code;
	return 0;
}

int stack_Push(stack* s, stack_elem value)
{
	#ifdef DEBUG
	assert(s);
	isfinite(value);
	#endif
	
	if (s->current < s->amount_of_elements)
	{
		s->elements[s->current] = value;
		s->current++;
		s->hashsum = stack_calc_hashsum(s);
		return stack_check(s);
		return 0;
	}
	else
	{
		#ifdef DEBUG
		stack_help(ERROR_PUSH_TO_FULL_STACK);
		#endif
		return ERROR_PUSH_TO_FULL_STACK;
	}
}

stack_elem stack_Pop(stack* s)
{
	#ifdef DEBUG
	assert(s);
	#endif
	if (s->current > 0)
	{
		stack_elem value = s->elements[s->current-1];
		s->current--;
		s->elements[s->current] = POISON;
		
		s->hashsum = stack_calc_hashsum(s);
		return value;
	}
	else
	{
		#ifdef DEBUG
		stack_help(ERROR_POP_FROM_EMPTY_STACK);
		#endif
		return POISON;
	}
}

int	stack_calc_hashsum(stack* s)
{
	#ifdef DEBUG
	assert(s);
	#endif
	int hashsum = s->kanar1 + s->kanar2 + s->current + s->amount_of_elements;
	int i = 0;
	for (i = 0; i < s->current; i++) hashsum += s->elements[i];
	
	return hashsum;
}

stack_error_code	stack_check(stack* s)
{
	if (s->elements[s->current] != POISON) return STACK_TAIL_IS_NOT_POISON;
	if (s->current >= s->amount_of_elements) return STACK_CURRENT_MORE_THAN_LENGTH;
	for (int i = 0; i < s->current - 1; i++) if (s->elements[i] == POISON) return STACK_ELEMENT_IS_POISON;
	if (s->hashsum != stack_calc_hashsum(s)) return STACK_HASHSUM_ERROR;
	
	return STACK_OK;
}

char* stack_error_message(stack_error_code error_code)
{
	
	#define RET_CODE_(code)  case code: return #code;
	
	switch (error_code)
	{
		RET_CODE_ (STACK_OK)
		RET_CODE_ (STACK_TAIL_IS_NOT_POISON)
		RET_CODE_ (STACK_ELEMENT_IS_POISON)
		RET_CODE_ (STACK_HASHSUM_ERROR)
		RET_CODE_ (ERROR_POP_FROM_EMPTY_STACK)
		RET_CODE_ (ERROR_PUSH_TO_FULL_STACK)
		RET_CODE_ (STACK_CURRENT_MORE_THAN_LENGTH)
		RET_CODE_ (STACK_GET_FROM_INVALID_INDEX)
		default: return "STACK_UNKNOWN_ERROR";
	}
	
	#undef RET_CODE_
	
}
void stack_help(stack_error_code error_code)
{
	printf("%s\n", stack_error_message(error_code));
}

void stack_print_dump(stack* s)
{
	FILE* dump = fopen("stack_dump.txt", "w");
	
	fprintf(dump,
	"stack \"s\" (%s) [%p] {\n"
	"	current = %d\n\n", stack_error_message(stack_check(s)), s, s->current);
	
	for (int i = 0; i < s->current; i++) fprintf(dump, "	elements[%d]: %lg\n", i, s->elements[i]);
	fprintf(dump, "}\n");
	
	fclose(dump);
}

void stack_dump(stack* s, const char* dump_name)
{
	FILE* dump = fopen(dump_name, "w");
	
	fprintf(dump,
	"stack \"s\" (%s) [%p] {\n"
	"	current = %d\n\n", stack_error_message(stack_check(s)), s, s->current);
	
	for (int i = 0; i < s->current; i++) fprintf(dump, "	elements[%d]: %lg\n", i, s->elements[i]);
	fprintf(dump, "}\n");
	
	fclose(dump);
}

int terminate_message(int error)
{
	printf("\aProgram terminated with error message: %d\n" "Meow.\n", error);
	return error;
}

stack_elem	stack_Get(stack* s, int n)
{
	#ifdef DEBUG
	assert(s);
	#endif
	if (n > 0 && n <= s->current)
	{
		return s->elements[s->current - n];
	}
	else
	{
		#ifdef DEBUG
		stack_help(STACK_GET_FROM_INVALID_INDEX);
		#endif
		printf("element: %d\n", s->current - n);
		return POISON;
	}
}



typedef int body_t;
typedef struct bn_s
{
	body_t* body;
	int bodysize;
	int amount_of_allocated_blocks;
	char sign;
} bn;


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
	new_bn->bodysize = 0;
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

// Инициализировать значение BN представлением строки
// в системе счисления radix
int bn_init_string_radix(bn *t, const char *init_string, int radix)
{
	#ifdef DEBUG
	_BN_ASSERT(t);
	assert(init_string);
	isfinite(radix);
	#endif
	if (_NO_BN(t))
		return BN_NULL_OBJECT;
	
	//читать посимвольно
	//опредеить значение каждого символа
	//прибавить полученное значение инициализируемому числу
	// и так со всеми симвлоами
	
	
	
	//присвоить t
	return BN_OK;
}

// Инициализировать значение BN заданным целым числом
int bn_init_int(bn *t, int init_int)
{
	#ifdef DEBUG
	_BN_ASSERT(t);
	isfinite(init_int);
	#endif
	if (_NO_BN(t))
		return BN_NULL_OBJECT;
	//Брать остаток от деления на 10 и прибавлять к соответствующему разряду
	
	//присвоить t
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
				//сравнение поразрядно
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
	else
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
	
	assert(0);
}

int bn_neg(bn *t) // Изменить знак на противоположный
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
		t->sign = -t->sign;
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
	fprintf(f, "bn's dump.\n{\nbodysize: %d\nbody: ", b->bodysize);
	
	for (int i = 0; i < b->amount_of_allocated_blocks*DEFAULT_SIZE; i++)
		fprintf(f, "%d", b->body[b->amount_of_allocated_blocks*DEFAULT_SIZE - 1 - i]);
		
	fprintf(f,"\namount of allocated blocks: %d\nsign: %d\n}\n", b->amount_of_allocated_blocks, b->sign);
}

void bn_stabilize(bn* t)
{
	#ifdef DEBUG
	_BN_ASSERT(t);
	#endif
	
	for (int i = 0; i < t->amount_of_allocated_blocks*DEFAULT_SIZE; i++)
	{
		if (t->body[i] > 9)
		{
			t->body[i+1] += t->body[i] / 10;
			t->body[i] %= 10;
		}
		while (t->body[i] < 0)
		{
			t->body[i] += 10;
			t->body[i+1]--;
		}
	}
	t->bodysize = 0;
	for (int i = 0; i < t->amount_of_allocated_blocks*DEFAULT_SIZE; i++)
	{
		if (t->body[i] != 0)
			t->bodysize = i + 1;
	}
	if (t->bodysize == 0) t->sign = 0;
}


// Операции, аналогичные +=, -=, *=, /=, %=
int bn_add_to(bn *t, bn const *right)
{
	#ifdef DEBUG
	_BN_ASSERT(t);
	_BN_ASSERT(right);
	#endif
	if (_NO_BN(t) || _NO_BN(right))
		return BN_NULL_OBJECT;
	//предварительное расширение буфера. Если размер буфера right больше, чем t
		//Расширить до расмера right + 1
	//Расширить до размера t + 1
	if (right->amount_of_allocated_blocks > t->amount_of_allocated_blocks)
	{
		int amount_of_blocks = right->amount_of_allocated_blocks + 1;
		bn* the_bn = t;
		
		body_t* tmp = (body_t* )calloc(amount_of_blocks*DEFAULT_SIZE, sizeof(body_t));
		if (!tmp)
			return BN_NO_MEMORY;
		for (int i = 0; i < the_bn->bodysize; i++) tmp[i] = the_bn->body[i]; 
		if (the_bn->body) free(the_bn->body);
		the_bn->body = tmp;
		the_bn->amount_of_allocated_blocks = amount_of_blocks;
	}
	else
	{
		int amount_of_blocks = t->amount_of_allocated_blocks + 1;
		bn* the_bn = t;
		
		body_t* tmp = (body_t* )calloc(amount_of_blocks*DEFAULT_SIZE, sizeof(body_t));
		if (!tmp)
			return BN_NO_MEMORY;
		for (int i = 0; i < the_bn->bodysize; i++) tmp[i] = the_bn->body[i]; 
		if (the_bn->body) free(the_bn->body);
		the_bn->body = tmp;
		the_bn->amount_of_allocated_blocks = amount_of_blocks;
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
				t->sign = 0;
				for (int i = 0; i < t->bodysize; i++) t->body[i] = 0;
				break;
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
	bn_stabilize(t);
	
	//если можно обойтись меньшей памятью
		//выделить нужную память
		//записать туда t->body
		//осводобить t->body
		//обновить t->body
		//изменить amount_of...
	
	
	if ((int)(trunc(t->bodysize / DEFAULT_SIZE) + 1) < t->amount_of_allocated_blocks)
	{
		t->amount_of_allocated_blocks = (int)(trunc(t->bodysize / DEFAULT_SIZE) + 1);
		body_t* tmp = calloc(t->amount_of_allocated_blocks*DEFAULT_SIZE, sizeof(body_t));
		if (!tmp)
			return BN_NO_MEMORY;
		for (int i = 0; i < t->bodysize; i++)
			tmp[i] = t->body[i];
		free(t->body);
		t->body = tmp;
	}
	return BN_OK;
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
	bn* r = bn_init(right);
	if (!r)
		return BN_NO_MEMORY;
	r->sign *= -1;
	bn_add_to(t, r);
	bn_delete(r);
	return BN_OK;
}

int bn_mul_to(bn *t, bn const *right)
{
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
	
	int amount_of_blocks = t->amount_of_allocated_blocks + right->amount_of_allocated_blocks + 1;
	body_t* mul_result = (body_t* )calloc(amount_of_blocks*DEFAULT_SIZE, sizeof(body_t));
	if (!mul_result)
		return BN_NO_MEMORY;
	
	int sign_result = t->sign * right->sign;
	for (int i = 0; i < right->bodysize; i++)
	{
		for (int j = 0; j < t->bodysize; j++)
		{
			mul_result[i+j] += t->body[j]*right->body[i];
		}
	}
	t->sign = sign_result;
	free(t->body);
	t->body = mul_result;
	t->amount_of_allocated_blocks = amount_of_blocks;
	bn_stabilize(t);
	
	
	if ((int)(trunc(t->bodysize / DEFAULT_SIZE) + 1) < t->amount_of_allocated_blocks)
	{
		t->amount_of_allocated_blocks = (int)(trunc(t->bodysize / DEFAULT_SIZE) + 1);
		body_t* tmp = calloc(t->amount_of_allocated_blocks*DEFAULT_SIZE, sizeof(body_t));
		if (!tmp)
			return BN_NO_MEMORY;
		for (int i = 0; i < t->bodysize; i++)
			tmp[i] = t->body[i];
		free(t->body);
		t->body = tmp;
	}
	
	return BN_OK;
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
	
	//Сравнить длину делимого и делителя
		//если длина делителя больше
			//выходит ноль
		//иначе
			//отсечь от делимого часть, длина которой равна длине делителя
			//если по модулю отсеченная часть меньше модуля делителя
				//приписать следующую цифру
			
			//все оставшиеся цифры класть в стек st
			
			//перебор числа premul:
			//пока premul < x
				//premul += abs_right
				//i++
			//класть в стек i
			
			//если стек не пуст:
				//x *= 10
				//x += stack_Pop(st)
			//иначе
				//вытакивать результат из стека s
				//сформировать результат
				//оптимизация памяти
			
	if (right->sign == 0)
	{
		return BN_DIVIDE_BY_ZERO;
	}
	
	int sign_result = t->sign * right->sign;
	if (right->bodysize > t->bodysize)
	{
		bn_delete(t);
		t = bn_new();
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
				bn_delete(t);
				t = bn_new();
				if (!t)
					return BN_NO_MEMORY;
				break;
			}
			case 0:
			{
				bn_delete(t);
				t = bn_init(one);
				if (!t)
					return BN_NO_MEMORY;
				t->sign = sign_result;
				break;
			}
			case 1:
			{
				bn* x = bn_new(); //отсеченная часть
				if (!x)
					return BN_NO_MEMORY;
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
				
				//~ _RED_DUMP(x);
				
				stack* st = stack_Construct(t->bodysize - x->bodysize + 1);
				for (int i = 0; i < t->bodysize - x->bodysize; i++)
					stack_Push(st, t->body[i]);
				stack* s = stack_Construct(t->bodysize);
				
				
				
				while (st->current > 0 || bn_cmp(x, abs_right) != -1)
				{
					bn* premul = bn_init(abs_right);
					if (!premul)
						return BN_NO_MEMORY;
					int digit = 1;
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
					//~ printf("digit: %d\n", digit);
					stack_Push(s, digit);
					
					bn_sub_to(x, premul);
				
					//~ _RED_DUMP(x);
					int placed = 0;
					while (st->current > 0 && bn_cmp(x, abs_right) == -1)
					{
						bn* tmp = bn_new();
						if (!tmp)
							return BN_NO_MEMORY;
						bn_init_int(tmp, stack_Pop(st));
						
						if (tmp->sign == 0 && x->sign == 0)
						{
							stack_Push(s, 0);
						}
						else
						{
							bn_mul_to(x, ten);
							bn_add_to(x, tmp);
							placed++;
						}
						if (placed > 1)
							stack_Push(s, 0);
						bn_delete(tmp);
					}
					if (st->current == 0 && placed > 0 && bn_cmp(x, abs_right) == -1) stack_Push(s, 0);
					//~ _RED_DUMP(x);
					bn_delete(premul);
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
					t->body[i] = stack_Pop(s);
					i++;
				}
				//~ stack_print_dump(st);
				//~ stack_print_dump(s);
				
				stack_Destroy(st);
				stack_Destroy(s);
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
	
	return BN_OK;
}

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
	return BN_OK;
}

// Возвести число в степень degree
int bn_pow_to(bn *t, int degree)
{
	#ifdef DEBUG
	_BN_ASSERT(t);
	isfinite(degree);
	#endif
	if (_NO_BN(t))
		return BN_NULL_OBJECT;
	//Бинарное возведение в степень
	bn* result = bn_new();
	if (!result)
		return BN_NO_MEMORY;
	
	bn_init_int(result, 1);
	int pow = degree;
	while (pow > 0)
	{
		if (pow % 2 == 1)
			bn_mul_to(result, t);
		bn_mul_to(t, t);
		pow /= 2;
	}
	bn_delete(t);
	t = bn_init(result);
	if (!t)
		return BN_NO_MEMORY;
	return BN_OK;
}

// Извлечь корень степени reciprocal из BN (бонусная функция)
int bn_root_to(bn *t, int reciprocal)
{
	#ifdef DEBUG
	_BN_ASSERT(t);
	isfinite(reciprocal);
	#endif
	if (_NO_BN(t))
		return BN_NULL_OBJECT;
	
	
	return BN_OK;
}

// Аналоги операций x = l+r (l-r, l*r, l/r, l%r)
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
	assert(isfinite(radix));
	#endif
	if (_NO_BN(t))
		return NULL;
	
	//цикл, пока число больше radix
		//взять остаток от деления числа на radix
		//полученноу числу сопоставить символ и вывести
	return NULL;
}


