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

#include "qgscodeeditorpython.h"

#include "qgis.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsprocessingutils.h"
#include "qgspythonrunner.h"
#include "qgssettings.h"
#include "qgssettingsentryenumflag.h"
#include "qgssettingsentryimpl.h"
#include "qgssymbollayerutils.h"

#include <QAction>
#include <QDesktopServices>
#include <QFileInfo>
#include <QFont>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QString>
#include <QTextStream>
#include <QUrl>
#include <QWidget>
#include <Qsci/qscilexerpython.h>

#include "moc_qgscodeeditorpython.cpp"

const QMap<QString, QString> QgsCodeEditorPython::sCompletionPairs {
  { "(", ")" },
  { "[", "]" },
  { "{", "}" },
  { "'", "'" },
  { "\"", "\"" }
};
const QStringList QgsCodeEditorPython::sCompletionSingleCharacters { "`", "*" };
///@cond PRIVATE
const QgsSettingsEntryString *QgsCodeEditorPython::settingCodeFormatter = new QgsSettingsEntryString( u"formatter"_s, sTreePythonCodeEditor, u"autopep8"_s, u"Python code autoformatter"_s );
const QgsSettingsEntryInteger *QgsCodeEditorPython::settingMaxLineLength = new QgsSettingsEntryInteger( u"max-line-length"_s, sTreePythonCodeEditor, 80, u"Maximum line length"_s );
const QgsSettingsEntryBool *QgsCodeEditorPython::settingSortImports = new QgsSettingsEntryBool( u"sort-imports"_s, sTreePythonCodeEditor, true, u"Whether imports should be sorted when auto-formatting code"_s );
const QgsSettingsEntryInteger *QgsCodeEditorPython::settingAutopep8Level = new QgsSettingsEntryInteger( u"autopep8-level"_s, sTreePythonCodeEditor, 1, u"Autopep8 aggressive level"_s );
const QgsSettingsEntryBool *QgsCodeEditorPython::settingBlackNormalizeQuotes = new QgsSettingsEntryBool( u"black-normalize-quotes"_s, sTreePythonCodeEditor, true, u"Whether quotes should be normalized when auto-formatting code using black"_s );
const QgsSettingsEntryString *QgsCodeEditorPython::settingExternalPythonEditorCommand = new QgsSettingsEntryString( u"external-editor"_s, sTreePythonCodeEditor, QString(), u"Command to launch an external Python code editor. Use the token <file> to insert the filename, <line> to insert line number, and <col> to insert the column number."_s );
const QgsSettingsEntryEnumFlag<Qgis::DocumentationBrowser> *QgsCodeEditorPython::settingContextHelpBrowser = new QgsSettingsEntryEnumFlag<Qgis::DocumentationBrowser>( u"context-help-browser"_s, sTreePythonCodeEditor, Qgis::DocumentationBrowser::DeveloperToolsPanel, u"Web browser used to display the api documentation"_s );
///@endcond PRIVATE


QgsCodeEditorPython::QgsCodeEditorPython( QWidget *parent, const QList<QString> &filenames, Mode mode, Flags flags )
  : QgsCodeEditor( parent, QString(), false, false, flags, mode )
  , mAPISFilesList( filenames )
{
  if ( !parent )
  {
    setTitle( tr( "Python Editor" ) );
  }

  setCaretWidth( 2 );

  QgsCodeEditorPython::initializeLexer();

  connect( this, &QgsCodeEditorPython::helpRequested, this, &QgsCodeEditorPython::showApiDocumentation );

  updateCapabilities();
}

Qgis::ScriptLanguage QgsCodeEditorPython::language() const
{
  return Qgis::ScriptLanguage::Python;
}

Qgis::ScriptLanguageCapabilities QgsCodeEditorPython::languageCapabilities() const
{
  return mCapabilities;
}

