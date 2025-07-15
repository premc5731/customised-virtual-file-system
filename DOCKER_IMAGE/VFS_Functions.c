/*
FUNCTION DEFINITIONS
*/

//include header file created for this project
#include "VirtualFileSystem.h"

//these variables declared in other file
extern struct SuperBlock SUPERBLOCK;
extern struct Inode INODE;
extern struct FileTable FILETABLE;
extern struct ufdt UFDT[MAXFILES];
extern struct Inode* Head;

//1)===========================================================================================
//function to initialise the super block - initialising the members of UFDT to NULL, initialising the two members of SUPERBLOCK
void InitialiseSuperBlock()
{
	int i = 0;
	//struct ufdt has one member - pointer to filetable entry, ufdt is an array of 100 such objects
	//initialise all members of UFDT to NULL
	for(i=0;i<100;i++)
	{
		UFDT[i].PtrFileTable = NULL;
	}
	SUPERBLOCK.TotalInodes = MAXINODES;
	SUPERBLOCK.FreeInodes = MAXINODES;
	
	printf("SuperBlock is initialised successfully\n");
}

//2)===========================================================================================
void createDILB()
{
	//loop counter
	int i = 0;
	//create a temp pointer for navigation
	// since head is a global variable so default value of head is NULL
	struct Inode* temp = Head;
	for(i=0;i<MAXINODES;i++)
	{
		//create a new node
		struct Inode* newn = (struct Inode*) malloc(sizeof(struct Inode));
		//initialise the new node
		newn->InodeNumber = i;
		newn->FileSize = 0;
		newn->ActualFileSize = 0;
		newn->FileType = 0;
		newn->FileData = NULL;
		newn->FileLinkCount = 0;
		newn->ReferenceCount = 0;
		newn->FilePermission = 0;
		newn->next = NULL;
		
		//add this new node at the end of the linked list
		//if linked list is empty
		if(temp == NULL) 
		{
			temp = newn;
			//store newn is Head - first node address
			Head = newn;
		}
		//if linked list is not empty - add newn after temp
		else
		{
			temp->next = newn;
			temp = temp->next; 
		}
	}
	printf("DILB is initialised successfully\n");
}

//3)===========================================================================================
void DisplayHelp()
{
	printf("ls: List the files\n");
	printf("closeall: Close all the open files\n");
	printf("cls: Clear the console\n");
	printf("exit: Terminate the filesystem application\n");
	printf("stat: Display information of a file using filename\n");
	printf("fstat: Display information of a file using file descriptor\n");
	printf("rm: Remove/delete a file\n");
	printf("man: Display man page of the command\n");
	printf("write: Write to an existing file\n");
	printf("truncate: Remove all data from the file\n");
	printf("create: Create a new file\n");
	printf("open: Open an existing file\n");
	printf("read: Read an existing file\n");
	printf("offset: To get an offset of a file\n");
}

//4)===========================================================================================
//function to display the list of files
void ls_file()
{
	//create a temp pointer to navigate through the DILB
	struct Inode* temp = Head;
	
	//check if there are no files in filesystem - means all inodes are free
	if(SUPERBLOCK.FreeInodes == MAXINODES)
	{
		printf("Error: There are no files\n");
		return;
	}
	
	printf("Filename\tInode Number\tFile size\tLink Count\n");
	printf("---------------------------------------------------------\n");
	while(temp != NULL)
	{
		if(temp->FileType != 0)
		{
			printf("%s\t\t%d\t\t%d\t\t%d\n",temp->FileName,temp->InodeNumber,temp->ActualFileSize,temp->FileLinkCount);
		}
		temp = temp->next;
	}
	printf("---------------------------------------------------------\n");
}

