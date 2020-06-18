/* embeddedC/simpleC/avr/simple01/simple.c */

#include <avr/io.h>

int main(void)
{
  unsigned int prescaler = 0;

  /* configuro PORT B como saida */
  PORTB = 0xFF;
  DDRB = 0xFF;

  while(1)
    {
      if(prescaler++ > 10000)
        {
          prescaler = 0;
          PORTB--;
        }
    }
  return 0;
}
