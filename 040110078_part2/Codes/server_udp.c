
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <errno.h>

    #include <netdb.h>
    #include <unistd.h>
    #include <pthread.h>

    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <sys/types.h>
    #include <time.h>
    #include <errno.h>
    #define CLIENTS 20
    #define GAMES 20


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
    int userid;
    int oyunId;
    char username[1024];
    char password[1024];
};

struct MOVE{
    char guess;
    int playerid; //socketid
    int exist;
    int val;
};

struct node {
    struct userThread threadinfo;
    struct node *next;
};

struct linkedList {
    struct node *head, *tail;
    int size;
};

//linkedlist functions
int compare(struct userThread *a, struct userThread *b) {
    return a->userid - b->userid;
}

void createList(struct linkedList *_list) {
    _list->head = _list->tail = NULL;
    _list->size = 0;
}

int insert(struct linkedList *_list, struct userThread *thr_info) {
    if(_list->size == CLIENTS) return -1;
    if(_list->head == NULL) {
        _list->head = (struct node *)malloc(sizeof(struct node));
        _list->head->threadinfo = *thr_info;
        _list->head->next = NULL;
        _list->tail = _list->head;
    }
    else {
        _list->tail->next = (struct node *)malloc(sizeof(struct node));
        _list->tail->next->threadinfo = *thr_info;
        _list->tail->next->next = NULL;
        _list->tail = _list->tail->next;
    }
    _list->size++;
    return 0;
}

int list_delete(struct linkedList *_list, struct userThread *thr_info) {
    struct node *curr, *temp;
    if(_list->head == NULL) return -1;
    if(compare(thr_info, &_list->head->threadinfo) == 0) {
        temp = _list->head;
        _list->head = _list->head->next;
        if(_list->head == NULL) _list->tail = _list->head;
        free(temp);
        _list->size--;
        return 0;
    }
    for(curr = _list->head; curr->next != NULL; curr = curr->next) {
        if(compare(thr_info, &curr->next->threadinfo) == 0) {
            temp = curr->next;
            if(temp == _list->tail) _list->tail = curr;
            curr->next = curr->next->next;
            free(temp);
            _list->size--;
            return 0;
        }
    }
    return -1;
}


char randomNumber(int max_number) {
    srand(time(NULL));
    int g = (rand() % (max_number + 1));
    return g;
}



char *getWord(char * category) {
    char buffer[50000];
    int n;
    FILE *file=fopen(category, "r");
    
    if(file==NULL) {
        printf("An error has occured: can't open file\n");
        exit(1);}
    
    n = fread(buffer, 1, 50000, file);
    buffer[n] = '\0';
    
    /* Separating the contents, divided by | */
    char *token = strtok(buffer, "|");
    char *words[200] = {0};
    int f = 0;
    while(token != NULL)
    { words[f] = malloc(strlen(token)+1);
        strcpy(words[f],token);
        token = strtok(NULL, "|");
        f++;
    }
    fclose(file);
    
    int wordN = randomNumber(f);
    int q;
    for(q  = 0; q < 200; q++)
    {    if( q != wordN)
        free(words[q]);
    }
    return words[wordN];
}