//5)===========================================================================================
//function to display man page of a command
void man(char* command)
{
	if(command == NULL)
	{
		return;
	}
	
	if(strcmp(command,"ls") == 0)
	{
		printf("DESCRIPTION: This command is used to list the files and their properties\n");
		printf("USAGE: ls\n");
	}
	else if(strcmp(command,"closeall") == 0)
	{
		printf("DESCRIPTION: close all the open files\n");
		printf("USAGE: closeall\n");
	}
	else if(strcmp(command,"help") == 0)
	{
		printf("DESCRIPTION: Displays list of all commands supported\n");
		printf("USAGE: help\n");
	}
	else if(strcmp(command,"exit") == 0)
	{
		printf("DESCRIPTION: Exit the application\n");
		printf("USAGE: exit\n");
	}
	else if(strcmp(command,"stat") == 0)
	{
		printf("DESCRIPTION: Displays the properties of a file whose filename is specified\n");
		printf("USAGE: stat filename\n");
	}
	else if(strcmp(command,"fstat") == 0)
	{
		printf("DESCRIPTION: Displays the properties of a file whose file descriptor is specified\n");
		printf("USAGE: fstat filedescriptor\n");
	}
	else if(strcmp(command,"close") == 0)
	{
		printf("DESCRIPTION: close the specified file\n");
		printf("close filename\n");
	}
	else if(strcmp(command,"rm") == 0)
	{
		printf("DESCRIPTION: delete the specified file\n");
		printf("USAGE: rm filename\n");
	}
	else if(strcmp(command,"man") == 0)
	{
		printf("DESCRIPTION: Displays description and usage of a command\n");
		printf("USAGE: man command\n");
	}
	else if(strcmp(command,"write") == 0)
	{
		printf("DESCRIPTION: Write to a regular file\n");
		printf("USAGE: write filename content_to_be_written_to_file\n");
	}
	else if(strcmp(command,"truncate") == 0)
	{
		printf("DESCRIPTION: Delete contents of sepcified file\n");
		printf("USAGE: truncate filename\n");
	}
	else if(strcmp(command,"create") == 0)
	{
		printf("DESCRIPTION: Create a new file\n");
		printf("USAGE: create filename file_permission\n");
	}
	else if(strcmp(command,"open") == 0)
	{
		printf("DESCRIPTION: Open specified file in specified mode\n");
		printf("USAGE: open filename mode\n");
	}
	else if(strcmp(command,"read") == 0)
	{
		printf("DESCRIPTION: Read the specified file\n");
		printf("USAGE: read filename number_of_bytes_to_read\n");
	}
	else if(strcmp(command,"lseek") == 0)
	{
		printf("DESCRIPTION: Change the file offset\n");
		printf("USAGE: lseek filename offset start_point\n");
	}
	else if(strcmp(command,"offset") == 0)
	{
		printf("DESCRIPTION: To Check the file offset\n");
		printf("USAGE: offset filename \n");
	}
	else
	{
		printf("No man documentation found\n");
	}
}

//6)===========================================================================================
//function to get an inode for specified file (filename to inode conversion)
struct Inode* GetInode(char* filename)
{
	//create a temp pointer for navigation through the DILB
	struct Inode* temp = Head;
	
	//if filename is NULL - do nothing, return
	if(filename == NULL)
	{
		return NULL;
	}
	
	//navigate through the DILB till you reach unallocated inode
	while(temp != NULL)
	{
		//check the input parameter filename with filename stored in inode (pointed to by temp)
		if((strcmp(filename,temp->FileName) == 0) && (temp->FileType != 0))
		{
			break;
		}
		temp = temp->next;
	}
	return temp;
}

