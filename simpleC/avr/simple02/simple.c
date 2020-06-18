/* embeddedC/simpleC/avr/simple02/simple.c */

/* http://www.nongnu.org/avr-libc/user-manual/modules.html
   http://www.nongnu.org/avr-libc/user-manual/group__util__delay.html */

#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
  /* configuro PORT B como saida */
  PORTB = 0xFF;
  DDRB = 0xFF;

  while(1)
    {
      _delay_ms(500);
      PORTB--;
    }

  return 0;
}
