#ifndef BEETERPRETER_H
#define BEETERPRETER_H

#include <QTextEdit>
#include <QObject>
#include <string>
#include <sstream>
#include <utility>
#include <stack>
#include <mutex>

template <class T> class Beestack;
class Beefunge;

class Beeterpreter: public QObject {
    Q_OBJECT
    
public:
    enum op {
        PSI, //push int
        PSC, //push char (string mode)
        NOP, //nop
        POP, //pop
        ADD, //addition
        SUB, //subtraction
        MUL, //multiplication
        DIV, //division
        MOD, //mod
        NOT, //not
        CMP, //compare
        MOR, //move right
        MOL, //move left
        MUP, //move up
        MDN, //move down
        MRD, //move random
        HIF, //horizontal if
        VIF, //vertical if
        STR, //string mode
        DUP, //duplicate
        SWP, //swap
        OIN, //out integer
        OCH, //out char
        SKP, //bridge (skip)
        PUT, //put
        GET, //get
        GIN, //get integer
        GCH, //get char
        END, //end
    };

    enum direction {
        RIGHT,
        DOWN,
        LEFT,
        UP,
    };

    Beeterpreter();
    Beeterpreter(std::stringstream*, Beefunge*);

    ~Beeterpreter();

    void start();
    void step();
    
    void wait(int);
    void crawl(long);
    
    
    void suspend();
    void resume();
    void run();
    void stop();
    void stop_int();
    
    void setInput(std::stringstream*);
    void emptyStack();
    
    int execute(op);
    op interpret(std::pair<long, long>);

    void expandStack();
    void expand(std::pair<long, long>);
    
    std::pair<long, long> getInstructionPointer();
    long getWidth();
    long getHeight();
    direction getDirection();
    const long *getStack() const;
    //bool isStarted();
    bool isRunning();
    
signals:
    void willSuspend();
    void didSuspend();
    void willStop();
    void didStop();
    void willResume();
    void didResume();
    void willStart();
    void didStart();
    
    void displayOnStack_ot(long);
    void displayOnStackPop_ot();
    void displayOnOutput_ot(long);
    void displayOnOutputAsChar_ot(long);
    
private:
    QTextEdit *_outputDisplay;
    char **_map;
    long _width, _height;
    std::pair<long, long> _ptr;
    long *_stack;
    long _sz, _cap;
    direction _dir;
    bool _isRunning, _isStringMode;
    Beefunge *_parent;
    std::mutex _mtx;
};

#endif // BEETERPRETER_H