//7)===========================================================================================
//function to create a new file
int CreateFile(char* filename, int permission)
{
	//loop counter
	int i = 0;
	//temp pointer to navigate through the DILB
	struct Inode* temp = Head;
	
	//check for incorrect input parameters ,blank file name, incorrect permissions
	if((filename == NULL) || (permission <= 0) || (permission > 3))
	{
		//Incorrect create parameters
		printf("permission %d\n",permission);
		printf("filename: %s\n",filename);
		return -1;
	}
	
	//check if there are no free inodes in the system
	if(SUPERBLOCK.FreeInodes == 0)
	{
		//No free inodes available
		return -2;
	}
	
	//check if file with this name is already present
	if(GetInode(filename) != NULL)
	{
		//file already exists
		return -3;
	}
	
	//navigate through the DILB till you either get an inode with filetype = 0 means inode for deleted file or a blank inode at the end
	while(temp != NULL)
	{
		if(temp->FileType == 0)
		{
			break;
		}
		temp = temp->next;
	}
	
	SUPERBLOCK.FreeInodes = SUPERBLOCK.FreeInodes - 1;
	
	//now temp is pointing to a free inode that we can use for our new file
	
	//get an empty slot in UFDT - UFDT is an array of pointers
	for(i=0;i<MAXFILES;i++)
	{
		if(UFDT[i].PtrFileTable == NULL)
		{
			break;
		}
	}
	
	//create a FileTable slot
	UFDT[i].PtrFileTable = (struct FileTable*)malloc(sizeof(struct FileTable));
	
	//check if memory allocation failed
	if(UFDT[i].PtrFileTable == NULL)
	{
		//memory allocation failed
		return -4;
	}
	
	//initialise the members of FileTable slot that we just created
	UFDT[i].PtrFileTable->ReadOffset = 0;
	UFDT[i].PtrFileTable->WriteOffset = 0;
	UFDT[i].PtrFileTable->count = 1;
	UFDT[i].PtrFileTable->mode = permission;
	UFDT[i].PtrFileTable->PtrInode = temp;
	
	//initialise the members of the new inode that we fetched
	strcpy(UFDT[i].PtrFileTable->PtrInode->FileName,filename);
	UFDT[i].PtrFileTable->PtrInode->InodeNumber = i;
	UFDT[i].PtrFileTable->PtrInode->FileSize = MAXFILESIZE;
	UFDT[i].PtrFileTable->PtrInode->ActualFileSize = 0;
	UFDT[i].PtrFileTable->PtrInode->FileType = REGULAR;
	UFDT[i].PtrFileTable->PtrInode->FileData = (char*) malloc(MAXFILESIZE);
	memset(UFDT[i].PtrFileTable->PtrInode->FileData,0,1024);
	UFDT[i].PtrFileTable->PtrInode->FileLinkCount = 1;
	UFDT[i].PtrFileTable->PtrInode->ReferenceCount = 1;
	UFDT[i].PtrFileTable->PtrInode->FilePermission = permission;
	
	//return the file descriptor - index of UFDT
	return i;
}

//8)===========================================================================================
//function to return file descriptor for a filename
int GetFDFromName(char* filename)
{
	//loop counter
	int i = 0;
	for(i=0;i<MAXFILES;i++)
	{
		if(UFDT[i].PtrFileTable != NULL)
		{
			if(strcmp(UFDT[i].PtrFileTable->PtrInode->FileName,filename) == 0)
			{
				break;
			}
		}		
	}
	
	if(i == MAXFILES)
	{
		return -1;
	}
	else 
	{
		return i;
	}
}

//9)===========================================================================================
//function to delete a file
int RemoveFile(char* filename)
{
	int fd = GetFDFromName(filename);
	//if fd = -1, means the file does not exist
	if(fd == -1)
	{
		return -1;
	}
	
	//if fd found, means file exists
	//decrement the link count of this file
	UFDT[fd].PtrFileTable->PtrInode->FileLinkCount = (UFDT[fd].PtrFileTable->PtrInode->FileLinkCount) - 1;
	
	//if resulting link count is 0, means the file is deleted
	if(UFDT[fd].PtrFileTable->PtrInode->FileLinkCount == 0)
	{
		UFDT[fd].PtrFileTable->PtrInode->FileType = 0;
		//free the file table slot
		free(UFDT[fd].PtrFileTable);
		UFDT[fd].PtrFileTable = NULL;
		//increment the count of free inodes in superblock
		SUPERBLOCK.FreeInodes = SUPERBLOCK.FreeInodes + 1;
	}
	
	return 0;
}

