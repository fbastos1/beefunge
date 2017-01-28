#ifndef BEEFUNGE_H
#define BEEFUNGE_H

#define GETCH_STOP_CODE -1

#include <QMainWindow>
#include <QTextEdit>
#include <QSyntaxHighlighter>
#include <QPushButton>
#include <map>
#include <sstream>
#include <stack>
#include <thread>
#include <QMessageBox>
#include <QInputDialog>

#include "honeycombsyntaxhighlighter.h"

namespace Ui {
    class Beefunge;
}

class Beeterpreter;

class Beefunge: public QMainWindow {
    Q_OBJECT
    friend class Beeterpreter;

public:
    explicit Beefunge(QWidget *parent = 0);
    ~Beefunge();
    
    void newFile();
    void openFile(const QString &path = QString());
    void saveFile(const QString &path);
    //void closeFile();
    
    void display(long);
    void displayAsChar(long);
    void displayOnStack(long);
    void displayStackPop();
    
    char getChar();
    const long *getLong();
    
public slots:
    void newFile_sig();
    void openFile_sig();
    void saveFile_sig();
    void saveAs_sig();
    //void closeFile_sig();
    
    void interpreterWillSuspend();
    void interpreterDidSuspend();
    void interpreterWillStop();
    void interpreterDidStop();
    void interpreterWillResume();
    void interpreterDidResume();
    void interpreterWillStart();
    void interpreterDidStart();

    void showAbout();
    //void showLicense();
    
private:
    int start();
    void end();
    void run();
    void step();
    void crawl();
    void suspend();
    void resume();
    int resetInterpreter();
    
    Ui::Beefunge *_ui;
    QTextEdit *_honeycomb, *_stackField, *_outputField;
    HoneycombSyntaxHighlighter *_honeycombHighlighter;
    Beeterpreter *_beeterpreter;
    std::stack<long> *_beeterpreterStack;
    QTextDocument *_stackDocument, *_outputDocument;
    std::stringstream *_inputStream;
    std::map<const char*, std::pair<QPushButton*, void(Beefunge::*)()>> _buttons;
    QInputDialog _inputPrompt;
    QString _aboutDialog, _windowTitle, _filePath;
    QMessageBox *_aboutMessageBox, *_savePromptBox;
    QPushButton *_aboutLicenseButton;
    bool _isFilepathSet;
};

#endif // BEEFUNGE_H
