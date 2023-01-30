/***************************************************************************
    qgscodeeditor.cpp - A base code editor for QGIS and plugins.  Provides
                        a base editor using QScintilla for editors
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
#include "qgscodeeditor.h"
#include "qgssettings.h"
#include "qgssymbollayerutils.h"
#include "qgsgui.h"
#include "qgscodeeditorcolorschemeregistry.h"
#include "qgscodeeditorhistorydialog.h"

#include <QLabel>
#include <QWidget>
#include <QFont>
#include <QFontDatabase>
#include <QDebug>
#include <QFocusEvent>
#include <Qsci/qscistyle.h>
#include <QMenu>
#include <QClipboard>

QMap< QgsCodeEditorColorScheme::ColorRole, QString > QgsCodeEditor::sColorRoleToSettingsKey
{
  {QgsCodeEditorColorScheme::ColorRole::Default, QStringLiteral( "defaultFontColor" ) },
  {QgsCodeEditorColorScheme::ColorRole::Keyword, QStringLiteral( "keywordFontColor" ) },
  {QgsCodeEditorColorScheme::ColorRole::Class, QStringLiteral( "classFontColor" ) },
  {QgsCodeEditorColorScheme::ColorRole::Method, QStringLiteral( "methodFontColor" ) },
  {QgsCodeEditorColorScheme::ColorRole::Decoration, QStringLiteral( "decoratorFontColor" ) },
  {QgsCodeEditorColorScheme::ColorRole::Number, QStringLiteral( "numberFontColor" ) },
  {QgsCodeEditorColorScheme::ColorRole::Comment, QStringLiteral( "commentFontColor" ) },
  {QgsCodeEditorColorScheme::ColorRole::CommentLine, QStringLiteral( "commentLineFontColor" ) },
  {QgsCodeEditorColorScheme::ColorRole::CommentBlock, QStringLiteral( "commentBlockFontColor" ) },
  {QgsCodeEditorColorScheme::ColorRole::Background, QStringLiteral( "paperBackgroundColor" ) },
  {QgsCodeEditorColorScheme::ColorRole::Cursor, QStringLiteral( "cursorColor" ) },
  {QgsCodeEditorColorScheme::ColorRole::CaretLine, QStringLiteral( "caretLineColor" ) },
  {QgsCodeEditorColorScheme::ColorRole::Operator, QStringLiteral( "operatorFontColor" ) },
  {QgsCodeEditorColorScheme::ColorRole::QuotedOperator, QStringLiteral( "quotedOperatorFontColor" ) },
  {QgsCodeEditorColorScheme::ColorRole::Identifier, QStringLiteral( "identifierFontColor" ) },
  {QgsCodeEditorColorScheme::ColorRole::QuotedIdentifier, QStringLiteral( "quotedIdentifierFontColor" ) },
  {QgsCodeEditorColorScheme::ColorRole::Tag, QStringLiteral( "tagFontColor" ) },
  {QgsCodeEditorColorScheme::ColorRole::UnknownTag, QStringLiteral( "unknownTagFontColor" ) },
  {QgsCodeEditorColorScheme::ColorRole::SingleQuote, QStringLiteral( "singleQuoteFontColor" ) },
  {QgsCodeEditorColorScheme::ColorRole::DoubleQuote, QStringLiteral( "doubleQuoteFontColor" ) },
  {QgsCodeEditorColorScheme::ColorRole::TripleSingleQuote, QStringLiteral( "tripleSingleQuoteFontColor" ) },
  {QgsCodeEditorColorScheme::ColorRole::TripleDoubleQuote, QStringLiteral( "tripleDoubleQuoteFontColor" ) },
  {QgsCodeEditorColorScheme::ColorRole::MarginBackground, QStringLiteral( "marginBackgroundColor" ) },
  {QgsCodeEditorColorScheme::ColorRole::MarginForeground, QStringLiteral( "marginForegroundColor" ) },
  {QgsCodeEditorColorScheme::ColorRole::SelectionBackground, QStringLiteral( "selectionBackgroundColor" ) },
  {QgsCodeEditorColorScheme::ColorRole::SelectionForeground, QStringLiteral( "selectionForegroundColor" ) },
  {QgsCodeEditorColorScheme::ColorRole::MatchedBraceBackground, QStringLiteral( "matchedBraceBackground" ) },
  {QgsCodeEditorColorScheme::ColorRole::MatchedBraceForeground, QStringLiteral( "matchedBraceColor" ) },
  {QgsCodeEditorColorScheme::ColorRole::Edge, QStringLiteral( "edgeColor" ) },
  {QgsCodeEditorColorScheme::ColorRole::Fold, QStringLiteral( "foldColor" ) },
  {QgsCodeEditorColorScheme::ColorRole::Error, QStringLiteral( "stderrFontColor" ) },
  {QgsCodeEditorColorScheme::ColorRole::ErrorBackground, QStringLiteral( "stderrBackgroundColor" ) },
  {QgsCodeEditorColorScheme::ColorRole::FoldIconForeground, QStringLiteral( "foldIconForeground" ) },
  {QgsCodeEditorColorScheme::ColorRole::FoldIconHalo, QStringLiteral( "foldIconHalo" ) },
  {QgsCodeEditorColorScheme::ColorRole::IndentationGuide, QStringLiteral( "indentationGuide" ) },
};


QgsCodeEditor::QgsCodeEditor( QWidget *parent, const QString &title, bool folding, bool margin, QgsCodeEditor::Flags flags, QgsCodeEditor::Mode mode )
  : QsciScintilla( parent )
  , mWidgetTitle( title )
  , mMargin( margin )
  , mFlags( flags )
  , mMode( mode )
{
  if ( !parent && mWidgetTitle.isEmpty() )
  {
    setWindowTitle( QStringLiteral( "Text Editor" ) );
  }
  else
  {
    setWindowTitle( mWidgetTitle );
  }

  if ( folding )
    mFlags |= QgsCodeEditor::Flag::CodeFolding;

  mSoftHistory.append( QString() );

  setSciWidget();
  setHorizontalScrollBarPolicy( Qt::ScrollBarAsNeeded );

  SendScintilla( SCI_SETADDITIONALSELECTIONTYPING, 1 );
  SendScintilla( SCI_SETMULTIPASTE, 1 );
  SendScintilla( SCI_SETVIRTUALSPACEOPTIONS, SCVS_RECTANGULARSELECTION );

  SendScintilla( SCI_SETMARGINTYPEN, static_cast< int >( QgsCodeEditor::MarginRole::ErrorIndicators ), SC_MARGIN_SYMBOL );
  SendScintilla( SCI_SETMARGINMASKN, static_cast< int >( QgsCodeEditor::MarginRole::ErrorIndicators ), 1 << MARKER_NUMBER );
  setMarginWidth( static_cast< int >( QgsCodeEditor::MarginRole::ErrorIndicators ), 0 );
  setAnnotationDisplay( QsciScintilla::AnnotationBoxed );

  connect( QgsGui::instance(), &QgsGui::optionsChanged, this, [ = ]
  {
    setSciWidget();
    initializeLexer();
  } );

  switch ( mMode )
  {
    case QgsCodeEditor::Mode::ScriptEditor:
      break;

    case QgsCodeEditor::Mode::OutputDisplay:
    {
      // Don't want to see the horizontal scrollbar at all
      SendScintilla( QsciScintilla::SCI_SETHSCROLLBAR, 0 );

      setWrapMode( QsciScintilla::WrapCharacter );
      break;
    }

    case QgsCodeEditor::Mode::CommandInput:
    {
      // Don't want to see the horizontal scrollbar at all
      SendScintilla( QsciScintilla::SCI_SETHSCROLLBAR, 0 );

      setWrapMode( QsciScintilla::WrapCharacter );
      SendScintilla( QsciScintilla::SCI_EMPTYUNDOBUFFER );
      break;
    }
  }
}

// Workaround a bug in QScintilla 2.8.X
void QgsCodeEditor::focusOutEvent( QFocusEvent *event )
{
#if QSCINTILLA_VERSION >= 0x020800 && QSCINTILLA_VERSION < 0x020900
  if ( event->reason() != Qt::ActiveWindowFocusReason )
  {
    /* There's a bug in all QScintilla 2.8.X, where
       a focus out event that is not due to ActiveWindowFocusReason doesn't
       lead to the bliking caret being disabled. The hack consists in making
       QsciScintilla::focusOutEvent believe that the event is a ActiveWindowFocusReason
       The bug was fixed in 2.9 per:
        2015-04-14  Phil Thompson  <phil@riverbankcomputing.com>

        * qt/qsciscintillabase.cpp:
        Fixed a problem notifying when focus is lost to another application
        widget.
        [41734678234e]
    */
    QFocusEvent newFocusEvent( QEvent::FocusOut, Qt::ActiveWindowFocusReason );
    QsciScintilla::focusOutEvent( &newFocusEvent );
  }
  else
