#include <ncurses.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <form.h>
#include <ctype.h>

#define INPUT_HEIGHT 3
#define MENU_WIDTH 30
#define MENU_ITEMS 5

bool menu_focused = false;
int menu_highlight = 0;
bool menu_active = true;


pthread_mutex_t mutex;

WINDOW *menu_win, *chat_win, *input_win, *login_win, *register_win;

void init_windows();
void init_menu();
void init_chat();
void init_input();
void* input_handler(void* arg);
void* message_handler(void* arg);
void draw_menu(int highlight);
void display_settings();
void draw_login_window();
void draw_register_window();
void quit();

int main() {
    stdscr = initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLUE);

    pthread_mutex_init(&mutex, NULL);

    draw_login_window();
//    init_windows();

    pthread_t input_thread, message_thread;
    pthread_create(&input_thread, NULL, input_handler, NULL);
    pthread_create(&message_thread, NULL, message_handler, NULL);


    pthread_join(input_thread, NULL);
    pthread_join(message_thread, NULL);

    pthread_mutex_destroy(&mutex);
    endwin();

    return 0;
}

void init_windows() {
    init_menu();
    init_chat();
    init_input();
}
void apply_highlight(WINDOW *win, int y, int x, const char *str) {
    wattron(win, A_REVERSE);
    mvwprintw(win, y, x, str);
    wattroff(win, A_REVERSE);
}

void remove_highlight(WINDOW *win, int y, int x, const char *str) {
    mvwprintw(win, y, x, str);
}

void draw_login_window()
{
    int x, y, login_x, login_y, password_x, password_y, login_len, password_len, display_name_x, display_name_y, display_name_len;
    char login[20], password[20], display_name[20];
    MEVENT event;
    mousemask(ALL_MOUSE_EVENTS, NULL);

    login_win = newwin(LINES, COLS, 0, 0);
    wbkgd(login_win, COLOR_PAIR(1));
    wrefresh(login_win);
    keypad(login_win, TRUE);
    getmaxyx(login_win, y, x);

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
                        // Get login and password in the format login ETX password ETX
                        werase(login_win);
                        wrefresh(login_win);
                        clear();
                        noecho();
                        init_windows();
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

    werase(login_win);
    wrefresh(login_win);
    clear();
}

void draw_register_window()
{
    int x, y, login_x, login_y, password_x, password_y, login_len, password_len, display_name_x, display_name_y, display_name_len;
    char login[20], password[20], display_name[20];
    MEVENT event;
    char *body;

    register_win = newwin(LINES, COLS, 0, 0);
    keypad(register_win, TRUE);
    mousemask(ALL_MOUSE_EVENTS, NULL);
    getmaxyx(register_win, y, x);

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

    // concat login ETX password ETX display_name ETX
    body = malloc(strlen(login) + strlen(password) + strlen(display_name) + 3);
    strcpy(body, login);
    strcat(body, "\3");
    strcat(body, password);
    strcat(body, "\3");
    strcat(body, display_name);
    strcat(body, "\3");

    while (true)
    {
        move(login_y, login_x + login_len);
        refresh();
        int ch = getch();

        if (ch == KEY_MOUSE)
        {
            if (getmouse(&event) == OK)
            {
                if (event.bstate & BUTTON1_CLICKED)
                {
                    if (event.x >= x / 2 && event.x <= x / 2 + 10 && event.y == display_name_y + 2)
                    {
                        // Register button clicked send packet to server in the following format: login ETX password ETX display_name ETX
//                        send_create_user(struct dc_env *env, struct dc_error *err, int fd, const char * body)
                        clear();
                        draw_login_window();
                    }
                    else if (event.x >= x / 2 - 10 && event.x <= x / 2 && event.y == display_name_y + 2)
                    {
                        clear();
                        draw_login_window();
                    }
                }
            }
        } else if (ch == '\n')
        {
            break;
        } else if (ch == KEY_BACKSPACE || ch == 127 || ch == KEY_DC)
        {
            if (strlen(login) > 0)
                login[strlen(login) - 1] = '\0';
            if (strlen(password) > 0)
                password[strlen(password) - 1] = '\0';
            if (strlen(display_name) > 0)
                display_name[strlen(display_name) - 1] = '\0';
            // update the body variable
            body = realloc(body, strlen(login) + strlen(password) + strlen(display_name) + 3);
            strcpy(body, login);
            strcat(body, "\3");
            strcat(body, password);
            strcat(body, "\3");
            strcat(body, display_name);
            strcat(body, "\3");
        } else if (ch == KEY_LEFT)
        {
            clear();
            draw_login_window();
        } else
        {
            getstr(login);
            move(password_y, password_x + password_len);
            getstr(password);

            move(display_name_y, display_name_x + display_name_len);
            getstr(display_name);

            // update the body variable
            body = realloc(body, strlen(login) + strlen(password) + strlen(display_name) + 3);
            strcpy(body, login);
            strcat(body, "\3");
            strcat(body, password);
            strcat(body, "\3");
            strcat(body, display_name);
            strcat(body, "\3");
//            printf("%s", body);
        }
    }

    refresh();
}

