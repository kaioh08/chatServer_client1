#include "new_ncurses.h"
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define INPUT_HEIGHT 3


WINDOW *main_window, *input_window, *chat_window, *chats_window, *user_profile_window, *login_window, *register_window,
        *confirm_exit_window, *menu_window, *chat_window_box, *input_window_box, *menu_window_box;




void init_colors()
{
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_BLACK, COLOR_WHITE);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_GREEN, COLOR_BLACK);
    init_pair(5, COLOR_YELLOW, COLOR_BLACK);
    init_pair(6, COLOR_BLUE, COLOR_BLACK);
    init_pair(7, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(8, COLOR_CYAN, COLOR_BLACK);
    init_pair(9, COLOR_WHITE, COLOR_RED);
    init_pair(10, COLOR_WHITE, COLOR_GREEN);
    init_pair(11, COLOR_WHITE, COLOR_YELLOW);
    init_pair(12, COLOR_WHITE, COLOR_BLUE);
    init_pair(13, COLOR_WHITE, COLOR_MAGENTA);
    init_pair(14, COLOR_WHITE, COLOR_CYAN);
    init_pair(15, COLOR_BLACK, COLOR_WHITE);
    init_pair(16, COLOR_BLACK, COLOR_RED);
    init_pair(17, COLOR_BLACK, COLOR_GREEN);
    init_pair(18, COLOR_BLACK, COLOR_YELLOW);
    init_pair(19, COLOR_BLACK, COLOR_BLUE);
    init_pair(20, COLOR_BLACK, COLOR_MAGENTA);
    init_pair(21, COLOR_BLACK, COLOR_CYAN);
}

void init_ncurses()
{
    main_window = initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();
    init_colors();
    refresh();
}

void draw_login_window()
{
    int x, y, login_x, login_y, password_x, password_y, login_len, password_len, display_name_x, display_name_y, display_name_len;
    char login[20], password[20], display_name[20];
    MEVENT event;
    mousemask(ALL_MOUSE_EVENTS, NULL);

    login_window = subwin(main_window, LINES, COLS, 0, 0);
    wbkgd(login_window, COLOR_PAIR(1));
    wrefresh(login_window);
    keypad(login_window, TRUE);
    getmaxyx(login_window, y, x);



    login_len = strlen("Login: ");
    login_x = x / 2 - login_len;
    login_y = y / 2;
    password_len = strlen("Password: ");
    password_x = x / 2 - password_len;
    password_y = login_y + 1;

    mvprintw(login_y - 2, x / 2 - 10, "Please enter your credentials:");
    mvprintw(login_y, login_x, "Login: ");
    echo();
    mvprintw(password_y, password_x, "Password: ");
    echo();

    mvprintw(password_y + 2, x / 2 - 10, "  Login  ");
    mvprintw(password_y + 2, x / 2, "  Register  ");

    attron(A_REVERSE);
    mvprintw(password_y + 2, x / 2 - 10, "  Login  ");
    attroff(A_REVERSE);

    move(login_y, login_x + login_len);

    while (true) {
        refresh();
        int ch = getch();

        if (ch == KEY_MOUSE) {
            if (getmouse(&event) == OK) {
                if (event.bstate & BUTTON1_CLICKED) { // left mouse button clicked
                    if (event.x >= x / 2 && event.y == password_y + 2) {
                        clear();
                        draw_register_window();
                    } else if (event.x < x / 2 && event.y == password_y + 2) {
                        werase(login_window);
                        wrefresh(login_window);
                        clear();
                        draw_main_window();
                    }
                }

            }
        } else if (ch == KEY_RIGHT) {
            clear();
            draw_register_window();
        } else if (ch == '\n') {
            break;
        } else {
//            mvprintw(login_y, login_x + login_len, "%c", ch);
//            login[login_len] = ch;
//            login_len++;
        }

    }

    move(password_y, password_x + password_len);
    refresh();

    werase(login_window);
    wrefresh(login_window);
    clear();
}

