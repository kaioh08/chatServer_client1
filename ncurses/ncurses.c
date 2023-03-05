#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define INPUT_HEIGHT 3
#define MAX_MESSAGES 100
#define MAX_MESSAGE_LENGTH 100
#define INPUT_WIDTH 60
#define INPUT_Y (LINES - INPUT_HEIGHT)
#define INPUT_X 0
char messages[MAX_MESSAGES][MAX_MESSAGE_LENGTH + 1];
int num_messages = 0;

static void register_window();
static void login_window();
void chat_window();
void draw_chat_window();
void draw_buttons();
void draw_messages();
void draw_input_box();
void clear_input_box(char *input, int input_len, int buf_size);
void handle_input();
void add_message(char *input, char *username);


void init_pairs()
{
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_BLACK, COLOR_WHITE);
}

static void login_window() {
    int x, y, login_x, login_y, password_x, password_y, login_len, password_len;
    char login[20], password[20];
    MEVENT event;

    clear();
    mousemask(ALL_MOUSE_EVENTS, NULL);

    getmaxyx(stdscr, y, x);

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

    // highlight login button
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
                        register_window();
                    } else if (event.x < x / 2 && event.y == password_y + 2) {
                        chat_window();
                    }
                }

            }
        } else if (ch == KEY_RIGHT) {
            register_window();
        } else if (ch == '\n') {
            break;
        } else {
            getstr(login);

            move(password_y, password_x + password_len);
            getstr(password);

            printf("Entered login: %s\n", login);
            printf("Entered password: %s\n", password);
        }

    }


    refresh();


}

static void register_window()
{
    int x, y, login_x, login_y, password_x, password_y, login_len, password_len, display_name_x, display_name_y, display_name_len;
    char login[20], password[20], display_name[20];
    MEVENT event;

    clear();
    getmaxyx(stdscr, y, x);

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
                        // Login button
                        login_window();
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
            login_window();
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

/**
 * Gets the text from the input box and returns it as a string.
 */
char* get_input_text() {
    int buf_size = INPUT_WIDTH - 2;
    char* input_text = malloc(buf_size + 1);
    memset(input_text, 0, buf_size + 1);
    int i = 0;
    int ch;

    // disable cursor
    curs_set(0);
    getstr(input_text);
    curs_set(1);

    refresh();

    return input_text;
}

void handle_input()
{
    char *input = get_input_text();
    char test_username[] = "test";
    if (strlen(input) > 0)
    {
        add_message(input, test_username);
        draw_messages();
    }
    clear_input_box(input, strlen(input), INPUT_WIDTH - 2);
    refresh();
}

void add_message(char *input, char *username)
{
    if (num_messages < MAX_MESSAGES) {
        // get current time
        time_t current_time = time(NULL);
        struct tm *local_time = localtime(&current_time);

        // format timestamp
        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "%d/%m %H:%M:%S", local_time);

        // add message with timestamp to the messages array
        snprintf(messages[num_messages], MAX_MESSAGE_LENGTH + 1, "[%s] %s: %s\n", timestamp, username , input);
        num_messages++;
    }
}

void draw_messages() {
    int i, j;
    int height, width, start_y, start_x;
    int num_displayed_messages = LINES - INPUT_HEIGHT - 2;
    int scroll_offset = 0;

    // get the dimensions of the chat window
    getmaxyx(stdscr, height, width);

    // calculate the start position of the messages
    start_y = 1;
    start_x = 1;

    // calculate the scroll offset based on the number of messages
    if (num_messages > num_displayed_messages) {
        scroll_offset = num_messages - num_displayed_messages;
    }

    // clear the messages window
    clear();

    // draw the messages in reverse order
    for (i = num_messages - 1, j = 0 ; i >= 0 &&  j < num_displayed_messages; i--, j++) {
        mvprintw(start_y + num_displayed_messages - j - 1, start_x, messages[i]);
    }

    // draw the scroll bar if necessary
    if (num_messages > num_displayed_messages) {
        int scroll_height = (num_displayed_messages * num_displayed_messages) / num_messages;
        int scroll_pos = (scroll_offset * (num_displayed_messages - scroll_height)) / (num_messages - num_displayed_messages);
        attron(COLOR_PAIR(2));
        for (i = 0; i < scroll_height; i++) {
            mvprintw(start_y + num_displayed_messages - i - 1, width - 3, " ");
        }
        mvprintw(start_y + num_displayed_messages - scroll_pos - scroll_height, width - 3, "|");
        attroff(COLOR_PAIR(2));
    }

}

