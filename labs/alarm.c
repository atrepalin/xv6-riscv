#include "kernel/types.h"
#include "user/user.h"

void periodic()
{
  printf("alarm!\n");
  sigreturn();
}

int main()
{
  sigalarm(5, periodic);

  for(int i = 0; i < 1000 * 500000; i++){
    if((i % 1000000) == 0)
      fprintf(2, ".");
  }

  sigalarm(0, 0);

  exit(0);
}