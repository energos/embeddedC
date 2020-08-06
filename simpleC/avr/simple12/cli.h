/* embeddedC/simpleC/avr/simple12/cli.h */

#ifndef _CLI_H_
#define _CLI_H_ 1

#define PEEK_AND_POKE_ARE_FUNCTIONS

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
enum {
      ERRO_NENHUM = 0,
      ERRO_COMANDO_DESCONHECIDO,
      ERRO_NUMERO_PARAMETROS_INSUFICIENTE,
      ERRO_NUMERO_INVALIDO,
      IHEX_ERROR_NOT_INTEL,
      IHEX_ERROR_NOT_HEX,
      IHEX_ERROR_XSUM,
      IHEX_ERROR_SIZE,
      IHEX_ERROR_VERIFY,
      IHEX_ERROR_WRITE,
      ERROR_UNWRITABLE_LOCATION };

enum {
      IHEXMODE_IDDLE = 0,
      IHEXMODE_VERIFY,
      IHEXMODE_WRITE,
      IHEXMODE_ENDRECORD };

#define MEMORY_RAM              0
#define MEMORY_FLASH            1

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
unsigned char hex2nibble(unsigned char c);
unsigned char sgetchar(unsigned char **pp);
unsigned char shex_getbyte(unsigned char **pp);
unsigned int shex_getword(unsigned char **pp);

#ifdef PEEK_AND_POKE_ARE_FUNCTIONS
void (*poke)(unsigned int address, unsigned char byte);
unsigned char (*peek)(unsigned int address);

void ram_write(unsigned int address, unsigned char byte);
unsigned char ram_read(unsigned int address);
void null_write(unsigned int address, unsigned char byte);
unsigned char flash_read(unsigned int address);
void use_ram(void);
void use_flash(void);
#else
#define poke(X, Y) (*(unsigned char *)(X) = (Y))
#define peek(X) (*(unsigned char *)(X))
# endif

#endif
