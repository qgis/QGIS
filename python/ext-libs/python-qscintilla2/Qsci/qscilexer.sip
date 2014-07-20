// This is the SIP interface definition for QsciLexer.
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


class QsciLexer : QObject
{
%TypeHeaderCode
#include <Qsci/qscilexer.h>
%End

public:
%If (Qsci_Qt4)
    QsciLexer(QObject *parent /TransferThis/ = 0);
%End
%If (!Qsci_Qt4)
    QsciLexer(QObject *parent /TransferThis/ = 0, const char *name = 0);
%End

    virtual ~QsciLexer();

    virtual const char *language() const = 0;
    virtual const char *lexer() const;
    virtual int lexerId() const;
    QsciAbstractAPIs *apis() const;
    int autoIndentStyle();
    virtual QColor color(int style) const;
    virtual bool eolFill(int style) const;
    virtual QFont font(int style) const;
    virtual const char *keywords(int set) const;
    virtual QString description(int style) const = 0;
    virtual QColor paper(int style) const;
    QColor defaultColor() const;
    virtual QColor defaultColor(int style) const;
    virtual bool defaultEolFill(int style) const;
    QFont defaultFont() const;
    virtual QFont defaultFont(int style) const;
    QColor defaultPaper() const;
    virtual QColor defaultPaper(int style) const;
    QsciScintilla *editor() const;
    virtual void refreshProperties();
    void setAPIs(QsciAbstractAPIs *apis);
    void setDefaultColor(const QColor &c);
    void setDefaultFont(const QFont &f);
    void setDefaultPaper(const QColor &c);
    virtual int styleBitsNeeded() const;
    virtual const char *wordCharacters() const;
    bool readSettings(QSettings &qs, const char *prefix = "/Scintilla");
    bool writeSettings(QSettings &qs, const char *prefix = "/Scintilla") const;

public slots:
    virtual void setAutoIndentStyle(int autoindentstyle);
    virtual void setColor(const QColor &c, int style = -1);
    virtual void setEolFill(bool eolfill, int style = -1);
    virtual void setFont(const QFont &f, int style = -1);
    virtual void setPaper(const QColor &c, int style = -1);

signals:
    void colorChanged(const QColor &c, int style);
    void eolFillChanged(bool eolfilled, int style);
    void fontChanged(const QFont &f, int style);
    void paperChanged(const QColor &c, int style);
    void propertyChanged(const char *prop, const char *val);

protected:
    virtual bool readProperties(QSettings &qs, const QString &prefix);
    virtual bool writeProperties(QSettings &qs, const QString &prefix) const;

private:
    QsciLexer(const QsciLexer &);
};