//10)===========================================================================================
//function to display file attributes using file name
int file_stat(char* filename)
{
	if(filename == NULL)
	{
		return -1;
	}
		
	//temp pointer for navigation through the DILB
	struct Inode* temp = Head;
	while(temp != NULL)
	{
		if((strcmp(temp->FileName,filename) == 0) && (temp->FileType != 0))
		{
			break;
		}
		temp = temp->next;
	}
	//if you navigate through DILB without finding filename, means file is not found
	if(temp == NULL)
	{
		return -2;
	}
	
	//display the file attributes
	printf("--------------------------------------\n");
	printf("FileName: %s\n",temp->FileName);
	printf("FileInodeNumber: %d\n",temp->InodeNumber);
	printf("FileSize: %d\n",temp->ActualFileSize);
	printf("FileLinkCount: %d\n",temp->FileLinkCount);
	printf("FileType: ");
	if(temp->FileType == 1)
	{
		printf("REGULAR\n");
	}
	else if(temp->FileType == 2)
	{
		printf("SPECIAL\n");
	}		
	printf("File Permissions: ");
	if(temp->FilePermission == 1)
	{
		printf("Read\n");
	}
	else if(temp->FilePermission == 2)
	{
		printf("Write\n");
	}
	else if(temp->FilePermission == 3)
	{
		printf("Read + Write\n");
	}
	printf("--------------------------------------\n");

	return 0;
}

//11)===========================================================================================
//function to display file attributes using file descriptor
int file_fstat(int fd)
{
	printf("fd = %d\n",fd);
	//incorrect fd
	if((fd < 0)||(fd >= MAXFILES))
	{
		return -1;
	}
	
	//file with this fd does not exist
	else if(UFDT[fd].PtrFileTable == NULL)
	{
		return -2;
	}
	
	//display the file attributes
	printf("--------------------------------------\n");
	printf("FileName: %s\n",UFDT[fd].PtrFileTable->PtrInode->FileName);
	printf("FileInodeNumber: %d\n",UFDT[fd].PtrFileTable->PtrInode->InodeNumber);
	printf("FileSize: %d\n",UFDT[fd].PtrFileTable->PtrInode->ActualFileSize);
	printf("FileLinkCount: %d\n",UFDT[fd].PtrFileTable->PtrInode->FileLinkCount);
	printf("FileType: ");
	if(UFDT[fd].PtrFileTable->PtrInode->FileType == 1)
	{
		printf("REGULAR\n");
	}
	else if(UFDT[fd].PtrFileTable->PtrInode->FileType == 2)
	{
		printf("SPECIAL\n");
	}		
	printf("File Permissions: ");
	if(UFDT[fd].PtrFileTable->PtrInode->FilePermission == 1)
	{
		printf("Read\n");
	}
	else if(UFDT[fd].PtrFileTable->PtrInode->FilePermission == 2)
	{
		printf("Write\n");
	}
	else if(UFDT[fd].PtrFileTable->PtrInode->FilePermission == 3)
	{
		printf("Read + Write\n");
	}
	printf("--------------------------------------\n");

	return 0;
}

//12)===========================================================================================
//function to close a file by its name
int CloseFileByName(char* filename)
{
	//get the file descriptor of the file
	int fd = GetFDFromName(filename);
	
	//if file does not exist
	if(fd == -1)
	{
		return -1;
	}
	
	//decrement the reference count
	UFDT[fd].PtrFileTable->PtrInode->ReferenceCount = UFDT[fd].PtrFileTable->PtrInode->ReferenceCount - 1;
	
	if(UFDT[fd].PtrFileTable->PtrInode->ReferenceCount == 0)
	{
		free(UFDT[fd].PtrFileTable);
		UFDT[fd].PtrFileTable = NULL;
	}
	
	return 0;
}

