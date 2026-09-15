#include "utility.h"

char *read_whole_file_in_buffer(const char *path)
{
    if (path == NULL) {
        log_error("Path cannot be NULL");
        return NULL;
    }
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        log_error("Failed to open file: %s", path);
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) < 0) {
        log_error("Failed fseek");
        return NULL;
    }
    int file_size = ftell(file);
    rewind(file);

    char *buff = malloc(file_size * sizeof(char));
    if (buff == NULL) {
        log_error("Failed to allocate size to the buffer");
        return NULL;
    }

    size_t bytes_read = fread(buff, 1, file_size, file);
    if (bytes_read > 0) return buff;
    buff[bytes_read] = '\0';
    return NULL;
}

int write_output_file(const char *path, const char *content)
{
    FILE *file = fopen(path, "w");

    if (file == NULL) {
        perror("fopen");
        return 0;
    }

    fputs(content, file);

    fclose(file);
    return 1;
}

char* shift_args(int *argc, char ***argv)
{
    if (*argc < 1) {
        return NULL;
    }
    *argc -= 1;
    return *((*argv)++);
}
