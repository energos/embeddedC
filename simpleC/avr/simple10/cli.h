/* embeddedC/simpleC/avr/simple10/cli.h */

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

// function pointer prototype
typedef void (*fct_format_t)(void);

// the table element prototype
typedef struct
{
  const char* name;
  fct_format_t fct;
} func_map_t;

void monitor(void);
void breakLine(void);
void parse(void);

#endif
