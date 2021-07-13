#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <pthread.h>
#include <fcntl.h>

struct user_info {
    int user_id;
    char type[20];
    char username[20];
    char password[20];
};

//struct details
struct account_info {
    float balance;
    int user_id;
};

int global_user_id = 0;

int signup(char*, char*, char*); 
void* backend(void*);

int delete_user(char* username){
    static struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;
    lock.l_pid = getpid();

    char filename[100];
    strcpy(filename, username);
    strncat(filename, ".txt", 5 * sizeof(char));
    int fd = open(filename, O_RDWR, 0644);

    if(fd == -1) {
        perror("Error"); 
        return -1;
    }
    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("Error during file locking"); 
        return -1;
    }  
    return unlink(filename);
}

int modify_user(char* username, char* new_username, char* password) {
    struct user_info user;
    static struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_whence = SEEK_SET;
    lock.l_pid = getpid();

    char filename[100];
    strcpy(filename, username);
    strncat(filename, ".txt", 5 * sizeof(char));
    int fd = open(filename, O_RDWR, 0644);

    char* mode = malloc(10 * sizeof(char));
    if(fd == -1) {
        perror("Error"); 
        return -1;
    }
    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("Error during file locking"); 
        return -1;
    }    
    // start of critical section
    lseek(fd, 0, SEEK_SET);
    if (read(fd, &user, sizeof(struct user_info)) == -1) { 
        perror("Error during read"); 
        return -1; 
    }
    delete_user(username);
    mode = !strcmp(user.type,"normal") ? "SIGNUP_U" : "SIGNUP_J";
    strcpy(user.username, new_username);
    signup(mode, new_username, password);
    // end of critical section
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &lock);
    close(fd);
    return 0;
}

int check_user_type(char* user_type, char* mode) {
    if((strcmp(user_type,"normal") && !strcmp(mode, "SIGNIN_U")) 
        || (strcmp(user_type,"joint") && !strcmp(mode, "SIGNIN_J")) 
        || (strcmp(user_type,"admin") && !strcmp(mode, "SIGNIN_A")))    
            return -1;
    return 0;
}

int signup(char* mode, char* username, char* password){
    struct user_info user;
    struct account_info account;
    char filename[100];
    strcpy(filename, username);
    strncat(filename, ".txt", 5 * sizeof(char));
    int fd = open(filename, O_RDWR, 0644);

    if(fd != -1) {  // File (username) already exists
        perror("Error"); 
        return -1;
    }
    else close(fd);
    fd = open(filename, O_WRONLY | O_CREAT,0644);
    if(fd == -1) {
        perror("Error"); 
        return -1;
    }
    user.user_id = global_user_id++;
    strcpy(user.username, username);
    strcpy(user.password, password);
    if (!strcmp(mode, "SIGNUP_U") || !strcmp(mode, "ADD_U")) strcpy(user.type,"normal");
    else if (!strcmp(mode, "SIGNUP_J")) strcpy(user.type,"joint");
    else if (!strcmp(mode, "SIGNUP_A")) strcpy(user.type,"admin");
    write(fd, &user, sizeof(struct user_info));
  
    account.balance = 0.0;
    account.user_id = user.user_id;
    write(fd, &account, sizeof(struct account_info));
    close(fd);
    return 0;
}

int signin(char* option, char* username, char* password){
    struct user_info user;
    static struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(struct user_info);
    lock.l_pid = getpid();

    char filename[100];
    strcpy(filename, username);
    strncat(filename, ".txt", 5 * sizeof(char));
    int fd = open(filename, O_RDWR, 0644);
    if(fd == -1) {
        perror("Error"); 
        return -1;
    }
    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("Error during file locking"); 
        return -1;
    }
    // start of critical section
    lseek(fd, 0, SEEK_SET);
    read(fd, &user, sizeof(struct user_info));

    if (check_user_type(user.type, option) == -1) return -1;
    if (strcmp(user.password, password)) return -1; // Incorrect pAssword 
    // end of critical section
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &lock);
    close(fd);
    return 0;
}

int deposit(char* username, float amount){
    struct account_info account;
    static struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_start = sizeof(struct user_info);
    lock.l_len = sizeof(struct account_info);
    lock.l_whence = SEEK_SET;
    lock.l_pid = getpid();

    char filename[100];
    strcpy(filename, username);
    strncat(filename, ".txt", 5 * sizeof(char));
    int fd = open(filename, O_RDWR, 0644);
    if(fd == -1) {
        perror("Error"); 
        return -1;
    }
    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("Error during file locking"); 
        return -1;
    }
    // start of critical section
    lseek(fd, sizeof(struct user_info), SEEK_SET);
    if (read(fd, &account, sizeof(struct account_info)) == -1) {
        perror("Error during read"); 
        return -1;
    }
    printf("Balance = %f\n", account.balance);
    if (amount > 5000000) {
        printf("Cannot deposit more than 50L in one day\n");
        return -1;
    }
    account.balance += amount;
    lseek(fd, sizeof(struct user_info), SEEK_SET);
    if (write(fd, &account, sizeof(struct account_info)) == -1) {
        perror("Error during write"); 
        return -1;
    }
    // end of critical section
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &lock);
    close(fd);
    return 0;
}

