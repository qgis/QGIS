/*********************************************************************************
    qgsexpressionfinder.h - A helper class to locate expression in text editors
     --------------------------------------
    begin                : September 2023
    copyright            : (C) 2023 by Yoann Quenach de Quivillic
    email                : yoann dot quenach at gmail dot com
 *********************************************************************************
 *                                                                               *
 *   This program is free software; you can redistribute it and/or modify        *
 *   it under the terms of the GNU General Public License as published by        *
 *   the Free Software Foundation; either version 2 of the License, or           *
 *   (at your option) any later version.                                         *
 *                                                                               *
 *********************************************************************************/

#ifndef QGSEXPRESSIONFINDER_H
#define QGSEXPRESSIONFINDER_H

#include <QString>

#include "qgis_sip.h"
#include "qgis_gui.h"

#define SIP_NO_FILE

class QgsCodeEditor;
class QTextEdit;
class QPlainTextEdit;

/**
 * \ingroup gui
 * \brief Helper methods to locate expressions within text editors
 * \note not available in Python bindings
 * \since QGIS 3.36
 */
class GUI_EXPORT QgsExpressionFinder
{
  public:
    /**
     * Find an expression at the given position in the given text
     *
     * If an expression is found, start and end are set to the position of the
     * opening and closing brakets of the expression, and expression is set to
     * the trimmed expression (excluding the surrounding [% %] characters)
     *
     * Otherwise, start and end are set to startSelectionPos and endSelectionPos
     * and expression is set to the selected text
     *
     * Optionally, a custom regex pattern can be used to find the expression.
     * This pattern must contain a capture group to extract the expression from
     * the match.
     */
    static void findExpressionAtPos( const QString &text, int startSelectionPos, int endSelectionPos, int &start, int &end, QString &expression, const QString &pattern = QString() );

    /**
     * Find the expression under the cursor in the given editor and select it
     *
     * If an expression is found, it is returned (excluding the surrounding [% %] characters)
     * Otherwise, the selection is kept unchanged and the selected text is returned
     */
    static QString findAndSelectActiveExpression( QgsCodeEditor *editor, const QString &pattern = QString() );

    /**
     * Find the expression under the cursor in the given editor and select it
     *
     * If an expression is found, it is returned (excluding the surrounding [% %] characters)
     * Otherwise, the selection is kept unchanged and the selected text is returned
     */
    static QString findAndSelectActiveExpression( QTextEdit *editor, const QString &pattern = QString() );

    /**
     * Find the expression under the cursor in the given editor and select it
     *
     * If an expression is found, it is returned (excluding the surrounding [% %] characters)
     * Otherwise, the selection is kept unchanged and the selected text is returned
     */
    static QString findAndSelectActiveExpression( QPlainTextEdit *editor, const QString &pattern = QString() );
};

#endif // QGSEXPRESSIONFINDER_H
