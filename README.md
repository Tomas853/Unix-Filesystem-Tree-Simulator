## Unix-like Filesystem

Implements a hierarchical directory structure and fully implements a Unix-like file system structure with a few instructions

### Features
- Make a new directory
- Remove a directory
- List directory contents
- Change current working directory
- Chow absolute path of current directory
- Create a new file
- Remove a gile
- Save a filesystem to file
- Load filesystem from file
- Save and exit the shell

### Technologies

- C
- WSL(Ubuntu)
- Github

### Architecture 
The application uses a tree structure of nodes that resembles a typical unix-like file structure where each node is either a file or a folder with files as children.
By allocating and deleting memory dynamically, the system maintains a growing connected tree structure that can be parsed back and forth.

### Challenges

- parsing the tree backwards
- Debugging recursive functions(e.g during freeing each node)

### Future improvements
- Add more useful commands
- Add styles and terminal GUI

### Installation
- use `gcc -o filesystem pa0_gebrewold.c` to compile code and run it by typing `./filesystem` on WSL