int withdraw(char* username, float amount){
    struct account_info account;
    static struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_start = sizeof(struct user_info);
    lock.l_len = sizeof(struct account_info);
    lock.l_whence = SEEK_SET;
    lock.l_pid = getpid();

    char filename[100];
    strcpy(filename, username);
    strncat(filename, ".txt", 5 * sizeof(char));
    int fd = open(filename, O_RDWR, 0644);
    if(fd == -1) {
        perror("Error"); 
        return -1;
    }
    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("Error during file locking"); 
        return -1;
    }
    // start of critical section
    lseek(fd, sizeof(struct user_info), SEEK_SET);
    if (read(fd, &account, sizeof(struct account_info)) == -1) {
        perror("Error during read"); 
        return -1;
    }
    printf("Balance = %f\n", account.balance);
    if (account.balance < amount) return -1;
    account.balance -= amount;

    lseek(fd, sizeof(struct user_info), SEEK_SET);
    if (write(fd, &account, sizeof(struct account_info)) == -1) {
        perror("Error during write"); 
        return -1;
    }
    // end of critical section
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &lock);
    close(fd);
    return 0;
}

float balance(char* username){
    struct account_info account;
    static struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_start = sizeof(struct user_info);
    lock.l_len = sizeof(struct account_info);
    lock.l_whence = SEEK_SET;
    lock.l_pid = getpid();

    char filename[10];
    strcpy(filename,username);
    strncat(filename, ".txt", 5 * sizeof(char));
    int fd = open(filename, O_RDONLY, 0644);
    if(fd == -1) {
        perror("Error"); 
        return -1;
    }
    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("Error during file locking"); 
        return -1;
    }
    // start of critical section
    lseek(fd, sizeof(struct user_info), SEEK_SET);
    if (read(fd, &account, sizeof(struct account_info)) == -1) {
        perror("Error during read");
        return -1;
    }
    // end of critical section
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &lock);
    close(fd);
    return account.balance;
}

int change_password(char* username, char* password){
    struct user_info user;
    char filename[100];

    static struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(struct user_info);
    lock.l_pid = getpid();

    strcpy(filename, username);
    strncat(filename, ".txt", 5 * sizeof(char));
    int fd = open(filename, O_RDWR, 0644);
    if(fd == -1) {
        perror("Error"); 
        return -1;
    }
    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("Error during file locking"); 
        return -1;
    }
    // start of critical section
    lseek(fd, 0, SEEK_SET);
    if (read(fd, &user, sizeof(struct user_info)) == -1) {
        perror("Error during read"); 
        return -1;
    }
    strcpy(user.password, password);
    lseek(fd, 0, SEEK_SET);
    if (write(fd, &user, sizeof(struct user_info)) == -1) {
        perror("Error during write"); 
        return -1;
    }
    // end of critical section
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &lock);
    close(fd);
    return 0;
}

char* get_details(char* username){
    struct account_info account;
    struct user_info user;

    static struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;
    lock.l_pid = getpid();

    char filename[100];
    strcpy(filename, username);
    strncat(filename, ".txt", 5 * sizeof(char));
    int fd = open(filename, O_RDWR, 0644);
    if(fd == -1) {
        perror("Error"); 
        return "User does not exist\n";
    }
    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("Error during file locking"); 
        return "Error: File is locked";
    }
    // start of critical section
    lseek(fd, 0, SEEK_SET);
    if (read(fd, &user, sizeof(struct user_info)) == -1 || read(fd, &account, sizeof(struct account_info))==-1) {
        perror("Error during read"); 
        return "Unable to read file\n";
    }
    // end of critical section
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &lock);
    close(fd);
    char* return_string = (char*) malloc(100 * sizeof(char));
    sprintf(return_string, "Username: %s\nPassword: %s\nAccount Type: %s\nBalance: %f\n",
        user.username, user.password, user.type, account.balance);
    return return_string;
}

