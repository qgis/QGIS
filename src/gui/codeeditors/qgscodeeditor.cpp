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
#include "moc_qgscodeeditor.cpp"
#include "qgssettings.h"
#include "qgssymbollayerutils.h"
#include "qgsgui.h"
#include "qgscodeeditorcolorschemeregistry.h"
#include "qgscodeeditorhistorydialog.h"
#include "qgsstringutils.h"
#include "qgsfontutils.h"
#include "qgssettingsentryimpl.h"

#include <QLabel>
#include <QWidget>
#include <QFont>
#include <QFontDatabase>
#include <QDebug>
#include <QFocusEvent>
#include <Qsci/qscistyle.h>
#include <QMenu>
#include <QClipboard>
#include <QScrollBar>
#include <QMessageBox>
#include "Qsci/qscilexer.h"

///@cond PRIVATE
const QgsSettingsEntryBool *QgsCodeEditor::settingContextHelpHover = new QgsSettingsEntryBool( QStringLiteral( "context-help-hover" ), sTreeCodeEditor, false, QStringLiteral( "Whether the context help should works on hovered words" ) );
///@endcond PRIVATE


QMap<QgsCodeEditorColorScheme::ColorRole, QString> QgsCodeEditor::sColorRoleToSettingsKey {
  { QgsCodeEditorColorScheme::ColorRole::Default, QStringLiteral( "defaultFontColor" ) },
  { QgsCodeEditorColorScheme::ColorRole::Keyword, QStringLiteral( "keywordFontColor" ) },
  { QgsCodeEditorColorScheme::ColorRole::Class, QStringLiteral( "classFontColor" ) },
  { QgsCodeEditorColorScheme::ColorRole::Method, QStringLiteral( "methodFontColor" ) },
  { QgsCodeEditorColorScheme::ColorRole::Decoration, QStringLiteral( "decoratorFontColor" ) },
  { QgsCodeEditorColorScheme::ColorRole::Number, QStringLiteral( "numberFontColor" ) },
  { QgsCodeEditorColorScheme::ColorRole::Comment, QStringLiteral( "commentFontColor" ) },
  { QgsCodeEditorColorScheme::ColorRole::CommentLine, QStringLiteral( "commentLineFontColor" ) },
  { QgsCodeEditorColorScheme::ColorRole::CommentBlock, QStringLiteral( "commentBlockFontColor" ) },
  { QgsCodeEditorColorScheme::ColorRole::Background, QStringLiteral( "paperBackgroundColor" ) },
  { QgsCodeEditorColorScheme::ColorRole::Cursor, QStringLiteral( "cursorColor" ) },
  { QgsCodeEditorColorScheme::ColorRole::CaretLine, QStringLiteral( "caretLineColor" ) },
  { QgsCodeEditorColorScheme::ColorRole::Operator, QStringLiteral( "operatorFontColor" ) },
  { QgsCodeEditorColorScheme::ColorRole::QuotedOperator, QStringLiteral( "quotedOperatorFontColor" ) },
  { QgsCodeEditorColorScheme::ColorRole::Identifier, QStringLiteral( "identifierFontColor" ) },
  { QgsCodeEditorColorScheme::ColorRole::QuotedIdentifier, QStringLiteral( "quotedIdentifierFontColor" ) },
  { QgsCodeEditorColorScheme::ColorRole::Tag, QStringLiteral( "tagFontColor" ) },
  { QgsCodeEditorColorScheme::ColorRole::UnknownTag, QStringLiteral( "unknownTagFontColor" ) },
  { QgsCodeEditorColorScheme::ColorRole::SingleQuote, QStringLiteral( "singleQuoteFontColor" ) },
  { QgsCodeEditorColorScheme::ColorRole::DoubleQuote, QStringLiteral( "doubleQuoteFontColor" ) },
  { QgsCodeEditorColorScheme::ColorRole::TripleSingleQuote, QStringLiteral( "tripleSingleQuoteFontColor" ) },
  { QgsCodeEditorColorScheme::ColorRole::TripleDoubleQuote, QStringLiteral( "tripleDoubleQuoteFontColor" ) },
  { QgsCodeEditorColorScheme::ColorRole::MarginBackground, QStringLiteral( "marginBackgroundColor" ) },
  { QgsCodeEditorColorScheme::ColorRole::MarginForeground, QStringLiteral( "marginForegroundColor" ) },
  { QgsCodeEditorColorScheme::ColorRole::SelectionBackground, QStringLiteral( "selectionBackgroundColor" ) },
  { QgsCodeEditorColorScheme::ColorRole::SelectionForeground, QStringLiteral( "selectionForegroundColor" ) },
  { QgsCodeEditorColorScheme::ColorRole::MatchedBraceBackground, QStringLiteral( "matchedBraceBackground" ) },
  { QgsCodeEditorColorScheme::ColorRole::MatchedBraceForeground, QStringLiteral( "matchedBraceColor" ) },
  { QgsCodeEditorColorScheme::ColorRole::Edge, QStringLiteral( "edgeColor" ) },
  { QgsCodeEditorColorScheme::ColorRole::Fold, QStringLiteral( "foldColor" ) },
  { QgsCodeEditorColorScheme::ColorRole::Error, QStringLiteral( "stderrFontColor" ) },
  { QgsCodeEditorColorScheme::ColorRole::ErrorBackground, QStringLiteral( "stderrBackgroundColor" ) },
  { QgsCodeEditorColorScheme::ColorRole::FoldIconForeground, QStringLiteral( "foldIconForeground" ) },
  { QgsCodeEditorColorScheme::ColorRole::FoldIconHalo, QStringLiteral( "foldIconHalo" ) },
  { QgsCodeEditorColorScheme::ColorRole::IndentationGuide, QStringLiteral( "indentationGuide" ) },
  { QgsCodeEditorColorScheme::ColorRole::SearchMatchBackground, QStringLiteral( "searchMatchBackground" ) }
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

  SendScintilla( SCI_SETMARGINTYPEN, static_cast<int>( QgsCodeEditor::MarginRole::ErrorIndicators ), SC_MARGIN_SYMBOL );
  SendScintilla( SCI_SETMARGINMASKN, static_cast<int>( QgsCodeEditor::MarginRole::ErrorIndicators ), 1 << MARKER_NUMBER );
  setMarginWidth( static_cast<int>( QgsCodeEditor::MarginRole::ErrorIndicators ), 0 );
  setAnnotationDisplay( QsciScintilla::AnnotationBoxed );

  connect( QgsGui::instance(), &QgsGui::optionsChanged, this, [=] {
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

#if QSCINTILLA_VERSION < 0x020d03
  installEventFilter( this );
#endif

  mLastEditTimer = new QTimer( this );
  mLastEditTimer->setSingleShot( true );
  mLastEditTimer->setInterval( 1000 );
  connect( mLastEditTimer, &QTimer::timeout, this, &QgsCodeEditor::onLastEditTimeout );
  connect( this, &QgsCodeEditor::textChanged, mLastEditTimer, qOverload<>( &QTimer::start ) );
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
  onLastEditTimeout();
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
    QWidget::keyPressEvent( event ); // NOLINT(bugprone-parent-virtual-call) clazy:exclude=skipped-base-method
    return;
  }

  if ( event->key() == Qt::Key_F1 )
  {
    // Check if some text is selected
    QString text = selectedText();

    // Check if mouse is hovering over a word
    if ( text.isEmpty() && settingContextHelpHover->value() )
    {
      text = wordAtPoint( mapFromGlobal( QCursor::pos() ) );
    }

    // Otherwise, check if there is a word at the current text cursor position
    if ( text.isEmpty() )
    {
      int line, index;
      getCursorPosition( &line, &index );
      text = wordAtLineIndex( line, index );
    }
    emit helpRequested( text );
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

  const bool ctrlModifier = event->modifiers() & Qt::ControlModifier;
  const bool altModifier = event->modifiers() & Qt::AltModifier;

  // Ctrl+Alt+F: reformat code
  const bool canReformat = languageCapabilities() & Qgis::ScriptLanguageCapability::Reformat;
  if ( !isReadOnly() && canReformat && ctrlModifier && altModifier && event->key() == Qt::Key_F )
  {
    event->accept();
    reformatCode();
    return;
  }

  // Toggle comment when user presses  Ctrl+:
  const bool canToggle = languageCapabilities() & Qgis::ScriptLanguageCapability::ToggleComment;
  if ( !isReadOnly() && canToggle && ctrlModifier && event->key() == Qt::Key_Colon )
  {
    event->accept();
    toggleComment();
    return;
  }

  QsciScintilla::keyPressEvent( event );

  // Update calltips unless event is autorepeat
  if ( !event->isAutoRepeat() )
  {
    callTip();
  }
}

void QgsCodeEditor::contextMenuEvent( QContextMenuEvent *event )
{
  switch ( mMode )
  {
    case Mode::ScriptEditor:
    {
      QMenu *menu = createStandardContextMenu();
      menu->setAttribute( Qt::WA_DeleteOnClose );

      if ( ( languageCapabilities() & Qgis::ScriptLanguageCapability::Reformat ) || ( languageCapabilities() & Qgis::ScriptLanguageCapability::CheckSyntax ) )
      {
        menu->addSeparator();
      }

      if ( languageCapabilities() & Qgis::ScriptLanguageCapability::Reformat )
      {
        QAction *reformatAction = new QAction( tr( "Reformat Code" ), menu );
        reformatAction->setShortcut( QStringLiteral( "Ctrl+Alt+F" ) );
        reformatAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "console/iconFormatCode.svg" ) ) );
        reformatAction->setEnabled( !isReadOnly() );
        connect( reformatAction, &QAction::triggered, this, &QgsCodeEditor::reformatCode );
        menu->addAction( reformatAction );
      }

      if ( languageCapabilities() & Qgis::ScriptLanguageCapability::CheckSyntax )
      {
        QAction *syntaxCheckAction = new QAction( tr( "Check Syntax" ), menu );
        syntaxCheckAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "console/iconSyntaxErrorConsole.svg" ) ) );
        connect( syntaxCheckAction, &QAction::triggered, this, &QgsCodeEditor::checkSyntax );
        menu->addAction( syntaxCheckAction );
      }

      if ( languageCapabilities() & Qgis::ScriptLanguageCapability::ToggleComment )
      {
        QAction *toggleCommentAction = new QAction( tr( "Toggle Comment" ), menu );
        toggleCommentAction->setShortcut( QStringLiteral( "Ctrl+:" ) );
        toggleCommentAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "console/iconCommentEditorConsole.svg" ) ) );
        toggleCommentAction->setEnabled( !isReadOnly() );
        connect( toggleCommentAction, &QAction::triggered, this, &QgsCodeEditor::toggleComment );
        menu->addAction( toggleCommentAction );
      }

      populateContextMenu( menu );

      menu->exec( mapToGlobal( event->pos() ) );
      break;
    }

    case Mode::CommandInput:
    {
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
      break;
    }

    case Mode::OutputDisplay:
      QsciScintilla::contextMenuEvent( event );
      break;
  }
}


