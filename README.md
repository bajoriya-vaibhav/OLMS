# Online Library Management System(OLMS)

This is a simple client-server based Library Management System implemented in C. The server manages the library's book inventory and handles user authentication and book transactions, while the client interacts with the server to perform various library operations.

## Features

- **Admin Functions:**
  - Manage Users
  - Load Books
  - Add Book
  - Delete Book
  - Search Book

- **User Functions:**
  - Load Books
  - Search Books
  - Issue Book
  - Return Book

## Project Structure

The project consists of several source files:

- `server.c` - Contains the server-side code for handling client requests and managing the library.
- `client.c` - Contains the client-side code for interacting with the server.
- `book.h` - Contains the definition of the `Book` structure and function prototypes.
- `user.h` - Contains the definition of the `User` structure and function prototypes.
- `makefile` - A Makefile to compile the project.

## Getting Started

### Prerequisites

- GCC (GNU Compiler Collection)

### Compiling the Project

To compile the project, run the following command:

```bash
# For Server
gcc -o server server.c user.c library.c -lpthread

# For Client
gcc -o client client.c

