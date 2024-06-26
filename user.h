#ifndef USER_H
#define USER_H

#include <stdbool.h>
#include "library.h"

#define MAX_USERS 100
#define MAX_USERNAME_LENGTH 50
#define MAX_PASSWORD_LENGTH 50

typedef struct {
    char user_id[6];
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    bool is_admin;
} User;

bool authenticate(User users[], int user_count, const char* username, const char* password);
bool is_admin(User users[], int user_count, const char* username);
void load_users(User users[], int* user_count);
void save_users(User users[], int user_count);
int add_user(User users[], int* user_count, const char* username, const char* password, bool is_admin);
int delete_user(User users[], int* user_count, const char* username,Book books[],int book_count);
char * list_users(User users[], int user_count);

#endif
