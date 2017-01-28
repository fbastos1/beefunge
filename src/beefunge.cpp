#define nil 0
#define _BEEFUNGE_VERSION_STRING_ "Beefunge v0.1b <i>(build e042c9)</i>"


#if DEBUG
#include <cassert>
#include <iostream>
#include <chrono>
#endif


#include <QMainWindow>
#include <QWidget>
#include <QTextCursor>
#include <QfontMetrics>
#include <QPalette>
#include <QColor>
#include <QInputDialog>
#include <QAction>
#include <QMessageBox>
#include <QFileDialog>
#include <QIODevice>
#include <QTextStream>

#include "beefunge.h"
#include "beeterpreter.h"
#include "ui_beefunge.h"
#include "honeycombsyntaxhighlighter.h" 


Beefunge::Beefunge(QWidget *parent): QMainWindow(parent), _ui(new Ui::Beefunge) {
    
    _ui->setupUi(this);
    
    QAction *about = new QAction("About", this);
    findChild<QMenu*>("_mainBar")->addAction(about);
    
    connect(about, &QAction::triggered, this, &Beefunge::showAbout);
    
    _aboutDialog = "<br><br>Built with the Qt Library, freely available at <a href='https://qt.io'>qt.io</a><br><br>Licensed under the GNU LGPL v3";
    
    _aboutMessageBox = new QMessageBox(this);
    _aboutMessageBox->setText(_BEEFUNGE_VERSION_STRING_);
    _aboutMessageBox->setInformativeText(_aboutDialog);
    _aboutMessageBox->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
    
    _aboutMessageBox->addButton("OK", QMessageBox::AcceptRole);
    
    
    _savePromptBox = new QMessageBox(this);
    _savePromptBox->setText("You have unsaved changes.");
    _savePromptBox->setInformativeText("Do you want to save?");
    _savePromptBox->setTextInteractionFlags(Qt::NoTextInteraction);
    
    _savePromptBox->addButton("Cancel", QMessageBox::RejectRole);
    _savePromptBox->addButton("No", QMessageBox::NoRole);
    _savePromptBox->addButton("Yes", QMessageBox::YesRole);
    
    
    QFont font;
    font.setFamily("Monaco");
    font.setFixedPitch(true);
    font.setPointSize(10);
    
    _honeycomb = findChild<QTextEdit*>("_honeycomb");
    
    QFontMetrics fm(font);
    _honeycomb->setCursorWidth(fm.averageCharWidth());
    _honeycomb->setFont(font);
    
    QTextOption opt = _honeycomb->document()->defaultTextOption();
    opt.setFlags(QTextOption::ShowTabsAndSpaces);
    opt.setWrapMode(QTextOption::NoWrap);
    
    _honeycomb->document()->setDefaultTextOption(opt);
    _honeycomb->setTabStopWidth(fm.averageCharWidth());
    
    _honeycombHighlighter = new HoneycombSyntaxHighlighter(_honeycomb->document());

    _buttons["run"] = std::pair<QPushButton*, void(Beefunge::*)()>(findChild<QPushButton*>("_runButton"), &Beefunge::run);
    _buttons["step"] = std::pair<QPushButton*, void(Beefunge::*)()>(findChild<QPushButton*>("_stepButton"), &Beefunge::step);
    _buttons["crawl"] = std::pair<QPushButton*, void(Beefunge::*)()>(findChild<QPushButton*>("_crawlButton"), &Beefunge::crawl);
    _buttons["suspend"] = std::pair<QPushButton*, void(Beefunge::*)()>(findChild<QPushButton*>("_suspendButton"), &Beefunge::suspend);
    _buttons["stop"] = std::pair<QPushButton*, void(Beefunge::*)()>(findChild<QPushButton*>("_stopButton"), &Beefunge::end);
    
    _buttons["suspend"].first->setDisabled(true);
    _buttons["stop"].first->setDisabled(true);
    
    for (auto&& i: _buttons)
        connect(i.second.first, &QPushButton::released, this, i.second.second);
    
    _beeterpreterStack = nil;

    _stackField = findChild<QTextEdit*>("_stackDisplay");
    _outputField = findChild<QTextEdit*>("_outputDisplay");

    _stackDocument = const_cast<QTextDocument*>(_stackField->document());
    _outputDocument = const_cast<QTextDocument*>(_outputField->document());

    _inputStream = new std::stringstream(_honeycomb->toPlainText().toUtf8().constData());
    
    _beeterpreter = nil;

    _inputPrompt.setCancelButtonText("Stop");
    
    _filePath = "Untitled";
    _isFilepathSet = false;
    
    _windowTitle = "Beefunge - " + _filePath;
    setWindowTitle(_windowTitle);
    
    {
        QAction *action = findChild<QAction*>("_actionNewFile");
        action->setShortcuts(QKeySequence::New);
        action->setStatusTip("Create a new file");
        connect(action, &QAction::triggered, this, &Beefunge::newFile_sig);
        
        action = findChild<QAction*>("_actionOpenFile");
        action->setShortcuts(QKeySequence::Open);
        action->setStatusTip("Open an existing file");
        connect(action, &QAction::triggered, this, &Beefunge::openFile_sig);
        
        action = findChild<QAction*>("_actionSaveFile");
        action->setShortcuts(QKeySequence::Save);
        action->setStatusTip("Save to disk");
        connect(action, &QAction::triggered, this, &Beefunge::saveFile_sig);
        
        action = findChild<QAction*>("_actionSaveAs");
        action->setShortcuts(QKeySequence::SaveAs);
        action->setStatusTip("Save to disk as...");
        connect(action, &QAction::triggered, this, &Beefunge::saveAs_sig);
        
        /*action = findChild<QAction*>("_actionCloseFile");
        action->setShortcuts(QKeySequence::Close);
        action->setStatusTip("Close file");
        connect(action, &QAction::triggered, this, &Beefunge::closeFile_sig);*/
    }
}

