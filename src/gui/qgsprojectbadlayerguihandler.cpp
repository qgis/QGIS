#include "qgsprojectbadlayerguihandler.h"

#include <QApplication>
#include <QDomDocument>
#include <QFileInfo>
#include <QMessageBox>
#include <QPushButton>

#include "qgslogger.h"
#include "qgisgui.h"
#include "qgsproviderregistry.h"

QgsProjectBadLayerGuiHandler::QgsProjectBadLayerGuiHandler()
{
}

bool QgsProjectBadLayerGuiHandler::mIgnore = false;

void QgsProjectBadLayerGuiHandler::handleBadLayers( QList<QDomNode> layers, QDomDocument projectDom )
{
  Q_UNUSED( projectDom );

  QgsDebugMsg( QString( "%1 bad layers found" ).arg( layers.size() ) );

  // make sure we have arrow cursor (and not a wait cursor)
  QApplication::setOverrideCursor( Qt::ArrowCursor );

  QMessageBox messageBox;

  QAbstractButton *ignoreButton =
    messageBox.addButton( tr( "Ignore" ), QMessageBox::ActionRole );

  QAbstractButton *okButton = messageBox.addButton( QMessageBox :: Ok );

  messageBox.addButton( QMessageBox :: Cancel );

  messageBox.setWindowTitle( tr( "QGIS Project Read Error" ) );
  messageBox.setText( tr( "Unable to open one or more project layers.\nChoose "
                          "ignore to continue loading without the missing layers. Choose cancel to "
                          "return to your pre-project load state. Choose OK to try to find the "
                          "missing layers." ) );
  messageBox.setIcon( QMessageBox::Critical );
  messageBox.exec();

  QgsProjectBadLayerGuiHandler::mIgnore = false;

  if ( messageBox.clickedButton() == okButton )
  {
    QgsDebugMsg( "want to find missing layers is true" );

    // attempt to find the new locations for missing layers
    // XXX vector file hard-coded -- but what if it's raster?

    QString filter = QgsProviderRegistry::instance()->fileVectorFilters();
    findLayers( filter, layers );
  }
  else if ( messageBox.clickedButton() == ignoreButton )
  {
    QgsProjectBadLayerGuiHandler::mIgnore = true;
  }
  else
  {
    // Do nothing
  }

  QApplication::restoreOverrideCursor();
}

QgsProjectBadLayerGuiHandler::DataType QgsProjectBadLayerGuiHandler::dataType( QDomNode & layerNode )
{
  QString type = layerNode.toElement().attribute( "type" );

  if ( QString::null == type )
  {
    QgsDebugMsg( "cannot find ``type'' attribute" );

    return IS_BOGUS;
  }

  if ( "raster" == type )
  {
    QgsDebugMsg( "is a raster" );

    return IS_RASTER;
  }
  else if ( "vector" == type )
  {
    QgsDebugMsg( "is a vector" );

    return IS_VECTOR;
  }

  QgsDebugMsg( "is unknown type " + type );

  return IS_BOGUS;
} // dataType_( QDomNode & layerNode )


QString QgsProjectBadLayerGuiHandler::dataSource( QDomNode & layerNode )
{
  QDomNode dataSourceNode = layerNode.namedItem( "datasource" );

  if ( dataSourceNode.isNull() )
  {
    QgsDebugMsg( "cannot find datasource node" );

    return QString::null;
  }

  return dataSourceNode.toElement().text();

} // dataSource( QDomNode & layerNode )




QgsProjectBadLayerGuiHandler::ProviderType QgsProjectBadLayerGuiHandler::providerType( QDomNode & layerNode )
{
  // XXX but what about rasters that can be URLs?  _Can_ they be URLs?

  switch ( dataType( layerNode ) )
  {
    case IS_VECTOR:
    {
      QString ds = dataSource( layerNode );

      QgsDebugMsg( "datasource is " + ds );

      if ( ds.contains( "host=" ) )
      {
        return IS_URL;
      }
#ifdef HAVE_POSTGRESQL
      else if ( ds.contains( "dbname=" ) )
      {
        return IS_DATABASE;
      }
#endif
      // be default, then, this should be a file based layer data source
      // XXX is this a reasonable assumption?

      return IS_FILE;
    }

    case IS_RASTER:         // rasters are currently only accessed as
      // physical files
      return IS_FILE;

    default:
      QgsDebugMsg( "unknown ``type'' attribute" );
  }

  return IS_Unknown;

} // providerType



