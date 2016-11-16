#include "editor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


// Main window margins to not print on the terminal borders
#define MARGIN_X  1
#define MARGIN_Y  0
// Position of the "F1: Menu" label in the terminal window
#define F1_LABEL_X  MARGIN_X
#define F1_LABEL_Y  MARGIN_Y
// Position of the Text Box window in the terminal window
#define TEXT_BOX_X  MARGIN_X
#define TEXT_BOX_Y  1
// Position of the Text window in the terminal window
#define TEXT_X  (TEXT_BOX_X + 1)
#define TEXT_Y  (TEXT_BOX_Y + 1)

// MENU window

// Size
#define MENU_WINDOW_HEIGHT          8
#define MENU_WINDOW_WIDTH           20
#define MENU_ITEMS_WINDOW_HEIGHT    5
#define MENU_ITEMS_WINDOW_WIDTH     (MENU_WINDOW_WIDTH - 2)
// Position of the Menu window title refered to the menu window
#define MENU_TITLE_Y           1
#define MENU_TITLE_X           3
#define MENU_TITLE_HEIGHT   1
#define MENU_TITLE_WIDTH    (MENU_WINDOW_WIDTH - 2)
// Position of the first item refered to the menu window
#define MENU_ITEMS_Y           3
#define MENU_ITEMS_X           1

//#define MENU_WINDOW_Y           2
//#define MENU_WINDOW_X           2


// Size
#define CONFIRMATION_WINDOW_HEIGHT          6
#define CONFIRMATION_WINDOW_WIDTH           20
#define CONFIRMATION_ITEMS_WINDOW_HEIGHT    3
#define CONFIRMATION_ITEMS_WINDOW_WIDTH     (CONFIRMATION_WINDOW_WIDTH - 2)
// Position of the Menu window title refered to the menu window
#define CONFIRMATION_TITLE_Y           1
#define CONFIRMATION_TITLE_X           3
#define CONFIRMATION_TITLE_HEIGHT   1
#define CONFIRMATION_TITLE_WIDTH    (CONFIRMATION_WINDOW_WIDTH - 2)
// Position of the first item refered to the menu window
#define CONFIRMATION_ITEMS_Y           3
#define CONFIRMATION_ITEMS_X           1

// Dialogs
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
// Menu items
#define OPEN_STR    "Open"
#define SAVE_STR    "Save"
#define SAVE_AS_STR "Save As..."
#define EXIT_STR    "Exit"
char const *menuOptions[] = {
                        OPEN_STR,
                        SAVE_STR,
                        SAVE_AS_STR,
                        EXIT_STR
                        };
// Confirmation dialog
#define NO_STR     "NO"
#define YES_STR    "YES"
char const *confirmationOptions[] = {
                        NO_STR,
                        YES_STR
                        };

#define NEW_FILE_NAME   "Untitled"

#define SUCCESS 1
#define ERROR   0

// Constructor for the Buffer class

Buffer::Buffer()
{
}


   //Function to insert line at provided index in the buffer
 
void Buffer::insertLine(string line, int n)
{
    line = substituteTabs(line);                   // Conversion (happens every time)
    lines.insert(lines.begin() + n, line);
}


   //append line to the buffer
 
void Buffer::appendLine(string line)
{
    line = substituteTabs(line);
    lines.push_back(line);
}


   //remove line at provided index

void Buffer::removeLine(int n)
{
    lines.erase(lines.begin() + n);
}


   // Function empty the buffer
void Buffer::clear()
{
    lines.clear();
}

/**
   * Function to substitute any tab for four spaces in the provided line
   */
string Buffer::substituteTabs(string line)
{
    std::size_t tab = line.find("\t");
    if(tab == line.npos)
        return line;
    else
        return substituteTabs(line.replace(tab, 1, "    "));
}


/*
   *  constructor of the Editor class
 
   */
Editor::Editor()
{
    init();
}

/**
   * Auxiliar constructor of the Editor class to allow opening a file
   */