Beefunge::~Beefunge() {
    ///todo: prompt for saving before closing, if at all possible at this point
    delete _ui;
    delete _honeycombHighlighter;
    delete _inputStream;
}

void Beefunge::newFile_sig() {
    int inp;
    QString path;
    
    if (_isFilepathSet) {
        QFile temp(_filePath);
        
        if (_honeycomb->toPlainText() != temp.readAll()) {
            inp = _savePromptBox->exec();
            switch (inp) {
                case 0:
                    return;
                case 2:
                    path = QFileDialog::getSaveFileName(this, "Save file", "./", "Befunge Source (*.bf);;All files (*)");
                    if (!path.length())
                        return;
                    _filePath = path;
                    saveFile(path);
                case 1:
                default:
                    break;
            }
        }
    } else if (_honeycomb->toPlainText().length()) {
        inp = _savePromptBox->exec();
        switch (inp) {
            case 0:
                return;
            case 2:
                path = QFileDialog::getSaveFileName(this, "Save file", "./", "Befunge Source (*.bf);;All files (*)");
                if (!path.length())
                    return;
                _filePath = path;
                saveFile(path);
            case 1:
            default:
                break;
        }
    }
    
    if (_beeterpreter)
        resetInterpreter();
    else
        _honeycomb->setPlainText("");
    
    newFile();
    
    _isFilepathSet = false;
}

void Beefunge::openFile_sig() {
    QString path = QFileDialog::getOpenFileName(this, "Open file", "./", "Befunge Source (*.bf);;Text files (*.txt);;All files (*)");
    
    bool userDidOpenFile = false;
    
    if (path.length()) {
        _isFilepathSet = true;
        userDidOpenFile = true;
        _filePath = path;
    }
    
    if (userDidOpenFile) {
        if (_beeterpreter)
            resetInterpreter();
        openFile(_filePath);
    }
}

void Beefunge::saveFile_sig() {
    ///todo: "save as" if writing to file fails
    
    if (!_isFilepathSet) {
        QString path = QFileDialog::getSaveFileName(this, "Save file", "./", "Befunge Source (*.bf);;All files (*)");
        
        if (path.length()) {
            _isFilepathSet = true;
            _filePath = path;
        }
    }
    
    if (_isFilepathSet)
        saveFile(_filePath);
}

void Beefunge::saveAs_sig() {
    QString path = QFileDialog::getSaveFileName(this, "Save file", "./", "Befunge Source (*.bf);;All files (*)");
    
    if (path.length()) {
        _isFilepathSet = true;
        _filePath = path;
    }
    
    if (_isFilepathSet)
        saveFile(_filePath);
}

//void Beefunge::closeFile_sig() {
    //prompt to save depending on whether modified
    
    //_didsaveFile = true
    //_didmodifyfile = false
    //clear everything, maybe destroy beeterpreter
//}



void Beefunge::interpreterWillSuspend() {
    _buttons["suspend"].first->setDisabled(true);
}

void Beefunge::interpreterDidSuspend() {
    _buttons["run"].first->setDisabled(false);
    _buttons["crawl"].first->setDisabled(false);
    _buttons["step"].first->setDisabled(false);
    _buttons["stop"].first->setDisabled(false);
}