void QgsProjectBadLayerGuiHandler::setDataSource( QDomNode & layerNode, QString const & dataSource )
{
  QDomNode dataSourceNode = layerNode.namedItem( "datasource" );
  QDomElement dataSourceElement = dataSourceNode.toElement();
  QDomText dataSourceText = dataSourceElement.firstChild().toText();

  QgsDebugMsg( "datasource changed from " + dataSourceText.data() );

  dataSourceText.setData( dataSource );

  QgsDebugMsg( "to " + dataSourceText.data() );
} // setDataSource




bool QgsProjectBadLayerGuiHandler::findMissingFile( QString const & fileFilters, QDomNode & layerNode )
{
  // Prepend that file name to the valid file format filter list since it
  // makes it easier for the user to not only find the original file, but to
  // perhaps find a similar file.

  QFileInfo originalDataSource( dataSource( layerNode ) );

  QString memoryQualifier;    // to differentiate between last raster and
  // vector directories

  switch ( dataType( layerNode ) )
  {
    case IS_VECTOR:
    {
      memoryQualifier = "lastVectorFileFilter";

      break;
    }
    case IS_RASTER:
    {
      memoryQualifier = "lastRasterFileFilter";

      break;
    }
    default:
      QgsDebugMsg( "unable to determine data type" );
      return false;
  }

  // Prepend the original data source base name to make it easier to pick it
  // out from a list of other files; however the appropriate filter strings
  // for the file type will also be added in case the file name itself has
  // changed, too.

  QString myFileFilters = originalDataSource.fileName() + ";;" + fileFilters;

  QStringList selectedFiles;
  QString enc;
  QString title = QObject::tr( "Where is '%1' (original location: %2)?" )
                  .arg( originalDataSource.fileName() )
                  .arg( originalDataSource.absoluteFilePath() );

  bool retVal = QgisGui::openFilesRememberingFilter( memoryQualifier,
                myFileFilters,
                selectedFiles,
                enc,
                title,
                true );

  if ( selectedFiles.isEmpty() )
  {
    return retVal;
  }
  else
  {
    setDataSource( layerNode, selectedFiles.first() );
    if ( ! QgsProject::instance()->read( layerNode ) )
    {
      QgsDebugMsg( "unable to re-read layer" );
    }
  }
  return retVal;
} // findMissingFile




bool QgsProjectBadLayerGuiHandler::findLayer( QString const & fileFilters, QDomNode const & constLayerNode )
{
  // XXX actually we could possibly get away with a copy of the node
  QDomNode & layerNode = const_cast<QDomNode&>( constLayerNode );

  bool retVal = false;

  switch ( providerType( layerNode ) )
  {
    case IS_FILE:
      QgsDebugMsg( "layer is file based" );
      retVal = findMissingFile( fileFilters, layerNode );
      break;

    case IS_DATABASE:
      QgsDebugMsg( "layer is database based" );
      break;

    case IS_URL:
      QgsDebugMsg( "layer is URL based" );
      break;

    case IS_Unknown:
      QgsDebugMsg( "layer has an unknown type" );
      break;
  }
  return retVal;
} // findLayer




void QgsProjectBadLayerGuiHandler::findLayers( QString const & fileFilters, QList<QDomNode> const & layerNodes )
{

  for ( QList<QDomNode>::const_iterator i = layerNodes.begin();
        i != layerNodes.end();
        ++i )
  {
    if ( findLayer( fileFilters, *i ) )
    {
      // If findLayer returns true, the user hit Cancel All button
      break;
    }
  }

} // findLayers

