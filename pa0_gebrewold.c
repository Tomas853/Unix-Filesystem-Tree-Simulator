#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME_SIZE 64
#define MAX_INPUT 128

typedef struct node {
	char name[NAME_SIZE];     // name of the file or directory
	char type;                // 'D' for directory, 'F' for file
	struct node* parent;      // pointer to parent node
	struct node* child;       // pointer to oldest child
	struct node* sibling;     // pointer to next sibling
} Node;

Node *root;
Node *cwd;
char* cmd[] = {
    "mkdir", "rmdir", "ls", "cd", "pwd", "creat", "rm",
    "reload", "save", "quit", 0
};

// Helper to find command index
int find_command(char* user_command)
{
    int i = 0;
    while (cmd[i]) {
        if (strcmp(user_command, cmd[i]) == 0)
            return i;
        i++;
    }
    return -1;
}

void init_root()
{
	root = (Node *)malloc(sizeof(Node));
	if (root == NULL) {
		perror("Failed to allocate memory for root");
		exit(1);
	}

	strcpy(root->name, "/");
	root->parent = root;
	root->sibling = root;
	root->child = NULL;
	root->type = 'D';
	cwd = root;

	printf("Filesystem initialized!\n");
}

Node* find_child(Node* parent, char* name) {
    Node* current = parent->child;
    while (current) {
        if (strcmp(current->name, name) == 0) return current;
        current = current->sibling;
    }
    return NULL;
}

Node* get_node_by_path(char* path) {
    char temp[strlen(path) + 1];
    strcpy(temp, path);

    Node* current = (path[0] == '/') ? root : cwd;
    char* token = strtok(temp, "/");

    while (token != NULL) {
        current = find_child(current, token);
        if (!current || current->type != 'D') return NULL;
        token = strtok(NULL, "/");
    }

    return current;
}

Node* make_node(char* name, char type) {
    Node* node = (Node*)malloc(sizeof(Node));
    strcpy(node->name, name);
    node->type = type;
    node->parent = NULL;
    node->child = NULL;
    node->sibling = NULL;
    return node;
}

void mkdir(char* pathname) {
    if (pathname == NULL || strlen(pathname) == 0) {
        printf("Invalid pathname.\n");
        return;
    }

    // Copy and tokenize pathname
    char temp[strlen(pathname) + 1];
    strcpy(temp, pathname);

    // Extract the last component (or the new directory that we're boutta create)
    char* last = strrchr(temp, '/');
    char* dir_name;

    Node* parent;
    if (last) {
        *last = '\0';  // Split path
        dir_name = last + 1;
        parent = get_node_by_path(temp[0] == '\0' ? "/" : temp);
    }
    else {
        parent = cwd;
        dir_name = temp;
    }

    if (parent == NULL || parent->type != 'D') {
        printf("Invalid parent path.\n");
        return;
    }

    // Check if it already exists
    if (find_child(parent, dir_name)) {
        printf("DIR %s already exists!\n", pathname);
        return;
    }

    // Create and add
    Node* new_dir = make_node(dir_name, 'D');
    add_child(parent, new_dir);
    printf("Directory %s created.\n", pathname);
}

void creat(char* pathname) {
    if (pathname == NULL || strlen(pathname) == 0) {
        printf("Invalid pathname.\n");
        return;
    }

    char temp[strlen(pathname) + 1];
    strcpy(temp, pathname);

    // Extract new FILE name and parent path
    char* last = strrchr(temp, '/');
    char* file_name;
    Node* parent;

    if (last) {
        *last = '\0';  // Split
        file_name = last + 1;
        parent = get_node_by_path(temp[0] == '\0' ? "/" : temp);
    }
    else {
        parent = cwd;
        file_name = temp;
    }

    if (!parent || parent->type != 'D') {
        printf("Invalid parent path.\n");
        return;
    }

    if (find_child(parent, file_name)) {
        printf("%s already exists!\n", pathname);
        return;
    }

    Node* new_file = make_node(file_name, 'F');
    add_child(parent, new_file);
    printf("File %s created.\n", pathname);
}