bool QgsCodeEditor::eventFilter( QObject *watched, QEvent *event )
{
#if QSCINTILLA_VERSION < 0x020d03
  if ( watched == this && event->type() == QEvent::InputMethod )
  {
    // swallow input method events, which cause loss of selected text.
    // See https://sourceforge.net/p/scintilla/bugs/1913/ , which was ported to QScintilla
    // in version 2.13.3
    return true;
  }
#endif

  return QsciScintilla::eventFilter( watched, event );
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
    QgsFontUtils::setFontFamily( font, mFontFamily );

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
  SendScintilla( SCI_MARKERSETBACK, SC_MARKNUM_FOLDEROPEN, lexerColor( QgsCodeEditorColorScheme::ColorRole::FoldIconForeground ) );
  SendScintilla( SCI_MARKERSETFORE, SC_MARKNUM_FOLDER, lexerColor( QgsCodeEditorColorScheme::ColorRole::FoldIconHalo ) );
  SendScintilla( SCI_MARKERSETBACK, SC_MARKNUM_FOLDER, lexerColor( QgsCodeEditorColorScheme::ColorRole::FoldIconForeground ) );
  SendScintilla( SCI_STYLESETFORE, STYLE_INDENTGUIDE, lexerColor( QgsCodeEditorColorScheme::ColorRole::IndentationGuide ) );
  SendScintilla( SCI_STYLESETBACK, STYLE_INDENTGUIDE, lexerColor( QgsCodeEditorColorScheme::ColorRole::IndentationGuide ) );

  SendScintilla( QsciScintilla::SCI_INDICSETSTYLE, SEARCH_RESULT_INDICATOR, QsciScintilla::INDIC_STRAIGHTBOX );
  SendScintilla( QsciScintilla::SCI_INDICSETFORE, SEARCH_RESULT_INDICATOR, lexerColor( QgsCodeEditorColorScheme::ColorRole::SearchMatchBackground ) );
  SendScintilla( QsciScintilla::SCI_INDICSETALPHA, SEARCH_RESULT_INDICATOR, 100 );
  SendScintilla( QsciScintilla::SCI_INDICSETUNDER, SEARCH_RESULT_INDICATOR, true );
  SendScintilla( QsciScintilla::SCI_INDICGETOUTLINEALPHA, SEARCH_RESULT_INDICATOR, 255 );

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

void QgsCodeEditor::onLastEditTimeout()
{
  mLastEditTimer->stop();
  emit editingTimeout();
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
  setMarginWidth( static_cast<int>( QgsCodeEditor::MarginRole::FoldingControls ), 0 );

  setMarginWidth( static_cast<int>( QgsCodeEditor::MarginRole::ErrorIndicators ), 0 );

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

  markerDefine( QgsApplication::getThemePixmap( "console/iconSyntaxErrorConsoleParams.svg", lexerColor( QgsCodeEditorColorScheme::ColorRole::Error ), lexerColor( QgsCodeEditorColorScheme::ColorRole::ErrorBackground ), 16 ), MARKER_NUMBER );
}

void QgsCodeEditor::setTitle( const QString &title )
{
  setWindowTitle( title );
}

Qgis::ScriptLanguage QgsCodeEditor::language() const
{
  return Qgis::ScriptLanguage::Unknown;
}

Qgis::ScriptLanguageCapabilities QgsCodeEditor::languageCapabilities() const
{
  return Qgis::ScriptLanguageCapabilities();
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
    case Qgis::ScriptLanguage::Batch:
      return tr( "Batch" );
    case Qgis::ScriptLanguage::Bash:
      return tr( "Bash" );
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
    setMarginWidth( static_cast<int>( QgsCodeEditor::MarginRole::LineNumbers ), QStringLiteral( "00000" ) );
    setMarginsForegroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::MarginForeground ) );
    setMarginsBackgroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::MarginBackground ) );
  }
  else
  {
    setMarginWidth( static_cast<int>( QgsCodeEditor::MarginRole::LineNumbers ), 0 );
    setMarginWidth( static_cast<int>( QgsCodeEditor::MarginRole::ErrorIndicators ), 0 );
    setMarginWidth( static_cast<int>( QgsCodeEditor::MarginRole::FoldingControls ), 0 );
  }
}

