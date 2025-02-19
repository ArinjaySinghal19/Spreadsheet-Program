#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#define BUFFER_SIZE 4096
#define READ_END 0
#define WRITE_END 1

typedef struct {
    int rows, cols;
    char *commands[20];
    char *expected_status[20];
    int num_cmds;
} TestCase;

TestCase test_cases[] = {
    {
        .rows = 2, 
        .cols = 2,
        .commands = {
            "A1=2",
            "B1=A1+1",
            "A2=MAX(A1:B1)",
            "q"
        },
        .expected_status = {
            "(ok)",
            "(ok)",
            "(ok)",
            "(ok)"
        },
        .num_cmds = 4
    },
    {
        .rows = 3, 
        .cols = 3,
        .commands = {
            "A1=1",
            "B1=A1-100",
            "B2=1/A1",
            "C1=MAX(B1:B2)",
            "A1=0",
            "q"
        },
        .expected_status = {
            "(ok)",
            "(ok)",
            "(ok)",
            "(ok)",
            "(ok)",
            "(ok)"
        },
        .num_cmds = 6
    }
};

// Function to read until we find a prompt or timeout
int read_until_prompt(int fd, char *buffer, size_t size) {
    size_t total = 0;
    char c;
    while (total < size - 1) {
        ssize_t n = read(fd, &c, 1);
        if (n <= 0) return -1;
        
        buffer[total++] = c;
        buffer[total] = '\0';
        
        // Check if we've found a prompt
        if (total >= 2 && buffer[total-2] == '>' && buffer[total-1] == ' ') {
            return 0;
        }
    }
    return -1;
}

int run_test(TestCase *test, int test_num) {
    int to_child[2];   // Parent writes to child
    int to_parent[2];  // Child writes to parent
    
    if (pipe(to_child) == -1 || pipe(to_parent) == -1) {
        perror("pipe failed");
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {  // Child process
        // Redirect stdin to read end of to_child pipe
        dup2(to_child[READ_END], STDIN_FILENO);
        // Redirect stdout to write end of to_parent pipe
        dup2(to_parent[WRITE_END], STDOUT_FILENO);
        
        // Close unused pipe ends
        close(to_child[WRITE_END]);
        close(to_parent[READ_END]);
        close(to_child[READ_END]);
        close(to_parent[WRITE_END]);

        // Convert rows and cols to strings
        char rows_str[32], cols_str[32];
        snprintf(rows_str, sizeof(rows_str), "%d", test->rows);
        snprintf(cols_str, sizeof(cols_str), "%d", test->cols);

        // Execute the spreadsheet program
        execl("./sheet", "./sheet", rows_str, cols_str, NULL);
        perror("execl failed");
        exit(1);
    }

    // Parent process
    close(to_child[READ_END]);
    close(to_parent[WRITE_END]);

    char buffer[BUFFER_SIZE];
    int passed = 1;

    // Set non-blocking read
    fcntl(to_parent[READ_END], F_SETFL, O_NONBLOCK);

    // Wait for initial prompt
    usleep(100000);  // Give the program time to start
    read_until_prompt(to_parent[READ_END], buffer, BUFFER_SIZE);

    for (int i = 0; i < test->num_cmds && passed; i++) {
        printf("Running command: %s\n", test->commands[i]);
        
        // Send command
        write(to_child[WRITE_END], test->commands[i], strlen(test->commands[i]));
        write(to_child[WRITE_END], "\n", 1);
        
        // Give the program time to process
        usleep(100000);

        // Clear buffer
        memset(buffer, 0, BUFFER_SIZE);

        // Read response
        if (read_until_prompt(to_parent[READ_END], buffer, BUFFER_SIZE) == 0) {
            // Check if status is in the response
            if (strstr(buffer, test->expected_status[i]) == NULL) {
                printf("Test %d failed at command: %s\n", test_num, test->commands[i]);
                printf("Expected status: %s\n", test->expected_status[i]);
                printf("Got response: %s\n", buffer);
                passed = 0;
            }
        } else {
            printf("Test %d failed: No proper response for command: %s\n", 
                   test_num, test->commands[i]);
            passed = 0;
        }

        if (test->commands[i][0] == 'q') break;
    }

    // Clean up
    close(to_child[WRITE_END]);
    close(to_parent[READ_END]);
    
    // Wait for child process to end
    int status;
    waitpid(pid, &status, 0);

    if (passed) {
        printf("Test %d passed ✅\n", test_num);
    } else {
        printf("Test %d failed ❌\n", test_num);
    }

    return !passed;
}

int main() {
    int failed_tests = 0;
    int total_tests = sizeof(test_cases) / sizeof(TestCase);

    printf("Running %d test cases...\n\n", total_tests);

    for (int i = 0; i < total_tests; i++) {
        if (run_test(&test_cases[i], i + 1)) {
            failed_tests++;
        }
        // Add delay between tests
        usleep(500000);  // 500ms delay between tests
    }

    printf("\nTest Summary: %d/%d tests passed.\n", total_tests - failed_tests, total_tests);
    return failed_tests;
}
