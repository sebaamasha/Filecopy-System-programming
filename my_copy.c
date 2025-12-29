#include <fcntl.h> 
#include <unistd.h>  
#include <stdlib.h>   
#include <errno.h>    

#define BUFFER_SIZE 4096

int main(int argc, char *argv[]) {
    int src_fd, dest_fd;
    ssize_t bytes_read;
    char buffer[BUFFER_SIZE];
    char user_input;
    char tmp;

    /* Check if the user provided exactly two arguments */
    if (argc != 3) {
        const char msg[] =
            "Error: Invalid arguments.\nUsage: ./copy_my <source> <destination>\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(1);
    }

    /* Check if destination file exists using access() */
    int dest_exists = 0;
    if (access(argv[2], F_OK) == 0) {
        dest_exists = 1;
    } else {
        /* If no permission to check, it still may exist */
        if (errno == EACCES) {
            dest_exists = 1;
        } else if (errno == ENOENT) {
            dest_exists = 0;
        } else {
            const char msg[] = "Error: Failed to check destination file.\n";
            write(STDERR_FILENO, msg, sizeof(msg) - 1);
            exit(1);
        }
    }

    if (dest_exists) {
        const char prompt[] = "Target file exists. Overwrite? (y/n): ";

        while (1) {
            write(STDOUT_FILENO, prompt, sizeof(prompt) - 1);

            ssize_t r = read(STDIN_FILENO, &user_input, 1);
            if (r == 0) { /* EOF */
                const char msg[] = "Error: No input received (EOF).\n";
                write(STDERR_FILENO, msg, sizeof(msg) - 1);
                exit(1);
            }
            if (r < 0) {
                const char msg[] = "Error: Failed to read user input.\n";
                write(STDERR_FILENO, msg, sizeof(msg) - 1);
                exit(1);
            }

            /* Ignore newline */
            if (user_input == '\n' || user_input == '\r') {
                continue;
            }

            /* Consume rest of the line to avoid leftovers */
            while (read(STDIN_FILENO, &tmp, 1) == 1 && tmp != '\n') { }

            if (user_input == 'y' || user_input == 'Y') {
                break;
            } else if (user_input == 'n' || user_input == 'N') {
                const char msg[] = "Copy canceled by user.\n";
                write(STDOUT_FILENO, msg, sizeof(msg) - 1);
                exit(0);
            }
            /* Any other input -> loop repeats and re-asks */
        }
    }

    /* Open the source file for reading */
    src_fd = open(argv[1], O_RDONLY);
    if (src_fd == -1) {
        const char msg[] = "Error: Cannot open source file for reading.\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(1);
    }

    /* Open/Create the destination file for writing (overwrite) */
    dest_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd == -1) {
        const char msg[] = "Error: Cannot create or open destination file.\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        close(src_fd);
        exit(1);
    }

    /* Copy loop with buffer + handle partial writes */
    while ((bytes_read = read(src_fd, buffer, BUFFER_SIZE)) > 0) {
        ssize_t total_written = 0;

        while (total_written < bytes_read) {
            ssize_t w = write(dest_fd, buffer + total_written,
                              (size_t)(bytes_read - total_written));

            if (w < 0) {
                const char msg[] = "Error: Failed to write data to destination.\n";
                write(STDERR_FILENO, msg, sizeof(msg) - 1);
                close(src_fd);
                close(dest_fd);
                exit(1);
            }
            if (w == 0) {
                const char msg[] = "Error: write() returned 0 unexpectedly.\n";
                write(STDERR_FILENO, msg, sizeof(msg) - 1);
                close(src_fd);
                close(dest_fd);
                exit(1);
            }

            total_written += w;
        }
    }

    if (bytes_read < 0) {
        const char msg[] = "Error: Problem occurred during reading.\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        close(src_fd);
        close(dest_fd);
        exit(1);
    }

    {
        const char msg[] = "File copied successfully!\n";
        write(STDOUT_FILENO, msg, sizeof(msg) - 1);
    }

    /* Close file descriptors */
    close(src_fd);
    close(dest_fd);

    return 0;
}
