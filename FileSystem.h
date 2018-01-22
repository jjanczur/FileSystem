//
// Created by jacek on 22.01.18.
//

#ifndef SOI_LAB6C_FILESYSTEM_H
#define SOI_LAB6C_FILESYSTEM_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>

#define BLOCK_SIZE 		512
#define SYSTEM_BLOCKS 	8

#define KNOR  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define RESET "\033[0m"

typedef char block[BLOCK_SIZE];

struct inode
{
	unsigned begin;
	unsigned size;
	unsigned blocks;
	char name[32];
	struct inode *next;
};

struct FileSystem
{
	bool inUse;
	char name[32];
	unsigned size;	// size in blocks
	unsigned blocksUsed;
	unsigned numberOfNodes;
	struct inode *inodes;
};

extern struct FileSystem fs;

int createFileSystem(unsigned);
int copyFileToFileSystem(const char*);
int copyFileFromFileSystem(const char*);
int deleteFileFromFileSystem(const char*);
int openFileSystem();
void defragment();
void updateFileSystem();
void printFileSystem();
void closeFileSystem();
struct inode * alloc(unsigned);

#endif //SOI_LAB6C_FILESYSTEM_H

