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
	char buffer[1024];
	//character pointer
	char* ptr = NULL;
	
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
		printf("VFS> ");
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
				ls_file();
				continue;
			}
			else if(strcmp(cmd[0],"exit") == 0)
			{
				printf("Thank you for using the application\n");
				break;
			}
			else if(strcmp(cmd[0],"closeall") == 0)
			{
				CloseAllFiles();
				printf("All files closed successfully\n");
				continue;
			}
			else
			{
				printf("Command not found\n");
				// printf("in cnt 1 \n");
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
				ret = RemoveFile(cmd[1]);
				if(ret == -1)
				{
					printf("ERROR: File does not exist\n");
				}
				else if(ret == 0)
				{
					printf("File deleted successfully\n");
				}
				continue;
			}
			else if(strcmp(cmd[0],"stat") == 0)
			{
				ret = file_stat(cmd[1]);
				if(ret == -1)
				{
					printf("Incorrect parameters\n");
				}
				else if(ret == -2)
				{
					printf("File does not exist\n");
				}
				continue;
			}
			else if(strcmp(cmd[0],"fstat") == 0)
			{
				// printf("cmd[1] = %d\n",atoi(cmd[1]));
				ret = file_fstat(atoi(cmd[1]));
				if(ret == -1)
				{
					printf("Incorrect file descriptor\n");
				}
				else if(ret == -2)
				{
					printf("File with this fd does not exist\n");
				}
				continue;
			}
			else if(strcmp(cmd[0],"close") == 0)
			{
				ret = CloseFileByName(cmd[1]);
				if(ret == -1)
				{
					printf("File with this name does not exist\n");
				}
				else if(ret == 0)
				{
					printf("File closed sucessfully\n");
				}
				continue;
			}
			else if(strcmp(cmd[0],"write") == 0)
			{
				//get fd for the file name
				fd = GetFDFromName(cmd[1]);
				if(fd == -1)
				{
					printf("ERROR: File does not exist\n");
					continue;
				}
				printf("Enter the data: \n");
				scanf("%[^\n]",buffer);
				getchar();
				//get the count of data entered
				ret = strlen(buffer);
				if(ret == 0)
				{
					printf("ERROR: Incorrect data entry\n");
					continue;
				}
				//call write function
				ret = Write_File(fd,buffer,ret);
				if(ret == -1)
				{
					printf("ERROR: Permission denied\n");
				}
				else if(ret == -2)
				{
					printf("ERROR: Insufficient memory to write\n");
				}
				else if(ret == -3)
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
				ret = truncate_file(cmd[1]);
				if(ret == -1)
				{
					printf("ERROR: Incorrect parameters\n");
				}
			}
			else if(strcmp(cmd[0],"offset") == 0)
			{
				get_offset(cmd[1]);
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
				fd = CreateFile(cmd[1],atoi(cmd[2]));
				if(fd == -1)
				{
					printf("Incorrect create file parameters\n");
				}
				else if(fd == -2)
				{
					printf("No free inodes available\n");
				}
				else if(fd == -3)
				{
					printf("File already exists\n");
				}
				else if(fd == -4)
				{
					printf("Memory allocation failed\n");
				}
				else if(fd >= 0)
				{
					printf("File successfully created with file descriptor: %d\n",fd);
				}
				continue;
			}
			if(strcmp(cmd[0],"open") == 0)
			{
				fd = OpenFile(cmd[1],atoi(cmd[2]));
				if(fd >= 0)
				{
					printf("File opened successfully with file descriptor: %d\n",fd);
				}
				else if(fd == -1)
				{
					printf("ERROR: Incorrect parameters\n");
				}
				else if(fd == -2)
				{
					printf("ERROR: File does not exist\n");
				}
				else if(fd == -3)
				{
					printf("ERROR: Permission denied\n");
				}
				continue;
			}
			else if(strcmp(cmd[0],"read") == 0)
			{
				//get the file descriptor
				fd = GetFDFromName(cmd[1]);
				if(fd == -1)
				{
					printf("ERROR: Incorrect parameter\n");
					continue;
				}
				//create a buffer to read data into
				int read_count = atoi(cmd[2]);
				ptr = (char*) malloc(sizeof(read_count+1));
				memset(ptr, 0, sizeof(read_count+1));
				//check for memory allocation failure
				if(ptr == NULL)
				{
					printf("ERROR: Memory allocation failure\n");
					continue;
				}
				ret = Read_File(fd,ptr,atoi(cmd[2]));
				if(ret == -1)
				{
					printf("ERROR: File does not exist\n");
				}
				else if(ret == -2)
				{
					printf("ERROR: Permission denied\n");
				}
				else if(ret == -3)
				{
					printf("ERROR: Reached end of file\n");
				}
				else if(ret == -4)
				{
					printf("ERROR: Not a regular file\n");
				}
				else if(ret == 0)
				{
					printf("ERROR: File is empty\n");
				}
				else if(ret > 0)
				{
					printf("%s \n",ptr);
				}
				continue;
			}
			continue;
		}
		else if(count == 4)
		{
			if(strcmp(cmd[0],"lseek") == 0)
			{
				//get the fd from the file name
				fd = GetFDFromName(cmd[1]);
				if(fd == -1)
				{
					printf("ERROR: Incorrect parameters\n");
					continue;
				}
				ret = LseekFile(fd, atoi(cmd[2]), atoi(cmd[3]));
				if(ret == -1)
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