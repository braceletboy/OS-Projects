#include "syscall.h"

void fastRead(){
	OpenFileId src;
	int count;
	char buf[10];
	int totalRead;
        src = Open("in.dat");
	totalRead = 0;
	while ( (count = Read(buf, 2, src)) > 0 )
	{
	    Write("##########fastRead: ",20, ConsoleOutput);
	    Write(buf, count, ConsoleOutput);
		totalRead += count;
		Yield();
	}
	Close(src);


	Exit(totalRead);
}

int main()
{
        OpenFileId src;
        int count;
        char buf[10];
        int totalRead;
	int pid;

	pid = Fork(fastRead);
	Yield();

        src = Open("in.dat");
        totalRead = 0;
        while ( (count = Read(buf, 1, src)) > 0 )
        {
            Write("slowRead: ",10, ConsoleOutput);
            Write(buf, count, ConsoleOutput);
                totalRead += count;
                Yield();
        }
        Close(src);
	Yield();

	Exit(totalRead);
}