#endif
  {
    QsciScintilla::focusOutEvent( event );
  }
}

// This workaround a likely bug in QScintilla. The ESC key should not be consumned
// by the main entry, so that the default behavior (Dialog closing) can trigger,
// but only is the auto-completion suggestion list isn't displayed
void QgsCodeEditor::keyPressEvent( QKeyEvent *event )
{
  if ( isListActive() )
  {
    QsciScintilla::keyPressEvent( event );
    return;
  }

  if ( event->key() == Qt::Key_Escape )
  {
    // Shortcut QScintilla and redirect the event to the QWidget handler
    QWidget::keyPressEvent( event ); // clazy:exclude=skipped-base-method
    return;
  }

  if ( mMode == QgsCodeEditor::Mode::CommandInput )
  {
    switch ( event->key() )
    {
      case Qt::Key_Return:
      case Qt::Key_Enter:
        runCommand( text() );
        updatePrompt();
        return;

      case Qt::Key_Down:
        showPreviousCommand();
        updatePrompt();
        return;

      case Qt::Key_Up:
        showNextCommand();
        updatePrompt();
        return;

      default:
        break;
    }
  }

  QsciScintilla::keyPressEvent( event );

}

void QgsCodeEditor::contextMenuEvent( QContextMenuEvent *event )
{
  if ( mMode != QgsCodeEditor::Mode::CommandInput )
  {
    QsciScintilla::contextMenuEvent( event );
    return;
  }

  QMenu *menu = new QMenu( this );
  QMenu *historySubMenu = new QMenu( tr( "Command History" ), menu );

  historySubMenu->addAction( tr( "Show" ), this, &QgsCodeEditor::showHistory, QStringLiteral( "Ctrl+Shift+SPACE" ) );
  historySubMenu->addAction( tr( "Clear File" ), this, &QgsCodeEditor::clearPersistentHistory );
  historySubMenu->addAction( tr( "Clear Session" ), this, &QgsCodeEditor::clearSessionHistory );

  menu->addMenu( historySubMenu );
  menu->addSeparator();

  QAction *copyAction = menu->addAction( QgsApplication::getThemeIcon( "mActionEditCopy.svg" ), tr( "Copy" ), this, &QgsCodeEditor::copy, QKeySequence::Copy );
  QAction *pasteAction = menu->addAction( QgsApplication::getThemeIcon( "mActionEditPaste.svg" ), tr( "Paste" ), this, &QgsCodeEditor::paste, QKeySequence::Paste );
  copyAction->setEnabled( hasSelectedText() );
  pasteAction->setEnabled( !QApplication::clipboard()->text().isEmpty() );

  populateContextMenu( menu );

  menu->exec( mapToGlobal( event->pos() ) );
}

