/*
Entry point function - program execution begins here
*/

//include the header file created for this project
#include "VirtualFileSystem.h"

//create objects of the data structures
struct SuperBlock SUPERBLOCK;
struct Inode INODE;
struct FileTable FILETABLE;
struct ufdt UFDT[MAXFILES];

//pointer to hold first inode
struct Inode* Head = NULL;

//pointer to hold the current working directory
PDnode Cwd = NULL;

//pointer to hold permanently the first directory
PDnode root = NULL;

// to hold path
char path[MAXPATH] = {'\0'};

int main(void)
{
	//variable to hold the command line entered at the command prompt
	char str[80] = "\0";
	//variable to hold the command line arguments - 4 command line arguments - each with max size limit 50 characters
	char cmd[4][50];
	//variable to hold the number of command line arguments
	int count = 0;
	//variable for holding file descriptor
	int fd = 0;
	//variable to hold return value of a system call
	int ret = 0;
	//buffer
	char buffer[BSIZE];
	//character pointer
	char* ptr = NULL;
	//to hold te multi line data
	char line[256];
	//to hold the exit question value
	char ch = '\0';

	
	//printf the banner
	printf("=====================================================\n");
	printf("CUSTOMISED VIRTUAL FILE SYSTEM\n");
	printf("=====================================================\n");
	
	//call the function to create inodes - DILB
	createDILB();
	//call the function to initialise the super block
	InitialiseSuperBlock();	
	
	//infinite loop - for shell
	while(1)
	{    	
		//print the shell prompt
		printf("CVFS> ");
		//read the command line - string with spaces
		scanf("%[^\n]s",str);
		//flush the enter key pressed after entering the command line (above step)
		getchar();
		
		//split the command line - sscanf reads a string in formatted way
		count = sscanf(str,"%s%s%s%s",cmd[0],cmd[1],cmd[2],cmd[3]);
		
		//check for different command line argument counts
		if(count == 1)
		{
			if(strcmp(cmd[0],"help") == 0)
			{
				DisplayHelp();
				continue;
			}
			else if(strcmp(cmd[0],"cls") == 0)
			{
				system("cls");
				continue;
			}
			else if(strcmp(cmd[0],"ls") == 0)
			{
				lsDirFile(Cwd);
				continue;
			}
			else if(strcmp(cmd[0],"exit") == 0)
			{
				printf("Do you want to save all files and directories? [Y/N]: ");
    
				ch = getchar();

				if (ch == 'y' || ch == 'Y')
				{
					ret = OSWalk(root);
					if(ret == ERR_MKDIR)
					{
						printf("Error: unable to make directory\n");
					}
					else if(ret == ERR_CREAT)
					{
						printf("Error: unable to make file\n");
					}
					else if(ret == ERR_NO_DIR)
					{
						printf("Error: there should be atleast one directory\n");
					}
				}
				printf("Thank you for using the application\n");
				break;
			}
			else if(strcmp(cmd[0],"closeall") == 0)
			{
				ret = CloseAllFiles();
				if(ret == SUCCESS)
				{
					printf("All files closed successfully\n");
				}
				continue;
			}
			else if(strcmp(cmd[0],"pwd") == 0)
			{
				//to display the absolute path of present working directory
				DisplayPath();
				continue;
			}
			else if(strcmp(cmd[0],"save") == 0)
			{
				ret = OSWalk(root);
				if(ret == ERR_MKDIR)
				{
					printf("Error: unable to make directory\n");
				}
				else if(ret == ERR_CREAT)
				{
					printf("Error: unable to make file\n");
				}
				else if(ret == ERR_NO_DIR)
				{
					printf("Error: there should be atleast one directory\n");
				}
				continue;
			}
			else
			{
				printf("Command not found\n");
				continue;
			}
		}
		else if(count == 2)
		{
			if(strcmp(cmd[0],"man") == 0)
			{
				man(cmd[1]);
				continue;
			}
			else if(strcmp(cmd[0],"rm") == 0)
			{
				ret = RemoveFile(cmd[1], Cwd);
				if(ret == ERR_FILE_NOT_EXIST)
				{
					printf("ERROR: File does not exist\n");
				}
				else if(ret == ERR_EMPTY_DIRECTORY)
				{
					printf("ERROR: Directory is empty\n");
				}
				else if(ret == SUCCESS)
				{
					printf("File deleted successfully\n");
				}
				continue;
			}
			else if(strcmp(cmd[0],"stat") == 0)
			{
				ret = file_stat(cmd[1], Cwd);
				if(ret == ERR_OTHER)
				{
					printf("Error: Incorrect parameters\n");
				}
				else if(ret == ERR_FILE_NOT_EXIST)
				{
					printf("ERROR: File does not exist\n");
				}
				continue;
			}
			else if(strcmp(cmd[0],"fstat") == 0)
			{
				ret = file_fstat(atoi(cmd[1]));
				if(ret == ERR_INCORRECT_FD)
				{
					printf("Incorrect file descriptor\n");
				}
				else if(ret == ERR_FILE_FD_NOT_FOUND)
				{
					printf("File with this fd does not exist\n");
				}
				continue;
			}
			else if(strcmp(cmd[0],"close") == 0)
			{
				ret = CloseFileByName(cmd[1], Cwd);
				if(ret == ERR_FILE_NOT_EXIST)
				{
					printf("Error: File does not exist\n");
				}
				else if(ret == SUCCESS)
				{
					printf("File closed sucessfully\n");
				}
				continue;
			}
			else if(strcmp(cmd[0],"write") == 0)
			{
				//get fd for the file name
				fd = Namefd(cmd[1], Cwd);
				if(fd == ERR_FILE_NOT_EXIST)
				{
					printf("ERROR: File does not exist\n");
					continue;
				}
				memset(buffer,'\0',BSIZE);
				printf("Enter the data: \n");
				while (fgets(line, sizeof(line), stdin) != NULL)
				{
					strcat(buffer, line);
				}
				//press ctrl + D to come out of the loop or submit the data
				//scanf("%[^\n]",buffer);
				//getchar();
				//get the count of data entered
				ret = strlen(buffer);
				printf("size : %d \n",ret);
				if(ret == 0)
				{
					printf("ERROR: Incorrect data entry\n");
					continue;
				}
				//call write function
				ret = Write_File(fd,buffer,ret);
				if(ret == ERR_PERMISSION)
				{
					printf("ERROR: Permission denied\n");
				}
				else if(ret == ERR_INSUFFICIENT_MEMORY)
				{
					printf("ERROR: Insufficient memory to write\n");
				}
				else if(ret == ERR_NO_REGULAR)
				{
					printf("ERROR: It is not a regular file\n");
				}
				else
				{
					printf("Data written successfully\n");
				}
				continue;
			}
			else if(strcmp(cmd[0],"truncate") == 0)
			{
				ret = truncate_file(cmd[1], Cwd);
				if(ret == ERR_FILE_NOT_EXIST)
				{
					printf("ERROR: File does not exist\n");
				}
				continue;
			}
			else if(strcmp(cmd[0],"offset") == 0)
			{
				//used for testing purpose to get the read and write offset
				get_offset(cmd[1]);
				continue;
			}
			else if(strcmp(cmd[0], "mkdir") == 0)
			{
				ret = MakeDirectory(cmd[1], &Cwd, &root);
				if(ret == ERR_MAXFILES)
				{
					printf("ERROR: No free Inodes\n");
				}
				else if(ret == ERR_UNIQUE_FILE)
				{
					printf("ERROR: Directory already present\n");
				}
				else if(ret == SUCCESS)
				{
					printf("Directory created successfully\n");
				}
				continue;
			}
			else if(strcmp(cmd[0], "cd") == 0)
			{
				// to change the current directory
				ret = ChangeDirectory(cmd[1], &Cwd);
				if(ret == ERR_NO_DIR)
				{
					printf("ERROR: No such Directory\n");
				}

				continue;
			}
			else if(strcmp(cmd[0], "rmdir") == 0)
			{
				ret = RemoveDirectory(cmd[1], Cwd);
				if(ret == ERR_NO_DIR)
				{
					printf("ERROR: No such Directory\n");
				}
				else if(ret == ERR_NON_EMPTY)
				{
					printf("ERROR: Directory is not empty\n");
				}
				else if(ret == SUCCESS)
				{
					printf("Directory deleted successfully\n");
				}
				continue;
			}
			else if((strcmp(cmd[0], "ls") == 0) && (strcmp(cmd[1], "-s") == 0))
			{
				//used for testing purpose to display the free inodes
				SBDisplay();
				continue;
			}
			else if(strcmp(cmd[0],"create") == 0)// it is executed when user dont give permission
			{
				//if user dont give permission then default permission is 3
				fd = CreateFile(cmd[1], DPERMISSION , Cwd);
				if(fd == ERR_NO_DIR)
				{
					printf("Error: There should be atleast 1 Directory\n");
				}
				else if(fd == ERR_PERMISSION)
				{
					printf("Error: Permission denied\n");
				}
				else if(fd == ERR_MAXFILES)
				{
					printf("Error: No free Inodes\n");
				}
				else if(fd == ERR_UNIQUE_FILE)
				{
					printf("Error: Filename already exist\n");
				}
				else if(fd >= 0)
				{
					printf("File successfully created with file descriptor: %d\n",fd);
				}
				continue;
			}
			else if(strcmp(cmd[0],"read") == 0)// it is executed when user dont mention the no of bytes to read
			{
				//get the file descriptor
				fd = Namefd(cmd[1], Cwd);
				ret = Namei(cmd[1], Cwd);//convert name to ino
				ret = (GetInoAddr(ret))->ActualFileSize;//get the ino address to get the actual file size
				if(fd == ERR_FILE_NOT_EXIST)
				{
					printf("Error: File does not exist\n");
					continue;
				}
				//create a buffer to read data into
				int read_count = ret;
				ptr = (char*) malloc(sizeof(char)*(read_count+1));
				memset(ptr, '\0', read_count+1);
				//check for memory allocation failure
				if(ptr == NULL)
				{
					printf("ERROR: Memory allocation failure\n");
					continue;
				}
				ret = Read_File(fd,ptr,ret);
				if(ret == ERR_FILE_NOT_EXIST)
				{
					printf("ERROR: File does not exist\n");
				}
				else if(ret == ERR_NO_PERMISSION)
				{
					printf("ERROR: Permission denied\n");
				}
				else if(ret == ERR_OFFSET)
				{
					printf("ERROR: Reached end of file\n");
				}
				else if(ret == ERR_NO_REGULAR)
				{
					printf("ERROR: Not a regular file\n");
				}
				else if(ret == ERR_EMPTY_FILE)
				{
					printf("ERROR: File is empty\n");
				}
				else if(ret > 0)
				{
					printf(" Data : %s \n",ptr);
				}
				continue;
			}
			else
			{
				printf("Incorrect command\n");
				continue;
			}
		}
		else if(count == 3)
		{
			if(strcmp(cmd[0],"create") == 0)
			{
				fd = CreateFile(cmd[1],atoi(cmd[2]), Cwd);
				if(fd == ERR_NO_DIR)
				{
					printf("Error: There should be atleast 1 Directory\n");
				}
				else if(fd == ERR_PERMISSION)
				{
					printf("Error: Permission denied\n");
				}
				else if(fd == ERR_MAXFILES)
				{
					printf("Error: No free Inodes\n");
				}
				else if(fd == ERR_UNIQUE_FILE)
				{
					printf("Error: Filename already exist\n");
				}
				else if(fd >= 0)
				{
					printf("File successfully created with file descriptor: %d\n",fd);
				}
				continue;
			}
			if(strcmp(cmd[0],"open") == 0)
			{
				fd = OpenFile(cmd[1],atoi(cmd[2]), Cwd);
				if(fd >= 0)
				{
					printf("File opened successfully with file descriptor: %d\n",fd);
				}
				else if(fd == ERR_FILE_MODE)
				{
					printf("ERROR: Incorrect parameters\n");
				}
				else if(fd == ERR_FILE_NOT_EXIST)
				{
					printf("ERROR: File does not exist\n");
				}
				else if(fd == ERR_NO_PERMISSION)
				{
					printf("ERROR: Permission denied\n");
				}
				continue;
			}
			else if(strcmp(cmd[0],"read") == 0)
			{
				//get the file descriptor
				fd = Namefd(cmd[1], Cwd);
				if(fd == ERR_FILE_NOT_EXIST)
				{
					printf("Error: File does not exist\n");
					continue;
				}
				//create a buffer to read data into
				int read_count = atoi(cmd[2]);
				ptr = (char*) malloc(sizeof(char)*(read_count+1));
				memset(ptr, '\0', read_count+1);
				//check for memory allocation failure
				if(ptr == NULL)
				{
					printf("ERROR: Memory allocation failure\n");
					continue;
				}
				ret = Read_File(fd,ptr,atoi(cmd[2]));
				if(ret == ERR_FILE_NOT_EXIST)
				{
					printf("ERROR: File does not exist\n");
				}
				else if(ret == ERR_NO_PERMISSION)
				{
					printf("ERROR: Permission denied\n");
				}
				else if(ret == ERR_OFFSET)
				{
					printf("ERROR: Reached end of file\n");
				}
				else if(ret == ERR_NO_REGULAR)
				{
					printf("ERROR: Not a regular file\n");
				}
				else if(ret == ERR_EMPTY_FILE)
				{
					printf("ERROR: File is empty\n");
				}
				else if(ret > 0)
				{
					printf(" Data : %s \n",ptr);
				}
				continue;
			}
			else
			{
				printf("Incorrect command\n");
				continue;
			}
		}
		else if(count == 4)
		{
			if(strcmp(cmd[0],"lseek") == 0)
			{
				//get the fd from the file name
				fd = Namefd(cmd[1], Cwd);
				if(fd == ERR_FILE_NOT_EXIST)
				{
					printf("ERROR: File does not exist\n");
					continue;
				}
				ret = LseekFile(fd, atoi(cmd[2]), atoi(cmd[3]));
				if(fd == ERR_FILE_NOT_EXIST)
				{
					printf("ERROR: File does not exist\n");
					continue;
				}
				else if(ret == ERR_OFFSET)
				{
					printf("ERROR: Unable to perform lseek\n");
				}
			}
			else
			{
				printf("ERROR: Command not found\n");
			}
		}
		else
		{
			printf("ERROR: Bad command\n");
		}
		
	}
	
	exit(0);
}