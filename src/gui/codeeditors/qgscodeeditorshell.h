/***************************************************************************
    qgscodeeditorshell.h
     -------------------
    Date                 : April 2023
    Copyright            : (C) 2023 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCODEEDITORSHELL_H
#define QGSCODEEDITORSHELL_H

#include "qgscodeeditor.h"
#include "qgis_sip.h"
#include "qgis_gui.h"
#include <Qsci/qscilexer.h>

SIP_IF_MODULE( HAVE_QSCI_SIP )

#ifndef SIP_RUN

///@cond PRIVATE
class GUI_EXPORT QgsQsciLexerBash : public QsciLexer
{
    Q_OBJECT
  public:
    enum Styles
    {
      Default = 0, // whitespace
      Error = 1,
      LineComment = 2,
      Number = 3, // numeric literal
      Keyword = 4,
      String = 5,             // string literal
      SingleQuotedString = 6, // string literal
      Operator = 7,
      Identifier = 8,
      ScalarVariable = 9,         // identifier
      Parameter = 10,             // identifier
      BacktickQuotedCommand = 11, // string literal
      HeredocDelimiter = 12,      // operator
      HeredocQuotedString = 13    // string literal
    };

    QgsQsciLexerBash( QObject *parent = nullptr );
    const char *language() const override;
    const char *lexer() const override;
    int lexerId() const override;
    QString description( int style ) const override;
    const char *keywords( int set ) const override;
};

class GUI_EXPORT QgsQsciLexerBatch : public QsciLexer
{
    Q_OBJECT
  public:
    enum Styles
    {
      Default = 0,    // whitespace
      Comment = 1,    // "Fake label"
      Word = 2,       // keyword
      Label = 3,      // "real label"
      Hide = 4,       // hide command -- @echo off/on
      Command = 5,    // external command/program
      Identifier = 6, // argument / variable
      Operator = 7,
    };

    QgsQsciLexerBatch( QObject *parent = nullptr );
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
 * \brief A shell script code editor based on QScintilla2. Adds syntax highlighting and
 * code autocompletion.
 *
 * QgsCodeEditorShell supports either Bash (Linux) or Batch (Windows) code interpreters.
 * By default the Batch interpreter will be used on Windows platforms and the Bash interpreter
 * on all other platforms.
 *
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsCodeEditorShell : public QgsCodeEditor
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsCodeEditorShell.
     *
     * The \a language argument may be Qgis::ScriptLanguage::Unknown, Qgis::ScriptLanguage::Bash or Qgis::ScriptLanguage::Batch.
     * If the \a language is Qgis::ScriptLanguage::Unknown, then the Qgis::ScriptLanguage::Batch interpreter will be used on Windows platforms and the Qgis::ScriptLanguage::Bash interpreter
     * on all other platforms.
     */
    QgsCodeEditorShell( QWidget *parent SIP_TRANSFERTHIS = nullptr, QgsCodeEditor::Mode mode = QgsCodeEditor::Mode::ScriptEditor, Qgis::ScriptLanguage language = Qgis::ScriptLanguage::Unknown );
    Qgis::ScriptLanguage language() const override;

  protected:
    void initializeLexer() override;

  private:
    Qgis::ScriptLanguage mLanguage = Qgis::ScriptLanguage::Unknown;
};

#endif // QGSCODEEDITORSHELL_H
