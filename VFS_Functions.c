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
extern PDnode Cwd;
extern PDnode root;
extern char path[MAXPATH];

//=============================================================================
//FILE RELATED FUNCTIONS DEFINITION 
//=============================================================================

//1)function to create a new file
int CreateFile(char* filename, int permission, PDnode Cwd)
{
	//ptr to hold Dfile entry
	PDfile ptrDfile = NULL;

	//to hold the free ino
	int iNo = -1;

	//loop counter
	int i = 0;

	//temp pointer to hold the free inode
	struct Inode* temp = NULL;

	if(Cwd == NULL) //there should be at least 1 primary directory 
	{
		return ERR_NO_DIR;
	}
	
	//check for incorrect input parameters ,blank file name, incorrect permissions
	if((filename == NULL) || (permission <= 0) || (permission > 3))
	{
		//Incorrect create parameters
		//printf("permission %d\n",permission);
		//printf("filename: %s\n",filename);
		return ERR_PERMISSION;
	}
	
	//check if there are no free inodes in the system
	if(SUPERBLOCK.FreeInodes == 0)
	{
		//No free inodes available
		return ERR_MAXFILES;
	}
	
	//check if file with this name is already present
	if(Findfilename(filename, Cwd->Dfilell))
	{
		//file already exists
		return ERR_UNIQUE_FILE;
	}
	
	//navigate through the DILB till you either get an inode with filetype = 0 means inode for deleted file or a blank inode at the end
    temp = GetFreeInode();

	//no free inodes
	if(temp == NULL)
	{
		return ERR_MAXFILES;
	}
	
	//now temp is pointing to a free inode that we can use for our new file

	SUPERBLOCK.FreeInodes = SUPERBLOCK.FreeInodes - 1;
	
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
		printf("Unable to allocate memory\n");
		exit(-1);
	}
	
	//initialise the members of FileTable slot that we just created
	UFDT[i].PtrFileTable->ReadOffset = 0;
	UFDT[i].PtrFileTable->WriteOffset = 0;
	UFDT[i].PtrFileTable->count = 1;
	UFDT[i].PtrFileTable->mode = permission;
	UFDT[i].PtrFileTable->PtrInode = temp;
	
	//initialise the members of the new inode that we fetched
	//strcpy(UFDT[i].PtrFileTable->PtrInode->FileName,filename); // remove Fname from inode
	//UFDT[i].PtrFileTable->PtrInode->InodeNumber = i;
	UFDT[i].PtrFileTable->PtrInode->FileSize = MAXFILESIZE;
	UFDT[i].PtrFileTable->PtrInode->ActualFileSize = 0; 
	UFDT[i].PtrFileTable->PtrInode->FileType = REGULAR;
	UFDT[i].PtrFileTable->PtrInode->FileData = (char*) malloc(MAXFILESIZE);
	memset(UFDT[i].PtrFileTable->PtrInode->FileData,0,1024);
	UFDT[i].PtrFileTable->PtrInode->FileLinkCount = 1;
	UFDT[i].PtrFileTable->PtrInode->ReferenceCount = 1;
	UFDT[i].PtrFileTable->PtrInode->FilePermission = permission;

	//create and add the file entry in the linked list of Dfile of cwd
	ptrDfile = CreateDfile(filename, temp->InodeNumber);
	InsertFile(&(Cwd->Dfilell), ptrDfile);
	
	//return the file descriptor - index of UFDT
	return i;
}