int main(int argc, char const *argv[]) { 
	int server_fd, new_socket, valread; 
	struct sockaddr_in address; 
	int opt = 1; 
	int addrlen = sizeof(address); 
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) { 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) { 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons(8080); 
	
	// Attaching socket to the port 8080 
	if (bind(server_fd, (struct sockaddr*) &address, sizeof(address))<0) { 
		perror("Could not bind socket to port."); 
		exit(EXIT_FAILURE); 
	} 
	if (listen(server_fd, 3) < 0) { 
		perror("Error"); 
		exit(EXIT_FAILURE); 
	} 
    while(1) {
        if ((new_socket = accept(server_fd, (struct sockaddr*) &address, (socklen_t*) &addrlen)) < 0) { 
            perror("Error"); 
            exit(EXIT_FAILURE); 
        } 
        pthread_t thread_id;
        if(pthread_create(&thread_id, NULL, backend, (void*) &new_socket) < 0) {
            perror("Error during thread creation");
            return 1;
        }
        puts("Listening...");
    }
	return 0; 
} 

int is_authentication(char* choice) {
    if (!strcmp(choice, "SIGNUP_U") || !strcmp(choice, "SIGNUP_J") || !strcmp(choice, "SIGNUP_A")) return 1;
    if (!strcmp(choice, "SIGNIN_U") || !strcmp(choice, "SIGNIN_J") || !strcmp(choice, "SIGNIN_A")) return 2;
    return 0;
}

void *backend(void *socket_desc) {
	int socket = *(int*)socket_desc;
    int option, output;
	char* username = malloc(50 * sizeof(char));
	char* password = malloc(50 * sizeof(char));

	while(1){
		char* type = malloc(50 * sizeof(char));
		char* msg = malloc(100 * sizeof(char));
		char* choice = malloc(50*sizeof(char));
		read(socket, choice, sizeof(choice)); 

        if (is_authentication(choice) > 0) {
			read(socket, username, sizeof(username));
			read(socket, password, sizeof(password));
            int val = is_authentication(choice);
            int (*auth)(char*, char*, char*) = (val == 1) ? &signup : &signin; // 1 -> signup, 2-> signin
			int output = auth(choice, username, password);
			if(output == -1) msg = "Authentication failed\n";
			else msg = "User authenticated successfully!\n";
		}

        // User commands
		else if (!strcmp(choice, "DEPOSIT") || !strcmp(choice, "WITHDRAW")) {
	    	char* amount = malloc(10 * sizeof(char));
			read(socket, amount, sizeof(amount));
            int is_deposit = !strcmp(choice, "DEPOSIT") ? 1 : 0;
            int (*action)(char*, float) = is_deposit ? &deposit : &withdraw;
            output = (*action)(username, atof(amount));
			if (output == -1) 
                msg = (is_deposit) ? "Cannot deposit more than 50L!\n" : "Not enough bank balance!\n";
            else msg = (is_deposit) ? "Amount deposited\n" : "Amount withdrawn\n";
		}

		else if (!strcmp(choice, "BALANCE")) {
			sprintf(msg, "%f", balance(username));
		}

		else if (!strcmp(choice, "PASSWORD")) {
			read(socket, password, sizeof(password));
			output = change_password(username, password);
			if (output == -1) msg = "Error. Please try later.\n";
			else msg = "Password changed\n";
		}

		else if (!strcmp(choice, "DETAILS")) {
			msg = get_details(username);
		}

        // Admin Commands
        else if (!strcmp(choice, "ADD_U")) {
			char* username = malloc(50 * sizeof(char));
			char* password = malloc(50 * sizeof(char));
			read(socket, type, sizeof(type));
			read(socket, username, sizeof(username));
			read(socket, password, sizeof(password));
			printf("User type: %s\n Username = %s\n Passowrd = %s\n", type, username, password);
            char* mode = malloc(20 * sizeof(char));
			if(!strcmp(type,"1")) 
                mode = "SIGNUP_U";  // User
			else mode = "SIGNUP_J";  // Joint
			output = signup(mode, username, password);
			if(output == -1) msg = "User already exists\n";
			else msg = "User added!\n";
		}

        else if(!strcmp(choice, "MODIFY_U")) {
			char* username = malloc(50 * sizeof(char));
			char* password = malloc(50 * sizeof(char));
	    	char* new_username = malloc(50 * sizeof(char));
			read(socket, username, sizeof(username));
			read(socket, new_username, sizeof(new_username));
			read(socket, password, sizeof(password));
			output = modify_user (username, new_username,password);
			if (output == -1) msg = "Cannot modify user.\n";
			else msg = "User details modified!\n";
		}

		else if(!strcmp(choice, "DELETE_U")) {
			char* username = malloc(50 * sizeof(char));
			char* password = malloc(50 * sizeof(char));
			read(socket, username, sizeof(username));
			output = delete_user(username);
			if (output == -1) msg = "User does not exist\n";
			else msg = "User deleted!\n";
		}

		else if (!strcmp(choice, "GET_USER")) {
			char* username = malloc(50 * sizeof(char));
			read(socket, username, sizeof(username));
			msg = get_details(username);
		}
		send(socket, msg, 100 * sizeof(char), 0); 
	}
    return 0;
} 
