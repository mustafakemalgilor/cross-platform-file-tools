// libasync_file.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "filereader.h"

int main()
{

	mkemal::filetools::FileReader afr("c:\\sikimsonik2.bin");

	if (afr.Open())
	{
		printf("File open succeeded.\n");
		afr.PrintFileInformation();
		/*if (afr.CopyTo("D:\\update2.bin"))
		{
			printf("Copy operation #1 succeeded.\n");
			mkemal::filetools::FileReader afr2("D:\\update2.bin");
			if (afr2.Open())
			{
				printf("File copy #2 open succeeded.\n");
				afr2.PrintFileInformation();
			}
			
		}
		if (afr.CopyTo("D:\\update3.bin"))
		{
			printf("Copy operation #2 succeeded.\n");
			mkemal::filetools::FileReader afr3("D:\\update3.bin");
			if (afr3.Open())
			{
				printf("File copy #3 open succeeded.\n");
				afr3.PrintFileInformation();
			}

		}*/
	/*	if (afr.Move("C:\\sikimsonik2.bin"))
		{
			printf("move succeeded.\n");
			afr.PrintFileInformation();
		}*/
	}

	/*char c = 0;
	while (afr.Read<char>(&c) > 0)
		printf("%02x", c);*/

	system("pause");
    return 0;
}