void create_new_chat() {
    WINDOW *create_chat_win;
    menu_focused = false;
    int create_chat_win_width = 50;
    int create_chat_win_height = 12;
    int startx = (COLS - create_chat_win_width) / 2;
    int starty = (LINES - create_chat_win_height) / 2;

    create_chat_win = newwin(create_chat_win_height, create_chat_win_width, starty, startx);
    box(create_chat_win, 0, 0);
    wrefresh(create_chat_win);

    // Fields
    FIELD *field[4];
    field[0] = new_field(1, 20, 1, 18, 0, 0); // Chat name
    field[1] = new_field(1, 1, 3, 18, 0, 0);  // Private (y/n)
    field[2] = new_field(1, 20, 5, 18, 0, 0); // Password
    field[3] = NULL;

    set_field_back(field[0], A_UNDERLINE);
    set_field_back(field[1], A_UNDERLINE);
    set_field_back(field[2], A_UNDERLINE);

    FORM *form = new_form(field);
    set_form_win(form, create_chat_win);
    set_form_sub(form, derwin(create_chat_win, 8, create_chat_win_width - 4, 2, 2));

    post_form(form);
    wrefresh(create_chat_win);

    // Labels
    mvwprintw(create_chat_win, 2, 2, "Chat Name:");
    mvwprintw(create_chat_win, 4, 2, "Private (y/n):");
    mvwprintw(create_chat_win, 6, 2, "Password:");

    // Buttons
    mvwprintw(create_chat_win, 9, 10, "[Save]");
    mvwprintw(create_chat_win, 9, 28, "[Back]");

    bool quit = false;
    int ch;
    int current_field = 0;
    bool button_focused = false;
    form_driver(form, REQ_FIRST_FIELD);

    keypad(create_chat_win, TRUE); // Enable keypad mode to capture arrow keys
    MEVENT event;
    while (!quit) {
        ch = wgetch(create_chat_win);
        // Get the cursor position
        int cur_y, cur_x;
        getyx(create_chat_win, cur_y, cur_x);
        if(!button_focused)
        {
            set_field_back(field[current_field], A_REVERSE);
            remove_highlight(create_chat_win, 9, 10, "[Save]");
            remove_highlight(create_chat_win, 9, 28, "[Back]");
            switch (ch)
            {
                // if escape is pressed, quit the loop
                case 27:
                    quit = true;
                    break;
                case KEY_DOWN:
                    if (current_field < 2)
                    {
                        form_driver(form, REQ_NEXT_FIELD);
                        form_driver(form, REQ_END_LINE);
                        current_field++;
                    } else
                    {
                        button_focused = true;
                        wmove(create_chat_win, 9, 10); // Move cursor to "Save" button
                    }
                    break;
                case KEY_UP:
                    if (current_field > 0)
                    {
                        form_driver(form, REQ_PREV_FIELD);
                        form_driver(form, REQ_END_LINE);
                        current_field--;
                    }
                    break;
                case KEY_LEFT:
                    form_driver(form, REQ_PREV_CHAR);
                    break;
                case KEY_RIGHT:
                    form_driver(form, REQ_NEXT_CHAR);
                    break;
                case KEY_BACKSPACE:
                    form_driver(form, REQ_DEL_PREV);
                    break;
                default:
                    form_driver(form, ch);
                    break;
            }
        } else {
            // Remove highlight from the fields
            for (int i = 0; i < 3; i++) {
                set_field_back(field[i], A_UNDERLINE);
            }
            switch (ch) {
                case KEY_LEFT:
                    if (cur_x > 15) {
                        apply_highlight(create_chat_win, 9, 10, "[Save]");
                        remove_highlight(create_chat_win, 9, 28, "[Back]");
                    }
                    break;
                case KEY_RIGHT:
                    if (cur_x < 28) {
                        remove_highlight(create_chat_win, 9, 10, "[Save]");
                        apply_highlight(create_chat_win, 9, 28, "[Back]");
                    }
                    break;
                case KEY_UP:
                    button_focused = false;
                    form_driver(form, REQ_LAST_FIELD);
                    form_driver(form, REQ_END_LINE);
                    current_field = 2;
                    break;
                case KEY_ENTER:
                case '\n':
                case '\r':
                    if (cur_x >= 10 && cur_x <= 15) {
                        // Save button
                        // TODO: Send data to the server
                        char *chat_name = field_buffer(field[0], 0);
                        char *is_private = field_buffer(field[1], 0);
                        char *password = field_buffer(field[2], 0);
                        wprintw(chat_win, " Chat name: %s\n is private: %s\n password: %s\n", chat_name, is_private, password);
                        // You can process the data and send it to the server here
                        quit = true;
                    } else if (cur_x >= 28 && cur_x <= 33) {
                        // Back button
                        quit = true;
                    }
                    break;
                default:
                    break;
            }
        }
        // Refresh the window to apply the attribute changes
        wrefresh(create_chat_win);
    }

    // Clean up
    unpost_form(form);
    free_form(form);
    for (int i = 0; i < 3; i++) {
        free_field(field[i]);
    }

    // Return focus to the menu window
    delwin(create_chat_win);
    touchwin(menu_win);
    wrefresh(menu_win);
    touchwin(chat_win);
    wrefresh(chat_win);
    touchwin(input_win);
    wrefresh(input_win);
//    draw_menu(0);
    menu_focused = true;
}

void show_active_chats()
{
    int startx, starty, width, height;
    height = 20;
    width = 50;
    starty = (LINES - height) / 2;
    startx = (COLS - width) / 2;
    WINDOW *list_of_users_win = newwin(20, 50, starty, startx);
    box(list_of_users_win, 0, 0);
    wrefresh(list_of_users_win);

    for(int i = 0; i < 10; i++)
    {
        mvwprintw(list_of_users_win, i+1, 1, "Chat %d", i);
    }

    int ch;
    bool quit = false;
    keypad(list_of_users_win, TRUE);

    while (!quit) {
        ch = wgetch(list_of_users_win);
        switch (ch) {
            case 27:
                quit = true;
                break;
        }
    }
    delwin(list_of_users_win);
    touchwin(menu_win);
    wrefresh(menu_win);
    touchwin(chat_win);
    wrefresh(chat_win);
    touchwin(input_win);
    wrefresh(input_win);
    menu_focused = true;
}

void show_active_users()
{
    int startx, starty, width, height;
    height = 20;
    width = 50;
    starty = (LINES - height) / 2;
    startx = (COLS - width) / 2;
    WINDOW *active_users_win = newwin(20, 50, starty, startx);
    box(active_users_win, 0, 0);
    wrefresh(active_users_win);

    for(int i = 0; i < 10; i++)
    {
        mvwprintw(active_users_win, i+1, 1, "User %d", i);
    }

    int ch;
    bool quit = false;
    keypad(active_users_win, TRUE);

    while (!quit) {
        ch = wgetch(active_users_win);
        switch (ch) {
            case 27:
                quit = true;
                break;
        }
    }
    delwin(active_users_win);
    touchwin(menu_win);
    wrefresh(menu_win);
    touchwin(chat_win);
    wrefresh(chat_win);
    touchwin(input_win);
    wrefresh(input_win);
    menu_focused = true;
}

