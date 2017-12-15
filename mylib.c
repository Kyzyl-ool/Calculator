#define _CREATE_BN(the_name, the_size) bn* the_name = (bn* )calloc(1, sizeof(bn)); the_name->body = (body_t* )calloc(the_size*DEFAULT_SIZE, sizeof(body_t))
#define _ALLOC_SIZE(the_amount) (trunc(the_amount/DEFAULT_SIZE) + 1)*DEFAULT_SIZE
#define _OPTIMIZE_MEMORY(t) {t->allocated_space = (t->bodysize / DEFAULT_SIZE + 2)*DEFAULT_SIZE; body_t* newb = (body_t* )calloc(t->allocated_space, sizeof(body_t)); for (int i = 0; i < t->bodysize; i++) newb[i] = t->body[i]; free(t->body); t->body = newb;}
#define _STABILIZE(t) {for (int i = 0; i < t->allocated_space; i++) { if (t->body[i] > 9) { t->body[i+1] += t->body[i] / 10; t->body[i] %= 10; } while (t->body[i] < 0) { t->body[i] += 10; t->body[i+1]--; } } t->bodysize = 0; for (int i = 0; i < t->allocated_space; i++) { if (t->body[i] != 0) t->bodysize = i + 1; } if (t->bodysize == 0) t->sign = 0;}
#define _BN_DUMP(b, f) {fprintf(f, "bn's dump.\n{\nbodysize: %d\nbody: ", b->bodysize); for (int i = 0; i < b->bodysize; i++) fprintf(f, "%d", b->body[b->bodysize - 1 - i]); fprintf(f,"\nallocated space: %d\nsign: %d\n}\n", b->amount_of_allocated_blocks, b->sign);}
