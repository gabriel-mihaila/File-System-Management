#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>

#define MAX_SIZE 512

int list_nonrecursive(const char *path, char *name_ends_with, char *size_greater)
{
	DIR *dir = NULL;
    struct dirent *entry = NULL;
    char fullPath[MAX_SIZE];
    struct stat statbuf;

    dir = opendir(path);
    if(dir == NULL)
    {
    	return -1;
    }

    printf("SUCCESS\n");
    while((entry = readdir(dir)) != NULL)
    {
    	snprintf(fullPath, MAX_SIZE, "%s/%s", path, entry->d_name);
    	if(lstat(fullPath, &statbuf) == 0)
    	{
    		if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
    		{
    			if(strlen(name_ends_with) == 0 && strlen(size_greater) == 0)
	    		{
	    			printf("%s\n", fullPath);
	    		}
	    		else
	    		{
	    			if(strlen(name_ends_with) != 0)
	    			{
	    				char *p = NULL, *q = NULL;

	    				p = name_ends_with + strlen("name_ends_with=");
	    				if((q = strstr(fullPath, p)) != NULL && strlen(q + strlen(p)) == 0)
	    				{
	    					printf("%s\n", fullPath);
	    				}

	    			}
	    			else
	    			{
	    				size_t file_size;
	    				char *p = NULL;

	    				p = size_greater + strlen("size_greater=");
	    				sscanf(p, "%ld", &file_size);

	    				if(S_ISREG(statbuf.st_mode) && statbuf.st_size > file_size)
	    				{
	    					printf("%s\n", fullPath);
	    				}

	    			}
	    		}
    		}
	    		
    	}
    }

    closedir(dir);

    return 0;
}

int list_recursive(const char *path, char *name_ends_with, char *size_greater, bool *first_trace)
{
	DIR *dir = NULL;
    struct dirent *entry = NULL;
    char fullPath[MAX_SIZE];
    struct stat statbuf;

    dir = opendir(path);
    if(dir == NULL)
    {
    	return -1;
    }

    if(*first_trace)
    {
    	printf("SUCCESS\n");
    	*first_trace = false;
    }


    while((entry = readdir(dir)) != NULL)
    {
    	if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
    	{
    		snprintf(fullPath, MAX_SIZE, "%s/%s", path, entry->d_name);
    		if(lstat(fullPath, &statbuf) == 0)
    		{
    			if(strlen(name_ends_with) == 0 && strlen(size_greater) == 0)
    			{
    				printf("%s\n", fullPath);
    			}
    			else
	    		{
	    			if(strlen(name_ends_with) != 0)
	    			{
	    				char *p = NULL, *q = NULL;

	    				p = name_ends_with + strlen("name_ends_with=");
	    				if((q = strstr(fullPath, p)) != NULL && strlen(q + strlen(p)) == 0)
	    				{
	    					printf("%s\n", fullPath);
	    				}

	    			}
	    			else
	    			{
	    				size_t file_size;
	    				char *p = NULL;

	    				p = size_greater + strlen("size_greater=");
	    				if(sscanf(p, "%ld", &file_size) != 1)
	    				{
	    					return -1;
	    				}

	    				if(S_ISREG(statbuf.st_mode) && statbuf.st_size > file_size)
	    				{
	    					printf("%s\n", fullPath);
	    				}

	    			}
	    		}

	    		if(S_ISDIR(statbuf.st_mode))
	    		{
	    			list_recursive(fullPath, name_ends_with, size_greater, first_trace);
	    		}
    		}
    	}
    }

    closedir(dir);

    return 0;
}

//functia de reverse a unui sir

char *strrev(char *str)
{
      char *p1, *p2;

      if (! str || ! *str)
            return str;
      for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
      {
            *p1 ^= *p2;
            *p2 ^= *p1;
            *p1 ^= *p2;
      }
      return str;
}

