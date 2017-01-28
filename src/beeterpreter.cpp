#define nil 0

#include <QTime>
#include <QObject>
#include <QCoreApplication>
#include <vector>
#include <string>
#include <sstream>
#include <utility>
#include <cstdlib>
#include <new>
#include <cassert>
#include <cstring>
#include <QTextEdit>
#include <cstdint>

#include "beeterpreter.h"
#include "beefunge.h"

/**

   big todo: breakpoints, somehow
    probs setting flags or something

**/

Beeterpreter::Beeterpreter() {
    _width = _height = 0;
    _outputDisplay = nil;
    _map = nil;
    _stack = nil;
    _isRunning = _isStringMode = false;
    _dir = RIGHT;
    _ptr.first = _ptr.second = -1;
    _parent = nil;
}

static std::vector<std::string>::iterator largest_string_vect(std::vector<std::string>::iterator x, std::vector<std::string>::iterator y) {
    std::vector<std::string>::iterator max = x;
    for (std::vector<std::string>::iterator i = ++x; i < y; ++i) if ((*max).size() < (*i).size()) max = i;
    return max;
}


///todo: consider whether to replace tabs and other whitespace with spaces when reading in the file.
///      if doing that, probably should find a way to modify honeycomb
Beeterpreter::Beeterpreter(std::stringstream *inp, Beefunge *prnt) {
    srand((unsigned int)time(nil));

    std::vector<std::string> map;
    std::string tok;
    while (std::getline(*inp, tok))
        map.push_back(tok);

    assert(map.size());

    std::vector<std::string>::iterator largest_string_itr = largest_string_vect(map.begin(), map.end());

    _width = (*largest_string_itr).size();
    _height = map.size();
    
    _map = (char**)calloc(_height, _height*sizeof(char*));
    for (long i = 0; i < _height; ++i) {
        _map[i] = (char*)calloc(_width + 1, (_width + 1)*sizeof(char));
        strcpy(_map[i], map[i].c_str());
    }

    if (errno == 12 || _map == nil || _map[0] == nil)
        throw std::bad_alloc();
    
    _stack = nil;
    _ptr.first = -1;
    _ptr.second = -1;
    _dir = RIGHT;
    _isRunning = _isStringMode = false;
    _parent = prnt;
    
    _sz = 0;
    _cap = 32;
    _stack = (long*)calloc(_cap, _cap*sizeof(long));
    
    connect(this, &Beeterpreter::displayOnStackPop_ot, _parent, &Beefunge::displayStackPop);
    connect(this, &Beeterpreter::displayOnStack_ot, _parent, &Beefunge::displayOnStack);
    connect(this, &Beeterpreter::displayOnOutput_ot, _parent, &Beefunge::display);
    connect(this, &Beeterpreter::displayOnOutputAsChar_ot, _parent, &Beefunge::displayAsChar);
}

Beeterpreter::~Beeterpreter() {
    for (long i = 0; i < _height; ++i)
        free(_map[i]);

    free(_map);
    delete _stack;
}

void Beeterpreter::start() {
    willStart();
    
    _ptr.first = 0;
    _ptr.second = 0;
    _isRunning = true;
    
    didStart();
}

void Beeterpreter::step() {
    willResume();
    
    didResume();
    
    if (execute(interpret(_ptr))) {
        willStop();
        stop_int();
        return;
    }
    
    switch (_dir) {
    case Beeterpreter::RIGHT:
        _ptr.first = (_ptr.first + 1) % _width;
        break;
    case Beeterpreter::DOWN:
        _ptr.second = (_ptr.second + 1) % _height;
        break;
    case Beeterpreter::LEFT:
        _ptr.first = _ptr.first ? _ptr.first - 1 : _width - 1;
        break;
    case Beeterpreter::UP:
        _ptr.second = _ptr.second ? _ptr.second - 1 : _height - 1;
        break;
    default:
        break;
    }
    willSuspend();
    
    didSuspend();
}

