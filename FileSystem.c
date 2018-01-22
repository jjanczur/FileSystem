//
// Created by jacek on 21.01.18.
//

#include <stdbool.h>
#include "FileSystem.h"

struct FileSystem fs = { false, "myfilesystem" };
/**
*	create a file sytem
*/
int createFileSystem(unsigned size)
{
    unsigned i;
    fs.inUse = false;
    if(size <= SYSTEM_BLOCKS)
    {
        printf(" The disk size is too small, it must contain > %d blocks\n", SYSTEM_BLOCKS);
        return 1;
    }
    fs.size = size;
    fs.blocksUsed = SYSTEM_BLOCKS;
    fs.numberOfNodes = 0;
    fs.inodes = NULL;
    fs.inUse = true;
    block buff;	// single block
    for(i = 0; i < BLOCK_SIZE; ++i)
    {
        buff[i] = '\0';
    }
    FILE* newFS = fopen(fs.name, "w");
    for(i = 0; i < size; ++i)
    {
        // saving a single block size times - size - num of blocks
        int result = fwrite(buff, sizeof(char), sizeof(buff), newFS);
        if(result < (int)sizeof(buff))
            if (ferror(newFS))
            {
                printf("Error! Can't create a file system.!\n");
                return 1;
            }
    }
    fclose(newFS);
    updateFileSystem();
    return 0;
}

int copyFileToFileSystem(const char *fileName)
{
    openFileSystem();
    unsigned i, fileSize, fileSizeInBlocks;
    unsigned freeBlocks = fs.size - fs.blocksUsed;
    if(fs.inUse == false)
    {
        printf("No file system is in use!\n");
        return;
    }
    FILE* inputFile = fopen(fileName, "r");
    if(!inputFile)
    {
        printf("The file could not be opened '%s'!\n", fileName);
        return 1;
    }
    struct inode *seekInode = fs.inodes;
    if(seekInode != NULL)
    {
        if(strcmp(seekInode->name, fileName) == 0)
        {
            fclose(inputFile);
            return 1;
        }
        while(seekInode->next != NULL)
        {
            seekInode = seekInode->next;
            if(strcmp(seekInode->name, fileName) == 0)
            {
                fclose(inputFile);
                return 1;
            }
        }
    }
    /*fseek(FILE *file, long offset, int mode);*/
    /*the size of a new file */
    fseek(inputFile, 0, SEEK_END);
    fileSize = ftell(inputFile);
    fseek(inputFile, 0, SEEK_SET);
    fileSizeInBlocks = ((fileSize - 1) / BLOCK_SIZE) + 1;
    if(fileSizeInBlocks > freeBlocks)
    {
        printf("There is no space for the file '%s'\n", fileName);
        return 1;
    }
    struct inode *newNode = alloc(fileSizeInBlocks);
    fs.numberOfNodes++;
    fs.blocksUsed += fileSizeInBlocks;
    newNode->size = fileSize;
    newNode->blocks = fileSizeInBlocks;
    strcpy(newNode->name, fileName);
    /*add a new file to file system */
    FILE* source = fopen(fs.name, "r+");
    block buff;
    int a = 0;
    fseek(source, newNode->begin * BLOCK_SIZE, 0);
    for(i = 0; i < newNode->blocks; ++i)
    {
        fread(buff, sizeof(char), sizeof(buff), inputFile);
        fwrite(buff, sizeof(char), sizeof(buff), source);
        ++a;
    }
    fclose(source);
    fclose(inputFile);
    updateFileSystem();
    return 0;
}

int copyFileFromFileSystem(const char *fileName)
{
    openFileSystem();
    unsigned i, remainingData;
    int result;
    struct inode *node = fs.inodes;
    if(fs.inUse == false)
    {
        printf("No file system is in use!\n");
        return;
    }
    while(node != NULL)
    {
        if(strcmp(node->name, fileName) == 0)
        {
            FILE* inputFile = fopen(fs.name, "r");
            FILE* outputFile = fopen(node->name, "w");
            block buff;
            remainingData = node->size;
            fseek(inputFile, node->begin * BLOCK_SIZE, 0);
            while(remainingData > 0)
            {
                if(remainingData > BLOCK_SIZE)
                {
                    fread(buff, sizeof(char), BLOCK_SIZE, inputFile);
                    fwrite(buff, sizeof(char), BLOCK_SIZE, outputFile);
                    remainingData -= BLOCK_SIZE;
                }
                else
                {
                    fread(buff, sizeof(char), remainingData, inputFile);
                    fwrite(buff, sizeof(char), remainingData, outputFile);
                    remainingData = 0;
                }
            }
            fclose(inputFile);
            fclose(outputFile);
            printf("I read the file\n");
            return 0;
        }
        node = node->next;
    }
    printf("There is nothing to read :(\n");
    return 1;
}


