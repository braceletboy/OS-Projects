#include "syscall.h"

int main()
{
  OpenFileId id;
  char buf[128];
  Read(buf, 10, ConsoleOutput);
  Write(buf, 10, ConsoleInput);

  Create("piggy.txt");
  id = Open("piggy.txt");
  Read(buf, 10, id);
  Write(buf, 10, id);
  Close(id);
}
