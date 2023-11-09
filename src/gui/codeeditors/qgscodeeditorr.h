/***************************************************************************
    qgscodeeditorr.h - A R stats editor based on QScintilla
     --------------------------------------
    Date                 : October 2022
    Copyright            : (C) 2022 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCODEEDITORR_H
#define QGSCODEEDITORR_H

#include "qgscodeeditor.h"
#include "qgis_sip.h"
#include "qgis_gui.h"
#include <Qsci/qscilexer.h>

SIP_IF_MODULE( HAVE_QSCI_SIP )

#ifndef SIP_RUN

///@cond PRIVATE
class GUI_EXPORT QgsQsciLexerR : public QsciLexer
{
    Q_OBJECT
  public:

    enum Styles
    {
      Default = 0,
      Comment = 1,
      Kword = 2,
      BaseKword = 3,
      OtherKword = 4,
      Number = 5,
      String = 6,
      String2 = 7,
      Operator = 8,
      Identifier = 9,
      Infix = 10,
      InfixEOL = 11,
      Backticks = 12,
      RawString = 13,
      RawString2 = 14,
      EscapeSequence = 15
    };

    QgsQsciLexerR( QObject *parent = nullptr );
    const char *language() const override;
    const char *lexer() const override;
    int lexerId() const override;
    QString description( int style ) const override;
    const char *keywords( int set ) const override;


};
///@endcond
#endif

/**
 * \ingroup gui
 * \brief A R stats code editor based on QScintilla2. Adds syntax highlighting and
 * code autocompletion.
 * \since QGIS 3.30
 */
class GUI_EXPORT QgsCodeEditorR : public QgsCodeEditor
{
    Q_OBJECT

  public:

    //! Constructor for QgsCodeEditorR
    QgsCodeEditorR( QWidget *parent SIP_TRANSFERTHIS = nullptr, QgsCodeEditor::Mode mode = QgsCodeEditor::Mode::ScriptEditor );
    Qgis::ScriptLanguage language() const override;

  protected:
    void initializeLexer() override;

};

#endif // QGSCODEEDITORR_H