int deleteFileFromFileSystem(const char *filename)
{
    openFileSystem();
    struct inode *prev = NULL, *node = fs.inodes;
    if(fs.inUse == false)
    {
        printf("No file system is in use!\n");
        return 1;
    }
    while(node != NULL)
    {
        if(strcmp(node->name, filename) == 0)
        {
            if(prev == NULL)
            {
                fs.inodes = node->next;
            }
            else
            {
                prev->next = node->next;
            }
            fs.blocksUsed -= node->blocks;
            --fs.numberOfNodes;
            free(node);
            updateFileSystem();
            return 0;
        }
        prev = node;
        node = node->next;
    }
    return 1;
}

int openFileSystem()
{
    unsigned i, j, copiedNodes = 0;
    FILE* source = fopen(fs.name, "r");
    if(!source)
    {
        printf("Failed to open the file system '%s'!\n", fs.name);
        return 1;
    }
    block buff;
    char tmp[32];
    fread(buff, sizeof(char), sizeof(buff), source);	// information block about FS
    memcpy(&tmp, &buff[1], 32);
    sscanf(tmp, "%32s", fs.name);	// name the file system
    memcpy(&tmp, &buff[33], 32);
    sscanf(tmp, "%32u", &(fs.size));
    memcpy(&tmp, &buff[65], 32);
    sscanf(tmp, "%32u", &(fs.blocksUsed));
    memcpy(&tmp, &buff[87], 32);
    sscanf(tmp, "%32u", &(fs.numberOfNodes));
    fs.inodes = NULL;
    fs.inUse = true;
    struct inode *node = NULL, *newNode = NULL;
    for(i = 1; i < SYSTEM_BLOCKS; ++i)
    {
        fread(buff, sizeof(char), sizeof(buff), source);
        for(j = 0; j < 8; ++j)
        {
            if(copiedNodes < fs.numberOfNodes)
            {
                newNode = malloc(sizeof(struct inode));
                if(fs.inodes == NULL) fs.inodes = newNode;
                else node->next = newNode;
                newNode->next = NULL;
                memcpy(&tmp, &buff[64*j + 1], 31);
                sscanf(tmp, "%8u%16u%7u", &newNode->begin, &newNode->size, &newNode->blocks);
                memcpy(&tmp, &buff[64*j + 32], 32);
                sscanf(tmp, "%32s", newNode->name);
                ++copiedNodes;
                node = newNode;
            }
            else return 0;
        }
    }
    return 0;
}

void defragment()
{
    unsigned i, position = SYSTEM_BLOCKS;
    unsigned nodePosition = position;
    block buff;
    if(fs.inUse == false)
    {
        printf("No file system is in use!\n");
        return;
    }
    struct inode *node = fs.inodes;
    FILE* source = fopen(fs.name, "r+");
    fseek(source, SYSTEM_BLOCKS * BLOCK_SIZE, 0);
    while(node != NULL)
    {
        if(node->begin == position)
        {
            position = node->begin + node->blocks;
        }
        else
        {
            nodePosition = node->begin;
            node->begin = position;
            for(i = 0; i < node->blocks; ++i, ++position, ++nodePosition)
            {
                fseek(source, nodePosition * BLOCK_SIZE, 0);
                fread(buff, sizeof(char), sizeof(buff), source);
                fseek(source, position * BLOCK_SIZE, 0);
                fwrite(buff, sizeof(char), sizeof(buff), source);
            }
        }
        node = node->next;
    }
    fclose(source);
}

