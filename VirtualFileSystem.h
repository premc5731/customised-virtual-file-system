/*
HEADER FILE WITH ALL MACRO DEFINITIONS, FUNCTION AND DATA STRUCTURE DECLARATIONS
*/
//==================================================================
//HEADER FILE INCLUSIONS
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<fcntl.h>
#include<direct.h>
#include<io.h>
#include<sys/stat.h>

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
#define BSIZE 1024
#define DPERMISSION 3
//-------------------Directory---------------------
#define NSIZE 50
#define MAXPATH 1024
#define ADD true
#define REMOVE false

//--------------------Error------------------------
#define SUCCESS 0
#define ERR_FILE_NOT_EXIST -1
#define ERR_PERMISSION -2
#define ERR_NO_PERMISSION -3
#define ERR_MAXFILES -4
#define ERR_UNIQUE_FILE -5
#define ERR_OFFSET -6
#define ERR_NO_DIR -7
#define ERR_FILE_MODE -8
#define ERR_NO_REGULAR -9
#define ERR_EMPTY_FILE -10
#define ERR_INSUFFICIENT_MEMORY -11
#define ERR_EMPTY_DIRECTORY -12
#define ERR_NON_EMPTY -13
#define ERR_OTHER -14
#define ERR_INCORRECT_FD -15
#define ERR_FILE_FD_NOT_FOUND -16
#define ERR_MKDIR -17
#define ERR_CREAT -18
#define ERR_NO_FILE NULL
#define ERR_DIR_NOT_EXIST NULL

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
typedef struct Inode Inode;
typedef struct Inode* PInode;
typedef struct Inode** PPInode;

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

//5) structure of directory node in directory tree
struct Dnode 
{
	char Dname[NSIZE];
	struct Inode * Inode;
    struct Dnode * Parent;
    struct SDnode * SubDirll;
    struct Dfile * Dfilell;
};
typedef struct Dnode Dnode;
typedef struct Dnode*  PDnode;
typedef struct Dnode**  PPDnode;

//6) structure of sub directory in Linked List
struct SDnode  
{
	char Sname[NSIZE];
    struct Dnode * Saddr;
    struct SDnode * next;
};
typedef struct SDnode SDnode;
typedef struct SDnode*  PSDnode;
typedef struct SDnode**  PPSDnode;

//7) structure of directory file in Linked list
struct Dfile
{ 
	char Fname[NSIZE];
	int iNo;
    struct Dfile * next;
};
typedef struct Dfile Dfile;
typedef struct Dfile*  PDfile;
typedef struct Dfile**  PPDfile;

//==================================================================
//FILE RELATED FUNCTIONS DECLARATION
//==================================================================

//function to create a new file
int CreateFile(char* filename, int permission, PDnode Cwd);

//function to open a file
int OpenFile(char* filename, int mode, PDnode Cwd);

//function to read file
int Read_File(int fd, char* buffer, int count);

//function to write file
int Write_File(int fd, char* buffer, int count);

//function to close file by file name
int CloseFileByName(char* filename, PDnode Cwd);

//function to close all files
int CloseAllFiles();

//function to perform lseek in file
int LseekFile(int fd,int offset,int position);

//function to truncate the file
int truncate_file(char* filename, PDnode Cwd);

//function to delete a file
int RemoveFile(char* filename, PDnode Cwd);

//==================================================================
//DIRECTORY RELATED FUNCTIONS DECLARATION
//==================================================================

//function to create a new directory
int MakeDirectory(char* name, PPDnode first, PPDnode root);

//function to change the current working directory
int ChangeDirectory(char* name, PPDnode Cwd);

//function to delete a empty directory
int RemoveDirectory(char * dirname, PDnode Cwd);

//==================================================================
//UTILITY FUNCTIONS DECLARATION
//==================================================================

//function to create a directory node
PDnode CreateDnode(char* name , PDnode Parent, PInode Inode);

//function to create a sub directory node for linked list
PSDnode CreateSDnode(char* name, PDnode addr);

//function to create a directory file node for linked list
PDfile CreateDfile(char * name,int iNo);

//function to insert sub directory node in linked List
void InsertDir(PPSDnode first, PSDnode child);

//function to insert directory file node in linked List
void InsertFile(PPDfile first, PDfile);

//function to find directory in sub directory linked list
PDnode FindDir(char* name, PSDnode SubDirll);

//function to find filename in directory file linked list
bool Findfilename(char* fname, PDfile Dfilell);

//function to convert name into inode number
int Namei(char* fname, PDnode Cwd);

//function to get filedescriptor from name
int Namefd(char* fname, PDnode Cwd);

//function to get inode address from file name 
PInode GetInode(char* filename, PDnode Cwd);

//function to display sub directories
void DisplaySubDir(PDnode Cwd);

//function to display the files in current working directory 
void DisplayFiles(PDnode Cwd);

//function to get free inode 
PInode GetFreeInode();

//function to get inode address from inode number
PInode GetInoAddr(int iNo);

//function to handle the absolute path
void UpdatePath(const char *dirname, bool option);

//==================================================================
//MISCELLANEOUS FUNCTIONS DECLARATION
//==================================================================

//function to initialise super block
void InitialiseSuperBlock();

//function to create DILB
void createDILB();

//function to display list of commands
void DisplayHelp();

//function to display man page of a command
void man(char* command);

//function to list files and directories 
void lsDirFile(PDnode Cwd);

//function to display file statistics from file name
int file_stat(char* filename, PDnode Cwd);

//function to display file statistics from file descriptor
int file_fstat(int fd);

// function to get the read and write offset for testing
void get_offset(char* filename);

//function to display absolute path of current working directory
void DisplayPath();

//function to display inormation of superblock for testing
void SBDisplay();

//function to store the file structure on secondary storage
int OSWalk(Dnode * dirptr);



