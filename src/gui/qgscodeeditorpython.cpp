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

#include <QWidget>
#include <QString>
#include <QFont>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextStream>
#include <Qsci/qscilexerpython.h>

QgsCodeEditorPython::QgsCodeEditorPython( QWidget *parent, const QList<QString> &filenames )
  : QgsCodeEditor( parent )
  , mAPISFilesList( filenames )
{
  if ( !parent )
  {
    setTitle( tr( "Python Editor" ) );
  }
  setSciLexerPython();
}

void QgsCodeEditorPython::setSciLexerPython()
{
  QHash< QString, QColor > colors;
  if ( QgsApplication::instance()->themeName() != QStringLiteral( "default" ) )
  {
    QSettings ini( QgsApplication::instance()->uiThemes().value( QgsApplication::instance()->themeName() ) + "/qscintilla.ini", QSettings::IniFormat );
    for ( const auto &key : ini.allKeys() )
    {
      colors.insert( key, QgsSymbolLayerUtils::decodeColor( ini.value( key ).toString() ) );
    }
  }

  // current line
  setCaretWidth( 2 );

  setEdgeMode( QsciScintilla::EdgeLine );
  setEdgeColumn( 80 );
  setEdgeColor( colors.value( QStringLiteral( "edgeColor" ), QColor( 255, 0, 0 ) ) );

  setWhitespaceVisibility( QsciScintilla::WsVisibleAfterIndent );

  QFont font = getMonospaceFont();
  QColor defaultColor = colors.value( QStringLiteral( "python/defaultFontColor" ), Qt::black );

  QsciLexerPython *pyLexer = new QsciLexerPython( this );
  pyLexer->setDefaultFont( font );
  pyLexer->setDefaultColor( defaultColor );
  pyLexer->setDefaultPaper( colors.value( QStringLiteral( "python/paperBackgroundColor" ), Qt::white ) );
  pyLexer->setFont( font, -1 );
  pyLexer->setColor( defaultColor, QsciLexerPython::Default );
  pyLexer->setColor( colors.value( QStringLiteral( "python/classFontColor" ), QColor( 66, 113, 174 ) ), QsciLexerPython::ClassName );
  pyLexer->setColor( colors.value( QStringLiteral( "python/numberFontColor" ), QColor( 200, 40, 41 ) ), QsciLexerPython::Number );
  pyLexer->setColor( colors.value( QStringLiteral( "python/commentFontColor" ), QColor( 142, 144, 140 ) ), QsciLexerPython::Comment );
  pyLexer->setColor( colors.value( QStringLiteral( "python/commentBlockFontColor" ), QColor( 142, 144, 140 ) ), QsciLexerPython::CommentBlock );
  pyLexer->setColor( colors.value( QStringLiteral( "python/keywordFontColor" ), QColor( 137, 89, 168 ) ), QsciLexerPython::Keyword );
  pyLexer->setColor( colors.value( QStringLiteral( "python/decoratorFontColor" ), QColor( 62, 153, 159 ) ), QsciLexerPython::Decorator );
  pyLexer->setColor( colors.value( QStringLiteral( "python/singleQuoteFontColor" ), QColor( 113, 140, 0 ) ), QsciLexerPython::SingleQuotedString );
  pyLexer->setColor( colors.value( QStringLiteral( "python/doubleQuoteFontColor" ), QColor( 113, 140, 0 ) ), QsciLexerPython::DoubleQuotedString );
  pyLexer->setColor( colors.value( QStringLiteral( "python/tripleSingleQuoteFontColor" ), QColor( 234, 183, 0 ) ), QsciLexerPython::TripleSingleQuotedString );
  pyLexer->setColor( colors.value( QStringLiteral( "python/tripleDoubleQuoteFontColor" ), QColor( 234, 183, 0 ) ), QsciLexerPython::TripleDoubleQuotedString );

  QsciAPIs *apis = new QsciAPIs( pyLexer );

  // check if the file is a prepared apis file.
  //QString mPapFileName = QFileInfo( mAPISFilesList[0] ).fileName();
  //QString isPapFile = mPapFileName.right( 3 );
  //QgsDebugMsg( QStringLiteral( "file extension: %1" ).arg( isPapFile ) );

  if ( mAPISFilesList.isEmpty() )
  {
    mPapFile = QgsApplication::pkgDataPath() + QStringLiteral( "/python/qsci_apis/pyqgis.pap" );
    apis->loadPrepared( mPapFile );
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
    for ( int i = 0; i < mAPISFilesList.size(); i++ )
    {
      if ( !QFileInfo::exists( mAPISFilesList[i] ) )
      {
        QgsDebugMsg( QStringLiteral( "The apis file %1 was not found" ).arg( mAPISFilesList.at( i ) ) );
        return;
      }
      else
      {
        apis->load( mAPISFilesList[i] );
      }
    }
    apis->prepare();
    pyLexer->setAPIs( apis );
  }
  setLexer( pyLexer );

  setMarginVisible( true );
  setFoldingVisible( true );
  setIndentationsUseTabs( false );
}


void QgsCodeEditorPython::loadAPIs( const QList<QString> &filenames )
{
  mAPISFilesList = filenames;
  //QgsDebugMsg( QStringLiteral( "The apis files: %1" ).arg( mAPISFilesList[0] ) );
  setSciLexerPython();
}

bool QgsCodeEditorPython::loadScript( const QString &script )
{
  QgsDebugMsg( QStringLiteral( "The script file: %1" ).arg( script ) );
  QFile file( script );
  if ( !file.open( QIODevice::ReadOnly ) )
  {
    return false;
  }

  QTextStream in( &file );

  setText( in.readAll().trimmed() );
  file.close();

  setSciLexerPython();
  return true;
}
