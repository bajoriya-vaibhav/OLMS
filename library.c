#include <stdio.h>
#include <string.h>
#include "library.h"
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <fcntl.h> // For fcntl

extern pthread_mutex_t file_mutex;

void load_books(Book books[], int *book_count) {
    int fd = open("books.txt", O_RDONLY);
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
        perror("Failed to associate file descriptor with stream");
        close(fd);
        return;
    }

    *book_count = 0;
    while (fscanf(file, "%s %d %s %[^\n]", books[*book_count].isbn, &books[*book_count].available, books[*book_count].user_id, books[*book_count].title) != EOF) {
        (*book_count)++;
    }

    // Release the lock
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);

    fclose(file); // This will also close the file descriptor
}

void save_books(Book books[], int book_count) {
    int fd = open("books.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);
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

    FILE *file = fdopen(fd, "w");
    if (file == NULL) {
        perror("Failed to associate file descriptor with stream");
        close(fd);
        return;
    }

    for (int i = 0; i < book_count; i++) {
        fprintf(file, "%s %d %s %s\n", books[i].isbn, books[i].available, books[i].user_id, books[i].title);
    }

    // Release the lock
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);

    fclose(file); // This will also close the file descriptor
}

void add_book(Book books[], int *book_count, const char *isbn, const char *title) {
    strcpy(books[*book_count].isbn, isbn);
    books[*book_count].available = 1;
    strcpy(books[*book_count].user_id, "NULL");
    strcpy(books[*book_count].title, title);
    (*book_count)++;
    save_books(books, *book_count);
}

int delete_book(Book books[], int *book_count, const char *isbn) {
    for (int i = 0; i < *book_count; i++) {
        if (strcmp(books[i].isbn, isbn) == 0) {
            for (int j = i; j < *book_count - 1; j++) {
                books[j] = books[j + 1];
            }
            (*book_count)--;
            save_books(books, *book_count);
            return 1;
        }
    }
    return 0;
}

const char* search_book(Book books[], int book_count, const char *isbn) {
    for (int i = 0; i < book_count; i++) {
        if (strcmp(books[i].isbn, isbn) == 0) {
            return books[i].title;
        }
    }
    return "Book not found";
}

void issue_book(int client_socket, Book books[], int book_count, const char *userid) {
    char buffer[BUFFER_SIZE] = {0};
    char isbn[MAX_ISBN_LENGTH];
    char response[BUFFER_SIZE];

    snprintf(buffer, BUFFER_SIZE, "Enter ISBN of the book to issue: ");
    send(client_socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, BUFFER_SIZE);
    read(client_socket, buffer, BUFFER_SIZE);
    buffer[strcspn(buffer, "\n")] = 0;  // Ensure null-termination
    sscanf(buffer, "%s", isbn);

    pthread_mutex_lock(&file_mutex);

    // Check if the book exists
    const char* title = search_book(books, book_count, isbn);
    if (strcmp(title, "Book not found") == 0) {
        snprintf(response, BUFFER_SIZE, "Book not found.\n");
        send(client_socket, response, strlen(response), 0);
        return;
    } else {
        int t = -1;
        for (int i = 0; i < book_count; i++) {
            if (strcmp(books[i].isbn, isbn) == 0) {
                t = i;
                break;
            }
        }
        if (t != -1 && books[t].available == 0) {
            snprintf(response, BUFFER_SIZE, "Book is already issued.\nPress Enter to continue...\n");
        } else if (t != -1) {
            books[t].available = 0;  // Mark book as issued
            strcpy(books[t].user_id, userid);  // Assign the user ID to the book
            save_books(books, book_count);
            snprintf(response, BUFFER_SIZE, "Book issued successfully: %s\nPress Enter to continue...\n", books[t].title);
        } else {
            snprintf(response, BUFFER_SIZE, "An unexpected error occurred.\nPress Enter to continue...\n");
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
    buffer[strcspn(buffer, "\n")] = 0;  // Ensure null-termination
    sscanf(buffer, "%s", isbn);

    pthread_mutex_lock(&file_mutex);

    // Check if the book exists
    const char* title = search_book(books, book_count, isbn);
    if (strcmp(title, "Book not found") == 0) {
        snprintf(response, BUFFER_SIZE, "Book not found.\n");
        send(client_socket, response, strlen(response), 0);
        return;
    } else {
        int t = -1;
        for (int i = 0; i < book_count; i++) {
            if (strcmp(books[i].isbn, isbn) == 0) {
                t = i;
                break;
            }
        }

        if (t != -1 && books[t].available == 0 && strcmp(books[t].user_id, userid) == 0){
            books[t].available = 1;  // Mark book as available
            strcpy(books[t].user_id, "NULL");  // Remove the user ID from the book
            save_books(books, book_count);
            snprintf(response, BUFFER_SIZE, "Book successfully returned.\nPress Enter to continue...\n");
        }else {
            snprintf(response, BUFFER_SIZE, "Book already returned or An unexpected error occurred.\nPress Enter to continue...\n");
        }
    }

    pthread_mutex_unlock(&file_mutex);
    send(client_socket, response, strlen(response), 0);
}