void QgsCodeEditor::initializeLexer()
{

}

QColor QgsCodeEditor::lexerColor( QgsCodeEditorColorScheme::ColorRole role ) const
{
  if ( mUseDefaultSettings )
    return color( role );

  if ( !mOverrideColors )
  {
    return defaultColor( role, mColorScheme );
  }
  else
  {
    const QColor color = mCustomColors.value( role );
    return !color.isValid() ? defaultColor( role ) : color;
  }
}

QFont QgsCodeEditor::lexerFont() const
{
  if ( mUseDefaultSettings )
    return getMonospaceFont();

  QFont font = QFontDatabase::systemFont( QFontDatabase::FixedFont );

  const QgsSettings settings;
  if ( !mFontFamily.isEmpty() )
    font.setFamily( mFontFamily );

#ifdef Q_OS_MAC
  if ( mFontSize > 0 )
    font.setPointSize( mFontSize );
  else
  {
    // The font size gotten from getMonospaceFont() is too small on Mac
    font.setPointSize( QLabel().font().pointSize() );
  }
#else
  if ( mFontSize > 0 )
    font.setPointSize( mFontSize );
  else
  {
    const int fontSize = settings.value( QStringLiteral( "qgis/stylesheet/fontPointSize" ), 10 ).toInt();
    font.setPointSize( fontSize );
  }
#endif
  font.setBold( false );

  return font;
}

void QgsCodeEditor::runPostLexerConfigurationTasks()
{
  updateFolding();

  setMatchedBraceForegroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::MatchedBraceForeground ) );
  setMatchedBraceBackgroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::MatchedBraceBackground ) );

  SendScintilla( SCI_MARKERSETFORE, SC_MARKNUM_FOLDEROPEN, lexerColor( QgsCodeEditorColorScheme::ColorRole::FoldIconHalo ) );
  SendScintilla( SCI_MARKERSETBACK, SC_MARKNUM_FOLDEROPEN,  lexerColor( QgsCodeEditorColorScheme::ColorRole::FoldIconForeground ) );
  SendScintilla( SCI_MARKERSETFORE, SC_MARKNUM_FOLDER, lexerColor( QgsCodeEditorColorScheme::ColorRole::FoldIconHalo ) );
  SendScintilla( SCI_MARKERSETBACK, SC_MARKNUM_FOLDER,  lexerColor( QgsCodeEditorColorScheme::ColorRole::FoldIconForeground ) );
  SendScintilla( SCI_STYLESETFORE, STYLE_INDENTGUIDE, lexerColor( QgsCodeEditorColorScheme::ColorRole::IndentationGuide ) );
  SendScintilla( SCI_STYLESETBACK, STYLE_INDENTGUIDE,  lexerColor( QgsCodeEditorColorScheme::ColorRole::IndentationGuide ) );

  if ( mMode == QgsCodeEditor::Mode::CommandInput )
  {
    setCaretLineVisible( false );
    setLineNumbersVisible( false ); // NO linenumbers for the input line
    // Margin 1 is used for the '>' prompt (console input)
    setMarginLineNumbers( 1, true );
    setMarginWidth( 1, "00000" );
    setMarginType( 1, QsciScintilla::MarginType::TextMarginRightJustified );
    setMarginsBackgroundColor( color( QgsCodeEditorColorScheme::ColorRole::Background ) );
    setEdgeMode( QsciScintilla::EdgeNone );
  }
}

