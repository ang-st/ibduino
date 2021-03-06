#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#define CHECKMALLOC(var) if((var) == NULL) {printf("ERROR: malloc\n");abort();}

#define MAXOPSTACK 64
#define MAXNUMSTACK 64

unsigned int t = 0;

// TODO Arduino Refactor
char get_keypress()
{
	return (char)getchar();
}

// BITWISE OP
unsigned int eval_lshift(unsigned int a1, unsigned int a2)
{
	return a1 << a2;
}

unsigned int eval_rshift(unsigned int a1, unsigned int a2)
{
	return a1 >> a2;
}

unsigned int eval_xor(unsigned int a1, unsigned int a2)
{
	return a1 ^ a2;
}

unsigned int eval_or(unsigned int a1, unsigned int a2)
{
	return a1 | a2;
}

unsigned int eval_and(unsigned int a1, unsigned int a2)
{
	return a1 & a2;
}

unsigned int eval_complement(unsigned int a1, unsigned int a2)
{
	return ~a1;
}

// T VARIABLE
unsigned int eval_t(unsigned int a1, unsigned int a2)
{
	return t;
}

unsigned int eval_add(unsigned int a1, unsigned int a2)
{
	return a1 + a2;
}

unsigned int eval_sub(unsigned int a1, unsigned int a2)
{
	return a1 - a2;
}

//unsigned int eval_exp(unsigned int a1, unsigned int a2) { return a2<0 ? 0 : (a2==0?1:a1*eval_exp(a1, a2-1)); }
unsigned int eval_mul(unsigned int a1, unsigned int a2)
{
	return a1 * a2;
}

unsigned int eval_div(unsigned int a1, unsigned int a2)
{
	return a1 / a2;		// GIMME undef :)
}

unsigned int eval_mod(unsigned int a1, unsigned int a2)
{
	return a1 % a2;
}

enum { ASSOC_NONE = 0, ASSOC_LEFT, ASSOC_RIGHT };

struct operator_type {
	char op;
	int prec;
	int assoc;
	int unary;
	unsigned int (*eval) (unsigned int a1, unsigned int a2);
} operators[] = {
	{
	'~', 14, ASSOC_RIGHT, 1, eval_complement}, {
	'^', 7, ASSOC_LEFT, 0, eval_xor}, {
	'|', 6, ASSOC_LEFT, 0, eval_or}, {
	'&', 8, ASSOC_LEFT, 0, eval_and}, {
	'<', 11, ASSOC_LEFT, 0, eval_lshift}, {
	'>', 11, ASSOC_LEFT, 0, eval_rshift}, {
	'*', 8, ASSOC_LEFT, 0, eval_mul}, {
	'/', 8, ASSOC_LEFT, 0, eval_div}, {
	'%', 8, ASSOC_LEFT, 0, eval_mod}, {
	'+', 5, ASSOC_LEFT, 0, eval_add}, {
	'-', 5, ASSOC_LEFT, 0, eval_sub}, {
	't', 1, ASSOC_LEFT, 2, eval_t}, {
	'(', 0, ASSOC_NONE, 0, NULL}, {
	')', 0, ASSOC_NONE, 0, NULL}
};

struct operator_type *getop(char ch)
{
	int i;
	for (i = 0; i < sizeof operators / sizeof operators[0]; ++i) {
		if (operators[i].op == ch)
			return operators + i;
	}
	return NULL;
}

struct operator_type *opstack[MAXOPSTACK];
int nopstack = 0;

unsigned int numstack[MAXNUMSTACK];
int nnumstack = 0;

void push_opstack(struct operator_type *op)
{
	if (nopstack > MAXOPSTACK - 1) {
		fprintf(stderr, "ERROR: Operator stack overflow\n");
		exit(EXIT_FAILURE);
	}
	opstack[nopstack++] = op;
}

struct operator_type *pop_opstack()
{
	if (!nopstack) {
		fprintf(stderr, "ERROR: Operator stack empty\n");
		exit(EXIT_FAILURE);
	}
	return opstack[--nopstack];
}

void push_numstack(unsigned int num)
{
	if (nnumstack > MAXNUMSTACK - 1) {
		fprintf(stderr, "ERROR: Number stack overflow\n");
		exit(EXIT_FAILURE);
	}
	numstack[nnumstack++] = num;
}

