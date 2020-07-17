/* embeddedC/simpleC/avr/simple12/cli.h */

#ifndef _CLI_H_
#define _CLI_H_ 1

/* número de caracteres no buffer de edição da linha de comando + null */
/* originalmente tinha 28 bytes,
   aumentado para caber um intel hex record de 32 bytes (75 caracteres + null) */
#define LINE_BUFFER_SIZE 76

/* número máximo de argumentos na linha de comando */
#define ARGS_MAX 4

/* array de pointers para strings onde estão os elementos da linha de comando */
// !!! extern char *Args[];
/* número de argumentos na linha de comando */
// !!! extern unsigned char ArgsN;

/* códigos de erro retornado por erroN */
#define ERRO_NENHUM            0
#define ERRO_COMANDO_DESCONHECIDO 1
#define ERRO_NUMERO_PARAMETROS_INSUFICIENTE 2
#define ERRO_NUMERO_INVALIDO   3
#define IHEX_ERROR_NOT_INTEL   4
#define IHEX_ERROR_NOT_HEX     5
#define IHEX_ERROR_XSUM        6
#define IHEX_ERROR_SIZE        7
#define IHEX_ERROR_VERIFY      8
#define IHEX_ERROR_WRITE       9
// !!! extern unsigned char erroN;

#define IHEX_MODE_IDDLE         0
#define IHEX_MODE_VERIFY        1
#define IHEX_MODE_WRITE         2
#define IHEX_MODE_ENDRECORD     3

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
void ihex(void);
void ihexv(void);
void putspace(void);
void put2spaces(void);
void put3spaces(void);
void putcrlf(void);
void putprompt(void);
void ihexparser(void);

#define poke(X, Y) (*(unsigned char *)(X) = (Y))
#define peek(X) (*(unsigned char *)(X))

#endif