void draw_register_window()
{
    int x, y, login_x, login_y, password_x, password_y, login_len, password_len, display_name_x, display_name_y, display_name_len;
    char login[20], password[20], display_name[20];
    MEVENT event;

    register_window = newwin(LINES, COLS, 0, 0);
    keypad(register_window, TRUE);
    mousemask(ALL_MOUSE_EVENTS, NULL);
    getmaxyx(register_window, y, x);

    login_len = strlen("Login: ");
    login_x = x / 2 - login_len;
    login_y = y / 2;

    password_len = strlen("Password: ");
    password_x = x / 2 - password_len;
    password_y = login_y + 1;

    display_name_len = strlen("Display name: ");
    display_name_x = x / 2 - display_name_len;
    display_name_y = password_y + 1;

    mvprintw(login_y - 2, x / 2 - 10, "Please enter your credentials:");
    mvprintw(login_y, login_x, "Login: ");
    echo();
    mvprintw(password_y, password_x, "Password: ");
    echo();
    mvprintw(display_name_y, display_name_x, "Display name: ");
    echo();

    mvprintw(display_name_y + 2, x / 2 - 10, "  Login  ");
    mvprintw(display_name_y + 2, x / 2, "  Register  ");

    // highlight register button
    attron(A_REVERSE);
    mvprintw(display_name_y + 2, x / 2, "  Register  ");
    attroff(A_REVERSE);


    while (true) {
        move(login_y, login_x + login_len);
        refresh();
        int ch = getch();

        if (ch == KEY_MOUSE) {
            if (getmouse(&event) == OK) {
                if (event.bstate & BUTTON1_CLICKED) {
                    if (event.x >= x / 2 && event.x <= x / 2 + 10 && event.y == display_name_y + 2) {
                        // Register button clicked send packet to server
                    } else if (event.x >= x / 2 - 10 && event.x <= x / 2 && event.y == display_name_y + 2) {
                        clear();
                        draw_login_window();
                    }
                }
            }
        } else if (ch == '\n') {
            break;
        } else if(ch == KEY_BACKSPACE || ch == 127 || ch == KEY_DC) {
            if(strlen(login) > 0)
                login[strlen(login) - 1] = '\0';
            if(strlen(password) > 0)
                password[strlen(password) - 1] = '\0';
            if(strlen(display_name) > 0)
                display_name[strlen(display_name) - 1] = '\0';
        } else if(ch == KEY_LEFT)
        {
            clear();
            draw_login_window();
        }
        else {
            getstr(login);

            move(password_y, password_x + password_len);
            getstr(password);

            move(display_name_y, display_name_x + display_name_len);
            getstr(display_name);

            printf("Entered login: %s\n", login);
            printf("Enter pwd %s\n", password);
            printf("Enter display name %s\n", display_name);
        }
    }

    refresh();

}

void draw_main_window()
{
    draw_chat_window();
    draw_input_window();
    draw_menu_window();
    refresh();
}

void draw_chat_window()
{
    // Create window for chat box, draw said box
    chat_window_box = subwin(main_window, (LINES * 0.8), COLS - 25, 0, 0);
    box(chat_window_box, 0, 0);
    // Draw a slick title on it
    wrefresh(chat_window_box);
    // Create sub window in box to hold text
    chat_window = subwin(chat_window_box, (LINES * 0.8 - 2), COLS - 2, 1, 1);
    // Enable text scrolling
    scrollok(chat_window, TRUE);
}

void draw_input_window()
{
    char input_text[100];

    // create sub-window for input field
    input_window_box = subwin(main_window, (LINES * 0.2) - 1, COLS, (LINES * 0.8) + 1, 0);
    box(input_window_box, 0, 0);
    input_window = subwin(input_window_box, (LINES * 0.2) - 3, COLS - 2, (LINES * 0.8) + 2, 1);
    wrefresh(input_window_box);
    keypad(input_window, TRUE);
    mousemask(ALL_MOUSE_EVENTS, NULL);

    // draw input box
    box(input_window, 0, 1);
    wrefresh(input_window);

    // draw input text
    mvwprintw(input_window, 1, 2, "Enter your message: ");
    wrefresh(input_window);

    // draw send button on the right side of the input box
    mvwprintw(input_window, 1, COLS - 10, "Send");
    wrefresh(input_window);
}

void draw_menu_window()
{
    menu_window_box = subwin(main_window, (LINES * 0.8), 25, 0, COLS - 25);
    box(menu_window_box, 0, 0);
    wrefresh(menu_window_box);
    menu_window = subwin(menu_window_box, (LINES * 0.8) - 2, 23, 1, COLS - 24);
    keypad(menu_window, TRUE);
    mousemask(ALL_MOUSE_EVENTS, NULL);

    // draw menu items
    mvwprintw(menu_window, 1, 2, "My chats");
    mvwprintw(menu_window, 3, 2, "Online users");
    mvwprintw(menu_window, 5, 2, "Public chats");
    mvwprintw(menu_window, 7, 2, "Create chat");
    mvwprintw(menu_window, 9, 2, "Settings");
    wrefresh(menu_window);

}

int handle_input()
{
    char input_text[100];
    // handle mouse events
    echo();
    MEVENT event;
    while (true) {
        // move cursor to the input box
        wmove(input_window, 1, 22); // move to the beginning of the input box
        wgetstr(input_window, input_text);
        wrefresh(input_window);
        int ch = wgetch(input_window);
        if (ch == KEY_MOUSE) {
            if (getmouse(&event) == OK) {
                if (event.bstate & BUTTON1_CLICKED) {
                    if (event.x >= COLS - 10 && event.x <= COLS && event.y == 1) {
                        // Send button clicked send packet to server
                    }
                }
            }
        } else if (ch == '\n')
        {
            // Send packet to server
//            send_packet(input_text);
            // Clear input box
            memset(input_text, 0, sizeof(input_text));
            mvwprintw(input_window, 1, 22, "%s", input_text);
            wrefresh(input_window);
            break;
        } else if(ch == KEY_BACKSPACE || ch == 127 || ch == KEY_DC) {
            if(strlen(input_text) > 0)
                input_text[strlen(input_text) - 1] = '\0';
        } else {
            if (strlen(input_text) < 98) { // prevent buffer overflow
                input_text[strlen(input_text)] = ch;
                input_text[strlen(input_text) + 1] = '\0';
                mvwprintw(input_window, 1, 22, "%s", input_text);
                wrefresh(input_window);
            }
        }
    }
    return 0;
}

int main()
{
    init_ncurses();
    draw_login_window();
    draw_main_window();
    while(1)
    {
        handle_input();
    }
}