//=============================================================================
//2)function to open a file
int OpenFile(char* filename, int mode, PDnode Cwd)
{
	//temp poiner to hold the inode
	struct Inode* temp = NULL;
	
	//check for incorrect input parameters
	if((filename == NULL)||(mode <= 0))
	{
		return ERR_FILE_MODE;
	}
	
	//get the inode for the filename
	temp = GetInode(filename, Cwd);

	//check if no inode found - means file not found
	if(temp == NULL)
	{
		return ERR_FILE_NOT_EXIST;
	}
	
	//check for inode permission
	if(temp->FilePermission < mode)
	{
		return ERR_NO_PERMISSION;
	}
	
	//get an empty slot in UFDT
	//navigate through the UFDT
	int i;
	for(i=0;i<MAXFILES;i++)
	{
		//printf("577 : %d : %s\n",i , UFDT[i].PtrFileTable);
		if(UFDT[i].PtrFileTable == NULL)
		{
			//printf("579 : %d\n",i);
			break;
		}
	}
	
	//allocate memory for file table slot
	UFDT[i].PtrFileTable = (struct FileTable*) malloc(sizeof(struct FileTable));
	
	//check for memory allocation failure
	if(UFDT[i].PtrFileTable == NULL)
	{
		printf("Unable to allocate memory\n");
		exit(-1);
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

	//assign the inode in the filetable
	UFDT[i].PtrFileTable->PtrInode = temp;
	
	//increment the Linkcount count in the inode
	UFDT[i].PtrFileTable->PtrInode->FileLinkCount = UFDT[i].PtrFileTable->PtrInode->FileLinkCount + 1;

	return i;
}

//=============================================================================
//3)function to read a file
int Read_File(int fd, char* buffer, int count)
{
	int read_size = 0;
	
	//check if file exists
	if(UFDT[fd].PtrFileTable == NULL)
	{
		return ERR_FILE_NOT_EXIST;
	}
	//check if file has read permission in FileTable - means it is opened in read or read+write mode
	if((UFDT[fd].PtrFileTable->mode != READ) && (UFDT[fd].PtrFileTable->mode != READ + WRITE))
	{
		return ERR_NO_PERMISSION;
	}
	//check if file has read permission in Inode
	if((UFDT[fd].PtrFileTable->PtrInode->FilePermission != READ) && (UFDT[fd].PtrFileTable->PtrInode->FilePermission != READ+WRITE))
	{
		return ERR_NO_PERMISSION;
	}
	//check if read offset is at the end of the file
	if(UFDT[fd].PtrFileTable->ReadOffset == UFDT[fd].PtrFileTable->PtrInode->ActualFileSize)
	{
		return ERR_OFFSET;
	}
	if(UFDT[fd].PtrFileTable->PtrInode->ActualFileSize == 0)
	{
		return ERR_EMPTY_FILE;
	}
	//check if file is not a regular file
	if(UFDT[fd].PtrFileTable->PtrInode->FileType != REGULAR)
	{
		return ERR_NO_REGULAR;
	}
	
	//calculate the data to be read
	read_size = UFDT[fd].PtrFileTable->PtrInode->ActualFileSize - UFDT[fd].PtrFileTable->ReadOffset;
	
	if(read_size < count)
	{
		strncpy(buffer,UFDT[fd].PtrFileTable->PtrInode->FileData + UFDT[fd].PtrFileTable->ReadOffset,read_size);
		UFDT[fd].PtrFileTable->ReadOffset = UFDT[fd].PtrFileTable->ReadOffset + read_size;
		return read_size;
	}
	else
	{
		strncpy(buffer,UFDT[fd].PtrFileTable->PtrInode->FileData + UFDT[fd].PtrFileTable->ReadOffset,count);
		UFDT[fd].PtrFileTable->ReadOffset = UFDT[fd].PtrFileTable->ReadOffset + count;
		return count;
	}	
}


//=============================================================================
//4)function to write to a file
int Write_File(int fd, char* buffer, int count)
{
	//check file permission in FileTable
	if((UFDT[fd].PtrFileTable->mode != WRITE) && (UFDT[fd].PtrFileTable->mode != READ+WRITE)) 
	{
		return ERR_PERMISSION;
	}
	
	//check file permission in Inode
	if((UFDT[fd].PtrFileTable->PtrInode->FilePermission != WRITE) && (UFDT[fd].PtrFileTable->PtrInode->FilePermission != READ+ WRITE))
	{
		return ERR_PERMISSION;
	}
	
	//check if file already contains maximum data that it can hold
	if((UFDT[fd].PtrFileTable->WriteOffset) == MAXFILESIZE)
	{
		return ERR_INSUFFICIENT_MEMORY;
	}
	
	//check if file is not a regular file
	if(UFDT[fd].PtrFileTable->PtrInode->FileType != REGULAR)
	{
		return ERR_NO_REGULAR;
	}

	//copy the data
	strncpy(((UFDT[fd].PtrFileTable->PtrInode->FileData)+(UFDT[fd].PtrFileTable->WriteOffset)),buffer,count);
	
	//update the write offset
	UFDT[fd].PtrFileTable->WriteOffset = UFDT[fd].PtrFileTable->WriteOffset + count;
	
	//update the file size
	UFDT[fd].PtrFileTable->PtrInode->ActualFileSize = UFDT[fd].PtrFileTable->PtrInode->ActualFileSize + count;
	
	//return no of bytes written
	return count;	
}


//=============================================================================
//5)function to close a file by its name
int CloseFileByName(char* filename, PDnode Cwd)
{
	//get the file descriptor of the file
	int fd = Namefd(filename, Cwd);
	
	//if file does not exist
	if(fd == ERR_FILE_NOT_EXIST)
	{
		return ERR_FILE_NOT_EXIST;
	}
	
	//decrement the reference count
	UFDT[fd].PtrFileTable->PtrInode->ReferenceCount = UFDT[fd].PtrFileTable->PtrInode->ReferenceCount - 1;
	
	if(UFDT[fd].PtrFileTable->PtrInode->ReferenceCount == 0)
	{
		free(UFDT[fd].PtrFileTable);
		UFDT[fd].PtrFileTable = NULL;
	}
	
	return SUCCESS;
}

//=============================================================================
//6)function to close all files
int CloseAllFiles()
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

	return SUCCESS;
}

