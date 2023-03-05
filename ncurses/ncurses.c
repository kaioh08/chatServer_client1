#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

WINDOW *input_window;
WINDOW *output_window;
WINDOW *send_button;
WINDOW *exit_button;
char input_buffer[80];

static void register_window();
static void login_window();

void init_pairs()
{
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_BLACK, COLOR_WHITE);
}

void print_time(WINDOW *win)
{
    time_t now;
    struct tm *tm_info;
    time(&now);
    tm_info = localtime(&now);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%m/%d %H:%M:%S", tm_info);
    wattron(win, COLOR_PAIR(2));
    wprintw(win, "%s", time_str);
    wattroff(win, COLOR_PAIR(2));
}

void handle_input()
{
    // Print the entered string in the output window
    wattron(output_window, COLOR_PAIR(1));
    print_time(output_window);
    wprintw(output_window, "  - You: ");
    wattron(output_window, COLOR_PAIR(3));
    wprintw(output_window, "%s\n", input_buffer);
    wrefresh(output_window);

    // Clear the input buffer and the input window
    memset(input_buffer, 0, sizeof(input_buffer));
    wclear(input_window);
    box(input_window, '|', '-');
    wattron(input_window, COLOR_PAIR(3));
    mvwprintw(input_window, 1, 1, "Enter a string: ");
    wrefresh(input_window);

    wmove(input_window, 1, 17); // move the cursor to the input window and the beginning of the input buffer
    wrefresh(input_window); // refresh the input window to display the cursor

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
                    }
                }
            }
        } else if (ch == KEY_RIGHT) {
            register_window();
        } else if (ch == '\n') {
            break;
        }
    }
    getstr(login);

    move(password_y, password_x + password_len);
    getstr(password);

    refresh();

    printf("Entered login: %s\n", login);
    printf("Entered password: %s\n", password);
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

    move(login_y, login_x + login_len);

    while (true) {
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
        }
    }

    getstr(login);

    move(password_y, password_x + password_len);
    getstr(password);

    move(display_name_y, display_name_x + display_name_len);
    getstr(display_name);

    refresh();

}




void handle_mouse_event(MEVENT *event)
{
    if (event->bstate & BUTTON1_CLICKED)
    {
        if (event->x >= COLS - 20 && event->x <= COLS - 13 &&
            event->y == LINES - 4)
        {
            // "Send" button clicked
            handle_input();
        }
        else if (event->x >= COLS - 10 && event->x <= COLS - 3 &&
                 event->y == LINES - 4)
        {
            // "Exit" button clicked
            // clear the screen and exit
            login_window();
        }
    }
}

int main()
{
    int ch;
    MEVENT event;


    initscr(); // initialize ncurses
    cbreak(); // disable line buffering
    noecho(); // disable echoing of input
    start_color(); // enable color support
    init_pairs(); // initialize the color pairs
    mousemask(ALL_MOUSE_EVENTS, NULL); // enable mouse events
    keypad(stdscr, TRUE); // enable function keys

    // Print the "Send" button
    send_button = newwin(3, 8, LINES - 4, COLS - 10);
    wbkgd(send_button, COLOR_PAIR(1));
    wattron(send_button, COLOR_PAIR(3));
    wprintw(send_button, "Send");
    wrefresh(send_button); // Refresh the button window to display it

    // Print the "Exit" button
    exit_button = newwin(3, 8, LINES - 4, COLS - 20);
    wbkgd(exit_button, COLOR_PAIR(1));
    wattron(exit_button, COLOR_PAIR(3));
    wprintw(exit_button, "Exit");
    wrefresh(exit_button); // Refresh the button window to display it


    // Create a new window for the input line at the bottom of the screen
    input_window = newwin(3, COLS - 2, LINES - 4, 1);
    box(input_window, '|', '-');
    wattron(input_window, COLOR_PAIR(3));
    mvwprintw(input_window, 1, 1, "Enter a string: ");
    wrefresh(input_window); // Force display of the input window

    // Create a new window for the output
    output_window = newwin(LINES - 8, COLS - 2, 1, 1);
    box(output_window, '|', '-');
    wrefresh(output_window);

    while (1)
    {
        ch = getch();
        if (ch == KEY_MOUSE)
        {
            if (getmouse(&event) == OK)
            {
                handle_mouse_event(&event);
            }
        }
        else if (ch == KEY_ENTER || ch == '\n')
        {
            handle_input();
        }
        else if (ch == KEY_BACKSPACE || ch == 127)
        {
            if (strlen(input_buffer) > 0)
            {
                input_buffer[strlen(input_buffer) - 1] = '\0';
                wclear(input_window);
                box(input_window, '|', '-');
                wattron(input_window, COLOR_PAIR(3));
                mvwprintw(input_window, 1, 1, "Enter a string: ");
                wprintw(input_window, "%s", input_buffer);
                wrefresh(input_window);
            }
        }
        else if (ch == KEY_DC)
        {
            memset(input_buffer, 0, sizeof(input_buffer));
            wclear(input_window);
            box(input_window, '|', '-');
            wattron(input_window, COLOR_PAIR(3));
            mvwprintw(input_window, 1, 1, "Enter a string: ");
            wrefresh(input_window);
        }
        else if (ch == KEY_RESIZE)
        {
            // Resize the windows
            wresize(send_button, 3, 8);
            wresize(exit_button, 3, 8);
            mvwin(send_button, LINES - 4, COLS - 10);
            mvwin(exit_button, LINES - 4, COLS - 20);
            wclear(send_button);
            wclear(exit_button);
            wbkgd(send_button, COLOR_PAIR(2));
            wbkgd(exit_button, COLOR_PAIR(2));
            wattron(send_button, COLOR_PAIR(1));
            wprintw(send_button, "Exit");
            wattroff(send_button, COLOR_PAIR(1));
            wattron(exit_button, COLOR_PAIR(2));
            wprintw(exit_button, "Send");
            wattroff(exit_button, COLOR_PAIR(2));
            // Redraw the input line
            wclear(input_window);
            box(input_window, '|', '-');
            wattron(input_window, COLOR_PAIR(3));
            mvwprintw(input_window, 1, 1, "Enter a string: ");
            wprintw(input_window, "%s", input_buffer);
            wrefresh(input_window);

            wrefresh(output_window);
            wrefresh(send_button);
            wrefresh(exit_button);
        }
        else
        {
            if (strlen(input_buffer) < 80)
            {
                input_buffer[strlen(input_buffer)] = ch;
                wclear(input_window);
                box(input_window, '|', '-');
                wattron(input_window, COLOR_PAIR(3));
                mvwprintw(input_window, 1, 1, "Enter a string: ");
                wprintw(input_window, "%s", input_buffer);
                wrefresh(input_window);
            }
        }
    }

    endwin();
    return 0;
}