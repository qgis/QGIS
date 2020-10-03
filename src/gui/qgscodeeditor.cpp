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

#include <QLabel>
#include <QWidget>
#include <QFont>
#include <QFontDatabase>
#include <QDebug>
#include <QFocusEvent>

QMap< QgsCodeEditor::ColorRole, QString > QgsCodeEditor::sColorRoleToSettingsKey
{
  {ColorRole::Default, QStringLiteral( "defaultFontColor" ) },
  {ColorRole::Keyword, QStringLiteral( "keywordFontColor" ) },
  {ColorRole::Class, QStringLiteral( "classFontColor" ) },
  {ColorRole::Method, QStringLiteral( "methodFontColor" ) },
  {ColorRole::Decoration, QStringLiteral( "decoratorFontColor" ) },
  {ColorRole::Number, QStringLiteral( "numberFontColor" ) },
  {ColorRole::Comment, QStringLiteral( "commentFontColor" ) },
  {ColorRole::CommentLine, QStringLiteral( "commentLineFontColor" ) },
  {ColorRole::CommentBlock, QStringLiteral( "commentBlockFontColor" ) },
  {ColorRole::Background, QStringLiteral( "paperBackgroundColor" ) },
  {ColorRole::Cursor, QStringLiteral( "cursorColor" ) },
  {ColorRole::CaretLine, QStringLiteral( "caretLineColor" ) },
  {ColorRole::Operator, QStringLiteral( "operatorFontColor" ) },
  {ColorRole::QuotedOperator, QStringLiteral( "quotedOperatorFontColor" ) },
  {ColorRole::Identifier, QStringLiteral( "identifierFontColor" ) },
  {ColorRole::QuotedIdentifier, QStringLiteral( "quotedIdentifierFontColor" ) },
  {ColorRole::Tag, QStringLiteral( "tagFontColor" ) },
  {ColorRole::UnknownTag, QStringLiteral( "unknownTagFontColor" ) },
  {ColorRole::SingleQuote, QStringLiteral( "singleQuoteFontColor" ) },
  {ColorRole::DoubleQuote, QStringLiteral( "doubleQuoteFontColor" ) },
  {ColorRole::TripleSingleQuote, QStringLiteral( "tripleSingleQuoteFontColor" ) },
  {ColorRole::TripleDoubleQuote, QStringLiteral( "tripleDoubleQuoteFontColor" ) },
  {ColorRole::MarginBackground, QStringLiteral( "marginBackgroundColor" ) },
  {ColorRole::MarginForeground, QStringLiteral( "marginForegroundColor" ) },
  {ColorRole::SelectionBackground, QStringLiteral( "selectionBackgroundColor" ) },
  {ColorRole::SelectionForeground, QStringLiteral( "selectionForegroundColor" ) },
  {ColorRole::MatchedBraceBackground, QStringLiteral( "matchedBraceBackground" ) },
  {ColorRole::MatchedBraceForeground, QStringLiteral( "matchedBraceColor" ) },
  {ColorRole::Edge, QStringLiteral( "edgeColor" ) },
  {ColorRole::Fold, QStringLiteral( "foldColor" ) },
  {ColorRole::Error, QStringLiteral( "stderrFontColor" ) },
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

void QgsCodeEditor::setSciWidget()
{
  QFont font = getMonospaceFont();
  setFont( font );

  setUtf8( true );
  setCaretLineVisible( true );
  setCaretLineBackgroundColor( color( ColorRole::CaretLine ) );
  setCaretForegroundColor( color( ColorRole::Cursor ) );
  setSelectionForegroundColor( color( ColorRole::SelectionForeground ) );
  setSelectionBackgroundColor( color( ColorRole::SelectionBackground ) );

  setBraceMatching( QsciScintilla::SloppyBraceMatch );
  setMatchedBraceForegroundColor( color( ColorRole::MatchedBraceForeground ) );
  setMatchedBraceBackgroundColor( color( ColorRole::MatchedBraceBackground ) );
  // whether margin will be shown
  setMarginVisible( mMargin );
  setMarginsForegroundColor( color( ColorRole::MarginForeground ) );
  setMarginsBackgroundColor( color( ColorRole::MarginBackground ) );
  setIndentationGuidesForegroundColor( color( ColorRole::MarginForeground ) );
  setIndentationGuidesBackgroundColor( color( ColorRole::MarginBackground ) );
  // whether margin will be shown
  setFoldingVisible( mFolding );
  QColor foldColor = color( ColorRole::Fold );
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
    QFont marginFont = getMonospaceFont();
    marginFont.setPointSize( 10 );
    setMarginLineNumbers( 1, true );
    setMarginsFont( marginFont );
    setMarginWidth( 1, QStringLiteral( "00000" ) );
  }
  else
  {
    setMarginWidth( 0, 0 );
    setMarginWidth( 1, 0 );
    setMarginWidth( 2, 0 );
  }
}

