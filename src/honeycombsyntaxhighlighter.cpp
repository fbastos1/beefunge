#include <QRegExp>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QBrush>
#include <QColor>
#include <map>
#include <string>

#include "honeycombsyntaxhighlighter.h"

HoneycombSyntaxHighlighter::HoneycombSyntaxHighlighter(QTextDocument *parent): QSyntaxHighlighter(parent) {
    ///todo: make a map or something with the different styles & different colors.
    ///todo 2: make a light style
    
    _formats["flow"] = std::pair<QTextCharFormat, QRegExp>(QTextCharFormat(), QRegExp("[<>v^?_|#]"));
    _formats["flow"].first.setForeground(QBrush(QColor(230,90,200)));
    
    _formats["math"] = std::pair<QTextCharFormat, QRegExp>(QTextCharFormat(), QRegExp("[-+*/!%`]"));
    _formats["math"].first.setForeground(QBrush(QColor(0, 230, 230)));

    _formats["stack"] = std::pair<QTextCharFormat, QRegExp>(QTextCharFormat(), QRegExp("[\\\\:$.,]"));
    _formats["stack"].first.setForeground(QBrush(QColor(85, 198, 56)));

    _formats["input"] = std::pair<QTextCharFormat, QRegExp>(QTextCharFormat(), QRegExp("[&~]"));
    _formats["input"].first.setForeground(QBrush(QColor(200, 200, 200)));

    _formats["end"] = std::pair<QTextCharFormat, QRegExp>(QTextCharFormat(), QRegExp("[@]"));
    _formats["end"].first.setForeground(QBrush(QColor(255, 255, 255)));

    _formats["string"] = std::pair<QTextCharFormat, QRegExp>(QTextCharFormat(), QRegExp("\""/*"\"(.*?)\""*/));
    _formats["string"].first.setForeground(QBrush(QColor(230, 120, 100)));

    _formats["number"] = std::pair<QTextCharFormat, QRegExp>(QTextCharFormat(), QRegExp("[0-9]"));
    _formats["number"].first.setForeground(QBrush(QColor(104, 151, 187)));

    _formats["error"] = std::pair<QTextCharFormat, QRegExp>(QTextCharFormat(), QRegExp("\t"));
    _formats["error"].first.setForeground(QBrush(QColor(247, 32, 32)));
    
    _formats["whitespace"] = std::pair<QTextCharFormat, QRegExp>(QTextCharFormat(), QRegExp("[ ]"));
    _formats["whitespace"].first.setForeground(QBrush(QColor(130, 130, 130)));

    _formats["pg"] = std::pair<QTextCharFormat, QRegExp>(QTextCharFormat(), QRegExp("[pg]"));
    _formats["pg"].first.setForeground(QBrush(QColor(255, 198, 109)));

    _formats["comment"] = std::pair<QTextCharFormat, QRegExp>(QTextCharFormat(), QRegExp("[^@pg<>v^?_|+\\-*\\\\/!%`:$.,#&~0-9\\s\"]"));
    _formats["comment"].first.setForeground(QBrush(QColor(150, 150, 150)));

    for (auto itr = _formats.begin(); itr != _formats.end(); ++itr) {
        HighlightingRule rule;
        rule.format = (*itr).second.first;
        rule.pattern = (*itr).second.second;
        _highlightingRules.push_back(rule);
    }
}

void HoneycombSyntaxHighlighter::highlightBlock(QString const &block) {
    /*for (auto itr = _highlightingRules.begin(); itr != _highlightingRules.end(); ++itr) {
        QRegExp expr((*itr).pattern);
        int indx = expr.indexIn(block);
     
        while (indx >= 0) {
            int length = expr.matchedLength();
            setFormat(indx, length, (*itr).format);
            indx = expr.indexIn(block, indx + length);
        }
    }*/
    
    for (auto&& i: _highlightingRules) {
        QRegExp expr(i.pattern);
        int indx = expr.indexIn(block);
        
        while (indx >= 0) {
            int len = expr.matchedLength();
            setFormat(indx, len, i.format);
            indx = expr.indexIn(block, indx + len);
        }
    }
}
















