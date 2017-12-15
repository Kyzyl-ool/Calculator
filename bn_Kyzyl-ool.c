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


#ifdef BN
#include "bn.h"
#endif



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
	new_bn->body = (body_t* )calloc(DEFAULT_SIZE, sizeof(body_t));
	new_bn->sign = 0;
	new_bn->bodysize = 0;
	new_bn->amount_of_allocated_blocks = 1;
	
	return new_bn;
}

bn *bn_init(bn const *orig) // Создать копию существующего BN
{
	#ifdef DEBUG
	assert(orig);
	#endif
	
	bn* new_bn = (bn* )calloc(1, sizeof(bn));
	new_bn->amount_of_allocated_blocks = orig->amount_of_allocated_blocks;
	new_bn->bodysize = orig->bodysize;
	new_bn->sign = orig->sign;
	new_bn->body = (body_t* )calloc(new_bn->amount_of_allocated_blocks*DEFAULT_SIZE, sizeof(body_t));
	for (int i = 0; i < new_bn->amount_of_allocated_blocks*DEFAULT_SIZE; i++)
		new_bn->body[i] = orig->body[i];
	
	return new_bn;
	
	//вернуть
}

// Инициализировать значение BN десятичным представлением строки
int bn_init_string(bn *t, const char *init_string)
{
	#ifdef DEBUG
	assert(t);
	assert(init_string);
	#endif
	
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
	for (i = 0; i < t->bodysize; i++) t->body[t->bodysize - 1 - i] = number[i] - '0';
	
	return 0;
}

// Инициализировать значение BN представлением строки
// в системе счисления radix
int bn_init_string_radix(bn *t, const char *init_string, int radix)
{
	#ifdef DEBUG
	assert(t);
	assert(init_string);
	isfinite(radix);
	#endif
	
	//читать посимвольно
	//опредеить значение каждого символа
	//прибавить полученное значение инициализируемому числу
	// и так со всеми симвлоами
	
	
	
	//присвоить t
	return 0;
}

// Инициализировать значение BN заданным целым числом
int bn_init_int(bn *t, int init_int)
{
	#ifdef DEBUG
	assert(t);
	isfinite(init_int);
	#endif
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
	x = init_int*t->sign;
	for (int i = 0; i < t->bodysize; i++)
	{
		t->body[i] = x % 10;
		x /= 10;
	}
	
	return 0;
}

// Уничтожить BN (освободить память)
int bn_delete(bn *t)
{
	#ifdef DEBUG
	assert(t);
	#endif
	//освобдить body
	//освободить само число
	if (t->body)
		free(t->body);
	
	free(t);
	
	return 0;
}

// Если левое меньше, вернуть <0 если равны, вернуть 0 иначе  >0
int bn_cmp(bn const *left, bn const *right)
{
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
	return 0;
}

int bn_abs(bn *t) // Взять модуль
{
	if (t->sign < 0)
	{
		t->sign = -t->sign;
	}
	return 0;
}

int bn_sign(bn const *t) //-1 если t<0; 0 если t = 0, 1 если t>0
{
	return 0;
}


void bn_dump(bn* b, FILE* f)
{
	fprintf(f, "bn's dump.\n{\nbodysize: %d\nbody: ", b->bodysize);
	
	for (int i = 0; i < b->amount_of_allocated_blocks*DEFAULT_SIZE; i++)
		fprintf(f, "%d", b->body[b->amount_of_allocated_blocks*DEFAULT_SIZE - 1 - i]);
		
	fprintf(f,"\namount of allocated blocks: %d\nsign: %d\n}\n", b->amount_of_allocated_blocks, b->sign);
}

void bn_stabilize(bn* t)
{
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
	assert(t);
	assert(right);
	#endif
	//предварительное расширение буфера. Если размер буфера right больше, чем t
		//Расширить до расмера right + 1
	//Расширить до размера t + 1
	if (right->amount_of_allocated_blocks > t->amount_of_allocated_blocks)
	{
		int amount_of_blocks = right->amount_of_allocated_blocks + 1;
		bn* the_bn = t;
		
		body_t* tmp = (body_t* )calloc(amount_of_blocks*DEFAULT_SIZE, sizeof(body_t));
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
		bn* abs_right = bn_init(right);
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
	if (trunc(t->bodysize / DEFAULT_SIZE) + 1 < t->amount_of_allocated_blocks)
	{
		t->amount_of_allocated_blocks = trunc(t->bodysize / DEFAULT_SIZE) + 1;
		body_t* tmp = calloc(t->amount_of_allocated_blocks, sizeof(body_t));
		for (int i = 0; i < t->bodysize; i++)
			tmp[i] = t->body[i];
		free(t->body);
		t->body = tmp;
		
	}
	return 0;
}

int bn_sub_to(bn *t, bn const *right)
{
	//аналгично bn_add_to, но right имеет противоположный знак
	return 0;
}

int bn_mul_to(bn *t, bn const *right)
{
	//предварительное расширение буфера
		//размер буфера t = 2*max(buf_t, buf_right)
		//собственно расширение буфера
	
	//умножение столбиком (по старой схеме)
	return 0;
}

int bn_div_to(bn *t, bn const *right)
{
	//Деление столбиком
	
	//Сравнить длину делимого и делителя
		//если длина делителя больше
			//выходит ноль
		//иначе
			//отсечь от делимого часть, длина которой равна длине делителя
			
			//цикл, пока делимое больше делителя
				//начнать подбор i от 1 до 9, пока i*делитель не больше выбранной части
				//положить в стек i
				//дополнить остаток числами до тех пор, пока модуль выбранной части не больше делителя, при этом если дополняем большим, чем одной цифрой, кладем в стек 0
			
			//очистить t
			//вытаскивая их стека числа хаписывать в t
			
	
	return 0;
}

int bn_mod_to(bn *t, bn const *right)
{
	//испльзуя предыдущую функцию, вычислить остаток от деления
	
	return 0;
}

// Возвести число в степень degree
int bn_pow_to(bn *t, int degree)
{
	//Бинарное возведение в степень
	
	return 0;
}

// Извлечь корень степени reciprocal из BN (бонусная функция)
int bn_root_to(bn *t, int reciprocal)
{
	return 0;
}

// Аналоги операций x = l+r (l-r, l*r, l/r, l%r)
bn* bn_add(bn const *left, bn const *right)
{
	return NULL;
}

bn* bn_sub(bn const *left, bn const *right)
{
	return NULL;
}

bn* bn_mul(bn const *left, bn const *right)
{
	return NULL;
}

bn* bn_div(bn const *left, bn const *right)
{
	return NULL;
}

bn* bn_mod(bn const *left, bn const *right)
{
	return NULL;
}

// Выдать представление BN в системе счисления radix в виде строки
// Строку после использования потребуется удалить.
const char *bn_to_string(bn const *t, int radix)
{
	//цикл, пока число больше radix
		//взять остаток от деления числа на radix
		//полученноу числу сопоставить символ и вывести
	return NULL;
}


