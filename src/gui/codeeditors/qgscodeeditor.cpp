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

#include <QLabel>
#include <QWidget>
#include <QFont>
#include <QFontDatabase>
#include <QDebug>
#include <QFocusEvent>
#include <Qsci/qscistyle.h>

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


QgsCodeEditor::QgsCodeEditor( QWidget *parent, const QString &title, bool folding, bool margin )
  : QsciScintilla( parent )
  , mWidgetTitle( title )
  , mFolding( folding )
  , mMargin( margin )
{
  if ( !parent && mWidgetTitle.isEmpty() )
  {
    setWindowTitle( QStringLiteral( "Text Editor" ) );
  }
  else
  {
    setWindowTitle( mWidgetTitle );
  }
  setSciWidget();
  setHorizontalScrollBarPolicy( Qt::ScrollBarAsNeeded );

  SendScintilla( SCI_SETADDITIONALSELECTIONTYPING, 1 );
  SendScintilla( SCI_SETMULTIPASTE, 1 );
  SendScintilla( SCI_SETVIRTUALSPACEOPTIONS, SCVS_RECTANGULARSELECTION );

  SendScintilla( SCI_SETMARGINTYPEN, QgsCodeEditor::MarginRole::ErrorIndicators, SC_MARGIN_SYMBOL );
  SendScintilla( SCI_SETMARGINMASKN, QgsCodeEditor::MarginRole::ErrorIndicators, 1 << MARKER_NUMBER );
  setMarginWidth( QgsCodeEditor::MarginRole::ErrorIndicators, 0 );
  setAnnotationDisplay( QsciScintilla::AnnotationBoxed );

  connect( QgsGui::instance(), &QgsGui::optionsChanged, this, [ = ]
  {
    setSciWidget();
    initializeLexer();
  } );
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
  if ( event->key() == Qt::Key_Escape && !isListActive() )
  {
    // Shortcut QScintilla and redirect the event to the QWidget handler
    QWidget::keyPressEvent( event ); // clazy:exclude=skipped-base-method
  }
  else
  {
    QsciScintilla::keyPressEvent( event );
  }
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
  setMatchedBraceForegroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::MatchedBraceForeground ) );
  setMatchedBraceBackgroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::MatchedBraceBackground ) );

  SendScintilla( SCI_MARKERSETFORE, SC_MARKNUM_FOLDEROPEN, lexerColor( QgsCodeEditorColorScheme::ColorRole::FoldIconHalo ) );
  SendScintilla( SCI_MARKERSETBACK, SC_MARKNUM_FOLDEROPEN,  lexerColor( QgsCodeEditorColorScheme::ColorRole::FoldIconForeground ) );
  SendScintilla( SCI_MARKERSETFORE, SC_MARKNUM_FOLDER, lexerColor( QgsCodeEditorColorScheme::ColorRole::FoldIconHalo ) );
  SendScintilla( SCI_MARKERSETBACK, SC_MARKNUM_FOLDER,  lexerColor( QgsCodeEditorColorScheme::ColorRole::FoldIconForeground ) );
  SendScintilla( SCI_STYLESETFORE, STYLE_INDENTGUIDE, lexerColor( QgsCodeEditorColorScheme::ColorRole::IndentationGuide ) );
  SendScintilla( SCI_STYLESETBACK, STYLE_INDENTGUIDE,  lexerColor( QgsCodeEditorColorScheme::ColorRole::IndentationGuide ) );
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
  setFoldingVisible( false );

  setMarginWidth( QgsCodeEditor::MarginRole::ErrorIndicators, 0 );

  setMarginsForegroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::MarginForeground ) );
  setMarginsBackgroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::MarginBackground ) );
  setIndentationGuidesForegroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::MarginForeground ) );
  setIndentationGuidesBackgroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::MarginBackground ) );
  // whether margin will be shown
  setFoldingVisible( mFolding );
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

void QgsCodeEditor::setMarginVisible( bool margin )
{
  mMargin = margin;
  if ( margin )
  {
    QFont marginFont = lexerFont();
    marginFont.setPointSize( 10 );
    setMarginLineNumbers( 0, true );
    setMarginsFont( marginFont );
    setMarginWidth( QgsCodeEditor::MarginRole::LineNumbers, QStringLiteral( "00000" ) );
    setMarginsForegroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::MarginForeground ) );
    setMarginsBackgroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::MarginBackground ) );
  }
  else
  {
    setMarginWidth( QgsCodeEditor::MarginRole::LineNumbers, 0 );
    setMarginWidth( QgsCodeEditor::MarginRole::ErrorIndicators, 0 );
    setMarginWidth( QgsCodeEditor::MarginRole::FoldingControls, 0 );
  }
}

void QgsCodeEditor::setLineNumbersVisible( bool visible )
{
  if ( visible )
  {
    QFont marginFont = lexerFont();
    marginFont.setPointSize( 10 );
    setMarginLineNumbers( QgsCodeEditor::MarginRole::LineNumbers, true );
    setMarginsFont( marginFont );
    setMarginWidth( QgsCodeEditor::MarginRole::LineNumbers, QStringLiteral( "00000" ) );
    setMarginsForegroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::MarginForeground ) );
    setMarginsBackgroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::MarginBackground ) );
  }
  else
  {
    setMarginLineNumbers( QgsCodeEditor::MarginRole::LineNumbers, false );
    setMarginWidth( QgsCodeEditor::MarginRole::LineNumbers, 0 );
  }
}

bool QgsCodeEditor::lineNumbersVisible() const
{
  return marginLineNumbers( QgsCodeEditor::MarginRole::LineNumbers );
}

void QgsCodeEditor::setFoldingVisible( bool folding )
{
  mFolding = folding;
  if ( folding )
  {
    setMarginWidth( QgsCodeEditor::MarginRole::FoldingControls, "0" );
    setMarginsForegroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::MarginForeground ) );
    setMarginsBackgroundColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::MarginBackground ) );
    setFolding( QsciScintilla::PlainFoldStyle );
  }
  else
  {
    setFolding( QsciScintilla::NoFoldStyle );
    setMarginWidth( QgsCodeEditor::MarginRole::FoldingControls, 0 );
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
  setMarginWidth( QgsCodeEditor::MarginRole::ErrorIndicators, "000" );
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
  setMarginWidth( QgsCodeEditor::MarginRole::ErrorIndicators, 0 );
  mWarningLines.clear();
}
