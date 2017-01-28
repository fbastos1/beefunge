#ifndef HONEYCOMBSYNTAXHIGHLIGHTER_H
#define HONEYCOMBSYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <map>
#include <string>
#include <utility>

class HoneycombSyntaxHighlighter: public QSyntaxHighlighter {
public:
    HoneycombSyntaxHighlighter(QTextDocument *parent = 0);

protected:
    void highlightBlock(const QString&);

private:
    struct HighlightingRule {
        QRegExp pattern;
        QTextCharFormat format;
    };

    QVector<HighlightingRule> _highlightingRules;
    std::map<std::string, std::pair<QTextCharFormat, QRegExp>> _formats;
};

#endif // HONEYCOMBSYNTAXHIGHLIGHTER_H
