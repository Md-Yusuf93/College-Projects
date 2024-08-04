#include <stdio.h>
#include <stdint.h>
int main()
{
    float Voltage;
    printf ("Enter the Voltage :");
    scanf ("%f", &Voltage);
    

if (Voltage < 4.95)
  {
     printf ("Warning Vcc Low");

}
else if (Voltage > 5.05)
{
   printf ("Warning Vcc High");
  
}
else
{
    printf("Voltage OK");
}
return 0;
}