//=============================================================================
//7)function to lseek a file - change file offset
int LseekFile(int fd, int offset, int position)
{
	if(UFDT[fd].PtrFileTable == NULL)
	{
		return ERR_FILE_NOT_EXIST;
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
				return ERR_OFFSET;
			}
			//invalid new file offset - negative
			if((UFDT[fd].PtrFileTable->ReadOffset+offset) < 0)
			{
				return ERR_OFFSET;
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
				return ERR_OFFSET;
			}
			//if new file offset becomes negative
			if(offset < 0)
			{
				return ERR_OFFSET;
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
				return ERR_OFFSET;
			}
			//new file offset as negative
			if((UFDT[fd].PtrFileTable->PtrInode->ActualFileSize + offset) < 0)
			{
				return ERR_OFFSET;
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
				return ERR_OFFSET;
			}
			if((UFDT[fd].PtrFileTable->WriteOffset + offset) < 0)
			{
				return ERR_OFFSET;
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
				return ERR_OFFSET;
			}
			if(offset < 0)
			{
				return ERR_OFFSET;
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
				return ERR_OFFSET;
			}
			if((UFDT[fd].PtrFileTable->WriteOffset + offset) < 0)
			{
				return ERR_OFFSET;
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

	return SUCCESS;
}

//=============================================================================
//8)function to truncate a file
int truncate_file(char* filename, PDnode Cwd)
{
	//get file descriptor for the file
	int fd = Namefd(filename, Cwd);

	//file does not exist
	if(fd == -1)
	{
		return ERR_FILE_NOT_EXIST;
	}
	
	//zero out the File_Data for file with received fd
	memset(UFDT[fd].PtrFileTable->PtrInode->FileData,0,1024);
	
	//reset ReadOffset, WriteOffset and ActualFileSize to 0
	UFDT[fd].PtrFileTable->ReadOffset = 0;
	UFDT[fd].PtrFileTable->WriteOffset = 0;
	UFDT[fd].PtrFileTable->PtrInode->ActualFileSize = 0;
	
	return 0;
}

//=============================================================================
//9)function to delete a file
int RemoveFile(char* filename, PDnode Cwd)
{
	int fd = -1;
	int iNo = -1;
	//to hold the head of Dfile LL
	PPDfile Dhead = NULL;
	//temporary pointers
	PDfile curr = NULL;
	PDfile prev = NULL;
	//to hold the inode address of the file to be removed
	PInode node = NULL;

	//get the file descriptor for the file
	fd = Namefd(filename, Cwd);
	//if fd = -1, means the file does not exist
	if(fd == -1)
	{
		return ERR_FILE_NOT_EXIST;
	}
	
	//if fd found, means file exists
	//decrement the link count of this file
	UFDT[fd].PtrFileTable->PtrInode->FileLinkCount = (UFDT[fd].PtrFileTable->PtrInode->FileLinkCount) - 1;

	//if resulting link count is 0,then delete the file 
	if(UFDT[fd].PtrFileTable->PtrInode->FileLinkCount == 0)
	{
		UFDT[fd].PtrFileTable->PtrInode->FileType = 0;
		//free the file table slot
		free(UFDT[fd].PtrFileTable);
		UFDT[fd].PtrFileTable = NULL;
		
		//remove Dfile entry form Dfile linked list
		Dhead = &(Cwd->Dfilell);
		
		//Dfile linked list is empty
		if((*Dhead) == NULL)
		{
			return ERR_EMPTY_DIRECTORY;
		}
		//first entry in Dfile LL is the file to be removed
		else if(strcmp((*Dhead)->Fname, filename) == 0)
		{
			//get the inode number
			iNo = Namei(filename, Cwd);

			//file does not exist
			if(iNo == -1)
			{
				return ERR_FILE_NOT_EXIST;
			}

			//get the inode address of the file to be removed 
			node = GetInoAddr(iNo);
			if(node == NULL)
			{
				return ERR_FILE_NOT_EXIST;
			}

			//make FileType = 0 so that inode is free 
			node->FileType = 0;
			//remove the Dfile node form
			curr = (*Dhead);
			(*Dhead) = (*Dhead)->next;
			free(curr);
			SUPERBLOCK.FreeInodes = SUPERBLOCK.FreeInodes + 1;
			return SUCCESS;
		}
		//if the first file entry is not the file to be removed 
		curr = (*Dhead);

		while(curr != NULL) // more than 1 node 
		{
			if(strcmp(curr->Fname,filename) == 0)
			{				
				//get the inode number
				iNo = Namei(filename, Cwd);

				//file does not exist
				if(iNo == -1)
				{
					return ERR_FILE_NOT_EXIST;
				}

			    //get the inode address of the file to be removed 
				node = GetInoAddr(iNo);
				if(node == NULL)
				{
					return ERR_FILE_NOT_EXIST;
				}
				//make FileType = 0 so that inode is free 
				node->FileType = 0;
				prev->next = curr->next;
				free(curr);
				break;
			}
			prev = curr;
			curr = curr->next;
		}

		//file not found the Dfile LL
		if(curr == NULL)
		{
			return ERR_FILE_NOT_EXIST;
		}

	}

	//increment the count of free inodes in superblock
	SUPERBLOCK.FreeInodes = SUPERBLOCK.FreeInodes + 1;
	return SUCCESS;
}

//=============================================================================
//DIRECTORY RELATED FUNCTIONS DEFINITION
//=============================================================================

//10)function to create a new directory
int MakeDirectory(char* name, PPDnode first, PPDnode root)
{
	//to hold the new Dnode address
    PDnode DNode = NULL; 
	//to hold the new SDnode address
    PSDnode CNode = NULL;
	//to hold the free inode address
	PInode temp = NULL;
	
	temp = GetFreeInode();
	if(temp == NULL)
	{
		return ERR_MAXFILES;
	}


    if((*first) == NULL) // to create first directory in the filesystem    
    {
		//get new Dnode
        DNode = CreateDnode(name,NULL,temp);
        *first = DNode;
		*root = DNode;
		//update the absolute path
		UpdatePath(name, ADD);
    }
    else if((*first) != NULL)	//to create sub directories
	{   
		if(FindDir(name, (*first)->SubDirll) != NULL) // to check whether directory name is already there or not 
		{
			return ERR_UNIQUE_FILE;
		}
		//get new Dnode
        DNode = CreateDnode(name,(*first),temp);
		//get new SDnode
        CNode = CreateSDnode(name,DNode);
		//insert in the sub directory LL
        InsertDir(&((*first)->SubDirll),CNode);
    }
	//assigning filetype to special
	temp->FileType = SPECIAL;

	//decrement free inodes
	SUPERBLOCK.FreeInodes = SUPERBLOCK.FreeInodes - 1;
	
    return SUCCESS;
    
}

//=============================================================================
//11)function to change the current working directory
int ChangeDirectory(char* name, PPDnode Cwd)
{
	//to hold the directory address to be move
    PDnode cddir;
    if((*Cwd) == NULL)
    {
        return 0;
    }
	if(strcmp(name , ".") == 0)
	{
		printf("Cwd : %s\n",(*Cwd)->Dname);
	}
    else if((strcmp(name, "..") == 0) && ((*Cwd)->Parent != NULL))
    {
        (*Cwd) = (*Cwd)->Parent;
		UpdatePath(NULL,REMOVE);
    }
	else
	{
		//find the directory from sub directory LL
		cddir = FindDir(name, (*Cwd)->SubDirll);

		if(cddir == NULL)
		{
			return ERR_NO_DIR;
		}
		//update the Cwd pointer
		(*Cwd) = cddir;
		//update the absolute path of cwd
		UpdatePath(name,ADD);
	}

	return SUCCESS;
    
}

//=============================================================================
//12)function to delete a empty directory
int RemoveDirectory(char * dirname, PDnode Cwd)
{
	PPSDnode sdHead = NULL;
	PSDnode curr = NULL;
	PSDnode prev = NULL;
	PSDnode temp = NULL;
	int iNo = -1;

	//get the sub directory head
	sdHead = &(Cwd->SubDirll);

	//if sub directory LL is empty
	if((*sdHead) == NULL)
	{
		return ERR_NO_DIR;
	}
	//if first node of sub directory is the directory to be removed
	else if(strcmp((*sdHead)->Sname, dirname) == 0) 
	{
		//get the Dnode of sub directory
		PDnode subdir = (*sdHead)->Saddr;

		//check whether directory is empty
		if((subdir->SubDirll == NULL) && (subdir->Dfilell == NULL))
		{
			subdir->Inode->FileType = 0;// freeing inode
			free(subdir);	// free dir's Dnode
			//remove the SDNode of the directory from Parent's subdir LL 
			temp = (*sdHead);
			(*sdHead) = (*sdHead)->next;
			free(temp);	// free SDnode of the directory

			//increment the freeinodes
			SUPERBLOCK.FreeInodes = SUPERBLOCK.FreeInodes + 1;

			return 0;
		}
		//directory is not empty
		else 
		{
			return ERR_NON_EMPTY;
		}
	}
	else// if first directory is not the directory to be removed
	{
		//ge the sub directory head
		curr = (*sdHead);

		//traverse the sub dir LL
		while(curr != NULL)
		{ 
			if(strcmp(curr->Sname, dirname) == 0) // sub dir found
			{
				//get the Dnode address of the directory
				PDnode subdir = curr->Saddr;
				//check whether directory is empty
				if((subdir->SubDirll == NULL) && (subdir->Dfilell == NULL))
				{
					subdir->Inode->FileType = 0;// freeing inode
					free(subdir);	// free sub dir's Dnode
					prev->next = curr->next;
					free(curr);	// free sub dir corresponding LL block

					//increment the freeinodes
					SUPERBLOCK.FreeInodes = SUPERBLOCK.FreeInodes + 1;
				}
				//dir is not empty
				else
				{
					return ERR_NON_EMPTY;
				}
			}

			prev = curr;
			curr = curr->next;
		}

		//directory not found
		if(curr == NULL)
		{
			return ERR_NO_DIR;
		}
    }

	return SUCCESS;
	
}

//=============================================================================
//UTILITY FUNCTIONS DEFINITION
//=============================================================================

//13)function to create a directory node i.e Dnode
PDnode CreateDnode(char* name , PDnode Parent, PInode Inode)
{	
	// to hold the Dnode 
    PDnode temp = NULL;

	//dynamic allocation of memory
    temp = (PDnode)malloc(sizeof(Dnode));
	
	//check if memory allocation failed
    if(temp == NULL)
    {
        printf("Error : Unable to allocate memory\n");
        exit(-1);
    }

	//initialising with attributes 
    memset(temp->Dname, '\0', NSIZE);
	//copy the name
    strcpy(temp->Dname, name);
	//assign the inode
	temp->Inode = Inode;
	//assign the Parent Dnode
    temp->Parent = Parent;
	//assign the sud dir and dfile LL
    temp->SubDirll = NULL;
    temp->Dfilell = NULL;

    return temp;
}

//=============================================================================
//14)function to create a sub directory node for linked list i.e SDnode
PSDnode CreateSDnode(char* name, PDnode addr)
{
	// to hold the SDnode
    PSDnode temp = NULL;

    temp = (PSDnode)malloc(sizeof(struct SDnode));

	//check if memory allocation failed
    if(temp == NULL)
    {
        printf("Error : Unable to allocate memory\n");
        exit(-1);
    }

	//initialising the attributes
    memset(temp->Sname, '\0', NSIZE);
	//copy the name 
    strcpy(temp->Sname, name);
	//assign the address of the corresponding Dnode
    temp->Saddr = addr;
    temp->next = NULL;

    return temp;
}

//=============================================================================
//15)function to create a directory file node for linked list
PDfile CreateDfile(char * name, int iNo)
{
	// to hold the Dfile
    PDfile temp = NULL;
	//dynamic memory allocation
    temp = (PDfile)malloc(sizeof(Dfile));
	//check if memory allocation failed
    if(temp == NULL)
    {
        printf("Error : Unable to allocate memory\n");
        exit(-1);
    }
	//assigning the attributes
    memset(temp->Fname, '\0', NSIZE);
	//copy the file name
    strcpy(temp->Fname, name);
	//copy the inode number
    temp->iNo = iNo;
    temp->next = NULL;

    return temp;
}

//=============================================================================
//16)function to insert sub directory node in linked List
void InsertDir(PPSDnode first, PSDnode child) 
{
	//LL is empty
    if(*first == NULL)  
    {
        *first = child;
        return;
    }

    PSDnode temp = NULL;
    temp = *first;
    *first = child;
    child->next = temp;
}

//=============================================================================
//17)function to insert directory file node in linked List
void InsertFile(PPDfile first, PDfile ptrDfile) 
{
	// LL is empty
	if(*first == NULL)  
    {
        *first = ptrDfile;
        return;
    }

    PDfile temp = NULL;
    temp = *first;
    *first = ptrDfile;
    ptrDfile->next = temp;
}

//=============================================================================
//18)function to find directory in sub directory linked list
PDnode FindDir(char* name, PSDnode SubDirll)
{
	//if there is no sub directory or sub dir LL is empty
    if(SubDirll == NULL)
    {
        return ERR_DIR_NOT_EXIST;
    }
	//traverse the LL to find the directory
    while(( SubDirll != NULL) && (strcmp(SubDirll->Sname, name) != 0))
    {
        SubDirll = SubDirll->next;
    }
    if(SubDirll == NULL)
    {
        return ERR_DIR_NOT_EXIST;
    }
    return (SubDirll->Saddr);
}

//=============================================================================
//19)function to find filename in directory file linked list
bool Findfilename(char* Fname, PDfile Dfilell)
{
    bool flag = false;

	//if Dfile LL is empty or no file is there in the linked list
    if(Dfilell == NULL)
    {
        return flag;
    }
	//traverse the LL to find the file
    while(Dfilell != NULL)
    {
        if(strcmp(Dfilell->Fname, Fname) == 0)	//file found
        {
            flag = true;
            break;
        }
		Dfilell = Dfilell->next;
    }
    return flag;
}

//=============================================================================
//20)function to convert name into inode
int Namei(char* Fname, PDnode Cwd)
{
	// to hold the inode number
	int iNo = -1;
	//to hold the head of the Dfile LL
	PDfile temp = NULL;
	//assigning the head of the Dfile LL
	temp = Cwd->Dfilell;

	//if there is no files
	if(temp == NULL)
	{
		return ERR_FILE_NOT_EXIST;
	}
	//traverse the LL
	while(temp != NULL)
	{
		if(strcmp(Fname, temp->Fname) == 0)	//file found
		{
			iNo = temp->iNo;
			break;
		}
		temp = temp->next;
	}
	if(temp == NULL)//after traversing still file not found
	{
		return ERR_FILE_NOT_EXIST;
	}
	return iNo;
}

//=============================================================================
//21)function to get filedescriptor from name
int Namefd(char* Fname, PDnode Cwd)
{
	int fd = -1, iNo = -1;
	//get the ino of the file
	iNo = Namei(Fname, Cwd);
	//handle the file not found
	if(iNo == -1)
	{
		return ERR_FILE_NOT_EXIST;
	}
	//traverse the UFDT array to get the corresponding fd to ino
	for(fd=0;fd<MAXFILES;fd++)
	{
		if(UFDT[fd].PtrFileTable != NULL)
		{
			if(UFDT[fd].PtrFileTable->PtrInode->InodeNumber == iNo)
			{
				break;
			}
		}		
	}
	//after traversing reaching to the end still did not found the corresponding fd
	if(fd == MAXFILES)
	{
		return ERR_FILE_NOT_EXIST;
	}

	return fd;
}

//=============================================================================
//22)function to get inode address from file name 
PInode GetInode(char* filename, PDnode Cwd)
{
	int iNo = -1;
	PDfile dfileptr = NULL;

	//create a temp pointer for navigation through the DILB
	struct Inode* temp = Head;
	
	//if filename is NULL - do nothing, return
	if(filename == NULL)
	{
		return ERR_NO_FILE;
	}

	//assign the head of the Dfile LL
	dfileptr = Cwd->Dfilell;

	if(dfileptr == NULL)
	{
		return ERR_NO_FILE;
	}

	// get the inode of the filename
	iNo = Namei(filename, Cwd);

	if(iNo == -1)
	{
		return ERR_NO_FILE;
	}
	
	//navigate through the DILB till you reach allocated inode 
	temp = GetInoAddr(iNo);

	if(temp == NULL)
	{
		return ERR_NO_FILE;
	}
	return temp;
}

//=============================================================================
//23)function to display sub directories
void DisplaySubDir(PDnode Cwd)
{
	//assign head of the sub dir LL
    PSDnode subdir = Cwd->SubDirll;
	//sub dir is empty
	if(subdir == NULL)
	{
		return;
	}
	printf("Directories : \n");
	//traverse the LL and print
    while(subdir != NULL)
    {
        printf(" %s ",subdir->Sname);
        subdir = subdir->next;
    }
    printf("\n");
}

//=============================================================================
//24)function to display the files in current working directory 
void DisplayFiles(PDnode Cwd)
{
	// assign the head of the Dfile LL
    PDfile Dfilell = Cwd->Dfilell;
	//no file i.e Dfile LL is empty
	if(Dfilell == NULL)
	{
		return;
	}
	printf("Files : \n");

	//traverse the LL and print
    while(Dfilell != NULL)
    {
        printf(" %s ",Dfilell->Fname);
        Dfilell = Dfilell->next;
    }
    printf("\n");
}

//=============================================================================
//25)function to get free inode 
PInode GetFreeInode()
{
	//pointer to hold the free Inode
    PInode temp = NULL;
	//assigning the head of DILB
    temp = Head;

    while(temp != NULL)
	{
		if(temp->FileType == 0)
		{
			break;
		}
		temp = temp->next;
	}

    return temp;
}

//=============================================================================
//26)function to get inode address from inode number
PInode GetInoAddr(int iNo)
{
	//to hold the inode
	PInode temp = NULL;
	//assigning the head of DILB
	temp = Head;

	while(temp != NULL)
	{
		if(temp->InodeNumber == iNo)
		{
			break;
		}
		temp = temp->next;
	}

	return temp;
}

//=============================================================================
//27)function to handle the absolute path
void UpdatePath(const char *dirname, bool option)
{
    if (option)
    {
        // ADD directory
        if (path[0] != '\0')	//not the first directory to add
            strcat(path, "/");

        strcat(path, dirname);
    }
    else
    {
        // REMOVE last directory
        char *lastSlash = strrchr(path, '/');

        if (lastSlash)
        {
            *lastSlash = '\0';
        }
        else
        {
            // only one directory left
            return;
            
        }
    }
}

//=============================================================================
//MISCELLANEOUS FUNCTIONS DEFINITION
//=============================================================================

//28)function to initialise the super block - initialising the members of UFDT to NULL, initialising the two members of SUPERBLOCK
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

//=============================================================================
//29)function to create DILB
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

//=============================================================================
//30)function to display list of commands
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
	printf("mkdir: To create directory\n");
	printf("rmdir: to remove and empty directory\n");
	printf("cd: To change the current working directory\n");
	printf("pwd: to print the absolute path of current working directory\n");
	printf("ls -s: List the information of superblock\n");

}