Editor::Editor(string filename)
{
    init();
    isNewFile = 0;

    if(loadFile(filename) == ERROR)
    {
        fileName.clear();
        fileName.append(NEW_FILE_NAME);

        char message[100];
        sprintf(message,"ERROR: Could not open file \"%s\"", filename.c_str());
        runMessageDialog(message);
    }
    else
    {
        fileName.clear();
        fileName.append(filename);
        lineNum = 0;
        charNum = 0;
        wmove(textWindow,lineNum, charNum);
        refresh();
    }
    updateFilenameInGui();
}

void Editor::freeResources()
{
    delwin(borderWindow);
    delwin(textWindow);
    delwin(messageWindow);
    delwin(fileNameWindow);

    std::vector<string>::iterator iter = textBuffer->lines.begin();
    while (iter != textBuffer->lines.end())
    {
        iter = textBuffer->lines.erase(iter);
    }

    delete textBuffer;

    fileName.erase();
}

//Function to initialize internal variables and create the GUI
 
void Editor::init()
{
    lineNum = 0;
    charNum = 0;
    fileName = NEW_FILE_NAME;

    textBuffer = new Buffer();
    textBuffer->appendLine("");

    isNewFile = 1;
    unsavedChanges = 0;
    exitGui = 0;

    guiFirstLine = 0;

    createGui();
}

void Editor::createGui(void)
{
    mvprintw(F1_LABEL_Y, F1_LABEL_X, "F1: Menu");
    refresh();

    int height = LINES - 2; // Total lines - "F1:Menu" - fileName line
    int width = COLS - 2 * MARGIN_X;
    int starty = TEXT_BOX_Y;
    int startx = TEXT_BOX_X;

    borderWindow = newwin(height, width, starty, startx);
    textWindow = newwin(height-2, width-2, starty+1, startx+1);
    box(borderWindow, 0 , 0);
    wrefresh(borderWindow);
    wrefresh(textWindow);
    mvprintw(LINES-1, MARGIN_X, "%s", fileName.c_str());
    refresh();
}

   //Main loop of the Editor class

void Editor::run()
{
    int c = 0;
    while(!exitGui)
    {
        printText();
        c = getch();
        handleInput(c);
    }
}


   //Function to handle the key inputs from the user.

void Editor::handleInput(int c)
{
    switch(c)
    {
        case KEY_F(1):
            runMenuDialog();
            break;
        case KEY_LEFT:
            cursorLeft();
            break;
        case KEY_RIGHT:
            cursorRight();
            break;
        case KEY_UP:
            cursorUp();
            break;
        case KEY_DOWN:
            cursorDown();
            break;

        case 127:
        case KEY_BACKSPACE:
            // Check if cursor is on the first char on a line that is not the first one
            if(charNum == 0 && lineNum > 0)
            {
                // Append current line to line-1
                charNum = textBuffer->lines[lineNum-1].length();
                textBuffer->lines[lineNum-1] += textBuffer->lines[lineNum];
                // Delete current line
                deleteLine();
                // Move cursor
                cursorUp();
            }
            else if (charNum > 0)
            {
                textBuffer->lines[lineNum].erase(--charNum, 1);
            }
            unsavedChanges = 1;
            break;
        case KEY_DC:
            // Check if cursor is at the last char on a line that is not the last one
            if(charNum == textBuffer->lines[lineNum].length() && lineNum != textBuffer->lines.size() - 1)
            {
                // Append line+1 to line
                textBuffer->lines[lineNum] += textBuffer->lines[lineNum+1];
                // Delete the line
                deleteLine(lineNum+1);
            }
            else
            {
                textBuffer->lines[lineNum].erase(charNum, 1);
            }
            unsavedChanges = 1;
            break;
        case KEY_ENTER:
        case 10:
            // The Enter key
            // Check if cursor is not at the line end
            if(charNum < textBuffer->lines[lineNum].length())
            {
                // Put the rest of the line on a new line
                textBuffer->insertLine(textBuffer->lines[lineNum].substr(charNum, textBuffer->lines[lineNum].length() - charNum), lineNum + 1);
                // Remove that part of the line
                textBuffer->lines[lineNum].erase(charNum, textBuffer->lines[lineNum].length() - charNum);
            }
            else
            {
                textBuffer->insertLine("", lineNum+1);
            }
            charNum = 0;
            cursorDown();
            unsavedChanges = 1;
            break;
        case KEY_BTAB:
        case KEY_CTAB:
        case KEY_STAB:
        case KEY_CATAB:
        case 9:
            // The Tab key
            textBuffer->lines[lineNum].insert(charNum, 4, ' ');
            charNum += 4;
            unsavedChanges = 1;
            break;
        default:
            // Any other character
            textBuffer->lines[lineNum].insert(charNum, 1, char(c));
            charNum++;
            unsavedChanges = 1;
            break;
    }
}


   //  loads a file to the text editor