void QgsCodeEditor::setFoldingVisible( bool folding )
{
  mFolding = folding;
  if ( folding )
  {
    setFolding( QsciScintilla::PlainFoldStyle );
  }
  else
  {
    setFolding( QsciScintilla::NoFoldStyle );
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

QColor QgsCodeEditor::defaultColor( QgsCodeEditor::ColorRole role, const QString &theme )
{
  static QMap< QString, QMap< ColorRole, QColor > > sColors
  {
    {
      QString(),
      {
        {ColorRole::Default, QColor( "#4d4d4c" ) },
        {ColorRole::Keyword, QColor( "#8959a8" ) },
        {ColorRole::Class, QColor( "#4271ae" ) },
        {ColorRole::Method, QColor( "#4271ae" ) },
        {ColorRole::Decoration, QColor( "#3e999f" ) },
        {ColorRole::Number, QColor( "#c82829" ) },
        {ColorRole::Comment, QColor( "#8e908c" ) },
        {ColorRole::CommentLine, QColor( "#8e908c" ) },
        {ColorRole::CommentBlock, QColor( "#8e908c" ) },
        {ColorRole::Background, QColor( "#ffffff" ) },
        {ColorRole::Operator, QColor( "#8959a8" ) },
        {ColorRole::QuotedOperator, QColor( "#8959a8" ) },
        {ColorRole::Identifier, QColor( "#4271ae" ) },
        {ColorRole::QuotedIdentifier, QColor( "#4271ae" ) },
        {ColorRole::Tag, QColor( "#4271ae" ) },
        {ColorRole::UnknownTag, QColor( "#4271ae" ) },
        {ColorRole::Cursor, QColor( "#636363" ) },
        {ColorRole::CaretLine, QColor( "#efefef" ) },
        {ColorRole::SingleQuote, QColor( "#718c00" ) },
        {ColorRole::DoubleQuote, QColor( "#718c00" ) },
        {ColorRole::TripleSingleQuote, QColor( "#eab700" ) },
        {ColorRole::TripleDoubleQuote, QColor( "#eab700" ) },
        {ColorRole::MarginBackground, QColor( "#efefef" ) },
        {ColorRole::MarginForeground, QColor( "#636363" ) },
        {ColorRole::SelectionBackground, QColor( "#d7d7d7" ) },
        {ColorRole::SelectionForeground, QColor( "#303030" ) },
        {ColorRole::MatchedBraceBackground, QColor( "#b7f907" ) },
        {ColorRole::MatchedBraceForeground, QColor( "#303030" ) },
        {ColorRole::Edge, QColor( "#efefef" ) },
        {ColorRole::Fold, QColor( "#efefef" ) },
        {ColorRole::Error, QColor( "#e31a1c" ) },
      },
    },
    {
      QStringLiteral( "solarized" ),
      {
        {ColorRole::Default, QColor( "#627c84" ) },
        {ColorRole::Keyword, QColor( "#b79b00" ) },
        {ColorRole::Class, QColor( "#008fd5" ) },
        {ColorRole::Method, QColor( "#008fd5" ) },
        {ColorRole::Decoration, QColor( "#b79b00" ) },
        {ColorRole::Number, QColor( "#008fd5" ) },
        {ColorRole::Comment, QColor( "#93a2a3" ) },
        {ColorRole::CommentLine, QColor( "#93a2a3" ) },
        {ColorRole::CommentBlock, QColor( "#93a2a3" ) },
        {ColorRole::Background, QColor( "#fef6e4" ) },
        {ColorRole::Cursor, QColor( "#cfcfcf" ) },
        {ColorRole::CaretLine, QColor( "#f0e8d6" ) },
        {ColorRole::Operator, QColor( "#b79b00" ) },
        {ColorRole::QuotedOperator, QColor( "#b79b00" ) },
        {ColorRole::Identifier, QColor( "#008fd5" ) },
        {ColorRole::QuotedIdentifier, QColor( "#008fd5" ) },
        {ColorRole::Tag, QColor( "#008fd5" ) },
        {ColorRole::UnknownTag, QColor( "#008fd5" ) },
        {ColorRole::SingleQuote, QColor( "#3da59e" ) },
        {ColorRole::DoubleQuote, QColor( "#3da59e" ) },
        {ColorRole::TripleSingleQuote, QColor( "#3da59e" ) },
        {ColorRole::TripleDoubleQuote, QColor( "#3da59e" ) },
        {ColorRole::MarginBackground, QColor( "#f0e8d6" ) },
        {ColorRole::MarginForeground, QColor( "#93a2a3" ) },
        {ColorRole::SelectionBackground, QColor( "#d7d7d7" ) },
        {ColorRole::SelectionForeground, QColor( "#000000" ) },
        {ColorRole::MatchedBraceBackground, QColor( "#b7f907" ) },
        {ColorRole::MatchedBraceForeground, QColor( "#363636" ) },
        {ColorRole::Edge, QColor( "#f0e8d6" ) },
        {ColorRole::Fold, QColor( "#f0e8d6" ) },
        {ColorRole::Error, QColor( "#e31a1c" ) },
      },
    },
    {
      QStringLiteral( "solarized_dark" ),
      {
        {ColorRole::Default, QColor( "#f6f3e8" ) },
        {ColorRole::Keyword, QColor( "#6cbcf7" ) },
        {ColorRole::Class, QColor( "#b3e765" ) },
        {ColorRole::Method, QColor( "#ed5d36" ) },
        {ColorRole::Decoration, QColor( "#6cbcf7" ) },
        {ColorRole::Number, QColor( "#ed5d36" ) },
        {ColorRole::Comment, QColor( "#b3e765" ) },
        {ColorRole::CommentLine, QColor( "#b3e765" ) },
        {ColorRole::CommentBlock, QColor( "#aeaeae" ) },
        {ColorRole::Background, QColor( "#1f1f1f" ) },
        {ColorRole::Cursor, QColor( "#cfcfcf" ) },
        {ColorRole::CaretLine, QColor( "#363636" ) },
        {ColorRole::Operator, QColor( "#6cbcf7" ) },
        {ColorRole::QuotedOperator, QColor( "#6cbcf7" ) },
        {ColorRole::Identifier, QColor( "#b3e765" ) },
        {ColorRole::QuotedIdentifier, QColor( "#b3e765" ) },
        {ColorRole::Tag, QColor( "#ed5d36" ) },
        {ColorRole::UnknownTag, QColor( "#ed5d36" ) },
        {ColorRole::SingleQuote, QColor( "#65b042" ) },
        {ColorRole::DoubleQuote, QColor( "#65b042" ) },
        {ColorRole::TripleSingleQuote, QColor( "#65b042" ) },
        {ColorRole::TripleDoubleQuote, QColor( "#65b042" ) },
        {ColorRole::MarginBackground, QColor( "#000000" ) },
        {ColorRole::MarginForeground, QColor( "#9d9d9d" ) },
        {ColorRole::SelectionBackground, QColor( "#d7d7d7" ) },
        {ColorRole::SelectionForeground, QColor( "#000000" ) },
        {ColorRole::MatchedBraceBackground, QColor( "#b7f907" ) },
        {ColorRole::MatchedBraceForeground, QColor( "#363636" ) },
        {ColorRole::Edge, QColor( "#3a3a3a" ) },
        {ColorRole::Fold, QColor( "#3a3a3a" ) },
        {ColorRole::Error, QColor( "#e31a1c" ) },
      }
    }
  };

  if ( theme.isEmpty() && QgsApplication::instance()->themeName() == QStringLiteral( "default" ) )
  {
    // if using default theme, take certain colors from the palette
    QPalette pal = qApp->palette();

    switch ( role )
    {
      case ColorRole::SelectionBackground:
        return pal.color( QPalette::Highlight );
      case ColorRole::SelectionForeground:
        return pal.color( QPalette::HighlightedText );
      default:
        break;
    }
  }
  else if ( theme.isEmpty() )
  {
    // non default theme (e.g. Blend of Gray). Take colors from theme ini file...
    const QSettings ini( QgsApplication::instance()->uiThemes().value( QgsApplication::instance()->themeName() ) + "/qscintilla.ini", QSettings::IniFormat );

    static QMap< ColorRole, QString > sColorRoleToIniKey
    {
      {ColorRole::Default, QStringLiteral( "python/defaultFontColor" ) },
      {ColorRole::Keyword, QStringLiteral( "python/keywordFontColor" ) },
      {ColorRole::Class, QStringLiteral( "python/classFontColor" ) },
      {ColorRole::Method, QStringLiteral( "python/methodFontColor" ) },
      {ColorRole::Decoration, QStringLiteral( "python/decoratorFontColor" ) },
      {ColorRole::Number, QStringLiteral( "python/numberFontColor" ) },
      {ColorRole::Comment, QStringLiteral( "python/commentFontColor" ) },
      {ColorRole::CommentLine, QStringLiteral( "sql/commentLineFontColor" ) },
      {ColorRole::CommentBlock, QStringLiteral( "python/commentBlockFontColor" ) },
      {ColorRole::Background, QStringLiteral( "python/paperBackgroundColor" ) },
      {ColorRole::Cursor, QStringLiteral( "cursorColor" ) },
      {ColorRole::CaretLine, QStringLiteral( "caretLineColor" ) },
      {ColorRole::Operator, QStringLiteral( "sql/operatorFontColor" ) },
      {ColorRole::QuotedOperator, QStringLiteral( "sql/QuotedOperatorFontColor" ) },
      {ColorRole::Identifier, QStringLiteral( "sql/identifierFontColor" ) },
      {ColorRole::QuotedIdentifier, QStringLiteral( "sql/QuotedIdentifierFontColor" ) },
      {ColorRole::Tag, QStringLiteral( "html/tagFontColor" ) },
      {ColorRole::UnknownTag, QStringLiteral( "html/unknownTagFontColor" ) },
      {ColorRole::SingleQuote, QStringLiteral( "sql/singleQuoteFontColor" ) },
      {ColorRole::DoubleQuote, QStringLiteral( "sql/doubleQuoteFontColor" ) },
      {ColorRole::TripleSingleQuote, QStringLiteral( "python/tripleSingleQuoteFontColor" ) },
      {ColorRole::TripleDoubleQuote, QStringLiteral( "python/tripleDoubleQuoteFontColor" ) },
      {ColorRole::MarginBackground, QStringLiteral( "marginBackgroundColor" ) },
      {ColorRole::MarginForeground, QStringLiteral( "marginForegroundColor" ) },
      {ColorRole::SelectionBackground, QStringLiteral( "selectionBackgroundColor" ) },
      {ColorRole::SelectionForeground, QStringLiteral( "selectionForegroundColor" ) },
      {ColorRole::MatchedBraceBackground, QStringLiteral( "matchedBraceBackground" ) },
      {ColorRole::MatchedBraceForeground, QStringLiteral( "matchedBraceColor" ) },
      {ColorRole::Edge, QStringLiteral( "edgeColor" ) },
      {ColorRole::Fold, QStringLiteral( "foldColor" ) },
      {ColorRole::Error, QStringLiteral( "stderrFontColor" ) },
    };

    return QgsSymbolLayerUtils::decodeColor( ini.value( sColorRoleToIniKey.value( role ), sColors.value( QString() ).value( role ).name() ).toString() );
  }

  return sColors.value( theme ).value( role );
}

QColor QgsCodeEditor::color( QgsCodeEditor::ColorRole role )
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

void QgsCodeEditor::setColor( QgsCodeEditor::ColorRole role, const QColor &color )
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

  QgsSettings settings;
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
    int fontSize = settings.value( QStringLiteral( "qgis/stylesheet/fontPointSize" ), 10 ).toInt();
    font.setPointSize( fontSize );
  }
#endif
  font.setBold( false );

  return font;
}