void QgsCodeEditor::setSciWidget()
{
  const QFont font = lexerFont();
  setFont( font );

  setUtf8( true );
  setCaretLineVisible( true );
  setCaretLineBackgroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::CaretLine ) );
  setCaretForegroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Cursor ) );
  setSelectionForegroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::SelectionForeground ) );
  setSelectionBackgroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::SelectionBackground ) );

  setBraceMatching( QsciScintilla::SloppyBraceMatch );
  setMatchedBraceForegroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::MatchedBraceForeground ) );
  setMatchedBraceBackgroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::MatchedBraceBackground ) );

  setLineNumbersVisible( false );

  // temporarily disable folding, will be enabled later if required by updateFolding()
  setFolding( QsciScintilla::NoFoldStyle );
  setMarginWidth( static_cast< int >( QgsCodeEditor::MarginRole::FoldingControls ), 0 );

  setMarginWidth( static_cast< int >( QgsCodeEditor::MarginRole::ErrorIndicators ), 0 );

  setMarginsForegroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::MarginForeground ) );
  setMarginsBackgroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::MarginBackground ) );
  setIndentationGuidesForegroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::MarginForeground ) );
  setIndentationGuidesBackgroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::MarginBackground ) );
  // whether margin will be shown
  updateFolding();
  const QColor foldColor = lexerColor( QgsCodeEditorColorScheme::ColorRole::Fold );
  setFoldMarginColors( foldColor, foldColor );
  // indentation
  setAutoIndent( true );
  setIndentationWidth( 4 );
  setTabIndents( true );
  setBackspaceUnindents( true );
  setTabWidth( 4 );
  // autocomplete
  setAutoCompletionThreshold( 2 );
  setAutoCompletionSource( QsciScintilla::AcsAPIs );

  markerDefine( QgsApplication::getThemePixmap( "console/iconSyntaxErrorConsoleParams.svg", lexerColor( QgsCodeEditorColorScheme::ColorRole::Error ),
                lexerColor( QgsCodeEditorColorScheme::ColorRole::ErrorBackground ), 16 ), MARKER_NUMBER );
}

void QgsCodeEditor::setTitle( const QString &title )
{
  setWindowTitle( title );
}

Qgis::ScriptLanguage QgsCodeEditor::language() const
{
  return Qgis::ScriptLanguage::Unknown;
}

QString QgsCodeEditor::languageToString( Qgis::ScriptLanguage language )
{
  switch ( language )
  {
    case Qgis::ScriptLanguage::Css:
      return tr( "CSS" );
    case Qgis::ScriptLanguage::QgisExpression:
      return tr( "Expression" );
    case Qgis::ScriptLanguage::Html:
      return tr( "HTML" );
    case Qgis::ScriptLanguage::JavaScript:
      return tr( "JavaScript" );
    case Qgis::ScriptLanguage::Json:
      return tr( "JSON" );
    case Qgis::ScriptLanguage::Python:
      return tr( "Python" );
    case Qgis::ScriptLanguage::R:
      return tr( "R" );
    case Qgis::ScriptLanguage::Sql:
      return tr( "SQL" );
    case Qgis::ScriptLanguage::Unknown:
      return QString();
  }
  BUILTIN_UNREACHABLE
}

void QgsCodeEditor::setMarginVisible( bool margin )
{
  mMargin = margin;
  if ( margin )
  {
    QFont marginFont = lexerFont();
    marginFont.setPointSize( 10 );
    setMarginLineNumbers( 0, true );
    setMarginsFont( marginFont );
    setMarginWidth( static_cast< int >( QgsCodeEditor::MarginRole::LineNumbers ), QStringLiteral( "00000" ) );
    setMarginsForegroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::MarginForeground ) );
    setMarginsBackgroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::MarginBackground ) );
  }
  else
  {
    setMarginWidth( static_cast< int >( QgsCodeEditor::MarginRole::LineNumbers ), 0 );
    setMarginWidth( static_cast< int >( QgsCodeEditor::MarginRole::ErrorIndicators ), 0 );
    setMarginWidth( static_cast< int >( QgsCodeEditor::MarginRole::FoldingControls ), 0 );
  }
}

