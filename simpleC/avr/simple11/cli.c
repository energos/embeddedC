/* embeddedC/simpleC/avr/simple11/cli.c */

#include <avr/pgmspace.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "cli.h"
#include "uart.h"

char LineBuffer[LINE_BUFFER_SIZE];

/* array de pointers para strings onde estão os elementos da linha de comando */
char *Args[ARGS_MAX];
/* número de argumentos na linha de comando */
unsigned char ArgsN;

/* buffer temporário para conversões ascii/bin */
char tempbuffer[8];

/* código de erro */
unsigned char erroN = 0;

/*---------------------------------------------------------------------------*
 * Interpretador de linha de comando.
 * Copiado e adaptado de:
 * http://lists.gnu.org/archive/html/avr-gcc-list/2004-11/msg00023.html
 */

void fct1(void)
{
  // do whatever you want
  uart_puts("Essa é a função 1\n");
}

void hello(void)
{
  uart_puts_P(PSTR("Welcome to the Monitor\n"));
}

// function name strings
static const char fct1_name[] PROGMEM = "f1";
static const char fct2_name[] PROGMEM = "fill";
static const char fct3_name[] PROGMEM = "dump";
static const char fct4_name[] PROGMEM = "hello";
static const char fct5_name[] PROGMEM = "xsum";

// table
static const func_map_t func_table[] PROGMEM =
  {
   { fct1_name,    fct1 },
   { fct2_name,    fill },
   { fct3_name,    dump },
   { fct4_name,    hello },
   { fct5_name,    xsum }
  };

/*---------------------------------------------------------------------------*
 * void monitor(void)
 *
 * Editor de linha simples
 * A string da linha (terminada com \0) está em LineBuffer
 * O número máximo de caracteres é (LINE_BUFFER_SIZE - 1)
 * Deve ser chamada de dentro de um loop contínuo
 * Ao receber \n quebra em "palavras" a linha em LineBuffer e chama o
 * interpretador de comandos sobre a primeira "palavra" contida em Args[0]
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
  fct_format_t fct = NULL;
  PGM_P name;

  if(uart_kbhit())
    {
      c = uart_getchar();
      switch(c)
        {
        case '\n':
          putcrlf();
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
          /*** parse Args[0] start ***/
          if(n)
            {
              // compare found command against available commands
              for(n = 0; n < sizeof(func_table) / sizeof(func_map_t); n++)
                {
                  // retrieve command name from table
                  name = (PGM_P)pgm_read_word(&(func_table[n].name));
                  // compare with typed command
                  if(strcasecmp_P(Args[0], name) == 0)
                    {
                      // found!
                      // retrieve function pointer
                      fct = (fct_format_t)pgm_read_word(&(func_table[n].fct));
                      break;
                    }
                }
              // call command
              if(fct != NULL)
                {
                  erroN=0;
                  (*fct)();
                  if (erroN)
                    {
                      uart_puts_P(PSTR("Erro "));
                      puthex_byte(erroN);
                      putcrlf();
                      erroN = 0;
                    }
                }
              else
                {
                  uart_puts(Args[0]);
                  uart_puts_P(PSTR(": command not found\n"));
                }
            }
          /*** parse Args[0] end ***/
          putprompt();
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

/* dump address nbytes
   address  - default: (último endereço + 1) ou 0
   nbytes   - default: (último nbytes) ou 256
*/
void dump(void)
{
  static unsigned int a = 0;
  static unsigned int n = 256;

  unsigned char aux[16];
  unsigned char c;
  unsigned int i = 0;
  unsigned char tab;

  if(ArgsN > 2)
    n = strtouint(Args[2]);
  if(ArgsN > 1)
    a = strtouint(Args[1]);
  if(erroN) return;
  while(i < n)
    {
      if(uart_kbhit())
        {
          uart_getchar();
          break;
        }
      /* coloco endereço */
      // putcrlf();
      puthex_word(a);
      put3spaces();
      for(tab = 0; tab < 16; tab++)
        {
          if(i < n)
            {
              c = *(unsigned char *)a++;
              i++;
              puthex_byte(c);
              putspace();
            }
          else
            {
              c = ' ';
              put3spaces();
            }
          aux[tab] = c;
        }
      put3spaces();
      for(tab = 0; tab < 16; tab++)
        {
          /* coloco ascii */
          c = aux[tab];
          if((c < 0x20) || (c >= 0x7F))
            c = '.';
          uart_putchar(c);
        }
      putcrlf();
    }
}