int parse_SF(const char *path_file, bool findall_detector)
{
	int fd = -1, num_header_size, num_version, num_nr_sections;
	char magic[3], header_size[3], version[3], nr_sections[2], hex[10];

	fd = open(path_file, O_RDONLY);
	if(fd == -1)
	{
		return -1;
	}

	lseek(fd, -2, SEEK_END);
	read(fd, magic, 2);
	magic[2] = '\0';

	if(strcmp(magic, "gN") != 0)
	{
		return -2;
	}
	
	lseek(fd, -4, SEEK_END);
	read(fd, header_size, 2);
	header_size[2] = '\0';
	strcpy(header_size, strrev(header_size));

	//conversia unui string in hexa
	for(int i = 0, j = 0; i < strlen(header_size); i++, j+= 2)
        sprintf(hex + j, "%02x", header_size[i] & 0xff);

    //conversia din hexa in decimal (little endian)
    num_header_size = (int)strtol(hex, NULL, 16);

    lseek(fd, -num_header_size, SEEK_END);
	read(fd, version, 2);
	version[2] = '\0';
    strcpy(version, strrev(version));

    for(int i = 0, j = 0; i < strlen(version); i++, j+= 2)
        sprintf(hex + j, "%02x", version[i] & 0xff);

    num_version = (int)strtol(hex, NULL, 16);
    
    if(num_version < 127 || num_version > 215)
    {
    	return -3;
    }

    lseek(fd, -num_header_size + 2, SEEK_END);
    read(fd, nr_sections, 1);
    nr_sections[1] = '\0';
    sprintf(hex, "%02x", nr_sections[0] & 0xff);

	num_nr_sections = (int)strtol(hex, NULL, 16);

	if(num_nr_sections < 5 || num_nr_sections > 17)
	{
		return -4;
	}

	char sect_type[3], sect_name[21], sect_size[5];
	int num_sect_type, num_sect_size;

	for(int i = 0; i < num_nr_sections; i++)
	{
		lseek(fd, -num_header_size + 3 + 20 + i * 30, SEEK_END);
		read(fd, sect_type, 2);
		sect_type[2] = '\0';
		strcpy(sect_type, strrev(sect_type));

		for(int j = 0, k = 0; j < strlen(sect_type); j++, k+= 2)
        	sprintf(hex + k, "%02x", sect_type[j] & 0xff);

    	num_sect_type = (int)strtol(hex, NULL, 16);

    	if(num_sect_type != 44 && num_sect_type != 74 && num_sect_type != 24)
    	{
    		return -5;
    	}

	}

	if(findall_detector == false)
	{
		printf("SUCCESS\n");
		printf("version=%d\n", num_version);
		printf("nr_sections=%d\n", num_nr_sections);

		int cursor;

		for(int i = 0; i < num_nr_sections; i++)
		{
			printf("section%d: ", i + 1);

			cursor = -num_header_size + 3 + i * 30;

			lseek(fd, cursor, SEEK_END);
			read(fd, sect_name, 20);
			sect_name[20] = '\0';
			printf("%s ", sect_name);

			cursor += 20;

			lseek(fd, cursor, SEEK_END);
			read(fd, sect_type, 2);
			sect_type[2] = '\0';
			strcpy(sect_type, strrev(sect_type));

			for(int j = 0, k = 0; j < strlen(sect_type); j++, k+= 2)
	        	sprintf(hex + k, "%02x", sect_type[j] & 0xff);

	    	num_sect_type = (int)strtol(hex, NULL, 16);

	    	printf("%d ", num_sect_type);

	    	cursor += 6;

	    	lseek(fd, cursor, SEEK_END);
	    	read(fd, sect_size, 4);
	    	sect_size[4] = '\0';
	    	strcpy(sect_size, strrev(sect_size));

	    	for(int j = 0, k = 0; j < strlen(sect_size); j++, k+= 2)
	        	sprintf(hex + k, "%02x", sect_size[j] & 0xff);

	        num_sect_size = (int)strtol(hex, NULL, 16);

	        printf("%d\n", num_sect_size);

		}
	}


	return 0;

}

