// This is the SIP interface definition for QsciStyle.
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


class QsciStyle
{
%TypeHeaderCode
#include <Qsci/qscistyle.h>
%End

public:
    enum TextCase {
        OriginalCase,
        UpperCase,
        LowerCase
    };

    QsciStyle(int style = -1);
    QsciStyle(int style, const QString &description, const QColor &color,
            const QColor &paper, const QFont &font, bool eolFill = false);

    int style() const;

    void setDescription(const QString &description);
    QString description() const;

    void setColor(const QColor &color);
    QColor color() const;

    void setPaper(const QColor &paper);
    QColor paper() const;

    void setFont(const QFont &font);
    QFont font() const;

    void setEolFill(bool fill);
    bool eolFill() const;

    void setTextCase(TextCase text_case);
    TextCase textCase() const;

    void setVisible(bool visible);
    bool visible() const;

    void setChangeable(bool changeable);
    bool changeable() const;

    void setHotspot(bool hotspot);
    bool hotspot() const;

    void refresh();
};
