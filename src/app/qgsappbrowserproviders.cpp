/***************************************************************************
    qgsappbrowserproviders.cpp
    ---------------------------
    begin                : September 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsappbrowserproviders.h"
#include "qgisapp.h"
#include "qgsstyleexportimportdialog.h"
#include "qgsstyle.h"
#include "qgslayertreenode.h"
#include "qgslayertree.h"
#include <QDesktopServices>

//
// QgsQlrDataItem
//

QgsQlrDataItem::QgsQlrDataItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsLayerItem( parent, name, path, path, QgsLayerItem::NoType, QStringLiteral( "qlr" ) )
{
  setState( QgsDataItem::Populated ); // no children
  setIconName( QStringLiteral( ":/images/icons/qgis-icon-16x16.png" ) );
  setToolTip( QDir::toNativeSeparators( path ) );
}

bool QgsQlrDataItem::hasDragEnabled() const
{
  return true;
}

QgsMimeDataUtils::Uri QgsQlrDataItem::mimeUri() const
{
  QgsMimeDataUtils::Uri u;
  u.layerType = QStringLiteral( "custom" );
  u.providerKey = QStringLiteral( "qlr" );
  u.name = name();
  u.uri = path();
  return u;
}

//
// QgsQlrDataItemProvider
//

QString QgsQlrDataItemProvider::name()
{
  return QStringLiteral( "QLR" );
}

int QgsQlrDataItemProvider::capabilities()
{
  return QgsDataProvider::File;
}

QgsDataItem *QgsQlrDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  QFileInfo fileInfo( path );

  if ( fileInfo.suffix().compare( QLatin1String( "qlr" ), Qt::CaseInsensitive ) == 0 )
  {
    return new QgsQlrDataItem( parentItem, fileInfo.fileName(), path );
  }
  return nullptr;
}

QString QgsQlrDropHandler::customUriProviderKey() const
{
  return QStringLiteral( "qlr" );
}

void QgsQlrDropHandler::handleCustomUriDrop( const QgsMimeDataUtils::Uri &uri ) const
{
  QString path = uri.uri;
  QgisApp::instance()->openLayerDefinition( path );
}

//
// QgsQptDataItemProvider
//

QString QgsQptDataItemProvider::name()
{
  return QStringLiteral( "QPT" );
}

int QgsQptDataItemProvider::capabilities()
{
  return QgsDataProvider::File;
}

QgsDataItem *QgsQptDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  QFileInfo fileInfo( path );

  if ( fileInfo.suffix().compare( QLatin1String( "qpt" ), Qt::CaseInsensitive ) == 0 )
  {
    return new QgsQptDataItem( parentItem, fileInfo.baseName(), path );
  }
  return nullptr;
}

//
// QgsQptDropHandler
//

QString QgsQptDropHandler::customUriProviderKey() const
{
  return QStringLiteral( "qpt" );
}

void QgsQptDropHandler::handleCustomUriDrop( const QgsMimeDataUtils::Uri &uri ) const
{
  QString path = uri.uri;
  QgisApp::instance()->openTemplate( path );
}

bool QgsQptDropHandler::handleFileDrop( const QString &file )
{
  QFileInfo fi( file );
  if ( fi.completeSuffix().compare( QLatin1String( "qpt" ), Qt::CaseInsensitive ) == 0 )
  {
    QgisApp::instance()->openTemplate( file );
    return true;
  }
  return false;
}

//
// QgsQptDataItem
//

QgsQptDataItem::QgsQptDataItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataItem( QgsDataItem::Custom, parent, name, path )
{
  setState( QgsDataItem::Populated ); // no children
  setIconName( QStringLiteral( "/mIconQptFile.svg" ) );
  setToolTip( QDir::toNativeSeparators( path ) );
}

bool QgsQptDataItem::hasDragEnabled() const
{
  return true;
}

QgsMimeDataUtils::Uri QgsQptDataItem::mimeUri() const
{
  QgsMimeDataUtils::Uri u;
  u.layerType = QStringLiteral( "custom" );
  u.providerKey = QStringLiteral( "qpt" );
  u.name = name();
  u.uri = path();
  return u;
}

bool QgsQptDataItem::handleDoubleClick()
{
  QgisApp::instance()->openTemplate( path() );
  return true;
}

QList<QAction *> QgsQptDataItem::actions( QWidget *parent )
{
  QAction *newLayout = new QAction( tr( "New Layout from Template" ), parent );
  connect( newLayout, &QAction::triggered, this, [ = ]
  {
    QgisApp::instance()->openTemplate( path() );
  } );
  return QList<QAction *>() << newLayout;
}

//
// QgsPyDataItem
//

QgsPyDataItem::QgsPyDataItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataItem( QgsDataItem::Custom, parent, name, path )
{
  setState( QgsDataItem::Populated ); // no children
  setIconName( QStringLiteral( "/mIconPythonFile.svg" ) );
  setToolTip( QDir::toNativeSeparators( path ) );
}

bool QgsPyDataItem::hasDragEnabled() const
{
  return true;
}

QgsMimeDataUtils::Uri QgsPyDataItem::mimeUri() const
{
  QgsMimeDataUtils::Uri u;
  u.layerType = QStringLiteral( "custom" );
  u.providerKey = QStringLiteral( "py" );
  u.name = name();
  u.uri = path();
  return u;
}

bool QgsPyDataItem::handleDoubleClick()
{
  QgisApp::instance()->runScript( path() );
  return true;
}

QList<QAction *> QgsPyDataItem::actions( QWidget *parent )
{
  QAction *runScript = new QAction( tr( "&Run Script" ), parent );
  connect( runScript, &QAction::triggered, this, [ = ]
  {
    QgisApp::instance()->runScript( path() );
  } );
  QAction *editScript = new QAction( tr( "Open in External &Editor" ), this );
  connect( editScript, &QAction::triggered, this, [ = ]
  {
    QDesktopServices::openUrl( QUrl::fromLocalFile( path() ) );
  } );
  return QList<QAction *>() << runScript << editScript;
}

//
// QgsPyDataItemProvider
//

QString QgsPyDataItemProvider::name()
{
  return QStringLiteral( "py" );
}

int QgsPyDataItemProvider::capabilities()
{
  return QgsDataProvider::File;
}

QgsDataItem *QgsPyDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  QFileInfo fileInfo( path );

  if ( fileInfo.suffix().compare( QLatin1String( "py" ), Qt::CaseInsensitive ) == 0 )
  {
    return new QgsPyDataItem( parentItem, fileInfo.baseName(), path );
  }
  return nullptr;
}

//
// QgsPyDropHandler
//

QString QgsPyDropHandler::customUriProviderKey() const
{
  return QStringLiteral( "py" );
}

void QgsPyDropHandler::handleCustomUriDrop( const QgsMimeDataUtils::Uri &uri ) const
{
  QString path = uri.uri;
  QgisApp::instance()->runScript( path );
}

bool QgsPyDropHandler::handleFileDrop( const QString &file )
{
  QFileInfo fi( file );
  if ( fi.completeSuffix().compare( QLatin1String( "py" ), Qt::CaseInsensitive ) == 0 )
  {
    QgisApp::instance()->runScript( file );
    return true;
  }
  return false;
}




//
// QgsStyleXmlDataItem
//

QgsStyleXmlDataItem::QgsStyleXmlDataItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataItem( QgsDataItem::Custom, parent, name, path )
{
  setState( QgsDataItem::Populated ); // no children
  setIconName( QStringLiteral( "/mActionStyleManager.svg" ) );
  setToolTip( QStringLiteral( "<b>%1</b><br>%2" ).arg( tr( "QGIS style library" ), QDir::toNativeSeparators( path ) ) );
}

bool QgsStyleXmlDataItem::hasDragEnabled() const
{
  return true;
}

QgsMimeDataUtils::Uri QgsStyleXmlDataItem::mimeUri() const
{
  QgsMimeDataUtils::Uri u;
  u.layerType = QStringLiteral( "custom" );
  u.providerKey = QStringLiteral( "style_xml" );
  u.name = name();
  u.uri = path();
  return u;
}

bool QgsStyleXmlDataItem::handleDoubleClick()
{
  QgsStyleExportImportDialog dlg( QgsStyle::defaultStyle(), QgisApp::instance(), QgsStyleExportImportDialog::Import );
  dlg.setImportFilePath( mPath );
  dlg.exec();

  return true;
}

QList<QAction *> QgsStyleXmlDataItem::actions( QWidget *parent )
{
  QAction *importAction = new QAction( tr( "&Import Style…" ), parent );
  const QString path = mPath;
  connect( importAction, &QAction::triggered, this, [path]
  {
    QgsStyleExportImportDialog dlg( QgsStyle::defaultStyle(), QgisApp::instance(), QgsStyleExportImportDialog::Import );
    dlg.setImportFilePath( path );
    dlg.exec();
  } );
  return QList<QAction *>() << importAction;
}

//
// QgsStyleXmlDataItemProvider
//


bool isStyleFile( const QString &path )
{
  QFileInfo fileInfo( path );

  if ( fileInfo.suffix().compare( QLatin1String( "xml" ), Qt::CaseInsensitive ) != 0 )
    return false;

  // sniff the first line of the file to see if it's a style file
  if ( !QFile::exists( path ) )
    return false;

  QFile inputFile( path );
  if ( !inputFile.open( QIODevice::ReadOnly ) )
    return false;

  QTextStream stream( &inputFile );
  const QString line = stream.readLine();
  return line == QLatin1String( "<!DOCTYPE qgis_style>" );
}

QString QgsStyleXmlDataItemProvider::name()
{
  return QStringLiteral( "style_xml" );
}

int QgsStyleXmlDataItemProvider::capabilities()
{
  return QgsDataProvider::File;
}

QgsDataItem *QgsStyleXmlDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  if ( isStyleFile( path ) )
  {
    return new QgsStyleXmlDataItem( parentItem, QFileInfo( path ).fileName(), path );
  }
  return nullptr;
}

//
// QgsStyleXmlDropHandler
//

QString QgsStyleXmlDropHandler::customUriProviderKey() const
{
  return QStringLiteral( "style_xml" );
}

void QgsStyleXmlDropHandler::handleCustomUriDrop( const QgsMimeDataUtils::Uri &uri ) const
{
  QgsStyleExportImportDialog dlg( QgsStyle::defaultStyle(), QgisApp::instance(), QgsStyleExportImportDialog::Import );
  dlg.setImportFilePath( uri.uri );
  dlg.exec();
}

bool QgsStyleXmlDropHandler::handleFileDrop( const QString &file )
{
  if ( isStyleFile( file ) )
  {
    QgsStyleExportImportDialog dlg( QgsStyle::defaultStyle(), QgisApp::instance(), QgsStyleExportImportDialog::Import );
    dlg.setImportFilePath( file );
    dlg.exec();
    return true;
  }
  return false;
}

//
// QgsProjectRootDataItem
//

QgsProjectRootDataItem::QgsProjectRootDataItem( QgsDataItem *parent, const QString &path )
  : QgsProjectItem( parent, QFileInfo( path ).completeBaseName(), path )
{
  mCapabilities = Collapse | Fertile; // collapse by default to avoid costly population on startup
  setState( NotPopulated );
}


QVector<QgsDataItem *> QgsProjectRootDataItem::createChildren()
{
  QVector<QgsDataItem *> childItems;

  QgsProject p;
  if ( !p.read( mPath ) )
  {
    childItems.append( new QgsErrorItem( nullptr, p.error(), mPath + "/error" ) );
    return childItems;
  }

  // recursively create groups and layer items for project's layer tree
  std::function<void( QgsDataItem *parentItem, QgsLayerTreeGroup *group )> addNodes;
  addNodes = [this, &addNodes, &childItems]( QgsDataItem * parentItem, QgsLayerTreeGroup * group )
  {
    const QList< QgsLayerTreeNode * > children = group->children();
    for ( QgsLayerTreeNode *child : children )
    {
      switch ( child->nodeType() )
      {
        case QgsLayerTreeNode::NodeLayer:
        {
          if ( QgsLayerTreeLayer *layerNode = qobject_cast< QgsLayerTreeLayer * >( child ) )
          {
            QgsMapLayer *layer = layerNode->layer();
#if 0 // TODO
            QString style;
            if ( layer )
            {
              QString errorMsg;
              QDomDocument doc( QStringLiteral( "qgis" ) );
              QgsReadWriteContext context;
              context.setPathResolver( p.pathResolver() );
              layer->exportNamedStyle( doc, errorMsg, context );
              style = doc.toString();
            }
#endif

            QgsLayerItem *layerItem = new QgsLayerItem( nullptr, layerNode->name(),
                layer ? layer->source() : QString(),
                layer ? layer->source() : QString(),
                layer ? QgsLayerItem::typeFromMapLayer( layer ) : QgsLayerItem::NoType,
                layer ? layer->dataProvider()->name() : QString() );
            layerItem->setState( Populated ); // children are not expected
            layerItem->setToolTip( layer ? layer->source() : QString() );
            if ( parentItem == this )
              childItems << layerItem;
            else
              parentItem->addChildItem( layerItem, true );
          }
          break;
        }

        case QgsLayerTreeNode::NodeGroup:
        {
          if ( QgsLayerTreeGroup *groupNode = qobject_cast< QgsLayerTreeGroup * >( child ) )
          {
            QgsProjectLayerTreeGroupItem *groupItem = new QgsProjectLayerTreeGroupItem( nullptr, groupNode->name() );
            addNodes( groupItem, groupNode );
            groupItem->setState( Populated );
            if ( parentItem == this )
              childItems << groupItem;
            else
              parentItem->addChildItem( groupItem, true );
          }
        }
        break;
      }
    }
  };

  addNodes( this, p.layerTreeRoot() );
  return childItems;
}


//
// QgsProjectLayerTreeGroupItem
//

QgsProjectLayerTreeGroupItem::QgsProjectLayerTreeGroupItem( QgsDataItem *parent, const QString &name )
  : QgsDataCollectionItem( parent, name )
{
  mIconName = QStringLiteral( "mActionFolder.svg" );
  mCapabilities = NoCapabilities;
  setToolTip( name );
}


//
// QgsProjectDataItemProvider
//

QString QgsProjectDataItemProvider::name()
{
  return QStringLiteral( "project_item" );
}

int QgsProjectDataItemProvider::capabilities()
{
  return QgsDataProvider::File;
}

QgsDataItem *QgsProjectDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  QFileInfo fileInfo( path );
  if ( fileInfo.suffix().compare( QLatin1String( "qgs" ), Qt::CaseInsensitive ) == 0 || fileInfo.suffix().compare( QLatin1String( "qgz" ), Qt::CaseInsensitive ) == 0 )
  {
    return new QgsProjectRootDataItem( parentItem, path );
  }
  return nullptr;
}