//13)===========================================================================================
//function to close all files
void CloseAllFiles()
{
	//loop counter
	int i = 0;
	for(i=0;i<MAXFILES;i++)
	{
		if(UFDT[i].PtrFileTable != NULL)
		{
			UFDT[i].PtrFileTable->PtrInode->ReferenceCount = 0;
			free(UFDT[i].PtrFileTable);
			UFDT[i].PtrFileTable = NULL;
		}
	}
}

//14)===========================================================================================
//function to open a file
int OpenFile(char* filename, int mode)
{
	//temp poiner to navigae through the DILB
	struct Inode* temp = Head;
	
	//check for incorrect input parameters
	if((filename == NULL)||(mode <= 0))
	{
		// printf("mode : \n",mode);
		// printf("file name : \n",filename);
		return -1;
	}
	
	//get the inode for the filename
	temp = GetInode(filename);
	//check if no inode found - means file not found
	if(temp == NULL)
	{
		return -2;
	}
	
	//check for inode permission
	if(temp->FilePermission < mode)
	{
		return -3;
	}
	
	//get an empty slot in UFDT
	//navigate through the UFDT
	int i = 0;
	for(i=0;i<MAXFILES;i++)
	{
		if(UFDT[i].PtrFileTable == NULL)
		{
			break;
		}
	}
	
	//allocate memory for file table slot
	UFDT[i].PtrFileTable = (struct FileTable*) malloc(sizeof(struct FileTable));
	
	//check for memory allocation failure
	if(UFDT[i].PtrFileTable == NULL)
	{
		return -1;
	}
	
	//initialise the new FileTable node
	UFDT[i].PtrFileTable->count = 1;
	UFDT[i].PtrFileTable->mode = mode;
	if(mode == READ)
	{
		UFDT[i].PtrFileTable->ReadOffset = 0;
	}
	else if(mode == WRITE)
	{
		UFDT[i].PtrFileTable->WriteOffset = temp->ActualFileSize;
	}
	else if(mode == READ + WRITE)
	{
		UFDT[i].PtrFileTable->ReadOffset = 0;
		UFDT[i].PtrFileTable->WriteOffset = temp->ActualFileSize;
	}
	UFDT[i].PtrFileTable->PtrInode = temp;
	
	//increment the reference count in the inode
	UFDT[i].PtrFileTable->PtrInode->ReferenceCount = UFDT[i].PtrFileTable->PtrInode->ReferenceCount + 1;
	
	return i;
}

//15)===========================================================================================
//function to write to a file
int Write_File(int fd, char* buffer, int count)
{
	//check file permission in FileTable
	if((UFDT[fd].PtrFileTable->mode != WRITE) && (UFDT[fd].PtrFileTable->mode != READ+WRITE)) 
	{
		return -1;
	}
	
	//check file permission in Inode
	if((UFDT[fd].PtrFileTable->PtrInode->FilePermission != WRITE) && (UFDT[fd].PtrFileTable->PtrInode->FilePermission != READ+ WRITE))
	{
		return -1;
	}
	
	//check if file already contains maximum data that it can hold
	if((UFDT[fd].PtrFileTable->WriteOffset) == MAXFILESIZE)
	{
		return -2;
	}
	
	//check if file is not a regular file
	if(UFDT[fd].PtrFileTable->PtrInode->FileType != REGULAR)
	{
		return -3;
	}
	
	//copy the data
	strncpy(((UFDT[fd].PtrFileTable->PtrInode->FileData)+(UFDT[fd].PtrFileTable->WriteOffset)),buffer,count);
	
	//update the write offset
	UFDT[fd].PtrFileTable->WriteOffset = UFDT[fd].PtrFileTable->WriteOffset + count;
	
	//update the file size
	UFDT[fd].PtrFileTable->PtrInode->ActualFileSize = UFDT[fd].PtrFileTable->PtrInode->ActualFileSize + count;
	
	return count;	
}