unsigned int pop_numstack()
{
	if (!nnumstack) {
		fprintf(stderr, "ERROR: Number stack empty\n");
		exit(EXIT_FAILURE);
	}
	return numstack[--nnumstack];
}

void shunt_op(struct operator_type *op)
{
	struct operator_type *pop;
	unsigned int n1, n2;

	if (op->op == '(') {
		push_opstack(op);
		return;

	} else if (op->op == ')') {
		while (nopstack > 0 && opstack[nopstack - 1]->op != '(') {
			pop = pop_opstack();
			n1 = pop_numstack();

			if (pop->unary)
				push_numstack(pop->eval(n1, 0));
			else {
				n2 = pop_numstack();
				push_numstack(pop->eval(n2, n1));
			}
		}

		if (!(pop = pop_opstack()) || pop->op != '(') {
			fprintf(stderr,
				"ERROR: Stack error. No matching \'(\'\n");
			exit(EXIT_FAILURE);
		}
		return;
	}

	if (op->assoc == ASSOC_RIGHT) {
		while (nopstack && op->prec < opstack[nopstack - 1]->prec) {
			pop = pop_opstack();
			n1 = pop_numstack();
			if (pop->unary)
				push_numstack(pop->eval(n1, 0));
			else {
				n2 = pop_numstack();
				push_numstack(pop->eval(n2, n1));
			}
		}
	} else {
		while (nopstack && op->prec <= opstack[nopstack - 1]->prec) {
			pop = pop_opstack();
			n1 = pop_numstack();
			if (pop->unary)
				push_numstack(pop->eval(n1, 0));
			else {
				n2 = pop_numstack();
				push_numstack(pop->eval(n2, n1));
			}
		}
	}
	push_opstack(op);
}

int isdigit_or_decimal(int c)
{
	if (c == '.' || isdigit(c))
		return 1;
	else
		return 0;
}

int main(int argc, const char *argv[])
{
	char *expression;
	expression = (char *)malloc(128 * sizeof(char));
	CHECKMALLOC(expression);

	int size = 0;
	char c;

	while (size < 128) {
		c = get_keypress();
		if (c == EOF)
			break;

		if (c != '\n') {
			expression[size] = c;
			size++;
		}
	}

	/*printf("%d, %d", isdigit(expression[0]), isdigit_or_decimal(expression[0])); */

	printf("[%s]\n", expression);

	char *expr;
	char *tstart = NULL;
	struct operator_type startop = { 'X', 0, ASSOC_NONE, 0, NULL };	/* Dummy operator to mark start */
	struct operator_type *op = NULL;
	unsigned int n1, n2;
	struct operator_type *lastop = &startop;

	for (expr = expression; *expr; ++expr) {
		if (!tstart) {

			if ((op = getop(*expr))) {

				if (lastop
				    && (lastop == &startop
					|| lastop->op != ')')) {
					if (op->op == '-')
						op = getop('_');
					else if (op->op != '(') {
						fprintf(stderr,
							"ERROR: Illegal use of binary operator (%c)\n",
							op->op);
						exit(EXIT_FAILURE);
					}
				}
				shunt_op(op);
				lastop = op;
			} else if (isdigit_or_decimal(*expr))
				tstart = expr;
			else if (!isspace(*expr)) {
				fprintf(stderr, "ERROR: Syntax error\n");
				return EXIT_FAILURE;
			}
		} else {
			if (isspace(*expr)) {
				push_numstack(atof(tstart));
				tstart = NULL;
				lastop = NULL;
			} else if ((op = getop(*expr))) {
				push_numstack(atof(tstart));
				tstart = NULL;
				shunt_op(op);
				lastop = op;
			} else if (!isdigit_or_decimal(*expr)) {
				fprintf(stderr, "ERROR: Syntax error\n");
				return EXIT_FAILURE;
			}
		}
	}
	if (tstart)
		push_numstack(atof(tstart));

	while (nopstack) {
		op = pop_opstack();
		n1 = pop_numstack();
		if (op->unary)
			push_numstack(op->eval(n1, 0));
		else {
			n2 = pop_numstack();
			push_numstack(op->eval(n2, n1));
		}
	}

	if (nnumstack != 1) {
		fprintf(stderr,
			"ERROR: Number stack has %d elements after evaluation. Should be 1.\n",
			nnumstack);
		return EXIT_FAILURE;
	}
	printf("%x\n", numstack[2]);

	return EXIT_SUCCESS;
}