void Beefunge::interpreterWillStop() {
    _buttons["stop"].first->setDisabled(true);
    _buttons["suspend"].first->setDisabled(true);
}

void Beefunge::interpreterDidStop() {
    _buttons["run"].first->setDisabled(false);
    _buttons["crawl"].first->setDisabled(false);
    _buttons["step"].first->setDisabled(false);
    
    _honeycomb->setTextInteractionFlags(Qt::TextEditorInteraction);
    
    //clean up other shit (clean streams and all of the stuff
    //  that's done before the interpreter's run
}

void Beefunge::interpreterWillResume() {
    //cursor stuff!
    _buttons["run"].first->setDisabled(true);
    _buttons["crawl"].first->setDisabled(true);
    _buttons["step"].first->setDisabled(true);

    _honeycomb->setTextInteractionFlags(Qt::NoTextInteraction);
}

void Beefunge::interpreterDidResume() {
    _buttons["stop"].first->setDisabled(false);
    _buttons["suspend"].first->setDisabled(false);
    
    //cursor stuff!
}

void Beefunge::interpreterWillStart() {
    //cursor stuff!
    _buttons["run"].first->setDisabled(true);
    _buttons["crawl"].first->setDisabled(true);
    _buttons["step"].first->setDisabled(true);
    
    _honeycomb->setTextInteractionFlags(Qt::NoTextInteraction);
}

void Beefunge::interpreterDidStart() {
    //cursor stuff?
    
    _buttons["stop"].first->setDisabled(false);
    _buttons["suspend"].first->setDisabled(false);
}

void Beefunge::showAbout() {
    _aboutMessageBox->show();
}

char Beefunge::getChar() {
    ///todo: figure out why the prompt button says "Cancel" instead of "Stop", even though _inputPrompt.cancelButtonText() == "Stop"
    ///  might be a Qt bug, but probably something stupid I overlooked
    
    bool didInput = false;
    QString ret = _inputPrompt.getText(this, "Input", "Please input a char:", QLineEdit::Normal, "", &didInput);
    return didInput ? ret.data()[0].toLatin1() : GETCH_STOP_CODE;
}

const long *Beefunge::getLong() {
    long *ret = new long[2]();
    *ret = _inputPrompt.getInt(this, "Input", "Please input an int:", 0, -2147483647, 2147483647, 1, reinterpret_cast<bool*>(ret+1));
    ret[1] = !ret[1];
    return ret;
}

void Beefunge::newFile() {
    _filePath = "Untitled";
    
    _windowTitle = "Beefunge - " + _filePath;
    setWindowTitle(_windowTitle);
    
    return;
}

void Beefunge::openFile(QString const &path) {
    _filePath = path;
    
    QFile file(_filePath);
    
#if DEBUG
    assert(file.open(QIODevice::ReadWrite | QIODevice::Text));
#else
    ///todo: show error message if can't load file
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text))
        return;
#endif
    
    QTextStream fs(&file);
    _honeycomb->setText(fs.readAll());
    
    _windowTitle = "Beefunge - " + _filePath;
    setWindowTitle(_windowTitle);
    
    file.close();
}

void Beefunge::saveFile(const QString &path) {
    QFile file(_isFilepathSet ? _filePath : path);
    
#if DEBUG
    assert(file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate));
#else
    ///todo: show error message if can't write to file
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate))
        return;
#endif
    
    QTextStream ts(&file);
    ts << _honeycomb->toPlainText();
    
    _windowTitle = "Beefunge - " + _filePath;
    setWindowTitle(_windowTitle);
    
    file.close();
}


///todo: optimize (ie: make not dumb) display(), displayAsChar(),
///     displayOnStack() and displayStackPop().
/// consider:
///     · condensing the methods into one, two, or three (but probably not)
///     · instantiating one or two std::stringstream(s) at construction and
///       using those for displaying stuff. The display field stream doesn't
///       even need to be cleaned, ever, until the interpreter stops
///     · filing a qt bug report for allocating 50gb of memory every time
///       if you put the document and string declarations in the same line
///   QString qs = const_cast<QTextDocument*>(_outputField->document())->toPlainText();
///   ^ this line explodes your computer, probably. Or something. I wouldn't run it
///     · something else I forgot but will write down whenever I remember
void Beefunge::display(long x) {
    std::stringstream *ss = new std::stringstream;
    QTextDocument *doc = const_cast<QTextDocument*>(_outputField->document());
    *ss << doc->toPlainText().toStdString() << x << ' ';
    
    doc->setPlainText(QString(ss->str().c_str()));
    
    delete ss;
}