int Editor::loadFile(string filename)
{
    int status = ERROR;

    FILE *fd;
    fd = fopen(filename.c_str(), "r");
    if(fd)
    {
        textBuffer->clear();
        char c;
        c = fgetc(fd);
        string temp;
        while(c != EOF)
        {
            if(c != '\n')
            {
                temp += c;
            }
            else
            {
                //temp += c;
                textBuffer->appendLine(temp);
                temp.clear();
            }
            c = fgetc(fd);
        }
        textBuffer->appendLine(temp);
        fclose(fd);
        status = SUCCESS;
    }
    else
    {
        status = ERROR;
    }
    return status;
}


   //Function to save the current edited file
 
int Editor::saveFile()
{
    int status = ERROR;
    FILE *fd;
    char c;
    fd = fopen(fileName.c_str(), "w");
    if(fd)
    {
        string auxStr;
        for(unsigned int i=0; i<textBuffer->lines.size(); i++)
        {
            auxStr = textBuffer->lines.at(i);
            for(unsigned int j=0; j< auxStr.length(); j++)
            {
                c = auxStr.at(j);
                fputc(c, fd);
            }
            fputc('\n', fd);
            auxStr.clear();
        }
        fclose(fd);
        unsavedChanges = 0;
        status = SUCCESS;
    }
    return status;
}

bool Editor::fileExists(string filename)
{
    FILE *fd;
    fd = fopen(filename.c_str(), "r");
    if(fd)
    {
        fclose(fd);
        return true;
    }
    return false;
}


   //Function that generates the menu and manages the user inputs (move, enter,..)
 
void Editor::runMenuDialog()
{
    WINDOW *menuWindow;
    MENU *menu;
    ITEM *selectedItem;

    ITEM **menuItemList;
    int numberOfOptions, i;

    numberOfOptions = ARRAY_SIZE(menuOptions);
    menuItemList = (ITEM **)calloc(numberOfOptions + 1, sizeof(ITEM *));

    for(i = 0; i < numberOfOptions; ++i) menuItemList[i] = new_item(menuOptions[i], "");
    menuItemList[numberOfOptions] = (ITEM *)NULL;

    menu = new_menu((ITEM **)menuItemList);

    int x, y;
    getWindowOritinToKeepCentered(MENU_WINDOW_HEIGHT, MENU_WINDOW_WIDTH, &y, &x);

    // Create the window for the menu
    menuWindow = newwin(MENU_WINDOW_HEIGHT, MENU_WINDOW_WIDTH, y, x);
    keypad(menuWindow, TRUE);

    set_menu_win(menu, menuWindow);
    set_menu_sub(menu, derwin(menuWindow, MENU_ITEMS_WINDOW_HEIGHT, MENU_ITEMS_WINDOW_WIDTH, MENU_ITEMS_Y, MENU_ITEMS_X));

    // Menu marker
    set_menu_mark(menu, "* ");

    box(menuWindow, 0, 0);
    mvwprintw(menuWindow, MENU_TITLE_Y, MENU_TITLE_X, "Menu (F1 close)");
    mvwaddch(menuWindow, 2, 0, ACS_LTEE);
    mvwhline(menuWindow, 2, 1, ACS_HLINE, MENU_WINDOW_WIDTH - 2);
    mvwaddch(menuWindow, 2, MENU_WINDOW_WIDTH - 1, ACS_RTEE);

    refresh();

    /* Post the menu */
    post_menu(menu);
    wrefresh(menuWindow);

    int c;
    int finishMenu = 0;
    while(!finishMenu)
    {
        c = wgetch(menuWindow);
        switch(c)
        {
        case KEY_F(1):
            finishMenu = 1;
            break;
        case KEY_DOWN:
            menu_driver(menu, REQ_DOWN_ITEM);
            break;
        case KEY_UP:
            menu_driver(menu, REQ_UP_ITEM);
            break;
        case KEY_ENTER:
        case 10:
            selectedItem = current_item(menu);
            manageSelectedMenuItem(selectedItem);
            finishMenu = 1;
            break;
        default:
            break;
        }
        wrefresh(menuWindow);
    }

    /* Unpost and free all the memory taken up */
    unpost_menu(menu);
    free_menu(menu);
    for(i = 0; i < numberOfOptions; ++i) free_item(menuItemList[i]);
    free(menuItemList);
    refresh();
    delwin(menuWindow);
}


   // Function that generates a generic confirmation dialog
 

