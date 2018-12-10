#include <stdio.h>	//printf
#include <string.h>	//strlen
#include <sys/socket.h>	//socket
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>
#include <stdlib.h>


void printResponsePage(char * address, char * port, char * username, char * password, char * gamename, char * square, char * responseStringBuilder) {
	// printf("%s%c%c\n", "Content-Type:text/html;charset=iso-8859-1",13,10);
	printf("Content-Type: text/html;charset=us-ascii\n\n");
	printf("<html>\n"); 
	printf("<body>\n");
	printf("<form action=./ttt.cgi>\n");
	printf("<b>Server Address: <input type=\"text\" name=\"address\" size=\"20\" value=\"%s\"><br />\n", address);
	printf("<b>Server Port: <input type=\"text\" name=\"port\" size=\"20\" value=\"%s\"><br />\n", port);
	printf("<b>Username: <input type=\"text\" name=\"username\" size=\"20\" value=\"%s\"><br />\n", username);
	printf("<b>Password: <input type=\"text\" name=\"password\" size=\"20\" value=\"%s\"><br />\n", password);
	printf("<b>Gamename: <input type=\"text\" name=\"gamename\" size=\"20\" value=\"%s\"><br />\n", gamename);
	printf("<b>Square: <input type=\"text\" name=\"square\" size=\"20\" value=\"%s\"><br />\n", square); 
	printf("<input type=\"submit\" value=\"LOGIN\" name=\"LOGIN\">\n"); 
	printf("<input type=\"submit\" value=\"CREATE\" name=\"CREATE\">\n"); 
	printf("<input type=\"submit\" value=\"JOIN\" name=\"JOIN\">\n"); 
	printf("<input type=\"submit\" value=\"MOVE\" name=\"MOVE\">\n"); 
	printf("<input type=\"submit\" value=\"LIST\" name=\"LIST\">\n"); 
	printf("<input type=\"submit\" value=\"SHOW\" name=\"SHOW\">\n"); 
	printf("<br>%s\n", responseStringBuilder);
	printf("</form>\n"); 
	printf("</html>\n"); 
	printf("</body>\n");
}

void replaceToHTML(char * returnTarget, char * sourceReplace, char * toReplace, char * changeTo) {
	char * curr = sourceReplace; 
	char * prev = sourceReplace; 
	while ((curr = strstr(curr, toReplace)) != NULL) {
		strncat(returnTarget, prev, curr - prev); 
		strcat(returnTarget, changeTo);
		curr = curr + strlen(toReplace); 
		prev = curr;  
	}
	strcat(returnTarget, prev);
}