void QgsCodeEditor::setLineNumbersVisible( bool visible )
{
  if ( visible )
  {
    QFont marginFont = lexerFont();
    marginFont.setPointSize( 10 );
    setMarginLineNumbers( static_cast< int >( QgsCodeEditor::MarginRole::LineNumbers ), true );
    setMarginsFont( marginFont );
    setMarginWidth( static_cast< int >( QgsCodeEditor::MarginRole::LineNumbers ), QStringLiteral( "00000" ) );
    setMarginsForegroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::MarginForeground ) );
    setMarginsBackgroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::MarginBackground ) );
  }
  else
  {
    setMarginLineNumbers( static_cast< int >( QgsCodeEditor::MarginRole::LineNumbers ), false );
    setMarginWidth( static_cast< int >( QgsCodeEditor::MarginRole::LineNumbers ), 0 );
  }
}

bool QgsCodeEditor::lineNumbersVisible() const
{
  return marginLineNumbers( static_cast< int >( QgsCodeEditor::MarginRole::LineNumbers ) );
}

void QgsCodeEditor::setFoldingVisible( bool folding )
{
  if ( folding )
  {
    mFlags |= QgsCodeEditor::Flag::CodeFolding;
  }
  else
  {
    mFlags &= ~( static_cast< int >( QgsCodeEditor::Flag::CodeFolding ) );
  }
  updateFolding();
}

bool QgsCodeEditor::foldingVisible()
{
  return mFlags & QgsCodeEditor::Flag::CodeFolding;
}

void QgsCodeEditor::updateFolding()
{
  if ( ( mFlags & QgsCodeEditor::Flag::CodeFolding ) && mMode == QgsCodeEditor::Mode::ScriptEditor )
  {
    setMarginWidth( static_cast< int >( QgsCodeEditor::MarginRole::FoldingControls ), "0" );
    setMarginsForegroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::MarginForeground ) );
    setMarginsBackgroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::MarginBackground ) );
    setFolding( QsciScintilla::PlainFoldStyle );
  }
  else
  {
    setFolding( QsciScintilla::NoFoldStyle );
    setMarginWidth( static_cast< int >( QgsCodeEditor::MarginRole::FoldingControls ), 0 );
  }
}

bool QgsCodeEditor::readHistoryFile()
{
  if ( mHistoryFilePath.isEmpty() || !QFile::exists( mHistoryFilePath ) )
    return false;

  QFile file( mHistoryFilePath );
  if ( file.open( QIODevice::ReadOnly ) )
  {
    QTextStream stream( &file );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // Always use UTF-8
    stream.setCodec( "UTF-8" );
#endif
    QString line;
    while ( !stream.atEnd() )
    {
      line = stream.readLine(); // line of text excluding '\n'
      mHistory.append( line );
    }
    syncSoftHistory();
    return true;
  }

  return false;
}

void QgsCodeEditor::syncSoftHistory()
{
  mSoftHistory = mHistory;
  mSoftHistory.append( QString() );
  mSoftHistoryIndex = mSoftHistory.length() - 1;
}

void QgsCodeEditor::updateSoftHistory()
{
  mSoftHistory[mSoftHistoryIndex] = text();
}

void QgsCodeEditor::updateHistory( const QStringList &commands, bool skipSoftHistory )
{
  if ( commands.size() > 1 )
  {
    mHistory.append( commands );
  }
  else if ( !commands.value( 0 ).isEmpty() )
  {
    const QString command = commands.value( 0 );
    if ( mHistory.empty() || command != mHistory.constLast() )
      mHistory.append( command );
  }

  if ( !skipSoftHistory )
    syncSoftHistory();
}

void QgsCodeEditor::populateContextMenu( QMenu * )
{

}

void QgsCodeEditor::updatePrompt()
{
  if ( mInterpreter )
  {
    const QString prompt = mInterpreter->promptForState( mInterpreter->currentState() );
    SendScintilla( QsciScintilla::SCI_MARGINSETTEXT, static_cast< uintptr_t >( 0 ), prompt.toUtf8().constData() );
  }
}

QgsCodeInterpreter *QgsCodeEditor::interpreter() const
{
  return mInterpreter;
}

void QgsCodeEditor::setInterpreter( QgsCodeInterpreter *newInterpreter )
{
  mInterpreter = newInterpreter;
  updatePrompt();
}

QStringList QgsCodeEditor::history() const
{
  return mHistory;
}

void QgsCodeEditor::runCommand( const QString &command )
{
  updateHistory( { command } );

  if ( mInterpreter )
    mInterpreter->exec( command );

  clear();
  moveCursorToEnd();
}