void updateFileSystem()
{
    unsigned i, j;
    if(fs.inUse == false)
    {
        printf("No file system is in use!\n");
        return;
    }
    //    r+     Open  for  reading and writing.  The stream is positioned at the
    //           beginning of the file.
    FILE* source = fopen(fs.name, "r+");
    block buff;

    char tmp[32];
    sprintf(tmp, "%-32s", fs.name);	// name the file system
    memcpy(&buff[1], &tmp, 32);
    sprintf(tmp, "%-32u", fs.size);
    memcpy(&buff[33], &tmp, 32);
    sprintf(tmp, "%-32u", fs.blocksUsed);
    memcpy(&buff[65], &tmp, 32);
    sprintf(tmp, "%-32u", fs.numberOfNodes);
    memcpy(&buff[87], &tmp, 32);
    fwrite(buff, sizeof(char), sizeof(buff), source);
    struct inode *node = fs.inodes;
    for(i = 1; i < SYSTEM_BLOCKS; ++i)
    {
        for(j = 0; j < 8; ++j)
        {
            if(node != NULL)
            {
                sprintf(tmp, "%-8u%-16u%-7u", node->begin, node->size, node->blocks);
                memcpy(&buff[64*j + 1], &tmp, 31);
                sprintf(tmp, "%-32s", node->name);
                memcpy(&buff[64*j + 32], &tmp, 32);
                node = node->next;
            }
        }
        fwrite(buff, sizeof(char), sizeof(buff), source);
    }
    fclose(source);
}

void printFileSystem()
{
    openFileSystem();
    unsigned i = 0, position = 0;
    unsigned blocksInLine = 5;
    if(fs.inUse == false)
    {
        printf("No file system is in use!\n");
        return;
    }
    printf("----------------------------------------------------------\n");
    printf("        The contents of the file system '%s'\n", fs.name);
    struct inode *node = fs.inodes;
    printf(" - size: %d blocks, used: %d, free: %u\n", fs.size, fs.blocksUsed, fs.size - fs.blocksUsed);
    printf(" - SYSTEM_BLOCKS: %d\n", SYSTEM_BLOCKS);
    printf(" - numberOfNodes: %d\n", fs.numberOfNodes);
    printf("* Node:\t(name)\t\t(begin)\t(size)\t(blocks)\n");
    while(node != NULL)
    {
        printf("\t%s\t%d\t%d\t%d\n", node->name, node->begin+1, node->size, node->blocks);
        node = node->next;
    }
    printf("----------------------------------------------------------\n");
    printf("\t\t\t       Memory map\n");
    node = fs.inodes;
    while(position < SYSTEM_BLOCKS)
    {
        ++position;
        printf(KRED "SYS_BLOCK\t" RESET);
        if(++i >= blocksInLine) { printf("\n"); i = 0; }
    }
    while(node != NULL)
    {
        while(position < node->begin)
        {
            ++position;
            printf("FREE BLOCK\t");
            if(++i >= blocksInLine) { printf("\n"); i = 0; }
        }
        while(position < node->begin + node->blocks)
        {
            ++position;
            printf(KGRN "%s%s\t", node->name, RESET);
            if(++i >= blocksInLine) { printf("\n"); i = 0; }
        }
        node = node->next;
    }
    while(position < fs.size)
    {
        ++position;
        printf("FREE BLOCK\t");
        if(++i >= blocksInLine) { printf("\n"); i = 0; }
    }
    printf("----------------------------------------------------------\n");
}

struct inode *alloc(unsigned blocks)
{
    unsigned i, hole;
    struct inode *node = fs.inodes;
    struct inode *newNode = malloc(sizeof(struct inode));
    while(1)
    {
        if(node == NULL || (node->begin - SYSTEM_BLOCKS >= blocks))
        {
            newNode->next = fs.inodes;
            fs.inodes = newNode;
            newNode->begin = SYSTEM_BLOCKS;	// is a place after system  blocks
            return newNode;
        }
        while(node->next != NULL)
        {
            // we're looking for a hole between the fragments of memory
            hole = node->next->begin - (node->begin + node->blocks);
            if(hole >= blocks)
            {
                newNode->begin = (node->begin + node->blocks);
                newNode->next = node->next;
                node->next = newNode;
                return newNode;
            }
            node = node->next;
        }
        if(fs.size - (node->begin + node->blocks) >= blocks)
        {
            newNode->begin = node->begin + node->blocks;
            newNode->next = NULL;
            node->next = newNode;
            return newNode;	// there is a place at the end
        }
        // if you could not find enough continuous space
        // let's consolidate it
        defragment();
    }
}