int main() {


	// Read query string
	char * data = getenv("QUERY_STRING");

	char address[100] = {'\0'}; 
	memset(address, '\0', 100);

	char port[100] = {'\0'}; 
	memset(port, '\0', 100);

	char username[100] = {'\0'}; 
	memset(username, '\0', 100);

	char password[100] = {'\0'}; 
	memset(password, '\0', 100);

	char gamename[100] = {'\0'}; 
	memset(gamename, '\0', 100);

	char square[100] = {'\0'}; 
	memset(square, '\0', 100);

	char command[100] = {'\0'}; 
	memset(command, '\0', 100);


	char * pAddress = strstr(data, "address=") + strlen("address="); 
	char * pPort = strstr(data, "port=") + strlen("port="); 
	char * pUserName = strstr(data, "username=") + strlen("username="); 
	char * pPassword = strstr(data, "password=") + strlen("password="); 
	char * pGameName = strstr(data, "gamename=") + strlen("gamename="); 
	char * pSquare = strstr(data, "square=") + strlen("square="); 

	int indexCount = 0; 
	while (pAddress[indexCount] != '&') {
		address[indexCount] = pAddress[indexCount]; 
		indexCount++; 
	}

	indexCount = 0; 
	while (pPort[indexCount] != '&') {
		port[indexCount] = pPort[indexCount]; 
		indexCount++; 
	}

	indexCount = 0; 
	while (pUserName[indexCount] != '&') {
		username[indexCount] = pUserName[indexCount]; 
		indexCount++; 
	}

	indexCount = 0; 
	while (pPassword[indexCount] != '&') {
		password[indexCount] = pPassword[indexCount]; 
		indexCount++; 
	}

	indexCount = 0; 
	while (pGameName[indexCount] != '&') {
		gamename[indexCount] = pGameName[indexCount]; 
		indexCount++; 
	}

	indexCount = 0; 
	while (pSquare[indexCount] != '&') {
		square[indexCount] = pSquare[indexCount]; 
		indexCount++; 
	}

	if (strstr(data, "LOGIN") != NULL) {
		strcpy(command, "LOGIN"); 
	} else if (strstr(data, "CREATE") != NULL) {
		strcpy(command, "CREATE"); 
	} else if (strstr(data, "JOIN") != NULL) {
		strcpy(command, "JOIN"); 
	} else if (strstr(data, "MOVE") != NULL) {
		strcpy(command, "MOVE"); 
	} else if (strstr(data, "LIST") != NULL) {
		strcpy(command, "LIST"); 
	} else if (strstr(data, "SHOW") != NULL) {
		strcpy(command, "SHOW"); 
	}


	char promptStringBuilder[1000] = {'\0'}; 
	char responseStringBuilder[5000] = {'\0'}; 

	if (strlen(address) < 1) memset(address, '\0', 100); 
	if (strlen(port) < 1) memset(port, '\0', 100); 
	if (strlen(username) < 1) memset(username, '\0', 100); 
	if (strlen(password) < 1) memset(password, '\0', 100); 
	if (strlen(gamename) < 1) memset(gamename, '\0', 100); 
	if (strlen(square) < 1) memset(square, '\0', 100); 
	if (strlen(command) < 1) memset(command, '\0', 100); 

	sprintf(promptStringBuilder, "%s,%s,%s", command, username, password); 

	if (strcmp(command, "CREATE") == 0 || strcmp(command, "SHOW") == 0) {
		sprintf(promptStringBuilder + strlen(promptStringBuilder), ",%s", gamename); 
	} else if (strcmp(command, "JOIN") == 0 || strcmp(command, "MOVE") == 0) {
		sprintf(promptStringBuilder + strlen(promptStringBuilder), ",%s,%s", gamename, square); 
	}


	// Client

	// new Socket(internet, stream, 0); 
	int clientSocket = socket(AF_INET , SOCK_STREAM , 0); 
	if (clientSocket == -1) {
		sprintf(responseStringBuilder, "Could not create socket");
		printResponsePage(address, port, username, password, gamename, square, responseStringBuilder); 
		return -1; 
	}


	struct sockaddr_in server;

	server.sin_addr.s_addr = inet_addr(address);
	server.sin_family = AF_INET;
	server.sin_port = htons((unsigned short int)atoi(port));

	if (connect(clientSocket, (struct sockaddr *)&server, sizeof(server)) < 0) {
		sprintf(responseStringBuilder, "connect failed. Error");
		printResponsePage(address, port, username, password, gamename, square, responseStringBuilder); 
		return -1; 
	}

	if (send(clientSocket, promptStringBuilder, 1000, 0) < 0) {
		sprintf(responseStringBuilder, "Send failed");
		printResponsePage(address, port, username, password, gamename, square, responseStringBuilder);
		return 1;
	}

	char temp[5000] = {'\0'};
	size_t currentReadLength; 
	int readCount; 
	while (readCount < 5000) {
		currentReadLength = recv(clientSocket, temp, 5000, 0);
		if(currentReadLength < 1){
		    puts("Server disconnected");
		    fflush(stdout);
		    close(clientSocket);
		    return 0;
		}
		memcpy(responseStringBuilder + readCount, temp, currentReadLength);
		readCount = readCount + currentReadLength;
	}

	close(clientSocket);
	char * responseStringReplacedHTML = (char *)calloc(5000, (sizeof(char))); 
	memset(responseStringReplacedHTML, '\0', 5000); 
	replaceToHTML(responseStringReplacedHTML, responseStringBuilder, " ", "&ensp;");   
	char * responseStringReplacedHTML1 = (char *)calloc(5000, (sizeof(char))); 
	memset(responseStringReplacedHTML1, '\0', 5000); 
	replaceToHTML(responseStringReplacedHTML1, responseStringReplacedHTML, "\r\n", "<br>");
	char * responseStringReplacedHTML2 = (char *)calloc(5000, (sizeof(char))); 
	memset(responseStringReplacedHTML2, '\0', 5000); 
	replaceToHTML(responseStringReplacedHTML2, responseStringReplacedHTML1, "-", "&ndash;");
	printResponsePage(address, port, username, password, gamename, square, responseStringReplacedHTML2);
	free(responseStringReplacedHTML); 
	free(responseStringReplacedHTML1); 
	free(responseStringReplacedHTML2); 
	return 0;  
}