void QgsCodeEditor::clearSessionHistory()
{
  mHistory.clear();
  readHistoryFile();
  syncSoftHistory();

  emit sessionHistoryCleared();
}

void QgsCodeEditor::clearPersistentHistory()
{
  mHistory.clear();

  if ( !mHistoryFilePath.isEmpty() && QFile::exists( mHistoryFilePath ) )
  {
    QFile file( mHistoryFilePath );
    file.open( QFile::WriteOnly | QFile::Truncate );
  }

  emit persistentHistoryCleared();
}

bool QgsCodeEditor::writeHistoryFile()
{
  if ( mHistoryFilePath.isEmpty() )
    return false;

  QFile f( mHistoryFilePath );
  if ( !f.open( QFile::WriteOnly | QIODevice::Truncate ) )
  {
    return false;
  }

  QTextStream ts( &f );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  ts.setCodec( "UTF-8" );
#endif
  for ( const QString &command : std::as_const( mHistory ) )
  {
    ts << command + '\n';
  }
  return true;
}

void QgsCodeEditor::showPreviousCommand()
{
  if ( mSoftHistoryIndex < mSoftHistory.length() - 1 && !mSoftHistory.isEmpty() )
  {
    mSoftHistoryIndex += 1;
    setText( mSoftHistory[mSoftHistoryIndex] );
    moveCursorToEnd();
  }
}

void QgsCodeEditor::showNextCommand()
{
  if ( mSoftHistoryIndex > 0 && !mSoftHistory.empty() )
  {
    mSoftHistoryIndex -= 1;
    setText( mSoftHistory[mSoftHistoryIndex] );
    moveCursorToEnd();
  }
}

void QgsCodeEditor::showHistory()
{
  QgsCodeEditorHistoryDialog *dialog = new QgsCodeEditorHistoryDialog( this, this );
  dialog->setAttribute( Qt::WA_DeleteOnClose );

  dialog->show();
  dialog->activateWindow();
}

void QgsCodeEditor::removeHistoryCommand( int index )
{
  // remove item from the command history (just for the current session)
  mHistory.removeAt( index );
  mSoftHistory.removeAt( index );
  if ( index < mSoftHistoryIndex )
  {
    mSoftHistoryIndex -= 1;
    if ( mSoftHistoryIndex < 0 )
      mSoftHistoryIndex = mSoftHistory.length() - 1;
  }
}

void QgsCodeEditor::insertText( const QString &text )
{
  // Insert the text or replace selected text
  if ( hasSelectedText() )
  {
    replaceSelectedText( text );
  }
  else
  {
    int line, index;
    getCursorPosition( &line, &index );
    insertAt( text, line, index );
    setCursorPosition( line, index + text.length() );
  }
}