/* ??? Não sei se é o melhor modo de descobrir o fim da RAM usada
 */
extern unsigned char _end;
/* fill byte address nbytes
   byte     - requerido
   address  - default: (último endereço + 1) ou _end
   nbytes   - default: 1
*/
void fill(void)
{
  static unsigned char *a = &_end;
  unsigned char byte;
  unsigned int n;

  if(ArgsN > 3)
    n = strtouint(Args[3]);
  else
    n = 1;
  if(ArgsN > 2)
    a = (unsigned char *)strtouint(Args[2]);
  if(ArgsN > 1)
    byte = strtouint(Args[1]);
  else
    {
      erroN = ERRO_NUMERO_PARAMETROS_INSUFICIENTE;
      return;
    }
  if(erroN) return;
  while(n--)
    *a++ = byte;
}

/* xsum address nbytes
   address  - requerido
   nbytes   - requerido - 0 é interpretado como 65536 (0x10000)
*/
void xsum(void)
{
  unsigned char *a;
  unsigned long int sum = 0;
  unsigned int n;

  if(ArgsN < 3)
    {
      erroN = ERRO_NUMERO_PARAMETROS_INSUFICIENTE;
      return;
    }
  n = strtouint(Args[2]);
  a = (unsigned char *)strtouint(Args[1]);
  if(erroN) return;

  do { sum += *a++; } while(--n);
  puthex_byte(sum >> 16);
  puthex_word(sum);
  putcrlf();
}

/* unsigned int to hexadecimal string */
char *uinttohex(unsigned int x, unsigned char ndigits)
{
  unsigned char x1;
  char *p;
  p = tempbuffer + sizeof(tempbuffer) - sizeof(char);     /* fim do buffer */
  *p = '\0';
  while(ndigits--)
    {
      x1 = x & 0x0f;
      *--p = (x1 >= 10) ? x1 - 10 + 'A' : x1 + '0';
      x /= 16;
    }
  return p;
}

void puthex_byte(unsigned char x)
{
  uart_puts(uinttohex(x, 2));
}

void puthex_word(unsigned int x)
{
  uart_puts(uinttohex(x, 4));
}

/* unsigned int to string */
char *uinttostr(unsigned int numero)
{
  char *p;
  p = tempbuffer + sizeof(tempbuffer) - sizeof(char);     /* fim do buffer */
  *p = '\0';
  do
    {
      *--p = (numero % 10) + '0';
      numero /= 10;
    } while(numero);
  return p;
}

/* int to string */
char *inttostr(int numero)
{
  char *p;
  char minus;

  minus = 0;

  if(numero < 0)
    {
      numero = -numero;
      minus = '-';
    }
  p = uinttostr(numero);

  if(minus)
    {
      *--p = minus;
    }
  return p;
}

/* string to unsigned int */
/* deve ser possível otimizar se não usar strtoul da avr-libc */
unsigned int strtouint(char *s)
{
  unsigned int n;
  int base;
  char *erro;

  if( ((s[1] | 0x20) == 'x') || ((s[2] | 0x20) == 'x') )
    base = 16;
  else
    base = 10;
  n = (unsigned int)strtoul(s, &erro, base);
  if(strlen(erro))
    erroN = ERRO_NUMERO_INVALIDO;
  return n;
}

void putspace(void)
{
  uart_putchar(' ');
}

void put2spaces(void)
{
  putspace();
  putspace();
}

void put3spaces(void)
{
  putspace();
  put2spaces();
}

void putcrlf(void)
{
  uart_putchar('\n');
}

void putprompt(void)
{
  uart_puts_P(PSTR("> "));
}