//16)===========================================================================================
//function to read a file
int Read_File(int fd, char* buffer, int count)
{
	int read_size = 0;
	
	//check if file exists
	if(UFDT[fd].PtrFileTable == NULL)
	{
		return -1;
	}
	//check if file has read permission in FileTable - means it is opened in read or read+write mode
	if((UFDT[fd].PtrFileTable->mode != READ) && (UFDT[fd].PtrFileTable->mode != READ + WRITE))
	{
		return -2;
	}
	//check if file has read permission in Inode
	if((UFDT[fd].PtrFileTable->PtrInode->FilePermission != READ) && (UFDT[fd].PtrFileTable->PtrInode->FilePermission != READ+WRITE))
	{
		return -2;
	}
	//check if read offset is at the end of the file
	if(UFDT[fd].PtrFileTable->ReadOffset == UFDT[fd].PtrFileTable->PtrInode->ActualFileSize)
	{
		return -3;
	}
	//check if file is not a regular file
	if(UFDT[fd].PtrFileTable->PtrInode->FileType != REGULAR)
	{
		return -4;
	}
	
	//calculate the data to be read
	read_size = UFDT[fd].PtrFileTable->PtrInode->ActualFileSize - UFDT[fd].PtrFileTable->ReadOffset;
	
	if(read_size < count)
	{
		strncpy(buffer,UFDT[fd].PtrFileTable->PtrInode->FileData + UFDT[fd].PtrFileTable->ReadOffset,read_size);
		UFDT[fd].PtrFileTable->ReadOffset = UFDT[fd].PtrFileTable->ReadOffset + read_size;
		// printf("Inside read if , read_size : %d \n",read_size);
		return read_size;
	}
	else
	{
		strncpy(buffer,UFDT[fd].PtrFileTable->PtrInode->FileData + UFDT[fd].PtrFileTable->ReadOffset,count);
		UFDT[fd].PtrFileTable->ReadOffset = UFDT[fd].PtrFileTable->ReadOffset + count;
		// printf("Inside else if , count : %d \n",count);		
		return count;
	}	
}

//17)===========================================================================================
//function to truncate a file
int truncate_file(char* filename)
{
	//get file descriptor for the file
	int fd = GetFDFromName(filename);
	if(fd == -1)
	{
		return -1;
	}
	
	//zero out the File_Data for file with received fd
	memset(UFDT[fd].PtrFileTable->PtrInode->FileData,0,1024);
	
	//reset ReadOffset, WriteOffset and ActualFileSize to 0
	UFDT[fd].PtrFileTable->ReadOffset = 0;
	UFDT[fd].PtrFileTable->WriteOffset = 0;
	UFDT[fd].PtrFileTable->PtrInode->ActualFileSize = 0;
	
	return 0;
}

