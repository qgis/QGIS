/***************************************************************************
    qgscodeeditorhtml.cpp - A HTML editor based on QScintilla
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

#include "qgscodeeditorhtml.h"
#include "moc_qgscodeeditorhtml.cpp"
#include "qgspythonrunner.h"
#include "qgsprocessingutils.h"

#include <QWidget>
#include <QString>
#include <QFont>
#include <Qsci/qscilexerhtml.h>


QgsCodeEditorHTML::QgsCodeEditorHTML( QWidget *parent )
  : QgsCodeEditor( parent, QString(), false, false, QgsCodeEditor::Flag::CodeFolding )
{
  if ( !parent )
  {
    setTitle( tr( "HTML Editor" ) );
  }
  QgsCodeEditorHTML::initializeLexer();

  mCapabilities = Qgis::ScriptLanguageCapability::ToggleComment;
  if ( QgsPythonRunner::isValid() )
  {
    // we could potentially check for beautifulsoup4 import here and reflect the capability accordingly.
    // (current approach is to to always indicate this capability and raise a user-friendly warning
    // when attempting to reformat if the libraries can't be imported)
    mCapabilities |= Qgis::ScriptLanguageCapability::Reformat;
  }
}

Qgis::ScriptLanguage QgsCodeEditorHTML::language() const
{
  return Qgis::ScriptLanguage::Html;
}

Qgis::ScriptLanguageCapabilities QgsCodeEditorHTML::languageCapabilities() const
{
  return mCapabilities;
}

void QgsCodeEditorHTML::initializeLexer()
{
  QFont font = lexerFont();
  const QColor defaultColor = lexerColor( QgsCodeEditorColorScheme::ColorRole::Default );

  QsciLexerHTML *lexer = new QsciLexerHTML( this );
  lexer->setDefaultFont( font );
  lexer->setDefaultColor( defaultColor );
  lexer->setDefaultPaper( lexerColor( QgsCodeEditorColorScheme::ColorRole::Background ) );
  lexer->setFont( font, -1 );

  font.setItalic( true );
  lexer->setFont( font, QsciLexerHTML::HTMLComment );
  lexer->setFont( font, QsciLexerHTML::JavaScriptComment );
  lexer->setFont( font, QsciLexerHTML::JavaScriptCommentLine );

  lexer->setColor( defaultColor, QsciLexerHTML::Default );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Tag ), QsciLexerHTML::Tag );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::UnknownTag ), QsciLexerHTML::UnknownTag );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Method ), QsciLexerHTML::Attribute );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Method ), QsciLexerHTML::UnknownAttribute );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Class ), QsciLexerHTML::Entity );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Number ), QsciLexerHTML::HTMLNumber );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Comment ), QsciLexerHTML::HTMLComment );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Comment ), QsciLexerHTML::JavaScriptComment );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::CommentLine ), QsciLexerHTML::JavaScriptCommentLine );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Number ), QsciLexerHTML::JavaScriptNumber );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Keyword ), QsciLexerHTML::JavaScriptKeyword );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::DoubleQuote ), QsciLexerHTML::JavaScriptDoubleQuotedString );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::SingleQuote ), QsciLexerHTML::JavaScriptSingleQuotedString );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::SingleQuote ), QsciLexerHTML::HTMLSingleQuotedString );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::DoubleQuote ), QsciLexerHTML::HTMLDoubleQuotedString );

  setLexer( lexer );
  runPostLexerConfigurationTasks();
}

QString QgsCodeEditorHTML::reformatCodeString( const QString &string )
{
  if ( !QgsPythonRunner::isValid() )
  {
    return string;
  }
  QString newText = string;

  const QString definePrettify = QStringLiteral(
    "def __qgis_prettify(text):\n"
    "  try:\n"
    "    from bs4 import BeautifulSoup\n"
    "    return BeautifulSoup(text, 'html.parser').prettify()\n"
    "  except ImportError:\n"
    "    try:\n"
    "      import re\n"
    "      from lxml import etree, html\n"
    "      text = re.sub('>\\\\s+<', '><', text)\n"
    "      text = re.sub('\\n\\\\s*', '', text)\n"
    "      document_root = html.fromstring(text)\n"
    "      return etree.tostring(document_root, encoding='utf-8', pretty_print=True).decode('utf-8')\n"
    "    except ImportError:\n"
    "      return '_ImportError'\n"
  );


  if ( !QgsPythonRunner::run( definePrettify ) )
  {
    QgsDebugError( QStringLiteral( "Error running script: %1" ).arg( definePrettify ) );
    return string;
  }

  const QString script = QStringLiteral( "__qgis_prettify(%1)" ).arg( QgsProcessingUtils::stringToPythonLiteral( newText ) );
  QString result;
  if ( QgsPythonRunner::eval( script, result ) )
  {
    if ( result == QLatin1String( "_ImportError" ) )
    {
      showMessage( tr( "Reformat Code" ), tr( "HTML reformatting requires bs4 or lxml python modules to be installed" ), Qgis::MessageLevel::Warning );
    }
    else
    {
      newText = result;
    }
  }
  else
  {
    QgsDebugError( QStringLiteral( "Error running script: %1" ).arg( script ) );
    return newText;
  }

  return newText;
}

void QgsCodeEditorHTML::toggleComment()
{
  if ( isReadOnly() )
  {
    return;
  }

  const QString commentStart( "<!--" );
  const QString commentEnd( "-->" );

  int startLine, startPos, endLine, endPos;
  if ( hasSelectedText() )
  {
    getSelection( &startLine, &startPos, &endLine, &endPos );
  }
  else
  {
    getCursorPosition( &startLine, &startPos );
    endLine = startLine;
    endPos = startPos;
  }


  // Compute first and last non-blank lines
  while ( text( startLine ).trimmed().isEmpty() )
  {
    startLine++;
    if ( startLine > endLine )
    {
      // Only blank lines selected
      return;
    }
  }
  while ( text( endLine ).trimmed().isEmpty() )
  {
    endLine--;
  }

  // Remove leading spaces from the start line
  QString startLineTrimmed = text( startLine );
  startLineTrimmed.remove( QRegularExpression( "^\\s+" ) );
  // Remove trailing spaces from the end line
  QString endLineTrimmed = text( endLine );
  endLineTrimmed.remove( QRegularExpression( "\\s+$" ) );

  const bool commented = startLineTrimmed.startsWith( commentStart ) && endLineTrimmed.endsWith( commentEnd );

  // Special case, selected text is <!--> or <!--->
  if ( commented && startLine == endLine && text( endLine ).trimmed().size() < commentStart.size() + commentEnd.size() )
  {
    return;
  }

  beginUndoAction();

  // Selection is commented: uncomment it
  if ( commented )
  {
    int c1, c2;

    // Remove trailing comment tag ( --> )
    c2 = endLineTrimmed.size();
    if ( endLineTrimmed.endsWith( QStringLiteral( " " ) + commentEnd ) )
    {
      c1 = c2 - commentEnd.size() - 1;
    }
    else
    {
      c1 = c2 - commentEnd.size();
    }

    setSelection( endLine, c1, endLine, c2 );
    removeSelectedText();

    // Remove leading comment tag ( <!-- )
    c1 = indentation( startLine );
    if ( startLineTrimmed.startsWith( commentStart + QStringLiteral( " " ) ) )
    {
      c2 = c1 + commentStart.size() + 1;
    }
    else
    {
      c2 = c1 + commentStart.size();
    }

    setSelection( startLine, c1, startLine, c2 );
    removeSelectedText();
  }
  // Selection is not commented: comment it
  else
  {
    insertAt( QStringLiteral( " " ) + commentEnd, endLine, endLineTrimmed.size() );
    insertAt( commentStart + QStringLiteral( " " ), startLine, indentation( startLine ) );
  }

  endUndoAction();

  // Restore selection
  setSelection( startLine, startPos, endLine, endPos );
}
