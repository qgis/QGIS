/*  Copyright (C) 2008 e_k (e_k@users.sourceforge.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


#ifndef _Q_TERM_WIDGET
#define _Q_TERM_WIDGET

#include <QWidget>
#include "Filter.h"

class QVBoxLayout;
struct TermWidgetImpl;
class SearchBar;
class QUrl;

class QTermWidget : public QWidget {
    Q_OBJECT
public:

    enum ScrollBarPosition {
        /** Do not show the scroll bar. */
        NoScrollBar=0,
        /** Show the scroll bar on the left side of the display. */
        ScrollBarLeft=1,
        /** Show the scroll bar on the right side of the display. */
        ScrollBarRight=2
    };

    //Creation of widget
    QTermWidget(int startnow, // 1 = start shell programm immediatelly
                QWidget * parent = 0);
    // A dummy constructor for Qt Designer. startnow is 1 by default
    QTermWidget(QWidget *parent = 0);

    virtual ~QTermWidget();

    //Initial size
    QSize sizeHint() const;

    //start shell program if it was not started in constructor
    void startShellProgram();

    /**
     * Start terminal teletype as is
     * and redirect data for external recipient.
     * It can be used for display and control a remote terminal.
     */
    void startTerminalTeletype();

    int getShellPID();

    void changeDir(const QString & dir);

    //look-n-feel, if you don`t like defaults

    //  Terminal font
    // Default is application font with family Monospace, size 10
    // Beware of a performance penalty and display/alignment issues when using a proportional font.
    void setTerminalFont(const QFont & font);
    QFont getTerminalFont();
    void setTerminalOpacity(qreal level);

    //environment
    void setEnvironment(const QStringList & environment);

    //  Shell program, default is /bin/bash
    void setShellProgram(const QString & progname);

    //working directory
    void setWorkingDirectory(const QString & dir);
    QString workingDirectory();

    // Shell program args, default is none
    void setArgs(const QStringList & args);

    //Text codec, default is UTF-8
    void setTextCodec(QTextCodec * codec);

    /** @brief Sets the color scheme, default is white on black
     *
     * @param[in] name The name of the color scheme, either returned from
     * availableColorSchemes() or a full path to a color scheme.
     */
    void setColorScheme(const QString & name);
    static QStringList availableColorSchemes();

    //set size
    void setSize(int h, int v);

    // History size for scrolling
    void setHistorySize(int lines); //infinite if lines < 0

    // Presence of scrollbar
    void setScrollBarPosition(ScrollBarPosition);

    // Wrapped, scroll to end.
    void scrollToEnd();

    // Send some text to terminal
    void sendText(const QString & text);

    // Sets whether flow control is enabled
    void setFlowControlEnabled(bool enabled);

    // Returns whether flow control is enabled
    bool flowControlEnabled(void);

    /**
     * Sets whether the flow control warning box should be shown
     * when the flow control stop key (Ctrl+S) is pressed.
     */
    void setFlowControlWarningEnabled(bool enabled);

    /*! Get all available keyboard bindings
     */
    static QStringList availableKeyBindings();

    //! Return current key bindings
    QString keyBindings();

    void setMotionAfterPasting(int);

    /** Return the number of lines in the history buffer. */
    int historyLinesCount();

    int screenColumnsCount();
    int screenLinesCount();

    void setSelectionStart(int row, int column);
    void setSelectionEnd(int row, int column);
    void getSelectionStart(int& row, int& column);
    void getSelectionEnd(int& row, int& column);

    /**
     * Returns the currently selected text.
     * @param preserveLineBreaks Specifies whether new line characters should
     * be inserted into the returned text at the end of each terminal line.
     */
    QString selectedText(bool preserveLineBreaks = true);

    void setMonitorActivity(bool);
    void setMonitorSilence(bool);
    void setSilenceTimeout(int seconds);

    /** Returns the available hotspot for the given point \em pos.
     *
     * This method may return a nullptr if no hotspot is available.
     *
     * @param[in] pos The point of interest in the QTermWidget coordinates.
     * @return Hotspot for the given position, or nullptr if no hotspot.
     */
    Filter::HotSpot* getHotSpotAt(const QPoint& pos) const;

    /** Returns the available hotspots for the given row and column.
     *
     * @return Hotspot for the given position, or nullptr if no hotspot.
     */
    Filter::HotSpot* getHotSpotAt(int row, int column) const;

    /**
     * Returns a pty slave file descriptor.
     * This can be used for display and control
     * a remote terminal.
     */
    int getPtySlaveFd() const;

signals:
    void finished();
    void copyAvailable(bool);

    void termGetFocus();
    void termLostFocus();

    void termKeyPressed(QKeyEvent *);

    void urlActivated(const QUrl&);

    void bell(const QString& message);

    void activity();
    void silence();

    /**
     * Emitted when emulator send data to the terminal process
     * (redirected for external recipient). It can be used for
     * control and display the remote terminal.
     */
    void sendData(const char *,int);

public slots:
    // Copy selection to clipboard
    void copyClipboard();

    // Paste clipboard to terminal
    void pasteClipboard();

    // Paste selection to terminal
    void pasteSelection();

    // Set zoom
    void zoomIn();
    void zoomOut();

    /*! Set named key binding for given widget
     */
    void setKeyBindings(const QString & kb);

    /*! Clear the terminal content and move to home position
     */
    void clear();

    void toggleShowSearchBar();

protected:
    virtual void resizeEvent(QResizeEvent *);

protected slots:
    void sessionFinished();
    void selectionChanged(bool textSelected);

private slots:
    void find();
    void findNext();
    void findPrevious();
    void matchFound(int startColumn, int startLine, int endColumn, int endLine);
    void noMatchFound();

private:
    void search(bool forwards, bool next);
    void setZoom(int step);
    void init(int startnow);
    TermWidgetImpl * m_impl;
    SearchBar* m_searchBar;
    QVBoxLayout *m_layout;
};


//Maybe useful, maybe not

#ifdef __cplusplus
extern "C"
#endif
void * createTermWidget(int startnow, void * parent);

#endif

