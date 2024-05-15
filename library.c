#include <stdio.h>
#include <string.h>
#include "library.h"

void load_books(Book books[], int *book_count) {
    FILE *file = fopen("books.txt", "r");
    if (file == NULL) {
        perror("Failed to open books file");
        return;
    }
    
    *book_count = 0;
    while (fscanf(file, "%s %[^\n]", books[*book_count].isbn, books[*book_count].title) != EOF) {
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
        fprintf(file, "%s %s\n", books[i].isbn, books[i].title);
    }
    
    fclose(file);
}

void add_book(Book books[], int *book_count, const char *isbn, const char *title) {
    strcpy(books[*book_count].isbn, isbn);
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

// void list_books(Book books[], int book_count) {
//     for (int i = 0; i < book_count; i++) {
//         printf("Book Title: %s, ISBN: %s\n", books[i].title, books[i].isbn);
//     }
// }