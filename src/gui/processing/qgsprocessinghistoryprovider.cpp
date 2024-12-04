/***************************************************************************
                            qgsprocessinghistoryprovider.cpp
                            -------------------------
    begin                : December 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessinghistoryprovider.h"
#include "moc_qgsprocessinghistoryprovider.cpp"
#include "qgsapplication.h"
#include "qgsgui.h"
#include "qgshistoryproviderregistry.h"
#include "qgshistoryentry.h"
#include "qgshistoryentrynode.h"
#include "qgsprocessingregistry.h"
#include "qgscodeeditorpython.h"
#include "qgscodeeditorshell.h"
#include "qgscodeeditorjson.h"
#include "qgsjsonutils.h"

#include <nlohmann/json.hpp>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QAction>
#include <QMenu>
#include <QMimeData>
#include <QClipboard>

QgsProcessingHistoryProvider::QgsProcessingHistoryProvider()
{
}

QString QgsProcessingHistoryProvider::id() const
{
  return QStringLiteral( "processing" );
}

void QgsProcessingHistoryProvider::portOldLog()
{
  const QString logPath = oldLogPath();
  if ( !QFile::exists( logPath ) )
    return;

  QFile logFile( logPath );
  if ( logFile.open( QIODevice::ReadOnly ) )
  {
    QTextStream in( &logFile );
    QList<QgsHistoryEntry> entries;
    while ( !in.atEnd() )
    {
      const QString line = in.readLine().trimmed();
      QStringList parts = line.split( QStringLiteral( "|~|" ) );
      if ( parts.size() <= 1 )
        parts = line.split( '|' );

      if ( parts.size() == 3 && parts.at( 0 ).startsWith( QLatin1String( "ALGORITHM" ), Qt::CaseInsensitive ) )
      {
        QVariantMap details;
        details.insert( QStringLiteral( "python_command" ), parts.at( 2 ) );

        const thread_local QRegularExpression algIdRegEx( QStringLiteral( "processing\\.run\\(\"(.*?)\"" ) );
        const QRegularExpressionMatch match = algIdRegEx.match( parts.at( 2 ) );
        if ( match.hasMatch() )
          details.insert( QStringLiteral( "algorithm_id" ), match.captured( 1 ) );

        entries.append( QgsHistoryEntry( id(), QDateTime::fromString( parts.at( 1 ), QStringLiteral( "yyyy-MM-d hh:mm:ss" ) ), details ) );
      }
    }

    QgsGui::historyProviderRegistry()->addEntries( entries );
  }
}

///@cond PRIVATE


class ProcessingHistoryBaseNode : public QgsHistoryEntryGroup
{
  public:
    ProcessingHistoryBaseNode( const QgsHistoryEntry &entry, QgsProcessingHistoryProvider *provider )
      : mEntry( entry )
      , mAlgorithmId( mEntry.entry.value( "algorithm_id" ).toString() )
      , mPythonCommand( mEntry.entry.value( "python_command" ).toString() )
      , mProcessCommand( mEntry.entry.value( "process_command" ).toString() )
      , mProvider( provider )
    {
      const QVariant parameters = mEntry.entry.value( QStringLiteral( "parameters" ) );
      if ( parameters.userType() == QMetaType::Type::QVariantMap )
      {
        const QVariantMap parametersMap = parameters.toMap();
        mInputs = parametersMap.value( QStringLiteral( "inputs" ) ).toMap();
      }
    }

    bool doubleClicked( const QgsHistoryWidgetContext & ) override
    {
      if ( mPythonCommand.isEmpty() )
        return true;

      QString execAlgorithmDialogCommand = mPythonCommand;
      execAlgorithmDialogCommand.replace( QLatin1String( "processing.run(" ), QLatin1String( "processing.execAlgorithmDialog(" ) );

      // adding to this list? Also update the BatchPanel.py imports!!
      const QStringList script = {
        QStringLiteral( "import processing" ),
        QStringLiteral( "from qgis.core import QgsProcessingOutputLayerDefinition, QgsProcessingFeatureSourceDefinition, QgsProperty, QgsCoordinateReferenceSystem, QgsFeatureRequest" ),
        QStringLiteral( "from qgis.PyQt.QtCore import QDate, QTime, QDateTime" ),
        QStringLiteral( "from qgis.PyQt.QtGui import QColor" ),
        execAlgorithmDialogCommand
      };

      mProvider->emitExecute( script.join( '\n' ) );
      return true;
    }

    void populateContextMenu( QMenu *menu, const QgsHistoryWidgetContext & ) override
    {
      if ( !mPythonCommand.isEmpty() )
      {
        QAction *pythonAction = new QAction(
          QObject::tr( "Copy as Python Command" ), menu
        );
        pythonAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mIconPythonFile.svg" ) ) );
        QObject::connect( pythonAction, &QAction::triggered, menu, [=] {
          copyText( mPythonCommand );
        } );
        menu->addAction( pythonAction );
      }
      if ( !mProcessCommand.isEmpty() )
      {
        QAction *processAction = new QAction(
          QObject::tr( "Copy as qgis_process Command" ), menu
        );
        processAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionTerminal.svg" ) ) );
        QObject::connect( processAction, &QAction::triggered, menu, [=] {
          copyText( mProcessCommand );
        } );
        menu->addAction( processAction );
      }
      if ( !mInputs.isEmpty() )
      {
        QAction *inputsAction = new QAction(
          QObject::tr( "Copy as JSON" ), menu
        );
        inputsAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionEditCopy.svg" ) ) );
        QObject::connect( inputsAction, &QAction::triggered, menu, [=] {
          copyText( QString::fromStdString( QgsJsonUtils::jsonFromVariant( mInputs ).dump( 2 ) ) );
        } );
        menu->addAction( inputsAction );
      }

      if ( !mPythonCommand.isEmpty() )
      {
        if ( !menu->isEmpty() )
        {
          menu->addSeparator();
        }

        QAction *createTestAction = new QAction(
          QObject::tr( "Create Test…" ), menu
        );
        QObject::connect( createTestAction, &QAction::triggered, menu, [=] {
          mProvider->emitCreateTest( mPythonCommand );
        } );
        menu->addAction( createTestAction );
      }
    }

    void copyText( const QString &text )
    {
      QMimeData *m = new QMimeData();
      m->setText( text );
      QApplication::clipboard()->setMimeData( m );
    }

    QgsHistoryEntry mEntry;
    QString mAlgorithmId;
    QString mPythonCommand;
    QString mProcessCommand;
    QVariantMap mInputs;

    QgsProcessingHistoryProvider *mProvider = nullptr;
};

class ProcessingHistoryPythonCommandNode : public ProcessingHistoryBaseNode
{
  public:
    ProcessingHistoryPythonCommandNode( const QgsHistoryEntry &entry, QgsProcessingHistoryProvider *provider )
      : ProcessingHistoryBaseNode( entry, provider )
    {}

    QVariant data( int role = Qt::DisplayRole ) const override
    {
      switch ( role )
      {
        case Qt::DisplayRole:
        {
          QString display = mPythonCommand;
          if ( display.length() > 300 )
          {
            display = QObject::tr( "%1…" ).arg( display.left( 299 ) );
          }
          return display;
        }
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "mIconPythonFile.svg" ) );

        default:
          break;
      }
      return QVariant();
    }

    QWidget *createWidget( const QgsHistoryWidgetContext & ) override
    {
      QgsCodeEditorPython *codeEditor = new QgsCodeEditorPython();
      codeEditor->setReadOnly( true );
      codeEditor->setCaretLineVisible( false );
      codeEditor->setLineNumbersVisible( false );
      codeEditor->setFoldingVisible( false );
      codeEditor->setEdgeMode( QsciScintilla::EdgeNone );
      codeEditor->setWrapMode( QsciScintilla::WrapMode::WrapWord );


      const QString introText = QStringLiteral( "\"\"\"\n%1\n\"\"\"\n\n " ).arg( QObject::tr( "Double-click on the history item or paste the command below to re-run the algorithm" ) );
      codeEditor->setText( introText + mPythonCommand );

      return codeEditor;
    }
};

class ProcessingHistoryProcessCommandNode : public ProcessingHistoryBaseNode
{
  public:
    ProcessingHistoryProcessCommandNode( const QgsHistoryEntry &entry, QgsProcessingHistoryProvider *provider )
      : ProcessingHistoryBaseNode( entry, provider )
    {}

    QVariant data( int role = Qt::DisplayRole ) const override
    {
      switch ( role )
      {
        case Qt::DisplayRole:
        {
          QString display = mProcessCommand;
          if ( display.length() > 300 )
          {
            display = QObject::tr( "%1…" ).arg( display.left( 299 ) );
          }
          return display;
        }
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "mActionTerminal.svg" ) );

        default:
          break;
      }
      return QVariant();
    }

    QWidget *createWidget( const QgsHistoryWidgetContext & ) override
    {
      QgsCodeEditorShell *codeEditor = new QgsCodeEditorShell();
      codeEditor->setReadOnly( true );
      codeEditor->setCaretLineVisible( false );
      codeEditor->setLineNumbersVisible( false );
      codeEditor->setFoldingVisible( false );
      codeEditor->setEdgeMode( QsciScintilla::EdgeNone );
      codeEditor->setWrapMode( QsciScintilla::WrapMode::WrapWord );

      codeEditor->setText( mProcessCommand );

      return codeEditor;
    }
};


class ProcessingHistoryJsonNode : public ProcessingHistoryBaseNode
{
  public:
    ProcessingHistoryJsonNode( const QgsHistoryEntry &entry, QgsProcessingHistoryProvider *provider )
      : ProcessingHistoryBaseNode( entry, provider )
    {
      mJson = QString::fromStdString( QgsJsonUtils::jsonFromVariant( mInputs ).dump( 2 ) );
      mJsonSingleLine = QString::fromStdString( QgsJsonUtils::jsonFromVariant( mInputs ).dump() );
    }

    QVariant data( int role = Qt::DisplayRole ) const override
    {
      switch ( role )
      {
        case Qt::DisplayRole:
        {
          QString display = mJsonSingleLine;
          if ( display.length() > 300 )
          {
            display = QObject::tr( "%1…" ).arg( display.left( 299 ) );
          }
          return display;
        }
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "mIconFieldJson.svg" ) );

        default:
          break;
      }
      return QVariant();
    }

    QWidget *createWidget( const QgsHistoryWidgetContext & ) override
    {
      QgsCodeEditorJson *codeEditor = new QgsCodeEditorJson();
      codeEditor->setReadOnly( true );
      codeEditor->setCaretLineVisible( false );
      codeEditor->setLineNumbersVisible( false );
      codeEditor->setFoldingVisible( false );
      codeEditor->setEdgeMode( QsciScintilla::EdgeNone );
      codeEditor->setWrapMode( QsciScintilla::WrapMode::WrapWord );

      codeEditor->setText( mJson );

      return codeEditor;
    }

    QString mJson;
    QString mJsonSingleLine;
};


class ProcessingHistoryRootNode : public ProcessingHistoryBaseNode
{
  public:
    ProcessingHistoryRootNode( const QgsHistoryEntry &entry, QgsProcessingHistoryProvider *provider )
      : ProcessingHistoryBaseNode( entry, provider )
    {
      const QVariant parameters = mEntry.entry.value( QStringLiteral( "parameters" ) );
      if ( parameters.type() == QVariant::Map )
      {
        mDescription = QgsProcessingUtils::variantToPythonLiteral( mInputs );
      }
      else
      {
        // an older history entry which didn't record inputs
        mDescription = mPythonCommand;
      }

      if ( mDescription.length() > 300 )
      {
        mDescription = QObject::tr( "%1…" ).arg( mDescription.left( 299 ) );
      }

      addChild( new ProcessingHistoryPythonCommandNode( mEntry, mProvider ) );
      addChild( new ProcessingHistoryProcessCommandNode( mEntry, mProvider ) );
      addChild( new ProcessingHistoryJsonNode( mEntry, mProvider ) );
    }

    void setEntry( const QgsHistoryEntry &entry )
    {
      mEntry = entry;
    }

    QVariant data( int role = Qt::DisplayRole ) const override
    {
      if ( mAlgorithmInformation.displayName.isEmpty() )
      {
        mAlgorithmInformation = QgsApplication::processingRegistry()->algorithmInformation( mAlgorithmId );
      }

      switch ( role )
      {
        case Qt::DisplayRole:
        {
          const QString algName = mAlgorithmInformation.displayName;
          if ( !mDescription.isEmpty() )
            return QStringLiteral( "[%1] %2 - %3" ).arg( mEntry.timestamp.toString( QStringLiteral( "yyyy-MM-dd hh:mm" ) ), algName, mDescription );
          else
            return QStringLiteral( "[%1] %2" ).arg( mEntry.timestamp.toString( QStringLiteral( "yyyy-MM-dd hh:mm" ) ), algName );
        }

        case Qt::DecorationRole:
        {
          return mAlgorithmInformation.icon;
        }

        default:
          break;
      }
      return QVariant();
    }

    QString html( const QgsHistoryWidgetContext & ) const override
    {
      return mEntry.entry.value( QStringLiteral( "log" ) ).toString();
    }

    QString mDescription;
    mutable QgsProcessingAlgorithmInformation mAlgorithmInformation;
};

///@endcond

QgsHistoryEntryNode *QgsProcessingHistoryProvider::createNodeForEntry( const QgsHistoryEntry &entry, const QgsHistoryWidgetContext & )
{
  return new ProcessingHistoryRootNode( entry, this );
}

void QgsProcessingHistoryProvider::updateNodeForEntry( QgsHistoryEntryNode *node, const QgsHistoryEntry &entry, const QgsHistoryWidgetContext & )
{
  if ( ProcessingHistoryRootNode *rootNode = dynamic_cast<ProcessingHistoryRootNode *>( node ) )
  {
    rootNode->setEntry( entry );
  }
}

QString QgsProcessingHistoryProvider::oldLogPath() const
{
  const QString userDir = QgsApplication::qgisSettingsDirPath() + QStringLiteral( "/processing" );
  return userDir + QStringLiteral( "/processing.log" );
}

void QgsProcessingHistoryProvider::emitExecute( const QString &commands )
{
  emit executePython( commands );
}

void QgsProcessingHistoryProvider::emitCreateTest( const QString &command )
{
  emit createTest( command );
}
