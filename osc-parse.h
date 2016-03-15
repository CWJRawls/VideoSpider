//OSCParser

#include <string.h>
#include <stdlib.h>

//tests for common elements
#define ISNULL(b) (b == 0 ? 1 : 0)
#define ISSLASH(c) (c == '/' ? 1 : 0)
#define ISCOMMA(c) (c == ',' ? 1 : 0)
#define ISFLOAT(c) (c == 'f' ? 1 : 0)
#define ISDEC(c) (c == 'd' ? 1 : 0)
#define ISINT(c) (c == 'i' ? 1 : 0)
#define ISSTRING(c) (c == 's' ? 1 : 0)


//enum to keep track of what is being asked of it
typedef enum {no_value = 0, refresh, stop, disconnect, give_ip }Commands;

//enum to keep track of what is being stored in our od_cont struct
typedef enum {no_type = 0, integer, ofloat, decimal, string}O_Data;

//struct to hold the data from the osc packet as a byte array prior to parsing
typedef struct{
	char data[40];
	O_Data type;
} od_cont;

typedef struct { //location in byte array for packet
	int start,end;
} l_byte;

typedef struct{ //storing l_bytes for methods in packet
	l_byte name,path,junk,jitt;
	Commands cmd;
	od_cont data[100]; 
} meth;

//get the methods from the bundle
meth** unpackMethods(char input[], int size)
{
	printf("size = %d\n", size);
	int i = 0,counter = 0;
	l_byte temp;
	//allocate using calloc with size / 4 since the smallest types would be 4 bytes in length w/ padding
	meth** bundle = (meth **) calloc(size / 4, sizeof(meth *));
	while(i < size)
	{
		printf("i = %d : %2x\n", i, input[i]);
		if(input[i] == '#' && strncmp(&input[i], "#bundle", 7) == 0)
		{
			i += 19;
			printf("Bundle Found i = %d\n",i);
		}
		else if(ISSLASH(input[i]))
		{
			printf("Found a slash i = %d\n", i);
			temp.start = i;
			
			int last_slash = i;
			
			do{
				i++;
				
				if(ISSLASH(input[i]))
					last_slash = i;
				
			}while(!ISNULL((int)input[i]));
			printf("input[%d] is null? %2x\n", i, input[i] );
			int term = i;
			
			int s_size = (term - last_slash) + 1;
			
			while(i % 4 != 3) //check for corrct memory padding
			{
				i++;
			}
			
			temp.end = i;
			// increment the counter one more time so that we are at the beginning of a segment
			i++;
			if(sizeof("/command") <= s_size)
			{
				printf("Right Size");
				if(strncmp("/command", &input[last_slash], 8) == 0)
				{
					printf("Right Shape\n");
					//allocate just this instance of a command to all 0
					bundle[counter] = malloc(sizeof(meth));
					//start filling with data
					bundle[counter]->path.start = temp.start;
					bundle[counter]->path.end = last_slash - 1;
					bundle[counter]->name.start = last_slash;
					bundle[counter]->name.end = term;
					
					if(ISCOMMA(input[i])) 
					{
						printf("\t comma found\n");
						
						//want to know what kind of stuff we had in the array
						bundle[counter]->jitt.start = i;
						do 
						{
							i++;
						} while (!ISNULL((int)input[i]));
						bundle[counter]->jitt.end = i;
						
						//move over the padding
						while (i % 4 != 3) 
						{
							i++;
						}
						
						i++;
						
						//want to know how many fields in junk
						int num_fields = bundle[counter]->jitt.end - (bundle[counter]->jitt.start + 1); 
						
						int j = 0, dcounter = 0;
						
						int d_start = bundle[counter]->jitt.start + 1; //exclude the comma
						
						//loop to create and od_cont structs from method data
						while(j < num_fields && !ISSLASH(input[i]) && i < size)
						{
							if(ISFLOAT(input[d_start + j]))
							{
								bundle[counter]->data[dcounter].type = ofloat;
								
								int k;
								for(k = 0; k < 4 && i < size; k++) //32 bit float = 4 bytes
								{
									bundle[counter]->data[dcounter].data[k] = input[i + k];
								}
								
								i += k;
							}
							else if(ISINT(input[d_start + j]))
							{
								bundle[counter]->data[dcounter].type = integer;
								
								int k;
								for(k = 0; k < 4 && i < size; k++) //32 bit int = 4 bytes
								{
									bundle[counter]->data[dcounter].data[k] = input[i + k];
								}
								
								i += k;
							}
							else if(ISDEC(input[d_start + j]))
							{
								bundle[counter]->data[dcounter].type = decimal;
								
								int k;
								for(k = 0; k < 8 && i < size; k++) //64 bit decimal = 8 bytes
								{
									bundle[counter]->data[dcounter].data[k] = input[i + k];
								}
								
								i += k;
							}
							else if(ISSTRING(input[d_start + j])) 
							{
								printf("\t string %d found! bundle = %d\tdcounter = %d\n", string, counter, dcounter);
								bundle[counter]->data[dcounter].type = string;
								
								int k = 0;
								while(k < 40 && input[i + k] != 0 && i + k < size) //string can be of size n. capping at 40 bytes
								{
									bundle[counter]->data[dcounter].data[k] = input[i + k];
									k++;
								}
								
								i += k;
								
								while(i % 4 != 3 && i < size) //move past padding
								{
									i++;
								}
								
								i++;
							}
							j++;
						}
						
						if(j < num_fields - 1)
						{
							printf("malformed packet: expected %d data fields, only found %d\n", num_fields, ++j);
						}
						
						/*if(j < 99) //if we did not fill available fields, set rest to no value
						{
							for(; j < 100; j++)
							{
								bundle[counter].data[dcounter].type = no_value;
							}
						}*/
						
					}
					// looking for the comma, but didn't find it
					else {
						printf("malformed packet: comma expected, but got %2x instead\n", input[i]);
					}
					counter++;
				}
			}
			
		}
		
		i++;
	}
	
	printf("bundle size = %d\n", sizeof(bundle));
	return bundle;
}

