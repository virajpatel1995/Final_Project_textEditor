#ifndef EDITOR_H
#define EDITOR_H

#include <string>
#include <vector>
#include <ncurses.h>
#include <menu.h>

using namespace std;

class Buffer
{
public:
    Buffer();

    vector<string> lines;

    void insertLine(string, int);
    void appendLine(string);
    void removeLine(int);
    void clear();
    string substituteTabs(string); // Substitute tabs for 4 spaces
};

class Editor
{
private:
    unsigned int lineNum;
    unsigned int charNum;
    Buffer* textBuffer;
    string fileName;
    WINDOW *borderWindow;
    WINDOW *textWindow;
    int isNewFile;
    int unsavedChanges;
    int exitGui;
    WINDOW *messageWindow;
    WINDOW *fileNameWindow;
    unsigned int guiFirstLine;

    void init();

    int loadFile(string filename);
    int saveFile();
    bool fileExists(string filename);

    void createGui(void);
    void updateFilenameInGui();
    void handleInput(int c);
    void printText();

    void cursorUp();
    void cursorDown();
    void cursorLeft();
    void cursorRight();

    void deleteLine();
    void deleteLine(int);

    void runMenuDialog();
    void runMessageDialog(string msg);
    void runConfirmDialog(string message, int *confirmed);
    void runFileNameDialog(string title, string *filename);

    void getWindowOritinToKeepCentered(int windowHeight, int windowWidht, int *y, int *x);
    void manageSelectedMenuItem(const ITEM *item);

public:
    Editor();
    Editor(string filename);
    void run();
    void freeResources();
};

#endif
