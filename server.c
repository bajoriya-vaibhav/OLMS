#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "user.h"
#include "library.h"

#define PORT 8080
#define BUFFER_SIZE 1024

User users[MAX_USERS];
int user_count = 1;
Book books[MAX_BOOKS];
int book_count;
pthread_mutex_t file_mutex;

void send_admin_menu(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    snprintf(buffer, BUFFER_SIZE, "\nLibrary Management Menu:\n1. Manage Users\n2. List Books\n3. Add Book\n4. Delete Book\n5. Search Book\n6. Exit\nEnter your choice:");
    send(client_socket, buffer, strlen(buffer), 0);
}

void send_user_menu(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    snprintf(buffer, BUFFER_SIZE, "\nLibrary Menu:\n1. List Books\n2. Search Books\n3. Issue Book\n4. Return Book\n5. Exit\nEnter your choice:");
    send(client_socket, buffer, strlen(buffer), 0);
}

void manage_users(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    int choice;

    while (1) {
        snprintf(buffer, BUFFER_SIZE, "\nManage Users Menu:\n");
        send(client_socket, buffer, strlen(buffer), 0);
        snprintf(buffer, BUFFER_SIZE, "1. List Users\n2. Add User\n3. Delete User\n4. Exit\nEnter your choice: ");
        send(client_socket, buffer, strlen(buffer), 0);
        memset(buffer, 0, BUFFER_SIZE);
        read(client_socket, buffer, BUFFER_SIZE);
        choice = atoi(buffer);

        switch (choice) {
            case 1:
                pthread_mutex_lock(&file_mutex);
                list_users(users, user_count);
                pthread_mutex_unlock(&file_mutex);
                break;
            case 2: {
                char new_username[MAX_USERNAME_LENGTH];
                char new_password[MAX_PASSWORD_LENGTH];
                int is_admin;
                snprintf(buffer, BUFFER_SIZE, "Enter new username: ");
                send(client_socket, buffer, strlen(buffer), 0);
                memset(buffer, 0, BUFFER_SIZE);
                read(client_socket, buffer, BUFFER_SIZE);
                sscanf(buffer, "%s", new_username);
                snprintf(buffer, BUFFER_SIZE, "Enter new password: ");
                send(client_socket, buffer, strlen(buffer), 0);
                memset(buffer, 0, BUFFER_SIZE);
                read(client_socket, buffer, BUFFER_SIZE);
                sscanf(buffer, "%s", new_password);
                snprintf(buffer, BUFFER_SIZE, "Is admin (1 for yes, 0 for no): ");
                send(client_socket, buffer, strlen(buffer), 0);
                memset(buffer, 0, BUFFER_SIZE);
                read(client_socket, buffer, BUFFER_SIZE);
                sscanf(buffer, "%d", &is_admin);
                pthread_mutex_lock(&file_mutex);
                add_user(users, &user_count, new_username, new_password, is_admin);
                pthread_mutex_unlock(&file_mutex);
                snprintf(buffer, BUFFER_SIZE, "User added successfully.\n");
                send(client_socket, buffer, strlen(buffer), 0);
                break;
            }
            case 3: {
                char del_username[MAX_USERNAME_LENGTH];
                snprintf(buffer, BUFFER_SIZE, "Enter username to delete: ");
                send(client_socket, buffer, strlen(buffer), 0);
                memset(buffer, 0, BUFFER_SIZE);
                read(client_socket, buffer, BUFFER_SIZE);
                sscanf(buffer, "%s", del_username);
                pthread_mutex_lock(&file_mutex);
                delete_user(users, &user_count, del_username);
                pthread_mutex_unlock(&file_mutex);
                snprintf(buffer, BUFFER_SIZE, "User deleted successfully.\n");
                send(client_socket, buffer, strlen(buffer), 0);
                break;
            }
            case 4:
                return;
            default:
                snprintf(buffer, BUFFER_SIZE, "Invalid choice. Try again.\n");
                send(client_socket, buffer, strlen(buffer), 0);
                break;
        }
    }
}

