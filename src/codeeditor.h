#pragma once

#include <QApplication>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QTimer>

#include "assembler.h"
#include "syntaxhighlighter.h"

#include <set>

// Extended version of Qt's CodeEditor example
// http://doc.qt.io/qt-5/qtwidgets-widgets-codeeditor-example.html

namespace Ripes {

class LineNumberArea;
class BreakpointArea;

class CodeEditor : public QPlainTextEdit {
    Q_OBJECT
public:
    CodeEditor(QWidget* parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent* event);
    void breakpointAreaPaintEvent(QPaintEvent* event);
    void breakpointClick(const QPoint& pos);
    bool hasBreakpoint(const QPoint& pos) const;
    long addressForPos(const QPoint& pos) const;
    void clearBreakpoints();
    int lineNumberAreaWidth();
    void setupSyntaxHighlighter();
    void setupChangedTimer();
    void enableBreakpointArea();
    void reset() {
        m_highlighter->reset();
        m_tooltipForLine.clear();
        clear();
    }
    bool syntaxAccepted() const { return m_tooltipForLine.isEmpty(); }

signals:
    void textChanged();

protected:
    void resizeEvent(QResizeEvent* event) override;
    bool event(QEvent* e) override;

private slots:
    void updateSidebarWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateSidebar(const QRect&, int);
    void updateTooltip(int line, QString tip);

private:
    SyntaxHighlighter* m_highlighter;
    LineNumberArea* m_lineNumberArea;
    BreakpointArea* m_breakpointArea;
    int m_sidebarWidth;

    bool m_syntaxChecking = false;
    bool m_breakpointAreaEnabled = false;

    QMap<int, QString> m_tooltipForLine;

    QFont m_font = font();

    // A timer is needed for only catching one of the multiple wheel events that
    // occur on a regular mouse scroll
    QTimer m_fontTimer;
    QTimer m_changeTimer;

    bool eventFilter(QObject* observed, QEvent* event) override;
};

// base class for side area widgets that are attached to the code editor
class SideArea : public QWidget {
    Q_OBJECT
public:
    SideArea(QWidget* parent = nullptr);

private:
};

class LineNumberArea : public QWidget {
public:
    LineNumberArea(CodeEditor* editor) : QWidget(editor) { codeEditor = editor; }

    QSize sizeHint() const override { return QSize(codeEditor->lineNumberAreaWidth(), 0); }

protected:
    void paintEvent(QPaintEvent* event) override { codeEditor->lineNumberAreaPaintEvent(event); }

    void wheelEvent(QWheelEvent* event) override {
        codeEditor->verticalScrollBar()->setValue(codeEditor->verticalScrollBar()->value() +
                                                  (-event->angleDelta().y()) / 30);
    }

private:
    CodeEditor* codeEditor;
};

class BreakpointArea : public QWidget {
public:
    BreakpointArea(CodeEditor* editor);

    QSize sizeHint() const override { return QSize(width(), 0); }
    int width() const { return imageWidth + padding * 2; }
    QSize breakpointSize() { return QSize(imageWidth, imageHeight); }

    int imageWidth = 16;
    int imageHeight = 16;
    int padding = 3;  // padding on each side of the breakpoint
    QPixmap m_breakpoint = QPixmap(":/icons/breakpoint_enabled.png").scaled(imageWidth, imageHeight);
    QPixmap m_breakpoint_disabled = QPixmap(":/icons/breakpoint_disabled.png").scaled(imageWidth, imageWidth);

    void setBlockHeight(int height) { m_blockHeight = height; }

protected:
    void paintEvent(QPaintEvent* event) override { codeEditor->breakpointAreaPaintEvent(event); }

    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    CodeEditor* codeEditor;
    int m_blockHeight = 0;

    void mouseReleaseEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            codeEditor->breakpointClick(event->pos());
        }
    }

    void wheelEvent(QWheelEvent* event) override {
        codeEditor->verticalScrollBar()->setValue(codeEditor->verticalScrollBar()->value() +
                                                  (-event->angleDelta().y()) / 30);
    }
};
}  // namespace Ripes