QColor QgsCodeEditor::defaultColor( QgsCodeEditorColorScheme::ColorRole role, const QString &theme )
{
  if ( theme.isEmpty() && QgsApplication::themeName() == QLatin1String( "default" ) )
  {
    // if using default theme, take certain colors from the palette
    const QPalette pal = qApp->palette();

    switch ( role )
    {
      case QgsCodeEditorColorScheme::ColorRole::SelectionBackground:
        return pal.color( QPalette::Highlight );
      case QgsCodeEditorColorScheme::ColorRole::SelectionForeground:
        return pal.color( QPalette::HighlightedText );
      default:
        break;
    }
  }
  else if ( theme.isEmpty() )
  {
    // non default theme (e.g. Blend of Gray). Take colors from theme ini file...
    const QSettings ini( QgsApplication::uiThemes().value( QgsApplication::themeName() ) + "/qscintilla.ini", QSettings::IniFormat );

    static const QMap< QgsCodeEditorColorScheme::ColorRole, QString > sColorRoleToIniKey
    {
      {QgsCodeEditorColorScheme::ColorRole::Default, QStringLiteral( "python/defaultFontColor" ) },
      {QgsCodeEditorColorScheme::ColorRole::Keyword, QStringLiteral( "python/keywordFontColor" ) },
      {QgsCodeEditorColorScheme::ColorRole::Class, QStringLiteral( "python/classFontColor" ) },
      {QgsCodeEditorColorScheme::ColorRole::Method, QStringLiteral( "python/methodFontColor" ) },
      {QgsCodeEditorColorScheme::ColorRole::Decoration, QStringLiteral( "python/decoratorFontColor" ) },
      {QgsCodeEditorColorScheme::ColorRole::Number, QStringLiteral( "python/numberFontColor" ) },
      {QgsCodeEditorColorScheme::ColorRole::Comment, QStringLiteral( "python/commentFontColor" ) },
      {QgsCodeEditorColorScheme::ColorRole::CommentLine, QStringLiteral( "sql/commentLineFontColor" ) },
      {QgsCodeEditorColorScheme::ColorRole::CommentBlock, QStringLiteral( "python/commentBlockFontColor" ) },
      {QgsCodeEditorColorScheme::ColorRole::Background, QStringLiteral( "python/paperBackgroundColor" ) },
      {QgsCodeEditorColorScheme::ColorRole::Cursor, QStringLiteral( "cursorColor" ) },
      {QgsCodeEditorColorScheme::ColorRole::CaretLine, QStringLiteral( "caretLineColor" ) },
      {QgsCodeEditorColorScheme::ColorRole::Operator, QStringLiteral( "sql/operatorFontColor" ) },
      {QgsCodeEditorColorScheme::ColorRole::QuotedOperator, QStringLiteral( "sql/QuotedOperatorFontColor" ) },
      {QgsCodeEditorColorScheme::ColorRole::Identifier, QStringLiteral( "sql/identifierFontColor" ) },
      {QgsCodeEditorColorScheme::ColorRole::QuotedIdentifier, QStringLiteral( "sql/QuotedIdentifierFontColor" ) },
      {QgsCodeEditorColorScheme::ColorRole::Tag, QStringLiteral( "html/tagFontColor" ) },
      {QgsCodeEditorColorScheme::ColorRole::UnknownTag, QStringLiteral( "html/unknownTagFontColor" ) },
      {QgsCodeEditorColorScheme::ColorRole::SingleQuote, QStringLiteral( "sql/singleQuoteFontColor" ) },
      {QgsCodeEditorColorScheme::ColorRole::DoubleQuote, QStringLiteral( "sql/doubleQuoteFontColor" ) },
      {QgsCodeEditorColorScheme::ColorRole::TripleSingleQuote, QStringLiteral( "python/tripleSingleQuoteFontColor" ) },
      {QgsCodeEditorColorScheme::ColorRole::TripleDoubleQuote, QStringLiteral( "python/tripleDoubleQuoteFontColor" ) },
      {QgsCodeEditorColorScheme::ColorRole::MarginBackground, QStringLiteral( "marginBackgroundColor" ) },
      {QgsCodeEditorColorScheme::ColorRole::MarginForeground, QStringLiteral( "marginForegroundColor" ) },
      {QgsCodeEditorColorScheme::ColorRole::SelectionBackground, QStringLiteral( "selectionBackgroundColor" ) },
      {QgsCodeEditorColorScheme::ColorRole::SelectionForeground, QStringLiteral( "selectionForegroundColor" ) },
      {QgsCodeEditorColorScheme::ColorRole::MatchedBraceBackground, QStringLiteral( "matchedBraceBackground" ) },
      {QgsCodeEditorColorScheme::ColorRole::MatchedBraceForeground, QStringLiteral( "matchedBraceColor" ) },
      {QgsCodeEditorColorScheme::ColorRole::Edge, QStringLiteral( "edgeColor" ) },
      {QgsCodeEditorColorScheme::ColorRole::Fold, QStringLiteral( "foldColor" ) },
      {QgsCodeEditorColorScheme::ColorRole::Error, QStringLiteral( "stderrFontColor" ) },
      {QgsCodeEditorColorScheme::ColorRole::ErrorBackground, QStringLiteral( "stderrBackground" ) },
      {QgsCodeEditorColorScheme::ColorRole::FoldIconForeground, QStringLiteral( "foldIconForeground" ) },
      {QgsCodeEditorColorScheme::ColorRole::FoldIconHalo, QStringLiteral( "foldIconHalo" ) },
      {QgsCodeEditorColorScheme::ColorRole::IndentationGuide, QStringLiteral( "indentationGuide" ) },
    };

    const QgsCodeEditorColorScheme defaultScheme = QgsGui::codeEditorColorSchemeRegistry()->scheme( QStringLiteral( "default" ) );
    return QgsSymbolLayerUtils::decodeColor( ini.value( sColorRoleToIniKey.value( role ), defaultScheme.color( role ).name() ).toString() );
  }

  const QgsCodeEditorColorScheme scheme = QgsGui::codeEditorColorSchemeRegistry()->scheme( theme.isEmpty() ? QStringLiteral( "default" ) : theme );
  return scheme.color( role );
}

QColor QgsCodeEditor::color( QgsCodeEditorColorScheme::ColorRole role )
{
  const QgsSettings settings;
  if ( !settings.value( QStringLiteral( "codeEditor/overrideColors" ), false, QgsSettings::Gui ).toBool() )
  {
    const QString theme = settings.value( QStringLiteral( "codeEditor/colorScheme" ), QString(), QgsSettings::Gui ).toString();
    return defaultColor( role, theme );
  }
  else
  {
    const QString color = settings.value( QStringLiteral( "codeEditor/%1" ).arg( sColorRoleToSettingsKey.value( role ) ), QString(), QgsSettings::Gui ).toString();
    return color.isEmpty() ? defaultColor( role ) : QgsSymbolLayerUtils::decodeColor( color );
  }
}

