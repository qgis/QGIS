// This is the SIP interface definition for QsciAPIs.
//
// Copyright (c) 2014 Riverbank Computing Limited <info@riverbankcomputing.com>
// 
// This file is part of QScintilla.
// 
// This file may be used under the terms of the GNU General Public
// License versions 2.0 or 3.0 as published by the Free Software
// Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
// included in the packaging of this file.  Alternatively you may (at
// your option) use any later version of the GNU General Public
// License if such license has been publicly approved by Riverbank
// Computing Limited (or its successors, if any) and the KDE Free Qt
// Foundation. In addition, as a special exception, Riverbank gives you
// certain additional rights. These rights are described in the Riverbank
// GPL Exception version 1.1, which can be found in the file
// GPL_EXCEPTION.txt in this package.
// 
// If you are unsure which license is appropriate for your use, please
// contact the sales department at sales@riverbankcomputing.com.
// 
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.


class QsciAPIs : QsciAbstractAPIs
{
%TypeHeaderCode
#include <Qsci/qsciapis.h>
%End

public:
%If (Qsci_Qt4)
    QsciAPIs(QsciLexer *lexer /TransferThis/ = 0);
%End
%If (!Qsci_Qt4)
    QsciAPIs(QsciLexer *lexer /TransferThis/ = 0, const char *name = 0);
%End

    virtual ~QsciAPIs();

    void add(const QString &entry);
    void clear();
    bool load(const QString &fname);
    void remove(const QString &entry);
    void prepare();
    void cancelPreparation();
    QString defaultPreparedName() const;
    bool isPrepared(const QString &filename = QString()) const;
    bool loadPrepared(const QString &filename = QString());
    bool savePrepared(const QString &filename = QString()) const;
    virtual bool event(QEvent *e);
    QStringList installedAPIFiles() const;

    virtual void updateAutoCompletionList(const QStringList &context,
                QStringList &list);
    virtual void autoCompletionSelected(const QString &selection);

%If (Qsci_Qt4)
    virtual QStringList callTips(const QStringList &context, int commas,
            QsciScintilla::CallTipsStyle style, QList<int> &shifts);
%End
%If (!Qsci_Qt4)
    virtual QStringList callTips(const QStringList &context, int commas,
            QsciScintilla::CallTipsStyle style, QValueList<int> &shifts);
%End

signals:
    void apiPreparationCancelled();
    void apiPreparationStarted();
    void apiPreparationFinished();

private:
    QsciAPIs(const QsciAPIs &);
};