int getCommand(od_cont in)
{
	//test against types
	if(in.type != string)
	{
		return 0;
	}
	else
	{
		if(strncmp("refresh",in.data, strlen("refresh")) == 0)
		{
			return refresh;
		}
		else if(strncmp("stop",in.data, strlen("stop")) == 0)
		{
			return stop;
		}
		else if(strncmp("disconnect",in.data, strlen("disconnect")) == 0)
		{
			return disconnect;
		}
		else if(strncmp("give_ip",in.data, strlen("give_ip")) == 0)
		{
			return give_ip;
		}
		else
		{
			return no_value;
		}
	}
	
	return no_value;
}

//Function to remove any commands received in a packet and pass on the rest
void remove_command(char* in, int size, l_byte a, l_byte b)
{
	//find the size of the chunk to remove
	int r_size = b.end - a.start;
	
	char* ptr;//create and init return pointer
	ptr = in;
	
	int i = 0;
	while(i < a.start) //move the pointers to where things change
	{
		ptr++;
		in++;
	}
	
	//move the input pointer location past the area to be removed
	in += r_size;
	
	if(b.end < (size - 1)) //if there is something after the chunk
	{
		i = 0;
		while(i + b.end < size) //add evrything after the chunk
		{
			*ptr = *in;
			i++;
			ptr++;
			in++;
		}
	}
}


//Function to package ack message for replies to Max
char* create_ack(char addr[], int a_size, char msg[], int m_size)
{
	//default size with message '#bundle' + 3 lines of padding + ', s \0 \0'
	int size  = 24;
	//something to use for padding our data
	char pad = '\0';
	
	//find total address size
	int addr_size = a_size;
	
	while(addr_size % 4 != 0)
	{
		addr_size++; //add size until it fills whole line
	}
	
	//find total message size
	int msg_size = m_size;
	
	while(msg_size % 4 != 0)
	{
		msg_size++;
	}
	
	size += addr_size;
	size += msg_size;
	
	//create the array for the whole packet
	char pckt_out[size];
	//copy in the bundle header
	strcpy(pckt_out, "#bundle");
	//create index variable
	int i = 0;
	//move index to end of copy location
	while(pckt_out[i])
	{
		i++;
	}
	
	//if there is still space in the header padding
	while(i < 20)
	{
		pckt_out[i] = pad; //add padding value
	}
	
	int k = 0;
	while(i < (20 + addr_size)) //add in the address of the msg
	{
		if(k < a_size) //add in the address string
		{
			pckt_out[i] = addr[k];
		}
		else //fill the rest with padding
		{
			pckt_out[i] = pad;
		}
		i++;
		k++;
	}
	
	//add in the expected data type
	pckt_out[i] = ',';
	i++;
	pckt_out[i] = 's';
	i++;
	pckt_out[i] = pad;
	i++;
	pckt_out[i] = pad;
	i++;
	
	k = 0;
	while(i < size) //add in the message to the bundle
	{
		if(k < m_size)
		{
			pckt_out[i] = msg[k];
		}
		else
		{
			pckt_out[i] = pad;
		}
		i++;
		k++;
	}
	char* ptr = pckt_out;
	return ptr;
}

//Function to free up meth**
void free_meth(meth** m, int size)
{
	//since we expect that the size is actually the size of the char array received on the socket,
	//divide by line size to get the size used for calloc()
	size = size / 4;
	
	int i;
	for(i = 0; i < size; i++)
	{
		//free all non-NULL pointers
		if(m[i] != NULL)
		{
			free(m[i]);
		}
	}
	//free the whole pointer.
	free(m);
}
