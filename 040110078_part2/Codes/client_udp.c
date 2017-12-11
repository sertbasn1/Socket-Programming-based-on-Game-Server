
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>     
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>     
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <time.h>  
#include <sys/wait.h>

#include <errno.h>

struct GAME{
    int oyunid;
    int player1id;
    int player2id;
    char * category;
    char * word;
    char *currentWord;
    int errors;
    char wrongs[26];
    char enteredletters[26];
    int wordlength;
    int numofPlayers;//0 1 2
    int orderofMove;
    int movecounter;
    int isover;
    int player1score;
    int player2score;
    
};


struct Paket {
    char option[20];
    char username[1024];
    char password[1024];
    char Paket[50];
    char buff[1024];
    struct GAME pgame;
    int val;
};


struct userThread {
    pthread_t threadId; // thread's pointer
    int sockfd; // socket file descriptor
    int oyunId;
    char username[1024];
    char password[1024];
};

struct MOVE{
    char guess;
    int playerid;
    int exist;
    int val;
    
};

void menu(){
    printf("**** *** *** *** *** HANGMAN MENU *** *** *** *** ***\n");
    printf("To signup type signup\n");
    printf("To login type login\n");
    printf("To logout and exit type logout\n");
    printf("To create new game type create #categoryname\n");
    printf("        Categories: meyve  sehir  bitki\n");
    printf("To list games type showgames\n");
    printf("To enter a game type join #gameid\n");
    printf("To make a move in the game type move #guessed_character\n");
    printf("To see the users in any game type players #gameid\n");
    printf("**** *** *** *** *** *** *** *** *** *** ***\n");
}

void show_hang(int i,char wrongs [], int count, char entereds []);
int sockfd;
char option[1024];
char username[1024];
char password[1024];

void *receiver(void *param);
struct sockaddr_in server_addr,client_addr;

int main(int argc, char **argv){
	int recvd;
	int newfd;
	int sent,bytes_read;
   

	time_t curtime;
	
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("socket");
		exit(1);
	}
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	bzero(&(server_addr.sin_zero),8);


menu();
printf("Ready for receiving packages ...\n");

while(gets(option)) {
    
    if(!strncmp(option, "exit", 4)){
        exit(0);
    }
    else if(!strncmp(option, "signup", 6)){
        printf("Enter your username ");
        gets(username);
        printf("Enter your password ");
        gets(password);
        strcpy(package.option, "signup");
        strcpy(package.username, username);
        strcpy(package.password, password);
        sendto(sockfd, (void *)&package, sizeof(struct Paket), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        printf("Now you are member of Hangman you can login and start playing.\n");
    }
    else if(!strncmp(option, "login", 5)){
        printf("Enter your username ");
        gets(username);
        printf("Enter your password ");
        gets(password);
        strcpy(package.option, "login");
        strcpy(package.username, username);
        strcpy(package.password, password);
        sendto(sockfd, (void *)&package, sizeof(struct Paket), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));    }
    
    else if(!strncmp(option, "logout", 6)) {
        strcpy(package.option, "exit");
        sendto(sockfd, (void *)&package, sizeof(struct Paket), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));        exit(0);
    }
    else if(!strncmp(option, "menu",4)){
        menu();
    }
    else if(!strncmp(option, "create", 6)){
        strcpy(package.option, "create");
        strcpy(package.buff, option);
        sendto(sockfd, (void *)&package, sizeof(struct Paket), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));    }
    else if(!strncmp(option, "join", 4)){
        strcpy(package.option, "join");
        strcpy(package.buff, option);
        sendto(sockfd, (void *)&package, sizeof(struct Paket), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));    }
    else if(!strncmp(option, "showgames", 9)){
        strcpy(package.option, "showgames");
        strcpy(package.buff, option);
        sendto(sockfd, (void *)&package, sizeof(struct Paket), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));    }
    else if(!strncmp(option, "players", 7)){
        strcpy(package.option, "players");
        strcpy(package.buff, option);
        sendto(sockfd, (void *)&package, sizeof(struct Paket), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));    }
    
    else if(!strncmp(option, "move", 4)){
        strcpy(package.option, "move");
        strcpy(package.buff, option);
        sendto(sockfd, (void *)&package, sizeof(struct Paket), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));    }
    else { /* otherwise only send */
        strcpy(package.option, "send");
        strcpy(package.buff, option);
        sendto(sockfd, (void *)&package, sizeof(struct Paket), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));    }
}
return 0;
}

