#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>

#define CREATE_USER_ACCOUNT		1
#define SAVE_DATA 					2
#define CLOSE_MEETING				3
#define DELETE_USER_ACCOUNT		4
#define NO_ACTION						10

#define FOR_EVER 	1


void *location_receiver(void*);
void *display_location(void*);
void *doprocessing (int *);
char buffer;

static unsigned int update_room1 = 0;
static unsigned int update_room2 = 0;
static unsigned int ret = 0;

struct location {

char        name[30];
unsigned int id;
unsigned int fid;
};

#define WINDOW_WIDTH  700
#define WINDOW_HEIGHT 500

#define RADIUS 150
#define N_WORDS 10
#define FONT "Sans  10"

struct person{
	char name[10];
	int id;
	int fid;
};

struct person student[20];

void* recive_user_input(void *);

struct customer {
   char  fname[20],lname[20];
   int   acct_num;
   float acct_balance;
};
int file_r()
{
   FILE *infile;
	int i = 0;

   infile = fopen ("student.dat","r");
   if (infile == NULL)
   {
      fprintf(stderr, "\nError opening accounts.dat\n\n");
		fclose(infile);
		return 0;
   }

   fread (student, sizeof(student), 1, infile);

	for(i = 0; i < 20; i++)
      printf ("Name = %10s ID: %10d   finge id = %8d   \n",
              student[i].name, student[i].id, student[i].fid);
	fclose(infile);
}

int file_rd(char * name)
{

   FILE *outfile;
	int n = 0;
   outfile = fopen ("record.dat","r");
   if (outfile == NULL)
   {
      printf("\nError:  /home/pi/finger/record.dat not opend\n");
      return -1;
   }
		
  fscanf (outfile,"%s", name);
//	n = atoi(name);
//   printf("Name is stored is : %s fid: %d\n", name, n);
   fclose(outfile);
   return 0;
}



//=============
int file_w()
{
	int i = 0;
	FILE *outfile;
   outfile = fopen ("student.dat","w");
   if (outfile == NULL)
   {
      fprintf(stderr, "\nError opening accounts.dat\n\n");
      exit (1);
   }
   // instructions to user
   printf("Enter \"stop\" for First Name to end program.");

   while (1)
     {
      // prompt user
      printf("\nFirst Name: ");
      scanf ("%s", student[i].name);
      // exit if no name provided
      if (strcmp(student[i].name, "stop") == 0)
		{
			fflush(stdin);
			fclose(outfile);
         return 0;
		}
      // continue reading from keyboard
      printf("Id Num  : ");
      scanf ("%d", &student[i].id);
      printf("finger print ID    : ");
      scanf ("%d", &student[i].fid);

      // write entire structure to Accounts file
      fwrite (&student[i], sizeof(struct person), 1, outfile);
		i++;
     }

	fclose(outfile);
	return 0;
}

/**************************************/
int file_read( char *line)
{
   FILE *infile;
	int read = 0;
	size_t len = 20;
   infile = fopen ("found.dat","r");
   if (infile == NULL)
   {
      fprintf(stderr, "\nError opening found.dat\n\n");
		return 0;
   }
		
	read = fscanf(infile,"%s", line);
	printf("Name read from found.data is : %s\n", line);	
}

int file_readFinger( char *line)
{  
   FILE *infile;
   int read = 0;
   size_t len = 20;
   infile = fopen ("found2.dat","r");
   if (infile == NULL)
   {
      fprintf(stderr, "\nError opening found2.dat\n\n");
      return 0;
   }
      
   read = fscanf(infile,"%s", line);
   printf("Finger ID read from found.data is : %s\n", line);
}


void init_data()
{
   int i = 0;
   int c  = 0;

   while(i< 20)
   {
      memset(&student[i].name, c,29 );
      student[i].id = 0;
		student[i].fid = 0;	
      i++;
   }
}

