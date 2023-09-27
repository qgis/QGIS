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

class QgsCodeEditor;
class QTextEdit;
class QPlainTextEdit;

/**
 * \ingroup gui
 * \class QgsExpressionFinder
 */
class GUI_EXPORT QgsExpressionFinder SIP_SKIP
{
  public:
    static void findExpressionAtPos( const QString &text, int startSelectionPos, int endSelectionPos, int &start, int &end, QString &expression );

    static QString findAndSelectExpression( QgsCodeEditor *editor );

    static QString findAndSelectExpression( QTextEdit *editor );

    static QString findAndSelectExpression( QPlainTextEdit *editor );


};

#endif // QGSEXPRESSIONFINDER_H
