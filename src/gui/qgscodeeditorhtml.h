/***************************************************************************
    qgscodeeditorhtml.h - A HTML editor based on QScintilla
     --------------------------------------
    Date                 : 20-Jul-2014
    Copyright            : (C) 2014 by Nathan Woodrow
    Email                : woodrow.nathan (at) gmail (dot) com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCODEEDITORHTML_H
#define QGSCODEEDITORHTML_H

#include "qgscodeeditor.h"


/** \ingroup gui
 * A HTML editor based on QScintilla2. Adds syntax highlighting and
 * code autocompletion.
 * \note added in 2.6
 */
class GUI_EXPORT QgsCodeEditorHTML : public QgsCodeEditor
{
    Q_OBJECT

  public:
    QgsCodeEditorHTML( QWidget *parent = 0 );
    ~QgsCodeEditorHTML();

  private:
    void setSciLexerHTML();
};

#endif