void rmdir(char* pathname) {
    Node* target = get_node_by_path(pathname);

    if (!target || target->type != 'D') {
        printf("DIR %s does not exist!\n", pathname);
        return;
    }

    if (target->child != NULL) {
        printf("Cannot remove DIR %s (not empty)!\n", pathname);
        return;
    }

    // Disconnect from parent's child/sibling list
    Node* parent = target->parent;
    Node* prev = NULL;
    Node* curr = parent->child;

    while (curr) {
        if (curr == target) {
            if (prev) {
                prev->sibling = curr->sibling;
            }
            else {
                parent->child = curr->sibling;
            }
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->sibling;
    }
}

void rm(char* pathname) {
    Node* target = get_node_by_path(pathname);

    if (!target) {
        printf("File %s does not exist!\n", pathname);
        return;
    }

    if (target->type != 'F') {
        printf("Cannot remove %s (not a FILE)!\n", pathname);
        return;
    }

    // Disconnect from parent's child/sibling list
    Node* parent = target->parent;
    Node* prev = NULL;
    Node* curr = parent->child;

    while (curr) {
        if (curr == target) {
            if (prev) {
                prev->sibling = curr->sibling;
            }
            else {
                parent->child = curr->sibling;
            }
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->sibling;
    }
}

void add_child(Node* parent, Node* new_child) {
    new_child->parent = parent;

    if (!parent->child) {
        parent->child = new_child;
    }
    else {
        Node* curr = parent->child;
        while (curr->sibling) {
            curr = curr->sibling;
        }
        curr->sibling = new_child;
    }
}

void ls(char* pathname) {
    Node* target;

    if (!pathname || strlen(pathname) == 0) {
        target = cwd;
    }
    else {
        target = get_node_by_path(pathname);
    }

    if (!target) {
        printf("No such file or directory: %s\n", pathname);
        return;
    }

    if (target->type == 'F') {
        printf("F %s\n", target->name);
        return;
    }

    Node* child = target->child;
    while (child) {
        printf("%s %s\n", (child->type == 'D' ? "D" : "F"), child->name);
        child = child->sibling;
    }
}

void cd(char* pathname) {
    if (!pathname || strlen(pathname) == 0) {
        cwd = root;  // cd with no args goes to root
        return;
    }

    //handle "cd .." case
    if (strcmp(pathname, "..") == 0) {
        if (cwd != root && cwd->parent != NULL) {
            cwd = cwd->parent;
        }
        return;
    }

    Node* target = get_node_by_path(pathname);
    if (!target) {
        printf("No such file or directory: %s\n", pathname);
        return;
    }

    if (target->type != 'D') {
        printf("Not a directory: %s\n", pathname);
        return;
    }

    cwd = target;
}

void print_path(Node* node) {
    if (node == root) {
        printf("/");
        return;
    }

    print_path(node->parent);
    printf("%s/", node->name);
}

void pwd() {
    if (cwd == root) {
        printf("/\n");
        return;
    }

    print_path(cwd);
    printf("\n");
}

void save_helper(FILE* fp, Node* node, char* path) {
    if (!node) return;

    char newpath[1024];
    if (strcmp(path, "/") == 0)
        snprintf(newpath, sizeof(newpath), "/%s", node->name);
    else
        snprintf(newpath, sizeof(newpath), "%s/%s", path, node->name);

    fprintf(fp, "%c %s\n", node->type, newpath);

    save_helper(fp, node->child, newpath);
    save_helper(fp, node->sibling, path);
}

void save(char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to open file for saving");
        return;
    }
    save_helper(fp, root->child, "/");  // Skip the root itself
    fclose(fp);
    printf("Filesystem saved to %s\n", filename);
}

void reload(char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        perror("Failed to open file for reloading");
        return;
    }

    char line[1024];
    char type;
    char path[1024];

    // Reinitialize the root
    free_tree(root);
    init_root();

    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%c %s", &type, path) == 2) {
            if (type == 'D') {
                mkdir(path);
            }
            else if (type == 'F') {
                creat(path);
            }
        }
    }

    fclose(fp);
    printf("Filesystem reloaded from %s\n", filename);
}

void free_tree(Node* node) {
    if (!node) return;
    free_tree(node->child);
    free_tree(node->sibling);
    free(node);
}

void quit() {
    save("fssim_gebrewold.txt");
    free_tree(root);
    exit(0);
}

void greet() {
    printf("===========================================\n");
    printf(" Yo Yo Yo! Welcome to My Unix-like Filesystem Shell!\n");
    printf("===========================================\n\n");

    printf("You can type any of the following commands:\n\n");
    printf("  mkdir  <pathname>    - Make a new directory\n");
    printf("  rmdir  <pathname>    - Remove an empty directory\n");
    printf("  ls     [pathname]    - List directory contents\n");
    printf("  cd     [pathname]    - Change current working directory\n");
    printf("  pwd                 - Show absolute path of CWD\n");
    printf("  creat  <pathname>    - Create a new file\n");
    printf("  rm     <pathname>    - Remove a file\n");
    printf("  save   <filename>    - Save filesystem to file\n");
    printf("  reload <filename>    - Load filesystem from file\n");
    printf("  quit                - Save and exit the shell\n\n");

    printf("You can type 'quit' anytime to exit the shell. Have fun!\n\n");
    printf("Start typing your commands below:\n\n");
}

int main()
{
    init_root();
    greet();

    char line[MAX_INPUT], * command, * pathname;
    int cmd_index;

    while (1)
    {
        printf("> ");
        fgets(line, MAX_INPUT, stdin);
        line[strcspn(line, "\n")] = '\0'; // Remove newline

        command = strtok(line, " ");
        if (!command) continue;

        pathname = strtok(NULL, ""); // Get rest of line as one string (or pathname)

        cmd_index = find_command(command);
        switch (cmd_index) {
        case 0: mkdir(pathname); break;
        case 1: rmdir(pathname); break;
        case 2: ls(pathname); break;
        case 3: cd(pathname); break;
        case 4: pwd(); break;
        case 5: creat(pathname); break;
        case 6: rm(pathname); break;
        case 7: reload(pathname); break;
        case 8: save(pathname); break;
        case 9: quit(); break;
        default: printf("Command not found!\n"); break;

        };

    }

    return 1;
};