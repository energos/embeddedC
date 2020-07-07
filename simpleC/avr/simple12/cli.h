/* embeddedC/simpleC/avr/simple12/cli.h */

#ifndef _CLI_H_
#define _CLI_H_ 1

/* número de caracteres no buffer de edição da linha de comando + null */
#define LINE_BUFFER_SIZE 28

/* número máximo de argumentos na linha de comando */
#define ARGS_MAX 4

/* array de pointers para strings onde estão os elementos da linha de comando */
extern char *Args[];
/* número de argumentos na linha de comando */
extern unsigned char ArgsN;

/* códigos de erro retornado por erroN */
#define ERRO_NUMERO_PARAMETROS_INSUFICIENTE 1
#define ERRO_NUMERO_INVALIDO 2
extern unsigned char erroN;

// function pointer prototype
typedef void (*fct_format_t)(void);

// the table element prototype
typedef struct
{
  const char *name;
  fct_format_t fct;
} func_map_t;

void monitor(void);
char *uinttohex(unsigned int x, unsigned char ndigits);
void puthex_byte(unsigned char x);
void puthex_word(unsigned int x);
char *uinttostr(unsigned int numero);
char *inttostr(int numero);
unsigned int strtouint(char *s);
void dump(void);
void fill(void);
void hello(void);
void xsum(void);
void putspace(void);
void put2spaces(void);
void put3spaces(void);
void putcrlf(void);
void putprompt(void);

#define poke(X, Y) (*(unsigned char *)(X) = (Y))
#define peek(X) (*(unsigned char *)(X))

#endif
