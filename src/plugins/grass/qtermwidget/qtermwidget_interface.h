/*  Copyright (C) 2022 Francesc Martinez (info@francescmm.com)

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

#pragma once

#include <QFont>

class QKeyEvent;
class QAction;

class QTermWidgetInterface {
   public:
   /**
     * This enum describes the location where the scroll bar is positioned in the display widget.
     */
   enum ScrollBarPosition {
      /** Do not show the scroll bar. */
      NoScrollBar = 0,
      /** Show the scroll bar on the left side of the display. */
      ScrollBarLeft = 1,
      /** Show the scroll bar on the right side of the display. */
      ScrollBarRight = 2
   };

   virtual ~QTermWidgetInterface() = default;

   virtual void setTerminalSizeHint(bool enabled) = 0;
   virtual bool terminalSizeHint() = 0;
   virtual void startShellProgram() = 0;
   virtual void startTerminalTeletype() = 0;
   virtual int getShellPID() = 0;
   virtual void changeDir(const QString & dir) = 0;
   virtual void setTerminalFont(const QFont & font) = 0;
   virtual QFont getTerminalFont() = 0;
   virtual void setTerminalOpacity(qreal level) = 0;
   virtual void setTerminalBackgroundImage(const QString& backgroundImage) = 0;
   virtual void setTerminalBackgroundMode(int mode) = 0;
   virtual void setEnvironment(const QStringList & environment) = 0;
   virtual void setShellProgram(const QString & program) = 0;
   virtual void setWorkingDirectory(const QString & dir) = 0;
   virtual QString workingDirectory() = 0;
   virtual void setArgs(const QStringList & args) = 0;
   virtual void setTextCodec(QTextCodec * codec) = 0;
   virtual void setColorScheme(const QString & name) = 0;
   virtual QStringList getAvailableColorSchemes() = 0;
   virtual void setHistorySize(int lines) = 0;
   virtual int historySize() const = 0;
   virtual void setScrollBarPosition(ScrollBarPosition) = 0;
   virtual void scrollToEnd() = 0;
   virtual void sendText(const QString & text) = 0;
   virtual void sendKeyEvent(QKeyEvent* e) = 0;
   virtual void setFlowControlEnabled(bool enabled) = 0;
   virtual bool flowControlEnabled(void) = 0;
   virtual void setFlowControlWarningEnabled(bool enabled) = 0;
   virtual QString keyBindings() = 0;
   virtual void setMotionAfterPasting(int) = 0;
   virtual int historyLinesCount() = 0;
   virtual int screenColumnsCount() = 0;
   virtual int screenLinesCount() = 0;
   virtual void setSelectionStart(int row, int column) = 0;
   virtual void setSelectionEnd(int row, int column) = 0;
   virtual void getSelectionStart(int& row, int& column) = 0;
   virtual void getSelectionEnd(int& row, int& column) = 0;
   virtual QString selectedText(bool preserveLineBreaks = true) = 0;
   virtual void setMonitorActivity(bool) = 0;
   virtual void setMonitorSilence(bool) = 0;
   virtual void setSilenceTimeout(int seconds) = 0;
   virtual QList<QAction*> filterActions(const QPoint& position) = 0;
   virtual int getPtySlaveFd() const = 0;
   virtual void setBlinkingCursor(bool blink) = 0;
   virtual void setBidiEnabled(bool enabled) = 0;
   virtual bool isBidiEnabled() = 0;
   virtual void setAutoClose(bool) = 0;
   virtual QString title() const = 0;
   virtual QString icon() const = 0;
   virtual bool isTitleChanged() const = 0;
   virtual void bracketText(QString& text) = 0;
   virtual void disableBracketedPasteMode(bool disable) = 0;
   virtual bool bracketedPasteModeIsDisabled() const = 0;
   virtual void setMargin(int) = 0;
   virtual int getMargin() const = 0;
   virtual void setDrawLineChars(bool drawLineChars) = 0;
   virtual void setBoldIntense(bool boldIntense) = 0;
   virtual void setConfirmMultilinePaste(bool confirmMultilinePaste) = 0;
   virtual void setTrimPastedTrailingNewlines(bool trimPastedTrailingNewlines) = 0;
   virtual QTermWidgetInterface* createWidget(int startnow) const = 0;
};

#define QTermWidgetInterface_iid "lxqt.qtermwidget.QTermWidgetInterface/1.0"

Q_DECLARE_INTERFACE(QTermWidgetInterface, QTermWidgetInterface_iid)
