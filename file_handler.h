#include <stdlib.h>
#include <sys/stat.h>

enum {
    MAX_PATH_BUF_SIZE = 4096,
    MAX_ENTRY_COUNT = 256,
    MAX_FILENAME_LENGTH = 256
};

size_t current_entry_count;
char entry_names[MAX_ENTRY_COUNT][MAX_FILENAME_LENGTH];

void init_path(const char *path);

int get_file_stat(const char *file_path, struct stat *current_stat);

int get_current_file_stat(struct stat *current_stat);

void normalize_path();

void add_file_to_path(const char *file_name);

void remove_last_file_from_path();

int string_cmp(const void *a, const void *b);

void load_dir_entry(int show_hidden_files);

int open_file_or_directory(int file_position);

int delete_file(int file_position);

int copy_file(const char *file_name);

int cut_file(const char *file_name);

int paste_file_from_clipboard();