void Editor::runConfirmDialog(string message, int *confirmed)
{
    WINDOW *window;
    MENU *menu;
    ITEM *selectedItem;

    ITEM **menuItemList;
    int numberOfOptions, i;

    numberOfOptions = ARRAY_SIZE(confirmationOptions);
    menuItemList = (ITEM **)calloc(numberOfOptions + 1, sizeof(ITEM *));

    for(i = 0; i < numberOfOptions; ++i) menuItemList[i] = new_item(confirmationOptions[i], "");
    menuItemList[numberOfOptions] = (ITEM *)NULL;

    menu = new_menu((ITEM **)menuItemList);

    int windowWidth = strlen(message.c_str()) + 10;
    int x, y;
    getWindowOritinToKeepCentered(CONFIRMATION_WINDOW_HEIGHT, windowWidth, &y, &x);

    // Create the window for the menu
    window = newwin(CONFIRMATION_WINDOW_HEIGHT, windowWidth, y, x);
    keypad(window, TRUE);

    set_menu_win(menu, window);
    set_menu_sub(menu, derwin(window, CONFIRMATION_ITEMS_WINDOW_HEIGHT, windowWidth-2, CONFIRMATION_ITEMS_Y, CONFIRMATION_ITEMS_X));

    // Menu marker
    set_menu_mark(menu, "* ");

    box(window, 0, 0);
    mvwprintw(window, CONFIRMATION_TITLE_Y, CONFIRMATION_TITLE_X, message.c_str());
    mvwaddch(window, 2, 0, ACS_LTEE);
    mvwhline(window, 2, 1, ACS_HLINE, windowWidth - 2);
    mvwaddch(window, 2, windowWidth - 1, ACS_RTEE);

    refresh();

    /* Post the menu */
    post_menu(menu);
    wrefresh(window);

    int c;
    int finishMenu = 0;
    while(!finishMenu)
    {
        c = wgetch(window);
        switch(c)
        {
        case KEY_DOWN:
            menu_driver(menu, REQ_DOWN_ITEM);
            break;
        case KEY_UP:
            menu_driver(menu, REQ_UP_ITEM);
            break;
        case KEY_ENTER:
        case 10:
            selectedItem = current_item(menu);
            if(strcmp(selectedItem->name.str, YES_STR) == 0)
            {
                *confirmed = 1;
            }
            else *confirmed = 0;
            finishMenu = 1;
            break;
        default:
            break;
        }
        wrefresh(window);
    }

    /* Unpost and free all the memory taken up */
    unpost_menu(menu);
    free_menu(menu);
    for(i = 0; i < numberOfOptions; ++i) free_item(menuItemList[i]);
    free(menuItemList);
    refresh();
}