void display_settings() {
    WINDOW *settings_win;
    int settings_win_width = 50;
    int settings_win_height = 10;
    int startx = (COLS - settings_win_width) / 2;
    int starty = (LINES - settings_win_height) / 2;

    settings_win = newwin(settings_win_height, settings_win_width, starty, startx);
    box(settings_win, 0, 0);
    wrefresh(settings_win);

    // Fields
    FIELD *field[3];
    field[0] = new_field(1, 20, 1, 18, 0, 0); // Name
    field[1] = new_field(1, 20, 3, 18, 0, 0); // Password
    field[2] = NULL;

    set_field_back(field[0], A_UNDERLINE);
    set_field_back(field[1], A_UNDERLINE);

    FORM *form = new_form(field);
    set_form_win(form, settings_win);
    set_form_sub(form, derwin(settings_win, settings_win_height - 4, settings_win_width - 4, 2, 2));

    post_form(form);
    wrefresh(settings_win);

    // Labels
    mvwprintw(settings_win, 2, 2, "Name:");
    mvwprintw(settings_win, 4, 2, "Password:");

    // Buttons
    mvwprintw(settings_win, 7, 10, "[Save]");
    mvwprintw(settings_win, 7, 28, "[Back]");

    bool quit = false;
    int ch;
    int current_field = 0;
    bool button_focused = false;
    form_driver(form, REQ_FIRST_FIELD);

    keypad(settings_win, TRUE); // Enable keypad mode to capture arrow keys

    while (!quit) {
        ch = wgetch(settings_win);
        int cur_y, cur_x;
        getyx(settings_win, cur_y, cur_x);

        if (!button_focused) {
            // Handle arrow keys
            switch (ch) {
                case KEY_DOWN:
                    form_driver(form, REQ_NEXT_FIELD);
                    form_driver(form, REQ_END_LINE);
                    current_field++;
                    break;
                case KEY_UP:
                    form_driver(form, REQ_PREV_FIELD);
                    form_driver(form, REQ_END_LINE);
                    current_field--;
                    break;
                case KEY_LEFT:
                    form_driver(form, REQ_PREV_CHAR);
                    break;
                case KEY_RIGHT:
                    form_driver(form, REQ_NEXT_CHAR);
                    break;
                case 27: // ESC
                    quit = true;
                    break;
                case 10: // Enter
                    if (current_field == 2) {
                        button_focused = true;
                        mvwprintw(settings_win, 7, 10, " Save ");
                        mvwprintw(settings_win, 7, 28, " Back ");
                        wmove(settings_win, 7, 10);
                    }
                    break;
                default:
                    form_driver(form, ch);
                    break;
            }
        } else {
            switch (ch) {
                case KEY_LEFT:
                    if (cur_x == 28) {
                        mvwprintw(settings_win, 7, 10, " Save ");
                        mvwprintw(settings_win, 7, 28, " Back ");
                        wmove(settings_win, 7, 10);
                    }
                    break;
                case KEY_RIGHT:
                    if (cur_x == 10) {
                        mvwprintw(settings_win, 7, 10, " Save ");
                        mvwprintw(settings_win, 7, 28, " Back ");
                        wmove(settings_win, 7, 28);
                    }
                    break;
                case 10: // Enter
                    if (cur_x == 10) {
                        // Save
                        quit = true;
                    } else if (cur_x == 28) {
                        // Back
                        quit = true;
                    }
                    break;
                case 27: // ESC
                    quit = true;
                    break;
            }
        }

        wrefresh(settings_win);
    }

    // Clean up
    unpost_form(form);
    free_form(form);
    for (int i = 0; i < 2; i++) {
        free_field(field[i]);
    }

    // Close the settings window
    delwin(settings_win);
    touchwin(menu_win);
    wrefresh(menu_win);
    touchwin(chat_win);
    wrefresh(chat_win);
    touchwin(input_win);
    wrefresh(input_win);
    menu_focused = true;
}

void handle_menu_selection(int choice) {
    switch (choice) {
        case 0: // Create new chat
            create_new_chat();
            break;
        case 1: // Show list of active chats
            show_active_chats();
            break;
        case 2: // Show active users
            show_active_users();
            break;
        case 3: // Settings
            display_settings();
            break;
        case 4:
            quit();
            break;
    }
}

void draw_menu(int highlight) {
    const char *choices[] = {
            "Create new chat",
            "Show list of active chats",
            "Show active users",
            "Settings",
            "Quit",
    };

    for (int i = 0; i < MENU_ITEMS; i++) {
        if (highlight == i) {
            wattron(menu_win, A_REVERSE);
        }
        mvwprintw(menu_win, i + 1, 1, "%s", choices[i]);
        wattroff(menu_win, A_REVERSE);
    }
    wrefresh(menu_win);
}

void init_menu() {
    menu_win = newwin(LINES - INPUT_HEIGHT, MENU_WIDTH, 0, 0);
    wbkgd(menu_win, COLOR_PAIR(1));
    box(menu_win, 0, 0);
    wrefresh(menu_win);
    draw_menu(0);
//    refresh();
}

void init_chat() {
    chat_win = newwin(LINES - INPUT_HEIGHT, COLS - MENU_WIDTH, 0, MENU_WIDTH);
    scrollok(chat_win, TRUE);
    box(chat_win, 0, 0);
    wrefresh(chat_win);
}

void init_input() {
    input_win = newwin(INPUT_HEIGHT, COLS, LINES - INPUT_HEIGHT, 0);
    keypad(input_win, TRUE);
    scrollok(input_win, TRUE);
    box(input_win, 0, 0);
    wrefresh(input_win);
    mvwprintw(input_win, 1, 1, "Enter message: ");
    wrefresh(input_win);
    nodelay(input_win, TRUE);
}

