/*
HEADER FILE WITH ALL MACRO DEFINITIONS, FUNCTION AND DATA STRUCTURE DECLARATIONS
*/
//==================================================================
//HEADER FILE INCLUSIONS
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
//==================================================================
//MACRO DEFINITIONS
#define MAXFILES 100
#define MAXINODES 100
#define MAXFILESIZE 1024
#define READ 1
#define WRITE 2
#define REGULAR 1
#define SPECIAL 2
#define START 0
#define CURRENT 1
#define END 2
//==================================================================
//DATA STRUCTURE DECLARATIONS
//1) structure of super block
struct SuperBlock
{
	int TotalInodes;
	int FreeInodes;	
};

//2) structure of memory Inode
struct Inode
{
	char FileName[50];
	int InodeNumber;
	int FileSize;
	int ActualFileSize;
	int FileType;
	char *FileData;
	int FileLinkCount;
	int ReferenceCount;
	int FilePermission;
	struct Inode* next;
};

//3) structure of File Table
struct FileTable
{
	int ReadOffset;
	int WriteOffset;
	int count;
	int mode;
	struct Inode* PtrInode;
};

//4) structure of user file descriptor table - UFDT
struct ufdt
{
	struct FileTable* PtrFileTable;
};
//==================================================================
//FUNCTION DECLARATIONS
//display the help
void DisplayHelp();

//function to get file descriptor from file name
int GetFDFromName(char* filename);

//function to get the inode
struct Inode*  GetInode(char* filename);

//function to initialise SuperBlock
void InitialiseSuperBlock();

//function to create Disk Inode List Block
void createDILB();

//function to create file
int CreateFile(char* filename, int permission);

//function to remove file
int RemoveFile(char* filename);

//function to read file
int Read_File(int fd, char* buffer, int count);

//function to write file
int Write_File(int fd, char* buffer, int count);

//function to open file
int OpenFile(char* filename, int mode);

//function to close file by file descriptor
void CloseFileByFD(int fd);

//function to close file by file name
int CloseFileByName(char* filename);

//function to close all files
void CloseAllFiles();

//function to perform lseek in file
int LseekFile(int fd,int offset,int position);

//function to list files
void ls_file();

//function to get file statistics by FD
int file_fstat(int fd);

//function to get file statistics by name
int file_stat(char* filename);

//function to truncate file
int truncate_file(char* filename);

void man(char* command);

// function to get the read and write offset
void get_offset(char* filename);
//==================================================================