void *receiver(void *param) {
    int recvd;
    struct Paket package;
    
    while(1) {
        socklen_t len= sizeof(struct sockaddr_in);
        recvd =recvfrom(sockfd, (void *)&package, sizeof(struct Paket), 0, (struct sockaddr *)&server_addr, &len);
        if(!recvd) {
            printf( "Connection lost from server...\n");
            close(sockfd);
            break;
        }
        if(recvd > 0) {
            if(!strcmp(package.option, "showhang"))
                show_hang(package.pgame.errors,package.pgame.wrongs,package.pgame.movecounter, package.pgame.enteredletters);
            
            if(!strcmp(package.option, "loginresponse"))
                printf("[%s]:You authenticated at time %s\n", package.username, package.buff);
            
            if(!strcmp(package.option, "createresponse"))
                printf("Game is created. First join the game then start waiting for other player\n");
            
            if(!strcmp(package.option, "showgamesresponse"))
            {   if(package.pgame.oyunid==-1)
                printf("%s\n",package.buff);
            else{
                if(package.pgame.isover==1)//biten
                    printf("[Game %d] in %s category (This game ended, you cannot join)\n", package.pgame.oyunid, package.buff);
                else
                    printf("[Game %d] in %s category\n", package.pgame.oyunid, package.buff);
            }
            }
            
            if(!strcmp(package.option, "playersresponse"))
                printf("[Client %s] named %s\n", package.username, package.buff);
            
            else if(!strcmp(package.option, "gameresult"))
                printf("%s%d\n",package.buff, package.val);
            
            else if(!strcmp(package.option, "info"))
                printf("[%s]: %s\n", package.username, package.buff);
        }
        
        memset(&package, 0, sizeof(struct Paket));
    }
    return NULL;
}


void show_hang(int i,char wrongs [], int count, char entereds[]) {
    int j=-1;
    printf("%s","Entered letters are:");
    while(j<count){
        printf("%c%s",entereds[j]," ");
        j++;}
    
    printf("%s","\n");
    
    j=-1;
    printf("%s","Entered wrong letters are:");
    while(j<i){
        printf("%c%s",wrongs[j]," ");
        j++;}
    
    switch (i) {
        case 0 :
            printf("\nNum of wrong letters: %d\n\n", i);
            printf("\n");
            printf("\n");
            printf("\n");
            printf("\n");
            printf("\n");
            printf("\n");
            printf("____________\n\n");
            break;
        case 1 :
            printf("\nNum of wrong letters: %d\n\n", i);
            printf("\n");
            printf("  |\n");
            printf("  |\n");
            printf("  |\n");
            printf("  |\n");
            printf("  |\n");
            printf("__|_________\n\n");
            break;
        case 2 :
            printf("\nNum of wrong letters: %d\n\n", i);
            printf("  _______\n");
            printf("  |\n");
            printf("  |\n");
            printf("  |\n");
            printf("  |\n");
            printf("  |\n");
            printf("__|_________\n\n");
            break;
        case 3 :
            printf("\nNum of wrong letters: %d\n\n", i);
            printf("  _______\n");
            printf("  |/\n");
            printf("  |\n");
            printf("  |\n");
            printf("  |\n");
            printf("  |\n");
            printf("__|_________\n\n");
            break;
        case 4 :
            printf("\nNum of wrong letters: %d\n\n", i);
            printf("  _______\n");
            printf("  |/   | \n");
            printf("  |    O \n");
            printf("  |\n");
            printf("  |\n");
            printf("  |\n");
            printf("__|_________\n\n");
            break;
        case 5 :
            printf("\nNum of wrong letters: %d\n\n", i);
            printf("  _______\n");
            printf("  |/   | \n");
            printf("  |    O \n");
            printf("  |    |\n");
            printf("  |    |\n");
            printf("  |\n");
            printf("__|_________\n\n");
            break;
        case 6 :
            printf("\nNum of wrong letters: %d\n\n", i);
            printf("  _______\n");
            printf("  |/   | \n");
            printf("  |    O \n");
            printf("  |   \\|\n");
            printf("  |    | \n");
            printf("  |\n");
            printf("__|_________\n\n");
            break;
        case 7 :
            printf("\nNum of wrong letters: %d\n\n", i);
            printf("  _______\n");
            printf("  |/   | \n");
            printf("  |    O \n");
            printf("  |   \\|/\n");
            printf("  |    | \n");
            printf("  |\n");
            printf("__|_________\n\n");
            break;
        case 8 :
            printf("\nNum of wrong letters: %d\n\n", i);
            printf("  _______\n");
            printf("  |/   | \n");
            printf("  |    O \n");
            printf("  |   \\|/\n");
            printf("  |    | \n");
            printf("  |   /\n");
            printf("__|_________\n\n");
            break;
        case 9 :
            printf("\nNum of wrong letters: %d\n\n", i);
            printf("  _______\n");
            printf("  |/   | \n");
            printf("  |    O \n");
            printf("  |   \\|/\n");
            printf("  |    | \n");
            printf("  |   / \\\n");
            printf("__|_________\n\n");
            break;
        case 10 :
            printf("\nNum of wrong letters: %d\n\n", i);
            printf("  _______\n");
            printf("  |/   | \n");
            printf("  |    X \n");
            printf("  |   \\|/\n");
            printf("  |    | \n");
            printf("  |   / \\\n");
            printf("__|_________\n\n");
            break;
    }
}