void Beeterpreter::wait(int ms) {
    QTime end = QTime::currentTime().addMSecs(ms);
    while(QTime::currentTime() < end)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void Beeterpreter::crawl(long speed) {
    _isRunning = true;
    
    willResume();
    
    didResume();
    
    op ins;
    while (_isRunning) {
        ins = interpret(_ptr);
        
        if (!execute(ins)) {
            switch (_dir) {
            case Beeterpreter::RIGHT:
                _ptr.first = (_ptr.first + 1) % _width;
                break;
            case Beeterpreter::DOWN:
                _ptr.second = (_ptr.second + 1) % _height;
                break;
            case Beeterpreter::LEFT:
                _ptr.first = _ptr.first ? _ptr.first - 1 : _width - 1;
                break;
            case Beeterpreter::UP:
                _ptr.second = _ptr.second ? _ptr.second - 1 : _height - 1;
                break;
            default:
                break;
            }
        } else {
            stop_int();
            return;
        }
        
        if (ins != NOP && ins != SKP)
            wait((int)speed);
    }
    willSuspend();
    didSuspend();
}

void Beeterpreter::suspend() {
    _isRunning = false;
}

void Beeterpreter::run() {
    _isRunning = true;
    
    willResume();
    
    didResume();
    
    op ins;
    while (true && _isRunning) {
        ins = interpret(_ptr);

        //in case of an eternal loop, this makes it so that the
        //  interpreter doesn't hang the whole ui
        QCoreApplication::processEvents(QEventLoop::AllEvents, 1);
        //runs faster without this, but at the possible expense of stability.
        // this is the only safeguard against hanging in case of an infinite loop
        // in interpreted code; therefore:
        ///todo: when preferences panel is implemented, add a "high performance"
        /// option (i.e., no-compromises, not-even-the-same-method option): connect the
        /// "run" button to another nearly identical method without the line above
        /// (to avoid adding the overhead of a conditional, since the whole point
        /// of it is performance).
        
        
        if (!execute(ins)) {
            switch (_dir) {
            case Beeterpreter::RIGHT:
                _ptr.first = (_ptr.first + 1) % _width;
                break;
            case Beeterpreter::DOWN:
                _ptr.second = (_ptr.second + 1) % _height;
                break;
            case Beeterpreter::LEFT:
                _ptr.first = _ptr.first ? _ptr.first - 1 : _width - 1;
                break;
            case Beeterpreter::UP:
                _ptr.second = _ptr.second ? _ptr.second - 1 : _height - 1;
                break;
            default:
                break;
            }
        } else {
            stop_int();
            return;
        }
    }
    
    willSuspend();
    
    didSuspend();
}

void Beeterpreter::stop() {
    willStop();
    
    _isRunning = false;
}

void Beeterpreter::stop_int() {
    _isRunning = _isStringMode = false;
    _sz = 0;
    _dir = RIGHT;
    _ptr.first = _ptr.second = -1;
    
    didStop();
}


void Beeterpreter::setInput(std::stringstream *inp) {
    std::vector<std::string> map;
    std::string tok;
    while (std::getline(*inp, tok))
        map.push_back(tok);

    assert(map.size());
    
    std::vector<std::string>::iterator largest_string_itr = largest_string_vect(map.begin(), map.end());
    
    delete _map;
    
    _map = (char**)calloc(map.size(), map.size()*sizeof(char*));
    
    if (errno == 12 || _map == nil)
        throw std::bad_alloc();
    
    for (std::size_t i = 0; i < map.size(); ++i) {
        _map[i] = (char*)calloc((*largest_string_itr).size() + 1, ((*largest_string_itr).size() + 1) * sizeof(char));
        strcpy(_map[i], map[i].c_str());
    }
    
    if (_map[0] == nil)
        throw std::bad_alloc();
    
    _width = (*largest_string_itr).size();
    _height = map.size();
}

void Beeterpreter::emptyStack() {
    _sz = 0;
}

int Beeterpreter::execute(op op) {
    long x, y, z;
    const long *k;
    
    if (_sz+1 == _cap)
        expandStack();
    
    switch (op) {
    case Beeterpreter::PSI:
        //48 is offset of numbers in ascii table ((int)'0' == 48)
        _stack[_sz++] = (long)_map[_ptr.second][_ptr.first]-48;
        displayOnStack_ot((long)_map[_ptr.second][_ptr.first]-48);
        break;
    case Beeterpreter::PSC:
        _stack[_sz++] = (long)_map[_ptr.second][_ptr.first];
        displayOnStack_ot((long)_map[_ptr.second][_ptr.first]);
        break;
    case Beeterpreter::NOP:
        break;
    case Beeterpreter::POP:
        --_sz;
        displayOnStackPop_ot();
        break;
    case Beeterpreter::ADD:
        x = _sz ? _stack[--_sz] : 0;
        displayOnStackPop_ot();
        x += _sz ? _stack[--_sz] : 0;
        displayOnStackPop_ot();
        _stack[_sz++] = x;
        displayOnStack_ot(x);
        break;
    case Beeterpreter::SUB:
        x = _sz ? _stack[--_sz] : 0;
        displayOnStackPop_ot();
        y = _sz ? _stack[--_sz] : 0;
        displayOnStackPop_ot();
        _stack[_sz++] = y - x;
        displayOnStack_ot(y - x);
        break;
    case Beeterpreter::MUL:
        x = _sz ? _stack[--_sz] : 0;
        displayOnStackPop_ot();
        x *= _sz ? _stack[--_sz] : 0;
        displayOnStackPop_ot();
        _stack[_sz++] = x;
        displayOnStack_ot(x);
        break;
    case Beeterpreter::DIV:
        x = _sz ? _stack[--_sz] : 0;
        displayOnStackPop_ot();
        y = _sz ? _stack[--_sz] : 0;
        displayOnStackPop_ot();
        if (!x) {
            k = _parent->getLong();
            if (k[1]) {
                willStop();
                stop_int();
                delete k;
                return 1;
            }
            _stack[_sz++] = *k;
            delete k;
        } else {
            _stack[_sz++] = y / x;
        }
        displayOnStack_ot(_stack[_sz-1]);
        break;
    case Beeterpreter::MOD:
        x = _sz ? _stack[--_sz] : 0;
        displayOnStackPop_ot();
        y = _sz ? _stack[--_sz] : 0;
        displayOnStackPop_ot();
        if (!x) {
            k = _parent->getLong();
            if (k[1]) {
                willStop();
                stop_int();
                delete k;
                return 1;
            }
            _stack[_sz++] = *k;
            delete k;
        } else {
            _stack[_sz++] = y % x;
        }
        displayOnStack_ot(_stack[_sz-1]);
        break;
    case Beeterpreter::NOT:
        if (_sz) _stack[_sz-1] = !_stack[_sz-1];
        displayOnStackPop_ot();
        displayOnStack_ot(_sz ? _stack[_sz-1] : 1);
        break;
    case Beeterpreter::CMP:
        x = _sz ? _stack[--_sz] : 0;
        displayOnStackPop_ot();
        y = _sz ? _stack[--_sz] : 0;
        displayOnStackPop_ot();
        _stack[_sz++] = (long)(y > x);
        displayOnStack_ot((long)(y > x));
        break;
    case Beeterpreter::MOR:
        _dir = RIGHT;
        break;
    case Beeterpreter::MOL:
        _dir = LEFT;
        break;
    case Beeterpreter::MUP:
        _dir = UP;
        break;
    case Beeterpreter::MDN:
        _dir = DOWN;
        break;
    case Beeterpreter::MRD:
        _dir = static_cast<direction>(rand() % 4);
        break;
    case Beeterpreter::HIF:
        _dir = (_sz ? _stack[--_sz] : 0) ? LEFT : RIGHT;
        displayOnStackPop_ot();
        break;
    case Beeterpreter::VIF:
        _dir = (_sz ? _stack[--_sz] : 0) ? UP : DOWN;
        displayOnStackPop_ot();
        break;
    case Beeterpreter::STR:
        _isStringMode = !_isStringMode;
        break;
    case Beeterpreter::DUP:
        _stack[_sz] = _sz ? _stack[_sz-1] : 0;
        displayOnStack_ot(_stack[_sz++]);
        break;
    case Beeterpreter::SWP:
        switch (_sz) {
        case 0:
            _stack[_sz++] = 0;
            _stack[_sz++] = 0;
            displayOnStack_ot(0);
            displayOnStack_ot(0);
            break;
        case 1:
            _stack[_sz++] = 0;
            displayOnStack_ot(0);
            break;
        default:
            std::swap(_stack[_sz-1], _stack[_sz-2]);
            displayOnStackPop_ot();
            displayOnStackPop_ot();
            displayOnStack_ot(_stack[_sz-2]);
            displayOnStack_ot(_stack[_sz-1]);
        }
        break;
    case Beeterpreter::OIN:
        displayOnOutput_ot(_sz ? _stack[--_sz] : 0);
        displayOnStackPop_ot();
        break;
    case Beeterpreter::OCH:
        displayOnOutputAsChar_ot(_sz ? _stack[--_sz] : 0);
        displayOnStackPop_ot();
        break;
    case Beeterpreter::SKP:
        switch (_dir) {
        case Beeterpreter::RIGHT:
            _ptr.first = (_ptr.first + 1) % _width;
            break;
        case Beeterpreter::DOWN:
            _ptr.second = (_ptr.second + 1) % _height;
            break;
        case Beeterpreter::LEFT:
            _ptr.first = _ptr.first ? _ptr.first - 1 : _width - 1;
            break;
        case Beeterpreter::UP:
            _ptr.second = _ptr.second ? _ptr.second - 1 : _height - 1;
            break;
        }
        break;
    case Beeterpreter::PUT:
        y = _sz ? _stack[--_sz] : 0;
        displayOnStackPop_ot();
        x = _sz ? _stack[--_sz] : 0;
        displayOnStackPop_ot();
        z = _sz ? _stack[--_sz] : 0;
        displayOnStackPop_ot();
        if (x >= _width || y >= _height) expand(std::pair<long, long>(x, y));
        _map[y][x] = (char)z;
            
        ///todo: make this modify honeycomb display
        
        break;
    case Beeterpreter::GET:
        y = _sz ? _stack[--_sz] : 0;
        displayOnStackPop_ot();
        x = _sz ? _stack[--_sz] : 0;
        displayOnStackPop_ot();
        _stack[_sz++] = y < _height || x < _width ? (unsigned int)_map[y][x] : 32;
        displayOnStack_ot(y < _height || x < _width ? (unsigned int)_map[y][x] : 32);
        break;
    case Beeterpreter::GCH:
        x = _parent->getChar();
        if (x == GETCH_STOP_CODE) {
            willStop();
            stop_int();
            return 1;
        }
        _stack[_sz++] = x;
        displayOnStack_ot(x);
        break;
    case Beeterpreter::GIN:
        k = _parent->getLong();
        if (k[1]) {
            willStop();
            stop_int();
            delete k;
            return 1;
        }
        _stack[_sz++] = *k;
        displayOnStack_ot(*k);
        delete k;
        break;
    case Beeterpreter::END:
        willStop();
        stop_int();
        return 1;
    default:
        break;
    }
    
#if DEBUG
    assert(_sz >= 0);
#endif

    return 0;
    //if END, call stop() and return 1 (stop code)
}

Beeterpreter::op Beeterpreter::interpret(std::pair<long, long> ptr) {
    char ch = _map[ptr.second][ptr.first];

    if (_isStringMode && ch != '\"')
        return PSC;

    switch (ch) {
    case '\"':
        return STR;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        return PSI;
    case '$':
        return POP;
    case '+':
        return ADD;
    case '-':
        return SUB;
    case '*':
        return MUL;
    case '/':
        return DIV;
    case '%':
        return MOD;
    case '!':
        return NOT;
    case '`':
        return CMP;
    case '>':
        return MOR;
    case '<':
        return MOL;
    case 'v':
        return MDN;
    case '^':
        return MUP;
    case '_':
        return HIF;
    case '|':
        return VIF;
    case ':':
        return DUP;
    case '\\':
        return SWP;
    case '.':
        return OIN;
    case ',':
        return OCH;
    case '#':
        return SKP;
    case 'p':
        return PUT;
    case 'g':
        return GET;
    case '&':
        return GIN;
    case '~':
        return GCH;
    case '@':
        return END;
    default:
        return NOP;
    }
}

void Beeterpreter::expandStack() {
    _stack = (long*)realloc((void*)_stack, _cap*2*sizeof(long));

    if (!_stack || errno == 12)
        throw std::bad_alloc();

    _cap *= 2;
}

void Beeterpreter::expand(std::pair<long, long> bounds) {
    if (_height < bounds.second) {
        _height = bounds.second;
        _map = (char**)realloc((void*)_map, _height*sizeof(char*));
    }

    if (!_map || errno == 12)
        throw std::bad_alloc();
    
    if (_width < bounds.first) {
        _width = bounds.first;
        
        for (long i = 0; i < _height; ++i)
            _map[i] = (char*)realloc((void*)_map[i], _width*sizeof(char));
    }
    
    if (!_map[0] || errno == 12)
        throw std::bad_alloc();
}

std::pair<long, long> Beeterpreter::getInstructionPointer() {
    return _ptr;
}

long Beeterpreter::getWidth() {
    return _width;
}

long Beeterpreter::getHeight() {
    return _height;
}

Beeterpreter::direction Beeterpreter::getDirection() {
    return _dir;
}

const long *Beeterpreter::getStack() const {
    return const_cast<long*>(_stack);
}

//bool Beeterpreter::isStarted() {
//}

bool Beeterpreter::isRunning() {
    return _isRunning;
}
