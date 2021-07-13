#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <stdlib.h>

extern char* get_menu(char*);
void user_commands(int);
void admin_commands(int);

void send_to_server(int socket, char* choice, char* username, char* password) {
	send(socket, choice, sizeof(choice), 0); 
	send(socket, username, sizeof(username), 0); 
	send(socket, password, sizeof(password), 0); 
}

void authenticate_user(int socket){
	char* username = malloc(50 * sizeof(char));
	char* password = malloc(50 * sizeof(char));
	char* server_message = malloc(100 * sizeof(char));
	char* choice = get_menu("Start");
	if (strcmp(choice, "SIGN_UP") && strcmp(choice, "SIGN_IN")) {
		printf("Invalid Option\n");
		exit(1);
	}
	char* mode = (!strcmp(choice, "SIGN_UP") ? "Sign Up" : "Sign In");
	char* auth_mode = get_menu(mode);

	printf("Username: ");
	scanf("%s",username);
	printf("Password: ");
	scanf("%s",password);

	if (!strcmp(choice, "SIGN_UP") || !strcmp(choice, "SIGN_IN")) {
		send_to_server(socket, auth_mode, username, password);
		read(socket, server_message, 100 * sizeof(char)); 
		printf("%s\n", server_message); 
		if(!strcmp(server_message, "Authentication failed\n")) exit(1);
		while(1) {
			char type = auth_mode[strlen(auth_mode) - 1];
			if (type == 'U' || type == 'J') user_commands(socket); // user or joint account
			else if (type == 'A') admin_commands(socket);  // admin account
		}
	} 
}

void user_commands(int socket){
	char* choice = get_menu("User");
	char* option = malloc(10 * sizeof(char));
	char* amount = malloc(10 * sizeof(char));
	send(socket, choice, sizeof(option), 0); 

	if (!strcmp(choice, "EXIT")) exit(0);
	else if (!strcmp(choice, "DEPOSIT") || !strcmp(choice, "WITHDRAW")) {
		float amt;
		char* msg = !strcmp(choice, "DEPOSIT") ? "Deposit Amount" : "Withdraw Amount";
		printf("%s: ", msg);
		scanf("%f", &amt);
		sprintf(amount, "%f", amt);	
		send(socket, amount, sizeof(amount), 0);
	}
	else if (!strcmp(choice, "PASSWORD")) {
		char* password = malloc(50 * sizeof(char));
		printf("New password: ");
		scanf("%s", password);
		send(socket, password, sizeof(password), 0);
	}
	char* server_message = malloc(100*sizeof(char));
	read(socket, server_message, 100 * sizeof(char)); 
	printf("%s\n", server_message); 
}

void admin_commands(int socket){
	char* choice = get_menu("Admin");
	char* option_string = malloc(10*sizeof(char));
	char* username = malloc(50 * sizeof(char));
	char* password = malloc(50 * sizeof(char));
	send(socket, choice, sizeof(option_string), 0); 

	if (!strcmp(choice, "EXIT")) exit(0);
	else if (!strcmp(choice, "ADD_U")) {
		int user_type;
		printf("Enter User Type \n1. Normal Account \n2. Joint Account");
		scanf("%d", &user_type);
		if (user_type != 1 && user_type != 2) {
			printf("Invalid type\n");
			exit(0);
		}
		printf("Username: ");
		scanf("%s", username);
		printf("Password: ");
		scanf("%s", password);
		if (user_type == 1) send(socket, "1", sizeof(char), 0);
		else if (user_type == 2) send(socket, "2", sizeof(char), 0); 
		
		printf("Username = %s\n",username);
		printf("Password = %s\n",password);
		send(socket, username, sizeof(username), 0); 
		send(socket, password, sizeof(password), 0); 
	}
	else if (!strcmp(choice, "MODIFY_U")) {
		char* new_username = malloc(50 * sizeof(char));
		printf("Username: ");
		scanf("%s", username);
		printf("Enter new username: ");
		scanf("%s", new_username);
		printf("Enter new password: ");
		scanf("%s", password);
		send(socket, username, sizeof(username), 0); 
		send(socket, new_username, sizeof(new_username), 0); 
		send(socket, password, sizeof(password), 0); 
	} 
	else if (!strcmp(choice, "DELETE_U") || !strcmp(choice, "GET_USER")) {
		printf("Username: ");
		scanf("%s", username);
		send(socket, username, sizeof(username), 0); 
	}
	else {
		printf("Invalid input\n");
		exit(0);
	}
	char* server_message = malloc(100 * sizeof(char));
	read(socket, server_message, 100 * sizeof(char)); 
	printf("%s\n",server_message); 
}

int main(int argc, char const *argv[]) { 
	int sock = 0, valread; 
	struct sockaddr_in serv_addr; 
	char buffer[1024] = {0}; 
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
		printf("\n Socket creation error \n"); 
		return -1; 
	} 
	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(8080); 
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) { 
		printf("\nInvalid address/ Address not supported \n"); 
		return -1; 
	} 
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { 
		printf("\nConnection Failed \n"); 
		return -1; 
	} 
	authenticate_user(sock);
	return 0; 
} 

