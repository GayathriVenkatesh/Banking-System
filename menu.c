#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <stdlib.h>

char* get_menu(char* menu){
	int option;

	if( !strcmp(menu, "Start")) {
		printf("Welcome to our online banking system. Please choose one of the two options:\n");
		printf("1. Sign Up\n");
		printf("2. Sign In\n");
		scanf("%d", &option); 
		if (option == 1) return "SIGN_UP";
		else if (option == 2) return "SIGN_IN";
		else return "INVALID";
	}

	else if( !strcmp(menu, "Sign Up")) {
		printf("What kind of account would you like to create?\n");
		printf("1. Normal User Account\n");
		printf("2. Joint User Account\n");
		printf("3. Admin Account\n");
		scanf("%d", &option);
		if (option == 1) return "SIGNUP_U";
		else if (option == 2) return "SIGNUP_J";
		else if (option == 3) return "SIGNUP_A";
		else return "INVALID";
	}

	else if( !strcmp(menu, "Sign In")) {
		printf("Choose one of the following sign in methods:\n");
		printf("1. Normal User Account\n");
		printf("2. Joint User Account\n");
		printf("3. Admin Account\n");
		scanf("%d", &option);
		if (option == 1) return "SIGNIN_U";
		else if (option == 2) return "SIGNIN_J";
		else if (option == 3) return "SIGNIN_A";
		else return "INVALID";
	}

	else if( !strcmp(menu, "Admin")) {
		printf("What can we do for you?\n");
		printf("1. Add a new user\n");
		printf("2. Delete an existing user\n");
		printf("3. Modify an existing user\n");
		printf("4. Search for a user's account details\n");
		printf("5. Exit\n") ;
		scanf("%d",&option);

		if (option == 1) return "ADD_U";
		else if (option == 2) return "DELETE_U";
		else if (option == 3) return "MODIFY_U";
		else if (option == 4) return "GET_USER";
		else if (option == 5) return "EXIT";
		else return "INVALID";
	}
	else if( !strcmp(menu, "User")) {
		printf("What would you like to do?\n");
		printf("1. Deposit Money\n");
		printf("2. Withdraw Money\n");
		printf("3. Check Bank Balance\n");
		printf("4. Update Password\n");
		printf("5. View Account Details\n");
		printf("6. Exit\n");
		scanf("%d", &option);

		if (option == 1) return "DEPOSIT";
		else if (option == 2) return "WITHDRAW";
		else if (option == 3) return "BALANCE";
		else if (option == 4) return "PASSWORD";
		else if (option == 5) return "DETAILS";
		else if (option == 6) return "EXIT";
		else return "INVALID";
	}
}
