/***************************************************************************
    qgscodeeditorcss.h - A CSS editor based on QScintilla
     --------------------------------------
    Date                 : 27-Jul-2014
    Copyright            : (C) 2014 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCODEEDITORCSS_H
#define QGSCODEEDITORCSS_H

#include "qgscodeeditor.h"


/** \ingroup gui
 * A CSS editor based on QScintilla2. Adds syntax highlighting and
 * code autocompletion.
 * \note added in 2.6
 */
class GUI_EXPORT QgsCodeEditorCSS : public QgsCodeEditor
{
    Q_OBJECT

  public:
    QgsCodeEditorCSS( QWidget *parent = nullptr );
    ~QgsCodeEditorCSS();

  private:
    void setSciLexerCSS();
};

#endif
