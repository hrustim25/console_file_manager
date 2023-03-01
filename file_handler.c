#include "file_handler.h"

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

size_t current_path_length;
char current_path[MAX_PATH_BUF_SIZE];

void init_path(const char *path) {
    snprintf(current_path, MAX_PATH_BUF_SIZE, "%s", path);
    normalize_path();
    current_path_length = strlen(current_path);
}

struct stat get_file_stat(const char *file_name) {
    add_file_to_path(file_name);

    struct stat current_stat;
    int status = stat(current_path, &current_stat);
    if (status == -1) {
        error_exit("Error on stat syscall occurred");
    }

    remove_last_file_from_path();

    return current_stat;
}

void normalize_path() {
    char temp_path_buffer[MAX_PATH_BUF_SIZE];
    memset(temp_path_buffer, '\0', MAX_PATH_BUF_SIZE);
    char *result = realpath(current_path, temp_path_buffer);
    if (result == NULL) {
        error_exit("Error on getting absolute path");
    }
    memcpy(current_path, temp_path_buffer, MAX_PATH_BUF_SIZE);
    current_path_length = strlen(current_path);
}

void add_file_to_path(const char *file_name) {
    if (current_path[current_path_length - 1] != '/') {
        current_path[current_path_length] = '/';
        ++current_path_length;
    }
    strcpy(current_path + current_path_length, file_name);
    current_path_length += strlen(file_name);
}

void remove_last_file_from_path() {
    if (current_path[current_path_length - 1] == '/') {
        current_path[current_path_length - 1] = '\0';
        --current_path_length;
    }
    while (current_path_length > 0) {
        if (current_path[current_path_length - 1] == '/') {
            break;
        }
        --current_path_length;
        current_path[current_path_length] = '\0';
    }
}

int string_cmp(const void *a, const void *b) {
    const char *str1 = a;
    const char *str2 = b;
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);
    for (size_t i = 0; i < len1 && i < len2; ++i) {
        if (str1[i] < str2[i]) {
            return -1;
        } else if (str1[i] > str2[i]) {
            return 1;
        }
    }
    return len1 - len2;
}

void load_dir_entry(int show_hidden_files) {
    current_entry_count = 0;

    DIR *dir = opendir(current_path);
    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (current_entry_count == MAX_ENTRY_COUNT) {
            break;
        }
        if (strcmp(entry->d_name, ".") == 0) {
            continue;
        }
        if (!show_hidden_files && entry->d_name[0] == '.' &&
            strcmp(entry->d_name, "..") != 0) {
            continue;
        }
        strcpy(entry_names[current_entry_count], entry->d_name);
        ++current_entry_count;
    }
    closedir(dir);

    qsort(entry_names, current_entry_count, sizeof(*entry_names), string_cmp);
}

int open_file_or_directory(int file_position) {
    struct stat current_stat = get_file_stat(entry_names[file_position]);

    add_file_to_path(entry_names[file_position]);
    normalize_path();

    if (S_ISDIR(current_stat.st_mode)) {
        return 1;
    } else {
        current_path_length -= strlen(entry_names[file_position]);
        memset(current_path + current_path_length, '\0',
               strlen(entry_names[file_position]));
    }
    return 0;
}

int delete_file(int file_position) {
    struct stat current_stat = get_file_stat(entry_names[file_position]);

    add_file_to_path(entry_names[file_position]);
    normalize_path();

    if (S_ISDIR(current_stat.st_mode)) {
        remove_last_file_from_path();
        return 0;
    }

    int status = unlink(current_path);
    if (status == -1) {
        error_exit("Error on delete file operation");
    }

    remove_last_file_from_path();

    return 1;
}