void Beefunge::displayAsChar(long x) {
    QTextDocument *doc = const_cast<QTextDocument*>(_outputField->document());
    QString qs = doc->toPlainText() + QString((char)x);
    
    doc->setPlainText(qs);
    return;
}

void Beefunge::displayOnStack(long x) {
    QTextDocument *doc = const_cast<QTextDocument*>(_stackField->document());
    std::stringstream *ss = new std::stringstream;
    
    if (31 < x && x < 127)
        *ss << x << " (" << (char)x << ")\n" << doc->toPlainText().toStdString();
    else
        *ss << x << '\n' << doc->toPlainText().toStdString();
    
    //this line eats your performance on a large stack
    doc->setPlainText(QString(ss->str().c_str()));
    
    delete ss;
}

void Beefunge::displayStackPop() {
    QTextDocument *doc = const_cast<QTextDocument*>(_stackField->document());
    std::stringstream *ss = new std::stringstream;
    
    *ss << doc->toPlainText().toStdString();
    
    std::string tok;
    QString rs;
    
    //skip first line
    std::getline(*ss, tok);
    while (std::getline(*ss, tok))
        rs += QString((tok + '\n').c_str());
    
    doc->setPlainText(rs);
    
    delete ss;
}

int Beefunge::start() {
    if (_beeterpreter || !_honeycomb->document()->toPlainText().length())
        return 1; //fail (ie didn't have to start; was already started, or there was no text in honeycomb)

    _inputStream->str(_honeycomb->toPlainText().toUtf8().toStdString());

    _beeterpreter = new Beeterpreter(_inputStream, this);
    
    connect(_beeterpreter, &Beeterpreter::willSuspend, this, &Beefunge::interpreterWillSuspend);
    connect(_beeterpreter, &Beeterpreter::didSuspend, this, &Beefunge::interpreterDidSuspend);
    connect(_beeterpreter, &Beeterpreter::willStop, this, &Beefunge::interpreterWillStop);
    connect(_beeterpreter, &Beeterpreter::didStop, this, &Beefunge::interpreterDidStop);
    connect(_beeterpreter, &Beeterpreter::willResume, this, &Beefunge::interpreterWillResume);
    connect(_beeterpreter, &Beeterpreter::didResume, this, &Beefunge::interpreterDidResume);
    connect(_beeterpreter, &Beeterpreter::willStart, this, &Beefunge::interpreterWillStart);
    connect(_beeterpreter, &Beeterpreter::didStart, this, &Beefunge::interpreterDidStart);
    
    _beeterpreter->start();

    return 0;
}

void Beefunge::end() {
    _beeterpreter->stop();
}

void Beefunge::run() {
    if (!_beeterpreter) {
        if (!start())
            _beeterpreter->run();
    } else if (_beeterpreter->getInstructionPointer().first == -1) {
        if (!resetInterpreter())
            _beeterpreter->run();
    } else {
        _beeterpreter->run();
    }
}

void Beefunge::step() {
    if (!_beeterpreter) {
        if (!start())
            _beeterpreter->step();
    } else if (_beeterpreter->getInstructionPointer().first == -1) {
        if (!resetInterpreter())
            _beeterpreter->step();
    } else {
        _beeterpreter->step();
    }
}

void Beefunge::crawl() {
    ///todo: make this user-settable, but don't change the method signature
    ///     or qt will explode
    const long speed = 500;
    
    if (!_beeterpreter) {
        if (!start())
            _beeterpreter->crawl(speed);
    } else if (_beeterpreter->getInstructionPointer().first == -1) {
        if (!resetInterpreter())
            _beeterpreter->crawl(speed);
    } else {
        _beeterpreter->crawl(speed);
    }
}

void Beefunge::suspend() {
    _beeterpreter->suspend();
}

int Beefunge::resetInterpreter() {
#if DEBUG
    assert(_beeterpreter);
#endif
    
    if (!_beeterpreter || !_honeycomb->toPlainText().length())
        return 1;
    
    _ui->_outputDisplay->clear();
    _ui->_stackDisplay->clear();
    
    _beeterpreter->emptyStack();
    
    _inputStream->clear();
    _inputStream->str(_honeycomb->document()->toPlainText().toStdString());
    
    _beeterpreter->setInput(_inputStream);
    _beeterpreter->start();
    
    return 0;
}




