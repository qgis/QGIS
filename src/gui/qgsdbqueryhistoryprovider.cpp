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
#include "qgscodeeditorsql.h"
#include "qgshistoryentry.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"

#include <QIcon>

class QgsDatabaseQueryValueNode;

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
    QgsDatabaseQueryValueNode *mConnectionNode = nullptr;
    QgsDatabaseQueryValueNode *mRowsNode = nullptr;
    QgsDatabaseQueryValueNode *mTimeNode = nullptr;
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

      mProviderKey = mEntry.entry.value( QStringLiteral( "provider" ) ).toString();
    }

    QVariant data( int role = Qt::DisplayRole ) const override
    {
      switch ( role )
      {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
          return mEntry.entry.value( QStringLiteral( "query" ) );

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
        mConnectionNode = new DatabaseQueryValueNode( mEntry, mProvider, QStringLiteral( "Connection: %1" ).arg( entry.entry.value( QStringLiteral( "connection" ) ).toString() ) );
        addChild( mConnectionNode );
      }
      if ( entry.entry.contains( QStringLiteral( "rows" ) ) )
      {
        if ( !mRowsNode )
        {
          mRowsNode = new DatabaseQueryValueNode( mEntry, mProvider, QStringLiteral( "Row count: %1" ).arg( entry.entry.value( QStringLiteral( "rows" ) ).toString() ) );
          addChild( mRowsNode );
        }
      }
      if ( entry.entry.contains( QStringLiteral( "time" ) ) )
      {
        if ( !mTimeNode )
        {
          mTimeNode = new DatabaseQueryValueNode( mEntry, mProvider, QStringLiteral( "Execution time: %1 ms" ).arg( entry.entry.value( QStringLiteral( "time" ) ).toString() ) );
          addChild( mTimeNode );
        }
      }
    }

    QWidget *createWidget( const QgsHistoryWidgetContext & ) override
    {
      QgsCodeEditorSQL *editor = new QgsCodeEditorSQL();
      editor->setText( mEntry.entry.value( QStringLiteral( "query" ) ).toString() );
      editor->setReadOnly( true );
      editor->setCaretLineVisible( false );
      editor->setLineNumbersVisible( false );
      editor->setFoldingVisible( false );
      editor->setEdgeMode( QsciScintilla::EdgeNone );
      editor->setWrapMode( QsciScintilla::WrapMode::WrapWord );
      return editor;
    }

    void doubleClicked( const QgsHistoryWidgetContext & ) override
    {
      mProvider->emitOpenSqlDialog( mEntry.entry.value( QStringLiteral( "connection" ) ).toString(),
                                    mEntry.entry.value( QStringLiteral( "provider" ) ).toString(),
                                    mEntry.entry.value( QStringLiteral( "query" ) ).toString() );
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
  return QStringLiteral( "dbquery" );
}

QgsHistoryEntryNode *QgsDatabaseQueryHistoryProvider::createNodeForEntry( const QgsHistoryEntry &entry, const QgsHistoryWidgetContext & )
{
  return new DatabaseQueryRootNode( entry, this );
}

void QgsDatabaseQueryHistoryProvider::updateNodeForEntry( QgsHistoryEntryNode *node, const QgsHistoryEntry &entry, const QgsHistoryWidgetContext & )
{
  if ( DatabaseQueryRootNode *dbNode = dynamic_cast< DatabaseQueryRootNode * >( node ) )
  {
    dbNode->setEntry( entry );
  }
}

void QgsDatabaseQueryHistoryProvider::emitOpenSqlDialog( const QString &connectionUri, const QString &provider, const QString &sql )
{
  emit openSqlDialog( connectionUri, provider, sql );
}