void Editor::runFileNameDialog(string title, string *filename)
{
   
        // Window size
        int windowWidth = 30;
        int windowHeight = 5;

        int x, y;
        getWindowOritinToKeepCentered(windowHeight, windowWidth, &y, &x);
        WINDOW *dialog;
        dialog = newwin(windowHeight, windowWidth, y, x);
        box(dialog, 0, 0);
        mvwprintw(dialog, 1, 2, title.c_str());
        wmove(dialog, 3, 2);
        wrefresh(dialog);

        int c;
        string name;
        unsigned int maxNameLen = windowWidth - 4;
        int finish = 0;
        while(!finish)
        {
            wmove(dialog, 3, name.length()+2);
            c = wgetch(dialog);
            switch(c)
            {
            case KEY_ENTER:
            case 10:
                finish = 1;
                break;
            case 127:
            case KEY_BACKSPACE:
                if(name.length() > 0) name.erase(name.length()-1, 1);
                break;
            default: // TODO filter other keys
                if(name.length() < maxNameLen - 1) name += c;
                break;
            }
            mvwprintw(dialog, 3, 2, "%-*s", maxNameLen, name.c_str()); // String of maxNameLen aligned to left
            wrefresh(dialog);
            refresh();
        }

        filename->append(name);

        // Delete
        delwin(dialog);
}

   //Function that manages the menu entry selected by the user
 
void Editor::manageSelectedMenuItem(const ITEM* item)
{
    // OPEN
    if(strcmp(item->name.str, OPEN_STR) == 0)
    {

        // Check changes
        if(unsavedChanges)
        {
            int confirmed;
            runConfirmDialog("Unsaved changes. Save changes?", &confirmed);
            if(confirmed)
            {
                // Save changes
                if(fileName == NEW_FILE_NAME)
                {
                    string filename;
                    runFileNameDialog("Save file as..", &filename);
                    fileName.clear();
                    fileName.append(filename);
                }
                if(saveFile() == SUCCESS)
                {
                    runMessageDialog("File saved.");
                    unsavedChanges = 0;
                }
                else runMessageDialog("Error saving file.");
            }
        }

        string filename;
        runFileNameDialog("File to open?", &filename);

        if(loadFile(filename) == ERROR)
        {
            string message;
            message.append("ERROR: Could not open file ");
            message.append(filename);
            runMessageDialog(message);
        }
        else
        {
            fileName.clear();
            fileName.append(filename);
            lineNum = 0;
            charNum = 0;
            //runMessageDialog("File loaded.");
        }
        updateFilenameInGui();
    }
    // SAVE
    else if(strcmp(item->name.str, SAVE_STR) == 0)
    {
        if(fileName == NEW_FILE_NAME)
        {
            string filename;
            runFileNameDialog("Save file as..", &filename);
            fileName.clear();
            fileName.append(filename);
            // Check if file exists
            if(fileExists(filename.c_str()))
            {
                int confirmed;
                runConfirmDialog("File already exists. Overwrite?", &confirmed);
                if(!confirmed) return;
            }
        }
        if(saveFile() == SUCCESS)
        {
            runMessageDialog("File saved.");
            unsavedChanges = 0;
            updateFilenameInGui();
        }
        else runMessageDialog("Error saving file.");
    }
    // SAVE AS...
    else if(strcmp(item->name.str, SAVE_AS_STR) == 0)
    {
        string filename;
        runFileNameDialog("Save file as..", &filename);
        fileName.clear();
        fileName.append(filename);
        // Check if file exists
        if(fileExists(filename.c_str()))
        {
            int confirmed;
            runConfirmDialog("File already exists. Overwrite?", &confirmed);
            if(!confirmed) return;
        }
        if(saveFile() == SUCCESS)
        {
            runMessageDialog("File saved.");
            unsavedChanges = 0;
            updateFilenameInGui();
        }
        else runMessageDialog("Error saving file.");
    }
    else if(strcmp(item->name.str, EXIT_STR) == 0)
    {
        if(unsavedChanges)
        {
            int confirmed;
            runConfirmDialog("Unsaved changes. Exit anyway?", &confirmed);
            if(!confirmed) return;
        }
        exitGui = 1;
    }
}



   // updates the file name show in the GUI
 
void Editor::updateFilenameInGui()
{
    mvprintw(LINES-1, MARGIN_X, "%s              ", fileName.c_str());
    refresh();
    move(TEXT_Y, TEXT_X);
}


   //Function to get the origin point for a window, in order to show it centered on the terminal
 
void Editor::getWindowOritinToKeepCentered(int windowHeight, int windowWidht, int *y, int *x)
{
    *y = LINES/2 - windowHeight/2;
    *x = COLS/2 - windowWidht/2;
}