void QgsCodeEditor::setColor( QgsCodeEditorColorScheme::ColorRole role, const QColor &color )
{
  QgsSettings settings;
  if ( color.isValid() )
  {
    settings.setValue( QStringLiteral( "codeEditor/%1" ).arg( sColorRoleToSettingsKey.value( role ) ), color.name(), QgsSettings::Gui );
  }
  else
  {
    settings.remove( QStringLiteral( "codeEditor/%1" ).arg( sColorRoleToSettingsKey.value( role ) ), QgsSettings::Gui );
  }
}

// Settings for font and fontsize
bool QgsCodeEditor::isFixedPitch( const QFont &font )
{
  return font.fixedPitch();
}

QFont QgsCodeEditor::getMonospaceFont()
{
  QFont font = QFontDatabase::systemFont( QFontDatabase::FixedFont );

  const QgsSettings settings;
  if ( !settings.value( QStringLiteral( "codeEditor/fontfamily" ), QString(), QgsSettings::Gui ).toString().isEmpty() )
    font.setFamily( settings.value( QStringLiteral( "codeEditor/fontfamily" ), QString(), QgsSettings::Gui ).toString() );

  const int fontSize = settings.value( QStringLiteral( "codeEditor/fontsize" ), 0, QgsSettings::Gui ).toInt();

#ifdef Q_OS_MAC
  if ( fontSize > 0 )
    font.setPointSize( fontSize );
  else
  {
    // The font size gotten from getMonospaceFont() is too small on Mac
    font.setPointSize( QLabel().font().pointSize() );
  }
#else
  if ( fontSize > 0 )
    font.setPointSize( fontSize );
  else
  {
    const int fontSize = settings.value( QStringLiteral( "qgis/stylesheet/fontPointSize" ), 10 ).toInt();
    font.setPointSize( fontSize );
  }
#endif
  font.setBold( false );

  return font;
}

void QgsCodeEditor::setCustomAppearance( const QString &scheme, const QMap<QgsCodeEditorColorScheme::ColorRole, QColor> &customColors, const QString &fontFamily, int fontSize )
{
  mUseDefaultSettings = false;
  mOverrideColors = !customColors.isEmpty();
  mColorScheme = scheme;
  mCustomColors = customColors;
  mFontFamily = fontFamily;
  mFontSize = fontSize;

  setSciWidget();
  initializeLexer();
}

void QgsCodeEditor::addWarning( const int lineNumber, const QString &warning )
{
  setMarginWidth( static_cast< int >( QgsCodeEditor::MarginRole::ErrorIndicators ), "000" );
  markerAdd( lineNumber, MARKER_NUMBER );
  QFont font = lexerFont();
  font.setItalic( true );
  const QsciStyle styleAnn = QsciStyle( -1, QStringLiteral( "Annotation" ),
                                        lexerColor( QgsCodeEditorColorScheme::ColorRole::Error ),
                                        lexerColor( QgsCodeEditorColorScheme::ColorRole::ErrorBackground ),
                                        font,
                                        true );
  annotate( lineNumber, warning, styleAnn );
  mWarningLines.push_back( lineNumber );
}

void QgsCodeEditor::clearWarnings()
{
  for ( const int line : mWarningLines )
  {
    markerDelete( line );
    clearAnnotations( line );
  }
  setMarginWidth( static_cast< int >( QgsCodeEditor::MarginRole::ErrorIndicators ), 0 );
  mWarningLines.clear();
}

bool QgsCodeEditor::isCursorOnLastLine() const
{
  int line = 0;
  int index = 0;
  getCursorPosition( &line, &index );
  return line == lines() - 1;
}

void QgsCodeEditor::setHistoryFilePath( const QString &path )
{
  mHistoryFilePath = path;
  readHistoryFile();
}

void QgsCodeEditor::moveCursorToStart()
{
  setCursorPosition( 0, 0 );
  ensureCursorVisible();
  ensureLineVisible( 0 );

  if ( mMode == QgsCodeEditor::Mode::CommandInput )
    updatePrompt();
}

void QgsCodeEditor::moveCursorToEnd()
{
  const int endLine = lines() - 1;
  const int endLineLength = lineLength( endLine );
  setCursorPosition( endLine, endLineLength );
  ensureCursorVisible();
  ensureLineVisible( endLine );

  if ( mMode == QgsCodeEditor::Mode::CommandInput )
    updatePrompt();
}

QgsCodeInterpreter::~QgsCodeInterpreter() = default;

int QgsCodeInterpreter::exec( const QString &command )
{
  mState = execCommandImpl( command );
  return mState;
}