//=============================================================================
//31)function to display man page of a command
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
	else if(strcmp(command,"ls -s") == 0)
	{
		printf("DESCRIPTION: This command is used to list the files and their properties\n");
		printf("USAGE: ls -s\n");
	}
	else if(strcmp(command,"mkdir") == 0)
	{
		printf("DESCRIPTION: This command is used to create a directory\n");
		printf("USAGE: mkdir dirname\n");
	}
	else if(strcmp(command,"rmdir") == 0)
	{
		printf("DESCRIPTION: This command is used to delete an empty directory\n");
		printf("USAGE: rmdir dirname\n");
	}
	else if(strcmp(command,"pwd") == 0)
	{
		printf("DESCRIPTION: This command is used to display the absolute path of current working directory\n");
		printf("USAGE: pwd\n");
	}
	else if(strcmp(command,"cd") == 0)
	{
		printf("DESCRIPTION: This command is used to change the current working directory\n");
		printf("USAGE: cd dirname\n");
	}
	else
	{
		printf("No man documentation found\n");
	}
}

//=============================================================================
//32)function to list files and directories 
void lsDirFile(PDnode Cwd)
{
	//create a temp pointer to navigate through the DILB

	//check if there are no files in filesystem - means all inodes are free
	if(SUPERBLOCK.FreeInodes == MAXINODES)
	{
		printf("Error: There are no files\n");
		return;
	}

	DisplaySubDir(Cwd);	 //to display sub directories
	DisplayFiles(Cwd);	//to display files
}