void QgsCodeEditorPython::initializeLexer()
{
  // current line
  setEdgeMode( QsciScintilla::EdgeLine );
  setEdgeColumn( settingMaxLineLength->value() );
  setEdgeColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Edge ) );

  setWhitespaceVisibility( QsciScintilla::WsVisibleAfterIndent );

  SendScintilla( QsciScintillaBase::SCI_SETPROPERTY, "highlight.current.word", "1" );

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
  pyLexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Error ), QsciLexerPython::UnclosedString );
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
  pyLexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::SingleQuote ), QsciLexerPython::SingleQuotedFString );
  pyLexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::DoubleQuote ), QsciLexerPython::DoubleQuotedString );
  pyLexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::DoubleQuote ), QsciLexerPython::DoubleQuotedFString );
  pyLexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::TripleSingleQuote ), QsciLexerPython::TripleSingleQuotedString );
  pyLexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::TripleDoubleQuote ), QsciLexerPython::TripleDoubleQuotedString );

  auto apis = std::make_unique<QsciAPIs>( pyLexer );

  QgsSettings settings;
  if ( mAPISFilesList.isEmpty() )
  {
    if ( settings.value( u"pythonConsole/preloadAPI"_s, true ).toBool() )
    {
      mPapFile = QgsApplication::pkgDataPath() + u"/python/qsci_apis/PyQGIS.pap"_s;
      apis->loadPrepared( mPapFile );
    }
    else if ( settings.value( u"pythonConsole/usePreparedAPIFile"_s, false ).toBool() )
    {
      apis->loadPrepared( settings.value( u"pythonConsole/preparedAPIFile"_s ).toString() );
    }
    else
    {
      const QStringList apiPaths = settings.value( u"pythonConsole/userAPI"_s ).toStringList();
      for ( const QString &path : apiPaths )
      {
        if ( !QFileInfo::exists( path ) )
        {
          QgsDebugError( u"The apis file %1 was not found"_s.arg( path ) );
        }
        else
        {
          apis->load( path );
        }
      }
      apis->prepare();
    }
  }
  else if ( mAPISFilesList.length() == 1 && mAPISFilesList[0].right( 3 ) == "pap"_L1 )
  {
    if ( !QFileInfo::exists( mAPISFilesList[0] ) )
    {
      QgsDebugError( u"The apis file %1 not found"_s.arg( mAPISFilesList.at( 0 ) ) );
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
        QgsDebugError( u"The apis file %1 was not found"_s.arg( path ) );
      }
      else
      {
        apis->load( path );
      }
    }
    apis->prepare();
  }
  pyLexer->setAPIs( apis.release() );

  setLexer( pyLexer );

  const int threshold = settings.value( u"pythonConsole/autoCompThreshold"_s, 2 ).toInt();
  setAutoCompletionThreshold( threshold );
  if ( !settings.value( "pythonConsole/autoCompleteEnabled", true ).toBool() )
  {
    setAutoCompletionSource( AcsNone );
  }
  else
  {
    const QString autoCompleteSource = settings.value( u"pythonConsole/autoCompleteSource"_s, u"fromAPI"_s ).toString();
    if ( autoCompleteSource == "fromDoc"_L1 )
      setAutoCompletionSource( AcsDocument );
    else if ( autoCompleteSource == "fromDocAPI"_L1 )
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

  const QgsSettings settings;

  bool autoCloseBracket = settings.value( u"/pythonConsole/autoCloseBracket"_s, true ).toBool();
  bool autoSurround = settings.value( u"/pythonConsole/autoSurround"_s, true ).toBool();
  bool autoInsertImport = settings.value( u"/pythonConsole/autoInsertImport"_s, false ).toBool();

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
      const thread_local QRegularExpression re( u"^from [\\w.]+$"_s );
      if ( re.match( lineText.trimmed() ).hasMatch() )
      {
        insert( u" import"_s );
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
          // Update calltips (cursor position has changed)
          callTip();
        }
        else
        {
          QgsCodeEditor::keyPressEvent( event );
        }
        return;
      }

      // When closing character is entered inside an opening/closing pair, shift the cursor
      else if ( sCompletionPairs.key( eText ) != "" && nextChar == eText )
      {
        setCursorPosition( line, column + 1 );
        event->accept();

        // Will hide calltips when a closing parenthesis is entered
        callTip();
        return;
      }

      // Else, if not inside a string or comment and an opening character
      // is entered, also insert the closing character, provided the next
      // character is a space, a colon, or a closing character
      else if ( !isCursorInsideStringLiteralOrComment()
                && sCompletionPairs.contains( eText )
                && ( nextChar.isEmpty() || nextChar.at( 0 ).isSpace() || nextChar == ":" || sCompletionPairs.key( nextChar ) != "" ) )
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