void* input_handler(void* arg) {
    int ch;
    bool quit = false;
    int input_idx = 0;
    char input_buffer[COLS - 2];
    memset(input_buffer, 0, sizeof(input_buffer));
    draw_menu(menu_highlight);
    while (!quit) {
        ch = getch();

        pthread_mutex_lock(&mutex);

        if (menu_focused && menu_active) {
            // clear input window
            werase(input_win);
            box(input_win, 0, 0);
            wrefresh(input_win);
            switch (ch) {
                case KEY_UP:
                    if (menu_highlight > 0) {
                        menu_highlight--;
                    }
                    break;
                case KEY_DOWN:
                    if (menu_highlight < MENU_ITEMS - 1) {
                        menu_highlight++;
                    }
                    break;
                case KEY_ENTER:
                case '\n':
                case '\r':
                    handle_menu_selection(menu_highlight);
                    break;
                case '\t': // Press 'Tab' to switch focus between input and menu
                    menu_focused = !menu_focused;
                    break;
                default:
                    break;
            }

            draw_menu(menu_highlight);
        } else {
            // Enter message
            mvwprintw(input_win, 1, 1, "Enter message: ");
            wrefresh(input_win);
            switch (ch) {
                case KEY_ENTER:
                case '\n':
                case '\r':
                    // TODO: Handle sending the message
                    werase(input_win);
                    box(input_win, 0, 0);
                    wrefresh(input_win);
                    input_idx = 0;
                    memset(input_buffer, 0, sizeof(input_buffer));
                    mvwprintw(input_win, 1, 1, "Enter message: ");
                    wrefresh(input_win);
                    break;
                case KEY_BACKSPACE:
                case KEY_DC:
                case 127: // Backspace key
                    if (input_idx > 0) {
                        input_idx--;
                        input_buffer[input_idx] = '\0';
                        werase(input_win);
                        box(input_win, 0, 0);
                        mvwprintw(input_win, 1, 16, "%s", input_buffer);
                        wrefresh(input_win);
                    }
                    break;
                case '\t': // Press 'Tab' to switch focus between input and menu
                    menu_focused = !menu_focused;
                    break;
                case KEY_UP:
                    break;
                case KEY_DOWN:
                    break;
                case KEY_LEFT:
                    break;
                case KEY_RIGHT:
                    break;
                default:
                    if (input_idx < COLS - 3) {
                        input_buffer[input_idx] = ch;
                        input_idx++;
                        mvwprintw(input_win, 1, 16, "%s", input_buffer);
                        wrefresh(input_win);
                    }
                    break;
            }
        }

        pthread_mutex_unlock(&mutex);

        // Sleep to prevent high CPU usage
        usleep(10000);
    }

    return NULL;
}


void* message_handler(void* arg) {
    while (true) {
        // TODO: Implement message sending and receiving
        usleep(100000); // Sleep to prevent high CPU usage
    }

    return NULL;
}

void quit()
{
    endwin();
    exit(0);
}

//#include <ncurses.h>
//#include <pthread.h>
//#include <string.h>
//#include <sys/socket.h>
//#include <unistd.h>
//
//#define CHAT_HEIGHT 3*(LINES/4)
//#define CHAT_WIDTH COLS
//#define INPUT_HEIGHT LINES - CHAT_HEIGHT
//#define INPUT_WIDTH COLS
//#define MENU_HEIGHT LINES
//#define MENU_WIDTH COLS/4
//
//int next_chat_line = 1;
//
//WINDOW *chat_win;
//WINDOW *input_win;
//WINDOW *menu_win;
//
//void enter_chat_room(int number);
//
//void init_windows() {
//    // Calculate window dimensions
//    int chat_width = 3 * (COLS / 4);
//    int input_width = COLS;
//    int menu_width = COLS - chat_width;
//
//    // Debugging output
//    printw("Screen width: %d\n", COLS);
//    printw("Chat width: %d\n", chat_width);
//    printw("Menu width: %d\n", menu_width);
//    refresh();
//
//    // Create chat window
//    chat_win = newwin(CHAT_HEIGHT, chat_width, 0, 0);
//    box(chat_win, 0, 0);
//    wrefresh(chat_win);
//
//    // Create input window
//    input_win = newwin(INPUT_HEIGHT, input_width, CHAT_HEIGHT, 0);
//    box(input_win, 0, 0);
//    wrefresh(input_win);
//
//    // Create input prompt
//    echo();
//    mvprintw(CHAT_HEIGHT + 1, 1, "Enter message: ");
//    wrefresh(input_win);
//
//    // Create menu window
//    menu_win = newwin(MENU_HEIGHT, menu_width, 0, chat_width);
//    box(menu_win, 0, 0);
//    wrefresh(menu_win);
//
//    // Create menu options
//    mvwprintw(menu_win, 1, 1, "Menu");
//    mvwprintw(menu_win, 2, 1, "----");
//    mvwprintw(menu_win, 3, 1, "1. Option 1");
//    mvwprintw(menu_win, 4, 1, "2. Option 2");
//    mvwprintw(menu_win, 5, 1, "3. Option 3");
//    mvwprintw(menu_win, 6, 1, "4. Option 4");
//    mvwprintw(menu_win, 7, 1, "5. Option 5");
//    mvwprintw(menu_win, 8, 1, "6. Option 6");
//
//    // Create menu border
//    mvwvline(menu_win, 1, MENU_WIDTH-1, 0, MENU_HEIGHT-2);
//    mvwvline(menu_win, 1, 0, 0, MENU_HEIGHT-2);
//    mvwhline(menu_win, MENU_HEIGHT-1, 0, 0, MENU_WIDTH);
//    mvwhline(menu_win, 0, 0, 0, MENU_WIDTH);
//    wrefresh(menu_win);
//
//
//}
//
//void *chat_thread(void *arg) {
////    int sock_fd = *(int*)arg;
////    char buffer[CHAT_WIDTH - 2];
//
////    while (1) {
////        // Receive message from server
////        int bytes_received = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);
////        if (bytes_received <= 0) {
////            // Handle error or end of connection
////            break;
////        }
////
////        // Null-terminate message
////        buffer[bytes_received] = '\0';
////
////        // Print message to chat window
////        mvwprintw(chat_win, next_chat_line, 1, "%s", buffer);
////        wrefresh(chat_win);
////
////        // Update chat line index
////        next_chat_line++;
////        if (next_chat_line >= CHAT_HEIGHT - 2) {
////            next_chat_line = 1;
////        }
////    }
//
//    // Clean up and exit thread
////    close(sock_fd);
//    return NULL;
//}
//
//void input_message(char *message) {
//    // Send message to server or process it locally
//    // For example:
//    // send(sock_fd, message, strlen(message), 0);
//    // or
//    // process_local_input(message);
//
////     Print message to chat window
//    mvwprintw(chat_win, next_chat_line, 1, "%s", message);
//    wrefresh(chat_win);
//
//    // Update chat line index
//    next_chat_line++;
//    if (next_chat_line >= CHAT_HEIGHT - 2) {
//        next_chat_line = 1;
//    }
//
//    // Clear input window
//    wclear(input_win);
//    box(input_win, 0, 0);
//
//}
//
//
//void *input_thread(void *arg) {
//    char buffer[INPUT_WIDTH - 2];  // Buffer for user input
//    memset(buffer, 0, sizeof(buffer));
//    int index = 0;  // Index of next character to be written
//
//    wmove(input_win, 1, 15);
//    nodelay(input_win, TRUE);
//    while (1) {
//        // Wait for user input
//        int ch = getch();
//        if (ch == ERR) {
//            // No input available
//            continue;
//        }
//        // Backspace
//        if (ch == KEY_BACKSPACE || ch == 127) {
//            if (index > 0) {
//                index--;
//                buffer[index] = '\0';
//                mvwprintw(input_win, 1, 15 + index, " ");
//                wmove(input_win, 1, 15 + index);
//            }
//        }
//        // Enter
//        else if (ch == 10) {
//            // Send message to server
//            input_message(buffer);
//            memset(buffer, 0, sizeof(buffer));
//            index = 0;
//            wmove(input_win, 1, 15);
//        }
//        // Handle mouse input
//        else if (ch == KEY_MOUSE) {
//            MEVENT event;
//            if (getmouse(&event) == OK) {
//                // Check if mouse click was in menu window
//                if (event.y >= 0 && event.y < MENU_HEIGHT &&
//                    event.x >= 0 && event.x < MENU_WIDTH) {
//                    // Check if mouse click was on menu option
//                    if (event.y >= 3 && event.y <= 8) {
//                        // Enter chat room
//                        enter_chat_room(event.y - 2);
//                    }
//                }
//            }
//        }
//        // Other character
//        else {
//            if (index < INPUT_WIDTH - 2) {
//                buffer[index] = ch;
//                index++;
//                mvwprintw(input_win, 1, 15 + index - 1, "%c", ch);
//                wmove(input_win, 1, 15 + index);
//            }
//        }
//        wrefresh(input_win);
//    }
//
//
//    return NULL;
//}
//
//
//void enter_chat_room(int number)
//{
//    // Clear and redraw menu window
//    werase(chat_win);
//    box(chat_win, 0, 0);
//    mvwprintw(chat_win, 1, 2, "Chat Room %d", number);
//
//    // Refresh menu window
//    wrefresh(chat_win);
//
//}
//void show_my_chats() {
//    // Clear and redraw menu window
//    werase(menu_win);
//    box(menu_win, 0, 0);
//    mvwprintw(menu_win, 1, 2, "My Chats:");
//
//    // Display mock chat rooms
//    mvwprintw(menu_win, 3, 2, "1. Chat Room 1");
//    mvwprintw(menu_win, 4, 2, "2. Chat Room 2");
//    mvwprintw(menu_win, 5, 2, "3. Chat Room 3");
//    mvwprintw(menu_win, 6, 2, "4. Chat Room 4");
//    mvwprintw(menu_win, 7, 2, "5. Chat Room 5");
//
//    // Refresh menu window
//    wrefresh(menu_win);
//}
//
//
//void *menu_thread(void *arg) {
//    // Show menu
//    show_my_chats();
//
//    // Handle user input
//    while (1) {
//        MEVENT event;
//        int ch = getch();
//        if (ch == KEY_MOUSE) {
//            if (getmouse(&event) == OK) {
//                // Handle mouse click
//                if (event.bstate & BUTTON1_CLICKED) {
//                    // Handle left click
//                    if (event.y == 3) {
//                        // Handle first chat room
//                        enter_chat_room(1);
//                    } else if (event.y == 4) {
//                        // Handle second chat room
//                        enter_chat_room(2);
//                    } else if (event.y == 5) {
//                        // Handle third chat room
//                        enter_chat_room(3);
//                    } else if (event.y == 6) {
//                        // Handle fourth chat room
//                        enter_chat_room(4);
//                    } else if (event.y == 7) {
//                        // Handle fifth chat room
//                        enter_chat_room(5);
//                    }
//                }
//            }
//        }
//    }
//
//    // Clean up and exit thread
//    return NULL;
//}
//
//
//int main() {
//    // Initialize ncurses
//    initscr();
//    cbreak();
//    noecho();
//    keypad(stdscr, TRUE);
//    mousemask(ALL_MOUSE_EVENTS, NULL);
//
//    // Initialize windows
//    init_windows();
//
//    // Create threads
//    pthread_t chat_t, input_t, menu_t;
//    pthread_create(&chat_t, NULL, chat_thread, NULL);
//    pthread_create(&menu_t, NULL, menu_thread, NULL);
//    pthread_create(&input_t, NULL, input_thread, NULL);
//
//    // Wait for threads to finish
//    pthread_join(chat_t, NULL);
//    pthread_join(input_t, NULL);
//    pthread_join(menu_t, NULL);
//
//    // Clean up
//    delwin(chat_win);
//    delwin(input_win);
//    delwin(menu_win);
//    endwin();
//    return 0;
//}



