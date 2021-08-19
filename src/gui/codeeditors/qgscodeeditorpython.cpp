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

QgsCodeEditorPython::QgsCodeEditorPython( QWidget *parent, const QList<QString> &filenames )
  : QgsCodeEditor( parent )
  , mAPISFilesList( filenames )
{
  if ( !parent )
  {
    setTitle( tr( "Python Editor" ) );
  }

  setCaretWidth( 2 );

  QgsCodeEditorPython::initializeLexer();
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

  QsciAPIs *apis = new QsciAPIs( pyLexer );

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
      pyLexer->setAPIs( apis );
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
    for ( const QString &path : mAPISFilesList )
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
    pyLexer->setAPIs( apis );
  }
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
  setFoldingVisible( true );
  setIndentationsUseTabs( false );
  setIndentationGuides( true );

  runPostLexerConfigurationTasks();
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

void QgsCodeEditorPython::searchSelectedTextInPyQGISDocs()
{
  if ( !hasSelectedText() )
    return;

  QString text = selectedText();
  text = text.replace( QLatin1String( ">>> " ), QString() ).replace( QLatin1String( "... " ), QString() ).trimmed(); // removing prompts
  const QString version = QString( Qgis::version() ).split( '.' ).mid( 0, 2 ).join( '.' );
  QDesktopServices::openUrl( QUrl( QStringLiteral( "https://qgis.org/pyqgis/%1/search.html?q=%2" ).arg( version, text ) ) );
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
    return "True False and as assert break class continue def del elif else except exec "
           "finally for from global if import in is lambda None not or pass "
           "print raise return try while with yield";
  }

  return QsciLexerPython::keywords( set );
}
///@endcond PRIVATE