int extract_line(const char *path_file, int section_extract, int nrline_extract)
{
	int fd = -1, num_header_size, num_nr_sections, num_sect_size, num_sect_offset, actual_line = 0, cursor;
	char sect_size[5], sect_offset[5], header_size[3], nr_sections[2], hex[10];

	fd = open(path_file, O_RDONLY);
	if(fd == -1)
	{
		return -1;
	}

	lseek(fd, -4, SEEK_END);
	read(fd, header_size, 2);
	header_size[2] = '\0';
	strcpy(header_size, strrev(header_size));

	for(int i = 0, j = 0; i < strlen(header_size); i++, j+= 2)
        sprintf(hex + j, "%02x", header_size[i] & 0xff);

    num_header_size = (int)strtol(hex, NULL, 16);

    lseek(fd, -num_header_size + 2, SEEK_END);
    read(fd, nr_sections, 1);
    nr_sections[1] = '\0';
    sprintf(hex, "%02x", nr_sections[0] & 0xff);

	num_nr_sections = (int)strtol(hex, NULL, 16);

	if(section_extract > num_nr_sections)
	{
		return -2;
	}

	cursor = -num_header_size + 3 + (section_extract - 1) * 30 + 22;

	lseek(fd, cursor, SEEK_END);
	read(fd, sect_offset, 4);
	sect_offset[4] = '\0';
	strcpy(sect_offset, strrev(sect_offset));

	for(int i = 0, j = 0; i < strlen(sect_offset); i++, j+= 2)
        sprintf(hex + j, "%02x", sect_offset[i] & 0xff);

    num_sect_offset = (int)strtol(hex, NULL, 16);

    cursor += 4;
    lseek(fd, cursor, SEEK_END);
    read(fd, sect_size, 4);
    sect_size[4] = '\0';
    strcpy(sect_size, strrev(sect_size));

    for(int i = 0, j = 0; i < strlen(sect_size); i++, j+= 2)
        sprintf(hex + j, "%02x", sect_size[i] & 0xff);

    num_sect_size = (int)strtol(hex, NULL, 16);

    cursor = num_sect_offset; //+ num_sect_size;
    //lseek(fd, --cursor, SEEK_SET);
    lseek(fd, cursor, SEEK_SET);

    char *buffer = (char*)malloc((num_sect_size + 1) * sizeof(char));
    read(fd, buffer, num_sect_size);
    buffer[num_sect_size]='\0';
    strcpy(buffer, strrev(buffer));

    char delimeter[] = "\n";
    char *p = strtok(buffer, delimeter);

    while(p != NULL)
    {
    	++actual_line;
    	if(actual_line == nrline_extract)
    		break;

    	p = strtok(NULL, delimeter);
    }

   
    if(actual_line != nrline_extract)
    {
    	return -3;
    }


    printf("SUCCESS\n");
    printf("%s\n", strrev(p));

    free(buffer);
    buffer = NULL;

    return 0;

}

