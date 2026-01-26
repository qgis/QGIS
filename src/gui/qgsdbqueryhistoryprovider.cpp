/***************************************************************************
                            qgsdbqueryhistoryprovider.cpp
                            --------------------------
    begin                : April 2023
    copyright            : (C) 2023 by Nyall Dawson
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

#include "qgsdbqueryhistoryprovider.h"

#include "qgsapplication.h"
#include "qgscodeeditorsql.h"
#include "qgshistoryentry.h"
#include "qgshistorywidgetcontext.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"

#include <QAction>
#include <QClipboard>
#include <QIcon>
#include <QMenu>
#include <QMimeData>

#include "moc_qgsdbqueryhistoryprovider.cpp"

///@cond PRIVATE

class DatabaseQueryHistoryNode : public QgsHistoryEntryGroup
{
  public:
    DatabaseQueryHistoryNode( const QgsHistoryEntry &entry, QgsDatabaseQueryHistoryProvider *provider )
      : QgsHistoryEntryGroup()
      , mEntry( entry )
      , mProvider( provider )
    {
    }

  protected:
    QgsHistoryEntry mEntry;
    QgsDatabaseQueryHistoryProvider *mProvider = nullptr;
};

class DatabaseQueryValueNode : public DatabaseQueryHistoryNode
{
  public:
    DatabaseQueryValueNode( const QgsHistoryEntry &entry, QgsDatabaseQueryHistoryProvider *provider, const QString &value )
      : DatabaseQueryHistoryNode( entry, provider )
      , mValue( value )
    {}

    QVariant data( int role = Qt::DisplayRole ) const override
    {
      switch ( role )
      {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
          return mValue;

        default:
          return QVariant();
      }
    }

    QString html( const QgsHistoryWidgetContext & ) const override
    {
      return mValue;
    }

  private:
    QString mValue;
};

class DatabaseQueryRootNode : public DatabaseQueryHistoryNode
{
  public:
    DatabaseQueryRootNode( const QgsHistoryEntry &entry, QgsDatabaseQueryHistoryProvider *provider )
      : DatabaseQueryHistoryNode( entry, provider )
    {
      setEntry( entry );

      mProviderKey = mEntry.entry.value( u"provider"_s ).toString();
    }

    QVariant data( int role = Qt::DisplayRole ) const override
    {
      switch ( role )
      {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
          return mEntry.entry.value( u"query"_s );

        case Qt::DecorationRole:
        {
          if ( !mProviderIcon.isNull() )
            return mProviderIcon;

          if ( QgsProviderMetadata *md = QgsProviderRegistry::instance()->providerMetadata( mProviderKey ) )
          {
            mProviderIcon = md->icon();
          }
          return mProviderIcon;
        }

        default:
          break;
      }
      return QVariant();
    }

    void setEntry( const QgsHistoryEntry &entry )
    {
      if ( !mConnectionNode )
      {
        mConnectionNode = new DatabaseQueryValueNode( mEntry, mProvider, QObject::tr( "Connection: %1" ).arg( entry.entry.value( u"connection"_s ).toString() ) );
        addChild( mConnectionNode );
      }
      if ( entry.entry.contains( u"rows"_s ) )
      {
        if ( !mRowsNode )
        {
          mRowsNode = new DatabaseQueryValueNode( mEntry, mProvider, QObject::tr( "Row count: %1" ).arg( entry.entry.value( u"rows"_s ).toString() ) );
          addChild( mRowsNode );
        }
      }
      if ( entry.entry.contains( u"time"_s ) )
      {
        if ( !mTimeNode )
        {
          mTimeNode = new DatabaseQueryValueNode( mEntry, mProvider, QObject::tr( "Execution time: %1 ms" ).arg( entry.entry.value( u"time"_s ).toString() ) );
          addChild( mTimeNode );
        }
      }
    }

    QWidget *createWidget( const QgsHistoryWidgetContext & ) override
    {
      QgsCodeEditorSQL *editor = new QgsCodeEditorSQL();
      editor->setText( mEntry.entry.value( u"query"_s ).toString() );
      editor->setReadOnly( true );
      editor->setCaretLineVisible( false );
      editor->setLineNumbersVisible( false );
      editor->setFoldingVisible( false );
      editor->setEdgeMode( QsciScintilla::EdgeNone );
      editor->setWrapMode( QsciScintilla::WrapMode::WrapWord );
      return editor;
    }

    bool doubleClicked( const QgsHistoryWidgetContext &context ) override
    {
      if ( QgsDatabaseQueryHistoryWidget *queryHistoryWidget = qobject_cast< QgsDatabaseQueryHistoryWidget * >( context.historyWidget() ) )
      {
        queryHistoryWidget->emitSqlTriggered( mEntry.entry.value( u"connection"_s ).toString(), mEntry.entry.value( u"provider"_s ).toString(), mEntry.entry.value( u"query"_s ).toString() );
      }
      return true;
    }

    void populateContextMenu( QMenu *menu, const QgsHistoryWidgetContext &context ) override
    {
      if ( QgsDatabaseQueryHistoryWidget *queryHistoryWidget = qobject_cast< QgsDatabaseQueryHistoryWidget * >( context.historyWidget() ) )
      {
        QAction *loadAction = new QAction(
          QObject::tr( "Load SQL Command…" ), menu
        );
        QObject::connect( loadAction, &QAction::triggered, menu, [this, queryHistoryWidget] {
          queryHistoryWidget->emitSqlTriggered( mEntry.entry.value( u"connection"_s ).toString(), mEntry.entry.value( u"provider"_s ).toString(), mEntry.entry.value( u"query"_s ).toString() );
        } );
        menu->addAction( loadAction );
      }

      QAction *copyAction = new QAction(
        QObject::tr( "Copy SQL Command" ), menu
      );
      copyAction->setIcon( QgsApplication::getThemeIcon( u"mActionEditCopy.svg"_s ) );
      QObject::connect( copyAction, &QAction::triggered, menu, [this] {
        QMimeData *m = new QMimeData();
        m->setText( mEntry.entry.value( u"query"_s ).toString() );
        QApplication::clipboard()->setMimeData( m );
      } );
      menu->addAction( copyAction );
    }

  private:
    QString mProviderKey;
    mutable QIcon mProviderIcon;
    DatabaseQueryValueNode *mConnectionNode = nullptr;
    DatabaseQueryValueNode *mRowsNode = nullptr;
    DatabaseQueryValueNode *mTimeNode = nullptr;
};

///@endcond


QgsDatabaseQueryHistoryProvider::QgsDatabaseQueryHistoryProvider()
{
}

QString QgsDatabaseQueryHistoryProvider::id() const
{
  return u"dbquery"_s;
}

QgsHistoryEntryNode *QgsDatabaseQueryHistoryProvider::createNodeForEntry( const QgsHistoryEntry &entry, const QgsHistoryWidgetContext & )
{
  return new DatabaseQueryRootNode( entry, this );
}

void QgsDatabaseQueryHistoryProvider::updateNodeForEntry( QgsHistoryEntryNode *node, const QgsHistoryEntry &entry, const QgsHistoryWidgetContext & )
{
  if ( DatabaseQueryRootNode *dbNode = dynamic_cast<DatabaseQueryRootNode *>( node ) )
  {
    dbNode->setEntry( entry );
  }
}

//
// QgsDatabaseQueryHistoryWidget
//

QgsDatabaseQueryHistoryWidget::QgsDatabaseQueryHistoryWidget( Qgis::HistoryProviderBackends backends, QgsHistoryProviderRegistry *registry, const QgsHistoryWidgetContext &context, QWidget *parent )
  : QgsHistoryWidget( u"dbquery"_s, backends, registry, context, parent )
{
}

void QgsDatabaseQueryHistoryWidget::emitSqlTriggered( const QString &connectionUri, const QString &provider, const QString &sql )
{
  emit sqlTriggered( connectionUri, provider, sql );
}