QString QgsCodeEditorPython::reformatCodeString( const QString &string )
{
  if ( !QgsPythonRunner::isValid() )
  {
    return string;
  }

  const QString formatter = settingCodeFormatter->value();
  const int maxLineLength = settingMaxLineLength->value();

  QString newText = string;

  QStringList missingModules;

  if ( settingSortImports->value() )
  {
    const QString defineSortImports = QStringLiteral(
                                        "def __qgis_sort_imports(script):\n"
                                        "  try:\n"
                                        "    import isort\n"
                                        "  except ImportError:\n"
                                        "    return '_ImportError'\n"
                                        "  options={'line_length': %1, 'profile': '%2', 'known_first_party': ['qgis', 'console', 'processing', 'plugins']}\n"
                                        "  return isort.code(script, **options)\n"
    )
                                        .arg( maxLineLength )
                                        .arg( formatter == "black"_L1 ? u"black"_s : QString() );

    if ( !QgsPythonRunner::run( defineSortImports ) )
    {
      QgsDebugError( u"Error running script: %1"_s.arg( defineSortImports ) );
      return string;
    }

    const QString script = u"__qgis_sort_imports(%1)"_s.arg( QgsProcessingUtils::stringToPythonLiteral( newText ) );
    QString result;
    if ( QgsPythonRunner::eval( script, result ) )
    {
      if ( result == "_ImportError"_L1 )
      {
        missingModules << u"isort"_s;
      }
      else
      {
        newText = result;
      }
    }
    else
    {
      QgsDebugError( u"Error running script: %1"_s.arg( script ) );
      return newText;
    }
  }

  if ( formatter == "autopep8"_L1 )
  {
    const int level = settingAutopep8Level->value();

    const QString defineReformat = QStringLiteral(
                                     "def __qgis_reformat(script):\n"
                                     "  try:\n"
                                     "    import autopep8\n"
                                     "  except ImportError:\n"
                                     "    return '_ImportError'\n"
                                     "  options={'aggressive': %1, 'max_line_length': %2}\n"
                                     "  return autopep8.fix_code(script, options=options)\n"
    )
                                     .arg( level )
                                     .arg( maxLineLength );

    if ( !QgsPythonRunner::run( defineReformat ) )
    {
      QgsDebugError( u"Error running script: %1"_s.arg( defineReformat ) );
      return newText;
    }

    const QString script = u"__qgis_reformat(%1)"_s.arg( QgsProcessingUtils::stringToPythonLiteral( newText ) );
    QString result;
    if ( QgsPythonRunner::eval( script, result ) )
    {
      if ( result == "_ImportError"_L1 )
      {
        missingModules << u"autopep8"_s;
      }
      else
      {
        newText = result;
      }
    }
    else
    {
      QgsDebugError( u"Error running script: %1"_s.arg( script ) );
      return newText;
    }
  }
  else if ( formatter == "black"_L1 )
  {
    const bool normalize = settingBlackNormalizeQuotes->value();

    if ( !checkSyntax() )
    {
      showMessage( tr( "Reformat Code" ), tr( "Code formatting failed -- the code contains syntax errors" ), Qgis::MessageLevel::Warning );
      return newText;
    }

    const QString defineReformat = QStringLiteral(
                                     "def __qgis_reformat(script):\n"
                                     "  try:\n"
                                     "    import black\n"
                                     "  except ImportError:\n"
                                     "    return '_ImportError'\n"
                                     "  options={'string_normalization': %1, 'line_length': %2}\n"
                                     "  return black.format_str(script, mode=black.Mode(**options))\n"
    )
                                     .arg( QgsProcessingUtils::variantToPythonLiteral( normalize ) )
                                     .arg( maxLineLength );

    if ( !QgsPythonRunner::run( defineReformat ) )
    {
      QgsDebugError( u"Error running script: %1"_s.arg( defineReformat ) );
      return string;
    }

    const QString script = u"__qgis_reformat(%1)"_s.arg( QgsProcessingUtils::stringToPythonLiteral( newText ) );
    QString result;
    if ( QgsPythonRunner::eval( script, result ) )
    {
      if ( result == "_ImportError"_L1 )
      {
        missingModules << u"black"_s;
      }
      else
      {
        newText = result;
      }
    }
    else
    {
      QgsDebugError( u"Error running script: %1"_s.arg( script ) );
      return newText;
    }
  }

  if ( !missingModules.empty() )
  {
    if ( missingModules.size() == 1 )
    {
      showMessage( tr( "Reformat Code" ), tr( "The Python module %1 is missing" ).arg( missingModules.at( 0 ) ), Qgis::MessageLevel::Warning );
    }
    else
    {
      const QString modules = missingModules.join( ", "_L1 );
      showMessage( tr( "Reformat Code" ), tr( "The Python modules %1 are missing" ).arg( modules ), Qgis::MessageLevel::Warning );
    }
  }

  return newText;
}

void QgsCodeEditorPython::populateContextMenu( QMenu *menu )
{
  QgsCodeEditor::populateContextMenu( menu );

  QString text = selectedText();
  if ( text.isEmpty() )
  {
    text = wordAtPoint( mapFromGlobal( QCursor::pos() ) );
  }
  if ( text.isEmpty() )
  {
    return;
  }

  QAction *pyQgisHelpAction = new QAction(
    QgsApplication::getThemeIcon( u"console/iconHelpConsole.svg"_s ),
    tr( "Search Selection in PyQGIS Documentation" ),
    menu
  );

  pyQgisHelpAction->setEnabled( hasSelectedText() );
  pyQgisHelpAction->setShortcut( QKeySequence::StandardKey::HelpContents );
  connect( pyQgisHelpAction, &QAction::triggered, this, [text, this] { showApiDocumentation( text ); } );

  menu->addSeparator();
  menu->addAction( pyQgisHelpAction );
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
  //QgsDebugMsgLevel( u"The apis files: %1"_s.arg( mAPISFilesList[0] ), 2 );
  initializeLexer();
}