/* check the move is exist or not in the game, set the related parameters*/
int game(struct GAME *mygame,struct MOVE * mymove){
    char guess=mymove->guess;
    struct Paket statuspacket;
    int t,i,n;
    int check=0;//  1/2=bitti  0=devam
    int guessedLetter = 0;
    /*player entered a correct letter 0 = false, 1 = true */
    printf("\n%s%s\n", "Aim is to find: ", mygame->word);
    
    guess = tolower(guess); /* Removing caps */
    if (guess != '\n') {
        if (isalpha(guess)) { /*letter is alphanumeric */
            /* Check the letter occurs in the guessWord or not */
            for (i = 0; i < mygame->wordlength; i++) {
                if (mygame->word[i] == guess)
                {mygame->currentWord[i] = guess;
                    guessedLetter = 1;
                    mymove->val+=1;}     }
            
            if (guessedLetter == 0)
            {   //printf("%s%c","That letter was incorrect.\n\n",mymove->guess);
                mygame->wrongs[ mygame->errors]=guess;
                mygame->enteredletters[ mygame->movecounter]=guess;
                mygame->movecounter+=1;
                mygame->errors++;
                mymove->exist=0;}
            else{
                mygame->enteredletters[ mygame->movecounter]=guess;
                mygame->movecounter+=1;
                guessedLetter = 0;
                //printf("That letter was correct.\n");
                mymove->exist=1;}
            
            //printf("%s%s\n\n", "The word with guessed letters: ", mygame->currentWord);
            char msg[50]="The word with guessed letters: ";
            strcat(msg,mygame->currentWord);
            printf("%s\n", msg);
            
            strcpy(statuspacket.username,"Server");
            strcpy(statuspacket.buff, msg);
          //  send(mymove->playerid, (void *)&statuspacket, sizeof(struct Paket), 0);
            
            /*the word has not been guessed and the errors<10 = game will continue */
            if((strncmp(mygame->currentWord, mygame->word,mygame->wordlength)!= 0) && (mygame->errors < 10) )
                check=0;
            if(strncmp(mygame->currentWord, mygame->word,mygame->wordlength)== 0) //kelime bulundu
                check=1;
            if(mygame->errors == 10)
                check=2;
        }
        else
            printf("Only alphanumeric symbols are allowed (a-z, A-Z), try again:\n");			}
    
    return check;
}


int sockfd;
pthread_mutex_t clientlist_mutex;

struct userThread thread_info[CLIENTS];
struct linkedList client_list;
struct GAME * gamelist[GAMES];  //max 20 game is allowed
int allclientadresses[CLIENTS]; //max 20 client is allowed
int userindex=1;  //number of users in the system
int gameIndex=1;
void *cleanThreadFunc(void *param);
void *clientThreadFunc(void *fd);
void printallgames();

int findclient(int adres){
    int i;
    for(i=1;i<CLIENTS;i++)
       if( allclientadresses[i]==adres)
           return i;
    
    return -1;
   
}

struct sockaddr_in server_addr, client_addr;
int addr_len;

