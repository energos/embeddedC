/* embeddedC/simpleC/avr/simple11/cli.c */

#include <avr/pgmspace.h>
#include <stdbool.h>
#include <stddef.h>

#include "cli.h"
#include "uart.h"

char LineBuffer[LINE_BUFFER_SIZE];

/* array de pointers para strings onde estão os elementos da linha de comando */
char *Args[ARGS_MAX];
/* número de argumentos na linha de comando */
unsigned char ArgsN;


/*---------------------------------------------------------------------------*
 * void monitor(void)
 *
 * Editor de linha simples
 * A string da linha (terminada com \0) está em LineBuffer
 * O número máximo de caracteres é (LINE_BUFFER_SIZE - 1)
 * Deve ser chamada de dentro de um loop contínuo
 * Ao receber \n quebra em "palavras" a linha em LineBuffer e chama o
 * interpretador de comandos sobre a primeira "palavra"
 *
 * Palavra é qualquer conjunto de caracteres delimitado por um ou mais espaços
 * Espaço ou qualquer controle é aceito como delimitador de palavra
 *
 * a linha de comando será quebrada em até ARGS_MAX palavras (argumentos)
 * Args[0] apontará para o primeiro argumento (comando)
 * Args[1] apontará para o segundo  argumento (primeiro parâmetro)
 * ...
 * Args[3] apontará para o quarto   argumento (terceiro parâmetro)
 *
 * Retorno
 *    número de argumentos em ArgsN
 *    pointers para até ARGS_MAX argumentos em Args[0], Args[1], ...
 *
 * Efeito colateral
 *    altera o conteúdo de LineBuffer, os espaços são substituidos por \0 */

void monitor(void)
{
  static unsigned char charn = 0;   /* contador de caracteres na linha */
  char *p = LineBuffer;
  char c;
  bool firstchar = true;
  unsigned char n = 0;

  if(uart_kbhit())
    {
      c = uart_getchar();
      switch(c)
        {
        case '\n':
          LineBuffer[charn] = 0;
          charn = 0;
          /*** break LineBuffer start ***/
          while((c = *p))
            {
              if((unsigned char)c <= ' ')
                {
                  firstchar = true;
                  *p = 0;
                }
              else
                {
                  if(firstchar)
                    {
                      if(n >= ARGS_MAX) break;
                      Args[n] = p;
                      n++;
                      firstchar = false;
                    }
                }
              p++;
            }
          ArgsN = n;
          /*** break LineBuffer end ***/
          if(n) parse();
          uart_puts_P(PSTR("\n> "));
          break;
        case '\b':                  /* backspace */
        case 0x7f:                  /* DEL */
          if(charn == 0)
            uart_putchar('\a');
          else
            {
              uart_puts_P(PSTR("\b \b"));
              charn--;
              /* remove whole utf-8 sequence */
              while((charn > 0) && ((LineBuffer[charn] & 0xC0)) == 0x80) charn--;
            }
          break;
        default:
          /* coloco no buffer se não for um caracter de controle */
          if((unsigned char)c >= ' ')
            {
              if(charn >= LINE_BUFFER_SIZE - 1)
                {
                  uart_putchar('\a');
                }
              else
                {
                  uart_putchar(c);
                  LineBuffer[charn++] = c;
                }
            }
          break;
        }
    }
}

/*---------------------------------------------------------------------------*
 * Interpretador de linha de comando.
 * Copiado e adaptado de:
 * http://lists.gnu.org/archive/html/avr-gcc-list/2004-11/msg00023.html
 */

void fct1(void)
{
  // do whatever you want
  uart_puts("\nEssa é a função 1");
}

void fct2(void)
{
  // once again you're free to do what you want
  uart_puts("\nEssa é a função 2");
}

void fct3(void)
{
  // yup
  uart_puts("\nEssa é a função 3");
}

void fct4(void)
{
  // ...
  unsigned char i;

  uart_puts("\nHello");
  for(i = 1; i < ArgsN; i++)
    {
      uart_putchar(' ');
      uart_puts(Args[i]);
    }
  uart_putchar('!');
}

// function name strings
static const char fct1_name[] PROGMEM = "f1";
static const char fct2_name[] PROGMEM = "f2";
static const char fct3_name[] PROGMEM = "f3";
static const char fct4_name[] PROGMEM = "Hello";

// table
static const func_map_t func_table[] PROGMEM =
  {
   { fct1_name,    fct1 },
   { fct2_name,    fct2 },
   { fct3_name,    fct3 },
   { fct4_name,    fct4 }
  };

// the parse function
// Args[0] is the string to be compared against the table entries
void parse(void)
{
  fct_format_t fct = NULL;
  unsigned char i;
  PGM_P name;

  // compare found command against available commands
  for(i = 0; i < sizeof(func_table) / sizeof(func_map_t); i++)
    {
      // retrieve command name from table
      name = (PGM_P)pgm_read_word(&(func_table[i].name));
      // compare with typed command
      if(strcasecmp_P(Args[0], name) == 0)
        {
          // found!
          // retrieve function pointer
          fct = (fct_format_t)pgm_read_word(&(func_table[i].fct));
          break;
        }
    }

  // call command
  if(fct != NULL)
    {
      (*fct)();
    }
  else
    {
      uart_puts_P(PSTR("\ncommand not found"));
    }
}
/*
 *---------------------------------------------------------------------------*/
