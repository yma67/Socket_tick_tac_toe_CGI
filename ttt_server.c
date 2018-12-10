/*********************************************************
* FILE: ttt_server.c
* AUTHOR: Yuxiang Ma
***********************************************************/

#include <stdio.h>
#include <string.h>	
#include <sys/socket.h>
#include <arpa/inet.h>	
#include <unistd.h>	
#include <stdlib.h>
#include <time.h>

struct USER{
	char username[100];
	char password[100];
	struct USER *next;
};

enum GAME_STATE{ 
	CREATOR_WON=-2,
	IN_PROGRESS_CREATOR_NEXT=-1,
	DRAW=0,
	IN_PROGRESS_CHALLENGER_NEXT=1,
	CHALLENGER_WON=2
};

struct GAME{
	char gamename[100];
	struct USER *creator;
	struct USER *challenger;
	enum GAME_STATE state;
	char ttt[3][3];
	struct GAME *next;
};

struct USER *user_list_head = NULL;
struct GAME *game_list_head = NULL;


int checkPassword(struct USER * userPersistence, char * username, char * password) {
    struct USER * current = userPersistence;
    while (current != NULL) {
        if (strcmp(current->username, username) == 0) {
            if (strcmp(current->password, password) == 0) {
                // good
                return 1;
            } else {
                // bad
                return 0;
            }
        }
        current = current->next;
    }
    // not registered
    return -1;
}

struct USER * getUserByName(struct USER * userPersistence, char * username) {
    struct USER * current = userPersistence;
    while (current != NULL) {
        if (strcmp(current->username, username) == 0) {
            return current;
        }
        current = current->next;
    }
    // not registered
    return NULL;
}

struct GAME * getGameByName(struct GAME * gamePersistence, char * gamename) {
    struct GAME * current = gamePersistence;
    while (current != NULL) {
        if (strcmp(current->gamename, gamename) == 0) {
            return current;
        }
        current = current->next;
    }
    // not registered
    return NULL;
}

int hasWon(char ttt[3][3], char label) {
    if ((ttt[0][0] == label && ttt[0][1] == label && ttt[0][2] == label) ||
        (ttt[1][0] == label && ttt[1][1] == label && ttt[1][2] == label) ||
        (ttt[2][0] == label && ttt[2][1] == label && ttt[2][2] == label) ||
        (ttt[0][0] == label && ttt[1][0] == label && ttt[2][0] == label) ||
        (ttt[0][1] == label && ttt[1][1] == label && ttt[2][1] == label) ||
        (ttt[0][2] == label && ttt[1][2] == label && ttt[2][2] == label) ||
        (ttt[0][0] == label && ttt[1][1] == label && ttt[2][2] == label) ||
        (ttt[0][2] == label && ttt[1][1] == label && ttt[2][0] == label)) {
        return 1;
    }
    return 0;
}

int isOverWhelmed(char ttt[3][3]) {
    if (ttt[0][0] == ' ' || ttt[0][1] == ' ' || ttt[0][2] == ' ' ||
        ttt[1][0] == ' ' || ttt[1][1] == ' ' || ttt[1][2] == ' ' ||
        ttt[2][0] == ' ' || ttt[2][1] == ' ' || ttt[2][2] == ' ') {
        return 0;
    }
    return 1;
}