//#include "new_ncurses.h"
//#include <ncurses.h>
//#include <stdlib.h>
//#include <string.h>
//#include <time.h>
//#include <ctype.h>
//
//#define INPUT_HEIGHT 3
//
//
//WINDOW *main_window, *input_window, *chat_window, *chats_window, *user_profile_window, *login_window, *register_window,
//        *confirm_exit_window, *menu_window, *chat_window_box, *input_window_box, *menu_window_box, *user_list_window_box,
//        *user_list_window, *public_chat_window, *public_chat_window_box, *create_chat_window_box, *create_chat_window;
//WINDOW *user_settings_window_box, *user_settings_window;
//
//
//
//void init_colors()
//{
//    init_pair(1, COLOR_WHITE, COLOR_BLACK);
//    init_pair(2, COLOR_BLACK, COLOR_WHITE);
//    init_pair(3, COLOR_RED, COLOR_BLACK);
//    init_pair(4, COLOR_GREEN, COLOR_BLACK);
//    init_pair(5, COLOR_YELLOW, COLOR_BLACK);
//    init_pair(6, COLOR_BLUE, COLOR_BLACK);
//    init_pair(7, COLOR_MAGENTA, COLOR_BLACK);
//    init_pair(8, COLOR_CYAN, COLOR_BLACK);
//    init_pair(9, COLOR_WHITE, COLOR_RED);
//    init_pair(10, COLOR_WHITE, COLOR_GREEN);
//    init_pair(11, COLOR_WHITE, COLOR_YELLOW);
//    init_pair(12, COLOR_WHITE, COLOR_BLUE);
//    init_pair(13, COLOR_WHITE, COLOR_MAGENTA);
//    init_pair(14, COLOR_WHITE, COLOR_CYAN);
//    init_pair(15, COLOR_BLACK, COLOR_WHITE);
//    init_pair(16, COLOR_BLACK, COLOR_RED);
//    init_pair(17, COLOR_BLACK, COLOR_GREEN);
//    init_pair(18, COLOR_BLACK, COLOR_YELLOW);
//    init_pair(19, COLOR_BLACK, COLOR_BLUE);
//    init_pair(20, COLOR_BLACK, COLOR_MAGENTA);
//    init_pair(21, COLOR_BLACK, COLOR_CYAN);
//}
//
//void init_ncurses()
//{
//    main_window = initscr();
//    cbreak();
//    noecho();
//    keypad(stdscr, TRUE);
//    curs_set(0);
//    start_color();
//    init_colors();
//    refresh();
//}
//
//void draw_login_window()
//{
//    int x, y, login_x, login_y, password_x, password_y, login_len, password_len, display_name_x, display_name_y, display_name_len;
//    char login[20], password[20], display_name[20];
//    MEVENT event;
//    mousemask(ALL_MOUSE_EVENTS, NULL);
//
//    login_window = subwin(main_window, LINES, COLS, 0, 0);
//    wbkgd(login_window, COLOR_PAIR(1));
//    wrefresh(login_window);
//    keypad(login_window, TRUE);
//    getmaxyx(login_window, y, x);
//
//
//
//    login_len = strlen("Login: ");
//    login_x = x / 2 - login_len;
//    login_y = y / 2;
//    password_len = strlen("Password: ");
//    password_x = x / 2 - password_len;
//    password_y = login_y + 1;
//
//    mvprintw(login_y - 2, x / 2 - 10, "Please enter your credentials:");
//    mvprintw(login_y, login_x, "Login: ");
//    echo();
//    mvprintw(password_y, password_x, "Password: ");
//    echo();
//
//    mvprintw(password_y + 2, x / 2 - 10, "  Login  ");
//    mvprintw(password_y + 2, x / 2, "  Register  ");
//
//    attron(A_REVERSE);
//    mvprintw(password_y + 2, x / 2 - 10, "  Login  ");
//    attroff(A_REVERSE);
//
//    move(login_y, login_x + login_len);
//
//    while (true) {
//        refresh();
//        int ch = getch();
//
//        if (ch == KEY_MOUSE) {
//            if (getmouse(&event) == OK) {
//                if (event.bstate & BUTTON1_CLICKED) { // left mouse button clicked
//                    if (event.x >= x / 2 && event.y == password_y + 2) {
//                        clear();
//                        draw_register_window();
//                    } else if (event.x < x / 2 && event.y == password_y + 2) {
//                        werase(login_window);
//                        wrefresh(login_window);
//                        clear();
//                        draw_main_window();
//                    }
//                }
//
//            }
//        } else if (ch == KEY_RIGHT) {
//            clear();
//            draw_register_window();
//        } else if (ch == '\n') {
//            break;
//        } else {
////            mvprintw(login_y, login_x + login_len, "%c", ch);
////            login[login_len] = ch;
////            login_len++;
//        }
//
//    }
//
//    move(password_y, password_x + password_len);
//    refresh();
//
//    werase(login_window);
//    wrefresh(login_window);
//    clear();
//}
//
//void draw_register_window()
//{
//    int x, y, login_x, login_y, password_x, password_y, login_len, password_len, display_name_x, display_name_y, display_name_len;
//    char login[20], password[20], display_name[20];
//    MEVENT event;
//
//    register_window = newwin(LINES, COLS, 0, 0);
//    keypad(register_window, TRUE);
//    mousemask(ALL_MOUSE_EVENTS, NULL);
//    getmaxyx(register_window, y, x);
//
//    login_len = strlen("Login: ");
//    login_x = x / 2 - login_len;
//    login_y = y / 2;
//
//    password_len = strlen("Password: ");
//    password_x = x / 2 - password_len;
//    password_y = login_y + 1;
//
//    display_name_len = strlen("Display name: ");
//    display_name_x = x / 2 - display_name_len;
//    display_name_y = password_y + 1;
//
//    mvprintw(login_y - 2, x / 2 - 10, "Please enter your credentials:");
//    mvprintw(login_y, login_x, "Login: ");
//    echo();
//    mvprintw(password_y, password_x, "Password: ");
//    echo();
//    mvprintw(display_name_y, display_name_x, "Display name: ");
//    echo();
//
//    mvprintw(display_name_y + 2, x / 2 - 10, "  Login  ");
//    mvprintw(display_name_y + 2, x / 2, "  Register  ");
//
//    // highlight register button
//    attron(A_REVERSE);
//    mvprintw(display_name_y + 2, x / 2, "  Register  ");
//    attroff(A_REVERSE);
//
//
//    while (true) {
//        move(login_y, login_x + login_len);
//        refresh();
//        int ch = getch();
//
//        if (ch == KEY_MOUSE) {
//            if (getmouse(&event) == OK) {
//                if (event.bstate & BUTTON1_CLICKED) {
//                    if (event.x >= x / 2 && event.x <= x / 2 + 10 && event.y == display_name_y + 2) {
//                        // Register button clicked send packet to server
//                    } else if (event.x >= x / 2 - 10 && event.x <= x / 2 && event.y == display_name_y + 2) {
//                        clear();
//                        draw_login_window();
//                    }
//                }
//            }
//        } else if (ch == '\n') {
//            break;
//        } else if(ch == KEY_BACKSPACE || ch == 127 || ch == KEY_DC) {
//            if(strlen(login) > 0)
//                login[strlen(login) - 1] = '\0';
//            if(strlen(password) > 0)
//                password[strlen(password) - 1] = '\0';
//            if(strlen(display_name) > 0)
//                display_name[strlen(display_name) - 1] = '\0';
//        } else if(ch == KEY_LEFT)
//        {
//            clear();
//            draw_login_window();
//        }
//        else {
//            getstr(login);
//
//            move(password_y, password_x + password_len);
//            getstr(password);
//
//            move(display_name_y, display_name_x + display_name_len);
//            getstr(display_name);
//
//            printf("Entered login: %s\n", login);
//            printf("Enter pwd %s\n", password);
//            printf("Enter display name %s\n", display_name);
//        }
//    }
//
//    refresh();
//
//}
//
//void draw_main_window()
//{
//    draw_chat_window();
//    draw_input_window();
//    draw_menu_window();
//    refresh();
//}
//
//void draw_chat_window()
//{
//    // Create window for chat box, draw said box
//    chat_window_box = subwin(main_window, (LINES * 0.8), COLS - 25, 0, 0);
//    box(chat_window_box, 0, 0);
//    // Draw a slick title on it
//    wrefresh(chat_window_box);
//    // Create sub window in box to hold text
//    chat_window = subwin(chat_window_box, (LINES * 0.8 - 2), COLS - 2, 1, 1);
//    // Enable text scrolling
//    scrollok(chat_window, TRUE);
//}
//
//void draw_input_window()
//{
//    char input_text[100];
//
//    // create sub-window for input field
//    input_window_box = subwin(main_window, (LINES * 0.2) - 1, COLS, (LINES * 0.8) + 1, 0);
//    box(input_window_box, 0, 0);
//    input_window = subwin(input_window_box, (LINES * 0.2) - 3, COLS - 2, (LINES * 0.8) + 2, 1);
//    wrefresh(input_window_box);
//    keypad(input_window, TRUE);
//    mousemask(ALL_MOUSE_EVENTS, NULL);
//
//    // draw input box
//    box(input_window, 0, 1);
//    wrefresh(input_window);
//
//    // draw input text
//    mvwprintw(input_window, 1, 2, "Enter your message: ");
//    wrefresh(input_window);
//
//    // draw send button on the right side of the input box
//    mvwprintw(input_window, 1, COLS - 10, "Send");
//    wrefresh(input_window);
//}
//
//void draw_menu_window()
//{
//    menu_window_box = subwin(main_window, (LINES * 0.8), 25, 0, COLS - 25);
//    box(menu_window_box, 0, 0);
//    wrefresh(menu_window_box);
//    menu_window = subwin(menu_window_box, (LINES * 0.8) - 2, 23, 1, COLS - 24);
//    keypad(menu_window, TRUE);
//    mousemask(ALL_MOUSE_EVENTS, NULL);
//
//    // draw menu items
//    attron(A_BOLD);
//    attron(A_UNDERLINE);
//    mvwprintw(menu_window, 1, 2, "My chats");
//    mvwprintw(menu_window, 3, 2, "Online users");
//    mvwprintw(menu_window, 5, 2, "Public chats");
//    mvwprintw(menu_window, 7, 2, "Create chat");
//    mvwprintw(menu_window, 9, 2, "Settings");
//    wrefresh(menu_window);
//    attroff(A_BOLD);
//    attroff(A_UNDERLINE);
//    // handle input for menu items
//    int ch;
//    // while its in the menu window
//    while (true) {
//        MEVENT event;
//        ch = wgetch(menu_window);
//        if (ch == KEY_MOUSE) {
//            if (getmouse(&event) == OK) {
//                if (event.bstate & BUTTON1_CLICKED) {
//                    if (event.x >= COLS - 24 && event.x <= COLS - 2 && event.y == 2) {
//                        // My chats button clicked
//                        clear();
//                        draw_public_chat_window();
//                    } else if (event.x >= COLS - 24 && event.x <= COLS - 2 && event.y == 4) {
//                        // Online users button clicked
//                        clear();
//                        draw_user_list_window();
//                    } else if (event.x >= COLS - 24 && event.x <= COLS - 2 && event.y == 6) {
//                        // Public chats button clicked
//                        clear();
////                        draw_public_chat_list_window();
//                    } else if (event.x >= COLS - 24 && event.x <= COLS - 2 && event.y == 8) {
//                        // Create chat button clicked
//                        clear();
//                        draw_create_chat_window();
//                    } else if (event.x >= COLS - 24 && event.x <= COLS - 2 && event.y == 10) {
//                        // Settings button clicked
//                        clear();
//                        draw_user_settings_window();
//                    }
//                }
//            }
//        }
//    }
//
//}
//
//void draw_public_chat_window()
//{
//    // draw chat list window
//    public_chat_window_box = subwin(main_window, (LINES * 0.8), 25, 0, COLS - 25);
//    box(public_chat_window_box, 0, 0);
//    wrefresh(public_chat_window_box);
//    public_chat_window = subwin(public_chat_window_box, (LINES * 0.8) - 2, 23, 1, COLS - 24);
//
//    keypad(public_chat_window, TRUE);
//    mousemask(ALL_MOUSE_EVENTS, NULL);
//
//    // draw chat list items
//    // TODO: get chats from server and draw them here
//    for(int i = 0; i < 10; i++) {
//        mvwprintw(public_chat_window, i * 2, 2, "Chat %d", i);
//    }
//    wrefresh(public_chat_window);
//
//    // draw back button at the bottom of the window
//    mvwprintw(public_chat_window, (LINES * 0.8) - 4, 2, "Back");
//    wrefresh(public_chat_window);
//
//    int ch;
//    // while its in the chat list window
//    while (true) {
//        MEVENT event;
//        ch = wgetch(public_chat_window);
//        if (ch == KEY_MOUSE) {
//            if (getmouse(&event) == OK) {
//                if (event.bstate & BUTTON1_CLICKED) {
//                    // if back button is pressed
//                    if (event.x >= COLS - 24 && event.x <= COLS - 2 && event.y == (LINES * 0.8) - 3) {
//                        // Back button clicked
//                        clear();
//                        draw_menu_window();
//                    }
//                }
//            }
//        }
//    }
//}
//
//
//void draw_user_list_window()
//{
//    // draw user list window
//    user_list_window_box = subwin(main_window, (LINES * 0.8), 25, 0, COLS - 25);
//    box(user_list_window_box, 0, 0);
//    wrefresh(user_list_window_box);
//    user_list_window = subwin(user_list_window_box, (LINES * 0.8) - 2, 23, 1, COLS - 24);
//
//    keypad(user_list_window, TRUE);
//    mousemask(ALL_MOUSE_EVENTS, NULL);
//
//    // draw user list items
//    // TODO: get users from server and draw them here
//    for(int i = 0; i < 10; i++) {
//        mvwprintw(user_list_window, i * 2, 2, "User %d", i);
//    }
//    wrefresh(user_list_window);
//
//    // draw back button at the bottom of the window
//    mvwprintw(user_list_window, (LINES * 0.8) - 4, 2, "Back");
//    wrefresh(user_list_window);
//
//    // handle input for user list items
//    int ch;
//    // while its in the user list window
//    while (true) {
//        MEVENT event;
//        ch = wgetch(user_list_window);
//        if (ch == KEY_MOUSE) {
//            if (getmouse(&event) == OK) {
//                if (event.bstate & BUTTON1_CLICKED) {
//                    if (event.x >= COLS - 24 && event.x <= COLS - 2 && event.y == (LINES * 0.8) - 3) {
//                        // Back button clicked
//                        clear();
//                        draw_menu_window();
//                    }
//                }
//            }
//        }
//    }
//}
//
//void draw_create_chat_window()
//{
//    // Create window for chat box, draw said box
//    create_chat_window_box = subwin(main_window, (LINES * 0.8), 25, 0, COLS - 25);
//    box(create_chat_window_box, 0, 0);
//    wrefresh(create_chat_window_box);
//    create_chat_window = subwin(create_chat_window_box, (LINES * 0.8) - 2, 23, 1, COLS - 24);
//    keypad(create_chat_window, TRUE);
//
//    // draw input fields
//    mvwprintw(create_chat_window, 1, 2, "Name of channel:");
//
//    mvwprintw(create_chat_window, 4, 2, "Private (y/n):");
//
//
//    // draw save and back buttons
//    mvwprintw(create_chat_window, 7, 4, "[Save]");
//    mvwprintw(create_chat_window, (LINES * 0.8) - 4, 2, "Back");
//    wrefresh(create_chat_window);
//
//
//
//    bool done = false;
//    while (!done) {
//        MEVENT event;
//        int ch = wgetch(create_chat_window);
//        if (ch == KEY_MOUSE) {
//            if (getmouse(&event) == OK) {
//                if (event.bstate & BUTTON1_CLICKED) {
//                    if (event.y == 7 && event.x >= 4 && event.x <= 9) {
//                        // Save button clicked
//                        // TODO: send channel name and private status to server
//                        // TODO: clear and open chat window for the new channel
//                        clear();
//                        draw_menu_window();
//                        done = true;
//                    } else if (event.x >= COLS - 24 && event.x <= COLS - 2 && event.y == (LINES * 0.8) - 3) {
//                        // Back button clicked
//                        clear();
//                        draw_menu_window();
//                        done = true;
//                    }
//                }
//            }
//        } else if (ch == '\n') {
//            wmove(create_chat_window, 2, 2);
//            wrefresh(create_chat_window);
//            char channel_name[20];
//            wgetnstr(create_chat_window, channel_name, 20);
//
//            wmove(create_chat_window, 5, 2);
//            wrefresh(create_chat_window);
//            char is_private[2];
//            wgetnstr(create_chat_window, is_private, 2);
//        }
//    }
//}
//
//void draw_user_settings_window()
//{
//    // Create window for chat box, draw said box
//    user_settings_window_box = subwin(main_window, (LINES * 0.8), 25, 0, COLS - 25);
//    box(user_settings_window_box, 0, 0);
//    wrefresh(user_settings_window_box);
//    user_settings_window = subwin(user_settings_window_box, (LINES * 0.8) - 2, 23, 1, COLS - 24);
//    keypad(user_settings_window, TRUE);
//
//    // draw input fields
//    mvwprintw(user_settings_window, 1, 2, "Name:");
//    mvwprintw(user_settings_window, 8, 2, "Password:");
//
//    // draw save and back buttons
//    mvwprintw(user_settings_window, 4, 4, "[Save]");
//    mvwprintw(user_settings_window, 11, 4, "[Save]");
//    mvwprintw(user_settings_window, (LINES * 0.8) - 4, 2, "Back");
//    wrefresh(user_settings_window);
//
//    bool done = false;
//    while (!done)
//    {
//        MEVENT event;
//        int ch = wgetch(user_settings_window);
//        if (ch == KEY_MOUSE)
//        {
//            if (getmouse(&event) == OK)
//            {
//                if (event.bstate & BUTTON1_CLICKED)
//                {
//                    if (event.y == 7 && event.x >= 4 && event.x <= 9)
//                    {
//                        // Save button clicked
//                        clear();
//                        draw_menu_window();
//                        done = true;
//                    } else if (event.x >= COLS - 24 && event.x <= COLS - 2 && event.y == (LINES * 0.8) - 3)
//                    {
//                        // Back button clicked
//                        clear();
//                        draw_menu_window();
//                        done = true;
//                    }
//                }
//            }
//        } else if (ch == '\n')
//        {
//            wmove(user_settings_window, 2, 2);
//            wrefresh(user_settings_window);
//            char name[20];
//            wgetnstr(user_settings_window, name, 20);
//
//            wmove(user_settings_window, 9, 2);
//            wrefresh(user_settings_window);
//            char password[20];
//            wgetnstr(user_settings_window, password, 20);
//        }
//    }
//}
//
//int handle_input()
//{
//    char input_text[100];
//    // handle mouse events
//    echo();
//    MEVENT event;
//    while (true) {
//        // move cursor to the input box
//        wmove(input_window, 1, 22); // move to the beginning of the input box
//        wgetstr(input_window, input_text);
//        wrefresh(input_window);
//        int ch = wgetch(input_window);
//        if (ch == KEY_MOUSE) {
//            if (getmouse(&event) == OK) {
//                if (event.bstate & BUTTON1_CLICKED) {
//                    if (event.x >= COLS - 10 && event.x <= COLS && event.y == 1) {
//                        // Send button clicked send packet to server
//                    }
//                }
//            }
//        } else if (ch == '\n')
//        {
//            // Send packet to server
////            send_packet(input_text);
//            // Clear input box
//            memset(input_text, 0, sizeof(input_text));
//            mvwprintw(input_window, 1, 22, "%s", input_text);
//            wrefresh(input_window);
//            break;
//        } else if(ch == KEY_BACKSPACE || ch == 127 || ch == KEY_DC) {
//            if(strlen(input_text) > 0)
//                input_text[strlen(input_text) - 1] = '\0';
//        } else {
//            if (strlen(input_text) < 98) { // prevent buffer overflow
//                input_text[strlen(input_text)] = ch;
//                input_text[strlen(input_text) + 1] = '\0';
//                mvwprintw(input_window, 1, 22, "%s", input_text);
//                wrefresh(input_window);
//            }
//        }
//    }
//    return 0;
//}
//
//int main()
//{
//    init_ncurses();
//    draw_login_window();
//    draw_main_window();
//    while(1)
//    {
//        wrefresh(main_window);
//        wcursyncup(main_window);
//        wrefresh(input_window);
//        wcursyncup(input_window);
//        handle_input();
//    }
//}