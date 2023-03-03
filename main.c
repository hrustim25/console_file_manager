#include "file_handler.h"

#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

enum { MODIFY_TIME_DISPLAY_SHIFT = 20 };

enum {
    REG_FILE_COLOR_PAIR = 0,
    DIR_COLOR_PAIR = 1,
    SYM_LINK_COLOR_PAIR = 2,
    FIFO_COLOR_PAIR = 3
};

size_t window_width;
size_t window_height;

int cursor_position;
int left_entry_pos_bound;
int right_entry_pos_bound;

int is_showing_hidden_files = 0;

int get_file_attribute(int file_position, struct stat file_stat) {
    int result = 0;
    if (strcmp(entry_names[file_position], "..") == 0) {
        result = COLOR_PAIR(DIR_COLOR_PAIR);
    } else {
        if (S_ISREG(file_stat.st_mode)) {
            result = COLOR_PAIR(REG_FILE_COLOR_PAIR);
        } else if (S_ISDIR(file_stat.st_mode)) {
            result = COLOR_PAIR(DIR_COLOR_PAIR);
        } else if (S_ISLNK(file_stat.st_mode)) {
            result = COLOR_PAIR(SYM_LINK_COLOR_PAIR);
        } else if (S_ISFIFO(file_stat.st_mode)) {
            result = COLOR_PAIR(FIFO_COLOR_PAIR);
        }
    }
    if (file_position == cursor_position) {
        result |= A_REVERSE;
    }
    return result;
}

void display_content() {
    mvprintw(0, 0, "%s", "Name");
    mvprintw(0, window_width - MODIFY_TIME_DISPLAY_SHIFT, "%s", "Modified");

    int displayed_lines = 1;
    for (int i = 0; i < current_entry_count; ++i) {
        if (displayed_lines >= window_height) {
            break;
        }
        if (i < left_entry_pos_bound || i > right_entry_pos_bound) {
            continue;
        }

        add_file_to_path(entry_names[i]);
        struct stat file_stat;
        if (get_current_file_stat(&file_stat)) {
            return;
        }
        remove_last_file_from_path();
        int file_attribute = get_file_attribute(i, file_stat);

        attrset(file_attribute);
        if (strcmp(entry_names[i], "..") == 0) {
            mvprintw(displayed_lines, 0, "%s", entry_names[i]);
        } else {
            char result_string[window_width + 1];
            memset(result_string, ' ', window_width);
            result_string[window_width] = '\0';
            int file_name_print_bound =
                window_width - MODIFY_TIME_DISPLAY_SHIFT - 1;
            if (strlen(entry_names[i]) < file_name_print_bound) {
                file_name_print_bound = strlen(entry_names[i]);
            }
            strncpy(result_string, entry_names[i], file_name_print_bound);
            strftime(result_string + window_width - MODIFY_TIME_DISPLAY_SHIFT,
                     MODIFY_TIME_DISPLAY_SHIFT, "%H:%M:%S %d-%m-%Y",
                     localtime(&file_stat.st_mtime));

            mvprintw(displayed_lines, 0, "%s", result_string);
        }
        attroff(file_attribute);

        ++displayed_lines;
    }
}

void init_ncurses() {
    initscr();
    keypad(stdscr, TRUE);
    curs_set(0);
    noecho();
    start_color();

    init_pair(REG_FILE_COLOR_PAIR, COLOR_WHITE, COLOR_BLACK);
    init_pair(DIR_COLOR_PAIR, COLOR_BLUE, COLOR_BLACK);
    init_pair(SYM_LINK_COLOR_PAIR, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(FIFO_COLOR_PAIR, COLOR_YELLOW, COLOR_BLACK);
}

void init_parameters() {
    window_height = LINES;
    window_width = COLS;

    cursor_position = 0;
    left_entry_pos_bound = 0;
    right_entry_pos_bound = window_height - 1;
}

void handle_keypress(int ch) {
    if (ch == 'q') {
        endwin();
        exit(0);
    }

    int status;
    switch (ch) {
    case 'h':
        is_showing_hidden_files ^= 1;
        cursor_position = 0;
        left_entry_pos_bound = 0;
        right_entry_pos_bound = window_height - 1;
        break;
    case KEY_UP:
        cursor_position =
            (cursor_position - 1 + current_entry_count) % current_entry_count;
        if (cursor_position < left_entry_pos_bound) {
            --left_entry_pos_bound;
            --right_entry_pos_bound;
        }
        if (cursor_position == current_entry_count - 1) {
            left_entry_pos_bound = current_entry_count - window_height + 1;
            right_entry_pos_bound = current_entry_count - 1;
        }
        break;
    case KEY_DOWN:
        cursor_position = (cursor_position + 1) % current_entry_count;
        if (cursor_position > right_entry_pos_bound) {
            ++left_entry_pos_bound;
            ++right_entry_pos_bound;
        }
        if (cursor_position == 0) {
            left_entry_pos_bound = 0;
            right_entry_pos_bound = window_height - 2;
        }
        break;
    case '\n':
        status = open_file_or_directory(cursor_position);
        if (status == 1) {
            cursor_position = 0;
            left_entry_pos_bound = 0;
            right_entry_pos_bound = window_height - 2;
        }
        break;
    case 'd':
        status = delete_file(cursor_position);
        if (status) {
            --cursor_position;
        }
        break;
    case 'c':
        status = copy_file(entry_names[cursor_position]);
        break;
    case 'x':
        status = cut_file(entry_names[cursor_position]);
        break;
    case 'v':
        status = paste_file_from_clipboard();
        break;
    }
}

int main(int argc, char *argv[]) {
    if (argc > 2) {
        printf("%s\n", "Excess arguments");
        exit(1);
    }

    if (argc == 1) {
        init_path(".");
    } else if (argc == 2) {
        init_path(argv[1]);
    }

    init_ncurses();

    init_parameters();

    while (1) {
        erase();
        load_dir_entry(is_showing_hidden_files);
        display_content();
        refresh();

        int ch = getch();

        handle_keypress(ch);
    }

    return 0;
}
