#include <stdio.h>
#include <string.h>
#include "user.h"
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h> // For fcntl

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
    int fd = open("users.txt", O_RDONLY);
    if (fd == -1) {
        perror("Failed to open books file");
        return;
    }
    // Set up the flock structure for a shared lock
    struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0; // Lock the whole file

    // Obtain a shared lock
    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("Failed to lock file for reading");
        close(fd);
        return;
    }

    FILE *file = fdopen(fd, "r");
    if (file == NULL) {
        perror("Failed to open users file");
        close(fd);
        return;
    }
    *user_count = 0;
    while (fscanf(file, "%s %s %s %d",users[*user_count].user_id, users[*user_count].username, users[*user_count].password, (int*)&users[*user_count].is_admin) == 4) {
        (*user_count)++;
    }

    // Release the lock
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);

    fclose(file); // This will also close the file descriptor
}

void save_users(User users[], int user_count) {
    int fd = open("users.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        perror("Failed to open books file");
        return;
    }

    // Set up the flock structure for an exclusive lock
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0; // Lock the whole file

    // Obtain an exclusive lock
    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("Failed to lock file for writing");
        close(fd);
        return;
    }
    FILE* file = fdopen(fd, "w");
    if (file == NULL) {
        perror("Failed to open users file");
        return;
    }
    for (int i = 0; i < user_count; i++) {
        fprintf(file, "%s %s %s %d\n", users[i].user_id, users[i].username, users[i].password, users[i].is_admin);
    }
    fclose(file);
}

int add_user(User users[], int* user_count, const char* username, const char* password, bool is_admin) {
    if (*user_count >= MAX_USERS) {
        return 0;
    }
    for (int i = 0; i < *user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            return -1;
        }
    }
    char str[6];
    sprintf(str, "%d", 10000+*user_count+1);
    strcpy(users[*user_count].user_id,str);
    strcpy(users[*user_count].username, username);
    strcpy(users[*user_count].password, password);
    users[*user_count].is_admin = is_admin;
    (*user_count)++;
    save_users(users, *user_count);
    return 1;
}

int delete_user(User users[], int* user_count, const char* username,Book books[],int book_count) {
    int index = -1;
    for (int i = 0; i < *user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            index = i;
            for(int j=0;j<book_count;j++){
                if(strcmp(books[j].user_id,users[i].user_id)==0){
                    books[j].available = 1;
                    strcpy(books[j].user_id,"NULL");
                }
            }
            break;
        }
    }
    if (index == -1) {
        return 0;
    }
    for (int i = index; i < *user_count - 1; i++) {
        users[i] = users[i + 1];
    }
    (*user_count)--;
    save_users(users, *user_count);
    return 1;
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