int main(int argc , char *argv[])
{
	int socket_desc , client_sock , c , read_size;
	struct sockaddr_in server , client;
	char client_message[2000];

	unsigned short int port = 8888;

	if( argc > 1 )
		port = (unsigned short int)atoi(argv[1]);
	
	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}
	
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( port );
	
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("bind failed. Error");
		return 1;
	}

	listen(socket_desc , 3);

	printf( "Game server ready on port %d.\n", port );

	while( 1 ){
		c = sizeof(struct sockaddr_in);

		//accept connection from an incoming client
		client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
		if (client_sock < 0)
		{
			perror("accept failed");
			return 1;
		}

		char temp[200];
		memset(client_message, '\0', 200);
		int bytes_read = 0;
		while( bytes_read < 200 ){
			read_size = recv(client_sock , temp , 200, 0);
			if(read_size <= 0){
				puts("Client disconnected");
				fflush(stdout);
				close(client_sock);
				close(socket_desc);
				return 0;
			}
			memcpy( client_message+bytes_read, temp, read_size );
			bytes_read += read_size;
		}

	  	char response[5000];
	  	response[0] = '\0';
	  	char* command = strtok( client_message, "," );
	  	char *username = strtok( NULL, "," );
	  	char *password = strtok( NULL, ",");

		if( command == NULL || username == NULL || password == NULL ){
			sprintf( response, "MUST ENTER A VALID COMMAND WITH ARGUMENTS FROM THE LIST:\r\n" );
			sprintf( response+strlen(response), "LOGIN,USER,PASS\r\n" );
			sprintf( response+strlen(response), "CREATE,USER,PASS,GAMENAME\r\n" );
			sprintf( response+strlen(response), "JOIN,USER,PASS,GAMENAME,SQUARE\r\n" );
			sprintf( response+strlen(response), "MOVE,USER,PASS,GAMENAME,SQUARE\r\n" );
			sprintf( response+strlen(response), "LIST,USER,PASS\r\n" );
			sprintf( response+strlen(response), "SHOW,USER,PASS,GAMENAME\r\n" );
			write(client_sock , response , 5000);  
		  	close(client_sock);
			continue;
		}
        
        // Check identity before anything
        int resultChecked = checkPassword(user_list_head, username, password);
        if (resultChecked == 0) {
            strcpy( response, "BAD PASSWORD" );
            write(client_sock , response , 5000);
            close(client_sock);
            continue;
        } else if ((resultChecked == -1) && (strcmp( command, "LOGIN" ) != 0)) {
            strcpy( response, "USER NOT FOUND" );
            write(client_sock , response , 5000);
            close(client_sock);
            continue;
        }
        
		if( strcmp( command, "LOGIN" ) == 0 ){
            
            if (resultChecked == 1) {
                
                strcpy( response, "EXISTING USER LOGIN OK" );
                
            } else if (resultChecked == -1) {
                
                struct USER * pNewUser = (struct USER *)malloc(sizeof(struct USER));
                strcpy(pNewUser->username, username);
                strcpy(pNewUser->password, password);
                
                pNewUser->next = user_list_head;
                user_list_head = pNewUser;
                
                strcpy( response, "NEW USER CREATED OK" );
                
            }
            
	  	} else if( strcmp( command, "CREATE" ) == 0 ){
	  		char *game_name = strtok( NULL, ",");

			if( (game_name) == NULL ){
                
				sprintf( response, "CREATE COMMAND MUST BE CALLED AS FOLLOWS:\r\n" );
				sprintf( response+strlen(response), "CREATE,USER,PASS,GAMENAME\r\n" );
                
				write(client_sock , response , 5000);  
		  		close(client_sock);
                
		  		continue;
			}
            
            struct USER *loggedUser = getUserByName(user_list_head, username);
            
			struct GAME *game = (struct GAME*)malloc( sizeof(struct GAME) );
			strcpy( game->gamename, game_name );
            game->creator = loggedUser;
            game->state = IN_PROGRESS_CHALLENGER_NEXT;
            
			for( int row=0; row<3; row++ )
				for( int col=0; col<3; col++ )
					game->ttt[row][col] = ' ';
            
            game->next = game_list_head;
            game_list_head = game;
            
			sprintf( response, "GAME %s CREATED. WAITING FOR OPPONENT TO JOIN.\r\n", game->gamename);
			sprintf( response, "%sa  %c | %c | %c \r\n",response,  game->ttt[0][0],  game->ttt[0][1],  game->ttt[0][2]);
			sprintf( response, "%s  ---|---|---\r\n", response );
			sprintf( response, "%sb  %c | %c | %c \r\n", response, game->ttt[1][0],  game->ttt[1][1],  game->ttt[1][2]);
			sprintf( response, "%s  ---|---|---\r\n", response );
			sprintf( response, "%sc  %c | %c | %c \r\n", response, game->ttt[2][0],  game->ttt[2][1],  game->ttt[2][2]);
			sprintf( response, "%s\r\n", response );
			sprintf( response, "%s   %c   %c   %c\r\n", response, '1', '2', '3' );
            
		} else if( strcmp( command, "JOIN" ) == 0 ){
            
            char * game_name = strtok( NULL, ",");
            
            char * moveStep = strtok( NULL, ",");
            
            if( (game_name) == NULL || (moveStep) == NULL) {
                
                sprintf( response, "JOIN COMMAND MUST BE CALLED AS FOLLOWS:\r\n" );
                sprintf( response+strlen(response), "JOIN,USER,PASS,GAMENAME,SQUARE\r\n" );
                
                write(client_sock , response , 5000);
                close(client_sock);
                
                continue;
                
            }
            
            
            if (moveStep[1] < '1' || moveStep[1] > '3') {
                
                sprintf(response, "INVALID MOVE %s. COL MUST BE 1-3\r\n", moveStep);
                
                write(client_sock , response , 5000);
                close(client_sock);
                
                continue;
                
            }
            
            if (moveStep[0] < 'a' || moveStep[0] > 'c') {
                
                sprintf(response, "INVALID MOVE %s. ROW MUST BE a-c\r\n", moveStep);
                
                write(client_sock , response , 5000);
                close(client_sock);
                
                continue;
            }
            
            struct GAME * chosenGame;
            if ((chosenGame = getGameByName(game_list_head, game_name)) == NULL) {
                
                sprintf(response, "GAME %s DOES NOT EXIST\r\n", game_name);
                
                write(client_sock , response , 5000);
                close(client_sock);
                
                continue;
                
            }
            
            if (chosenGame->challenger != NULL) {
                
                sprintf(response, "GAME %s ALREADY HAS A CHALLENGER\r\n", game_name);
                
                write(client_sock , response , 5000);
                close(client_sock);
                
                continue;
                
            }
            
            chosenGame->challenger = getUserByName(user_list_head, username);
            
            chosenGame->ttt[moveStep[0] - 'a'][moveStep[1] - '1'] = 'x';
            
            chosenGame->state = IN_PROGRESS_CREATOR_NEXT;
            
            sprintf( response, "GAME %s BETWEEN %s AND %s.\r\n", game_name, chosenGame->creator->username, chosenGame->challenger->username);
            
            sprintf( response, "%sIN PROGRESS: %s TO MOVE NEXT AS o\r\n", response, chosenGame->creator->username);
            
            sprintf( response, "%sa  %c | %c | %c \r\n",response,  chosenGame->ttt[0][0],  chosenGame->ttt[0][1],  chosenGame->ttt[0][2]);
            sprintf( response, "%s  ---|---|---\r\n", response );
            sprintf( response, "%sb  %c | %c | %c \r\n", response, chosenGame->ttt[1][0],  chosenGame->ttt[1][1],  chosenGame->ttt[1][2]);
            sprintf( response, "%s  ---|---|---\r\n", response );
            sprintf( response, "%sc  %c | %c | %c \r\n", response, chosenGame->ttt[2][0],  chosenGame->ttt[2][1],  chosenGame->ttt[2][2]);
            sprintf( response, "%s\r\n", response );
            sprintf( response, "%s   %c   %c   %c\r\n", response, '1', '2', '3' );
            
		} else if( strcmp( command, "MOVE" ) == 0 ){
			
            char * game_name = strtok( NULL, ",");
            
            char * moveStep = strtok( NULL, ",");
            
            if( (game_name) == NULL || (moveStep) == NULL) {
                
                sprintf( response, "MOVE COMMAND MUST BE CALLED AS FOLLOWS:\r\n" );
                sprintf( response+strlen(response), "MOVE,USER,PASS,GAMENAME,SQUARE\r\n" );
                
                write(client_sock , response , 5000);
                close(client_sock);
                
                continue;
                
            }
            
            
            if (moveStep[1] < '1' || moveStep[1] > '3') {
                
                sprintf(response, "INVALID MOVE %s. COL MUST BE 1-3\r\n", moveStep);
                
                write(client_sock , response , 5000);
                close(client_sock);
                
                continue;
                
            }
            
            if (moveStep[0] < 'a' || moveStep[0] > 'c') {
                
                sprintf(response, "INVALID MOVE %s. ROW MUST BE a-c\r\n", moveStep);
                
                write(client_sock , response , 5000);
                close(client_sock);
                
                continue;
            }
            
            struct GAME * chosenGame;
            if ((chosenGame = getGameByName(game_list_head, game_name)) == NULL) {
                
                sprintf(response, "GAME %s DOES NOT EXIST\r\n", game_name);
                
                write(client_sock , response , 5000);
                close(client_sock);
                
                continue;
                
            }
            
            if (chosenGame->state == IN_PROGRESS_CREATOR_NEXT) {
                if (chosenGame->creator != getUserByName(user_list_head, username)) {
                    if (chosenGame->challenger == NULL) {
                        sprintf(response, "INVALID USER. ONLY (null) CAN MAKE THE NEXT MOVE AS o IN GAME %s\r\n", game_name);
                    } else {
                        sprintf(response, "INVALID USER. ONLY %s CAN MAKE THE NEXT MOVE AS o IN GAME %s\r\n", chosenGame->creator->username, game_name);
                    }
                    write(client_sock , response , 5000);
                    close(client_sock);
                    continue;
                }
            } else if (chosenGame->state == IN_PROGRESS_CHALLENGER_NEXT) {
                if (chosenGame->challenger != getUserByName(user_list_head, username)) {
                    if (chosenGame->challenger == NULL) {
                        sprintf(response, "INVALID USER. ONLY (null) CAN MAKE THE NEXT MOVE AS x IN GAME %s\r\n", game_name);
                    } else {
                        sprintf(response, "INVALID USER. ONLY %s CAN MAKE THE NEXT MOVE AS x IN GAME %s\r\n", chosenGame->challenger->username, game_name);
                    }
                    write(client_sock , response , 5000);
                    close(client_sock);
                    continue;
                }
            } else if (chosenGame->state == DRAW || chosenGame->state == CREATOR_WON || chosenGame->state == CHALLENGER_WON) {
                sprintf(response, "CANNOT MAKE A MOVE IN COMPLETED GAME %s\r\n", game_name);
                
                write(client_sock , response , 5000);
                close(client_sock);
                
                continue;
            }
            
            if (chosenGame->ttt[moveStep[0] - 'a'][moveStep[1] - '1'] != ' ') {
                
                sprintf(response, "INVALID MOVE, SQUARE NOT EMPTY\r\n");
                
                write(client_sock , response , 5000);
                close(client_sock);
                
                continue;
                
            }
            
            
            if (chosenGame->state == IN_PROGRESS_CREATOR_NEXT) {
                chosenGame->ttt[moveStep[0] - 'a'][moveStep[1] - '1'] = 'o';
            } else if (chosenGame->state == IN_PROGRESS_CHALLENGER_NEXT) {
                chosenGame->ttt[moveStep[0] - 'a'][moveStep[1] - '1'] = 'x';
            }
            
            
            
            sprintf(response, "GAME %s BETWEEN %s AND %s.\r\n", game_name, chosenGame->creator->username, chosenGame->challenger->username);
            
            if (chosenGame->state == IN_PROGRESS_CREATOR_NEXT) {
                if (hasWon(chosenGame->ttt, 'o')) {
                    chosenGame->state = CREATOR_WON;
                    sprintf(response, "GAME OVER: %s WON\r\n", chosenGame->creator->username);
                } else if (isOverWhelmed(chosenGame->ttt)) {
                    chosenGame->state = DRAW;
                    sprintf(response, "GAME OVER: DRAW\r\n");
                } else {
                    chosenGame->state = IN_PROGRESS_CHALLENGER_NEXT;
                    sprintf(response, "IN PROGRESS: %s TO MOVE NEXT AS x\r\n", chosenGame->challenger->username);
                }
            } else if (chosenGame->state == IN_PROGRESS_CHALLENGER_NEXT) {
                if (hasWon(chosenGame->ttt, 'x')) {
                    chosenGame->state = CHALLENGER_WON;
                    sprintf(response, "GAME OVER: %s WON\r\n", chosenGame->challenger->username);
                } else if (isOverWhelmed(chosenGame->ttt)) {
                    chosenGame->state = DRAW;
                    sprintf(response, "GAME OVER: DRAW\r\n");
                } else {
                    chosenGame->state = IN_PROGRESS_CREATOR_NEXT;
                    sprintf(response, "IN PROGRESS: %s TO MOVE NEXT AS o\r\n", chosenGame->creator->username);
                }
            }
            
            sprintf( response, "%sa  %c | %c | %c \r\n",response,  chosenGame->ttt[0][0],  chosenGame->ttt[0][1],  chosenGame->ttt[0][2]);
            sprintf( response, "%s  ---|---|---\r\n", response );
            sprintf( response, "%sb  %c | %c | %c \r\n", response, chosenGame->ttt[1][0],  chosenGame->ttt[1][1],  chosenGame->ttt[1][2]);
            sprintf( response, "%s  ---|---|---\r\n", response );
            sprintf( response, "%sc  %c | %c | %c \r\n", response, chosenGame->ttt[2][0],  chosenGame->ttt[2][1],  chosenGame->ttt[2][2]);
            sprintf( response, "%s\r\n", response );
            sprintf( response, "%s   %c   %c   %c\r\n", response, '1', '2', '3' );
            
		} else if( strcmp( command, "LIST" ) == 0 ){
			sprintf(response, "LIST OF GAMES:\r\n");
            struct GAME * current = game_list_head;
            while (current != NULL) {
                char statustr[500];
                if (current->state == CREATOR_WON) {
                    sprintf(statustr, "GAME OVER: %s WON", current->creator->username);
                } else if (current->state == IN_PROGRESS_CREATOR_NEXT) {
                    sprintf(statustr, "IN PROGRESS: %s TO MOVE NEXT AS o", current->creator->username);
                } else if (current->state == DRAW) {
                    sprintf(statustr, "GAME OVER: DRAW");
                } else if (current->state == IN_PROGRESS_CHALLENGER_NEXT) {
                    if (current->challenger == NULL) {
                        sprintf(statustr, "IN PROGRESS: (null) TO MOVE NEXT AS x");
                    } else {
                        sprintf(statustr, "IN PROGRESS: %s TO MOVE NEXT AS x", current->challenger->username);
                    }
                } else if (current->state == CHALLENGER_WON) {
                    sprintf(statustr, "GAME OVER: %s WON", current->challenger->username);
                }
                sprintf(response, "%sGAME %s: CREATED BY %s, CHALLENGED BY: %s. %s\r\n", response, current->gamename, current->creator->username, current->challenger->username, statustr);
                current = current->next;
            }
		} else if( strcmp( command, "SHOW" ) == 0 ){

            char * game_name = strtok( NULL, ",");
            if( (game_name) == NULL) {
                
                sprintf( response, "SHOW COMMAND MUST BE CALLED AS FOLLOWS:\r\n" );
                sprintf( response+strlen(response), "MOVE,USER,PASS,GAMENAME\r\n" );
                
                write(client_sock , response , 5000);
                close(client_sock);
                
                continue;
                
            }

            struct GAME * chosenGame;
            chosenGame = getGameByName(game_list_head, game_name); 
            if ((chosenGame) == NULL) {
                
                sprintf(response, "GAME %s DOES NOT EXIST\r\n", game_name);
                
                write(client_sock , response , 5000);
                close(client_sock);
                
                continue;
                
            }

            if (chosenGame->challenger == NULL) {

                sprintf( response, "GAME %s BETWEEN %s AND (null).\r\n", game_name, chosenGame->creator->username);

            } else {

                sprintf( response, "GAME %s BETWEEN %s AND %s.\r\n", game_name, chosenGame->creator->username, chosenGame->challenger->username);

            }



            if (chosenGame->state == CREATOR_WON) {

                sprintf(response, "%sGAME OVER: %s WON\r\n", response, chosenGame->creator->username);

            } else if (chosenGame->state == IN_PROGRESS_CREATOR_NEXT) {

                sprintf(response, "%sIN PROGRESS: %s TO MOVE NEXT AS o\r\n", response, chosenGame->creator->username);

            } else if (chosenGame->state == DRAW) {

                sprintf(response, "%sGAME OVER: DRAW\r\n", response);

            } else if (chosenGame->state == IN_PROGRESS_CHALLENGER_NEXT) {

                if (chosenGame->challenger == NULL) {

                    sprintf(response, "%sIN PROGRESS: (null) TO MOVE NEXT AS x\r\n", response);

                } else {

                    sprintf(response, "%sIN PROGRESS: %s TO MOVE NEXT AS x\r\n", response, chosenGame->challenger->username);
                
                }

            } else if (chosenGame->state == CHALLENGER_WON) {

                sprintf(response, "%sGAME OVER: %s WON\r\n",response, chosenGame->challenger->username);

            }



            sprintf( response, "%sa  %c | %c | %c \r\n",response,  chosenGame->ttt[0][0],  chosenGame->ttt[0][1],  chosenGame->ttt[0][2]);
            sprintf( response, "%s  ---|---|---\r\n", response );
            sprintf( response, "%sb  %c | %c | %c \r\n", response, chosenGame->ttt[1][0],  chosenGame->ttt[1][1],  chosenGame->ttt[1][2]);
            sprintf( response, "%s  ---|---|---\r\n", response );
            sprintf( response, "%sc  %c | %c | %c \r\n", response, chosenGame->ttt[2][0],  chosenGame->ttt[2][1],  chosenGame->ttt[2][2]);
            sprintf( response, "%s\r\n", response );
            sprintf( response, "%s   %c   %c   %c\r\n", response, '1', '2', '3' );
            
		} else{
	  		sprintf( response, "COMMAND %s NOT IMPLEMENTED", command );
		}

		write(client_sock , response , 5000);  
		close(client_sock);
	}

	close(socket_desc);	
	
	return 0;
}