int main(int argc, char **argv){
    int i;
    int PORT=atoi(argv[1]);
	int bytes_read;
	pthread_mutex_init(&clientlist_mutex, NULL);
	struct Paket packet;
    
    for(i=0;i<CLIENTS;i++)
        allclientadresses[i]=0;
    
    //socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
            perror("Socket");
            exit(1);
        }
    
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        server_addr.sin_addr.s_addr = INADDR_ANY;
        bzero(&(server_addr.sin_zero),8);
	
    if(bind(sockfd,(struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1)
        {
            perror("Bind");
            exit(1);
        }
    
	addr_len = sizeof(struct sockaddr);
 
    printf("Data transferi icin hazir\n");
    fflush(stdout);
    
	while(1){
        recvfrom(sockfd, (void *)&packet ,sizeof(struct Paket),0,(struct sockaddr *)&client_addr, &addr_len);
        struct userThread threadinfo;
        printf("\n connection from (%s , %d)",inet_ntoa(client_addr.sin_addr),ntohs(client_addr .sin_port));


        
        threadinfo.sockfd = sockfd;
        threadinfo.oyunId= 0;
        threadinfo.userid=userindex;
    
        
        pthread_mutex_lock(&clientlist_mutex);
        insert(&client_list, &threadinfo);
        
       
        
        for(i=1;i<CLIENTS;i++)
            if(allclientadresses[i]!=ntohs(client_addr.sin_port))
            {   allclientadresses[userindex]=ntohs(client_addr.sin_port);
                userindex++;
                int tmp=findclient(ntohs(client_addr.sin_port));
                
                if(tmp==1)
                    printf("There is only one connection\n");
                else if(tmp>1)
                    printf("Server multiplex sockets\n");
                break;
            }
                
            else break;
       
        fflush(stdout);
                
        
                
        pthread_mutex_unlock(&clientlist_mutex);
        pthread_create(&threadinfo.threadId, NULL, clientThreadFunc, (void *)&threadinfo);
    }
    
}

void *cleanThreadFunc(void *param) {
    char option[20];
    while(scanf("%s", option)==1) {
        if(!strcmp(option, "exit")) { /* clean operations */
            printf("Terminating server...\n");
            pthread_mutex_destroy(&clientlist_mutex);
            close(sockfd);
            exit(0);
        }
    }
    return NULL;
}

void *clientThreadFunc(void *fd) {
    int t;
    int pflag=0, uflag=0, loginflag=0;
    struct userThread threadinfo = *(struct userThread *)fd;
    struct Paket packet;
    struct node *curr;
    int bytes, sent;
    time_t curtime;
    char Paket[20];
    
    while(1) {
        time(&curtime);
        addr_len = sizeof(struct sockaddr);
        bytes =recvfrom(sockfd, (void *)&packet ,sizeof(struct Paket),0,(struct sockaddr *)&client_addr, &addr_len);

        if(!bytes) {
            fprintf(stderr, "Connection lost from [%d] %s...\n", threadinfo.userid, threadinfo.username);
            pthread_mutex_lock(&clientlist_mutex);
            list_delete(&client_list, &threadinfo);
            pthread_mutex_unlock(&clientlist_mutex);
            break;
        }
        printf("[%d] %s %s", threadinfo.userid, packet.option, packet.buff);
        
        
        if(!strcmp(packet.option, "login")){
            pthread_mutex_lock(&clientlist_mutex);
            struct Paket welcome;
            for(curr = client_list.head; curr != NULL; curr = curr->next){
                if(!strcmp(curr->threadinfo.username, packet.username)){
                    uflag=1; //username holds
                    if(!strcmp(curr->threadinfo.password, packet.password)){
                        pflag=1;//password holds
                        loginflag=1;
                        printf("%s is authenticated at time %s\n",curr->threadinfo.username,ctime(&curtime));
                        fflush(stdout);
                        strcpy(welcome.option, "loginresponse");
                        strcpy(welcome.username,"Server");
                        strcpy(welcome.buff,ctime(&curtime));
                        sendto(sockfd, (void *)&welcome, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                    }
                }}
            if(uflag==0){
                strcpy(welcome.option, "info");
                strcpy(welcome.username,"Server");
                strcpy(welcome.buff, "User is invalid\n");
                sendto(sockfd, (void *)&welcome, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
            }
            if(uflag==1 && pflag==0){
                strcpy(welcome.option, "info");
                strcpy(welcome.username,"Server");
                strcpy(welcome.buff, "Password is invalid\n");
                sendto(sockfd, (void *)&welcome, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));            }
            pthread_mutex_unlock(&clientlist_mutex);
        }
        else if(!strcmp(packet.option, "exit")) {
            printf("[Client %d] has disconnected...\n", threadinfo.sockfd);
            fflush(stdout);
            pthread_mutex_lock(&clientlist_mutex);
            list_delete(&client_list, &threadinfo);
            pthread_mutex_unlock(&clientlist_mutex);
            break;
        }
        else if(!strcmp(packet.option, "signup")){
            pthread_mutex_lock(&clientlist_mutex);
            for(curr = client_list.head; curr != NULL; curr = curr->next){
                if(curr->threadinfo.userid== threadinfo.userid){
                    strcpy(curr->threadinfo.username, packet.username);
                    strcpy(curr->threadinfo.password, packet.password);
                    curr->threadinfo.oyunId=0;
                    break;
                }
                
            }
            userindex++;
            pthread_mutex_unlock(&clientlist_mutex);
        }
        
        else if(!strcmp(packet.option, "create")){
            char * tempWord;
            char category[10];
            struct Paket warningpacket;
            struct GAME *mygame=malloc(sizeof(struct GAME));
            
            if(loginflag==0){
                strcpy(warningpacket.username,"Server");
                strcpy(warningpacket.option, "info");
                strcpy(warningpacket.buff, "You should login first\n");
                sendto(sockfd, (void *)&warningpacket, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
            }
            else {
                pthread_mutex_lock(&clientlist_mutex);
                //create and initialize the game
                for(curr = client_list.head; curr != NULL; curr = curr->next){
                    if(curr->threadinfo.userid== threadinfo.userid){
                        curr->threadinfo.oyunId= gameIndex; //game indices are 1...19
                        strncpy(category, &packet.buff[7], 6);
                        mygame->category= malloc(strlen(category));
                        strcpy(mygame->category,category);
                        strcat(category,".txt");
                        tempWord = getWord(category);
                        mygame->wordlength=strlen(tempWord);
                        mygame->word= malloc( mygame->wordlength );
                        strcpy(mygame->word,tempWord);
                        // free(tempWord);
                        
                        mygame->numofPlayers=0;
                        mygame->isover=0;
                        mygame->orderofMove=1;
                        mygame->player2id=0;
                        mygame->player1id=0;
                        mygame->player1score=0;
                        mygame->player2score=0;
                        mygame->movecounter=0;
                        mygame->oyunid=gameIndex;
                        mygame->errors=0;
                        mygame->currentWord= malloc( mygame->wordlength );
                        for (t = 0; t <=  mygame->wordlength; t++) {
                            if (t ==  mygame->wordlength)
                                mygame->currentWord[t] = '\0';
                            else
                                mygame->currentWord[t] =  '.';          }
                        
                        for (t = 0; t < 26; t++)
                            mygame->wrongs[t] = ' ';
                        
                        for (t = 0; t < 26; t++)
                            mygame->enteredletters[t] = ' ';
                        
                        
                        
                        printf("\nGame %d is created from selected category with [%s]\n", gameIndex,tempWord);
                        fflush(stdout);
                        
                    }
                }
                
                gamelist[gameIndex]=mygame;
                gameIndex++;
                strcpy(warningpacket.option, "createresponse");
                sendto(sockfd, (void *)&warningpacket, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                pthread_mutex_unlock(&clientlist_mutex);
            }
        }
        else if(!strcmp(packet.option, "join")){
            int filled=0;
            struct Paket warningpacket;
            if(loginflag==0){
                strcpy(warningpacket.username,"Server");
                strcpy(warningpacket.option, "info");
                strcpy(warningpacket.buff, "You should login first\n");
                sendto(sockfd, (void *)&warningpacket, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));            }
            else{
                pthread_mutex_lock(&clientlist_mutex);
                int id=packet.buff[5]-48;
                for(curr = client_list.head; curr != NULL; curr = curr->next){
                    if(curr->threadinfo.userid== threadinfo.userid){
                        curr->threadinfo.oyunId= id;
                        
                        
                        threadinfo.oyunId=id;
                        t=1;
                        while (gamelist[t]->oyunid!= threadinfo.oyunId)
                            t++;
                        
                        //ilgili oyun ve user ı buldum
                        if(gamelist[t]->numofPlayers==0)//oyuna ilk katılan kşi
                        {   gamelist[t]->numofPlayers=1;
                            gamelist[t]->player1id=curr->threadinfo.userid;
                            printf("\n[Client %d] join the session [%d] as first player\n", threadinfo.userid, id);
                            fflush(stdout);
                        }
                        else if(gamelist[t]->numofPlayers==1)//oyuna 2. katılan kşi
                        {   gamelist[t]->numofPlayers=2;
                            gamelist[t]->player2id=curr->threadinfo.userid;
                            printf("\n[Client %d] join the session [%d] as second player\n", threadinfo.userid, id);
                            fflush(stdout);}
                        else {//oyuna kat isteyen 3.kişi
                            filled=1;
                            struct Paket tmppacket;
                            memset(&tmppacket, 0, sizeof(struct Paket));
                            strcpy(tmppacket.buff, "This game is not empty" );
                            strcpy(tmppacket.option, "info");
                            sendto(sockfd, (void *)&tmppacket, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                        }
                    }
                }
                
                
                t=1;
                for(curr = client_list.head; curr != NULL; curr = curr->next) {
                    if( gamelist[t]->oyunid == curr->threadinfo.oyunId){
                        struct Paket tmppacket;
                        memset(&tmppacket, 0, sizeof(struct Paket));
                        strcpy(tmppacket.option, "info");
                        strcpy(tmppacket.username, "Server");
                        if(curr->threadinfo.userid == threadinfo.userid || filled==1 ) continue;
                        
                        
                        if(gamelist[t]->numofPlayers==1){
                            strcpy(tmppacket.option, "info");
                            strcpy(tmppacket.buff, "You joined the game, wait for the second player" );
                            client_addr.sin_port=allclientadresses[ gamelist[t]->player1id ];
                            sendto(sockfd, (void *)&tmppacket, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                        }
                        else if(gamelist[t]->numofPlayers==2){
                            strcpy(tmppacket.buff, "Second player joined the game, you can start playing" );
                            client_addr.sin_port=allclientadresses[ gamelist[t]->player1id ];
                            sendto(sockfd, (void *)&tmppacket, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                            
                            strcpy(tmppacket.buff, "You joined the game, wait your opponent to move" );
                            client_addr.sin_port=allclientadresses[ gamelist[t]->player2id ];
                            sendto(sockfd, (void *)&tmppacket, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                        break;
                    }
                    t++;
                }
                pthread_mutex_unlock(&clientlist_mutex);
            }
        }
        }
        else if(!strcmp(packet.option, "showgames")){
            struct Paket warningpacket;
            if(loginflag==0){
                strcpy(warningpacket.username,"Server");
                strcpy(warningpacket.option,"info");
                strcpy(warningpacket.buff, "You should login first\n");
                sendto(sockfd, (void *)&warningpacket, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
            }
            else{
                pthread_mutex_lock(&clientlist_mutex);
                int t;
                char c;
                struct Paket showpacket;
                memset(&showpacket, 0, sizeof(struct Paket));
                printf("Game list result:\n");
                fflush(stdout);
                printallgames();
                
                
                
                for(t=1; t < gameIndex; t++){
                    strcpy(showpacket.option,"showgamesresponse");
                    strncpy(showpacket.buff,gamelist[t]->category,strlen(gamelist[t]->category));
                    showpacket.pgame.oyunid=gamelist[t]->oyunid;
                    showpacket.pgame.isover=gamelist[t]->isover;
                    showpacket.pgame.numofPlayers=gamelist[t]->numofPlayers;
                    sendto(sockfd, (void *)&showpacket, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                }
                
                if(gameIndex==1){ //hic oyun yok
                    strcpy(showpacket.option,"showgamesresponse");
                    strcpy(showpacket.buff,"There is no game to show");
                    showpacket.pgame.oyunid=-1;
                    sendto(sockfd, (void *)&showpacket, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));                }
                
                pthread_mutex_unlock(&clientlist_mutex);
            }
        }
        
        
        else if(!strcmp(packet.option, "players")){
            struct Paket warningpacket;
            if(loginflag==0){
                strcpy(warningpacket.username,"Server");
                strcpy(warningpacket.buff, "You should login first\n");
                strcpy(warningpacket.option,"info");
                sendto(sockfd, (void *)&warningpacket, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
            }
            else{
                pthread_mutex_lock(&clientlist_mutex);
                struct Paket spacket;
                memset(&spacket, 0, sizeof(struct Paket));
                strcpy(spacket.option,"playersresponse");
                
                
                int id=packet.buff[8]-48;
                if(id>0 && id<20 && id<=gameIndex){
                    if(gamelist[id]->numofPlayers==0){
                        memset(&spacket, 0, sizeof(struct Paket));
                        strcpy(spacket.buff,"There is no player in this game");
                        strcpy(spacket.username,"Server");
                        strcpy(spacket.option,"info");
                        sendto(sockfd, (void *)&spacket, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                    }
                    
                    else if(gamelist[id]->numofPlayers==1){
                        for(curr = client_list.head; curr != NULL; curr = curr->next){
                            if(curr->threadinfo.userid==gamelist[id]->player1id){
                                strncpy(spacket.buff,curr->threadinfo.username,sizeof(curr->threadinfo.username));
                                snprintf(spacket.username, sizeof(gamelist[t]->player1id), "%d", gamelist[t]->player1id);
                                sendto(sockfd, (void *)&spacket, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                                break;
                            }
                        }
                    }
                    
                    else if(gamelist[id]->numofPlayers==2){
                        for(curr = client_list.head; curr != NULL; curr = curr->next){
                            if(curr->threadinfo.userid==gamelist[id]->player1id){
                                strncpy(spacket.buff,curr->threadinfo.username,sizeof(curr->threadinfo.username));
                                snprintf(spacket.username, sizeof(gamelist[t]->player1id), "%d", gamelist[t]->player1id);
                                sendto(sockfd, (void *)&spacket, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                                break;
                            }
                        }
                        for(curr = client_list.head; curr != NULL; curr = curr->next){
                            if(curr->threadinfo.userid==gamelist[id]->player2id){
                                strncpy(spacket.buff,curr->threadinfo.username,sizeof(curr->threadinfo.username));
                                snprintf(spacket.username, sizeof(gamelist[t]->player1id), "%d", gamelist[t]->player2id);
                                    sendto(sockfd, (void *)&spacket, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));                                break;
                            }
                        }
                        
                    }
                    
                }
                else {
                    memset(&spacket, 0, sizeof(struct Paket));
                    strcpy(spacket.option,"info");
                    strcpy(spacket.buff,"You are not entered a valid game id");
                    strcpy(spacket.username,"Server");
                    sendto(sockfd, (void *)&spacket, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
            }
            
            pthread_mutex_unlock(&clientlist_mutex);
            
        }
        }
        else if(!strcmp(packet.option, "move")){
            int requestoyunId;
            int score1=0,score2=0;
            struct Paket warningpacket;
            strcpy(warningpacket.option,"info");
            strcpy(warningpacket.username,"Server");
            int flag;
            
            if(loginflag==0){
                strcpy(warningpacket.buff, "You should login first");
                sendto(sockfd, (void *)&warningpacket, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
            }
            else{
                struct MOVE mymove;
                mymove.val=0;
                mymove.guess=packet.buff[5];
                mymove.playerid=threadinfo.userid;
                
                pthread_mutex_lock(&clientlist_mutex);
                struct Paket spacket;
                memset(&spacket, 0, sizeof(struct Paket));
                strcpy(spacket.option,"info");
                strcpy(spacket.username,"Server");
                
                
                for(curr = client_list.head; curr != NULL; curr = curr->next){
                    if(curr->threadinfo.userid == threadinfo.userid)
                        requestoyunId=curr->threadinfo.oyunId;  }
                
                
                if(gamelist[requestoyunId]->player1id==threadinfo.userid){//1.player istek yollamıs
                    if(gamelist[requestoyunId]->orderofMove==1){ //hamle sırasıda 1de
                        flag=game(gamelist[requestoyunId],&mymove);
                        if(mymove.exist!=0)//hamle dogru
                        { gamelist[requestoyunId]->player1score+=mymove.val*10;
                            strcpy(warningpacket.buff, "Your guess is correct");
                            printf("\n%s%s%s%d\n","[",packet.username,"] updated her score as ",gamelist[requestoyunId]->player1score);
                            fflush(stdout);
                            
                        }
                        else{
                            strcpy(warningpacket.buff, "Your guess is NOT correct");}
                        
                            sendto(sockfd, (void *)&warningpacket, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                            gamelist[requestoyunId]->orderofMove=2;
                    }
                    else{
                        strcpy(spacket.buff,"Opps! Wait your turn (hamle sırası rakipte)");
                        strcpy(spacket.option,"info");
                        strcpy(spacket.username,"Server");
                        sendto(sockfd, (void *)&spacket, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));                    }
                }
                
                else if(gamelist[requestoyunId]->player2id==threadinfo.userid){//2.player istek yollamıs
                    if(gamelist[requestoyunId]->orderofMove==2){ //hamle sırasıda 2de
                        flag=game(gamelist[requestoyunId],&mymove);
                        if(mymove.exist!=0)//hamle dogru
                        { gamelist[requestoyunId]->player2score+=mymove.val*10;
                            strcpy(warningpacket.buff, "Your guess is correct");
                            printf("\n%s%s%s%d\n","[",packet.username,"] updated her score as ",gamelist[requestoyunId]->player2score);
                            fflush(stdout);
                        }
                        else{
                            strcpy(warningpacket.buff, "Your guess is NOT correct");}
                        
                        sendto(sockfd, (void *)&warningpacket, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                        gamelist[requestoyunId]->orderofMove=1;
                        
                    }
                    else{
                        strcpy(spacket.buff,"Opps! Wait your turn (hamle sırası rakipte)");
                        strcpy(spacket.option,"info");
                        strcpy(spacket.username,"Server");
                        sendto(sockfd, (void *)&spacket, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));                    }
                    
                }
                
                
                strcpy(spacket.option,"showhang");
                int i=0;
                int err=gamelist[requestoyunId]->errors;
                spacket.pgame.errors=err;
                while(err--){
                    spacket.pgame.wrongs[i]=gamelist[t]->wrongs[i];
                    i++;}
                
                i=0;
                int numofentereds=gamelist[requestoyunId]->movecounter;
                spacket.pgame.movecounter=numofentereds;
                while(numofentereds--){
                    spacket.pgame.enteredletters[i]=gamelist[requestoyunId]->enteredletters[i];
                    i++;}
                
                client_addr.sin_port=allclientadresses[ gamelist[requestoyunId]->player1id ];
                sendto(sockfd, (void *)&spacket, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                
                client_addr.sin_port=allclientadresses[ gamelist[requestoyunId]->player2id ];
                sendto(sockfd, (void *)&spacket, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));

              
                
                
                if(flag!=0){
                    struct Paket resultpacket;
                    struct Paket dispacket;
                    strcpy(dispacket.username,"Server");
                    strcpy(dispacket.option,"info");
                    
                    if(flag==1)
                        strcpy(dispacket.buff,"Game is over, word is found!");
                    if(flag==2)
                        strcpy(dispacket.buff,"Game is over, max error limit!");
                    
                    
                    client_addr.sin_port=allclientadresses[ gamelist[t]->player1id ];
                    sendto(sockfd, (void *)&dispacket, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                    
                    client_addr.sin_port=allclientadresses[ gamelist[t]->player2id ];
                    sendto(sockfd, (void *)&dispacket, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));

                    
                    
                    strcpy(resultpacket.option,"gameresult");
                    score1=gamelist[requestoyunId]->player1score;
                    score2=gamelist[requestoyunId]->player2score;
                    
                    
                    if(score1>score2){
                        strcpy(resultpacket.buff,"You won the game with score ");
                        resultpacket.val=score1;
                        client_addr.sin_port=allclientadresses[ gamelist[requestoyunId]->player1id ];
                        sendto(sockfd, (void *)&resultpacket, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                        
                        
                        strcpy(resultpacket.buff,"You lose the game with score ");
                        resultpacket.val=score2;
                        client_addr.sin_port=allclientadresses[ gamelist[requestoyunId]->player2id ];
                        sendto(sockfd, (void *)&resultpacket, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                    }
                    else if(score1<score2)
                    {   strcpy(resultpacket.buff,"You won the game with score ");
                        resultpacket.val=score2;
                        client_addr.sin_port=allclientadresses[ gamelist[requestoyunId]->player2id ];
                        sendto(sockfd, (void *)&resultpacket, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                        
                        
                        strcpy(resultpacket.buff,"You lose the game with score ");
                        resultpacket.val=score1;
                        client_addr.sin_port=allclientadresses[ gamelist[requestoyunId]->player1id ];
                        sendto(sockfd, (void *)&resultpacket, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                    }
                    else
                    {   strcpy(resultpacket.buff,"Berabere, score ");
                        resultpacket.val=score1;
                        client_addr.sin_port=allclientadresses[ gamelist[requestoyunId]->player1id ];
                        sendto(sockfd, (void *)&resultpacket, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                        client_addr.sin_port=allclientadresses[ gamelist[requestoyunId]->player2id ];
                        sendto(sockfd, (void *)&resultpacket, sizeof(struct Paket), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                    }
                    
                    
                    gamelist[requestoyunId]->isover=1;
                }
                
                
                pthread_mutex_unlock(&clientlist_mutex);
                
            }
            
        }
        
    }
    
   // close(threadinfo.userid);
    return NULL;
}

void printallgames(){
    int t=0;
    if(gameIndex==1)
        printf("%s","There is no game to show\n");
    else
        for(t=1; t < gameIndex; t++)
            printf("Game %d in %s category with %d players\n", gamelist[t]->oyunid, gamelist[t]->category, gamelist[t]->numofPlayers );
}





