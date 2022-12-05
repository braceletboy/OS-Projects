#include "syscall.h"

int main()
{
  char buf[60];
  OpenFileId dst, src;
  int count;

  src = Open("cat.c");
  if (src < 0) Exit(-200);

  while((count = Read(buf, 80, src)) > 0)
    Write(buf, count, ConsoleOutput);

  Write("cat: file is over. \n", 20, ConsoleOutput);
}