int findall(const char *path, bool *first_trace)
{
	DIR *dir = NULL;
    struct dirent *entry = NULL;
    char fullPath[MAX_SIZE];
    struct stat statbuf;

    dir = opendir(path);
    if(dir == NULL)
    {
    	return -1;
    }

    if(*first_trace)
    {
    	printf("SUCCESS\n");
    	*first_trace = false;
    }

    while((entry = readdir(dir)) != NULL)
    {
    	if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
    	{
    		snprintf(fullPath, MAX_SIZE, "%s/%s", path, entry->d_name);
    		if(lstat(fullPath, &statbuf) == 0)
    		{
    			if(S_ISREG(statbuf.st_mode))
    			{

    				if(!parse_SF(fullPath, true))
    				{
    					int fd_body = -1, fd_header = -1, num_header_size, num_sect_offset, num_sect_size, num_nr_sections, cursor, nr_valid_sections, nr_lines, offset_test = 0;
    					char header_size[3], sect_offset[5], sect_size[5], nr_sections[2], hex[10];
    					char *buffer = NULL;

    					fd_header = open(fullPath, O_RDONLY);
    					fd_body = open(fullPath, O_RDONLY);

    					lseek(fd_header, -4, SEEK_END);
    					read(fd_header, header_size, 2);
						header_size[2] = '\0';
						strcpy(header_size, strrev(header_size));

						for(int i = 0, j = 0; i < strlen(header_size); i++, j+= 2)
        					sprintf(hex + j, "%02x", header_size[i] & 0xff);
    
    					num_header_size = (int)strtol(hex, NULL, 16);
    					cursor = -num_header_size + 2;

    					lseek(fd_header, cursor, SEEK_END);
					    read(fd_header, nr_sections, 1);
					    nr_sections[1] = '\0';
					    sprintf(hex, "%02x", nr_sections[0] & 0xff);

						num_nr_sections = (int)strtol(hex, NULL, 16);

						nr_valid_sections = 0;
						//printf("\n%s\n", fullPath);
						for(int i = 0; i < num_nr_sections && nr_valid_sections < 3; i++)
						{
							cursor = -num_header_size + 3 + 22 + i * 30;

							lseek(fd_header, cursor, SEEK_END);
							read(fd_header, sect_offset, 4);
							sect_offset[4] = '\0';
							strcpy(sect_offset, strrev(sect_offset));

							for(int i = 0, j = 0; i < strlen(sect_offset); i++, j+= 2)
						        sprintf(hex + j, "%02x", sect_offset[i] & 0xff);

						    num_sect_offset = (int)strtol(hex, NULL, 16);

						    if(i != 0)
						    {
						    	if(num_sect_offset < offset_test)
						    	{
						    		nr_valid_sections = 0;
						    		break;
						    	}
						    }
						    //printf("help\n");

						    cursor += 4;

					    	lseek(fd_header, cursor, SEEK_END);
					    	read(fd_header, sect_size, 4);
					    	sect_size[4] = '\0';
					    	strcpy(sect_size, strrev(sect_size));

					    	for(int j = 0, k = 0; j < strlen(sect_size); j++, k+= 2)
					        	sprintf(hex + k, "%02x", sect_size[j] & 0xff);

					        num_sect_size = (int)strtol(hex, NULL, 16);

					        buffer = (char*)malloc((num_sect_size + 1) * sizeof(char));
					        cursor = num_sect_offset;
					        lseek(fd_body, num_sect_offset, SEEK_SET);

					       	nr_lines = 0;

					       	read(fd_body, buffer, num_sect_size);
					       	buffer[num_sect_size] = '\0';

					       	char delimeter[] = "\n";
					       	char *p = strtok(buffer, delimeter);

					       	while(p != NULL)
					       	{
					       		++nr_lines;
					       		//printf("%s\n", p);
					       		p = strtok(NULL, delimeter);
					       	}

					       	//printf("Linii: %d\n\n", nr_lines);
					       	//printf("\n\n");
					       	if(nr_lines == 15)
					       		++nr_valid_sections;

					       	offset_test = num_sect_offset;

					       	free(p);
					       	p = NULL;
					       	free(buffer);
					       	buffer = NULL;

					       	//printf("Sectiunea %d are %d linii\n", i + 1, nr_lines);

						}
							
						if(nr_valid_sections == 3)
							printf("%s\n", fullPath);

    				}
    			}
    			

	    		if(S_ISDIR(statbuf.st_mode))
	    		{
	    			findall(fullPath, first_trace);
	    		}
    		}
    	}
    }

    closedir(dir);

    return 0;
}