void clear_input_box(char *input, int input_len, int buf_size)
{
    memset(input, 0, input_len);
    input[0] = '>'; // add the prompt character back
}

void draw_chat_window() {
    int height, width, start_y, start_x;

    // get the dimensions of the chat window
    getmaxyx(stdscr, height, width);

    // calculate the start position of the chat window
    start_y = 0;
    start_x = 0;

    box(stdscr, start_y, start_x);
}

void draw_buttons() {
    int height, width, start_y, start_x;

    // get the dimensions of the chat window
    getmaxyx(stdscr, height, width);

    // calculate the start position of the buttons
    start_y = height - INPUT_HEIGHT + 1;
    start_x = width - 14;

    // set the color pair, attributes, and text for the Send button
    attron(COLOR_PAIR(2));
    attron(A_BOLD | A_UNDERLINE | A_REVERSE);
    mvprintw(start_y, start_x, "Send");
    attroff(A_BOLD | A_UNDERLINE | A_REVERSE);
    attroff(COLOR_PAIR(2));

    // set the color pair, attributes, and text for the Exit button
    attron(COLOR_PAIR(2));
    attron(A_BOLD | A_UNDERLINE | A_REVERSE);
    mvprintw(start_y, start_x - 10, "Exit");
    attroff(A_BOLD | A_UNDERLINE | A_REVERSE);
    attroff(COLOR_PAIR(2));

    // set the color pair, attributes, and text for the Exit button
    attron(COLOR_PAIR(3));
    attron(A_BOLD | A_UNDERLINE | A_REVERSE);
    mvprintw(start_y, start_x - 20, "Chats");
    attroff(A_BOLD | A_UNDERLINE | A_REVERSE);
    attroff(COLOR_PAIR(3));

    // set the color pair, attributes, and text for the Exit button
    attron(COLOR_PAIR(3));
    attron(A_BOLD | A_UNDERLINE | A_REVERSE);
    mvprintw(start_y, start_x - 30, "Friends");
    attroff(A_BOLD | A_UNDERLINE | A_REVERSE);
    attroff(COLOR_PAIR(3));

    // if user clicks on the Exit button, return to login screen
    if (getch() == KEY_MOUSE) {
        MEVENT event;
        if (getmouse(&event) == OK) {
            if (event.bstate & BUTTON1_CLICKED) {
                if (event.y == start_y && event.x >= start_x - 10 && event.x <= start_x - 2) {
                    clear();
                    refresh();
                    login_window();
                }
            }
        }
    }
}

void draw_input_box() {
    int height, width, start_y, start_x;

    // get the dimensions of the input window
    getmaxyx(stdscr, height, width);

    // calculate the start position of the input box
    start_y = height - INPUT_HEIGHT + 1;
    start_x = 2;

    // set the color pair and attributes for the input box
    attron(COLOR_PAIR(2));
    attron(A_BOLD);

    // draw the input box
    mvprintw(start_y, start_x, "> ");
    clrtoeol();
    attroff(A_BOLD);
    attroff(COLOR_PAIR(2));

    move(start_y, start_x + 2);

    refresh();

    handle_input();
}

void draw_ui() {
    draw_chat_window();
    draw_messages();
    draw_buttons();
    draw_input_box();

}

int scroll_chat(int scroll_offset)
{
    int num_displayed_messages = LINES - INPUT_HEIGHT - 2;
    int max_scroll_offset = num_messages - num_displayed_messages;
    int new_scroll_offset = scroll_offset + num_messages;

    // make sure the new scroll offset is within bounds
    if (new_scroll_offset < 0)
    {
        new_scroll_offset = 0;
    } else if (new_scroll_offset > max_scroll_offset)
    {
        new_scroll_offset = max_scroll_offset;
    }

    // only redraw if the scroll offset has changed
    if (new_scroll_offset != scroll_offset)
    {
        scroll_offset = new_scroll_offset;
        draw_messages();
    }

    return scroll_offset;
}

void chat_window()
{
    clear();
    refresh();
    mousemask(ALL_MOUSE_EVENTS, NULL);

    while(true)
    {
        draw_ui();
        refresh();
    }

}


int main()
{
    initscr(); // initialize ncurses
    cbreak(); // disable line buffering
    noecho(); // disable echoing of input
    start_color(); // enable color support
    init_pairs(); // initialize the color pairs
    mousemask(ALL_MOUSE_EVENTS, NULL); // enable mouse events
    keypad(stdscr, TRUE); // enable function keys
    login_window();
    endwin();
    return 0;
}
