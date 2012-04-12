#include "qgsembedlayerdialog.h"
#include "qgsproject.h"
#include "qgisapp.h"
#include <QDomDocument>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>

QgsEmbedLayerDialog::QgsEmbedLayerDialog( QWidget * parent, Qt::WindowFlags f ): QDialog( parent, f )
{
  setupUi( this );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/EmbedLayer/geometry" ).toByteArray() );

  QObject::connect( mButtonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );
}

QgsEmbedLayerDialog::~QgsEmbedLayerDialog()
{
  QSettings settings;
  settings.setValue( "/Windows/EmbedLayer/geometry", saveGeometry() );
}

QList< QPair < QString, QString > > QgsEmbedLayerDialog::embeddedGroups() const
{
  QList< QPair < QString, QString > > result;

  QList<QTreeWidgetItem*> items = mTreeWidget->selectedItems();
  QList<QTreeWidgetItem*>::iterator itemIt = items.begin();
  for ( ; itemIt != items.end(); ++itemIt )
  {
    if (( *itemIt )->data( 0, Qt::UserRole ).toString() == "group" )
    {
      result.push_back( qMakePair(( *itemIt )->text( 0 ), mProjectPath ) );
    }
  }

  return result;
}

QList< QPair < QString, QString > > QgsEmbedLayerDialog::embeddedLayers() const
{
  QList< QPair < QString, QString > > result;

  QList<QTreeWidgetItem*> items = mTreeWidget->selectedItems();
  QList<QTreeWidgetItem*>::iterator itemIt = items.begin();
  for ( ; itemIt != items.end(); ++itemIt )
  {
    if (( *itemIt )->data( 0, Qt::UserRole ).toString() == "layer" )
    {
      result.push_back( qMakePair(( *itemIt )->data( 0, Qt::UserRole + 1 ).toString(), mProjectPath ) );
    }
  }
  return result;
}

void QgsEmbedLayerDialog::on_mBrowseFileToolButton_clicked()
{
  //line edit might emit editingFinished signal when loosing focus
  mProjectFileLineEdit->blockSignals( true );

  QSettings s;
  QString projectFile = QFileDialog::getOpenFileName( this,
                        tr( "Select project file" ),
                        s.value( "/qgis/last_embedded_project_path" ).toString() ,
                        tr( "QGis files" ) + " (*.qgs *.QGS)" );
  if ( !projectFile.isEmpty() )
  {
    mProjectFileLineEdit->setText( projectFile );
  }
  changeProjectFile();
  mProjectFileLineEdit->blockSignals( false );
}

void QgsEmbedLayerDialog::on_mProjectFileLineEdit_editingFinished()
{
  changeProjectFile();
}

void QgsEmbedLayerDialog::changeProjectFile()
{
  QFile projectFile( mProjectFileLineEdit->text() );
  if ( !projectFile.exists() )
  {
    return;
  }

  if ( mProjectPath == mProjectFileLineEdit->text() )
  {
    //already up to date
    return;
  }

  //check we are not embedding from/to the same project
  if ( mProjectFileLineEdit->text() == QgsProject::instance()->fileName() )
  {
    QMessageBox::critical( 0, tr( "Recursive embeding not possible" ), tr( "It is not possible to embed layers / groups from the current project" ) );
    return;
  }

  mTreeWidget->clear();

  //parse project file and fill tree
  if ( !projectFile.open( QIODevice::ReadOnly ) )
  {
    return;
  }

  QDomDocument projectDom;
  if ( !projectDom.setContent( &projectFile ) )
  {
    return;
  }

  QDomElement legendElem = projectDom.documentElement().firstChildElement( "legend" );
  if ( legendElem.isNull() )
  {
    return;
  }

  QDomNodeList legendChildren = legendElem.childNodes();
  QDomElement currentChildElem;

  for ( int i = 0; i < legendChildren.size(); ++i )
  {
    currentChildElem = legendChildren.at( i ).toElement();
    if ( currentChildElem.tagName() == "legendlayer" )
    {
      addLegendLayerToTreeWidget( currentChildElem );
    }
    else if ( currentChildElem.tagName() == "legendgroup" )
    {
      addLegendGroupToTreeWidget( currentChildElem );
    }
  }

  mProjectPath = mProjectFileLineEdit->text();
}

void QgsEmbedLayerDialog::addLegendGroupToTreeWidget( const QDomElement& groupElem, QTreeWidgetItem* parent )
{
  QDomNodeList groupChildren = groupElem.childNodes();
  QDomElement currentChildElem;

  if ( groupElem.attribute( "embedded" ) == "1" )
  {
    return;
  }

  QTreeWidgetItem* groupItem = 0;
  if ( !parent )
  {
    groupItem = new QTreeWidgetItem( mTreeWidget );
  }
  else
  {
    groupItem = new QTreeWidgetItem( parent );
  }
  groupItem->setIcon( 0, QgisApp::getThemeIcon( "mActionFolder.png" ) );
  groupItem->setText( 0, groupElem.attribute( "name" ) );
  groupItem->setData( 0, Qt::UserRole, "group" );

  for ( int i = 0; i < groupChildren.size(); ++i )
  {
    currentChildElem = groupChildren.at( i ).toElement();
    if ( currentChildElem.tagName() == "legendlayer" )
    {
      addLegendLayerToTreeWidget( currentChildElem, groupItem );
    }
    else if ( currentChildElem.tagName() == "legendgroup" )
    {
      addLegendGroupToTreeWidget( currentChildElem, groupItem );
    }
  }
}

void QgsEmbedLayerDialog::addLegendLayerToTreeWidget( const QDomElement& layerElem, QTreeWidgetItem* parent )
{
  if ( layerElem.attribute( "embedded" ) == "1" )
  {
    return;
  }

  QTreeWidgetItem* item = 0;
  if ( parent )
  {
    item = new QTreeWidgetItem( parent );
  }
  else
  {
    item = new QTreeWidgetItem( mTreeWidget );
  }
  item->setText( 0, layerElem.attribute( "name" ) );
  item->setData( 0, Qt::UserRole, "layer" );
  item->setData( 0, Qt::UserRole + 1, layerElem.firstChildElement( "filegroup" ).firstChildElement( "legendlayerfile" ).attribute( "layerid" ) );
}

void QgsEmbedLayerDialog::on_mTreeWidget_itemSelectionChanged()
{
  mTreeWidget->blockSignals( true );
  QList<QTreeWidgetItem*> items = mTreeWidget->selectedItems();
  QList<QTreeWidgetItem*>::iterator itemIt = items.begin();
  for ( ; itemIt != items.end(); ++itemIt )
  {
    //deselect children recursively
    unselectChildren( *itemIt );
  }
  mTreeWidget->blockSignals( false );
}

void QgsEmbedLayerDialog::unselectChildren( QTreeWidgetItem* item )
{
  if ( !item )
  {
    return;
  }

  QTreeWidgetItem* currentChild = 0;
  for ( int i = 0; i < item->childCount(); ++i )
  {
    currentChild = item->child( i );
    currentChild->setSelected( false );
    unselectChildren( currentChild );
  }
}

void QgsEmbedLayerDialog::on_mButtonBox_accepted()
{
  QSettings s;
  QFileInfo fi( mProjectPath );
  if ( fi.exists() )
  {
    s.setValue( "/qgis/last_embedded_project_path", fi.absolutePath() );
  }
  accept();
}


