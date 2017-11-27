/*
    Copyright 2013 Christian Surlykke

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA.
*/
#ifndef TASK_H
#define	TASK_H

#include <QObject>
#include <QPointer>
#include <QMap>

#include <Session.h>
#include <ScreenWindow.h>

#include "Emulation.h"
#include "TerminalCharacterDecoder.h"

using namespace Konsole;

typedef QPointer<Emulation> EmulationPtr;

class HistorySearch : public QObject
{
    Q_OBJECT

public:
    explicit HistorySearch(EmulationPtr emulation, QRegExp regExp, bool forwards,
                           int startColumn, int startLine, QObject* parent);

    ~HistorySearch();

    void search();

signals:
    void matchFound(int startColumn, int startLine, int endColumn, int endLine);
    void noMatchFound();

private:
    bool search(int startColumn, int startLine, int endColumn, int endLine);
    int findLineNumberInString(QList<int> linePositions, int position);


    EmulationPtr m_emulation;
    QRegExp m_regExp;
    bool m_forwards;
    int m_startColumn;
    int m_startLine;

    int m_foundStartColumn;
    int m_foundStartLine;
    int m_foundEndColumn;
    int m_foundEndLine;
};

#endif	/* TASK_H */

