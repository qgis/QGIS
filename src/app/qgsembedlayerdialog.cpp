#include "qgsembedlayerdialog.h"
#include <QDomDocument>
#include <QFileDialog>

QgsEmbedLayerDialog::QgsEmbedLayerDialog( QWidget * parent, Qt::WindowFlags f ): QDialog( parent, f )
{
  setupUi( this );
}

QgsEmbedLayerDialog::~QgsEmbedLayerDialog()
{
}

QList< QPair < QString, QString > > QgsEmbedLayerDialog::embeddedGroups() const
{
  QList< QPair < QString, QString > > result;

  QList<QTreeWidgetItem*> items = mTreeWidget->selectedItems();
  QList<QTreeWidgetItem*>::iterator itemIt = items.begin();
  for(; itemIt != items.end(); ++itemIt )
  {
    if( (*itemIt)->data(0, Qt::UserRole).toString() == "group" )
    {
      result.push_back( qMakePair( (*itemIt)->text( 0 ), mProjectFileLineEdit->text() ) );
    }
  }

  return result;
}

QList< QPair < QString, QString > > QgsEmbedLayerDialog::embeddedLayers() const
{
  QList< QPair < QString, QString > > result;

  QList<QTreeWidgetItem*> items = mTreeWidget->selectedItems();
  QList<QTreeWidgetItem*>::iterator itemIt = items.begin();
  for(; itemIt != items.end(); ++itemIt )
  {
    if( (*itemIt)->data(0, Qt::UserRole).toString() == "layer" )
    {
      result.push_back( qMakePair( (*itemIt)->data(0, Qt::UserRole + 1).toString(), mProjectFileLineEdit->text() ) );
    }
  }
  return result;
}

void QgsEmbedLayerDialog::on_mBrowseFileToolButton_clicked()
{
  QString projectFile = QFileDialog::getOpenFileName( 0, tr("Select project file"), "",tr("QGIS project files (*.qgs)") );
  if( !projectFile.isEmpty() )
  {
    mProjectFileLineEdit->setText( projectFile );
  }
  changeProjectFile();
}

void QgsEmbedLayerDialog::on_mProjectFileLineEdit_editingFinished()
{
  changeProjectFile();
}

void QgsEmbedLayerDialog::changeProjectFile()
{
  mTreeWidget->clear();
  QFile projectFile( mProjectFileLineEdit->text() );
  if( !projectFile.exists() )
  {
    return;
  }

  //parse project file and fill tree
  if( !projectFile.open( QIODevice::ReadOnly ) )
  {
    return;
  }

  QDomDocument projectDom;
  if( !projectDom.setContent( &projectFile ) )
  {
    return;
  }

  QDomElement legendElem = projectDom.documentElement().firstChildElement("legend");
  if( legendElem.isNull() )
  {
    return;
  }

  QDomNodeList legendChildren = legendElem.childNodes();
  QDomElement currentChildElem;

  for( int i = 0; i < legendChildren.size(); ++i )
  {
    currentChildElem = legendChildren.at( i ).toElement();
    if( currentChildElem.tagName() == "legendlayer" )
    {
      addLegendLayerToTreeWidget( currentChildElem );
    }
    else if( currentChildElem.tagName() == "legendgroup" )
    {
      addLegendGroupToTreeWidget( currentChildElem );
    }
  }
}

void QgsEmbedLayerDialog::addLegendGroupToTreeWidget( const QDomElement& groupElem, QTreeWidgetItem* parent )
{
  QDomNodeList groupChildren = groupElem.childNodes();
  QDomElement currentChildElem;

  QTreeWidgetItem* groupItem = 0;
  if( !parent )
  {
    groupItem = new QTreeWidgetItem( mTreeWidget );
  }
  else
  {
    groupItem = new QTreeWidgetItem( parent );
  }
  groupItem->setText( 0, groupElem.attribute("name") );
  groupItem->setData( 0, Qt::UserRole, "group" );

  for( int i = 0; i < groupChildren.size(); ++i )
  {
    currentChildElem = groupChildren.at( i ).toElement();
    if( currentChildElem.tagName() == "legendlayer" )
    {
      addLegendLayerToTreeWidget( currentChildElem, groupItem );
    }
    else if( currentChildElem.tagName() == "legendgroup" )
    {
      addLegendGroupToTreeWidget( currentChildElem, groupItem );
    }
  }
}

void QgsEmbedLayerDialog::addLegendLayerToTreeWidget( const QDomElement& layerElem, QTreeWidgetItem* parent )
{
  QTreeWidgetItem* item = 0;
  if( parent )
  {
    item = new QTreeWidgetItem( parent );
  }
  else
  {
    item = new QTreeWidgetItem( mTreeWidget );
  }
  item->setText( 0, layerElem.attribute("name") );
  item->setData( 0, Qt::UserRole, "layer" );
  item->setData( 0, Qt::UserRole + 1, layerElem.firstChildElement("filegroup").firstChildElement("legendlayerfile").attribute("layerid") );
}

void QgsEmbedLayerDialog::on_mTreeWidget_itemSelectionChanged()
{
  mTreeWidget->blockSignals( true );
  QList<QTreeWidgetItem*> items = mTreeWidget->selectedItems();
  QList<QTreeWidgetItem*>::iterator itemIt = items.begin();
  for(; itemIt != items.end(); ++itemIt )
  {
    //deselect children recursively
    unselectChildren( *itemIt );
  }
  mTreeWidget->blockSignals( false );
}

void QgsEmbedLayerDialog::unselectChildren( QTreeWidgetItem* item )
{
  if( !item )
  {
    return;
  }

  QTreeWidgetItem* currentChild = 0;
  for( int i = 0; i < item->childCount(); ++i )
  {
    currentChild = item->child( i );
    currentChild->setSelected( false );
    unselectChildren( currentChild );
  }
}


