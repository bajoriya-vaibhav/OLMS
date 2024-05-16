#include <stdio.h>
#include <string.h>
#include "library.h"
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

extern pthread_mutex_t file_mutex;

void load_books(Book books[], int *book_count) {
    FILE *file = fopen("books.txt", "r");
    if (file == NULL) {
        perror("Failed to open books file");
        return;
    }
    
    *book_count = 0;
    while (fscanf(file, "%s %s %s %[^\n]", books[*book_count].isbn,(char *) &books[*book_count].available,books[*book_count].user_id, books[*book_count].title) != EOF) {
        (*book_count)++;
    }
    
    fclose(file);
}

void save_books(Book books[], int book_count) {
    FILE *file = fopen("books.txt", "w");
    if (file == NULL) {
        perror("Failed to open books file");
        return;
    }
    
    for (int i = 0; i < book_count; i++) {
        fprintf(file, "%s %s %s %s\n", books[i].isbn,(char *) &books[i].available, books[i].user_id, books[i].title);
    }
    
    fclose(file);
}

void add_book(Book books[], int *book_count, const char *isbn, const char *title) {
    strcpy(books[*book_count].isbn, isbn);
    books[*book_count].available = true;
    strcpy(books[*book_count].user_id, "NULL");
    strcpy(books[*book_count].title, title);
    (*book_count)++;
    save_books(books, *book_count);
}

void delete_book(Book books[], int *book_count, const char *isbn) {
    for (int i = 0; i < *book_count; i++) {
        if (strcmp(books[i].isbn, isbn) == 0) {
            for (int j = i; j < *book_count - 1; j++) {
                books[j] = books[j + 1];
            }
            (*book_count)--;
            save_books(books, *book_count);
            return;
        }
    }
}

const char* search_book(Book books[], int book_count, const char *isbn) {
    for (int i = 0; i < book_count; i++) {
        if (strcmp(books[i].isbn, isbn) == 0) {
            return books[i].title;
        }
    }
    return "Book not found";
}

void issue_book(int client_socket,Book books[],int book_count, const char *userid) {
    char buffer[BUFFER_SIZE] = {0};
    char isbn[MAX_ISBN_LENGTH];
    char response[BUFFER_SIZE];

    snprintf(buffer, BUFFER_SIZE, "Enter ISBN of the book to issue: ");
    send(client_socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, BUFFER_SIZE);
    read(client_socket, buffer, BUFFER_SIZE);
    sscanf(buffer, "%s", isbn);

    pthread_mutex_lock(&file_mutex);

    // Check if the book exists
    const char* title = search_book(books, book_count, isbn);
    if (strcmp(title, "Book not found") == 0) {
        snprintf(response, BUFFER_SIZE, "Book not found.\n");
    } else {
        int t = -1;
        for (int i = 0; i < book_count; i++) {
            if (strcmp(books[i].isbn, isbn) == 0) {
                t=i;
            }
        }
        if (t != -1 && !books[t].available) {
            snprintf(response, BUFFER_SIZE, "Book is already issued.\n");
        } else {
            books[t].available = true;
            snprintf(response, BUFFER_SIZE, "Book issued successfully: %s\n", books[t].title);
        }
    }

    pthread_mutex_unlock(&file_mutex);
    send(client_socket, response, strlen(response), 0);
}

void return_book(int client_socket,Book books[],int book_count, const char *userid) {
    char buffer[BUFFER_SIZE] = {0};
    char isbn[MAX_ISBN_LENGTH];
    char response[BUFFER_SIZE];

    snprintf(buffer, BUFFER_SIZE, "Enter ISBN of the book to return: ");
    send(client_socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, BUFFER_SIZE);
    read(client_socket, buffer, BUFFER_SIZE);
    sscanf(buffer, "%s", isbn);

    pthread_mutex_lock(&file_mutex);
    int t = -1;
    for (int i = 0; i < book_count; i++) {
        if (strcmp(books[i].isbn, isbn) == 0) {
            t=i;
        }
    }
    if (t != -1 && !books[t].available) {
        books[t].available = true;
        snprintf(response, BUFFER_SIZE, "Book returned successfully.\n");
    } else {
        snprintf(response, BUFFER_SIZE, "Book not issued to you or does not exist.\n");
    }
    pthread_mutex_unlock(&file_mutex);
    send(client_socket, response, strlen(response), 0);
}