void Editor::runMessageDialog(string msg)
{
    string pressAnyKey;
    pressAnyKey.append("Press any key to continue.");

    // Message window size
    int windowWidth = max(pressAnyKey.length(), msg.length()) + 4; // +4 : 2 borders and 2 spaces
    int windowHeight = 5;

    int x, y;
    getWindowOritinToKeepCentered(windowHeight, windowWidth, &y, &x);
    messageWindow = newwin(windowHeight, windowWidth, y, x);
    box(messageWindow, 0, 0);
    mvwprintw(messageWindow, 1, 2, msg.c_str());
    mvwprintw(messageWindow, 3, 2, "Press any key to continue.");
    wrefresh(messageWindow);

    // Block until key pressed to acknoledge message
    wgetch(messageWindow);

    // Delete
    delwin(messageWindow);
}

   // Function to move cursor to left on text box

void Editor::cursorLeft()
{
    if(charNum > 0)
    {
        charNum--;
        wmove(textWindow,lineNum, charNum);
    }
}

   //Function to move cursor to right on text box

   void Editor::cursorRight()
{
    if( (charNum+1 < (unsigned int)COLS) && (charNum+1 <= textBuffer->lines[lineNum].length()) )
    {
        charNum++;
        wmove(textWindow,lineNum, charNum);
    }
}

   //Function to move cursor up on text box
 
void Editor::cursorUp()
{
    // Check if cursor is in the gui first line
    if(lineNum == guiFirstLine)
    {
        // Check if the gui first line is not buffer line 0
        if(guiFirstLine > 0)
        {
            lineNum--;
            guiFirstLine--;
        }
    }
    else if(lineNum > 0) // Check if buffer has lines above
    {
        lineNum--;
    }

    // Keep cursor position in line, or set to the line end
    if(charNum >= textBuffer->lines[lineNum].length())
        charNum = textBuffer->lines[lineNum].length();

    wmove(textWindow, lineNum - guiFirstLine, charNum);
}

   //Function to move cursor down on text box
   
void Editor::cursorDown()
{
    unsigned int textLines = LINES - 4; // Terminal lines -Menu -2(borders) -filename lines

    // Check if there are more lines below in the GUI
    if(lineNum+1 < (unsigned int)textLines-1)
    {
        // Check if there are more lines in the buffer
        if(lineNum+1 < textBuffer->lines.size())
        {
            lineNum++;
        }
    }
    else
    {
        // If there are no more lines in the GUI
        // If there are more lines in the buffer
        //
        // Check if there are more lines in the buffer
        if(lineNum+1 < textBuffer->lines.size())
        {
            lineNum++;
            guiFirstLine++;
        }
    }

    // Keep cursor position in line, or set to the line end
    if(charNum >= textBuffer->lines[lineNum].length())
        charNum = textBuffer->lines[lineNum].length();

    wmove(textWindow, lineNum - guiFirstLine, charNum);
}

   //Function to print the content of the internal text buffer to the text window
 
void Editor::printText()
{
    // Get size of the file number of lines string
    // For instance: File has 349 lines -> lineNumberSize = 3
    char totalLinesStr[10];
    int bufferLines = textBuffer->lines.size();
    sprintf(totalLinesStr, "%i", bufferLines);
    int lineNumberSize = strlen(totalLinesStr);

    unsigned int textLines = LINES - 4;
    for(unsigned int i=0; i<textLines-1; i++)
    {
        if(i >= textBuffer->lines.size()-guiFirstLine)
        {
            wmove(textWindow, i, 0);
        }
        else
        {
            mvwprintw(textWindow, i, 0, "%0*i %s", lineNumberSize, i+guiFirstLine, textBuffer->lines[i+guiFirstLine].c_str()); //printf ("%0*d\n", 3, 5); --> 005
            wrefresh(textWindow);
        }
        wclrtoeol(textWindow);
    }
    wmove(textWindow,lineNum-guiFirstLine, charNum + lineNumberSize + 1); // +1 for the space after the number
    wrefresh(textWindow);   
}


   //Function to delete current line

void Editor::deleteLine()
{
    textBuffer->removeLine(lineNum);
}

   //Function to delete line at provided index

void Editor::deleteLine(int i)
{
    textBuffer->removeLine(i);
}