int main(int argc, char **argv)
{
	char *path = NULL;
	path = (char*)malloc(MAX_SIZE * sizeof(char));	

   if(argc >= 2)
    {

        if(strcmp(argv[1], "variant") == 0)
        {
            printf("54563\n");
            goto end;
            
        }

        for(int i = 1; i < argc; i++)
        {
        	if(strcmp(argv[i], "list") == 0)
        	{
        		bool recursive = false;
	        	char *size_greater = NULL, *name_ends_with = NULL;

	        	size_greater = (char*)malloc(MAX_SIZE * sizeof(char));
	        	name_ends_with = (char*)malloc(MAX_SIZE * sizeof(char));
	        	

	        	if(argc == 2)
	        	{
	        		printf("ERROR\ninvalid directory path\n");

	        		free(size_greater);
	        		free(name_ends_with);
	        		size_greater = NULL;
	        		name_ends_with = NULL;

	        		break;
	        	}

	        	for(int i = 2; i < argc; i++)
	        	{
	        		if(strcmp(argv[i], "recursive") == 0)
	        		{
	        			recursive = true;
	        		}
	        		else
	        		{
	        			if(strstr(argv[i], "size_greater=") != NULL)
	        			{
	        				strcpy(size_greater, argv[i]);
	        			}
	        			else
	        			{
	        				if(strstr(argv[i], "name_ends_with=") != NULL)
	        				{
	        					strcpy(name_ends_with, argv[i]);
	        				}
	        				else
	        				{
	        					strcpy(path, argv[i] + 5);
	        				}
	        			}
	        		}
	        	}

	        	if(recursive == false)
	        	{
	        		if(list_nonrecursive(path, name_ends_with, size_greater) == -1)
	        		{
	        			printf("ERROR\ninvalid directory path\n");

	        		}
	        		
	        	}
	        	else
	        	{
	        		bool first_trace = true;
	        		if(list_recursive(path, name_ends_with, size_greater, &first_trace) == -1)
	        		{
	        			printf("ERROR\ninvalid directory path\n");
	        		}
	        	}

	        	free(size_greater);
	        	free(name_ends_with);
	        	size_greater = NULL;
	        	name_ends_with = NULL;
	        	break;
       		}

       		if(strcmp(argv[i], "parse") == 0)
       		{

       			for(int i = 1; i < argc; i++)
       			{
       				if(strcmp(argv[i], "parse") != 0)
       				{
       					strcpy(path, argv[i] + 5);
       					break;
       				}
       			}

       			int ok = parse_SF(path, false);

       			switch(ok)
       			{
       				case -1:
       					printf("ERROR\nwrong path_file\n");
       					break;
       				case -2:
       					printf("ERROR\nwrong magic\n");
       					break;
       				case -3:
       					printf("ERROR\nwrong version\n");
       					break;
       				case -4:
       					printf("ERROR\nwrong sect_nr\n");
       					break;
       				case -5:
       					printf("ERROR\nwrong sect_types\n");

       				default:
       					break;
       			}

       			break;
       		}

       		if(strcmp(argv[i], "extract") == 0)
       		{
       			int section_extract, nrline_extract, ok;

       			for(int i = 1; i < argc; i++)
       			{
       				if(strstr(argv[i], "path=") != NULL)
       				{
       					strcpy(path, argv[i] + 5);
       				}
       				else
       				{
       					if(strstr(argv[i], "section=") != NULL)
       					{
       						if(sscanf(argv[i] + 8, "%d", &section_extract) != 1)
       						{
       							printf("ERROR\ninvalid section\n");
       							break;
       						}
       					}
       					else
       					{
       						if(strstr(argv[i], "line=") != NULL)
       						{
       							if(sscanf(argv[i] + 5, "%d", &nrline_extract) != 1)
       							{
       								printf("ERROR\ninvalid line\n");
       								break;
       							}
       						}
       					}
       				}
       			}

       			ok = extract_line(path, section_extract, nrline_extract);

       			switch(ok)
       			{
       				case -1:
       					printf("ERROR\ninvalid file\n");
       					break;
       				case -2:
       					printf("ERROR\ninvalid section\n");
       					break;
       				case -3:
       					printf("ERROR\ninvalid line\n");
       					break;

       				default:
       					break;
       			}

       			break;
       		}

       		if(strcmp(argv[i], "findall") == 0)
       		{
       			int ok;
       			bool first_trace = true;

       			for(int i = 1; i < argc; i++)
       			{
       				if(strstr(argv[i], "path=") != NULL)
       				{
       					strcpy(path, argv[i] + 5);
       					break;
       				}
       			}

       			ok = findall(path, &first_trace);
       			if(ok == -1)
       			{
       				printf("ERROR\ninvalid directory path\n");
       			}

       			break;
       		}
        }     
    }

	end:
	free(path);
	path = NULL;
	


    return 0;
}