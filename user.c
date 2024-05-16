#include <stdio.h>
#include <string.h>
#include "user.h"

bool authenticate(User users[], int user_count, const char* username, const char* password) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0) {
            return true;
        }
    }
    return false;
}

bool is_admin(User users[], int user_count, const char* username) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            return users[i].is_admin;
        }
    }
    return false;
}

void load_users(User users[], int* user_count) {
    FILE* file = fopen("users.txt", "r");
    if (file == NULL) {
        perror("Failed to open users file");
        return;
    }
    *user_count = 0;
    while (fscanf(file, "%s %s %s %d",users[*user_count].user_id, users[*user_count].username, users[*user_count].password, (int*)&users[*user_count].is_admin) == 4) {
        (*user_count)++;
    }
    fclose(file);
}

void save_users(User users[], int user_count) {
    FILE* file = fopen("users.txt", "w");
    if (file == NULL) {
        perror("Failed to open users file");
        return;
    }
    for (int i = 0; i < user_count; i++) {
        fprintf(file, "%s %s %s %d\n", users[i].user_id, users[i].username, users[i].password, users[i].is_admin);
    }
    fclose(file);
}

void add_user(User users[], int* user_count, const char* username, const char* password, bool is_admin) {
    if (*user_count >= MAX_USERS) {
        printf("User limit reached. Cannot add more users.\n");
        return;
    }
    char str[6];
    sprintf(str, "%d", 10000+*user_count);
    strcpy(users[*user_count].user_id,str);
    strcpy(users[*user_count].username, username);
    strcpy(users[*user_count].password, password);
    users[*user_count].is_admin = is_admin;
    (*user_count)++;
    save_users(users, *user_count);
}

void delete_user(User users[], int* user_count, const char* username) {
    int index = -1;
    for (int i = 0; i < *user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            index = i;
            break;
        }
    }
    if (index == -1) {
        printf("User not found.\n");
        return;
    }
    for (int i = index; i < *user_count - 1; i++) {
        users[i] = users[i + 1];
    }
    (*user_count)--;
    save_users(users, *user_count);
}

char * list_users(User users[], int user_count) {
    char buffer[5000] ={0};
    snprintf(buffer, 5000, "User_Id, Username, Admin\n");
    for (int i = 0; i < user_count; i++) {
        char str[150] = {0};
        sprintf(str, "%s, %s, %s\n", users[i].user_id, users[i].username, users[i].is_admin ? "Yes" : "No");
        strncat(buffer,str,5000 - strlen(buffer) - 1);
    }
    return strdup(buffer);
}