void create_uid()
{
	static int i = 0;
	int c  = 0;
	int n = 0;
	int j = 0;
	char str[3];
	int k = 0;
	
	FILE *outfile;
   outfile = fopen ("student.dat","w");
   if (outfile == NULL)
   {
      fprintf(stderr, "\nError opening accounts.dat\n\n");
      return;
   }
	i = 0;
	while(i< 20)
	{
		printf("Enter User Name: \n");

		scanf("%s", student[i].name);
		
		printf("Enter User Id: \n");
		scanf("%d", &student[i].id);
		printf("Enter Finger print \n");
		system("sudo ./fingerprint &");

		for(k = 0; k < 700; k++)
			usleep(70000);

		system("sudo pkill fingerprint");

		file_rd(str);
		n = atoi(str);
		printf("User finger print Id : %d: \n", n);	
	//	printf("Enter any number to exit fingerprint  \n");
	//	scanf("%d", &j);	
		student[i].fid = n;
//	 ile_read(str);
//		n = atoi(str);
//		printf("User finger print Id found: %d: \n", n);		
			
		printf(" student Name: %s, id: %d,fingerId: %d \n", student[i].name, student[i].id,student[i].fid);
		i++;
		printf("Enter number greater 100 to exit, less than 100 to continue \n");
    	fflush(stdin);
		scanf("%d", &c);
		if(c > 100)
			break;
		else 
			continue;
	}
	printf("Saving Student.dat file \n");
   fwrite (student, sizeof(student), 1, outfile);
	fclose(outfile);
}

int main (int argc, char *argv[]){

    //Block SIGALRM in the main thread
   sigset_t sigset;
	pthread_t recv, disp;
	pthread_t cli_id;
   int err;
	unsigned int c = 0;
	
	init_data();
	printf("Reading Student details from file: student.dat \n");
	sleep(1);
	file_r();
	printf("Setting IP address \n");
	system("sudo ifconfig eth0 169.254.13.91");

	printf(" Starting  Threads ...\n\n");
	pthread_create( &recv, NULL,  location_receiver, NULL);
//	pthread_create( &disp, NULL, (void*) &display_location, NULL);
	err = pthread_create(&cli_id, NULL, recive_user_input, NULL);

	while(1);
    return 0;
}

void * recive_user_input(void * arg)
{
	char x;
	//char name[30];
	unsigned int y = 0;
//	unsigned int ret = NO_ACTION;
	int userId = 0;
	int gesutre_type = 0;

	while(FOR_EVER)	
	{

	 fflush(stdin);
    printf("To Start Enter any char...\n");

    scanf("%c",&x);

    printf("To Create New User  ... ... ... ... ... .Press - 1\n");
    printf("To Save Data    ... ... ... ... ... ... .Press - 2\n");
    printf("To Close Meeting .. ... ... ... ... ... .Press - 3\n");
    printf("To Delete a User Account... ... ... ... .Press - 4\n");

    fflush(stdin);
    scanf("%u",&y);
     switch(y)
     {
		
		case CREATE_USER_ACCOUNT:
		{
			create_uid();
			break;
		}
		case SAVE_DATA:
      {
          printf("Saving student data to  file \n");
          break;
      }
		case CLOSE_MEETING:
      {
            printf("Closeing Meeting \n");
            ret =  CLOSE_MEETING;
		    break;
		}
		case DELETE_USER_ACCOUNT:
		{
			printf("User Delet Starting  \n");
			//delet_user();
			ret = DELETE_USER_ACCOUNT;
			
			break;
		}
		default:
			printf(" Default value \n");
			ret = NO_ACTION;
			break;
	 }

	} // end of while loop

	return NULL;
}

