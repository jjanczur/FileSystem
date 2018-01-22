#include "FileSystem.h"

void printHelp(const char* name)
{
	printf("----------------------------------------------------------\n");
	printf("Use: %s%s %s[-cdpuwd]%s [option]%s\n", KGRN, name, KCYN, KYEL, KNOR);
	printf("   %s-c %s[size]%s	: Create a file system of a given size\n", KCYN, KYEL, KNOR);
	printf("   %s-s %s[name]%s  : Save s file from disk to the file system\n", KCYN, KYEL, KNOR);
	printf("   %s-p %s[name]%s  : Download a file from the file system to a disk\n", KCYN, KYEL, KNOR);
	printf("   %s-u %s[name]%s  : Delete a file from the file system\n", KCYN, KYEL, KNOR);
	printf("   %s-w%s          	: Display the contents of the file system (files + memory map)\n", KCYN, KNOR);
	printf("   %s-d%s          	: Disk defragmentation.\n", KCYN, KNOR);
	printf("----------------------------------------------------------\n");
}

int main(int argc, char* argv[])
{
	char command[3], parameter[32];
	unsigned size;
	if(argc == 1)
	{
		printHelp(argv[0]);
		return 0;
	}
	strcpy(command, argv[1]);
	if(argc > 2) strcpy(parameter, argv[2]);
	if(strcmp(command, "-c") == 0)
	{
		sscanf(parameter, "%u", &size);
		createFileSystem(size);
	}
	else if(strcmp(command, "-s") == 0) copyFileToFileSystem(parameter);
	else if(strcmp(command, "-p") == 0) copyFileFromFileSystem(parameter);
	else if(strcmp(command, "-u") == 0) deleteFileFromFileSystem(parameter);
	else if(strcmp(command, "-w") == 0) printFileSystem();
	else if(strcmp(command, "-d") == 0)
			{
				openFileSystem();
				defragment();
				updateFileSystem();
			}
	else printHelp(argv[0]);
	return 0;
}