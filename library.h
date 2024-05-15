#ifndef LIBRARY_H
#define LIBRARY_H

#define MAX_BOOKS 100
#define MAX_ISBN_LENGTH 20
#define MAX_TITLE_LENGTH 100

typedef struct {
    char isbn[MAX_ISBN_LENGTH];
    char title[MAX_TITLE_LENGTH];
} Book;

void load_books(Book books[], int *book_count);
void save_books(Book books[], int book_count);
void add_book(Book books[], int *book_count, const char *isbn, const char *title);
void delete_book(Book books[], int *book_count, const char *isbn);
const char* search_book(Book books[], int book_count, const char *isbn);

#endif // LIBRARY_H