//=============================================================================
//33)function to display file statistics from file name
int file_stat(char* filename, PDnode Cwd)
{
	if(filename == NULL)
	{
		return ERR_OTHER;
	}
		
	//temp pointer to hold the inode address
	struct Inode* temp = NULL;
	//get the inode address
	temp = GetInode(filename, Cwd);
	
	//handle file not found
	if(temp == NULL)
	{
		return ERR_FILE_NOT_EXIST;
	}
	
	//display the file attributes
	printf("--------------------------------------\n");
	//printf("FileName: %s\n",temp->FileName);
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

//=============================================================================
//34)function to display file statistics from file descriptor
int file_fstat(int fd)
{
	//printf("fd = %d\n",fd);
	//incorrect fd
	if((fd < 0)||(fd >= MAXFILES))
	{
		return ERR_INCORRECT_FD;
	}
	
	//file with this fd does not exist
	else if(UFDT[fd].PtrFileTable == NULL)
	{
		return ERR_FILE_FD_NOT_FOUND;
	}
	
	//display the file attributes
	printf("--------------------------------------\n");
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

//=============================================================================
//35)function to get the read and write offset for testing
void get_offset(char* filename)
{
	int fd = Namefd(filename, Cwd);
	if(fd == -1)
	{
		printf("file not found\n");
		return;
	}
	printf("ReadOffset: %d\n",UFDT[fd].PtrFileTable->ReadOffset);
	printf("WriteOffset: %d\n",UFDT[fd].PtrFileTable->WriteOffset);
	
}

//=============================================================================
//36)function to display absolute path of current working directory
void DisplayPath()
{
	printf("%s\n",path);
}

//=============================================================================
//37)function to display inormation of superblock for testing
void SBDisplay()
{
	printf("Free inodes  : %d\n",SUPERBLOCK.FreeInodes);
}

//=============================================================================
//38)function to store the file structure on secondary storage
int OSWalk(Dnode * dirptr)
{
	int iRet = -1, fd = -1;
	//to hold the current directory dnodell
	PSDnode dtemp = NULL;
	//to hold the current directory Dfilell
	PDfile ftemp = NULL;
	//to hold the inode
	PInode itemp = NULL;

	if(root == NULL)
	{
		return ERR_NO_DIR;
	}

	if(dirptr == NULL)
	{
		return 0;
	}

	//to create a directory
	iRet = _mkdir(dirptr->Dname);

	//handling the mkdir() syscall
    if(iRet != 0)
    {
        return ERR_MKDIR;
    }
	//to change the current working directory
	_chdir(dirptr->Dname);

	//assign the head of the Dfilell
	ftemp = dirptr->Dfilell;
	
	//first copy all the files
	while(ftemp != NULL)
	{

		fd = _creat(ftemp->Fname, _S_IREAD | _S_IWRITE);
		if (fd == -1)
		{
			return ERR_CREAT;
		}

		itemp = GetInoAddr(ftemp->iNo);

		_write(fd, itemp->FileData, itemp->ActualFileSize);
		_close(fd);

		ftemp = ftemp->next;
	}

	//assign the head of sdnodell
	dtemp = dirptr->SubDirll;

	//directory is empty so go back to Parent directory
	if(dtemp == NULL)
	{
		_chdir("..");
		return 0;
	}

	//to create sub directories
	while(dtemp != NULL)
	{
		OSWalk(dtemp->Saddr);

		dtemp = dtemp->next;
	}
	_chdir("..");

}

