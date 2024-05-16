#ifndef LIBRARY_H
#define LIBRARY_H

#include <stdbool.h>

#define MAX_BOOKS 100
#define MAX_ISBN_LENGTH 6
#define MAX_TITLE_LENGTH 100
#define BUFFER_SIZE 1024

typedef struct {
    char isbn[MAX_ISBN_LENGTH];
    int available;
    char user_id[6];
    char title[MAX_TITLE_LENGTH];
} Book;

void load_books(Book books[], int *book_count);
void save_books(Book books[], int book_count);
void add_book(Book books[], int *book_count, const char *isbn, const char *title);
int delete_book(Book books[], int *book_count, const char *isbn);
const char* search_book(Book books[], int book_count, const char *isbn);
void issue_book(int client_socket,Book books[],int book_count, const char *userid);
void return_book(int client_socket,Book books[],int book_count, const char *userid);

#endif // LIBRARY_H