void QgsCodeEditor::setLineNumbersVisible( bool visible )
{
  if ( visible )
  {
    QFont marginFont = lexerFont();
    marginFont.setPointSize( 10 );
    setMarginLineNumbers( static_cast<int>( QgsCodeEditor::MarginRole::LineNumbers ), true );
    setMarginsFont( marginFont );
    setMarginWidth( static_cast<int>( QgsCodeEditor::MarginRole::LineNumbers ), QStringLiteral( "00000" ) );
    setMarginsForegroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::MarginForeground ) );
    setMarginsBackgroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::MarginBackground ) );
  }
  else
  {
    setMarginLineNumbers( static_cast<int>( QgsCodeEditor::MarginRole::LineNumbers ), false );
    setMarginWidth( static_cast<int>( QgsCodeEditor::MarginRole::LineNumbers ), 0 );
  }
}

bool QgsCodeEditor::lineNumbersVisible() const
{
  return marginLineNumbers( static_cast<int>( QgsCodeEditor::MarginRole::LineNumbers ) );
}

void QgsCodeEditor::setFoldingVisible( bool folding )
{
  if ( folding )
  {
    mFlags |= QgsCodeEditor::Flag::CodeFolding;
  }
  else
  {
    mFlags &= ~( static_cast<int>( QgsCodeEditor::Flag::CodeFolding ) );
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
    setMarginWidth( static_cast<int>( QgsCodeEditor::MarginRole::FoldingControls ), "0" );
    setMarginsForegroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::MarginForeground ) );
    setMarginsBackgroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::MarginBackground ) );
    setFolding( QsciScintilla::PlainFoldStyle );
  }
  else
  {
    setFolding( QsciScintilla::NoFoldStyle );
    setMarginWidth( static_cast<int>( QgsCodeEditor::MarginRole::FoldingControls ), 0 );
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
#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
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

QString QgsCodeEditor::reformatCodeString( const QString &string )
{
  return string;
}

void QgsCodeEditor::showMessage( const QString &title, const QString &message, Qgis::MessageLevel level )
{
  switch ( level )
  {
    case Qgis::Info:
    case Qgis::Success:
    case Qgis::NoLevel:
      QMessageBox::information( this, title, message );
      break;

    case Qgis::Warning:
      QMessageBox::warning( this, title, message );
      break;

    case Qgis::Critical:
      QMessageBox::critical( this, title, message );
      break;
  }
}

void QgsCodeEditor::updatePrompt()
{
  if ( mInterpreter )
  {
    const QString prompt = mInterpreter->promptForState( mInterpreter->currentState() );
    SendScintilla( QsciScintilla::SCI_MARGINSETTEXT, static_cast<uintptr_t>( 0 ), prompt.toUtf8().constData() );
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

// Find the source substring index that most closely matches the target string
int findMinimalDistanceIndex( const QString &source, const QString &target )
{
  const int index = std::min( source.length(), target.length() );

  const int d0 = QgsStringUtils::levenshteinDistance( source.left( index ), target );
  if ( d0 == 0 )
    return index;

  int refDistanceMore = d0;
  int refIndexMore = index;
  if ( index < source.length() - 1 )
  {
    while ( true )
    {
      const int newDistance = QgsStringUtils::levenshteinDistance( source.left( refIndexMore + 1 ), target );
      if ( newDistance <= refDistanceMore )
      {
        refDistanceMore = newDistance;
        refIndexMore++;
        if ( refIndexMore == source.length() - 1 )
          break;
      }
      else
      {
        break;
      }
    }
  }

  int refDistanceLess = d0;
  int refIndexLess = index;
  if ( index > 0 )
  {
    while ( true )
    {
      const int newDistance = QgsStringUtils::levenshteinDistance( source.left( refIndexLess - 1 ), target );
      if ( newDistance <= refDistanceLess )
      {
        refDistanceLess = newDistance;
        refIndexLess--;
        if ( refIndexLess == 0 )
          break;
      }
      else
      {
        break;
      }
    }
  }

  if ( refDistanceMore < refDistanceLess )
    return refIndexMore;
  else
    return refIndexLess;
}

void QgsCodeEditor::reformatCode()
{
  if ( !( languageCapabilities() & Qgis::ScriptLanguageCapability::Reformat ) )
    return;

  const QString textBeforeCursor = text( 0, linearPosition() );
  const QString originalText = text();
  const QString newText = reformatCodeString( originalText );

  if ( originalText == newText )
    return;

  // try to preserve the cursor position and scroll position
  const int oldScrollValue = verticalScrollBar()->value();
  const int linearIndex = findMinimalDistanceIndex( newText, textBeforeCursor );

  beginUndoAction();
  selectAll();
  removeSelectedText();
  insert( newText );
  setLinearPosition( linearIndex );
  verticalScrollBar()->setValue( oldScrollValue );
  endUndoAction();
}

bool QgsCodeEditor::checkSyntax()
{
  return true;
}

void QgsCodeEditor::toggleComment()
{
}

void QgsCodeEditor::adjustScrollWidth()
{
  // A zero width would make setScrollWidth crash
  long maxWidth = 10;

  // Get the number of lines
  int lineCount = lines();

  // Loop through all the lines to get the longest one
  for ( int line = 0; line < lineCount; line++ )
  {
    // Get the linear position at the end of the current line
    const long endLine = SendScintilla( SCI_GETLINEENDPOSITION, line );
    // Get the x coordinates of the end of the line
    const long x = SendScintilla( SCI_POINTXFROMPOSITION, 0, endLine );
    maxWidth = std::max( maxWidth, x );
  }

  // Use the longest line width as the new scroll width
  setScrollWidth( static_cast<int>( maxWidth ) );
}

void QgsCodeEditor::setText( const QString &text )
{
  disconnect( this, &QgsCodeEditor::textChanged, mLastEditTimer, qOverload<>( &QTimer::start ) );
  QsciScintilla::setText( text );
  connect( this, &QgsCodeEditor::textChanged, mLastEditTimer, qOverload<>( &QTimer::start ) );
  onLastEditTimeout();
  adjustScrollWidth();
}

int QgsCodeEditor::editingTimeoutInterval() const
{
  return mLastEditTimer->interval();
}

void QgsCodeEditor::setEditingTimeoutInterval( int timeout )
{
  mLastEditTimer->setInterval( timeout );
}


QStringList QgsCodeEditor::history() const
{
  return mHistory;
}

void QgsCodeEditor::runCommand( const QString &command, bool skipHistory )
{
  if ( !skipHistory )
  {
    updateHistory( { command } );
    if ( mFlags & QgsCodeEditor::Flag::ImmediatelyUpdateHistory )
      writeHistoryFile();
  }

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
#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
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

    static const QMap<QgsCodeEditorColorScheme::ColorRole, QString> sColorRoleToIniKey {
      { QgsCodeEditorColorScheme::ColorRole::Default, QStringLiteral( "python/defaultFontColor" ) },
      { QgsCodeEditorColorScheme::ColorRole::Keyword, QStringLiteral( "python/keywordFontColor" ) },
      { QgsCodeEditorColorScheme::ColorRole::Class, QStringLiteral( "python/classFontColor" ) },
      { QgsCodeEditorColorScheme::ColorRole::Method, QStringLiteral( "python/methodFontColor" ) },
      { QgsCodeEditorColorScheme::ColorRole::Decoration, QStringLiteral( "python/decoratorFontColor" ) },
      { QgsCodeEditorColorScheme::ColorRole::Number, QStringLiteral( "python/numberFontColor" ) },
      { QgsCodeEditorColorScheme::ColorRole::Comment, QStringLiteral( "python/commentFontColor" ) },
      { QgsCodeEditorColorScheme::ColorRole::CommentLine, QStringLiteral( "sql/commentLineFontColor" ) },
      { QgsCodeEditorColorScheme::ColorRole::CommentBlock, QStringLiteral( "python/commentBlockFontColor" ) },
      { QgsCodeEditorColorScheme::ColorRole::Background, QStringLiteral( "python/paperBackgroundColor" ) },
      { QgsCodeEditorColorScheme::ColorRole::Cursor, QStringLiteral( "cursorColor" ) },
      { QgsCodeEditorColorScheme::ColorRole::CaretLine, QStringLiteral( "caretLineColor" ) },
      { QgsCodeEditorColorScheme::ColorRole::Operator, QStringLiteral( "sql/operatorFontColor" ) },
      { QgsCodeEditorColorScheme::ColorRole::QuotedOperator, QStringLiteral( "sql/QuotedOperatorFontColor" ) },
      { QgsCodeEditorColorScheme::ColorRole::Identifier, QStringLiteral( "sql/identifierFontColor" ) },
      { QgsCodeEditorColorScheme::ColorRole::QuotedIdentifier, QStringLiteral( "sql/QuotedIdentifierFontColor" ) },
      { QgsCodeEditorColorScheme::ColorRole::Tag, QStringLiteral( "html/tagFontColor" ) },
      { QgsCodeEditorColorScheme::ColorRole::UnknownTag, QStringLiteral( "html/unknownTagFontColor" ) },
      { QgsCodeEditorColorScheme::ColorRole::SingleQuote, QStringLiteral( "sql/singleQuoteFontColor" ) },
      { QgsCodeEditorColorScheme::ColorRole::DoubleQuote, QStringLiteral( "sql/doubleQuoteFontColor" ) },
      { QgsCodeEditorColorScheme::ColorRole::TripleSingleQuote, QStringLiteral( "python/tripleSingleQuoteFontColor" ) },
      { QgsCodeEditorColorScheme::ColorRole::TripleDoubleQuote, QStringLiteral( "python/tripleDoubleQuoteFontColor" ) },
      { QgsCodeEditorColorScheme::ColorRole::MarginBackground, QStringLiteral( "marginBackgroundColor" ) },
      { QgsCodeEditorColorScheme::ColorRole::MarginForeground, QStringLiteral( "marginForegroundColor" ) },
      { QgsCodeEditorColorScheme::ColorRole::SelectionBackground, QStringLiteral( "selectionBackgroundColor" ) },
      { QgsCodeEditorColorScheme::ColorRole::SelectionForeground, QStringLiteral( "selectionForegroundColor" ) },
      { QgsCodeEditorColorScheme::ColorRole::MatchedBraceBackground, QStringLiteral( "matchedBraceBackground" ) },
      { QgsCodeEditorColorScheme::ColorRole::MatchedBraceForeground, QStringLiteral( "matchedBraceColor" ) },
      { QgsCodeEditorColorScheme::ColorRole::Edge, QStringLiteral( "edgeColor" ) },
      { QgsCodeEditorColorScheme::ColorRole::Fold, QStringLiteral( "foldColor" ) },
      { QgsCodeEditorColorScheme::ColorRole::Error, QStringLiteral( "stderrFontColor" ) },
      { QgsCodeEditorColorScheme::ColorRole::ErrorBackground, QStringLiteral( "stderrBackground" ) },
      { QgsCodeEditorColorScheme::ColorRole::FoldIconForeground, QStringLiteral( "foldIconForeground" ) },
      { QgsCodeEditorColorScheme::ColorRole::FoldIconHalo, QStringLiteral( "foldIconHalo" ) },
      { QgsCodeEditorColorScheme::ColorRole::IndentationGuide, QStringLiteral( "indentationGuide" ) },
      { QgsCodeEditorColorScheme::ColorRole::SearchMatchBackground, QStringLiteral( "searchMatchBackground" ) },
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
    QgsFontUtils::setFontFamily( font, settings.value( QStringLiteral( "codeEditor/fontfamily" ), QString(), QgsSettings::Gui ).toString() );

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
  setMarginWidth( static_cast<int>( QgsCodeEditor::MarginRole::ErrorIndicators ), "000" );
  markerAdd( lineNumber, MARKER_NUMBER );
  QFont font = lexerFont();
  font.setItalic( true );
  const QsciStyle styleAnn = QsciStyle( -1, QStringLiteral( "Annotation" ), lexerColor( QgsCodeEditorColorScheme::ColorRole::Error ), lexerColor( QgsCodeEditorColorScheme::ColorRole::ErrorBackground ), font, true );
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
  setMarginWidth( static_cast<int>( QgsCodeEditor::MarginRole::ErrorIndicators ), 0 );
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

int QgsCodeEditor::linearPosition() const
{
  return static_cast<int>( SendScintilla( SCI_GETCURRENTPOS ) );
}

void QgsCodeEditor::setLinearPosition( int linearIndex )
{
  int line, index;
  lineIndexFromPosition( linearIndex, &line, &index );
  setCursorPosition( line, index );
}

int QgsCodeEditor::selectionStart() const
{
  int startLine, startIndex, _;
  getSelection( &startLine, &startIndex, &_, &_ );
  if ( startLine == -1 )
  {
    return linearPosition();
  }
  return positionFromLineIndex( startLine, startIndex );
}

int QgsCodeEditor::selectionEnd() const
{
  int endLine, endIndex, _;
  getSelection( &_, &_, &endLine, &endIndex );
  if ( endLine == -1 )
  {
    return linearPosition();
  }
  return positionFromLineIndex( endLine, endIndex );
}

void QgsCodeEditor::setLinearSelection( int start, int end )
{
  int startLine, startIndex, endLine, endIndex;
  lineIndexFromPosition( start, &startLine, &startIndex );
  lineIndexFromPosition( end, &endLine, &endIndex );
  setSelection( startLine, startIndex, endLine, endIndex );
}

QgsCodeInterpreter::~QgsCodeInterpreter() = default;

int QgsCodeInterpreter::exec( const QString &command )
{
  mState = execCommandImpl( command );
  return mState;
}


int QgsCodeEditor::wrapPosition( int line )
{
  // If wrapping is disabled, return -1
  if ( wrapMode() == WrapNone )
  {
    return -1;
  }
  // Get the current line
  if ( line == -1 )
  {
    int _index;
    lineIndexFromPosition( linearPosition(), &line, &_index );
  }

  // If line isn't wrapped, return -1
  if ( SendScintilla( SCI_WRAPCOUNT, line ) <= 1 )
  {
    return -1;
  }

  // Get the linear position at the end of the current line
  const long endLine = SendScintilla( SCI_GETLINEENDPOSITION, line );
  // Get the y coordinates of the start of the last wrapped line
  const long y = SendScintilla( SCI_POINTYFROMPOSITION, 0, endLine );
  // Return the linear position of the start of the last wrapped line
  return static_cast<int>( SendScintilla( SCI_POSITIONFROMPOINT, 0, y ) );
}


// Adapted from QsciScintilla source code (qsciscintilla.cpp) to handle line wrap
void QgsCodeEditor::callTip()
{
  if ( callTipsStyle() == CallTipsNone || lexer() == nullptr )
  {
    return;
  }

  QsciAbstractAPIs *apis = lexer()->apis();

  if ( !apis )
    return;

  int pos, commas = 0;
  bool found = false;
  char ch;

  pos = linearPosition();

  // Move backwards through the line looking for the start of the current
  // call tip and working out which argument it is.
  while ( ( ch = getCharacter( pos ) ) != '\0' )
  {
    if ( ch == ',' )
      ++commas;
    else if ( ch == ')' )
    {
      int depth = 1;

      // Ignore everything back to the start of the corresponding
      // parenthesis.
      while ( ( ch = getCharacter( pos ) ) != '\0' )
      {
        if ( ch == ')' )
          ++depth;
        else if ( ch == '(' && --depth == 0 )
          break;
      }
    }
    else if ( ch == '(' )
    {
      found = true;
      break;
    }
  }

  // Cancel any existing call tip.
  SendScintilla( SCI_CALLTIPCANCEL );

  // Done if there is no new call tip to set.
  if ( !found )
    return;

  int contextStart, lastWordStart;
  QStringList context = apiContext( pos, contextStart, lastWordStart );

  if ( context.isEmpty() )
    return;

  // The last word is complete, not partial.
  context << QString();

  QList<int> ctShifts;
  QStringList ctEntries = apis->callTips( context, commas, callTipsStyle(), ctShifts );

  int nbEntries = ctEntries.count();

  if ( nbEntries == 0 )
    return;

  const int maxNumberOfCallTips = callTipsVisible();

  // Clip to at most maxNumberOfCallTips entries.
  if ( maxNumberOfCallTips > 0 && maxNumberOfCallTips < nbEntries )
  {
    ctEntries = ctEntries.mid( 0, maxNumberOfCallTips );
    nbEntries = maxNumberOfCallTips;
  }

  int shift;
  QString ct;

  int nbShifts = ctShifts.count();

  if ( maxNumberOfCallTips < 0 && nbEntries > 1 )
  {
    shift = ( nbShifts > 0 ? ctShifts.first() : 0 );
    ct = ctEntries[0];
    ct.prepend( '\002' );
  }
  else
  {
    if ( nbShifts > nbEntries )
      nbShifts = nbEntries;

    // Find the biggest shift.
    shift = 0;

    for ( int i = 0; i < nbShifts; ++i )
    {
      int sh = ctShifts[i];

      if ( shift < sh )
        shift = sh;
    }

    ct = ctEntries.join( "\n" );
  }

  QByteArray ctBa = ct.toLatin1();
  const char *cts = ctBa.data();

  const int currentWrapPosition = wrapPosition();

  if ( currentWrapPosition != -1 )
  {
    SendScintilla( SCI_CALLTIPSHOW, currentWrapPosition, cts );
  }
  else
  {
    // Shift the position of the call tip (to take any context into account) but
    // don't go before the start of the line.
    if ( shift )
    {
      int ctmin = static_cast<int>( SendScintilla( SCI_POSITIONFROMLINE, SendScintilla( SCI_LINEFROMPOSITION, ct ) ) );
      if ( lastWordStart - shift < ctmin )
        lastWordStart = ctmin;
    }

    int line, index;
    lineIndexFromPosition( lastWordStart, &line, &index );
    SendScintilla( SCI_CALLTIPSHOW, positionFromLineIndex( line, index ), cts );
  }

  // Done if there is more than one call tip.
  if ( nbEntries > 1 )
    return;

  // Highlight the current argument.
  const char *astart;

  if ( commas == 0 )
    astart = strchr( cts, '(' );
  else
    for ( astart = strchr( cts, ',' ); astart && --commas > 0; astart = strchr( astart + 1, ',' ) )
      ;

  if ( !astart )
    return;

  astart++;
  if ( !*astart )
    return;

  // The end is at the next comma or unmatched closing parenthesis.
  const char *aend;
  int depth = 0;

  for ( aend = astart; *aend; ++aend )
  {
    char ch = *aend;

    if ( ch == ',' && depth == 0 )
      break;
    else if ( ch == '(' )
      ++depth;
    else if ( ch == ')' )
    {
      if ( depth == 0 )
        break;

      --depth;
    }
  }

  if ( astart != aend )
    SendScintilla( SCI_CALLTIPSETHLT, astart - cts, aend - cts );
}


// Duplicated from QsciScintilla source code (qsciscintilla.cpp)
// Get the "next" character (ie. the one before the current position) in the
// current line.  The character will be '\0' if there are no more.
char QgsCodeEditor::getCharacter( int &pos ) const
{
  if ( pos <= 0 )
    return '\0';

  char ch = static_cast<char>( SendScintilla( SCI_GETCHARAT, --pos ) );

  // Don't go past the end of the previous line.
  if ( ch == '\n' || ch == '\r' )
  {
    ++pos;
    return '\0';
  }

  return ch;
}
