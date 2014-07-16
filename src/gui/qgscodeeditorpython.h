/***************************************************************************
    qgscodeeditorpython.h - A Python editor based on QScintilla
     --------------------------------------
    Date                 : 06-Oct-2013
    Copyright            : (C) 2013 by Salvatore Larosa
    Email                : lrssvtml (at) gmail (dot) com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCODEEDITORPYTHON_H
#define QGSCODEEDITORPYTHON_H

#include "qgscodeeditor.h"


/** \ingroup gui
 * A Python editor based on QScintilla2. Adds syntax highlighting and
 * code autocompletion.
 * \note added in 2.6
 */
class GUI_EXPORT QgsCodeEditorPython : public QgsCodeEditor
{
    Q_OBJECT

  public:
    /**
     * Construct a new Python editor.
     *
     * @param parent The parent QWidget
     * @param filenames The list of apis files to load for the python lexer
     * @note added in 2.6
     */
    QgsCodeEditorPython( QWidget *parent = 0 , const QList<QString> &filenames = QList<QString>() );
    ~QgsCodeEditorPython();

    /** Load APIs from one or more files
     * @param filenames The list of apis files to load for the python lexer
     */
    void loadAPIs( QList<QString> const &filenames );

    /** Load a script file
     * @param script The script file to load
     */
    bool loadScript( const QString &script );

  private:
    //QgsCodeEditor *mSciWidget;
    //QWidget *mWidget;
    void setSciLexerPython();

    QList<QString> mAPISFilesList;
    QString mPapFile;

};

#endif
