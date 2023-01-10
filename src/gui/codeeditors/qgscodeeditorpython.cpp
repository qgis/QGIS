/***************************************************************************
    qgscodeeditorpython.cpp  - A Python editor based on QScintilla
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

#include "qgsapplication.h"
#include "qgscodeeditorpython.h"
#include "qgslogger.h"
#include "qgssymbollayerutils.h"
#include "qgssettings.h"
#include "qgis.h"

#include <QWidget>
#include <QString>
#include <QFont>
#include <QUrl>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextStream>
#include <Qsci/qscilexerpython.h>
#include <QDesktopServices>
#include <QKeyEvent>

const QMap<QString, QString> QgsCodeEditorPython::sCompletionPairs
{
  {"(", ")"},
  {"[", "]"},
  {"{", "}"},
  {"'", "'"},
  {"\"", "\""}
};
const QStringList QgsCodeEditorPython::sCompletionSingleCharacters{"`", "*"};

QgsCodeEditorPython::QgsCodeEditorPython( QWidget *parent, const QList<QString> &filenames, Mode mode )
  : QgsCodeEditor( parent,
                   QString(),
                   false,
                   false,
                   QgsCodeEditor::Flag::CodeFolding, mode )
  , mAPISFilesList( filenames )
{
  if ( !parent )
  {
    setTitle( tr( "Python Editor" ) );
  }

  setCaretWidth( 2 );

  QgsCodeEditorPython::initializeLexer();
}

Qgis::ScriptLanguage QgsCodeEditorPython::language() const
{
  return Qgis::ScriptLanguage::Python;
}

void QgsCodeEditorPython::initializeLexer()
{
  // current line
  setEdgeMode( QsciScintilla::EdgeLine );
  setEdgeColumn( 80 );
  setEdgeColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Edge ) );

  setWhitespaceVisibility( QsciScintilla::WsVisibleAfterIndent );

  QFont font = lexerFont();
  const QColor defaultColor = lexerColor( QgsCodeEditorColorScheme::ColorRole::Default );

  QsciLexerPython *pyLexer = new QgsQsciLexerPython( this );

  pyLexer->setIndentationWarning( QsciLexerPython::Inconsistent );
  pyLexer->setFoldComments( true );
  pyLexer->setFoldQuotes( true );

  pyLexer->setDefaultFont( font );
  pyLexer->setDefaultColor( defaultColor );
  pyLexer->setDefaultPaper( lexerColor( QgsCodeEditorColorScheme::ColorRole::Background ) );
  pyLexer->setFont( font, -1 );

  font.setItalic( true );
  pyLexer->setFont( font, QsciLexerPython::Comment );
  pyLexer->setFont( font, QsciLexerPython::CommentBlock );

  font.setItalic( false );
  font.setBold( true );
  pyLexer->setFont( font, QsciLexerPython::SingleQuotedString );
  pyLexer->setFont( font, QsciLexerPython::DoubleQuotedString );

  pyLexer->setColor( defaultColor, QsciLexerPython::Default );
  pyLexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Class ), QsciLexerPython::ClassName );
  pyLexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Method ), QsciLexerPython::FunctionMethodName );
  pyLexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Number ), QsciLexerPython::Number );
  pyLexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Operator ), QsciLexerPython::Operator );
  pyLexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Identifier ), QsciLexerPython::Identifier );
  pyLexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Comment ), QsciLexerPython::Comment );
  pyLexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::CommentBlock ), QsciLexerPython::CommentBlock );
  pyLexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Keyword ), QsciLexerPython::Keyword );
  pyLexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Decoration ), QsciLexerPython::Decorator );
  pyLexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::SingleQuote ), QsciLexerPython::SingleQuotedString );
  pyLexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::DoubleQuote ), QsciLexerPython::DoubleQuotedString );
  pyLexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::TripleSingleQuote ), QsciLexerPython::TripleSingleQuotedString );
  pyLexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::TripleDoubleQuote ), QsciLexerPython::TripleDoubleQuotedString );

  std::unique_ptr< QsciAPIs > apis = std::make_unique< QsciAPIs >( pyLexer );

  const QgsSettings settings;

  if ( mAPISFilesList.isEmpty() )
  {
    if ( settings.value( QStringLiteral( "pythonConsole/preloadAPI" ), true ).toBool() )
    {
      mPapFile = QgsApplication::pkgDataPath() + QStringLiteral( "/python/qsci_apis/pyqgis.pap" );
      apis->loadPrepared( mPapFile );
    }
    else if ( settings.value( QStringLiteral( "pythonConsole/usePreparedAPIFile" ), false ).toBool() )
    {
      apis->loadPrepared( settings.value( QStringLiteral( "pythonConsole/preparedAPIFile" ) ).toString() );
    }
    else
    {
      const QStringList apiPaths = settings.value( QStringLiteral( "pythonConsole/userAPI" ) ).toStringList();
      for ( const QString &path : apiPaths )
      {
        if ( !QFileInfo::exists( path ) )
        {
          QgsDebugMsg( QStringLiteral( "The apis file %1 was not found" ).arg( path ) );
        }
        else
        {
          apis->load( path );
        }
      }
      apis->prepare();
    }
  }
  else if ( mAPISFilesList.length() == 1 && mAPISFilesList[0].right( 3 ) == QLatin1String( "pap" ) )
  {
    if ( !QFileInfo::exists( mAPISFilesList[0] ) )
    {
      QgsDebugMsg( QStringLiteral( "The apis file %1 not found" ).arg( mAPISFilesList.at( 0 ) ) );
      return;
    }
    mPapFile = mAPISFilesList[0];
    apis->loadPrepared( mPapFile );
  }
  else
  {
    for ( const QString &path : std::as_const( mAPISFilesList ) )
    {
      if ( !QFileInfo::exists( path ) )
      {
        QgsDebugMsg( QStringLiteral( "The apis file %1 was not found" ).arg( path ) );
      }
      else
      {
        apis->load( path );
      }
    }
    apis->prepare();
  }
  if ( apis )
    pyLexer->setAPIs( apis.release() );

  setLexer( pyLexer );

  const int threshold = settings.value( QStringLiteral( "pythonConsole/autoCompThreshold" ), 2 ).toInt();
  setAutoCompletionThreshold( threshold );
  if ( !settings.value( "pythonConsole/autoCompleteEnabled", true ).toBool() )
  {
    setAutoCompletionSource( AcsNone );
  }
  else
  {
    const QString autoCompleteSource = settings.value( QStringLiteral( "pythonConsole/autoCompleteSource" ), QStringLiteral( "fromAPI" ) ).toString();
    if ( autoCompleteSource == QLatin1String( "fromDoc" ) )
      setAutoCompletionSource( AcsDocument );
    else if ( autoCompleteSource == QLatin1String( "fromDocAPI" ) )
      setAutoCompletionSource( AcsAll );
    else
      setAutoCompletionSource( AcsAPIs );
  }

  setLineNumbersVisible( true );
  setIndentationsUseTabs( false );
  setIndentationGuides( true );

  runPostLexerConfigurationTasks();
}

void QgsCodeEditorPython::keyPressEvent( QKeyEvent *event )
{
  // If editor is readOnly, use the default implementation
  if ( isReadOnly() )
  {
    return QgsCodeEditor::keyPressEvent( event );
  }
  const bool ctrlModifier = event->modifiers() & Qt::ControlModifier;

  // Toggle comment when user presses  Ctrl+:
  if ( ctrlModifier && event->key() == Qt::Key_Colon )
  {
    event->accept();
    toggleComment();
    return;
  }

  const QgsSettings settings;

  bool autoCloseBracket = settings.value( QStringLiteral( "/pythonConsole/autoCloseBracket" ), true ).toBool();
  bool autoSurround = settings.value( QStringLiteral( "/pythonConsole/autoSurround" ), true ).toBool();
  bool autoInsertImport = settings.value( QStringLiteral( "/pythonConsole/autoInsertImport" ), false ).toBool();

  // Update calltips when cursor position changes with left and right keys
  if ( event->key() == Qt::Key_Left  ||
       event->key() == Qt::Key_Right ||
       event->key() == Qt::Key_Up ||
       event->key() == Qt::Key_Down )
  {
    QgsCodeEditor::keyPressEvent( event );
    callTip();
    return;
  }

  // Get entered text and cursor position
  const QString eText = event->text();
  int line, column;
  getCursorPosition( &line, &column );

  // If some text is selected and user presses an opening character
  // surround the selection with the opening-closing pair
  if ( hasSelectedText() && autoSurround )
  {
    if ( sCompletionPairs.contains( eText ) )
    {
      int startLine, startPos, endLine, endPos;
      getSelection( &startLine, &startPos, &endLine, &endPos );

      // Special case for Multi line quotes (insert triple quotes)
      if ( startLine != endLine && ( eText == "\"" || eText == "'" ) )
      {
        replaceSelectedText(
          QString( "%1%1%1%2%3%3%3" ).arg( eText, selectedText(), sCompletionPairs[eText] )
        );
        setSelection( startLine, startPos + 3, endLine, endPos + 3 );
      }
      else
      {
        replaceSelectedText(
          QString( "%1%2%3" ).arg( eText, selectedText(), sCompletionPairs[eText] )
        );
        setSelection( startLine, startPos + 1, endLine, endPos + 1 );
      }
      event->accept();
      return;
    }
    else if ( sCompletionSingleCharacters.contains( eText ) )
    {
      int startLine, startPos, endLine, endPos;
      getSelection( &startLine, &startPos, &endLine, &endPos );
      replaceSelectedText(
        QString( "%1%2%1" ).arg( eText, selectedText() )
      );
      setSelection( startLine, startPos + 1, endLine, endPos + 1 );
      event->accept();
      return;
    }
  }

  // No selected text
  else
  {
    // Automatically insert "import" after "from xxx " if option is enabled
    if ( autoInsertImport && eText == " " )
    {
      const QString lineText = text( line );
      const thread_local QRegularExpression re( QStringLiteral( "^from [\\w.]+$" ) );
      if ( re.match( lineText.trimmed() ).hasMatch() )
      {
        insert( QStringLiteral( " import" ) );
        setCursorPosition( line, column + 7 );
        return QgsCodeEditor::keyPressEvent( event );
      }
    }

    // Handle automatic bracket insertion/deletion if option is enabled
    else if ( autoCloseBracket )
    {
      const QString prevChar = characterBeforeCursor();
      const QString nextChar = characterAfterCursor();

      // When backspace is pressed inside an opening/closing pair, remove both characters
      if ( event->key() == Qt::Key_Backspace )
      {
        if ( sCompletionPairs.contains( prevChar ) && sCompletionPairs[prevChar] == nextChar )
        {
          setSelection( line, column - 1, line, column + 1 );
          removeSelectedText();
          event->accept();
        }
        else
        {
          QgsCodeEditor::keyPressEvent( event );
        }

        // Update calltips (cursor position has changed)
        callTip();
        return;
      }

      // When closing character is entered inside an opening/closing pair, shift the cursor
      else if ( sCompletionPairs.key( eText ) != ""  && nextChar == eText )
      {
        setCursorPosition( line, column + 1 );
        event->accept();

        // Will hide calltips when a closing parenthesis is entered
        callTip();
        return;
      }

      // Else, if not inside a string or comment and an opening character
      // is entered, also insert the closing character
      else if ( !isCursorInsideStringLiteralOrComment() && sCompletionPairs.contains( eText ) )
      {
        // Check if user is not entering triple quotes
        if ( !( ( eText == "\"" || eText == "'" ) && prevChar == eText ) )
        {
          QgsCodeEditor::keyPressEvent( event );
          insert( sCompletionPairs[eText] );
          event->accept();
          return;
        }
      }
    }
  }

  // Let QgsCodeEditor handle the keyboard event
  return QgsCodeEditor::keyPressEvent( event );
}

void QgsCodeEditorPython::autoComplete()
{
  switch ( autoCompletionSource() )
  {
    case AcsDocument:
      autoCompleteFromDocument();
      break;

    case AcsAPIs:
      autoCompleteFromAPIs();
      break;

    case AcsAll:
      autoCompleteFromAll();
      break;

    case AcsNone:
      break;
  }
}

void QgsCodeEditorPython::loadAPIs( const QList<QString> &filenames )
{
  mAPISFilesList = filenames;
  //QgsDebugMsg( QStringLiteral( "The apis files: %1" ).arg( mAPISFilesList[0] ) );
  initializeLexer();
}

bool QgsCodeEditorPython::loadScript( const QString &script )
{
  QgsDebugMsgLevel( QStringLiteral( "The script file: %1" ).arg( script ), 2 );
  QFile file( script );
  if ( !file.open( QIODevice::ReadOnly ) )
  {
    return false;
  }

  QTextStream in( &file );

  setText( in.readAll().trimmed() );
  file.close();

  initializeLexer();
  return true;
}

bool QgsCodeEditorPython::isCursorInsideStringLiteralOrComment() const
{
  int line, index;
  getCursorPosition( &line, &index );
  int position = positionFromLineIndex( line, index );

  // Special case: cursor at the end of the document. Style will always be Default,
  // so  we have to  check the style of the previous character.
  // It it is an unclosed string (triple string, unclosed, or comment),
  // consider cursor is inside a string.
  if ( position >= length() && position > 0 )
  {
    long style = SendScintilla( QsciScintillaBase::SCI_GETSTYLEAT, position - 1 );
    return style == QsciLexerPython::Comment
           || style == QsciLexerPython::TripleSingleQuotedString
           || style == QsciLexerPython::TripleDoubleQuotedString
           || style == QsciLexerPython::TripleSingleQuotedFString
           || style == QsciLexerPython::TripleDoubleQuotedFString
           || style == QsciLexerPython::UnclosedString;
  }
  else
  {
    long style = SendScintilla( QsciScintillaBase::SCI_GETSTYLEAT, position );
    return style == QsciLexerPython::Comment
           || style == QsciLexerPython::DoubleQuotedString
           || style == QsciLexerPython::SingleQuotedString
           || style == QsciLexerPython::TripleSingleQuotedString
           || style == QsciLexerPython::TripleDoubleQuotedString
           || style == QsciLexerPython::CommentBlock
           || style == QsciLexerPython::UnclosedString
           || style == QsciLexerPython::DoubleQuotedFString
           || style == QsciLexerPython::SingleQuotedFString
           || style == QsciLexerPython::TripleSingleQuotedFString
           || style == QsciLexerPython::TripleDoubleQuotedFString;
  }
}

QString QgsCodeEditorPython::characterBeforeCursor() const
{
  int line, index;
  getCursorPosition( &line, &index );
  int position = positionFromLineIndex( line, index );
  if ( position <= 0 )
  {
    return QString();
  }
  return text( position - 1, position );
}

QString QgsCodeEditorPython::characterAfterCursor() const
{
  int line, index;
  getCursorPosition( &line, &index );
  int position = positionFromLineIndex( line, index );
  if ( position >= length() )
  {
    return QString();
  }
  return text( position, position + 1 );
}

void QgsCodeEditorPython::searchSelectedTextInPyQGISDocs()
{
  if ( !hasSelectedText() )
    return;

  QString text = selectedText();
  text = text.replace( QLatin1String( ">>> " ), QString() ).replace( QLatin1String( "... " ), QString() ).trimmed(); // removing prompts
  const QString version = QString( Qgis::version() ).split( '.' ).mid( 0, 2 ).join( '.' );
  QDesktopServices::openUrl( QUrl( QStringLiteral( "https://qgis.org/pyqgis/%1/search.html?q=%2" ).arg( version, text ) ) );
}

void QgsCodeEditorPython::toggleComment()
{
  if ( isReadOnly() )
  {
    return;
  }

  beginUndoAction();
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

  // Check comment state and minimum indentation for each selected line
  bool allEmpty = true;
  bool allCommented = true;
  int minIndentation = -1;
  for ( int line = startLine; line <= endLine; line++ )
  {
    const QString stripped = text( line ).trimmed();
    if ( !stripped.isEmpty() )
    {
      allEmpty = false;
      if ( !stripped.startsWith( '#' ) )
      {
        allCommented = false;
      }
      if ( minIndentation == -1 || minIndentation > indentation( line ) )
      {
        minIndentation = indentation( line );
      }
    }
  }

  // Special case, only empty lines
  if ( allEmpty )
  {
    return;
  }

  // Selection shift to keep the same selected text after a # is added/removed
  int delta = 0;

  for ( int line = startLine; line <= endLine; line++ )
  {
    const QString stripped = text( line ).trimmed();

    // Empty line
    if ( stripped.isEmpty() )
    {
      continue;
    }

    if ( !allCommented )
    {
      insertAt( QStringLiteral( "# " ), line, minIndentation );
      delta = -2;
    }
    else
    {
      if ( !stripped.startsWith( '#' ) )
      {
        continue;
      }
      if ( stripped.startsWith( QLatin1String( "# " ) ) )
      {
        delta = 2;
      }
      else
      {
        delta = 1;
      }
      setSelection( line, indentation( line ), line, indentation( line ) + delta );
      removeSelectedText();
    }
  }

  endUndoAction();
  setSelection( startLine, startPos - delta, endLine, endPos - delta );
}

///@cond PRIVATE
//
// QgsQsciLexerPython
//
QgsQsciLexerPython::QgsQsciLexerPython( QObject *parent )
  : QsciLexerPython( parent )
{

}

const char *QgsQsciLexerPython::keywords( int set ) const
{
  if ( set == 1 )
  {
    return "True False and as assert break class continue def del elif else except "
           "finally for from global if import in is lambda None not or pass "
           "raise return try while with yield async await nonlocal";
  }

  return QsciLexerPython::keywords( set );
}
///@endcond PRIVATE
