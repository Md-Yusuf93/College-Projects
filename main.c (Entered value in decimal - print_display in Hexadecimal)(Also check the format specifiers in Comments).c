#include <stdio.h>
#include <stdint.h>
int main (void)
{
    int8_t Value;
    printf ("Enter the Value : ");
    scanf ("%hhd", &Value);  /* if int is used format specifier %d should be used, if int8_t is used format specifier %hhd should be used, if unit32_t or uint8_t is used format specifier %u should be used*/
    
printf ("The value in Hexadecimal is : %x\n", Value);
return 0;
}
