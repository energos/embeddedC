/* embeddedC/simpleC/avr/simple12/cli.c */

#include <avr/pgmspace.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

#include "cli.h"
#include "uart.h"

char LineBuffer[LINE_BUFFER_SIZE];

/* array de pointers para strings onde estão os elementos da linha de comando */
char *Args[ARGS_MAX];
/* número de argumentos na linha de comando */
unsigned char ArgsN;

/* buffer temporário para conversões ascii/bin */
char tempbuffer[8];
/* buffer temporário para funções dump() e ihexparser() */
unsigned char aux[32];

/* código de erro */
unsigned char erroN = 0;

unsigned char ihexmode = 0;
unsigned char ihexxsum = 0;

/* define qual memória (ram, flash, externa, etc.) será acessada */
unsigned char memory = 0;

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
static const char fct6_name[] PROGMEM = "ihex";
static const char fct7_name[] PROGMEM = "ihexv";
static const char fct8_name[] PROGMEM = "use_ram";
static const char fct9_name[] PROGMEM = "use_flash";

// table
static const func_map_t func_table[] PROGMEM =
  {
   { fct1_name,    fct1 },
   { fct2_name,    fill },
   { fct3_name,    dump },
   { fct4_name,    hello },
   { fct5_name,    xsum },
   { fct6_name,    ihex },
   { fct7_name,    ihexv },
   { fct8_name,    use_ram },
   { fct9_name,    use_flash }
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
      LineBuffer[charn] = 0;
      switch(c)
        {
        case '\n':
          putcrlf();
          charn = 0;
          erroN = 0;
          /*** try parsing as intel hex ***/
          if(*p == ':')
            {
              ihexparser();
            }
          else
            {
              /*** break LineBuffer start ***/
              while((c = *p))
                {
                  /* ignoro tudo depois de um '#'
                     isso permite comentários na linha de comando */
                  if(c == '#') break;

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
                  ihexmode = IHEXMODE_IDDLE;
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
                      (*fct)();
                    }
                  else
                    {
                      erroN = ERRO_COMANDO_DESCONHECIDO;
                    }
                }
              /*** parse Args[0] end ***/
            }
          if (erroN)
            {
              uart_puts_P(PSTR("Erro "));
              puthex_byte(erroN);
              putcrlf();
            }
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
        case '\f':
          uart_puts_P(PSTR("\e[2J\e[H"));
          putprompt();
          uart_puts(LineBuffer);
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
      puthex_word(a);
      put3spaces();
      for(tab = 0; tab < 16; tab++)
        {
          if(i < n)
            {
              c = peek(a++);
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
  static unsigned int a = (unsigned int)&_end;
  unsigned char byte = 0;
  unsigned int n = 1;

  if(ArgsN > 3)
    n = strtouint(Args[3]);
  if(ArgsN > 2)
    a = strtouint(Args[2]);
  if(ArgsN > 1)
    byte = strtouint(Args[1]);
  else
      erroN = ERRO_NUMERO_PARAMETROS_INSUFICIENTE;
  if(erroN) return;
  while(n--)
    poke(a++, byte);
}

/* xsum address nbytes
   address  - requerido
   nbytes   - requerido - 0 é interpretado como 65536 (0x10000)
*/
void xsum(void)
{
  unsigned int a;
  unsigned long int sum = 0;
  unsigned int n;

  if(ArgsN < 3)
    {
      erroN = ERRO_NUMERO_PARAMETROS_INSUFICIENTE;
      return;
    }
  n = strtouint(Args[2]);
  a = strtouint(Args[1]);
  if(erroN) return;

  do { sum += peek(a++); } while(--n);
  puthex_byte(sum >> 16);
  puthex_word(sum);
  putcrlf();
}


static const char msg_receiving [] PROGMEM = "Receiving...\n";

void ihex(void)
{
  uart_puts_P(msg_receiving);
  ihexmode = IHEXMODE_WRITE;
}

void ihexv(void)
{
  uart_puts_P(msg_receiving);
  ihexmode = IHEXMODE_VERIFY;
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
  switch(ihexmode)              /* testing */
    {
    case IHEXMODE_IDDLE:
      uart_putchar('I');
      break;
    case IHEXMODE_VERIFY:
      uart_putchar('V');
      break;
    case IHEXMODE_WRITE:
      uart_putchar('W');
      break;
    default:
      uart_putchar('?');
      break;
    }
  switch(memory)
    {
    case MEMORY_RAM:
      poke = ram_write;
      peek = ram_read;
      uart_putchar('R');        /* testing */
      break;
    case MEMORY_FLASH:
      poke = null_write;
      peek = flash_read;
      uart_putchar('F');        /* testing */
      break;
    default:
      uart_putchar('?');        /* testing */
      break;
    }
  uart_puts_P(PSTR("> "));
}

// retorna um caracter da string apontada por pp
// a cada chamada o ponteiro da string (*pp) será incrementado, a menos
// que se tenha chegado ao fim da string (\0)
unsigned char sgetchar(unsigned char **pp)
{
  unsigned char c;
  c = *(*pp);
  if(c) (*pp)++;      // não deixo o pointer (*pp) ser incrementado além do \0
  return c;
}

// converte caracter ascii de 0-9, A-F, a-f em binário de 0x00 até 0x0f
// retorna 0x00 até 0x0f se caracter for hexadecimal válido, 0xff se não
unsigned char hex2nibble(unsigned char c)
{
  c -= '0';
  if(c < 10) return c;
  c |= 0x20;          // passo para minúsculo
  c -= 'a' - '0' - 0x0a;
  if(c < 10) return 0xff;
  if(c < 16) return c;
  return 0xff;
}

// converte 2 caracteres hexadecimais (nibbles) da string apontada por pp
// em um byte (unsigned char) sem sinal (nibble mais significativo primeiro)
// o checksum (intel hex) é acumulado em ihexxsum
// se receber caracter hexadecimal inválido, seta a variável global erroN
// e retorna 0x00
unsigned char shex_getbyte(unsigned char **pp)
{
  unsigned char i;
  unsigned char nibble;
  unsigned char n = 0;

  for(i = 0; i < 2; i++)
    {
      nibble = hex2nibble(sgetchar(pp));
      if(nibble == 0xff)
        {
          erroN = IHEX_ERROR_NOT_HEX;
          return 0;
        }
      n = (n << 4) + nibble;
    }
  ihexxsum += n;
  return n;
}

// converte 4 caracteres hexadecimais (nibbles) da string apontada por pp
// em um word (unsigned int) sem sinal (nibble mais significativo primeiro)
// o checksum (intel hex) é acumulado em ihexxsum
unsigned int shex_getword(unsigned char **pp)
{
  unsigned int n;

  n = shex_getbyte(pp) << 8;
  return n + shex_getbyte(pp);
}


/**
   Intel Hex Record parser
   hex record de até 32 bytes, ou linha de
   75 caracteres + NULL (ou CR ou LF) = buffer de 76 bytes

   Por exemplo:
   :2081A0009804DFC417CAB75FF69277C05AC21318C1359F15D2BEEA620D497449DC986C29E1

   Retorna 0 se linha é intel válida, e código de erro em caso contrário.

   Dependendo da variável global ihexmode é feita escrita ou verificação
   da memória.
**/
// !!! adicionar offset ???

void ihexparser(void)
{
  unsigned char n, i;
  unsigned int address;
  unsigned char type;

  unsigned char *p = (unsigned char*)LineBuffer;

  // pego ': '
  if(sgetchar(&p) != ':')
    {
      erroN = IHEX_ERROR_NOT_INTEL;
      return;
    }

  ihexxsum = 0;

  // pego número de bytes
  n = shex_getbyte(&p);

  if(n > 32)
    {
      erroN = IHEX_ERROR_SIZE;
      return;
    }

  // pego endereço
  address = shex_getword(&p);

  // pego record type
  type = shex_getbyte(&p);

  // pego n bytes
  for(i = 0; i < n; i++)
    {
      aux[i] = shex_getbyte(&p);
    }

  // pego xsum (e ignoro-o)
  shex_getbyte(&p);

  if(erroN)
    {
      return;
    }

  if(ihexxsum)
    {
      erroN = IHEX_ERROR_XSUM;
      return;
    }

  // se "End of File Record" termino
  if(type == 1)
    {
      ihexmode = IHEXMODE_IDDLE;
      return;
    }

  // gravo ou verifico
  switch(ihexmode)
    {
    case IHEXMODE_WRITE:
      for(i = 0; i < n; i++)
        {
          poke(address + i, aux[i]);
          if(erroN) ihexmode = IHEXMODE_IDDLE;
        }
      break;
    case IHEXMODE_VERIFY:
      for(i = 0; i < n; i++)
        {
          if(peek(address + i) != aux[i])
            {
              erroN = IHEX_ERROR_VERIFY;
              ihexmode = IHEXMODE_IDDLE;
              break;;
            }
        }
      break;
    }

  return;
}

#ifdef PEEK_AND_POKE_ARE_FUNCTIONS
void ram_write(unsigned int address, unsigned char byte)
{
  *(unsigned char*)address = byte;
}

unsigned char ram_read(unsigned int address)
{
  return *(unsigned char*)address;
}

void null_write(__attribute__((unused)) unsigned int address, __attribute__((unused)) unsigned char byte)
{
  erroN = ERROR_UNWRITABLE_LOCATION;
}

unsigned char flash_read(unsigned int address)
{
  return pgm_read_byte((unsigned char*)address);
}

void (*poke)(unsigned int address, unsigned char byte) = ram_write;
unsigned char (*peek)(unsigned int address) = ram_read;

void use_ram(void)
{
  memory = MEMORY_RAM;
}

void use_flash(void)
{
  memory = MEMORY_FLASH;
}

#endif
