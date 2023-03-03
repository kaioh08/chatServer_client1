#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

WINDOW *input_window;
WINDOW *output_window;
WINDOW *send_button;
WINDOW *exit_button;
char input_buffer[80];

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

    // Move the cursor to the input window
    wmove(input_window, 1, 17); // move the cursor to the input window and the beginning of the input buffer
    wrefresh(input_window); // refresh the input window to display the cursor

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
            endwin();
            exit(0);
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

    // Create a new window for the input line at the bottom of the screen
    input_window = newwin(3, COLS - 2, LINES - 7, 1);
    box(input_window, '|', '-');
    wattron(input_window, COLOR_PAIR(3));
    mvwprintw(input_window, 1, 1, "Enter a string: ");
    wrefresh(input_window); // Force display of the input window

    // Print the "Send" button
    send_button = newwin(3, 8, LINES - 7, 10);
    wbkgd(send_button, COLOR_PAIR(3));
    wattron(send_button, COLOR_PAIR(1));
    wprintw(send_button, "Send");
    wattroff(send_button, COLOR_PAIR(1));
    wrefresh(send_button);

    // Print the "Exit" button
    exit_button = newwin(3, 8, LINES - 4, COLS - 20);
    wbkgd(exit_button, COLOR_PAIR(3));
    wattron(exit_button, COLOR_PAIR(2));
    wprintw(exit_button, "Exit");
    wattroff(exit_button, COLOR_PAIR(2));
    wrefresh(exit_button);

    // Create a new window for the output
    output_window = newwin(LINES - 6, COLS - 2, 1, 1);
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
            wresize(input_window, 3, COLS - 2);
            wresize(output_window, LINES - 6, COLS - 2);
            wresize(send_button, 3, 8);
            wresize(exit_button, 3, 8);
            mvwin(input_window, LINES - 4, 1);
            mvwin(output_window, 1, 1);
            mvwin(send_button, LINES - 4, COLS - 10);
            mvwin(exit_button, LINES - 4, COLS - 20);
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