void signup(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    
    snprintf(buffer, BUFFER_SIZE, "Enter new username: ");
    send(client_socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, BUFFER_SIZE);
    read(client_socket, buffer, BUFFER_SIZE);
    sscanf(buffer, "%s", username);
    
    snprintf(buffer, BUFFER_SIZE, "Enter new password: ");
    send(client_socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, BUFFER_SIZE);
    read(client_socket, buffer, BUFFER_SIZE);
    sscanf(buffer, "%s", password);
    
    pthread_mutex_lock(&file_mutex);
    add_user(users, &user_count, username, password, false);  // New users are not admin by default
    pthread_mutex_unlock(&file_mutex);
    
    snprintf(buffer, BUFFER_SIZE, "Signup successful. You can now login with your new credentials.\n");
    send(client_socket, buffer, strlen(buffer), 0);
}


void* handle_client(void* arg) {
    int client_socket = *((int*)arg);
    free(arg);

    char buffer[BUFFER_SIZE] = {0};
    char username[MAX_USERNAME_LENGTH] = {0};
    int authenticated = 0;
    int is_admin_user = 0;

    // Initial menu for login or signup
    snprintf(buffer, BUFFER_SIZE, "Welcome to the Library Management System\n1. Signup\n2. Login\nEnter your choice: ");
    send(client_socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, BUFFER_SIZE);
    read(client_socket, buffer, BUFFER_SIZE);
    int choice = atoi(buffer);

    if (choice == 1) {
        signup(client_socket);
    }
    if (choice  == 1 || choice == 2){
        // Authentication
        send(client_socket, "\nEnter Login Credentials:\nUsername:", strlen("\nEnter Login Credentials:\nUsername:"), 0);
        read(client_socket, buffer, BUFFER_SIZE);
        char input_username[MAX_USERNAME_LENGTH];
        char input_password[MAX_PASSWORD_LENGTH];
        sscanf(buffer, "%[^\n]", input_username);
        send(client_socket, "Password:", strlen("Password:"), 0);
        memset(buffer, 0, BUFFER_SIZE);
        read(client_socket, buffer, BUFFER_SIZE);
        sscanf(buffer, "%s", input_password);
        if (authenticate(users, user_count, input_username, input_password)) {
            authenticated = 1;
            is_admin_user = is_admin(users, user_count, input_username);
            snprintf(buffer, BUFFER_SIZE, "Authentication successful\n");
            send(client_socket, buffer, strlen(buffer), 0);
            strcpy(username, input_username);
        } else {
            snprintf(buffer, BUFFER_SIZE, "Authentication failed\n");
            send(client_socket, buffer, strlen(buffer), 0);
            close(client_socket);
            return NULL;
        }
    }
    else {
        snprintf(buffer, BUFFER_SIZE, "Invalid choice\n");
        send(client_socket, buffer, strlen(buffer), 0);
        close(client_socket);
        return NULL;
    }

    while (authenticated) {
        if (is_admin_user) {
            send_admin_menu(client_socket);
        } else {
            send_user_menu(client_socket);
        }
        
        memset(buffer, 0, BUFFER_SIZE);
        read(client_socket, buffer, BUFFER_SIZE);

        if (is_admin_user) {
            int choice = atoi(buffer);
            switch (choice) {
                case 1:
                    manage_users(client_socket);
                    break;
                case 2:
                    pthread_mutex_lock(&file_mutex);
                    for (int i = 0; i < book_count; i++) {
                        snprintf(buffer, BUFFER_SIZE, "Book Title: %s, ISBN: %s\n", books[i].title, books[i].isbn);
                        send(client_socket, buffer, strlen(buffer), 0);
                        memset(buffer, 0, BUFFER_SIZE);
                    }
                    pthread_mutex_unlock(&file_mutex);
                    break;
                case 3: {
                    char isbn[MAX_ISBN_LENGTH];
                    char title[MAX_TITLE_LENGTH];
                    snprintf(buffer, BUFFER_SIZE, "Enter ISBN and Title (ISBN Title): ");
                    send(client_socket, buffer, strlen(buffer), 0);
                    memset(buffer, 0, BUFFER_SIZE);
                    read(client_socket, buffer, BUFFER_SIZE);
                    sscanf(buffer, "%s %[^\n]", isbn, title);
                    pthread_mutex_lock(&file_mutex);
                    add_book(books, &book_count, isbn, title);
                    pthread_mutex_unlock(&file_mutex);
                    snprintf(buffer, BUFFER_SIZE, "Book added successfully: %s\n", title);
                    send(client_socket, buffer, strlen(buffer), 0);
                    break;
                }
                case 4: {
                    char isbn[MAX_ISBN_LENGTH];
                    snprintf(buffer, BUFFER_SIZE, "Enter ISBN: ");
                    send(client_socket, buffer, strlen(buffer), 0);
                    memset(buffer, 0, BUFFER_SIZE);
                    read(client_socket, buffer, BUFFER_SIZE);
                    sscanf(buffer, "%s", isbn);
                    pthread_mutex_lock(&file_mutex);
                    delete_book(books, &book_count, isbn);
                    pthread_mutex_unlock(&file_mutex);
                    snprintf(buffer, BUFFER_SIZE, "Book deleted successfully.\n");
                    send(client_socket, buffer, strlen(buffer), 0);
                    break;
                }
                case 5: {
                    char isbn[MAX_ISBN_LENGTH];
                    snprintf(buffer, BUFFER_SIZE, "Enter ISBN: ");
                    send(client_socket, buffer, strlen(buffer), 0);
                    memset(buffer, 0, BUFFER_SIZE);
                    read(client_socket, buffer, BUFFER_SIZE);
                    sscanf(buffer, "%s", isbn);
                    const char* title = search_book(books, book_count, isbn);
                    snprintf(buffer, BUFFER_SIZE, "Book found: %s\n", title);
                    send(client_socket, buffer, strlen(buffer), 0);
                    break;
                }
                case 6:
                    authenticated = 0;
                    snprintf(buffer, BUFFER_SIZE, "Exiting...\n");
                    send(client_socket, buffer, strlen(buffer), 0);
                    break;
                default:
                    snprintf(buffer, BUFFER_SIZE, "Invalid choice. Try again.\n");
                    send(client_socket, buffer, strlen(buffer), 0);
                    break;
            }
        } else {
            int choice = atoi(buffer);
            switch (choice) {
                case 1:
                    pthread_mutex_lock(&file_mutex);
                    for (int i = 0; i < book_count; i++) {
                        snprintf(buffer, BUFFER_SIZE, "Book Title: %s, ISBN: %s\n", books[i].title, books[i].isbn);
                        send(client_socket, buffer, strlen(buffer), 0);
                        memset(buffer, 0, BUFFER_SIZE);
                    }
                    pthread_mutex_unlock(&file_mutex);
                    break;
                case 2: {
                    char isbn[MAX_ISBN_LENGTH];
                    snprintf(buffer, BUFFER_SIZE, "Enter ISBN: ");
                    send(client_socket, buffer, strlen(buffer), 0);
                    memset(buffer, 0, BUFFER_SIZE);
                    read(client_socket, buffer, BUFFER_SIZE);
                    sscanf(buffer, "%s", isbn);
                    const char* title = search_book(books, book_count, isbn);
                    snprintf(buffer, BUFFER_SIZE, "Book found: %s\n", title);
                    send(client_socket, buffer, strlen(buffer), 0);
                    break;
                }
                case 3:
                    // Issue Book (not implemented in this example)
                    snprintf(buffer, BUFFER_SIZE, "Issue Book feature is not implemented.\n");
                    send(client_socket, buffer, strlen(buffer), 0);
                    break;
                case 4:
                    // Return Book (not implemented in this example)
                    snprintf(buffer, BUFFER_SIZE, "Return Book feature is not implemented.\n");
                    send(client_socket, buffer, strlen(buffer), 0);
                    break;
                case 5:
                    authenticated = 0;
                    snprintf(buffer, BUFFER_SIZE, "Exiting...\n");
                    send(client_socket, buffer, strlen(buffer), 0);
                    break;
                default:
                    snprintf(buffer, BUFFER_SIZE, "Invalid choice. Try again.\n");
                    send(client_socket, buffer, strlen(buffer), 0);
                    break;
            }
        }
    }

    close(client_socket);
    return NULL;
}

int main() {
    load_users(users, &user_count);
    load_books(books, &book_count);

    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        pthread_t thread_id;
        int* client_socket = malloc(sizeof(int));
        *client_socket = new_socket;
        pthread_create(&thread_id, NULL, handle_client, client_socket);
        pthread_detach(thread_id);
    }

    close(server_fd);
    return 0;
}