void* location_receiver(void* a)
{
	 int socket_desc , client_sock , c;
    struct sockaddr_in server , client;

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 5001 );
    puts("binding ... ");
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return ;
    }

    puts("Listing for incoming connections...");
    //Listen
    listen(socket_desc , 3);
    //Accept and incoming connection
    c = sizeof(struct sockaddr_in);
    //iccept and incoming connection
    puts("Starting Accept for incoming connections...");

	c = sizeof(struct sockaddr_in);
    pthread_t thread_id;
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client,
                                                        (socklen_t*)&c)) )
    {
//        puts("Connection accepted");
        if( pthread_create( &thread_id , NULL , &doprocessing,
                                            (void*) &client_sock) < 0)
        {
            perror("could not create thread");
            return ;
        }
    }
	if (client_sock < 0)
    {
        perror("accept failed");
        return ;
    }
    return ;


}// end of location receiver: threads

void clean_ped()
{
	int c = 0;

	printf("Start of clean Pedestrian Data \n");
	sleep(2);

}
void* doprocessing (int *cli_sock)
{
	char line[20];
	char ch[2];
	char end[6] = "ends1";
	char start[6] = "start";
  	int n;
	int k = 0;
	int jj = 0;
	unsigned int ret = 0;
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int found = 0;
   char buffer[90];
	int sock =   *cli_sock;
   struct location *loc;
	static unsigned int p1= 0;
	static unsigned int p2= 0;
	int found2 = 0;
	int found3 = 0;
	int t = 0;

   memset(buffer, 0, sizeof (buffer));

   n = read(sock, buffer, 90 );
   	if (n < 0)
   	{
      	perror("ERROR reading from socket");
			return 0;
   	}
   	printf(" read value: %d: \n", n);
   	loc = (struct location *)(buffer);
	//  memcpy(&loc, buffer, 90);

	//   printf("Here is the message: %s\n",buffer);
    printf("  Name: %s, id: %u, fid:  %d,  \n",
                            loc->name,
                            loc->id,
                            loc->fid );
		printf("student Name is::: %s \n", student[0].name);
		printf("student Name is::: %s \n", student[1].name);
	
		for(i = 0; i < 20; i++)
		{
			printf("inside loop \n");
			if( (strcmp(loc->name, student[i].name) == 0) && (loc->id == student[i].id))
			{
				printf("\n Name and IDi remote USER Authentication Success !!!!\n\n");
				found = 1;
				k = i;
				break;
			}
		}	
		if(found == 0)
		{
			n = send(sock , end, 6, 0);
         if( n < 0)
         {
            printf("Send failed");
            return 0;
         }
			printf("Name and ID of remote USERi Authentication  Failed \n");
		}	
		sleep(4);
		system("sudo ./client_jpg");
      sleep(2);
		system(" sudo ./facerec_eigenfaces /home/pi/data/img/path.ext /home/pi/data/output/");
		file_read( line);
			
		printf("read from file read is : %s ,student name :: %s\n",line, student[k].name);
		if(strcmp(line, student[k].name) == 0)	
		{
			found2= 1;
			printf("Face identification is Success: %s\n", line); 
			usleep(3000);
			
		}
		usleep(3000);

		if(found2 == 1)
		{
			
			printf("Enter Finger print \n");
			system("sudo ./fingerprint &");
			for(t = 0; t < 500; t++)
				usleep(70000);

	      system("sudo pkill fingerprint");			

			//printf("Enter any number to exit fingerprint  \n");
   	  // scanf("%d", &jj);

		//	system("/home/pi/finger/fingerprint");
			file_readFinger(ch);
      	n = atoi(ch);
      	printf("User finger print Id : %d: \n", n);
			if(student[k].fid == n)
			{
     			 printf("User finger print Id matching : %d: \n", n);
				found3 = 1;
				
			}	
		}
			
		if(found && found2 && found3)
		{
			n = send(sock , start, 6, 0);	
			if( n < 0)
    		{
        		printf("Send failed");
        		return 0;
    		}
			printf("Authentication Success: Start Conference \n");

		//   system("sudo motion -n &");
			sleep(5);

 	//		system("x-www-browser http://169.254.143.3:8081");     	   
		}
		else
		{
			n = send(sock , end, 6, 0);
         if( n < 0)
         {
            printf("Send failed");
            return 0;
         }
			printf("\n\n========  Authentication Failure !!!!!!!!!!!========\n");
		}
}
