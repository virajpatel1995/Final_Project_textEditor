#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include "editor.h"

using namespace std;

/**
   * Function that initilizes ncurses
   * @param None
   * @return None
   */
void curses_init()
{
    initscr();                      // Start ncurses mode
    noecho();                       // Don't echo keystrokes
    cbreak();                       // Disable line buffering
    keypad(stdscr, true);           // Enable special keys to be recorded
}

/**
   * Main function
   * @param filename: if pressent, file "filename" is opened on the text editor
   * @return None
   */
int main(int argc, char* argv[])
{
    if(argc > 2)
    {
        printf("\n");
        printf("ERROR: invalid number of arguments.\n");
        printf("Usage: %s [filename]\n", argv[0]);
        printf("\n");
        return 0;
    }

    curses_init();

    Editor *editor;

    if(argc > 1)
    {
        editor = new Editor(argv[1]);
    }
    else
    {
        editor = new Editor();
    }

    // Main loop
    editor->run();
    editor->freeResources();

    delete editor;

    endwin();
    return 0;
}