bool QgsCodeEditorPython::loadScript( const QString &script )
{
  QgsDebugMsgLevel( u"The script file: %1"_s.arg( script ), 2 );
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
  int position = linearPosition();

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
  int position = linearPosition();
  if ( position <= 0 )
  {
    return QString();
  }
  return text( position - 1, position );
}

QString QgsCodeEditorPython::characterAfterCursor() const
{
  int position = linearPosition();
  if ( position >= length() )
  {
    return QString();
  }
  return text( position, position + 1 );
}

void QgsCodeEditorPython::updateCapabilities()
{
  mCapabilities = Qgis::ScriptLanguageCapability::ToggleComment;

  if ( !QgsPythonRunner::isValid() )
    return;

  mCapabilities |= Qgis::ScriptLanguageCapability::CheckSyntax;

  // we could potentially check for autopep8/black import here and reflect the capability accordingly.
  // (current approach is to to always indicate this capability and raise a user-friendly warning
  // when attempting to reformat if the libraries can't be imported)
  mCapabilities |= Qgis::ScriptLanguageCapability::Reformat;
}

bool QgsCodeEditorPython::checkSyntax()
{
  clearWarnings();

  if ( !QgsPythonRunner::isValid() )
  {
    return true;
  }

  const QString originalText = text();

  const QString defineCheckSyntax = QStringLiteral(
    "def __check_syntax(script):\n"
    "  try:\n"
    "    compile(script.encode('utf-8'), '', 'exec')\n"
    "  except SyntaxError as detail:\n"
    "    eline = detail.lineno or 1\n"
    "    eline -= 1\n"
    "    ecolumn = detail.offset or 1\n"
    "    edescr = detail.msg\n"
    "    return '!!!!'.join([str(eline), str(ecolumn), edescr])\n"
    "  return ''"
  );

  if ( !QgsPythonRunner::run( defineCheckSyntax ) )
  {
    QgsDebugError( u"Error running script: %1"_s.arg( defineCheckSyntax ) );
    return true;
  }

  const QString script = u"__check_syntax(%1)"_s.arg( QgsProcessingUtils::stringToPythonLiteral( originalText ) );
  QString result;
  if ( QgsPythonRunner::eval( script, result ) )
  {
    if ( result.size() == 0 )
    {
      return true;
    }
    else
    {
      const QStringList parts = result.split( u"!!!!"_s );
      if ( parts.size() == 3 )
      {
        const int line = parts.at( 0 ).toInt();
        const int column = parts.at( 1 ).toInt();
        addWarning( line, parts.at( 2 ) );
        setCursorPosition( line, column - 1 );
        ensureLineVisible( line );
      }
      return false;
    }
  }
  else
  {
    QgsDebugError( u"Error running script: %1"_s.arg( script ) );
    return true;
  }
}

void QgsCodeEditorPython::searchSelectedTextInPyQGISDocs()
{
  showApiDocumentation( selectedText() );
}

void QgsCodeEditorPython::showApiDocumentation( const QString &text )
{
  QString searchText = text;
  searchText = searchText.replace( ">>> "_L1, QString() ).replace( "... "_L1, QString() ).trimmed(); // removing prompts

  QRegularExpression qtExpression( "^Q[A-Z][a-zA-Z]" );

  if ( qtExpression.match( searchText ).hasMatch() )
  {
    const QString qtVersion = QString( qVersion() ).split( '.' ).mid( 0, 2 ).join( '.' );
    QString baseUrl = QString( "https://doc.qt.io/qt-%1" ).arg( qtVersion );
    QDesktopServices::openUrl( QUrl( u"%1/%2.html"_s.arg( baseUrl, searchText.toLower() ) ) );
    return;
  }
  const QString qgisVersion = QString( Qgis::version() ).split( '.' ).mid( 0, 2 ).join( '.' );
  if ( searchText.isEmpty() )
  {
    QDesktopServices::openUrl( QUrl( u"https://qgis.org/pyqgis/%1/"_s.arg( qgisVersion ) ) );
  }
  else
  {
    QDesktopServices::openUrl( QUrl( u"https://qgis.org/pyqgis/%1/search.html?q=%2"_s.arg( qgisVersion, searchText ) ) );
  }
}

void QgsCodeEditorPython::toggleComment()
{
  toggleLineComments( u"#"_s );
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