//18)===========================================================================================
//function to lseek a file - change file offset
int LseekFile(int fd, int offset, int position)
{
	//check for invalid file descriptors
	if((fd < 0) || (fd > 2))
	{
		return -1;
	}
	if(UFDT[fd].PtrFileTable == NULL)
	{
		return -1;
	}
	
	//lseek if file is opened in READ or READ+WRITE mode
	if((UFDT[fd].PtrFileTable->mode == READ)||(UFDT[fd].PtrFileTable->mode == READ+WRITE))
	{
		//for CURRENT position
		if(position == CURRENT)
		{
			//if new offset is beyond file
			if((UFDT[fd].PtrFileTable->ReadOffset+offset)>(UFDT[fd].PtrFileTable->PtrInode->ActualFileSize))
			{
				return -1;
			}
			//invalid new file offset - negative
			if((UFDT[fd].PtrFileTable->ReadOffset+offset) < 0)
			{
				return -1;
			}
			//valid new file offset - update the new file offset
			UFDT[fd].PtrFileTable->ReadOffset = UFDT[fd].PtrFileTable->ReadOffset + offset;
		}
		//for START position
		else if(position == START)
		{
			//if new file offset is beyond file size
			if(offset > UFDT[fd].PtrFileTable->PtrInode->ActualFileSize)
			{
				return -1;
			}
			//if new file offset becomes negative
			if(offset < 0)
			{
				return -1;
			}
			//update the new file offset
			UFDT[fd].PtrFileTable->ReadOffset = offset;
		}
		//for END position
		else if(position == END)
		{
			//chk if new file offset is beyond file
			if((UFDT[fd].PtrFileTable->PtrInode->ActualFileSize + offset) > UFDT[fd].PtrFileTable->PtrInode->ActualFileSize)
			{
				return -1;
			}
			//new file offset as negative
			if((UFDT[fd].PtrFileTable->PtrInode->ActualFileSize + offset) < 0)
			{
				return -1;
			}
			//valid new file offset - update the ReadOffset
			UFDT[fd].PtrFileTable->ReadOffset = UFDT[fd].PtrFileTable->PtrInode->ActualFileSize + offset;
		}
	}
	//lseek when file is opened in WRITE mode
	else if(UFDT[fd].PtrFileTable->mode == WRITE)
	{
		//lseek for position == CURRENT
		if(position == CURRENT)
		{
			if((UFDT[fd].PtrFileTable->WriteOffset + offset) > MAXFILESIZE)
			{
				return -1;
			}
			if((UFDT[fd].PtrFileTable->WriteOffset + offset) < 0)
			{
				return -1;
			}
			if((UFDT[fd].PtrFileTable->WriteOffset+offset)>(UFDT[fd].PtrFileTable->PtrInode->ActualFileSize))
			{
				UFDT[fd].PtrFileTable->PtrInode->ActualFileSize=UFDT[fd].PtrFileTable->WriteOffset+offset;
			}
			
			UFDT[fd].PtrFileTable->WriteOffset=UFDT[fd].PtrFileTable->WriteOffset + offset;

			
		}
		//lseek for position == START
		else if(position == START)
		{
			if(offset > MAXFILESIZE)
			{
				return -1;
			}
			if(offset < 0)
			{
				return -1;
			}
			if(offset > UFDT[fd].PtrFileTable->PtrInode->ActualFileSize)
			{
				UFDT[fd].PtrFileTable->PtrInode->ActualFileSize = offset;
			}
			
			UFDT[fd].PtrFileTable->WriteOffset = offset;
			
		}
		//lseek for position == END
		else if(position == END)
		{
			if((UFDT[fd].PtrFileTable->PtrInode->ActualFileSize + offset) > MAXFILESIZE)
			{
				return -1;
			}
			if((UFDT[fd].PtrFileTable->WriteOffset + offset) < 0)
			{
				return -1;
			}
			if((UFDT[fd].PtrFileTable->PtrInode->ActualFileSize + offset) > (UFDT[fd].PtrFileTable->PtrInode->ActualFileSize))
			{
				UFDT[fd].PtrFileTable->WriteOffset = (UFDT[fd].PtrFileTable->PtrInode->ActualFileSize + offset);

				UFDT[fd].PtrFileTable->PtrInode->ActualFileSize = (UFDT[fd].PtrFileTable->PtrInode->ActualFileSize + offset);
			}
			else
			{
				UFDT[fd].PtrFileTable->WriteOffset = (UFDT[fd].PtrFileTable->PtrInode->ActualFileSize + offset);
			}
		}
	}	
	printf("ReadOffset: %d\n",UFDT[fd].PtrFileTable->ReadOffset);
	printf("WriteOffset: %d\n",UFDT[fd].PtrFileTable->WriteOffset);
	return 0;
}

void get_offset(char* filename)
{
	int fd = GetFDFromName(filename);
	if(fd == -1)
	{
		printf("file not found\n");
		return;
	}
	printf("ReadOffset: %d\n",UFDT[fd].PtrFileTable->ReadOffset);
	printf("WriteOffset: %d\n",UFDT[fd].PtrFileTable->WriteOffset);
	
}
