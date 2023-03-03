#include "file_handler.h"

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

size_t current_path_length;
char current_path[MAX_PATH_BUF_SIZE];

int is_copy = -1;
char copy_cut_path[MAX_PATH_BUF_SIZE];
char copy_cut_file_name[MAX_PATH_BUF_SIZE];

void init_path(const char *path) {
    snprintf(current_path, MAX_PATH_BUF_SIZE, "%s", path);
    normalize_path();
    current_path_length = strlen(current_path);
}

int get_file_stat(const char *file_path, struct stat *current_stat) {
    int status = lstat(file_path, current_stat);
    if (status == -1) {
        return 1;
    }
    return 0;
}

int get_current_file_stat(struct stat *current_stat) {
    return get_file_stat(current_path, current_stat);
}

void normalize_path() {
    char temp_path_buffer[MAX_PATH_BUF_SIZE];
    memset(temp_path_buffer, '\0', MAX_PATH_BUF_SIZE);
    char *result = realpath(current_path, temp_path_buffer);
    if (result == NULL) {
        return;
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
    add_file_to_path(entry_names[file_position]);
    normalize_path();

    struct stat current_stat;
    if (get_file_stat(current_path, &current_stat)) {
        return 0;
    }

    if (S_ISDIR(current_stat.st_mode)) {
        if (current_stat.st_mode & S_IRUSR) {
            return 1;
        } else {
            remove_last_file_from_path();
            return 0;
        }
    } else {
        remove_last_file_from_path();
    }
    return 0;
}

int delete_file(int file_position) {
    if (strcmp(entry_names[file_position], "..") == 0) {
        return 0;
    }
    add_file_to_path(entry_names[file_position]);
    normalize_path();

    struct stat current_stat;
    if (get_file_stat(current_path, &current_stat)) {
        return 0;
    }

    if (S_ISDIR(current_stat.st_mode)) {
        remove_last_file_from_path();
        return 0;
    }

    int status = unlink(current_path);
    if (status == -1) {
        return 0;
    }

    remove_last_file_from_path();

    return 1;
}

int copy_file(const char *file_name) {
    if (strcmp(file_name, "..") == 0) {
        return 1;
    }
    add_file_to_path(file_name);

    struct stat current_stat;
    if (get_file_stat(current_path, &current_stat)) {
        return 0;
    }
    if (!S_ISREG(current_stat.st_mode)) {
        remove_last_file_from_path();
        return 1;
    }

    strcpy(copy_cut_path, current_path);
    remove_last_file_from_path();
    strcpy(copy_cut_file_name, file_name);
    is_copy = 1;
    return 0;
}

int cut_file(const char *file_name) {
    if (strcmp(file_name, "..") == 0) {
        return 1;
    }
    add_file_to_path(file_name);

    struct stat current_stat;
    if (get_file_stat(current_path, &current_stat)) {
        return 0;
    }
    if (!S_ISREG(current_stat.st_mode)) {
        remove_last_file_from_path();
        return 1;
    }

    strcpy(copy_cut_path, current_path);
    remove_last_file_from_path();
    strcpy(copy_cut_file_name, file_name);
    is_copy = 0;
    return 0;
}

int paste_file_from_clipboard() {
    if (is_copy == -1) {
        return 0;
    }

    int first_file_fd = open(copy_cut_path, O_RDONLY);
    if (first_file_fd == -1) {
        return 1;
    }
    struct stat first_file_stat;
    if (get_file_stat(copy_cut_path, &first_file_stat)) {
        return 0;
    }
    add_file_to_path(copy_cut_file_name);
    if (is_copy) {
        int second_file_fd = open(current_path, O_WRONLY | O_CREAT | O_EXCL,
                                  first_file_stat.st_mode);
        if (second_file_fd == -1) {
            remove_last_file_from_path();
            return 1;
        }
        char read_buffer[MAX_FILENAME_LENGTH];
        int result;
        while ((result = read(first_file_fd, read_buffer,
                              MAX_FILENAME_LENGTH)) > 0) {
            int written = 0;
            while (written < result) {
                int status = write(second_file_fd, read_buffer + written,
                                   result - written);
                written += status;
            }
        }
        close(second_file_fd);
        close(first_file_fd);
    } else {
        int second_file_fd =
            open(current_path, O_CREAT | O_EXCL, first_file_stat.st_mode);
        if (second_file_fd == -1) {
            remove_last_file_from_path();
            return 1;
        }
        close(second_file_fd);
        close(first_file_fd);
        rename(copy_cut_path, current_path);
    }
    remove_last_file_from_path();
    is_copy = -1;
    return 0;
}
