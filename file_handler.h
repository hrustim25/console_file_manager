#include <stdlib.h>

enum {
    MAX_PATH_BUF_SIZE = 4096,
    MAX_ENTRY_COUNT = 256,
    MAX_FILENAME_LENGTH = 256
};

size_t current_entry_count;
char entry_names[MAX_ENTRY_COUNT][MAX_FILENAME_LENGTH];

void error_exit(const char *msg);

void init_path(const char *path);

struct stat get_file_stat(const char *file_name);

void normalize_path();

void add_file_to_path(const char *file_name);

void remove_last_file_from_path();

int string_cmp(const void *a, const void *b);

void load_dir_entry(int show_hidden_files);

int open_file_or_directory(int file_position);

int delete_file(int file_position);
