/***************************************************************************
  qgsprocessingtininputlayerswidget.cpp
  ---------------------
  Date                 : August 2020
  Copyright            : (C) 2020 by Vincent Cloarec
  Email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingtininputlayerswidget.h"
#include "qgsproject.h"
#include "qgsprocessingcontext.h"

/// @cond PRIVATE

QgsProcessingTinInputLayersWidget::QgsProcessingTinInputLayersWidget( QgsProject *project ):
  mInputLayersModel( project )
{
  setupUi( this );
  mComboLayers->setFilters( QgsMapLayerProxyModel::VectorLayer );

  connect( mComboLayers, &QgsMapLayerComboBox::layerChanged, this, &QgsProcessingTinInputLayersWidget::onLayerChanged );
  connect( mButtonAdd, &QToolButton::clicked, this, &QgsProcessingTinInputLayersWidget::onCurrentLayerAdded );
  connect( mButtonRemove, &QToolButton::clicked, this, &QgsProcessingTinInputLayersWidget::onLayersRemove );
  connect( &mInputLayersModel, &QgsProcessingTinInputLayersModel::dataChanged, this, &QgsProcessingTinInputLayersWidget::changed );

  onLayerChanged( mComboLayers->currentLayer() );

  mTableView->setModel( &mInputLayersModel );
  mTableView->setItemDelegateForColumn( 1, new QgsProcessingTinInputLayersDelegate( mTableView ) );
}

QVariant QgsProcessingTinInputLayersWidget::value() const
{
  const QList<QgsProcessingParameterTinInputLayers::InputLayer> &layers = mInputLayersModel.layers();
  QVariantList list;

  for ( const QgsProcessingParameterTinInputLayers::InputLayer &layer : layers )
  {
    QVariantMap layerMap;
    layerMap[QStringLiteral( "source" )] = layer.source;
    layerMap[QStringLiteral( "type" )] = layer.type;
    layerMap[QStringLiteral( "attributeIndex" )] = layer.attributeIndex;
    list.append( layerMap );
  }

  return list;
}

void QgsProcessingTinInputLayersWidget::setValue( const QVariant &value )
{
  mInputLayersModel.clear();
  if ( !value.isValid() || value.type() != QVariant::List )
    return;

  const QVariantList list = value.toList();

  for ( const QVariant &layerValue : list )
  {
    if ( layerValue.type() != QVariant::Map )
      continue;
    const QVariantMap layerMap = layerValue.toMap();
    QgsProcessingParameterTinInputLayers::InputLayer layer;
    layer.source = layerMap.value( QStringLiteral( "source" ) ).toString();
    layer.type = static_cast<QgsProcessingParameterTinInputLayers::Type>( layerMap.value( QStringLiteral( "type" ) ).toInt() );
    layer.attributeIndex = layerMap.value( QStringLiteral( "attributeIndex" ) ).toInt();
    mInputLayersModel.addLayer( layer );
  }

  emit changed();
}

void QgsProcessingTinInputLayersWidget::setProject( QgsProject *project )
{
  mInputLayersModel.setProject( project );
}

void QgsProcessingTinInputLayersWidget::onLayerChanged( QgsMapLayer *layer )
{
  QgsVectorLayer *newLayer = qobject_cast<QgsVectorLayer *>( layer );

  if ( !newLayer || !newLayer->isValid() )
    return;

  QgsVectorDataProvider *provider = newLayer->dataProvider();

  if ( !provider )
    return;

  mComboFields->setLayer( newLayer );
  mComboFields->setCurrentIndex( 0 );
  mCheckBoxUseZCoordinate->setEnabled( QgsWkbTypes::hasZ( provider->wkbType() ) );
}

void QgsProcessingTinInputLayersWidget::onCurrentLayerAdded()
{
  QgsVectorLayer *currentLayer = qobject_cast<QgsVectorLayer *>( mComboLayers->currentLayer() );
  if ( !currentLayer )
    return;
  QgsProcessingParameterTinInputLayers::InputLayer layer;
  layer.source = mComboLayers->currentLayer()->id();

  switch ( currentLayer->geometryType() )
  {
    case QgsWkbTypes::PointGeometry:
      layer.type = QgsProcessingParameterTinInputLayers::Vertices;
      break;
    case QgsWkbTypes::LineGeometry:
    case QgsWkbTypes::PolygonGeometry:
      layer.type = QgsProcessingParameterTinInputLayers::BreakLines;
      break;
    case QgsWkbTypes::UnknownGeometry:
    case QgsWkbTypes::NullGeometry:
      return;
      break;
  }
  if ( mCheckBoxUseZCoordinate->isChecked() && mCheckBoxUseZCoordinate->isEnabled() )
    layer.attributeIndex = -1;
  else
    layer.attributeIndex = mComboFields->currentIndex();

  mInputLayersModel.addLayer( layer );

  emit changed();
}

void QgsProcessingTinInputLayersWidget::QgsProcessingTinInputLayersWidget::onLayersRemove()
{
  mInputLayersModel.removeLayer( mTableView->selectionModel()->currentIndex().row() );

  emit changed();
}

QgsProcessingTinInputLayersModel::QgsProcessingTinInputLayersModel( QgsProject *project ):
  mProject( project )
{}

int QgsProcessingTinInputLayersModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return mInputLayers.count();
}

int QgsProcessingTinInputLayersModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return 3;
}

QVariant QgsProcessingTinInputLayersModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  if ( index.row() >= mInputLayers.count() )
    return QVariant();

  switch ( role )
  {
    case Qt::DisplayRole:
    {
      QgsVectorLayer *layer = QgsProject::instance()->mapLayer<QgsVectorLayer *>( mInputLayers.at( index.row() ).source );
      switch ( index.column() )
      {
        case 0:
          if ( layer )
            return layer->name();
          else
            return QVariant();
          break;
        case 1:
          switch ( mInputLayers.at( index.row() ).type )
          {
            case QgsProcessingParameterTinInputLayers::Vertices:
              return tr( "Vertices" );
              break;
            case QgsProcessingParameterTinInputLayers::BreakLines:
              return tr( "Break Lines" );
              break;
            default:
              return QString();
              break;
          }
          break;
        case 2:
          const int attributeindex = mInputLayers.at( index.row() ).attributeIndex;
          if ( attributeindex < 0 )
            return tr( "Z coordinate" );
          else
          {
            if ( attributeindex < layer->fields().count() )
              return layer->fields().at( attributeindex ).name();
            else
              return tr( "Invalid field" );
          }
          break;
      }
    }
    break;
    case Qt::ForegroundRole:
      if ( index.column() == 2 )
      {
        const int attributeindex = mInputLayers.at( index.row() ).attributeIndex;
        if ( attributeindex < 0 )
          return QColor( Qt::darkGray );
      }
      break;
    case Qt::FontRole:
      if ( index.column() == 2 )
      {
        const int attributeindex = mInputLayers.at( index.row() ).attributeIndex;
        if ( attributeindex < 0 )
        {
          QFont font;
          font.setItalic( true );
          return font;
        }
      }
      break;
    case Type:
      if ( index.column() == 1 )
        return mInputLayers.at( index.row() ).type;
      break;
    default:
      break;
  }
  return QVariant();
}

bool QgsProcessingTinInputLayersModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( index.column() == 1 && role == Qt::EditRole )
  {
    mInputLayers[index.row()].type = static_cast<QgsProcessingParameterTinInputLayers::Type>( value.toInt() );
    emit dataChanged( QAbstractTableModel::index( index.row(), 1 ), QAbstractTableModel::index( index.row(), 1 ) );
    return true;
  }
  return false;
}

Qt::ItemFlags QgsProcessingTinInputLayersModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::NoItemFlags;

  if ( index.column() == 1 )
    return QAbstractTableModel::flags( index ) | Qt::ItemIsEditable;

  return QAbstractTableModel::flags( index );
}

QVariant QgsProcessingTinInputLayersModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
  {
    switch ( section )
    {
      case 0:
        return tr( "Vector Layer" );
        break;
      case 1:
        return tr( "Type" );
        break;
      case 2:
        return tr( "Z Value Attribute" );
        break;
      default:
        return QVariant();
        break;
    }
  }

  return QVariant();
}

void QgsProcessingTinInputLayersModel::addLayer( QgsProcessingParameterTinInputLayers::InputLayer &layer )
{
  beginInsertRows( QModelIndex(), mInputLayers.count() - 1, mInputLayers.count() - 1 );
  mInputLayers.append( layer );
  endInsertRows();
}

void QgsProcessingTinInputLayersModel::removeLayer( int index )
{
  if ( index < 0 || index >= mInputLayers.count() )
    return;
  beginRemoveRows( QModelIndex(), index, index );
  mInputLayers.removeAt( index );
  endRemoveRows();
}

void QgsProcessingTinInputLayersModel::clear()
{
  mInputLayers.clear();
}

QList<QgsProcessingParameterTinInputLayers::InputLayer> QgsProcessingTinInputLayersModel::layers() const
{
  return mInputLayers;
}

void QgsProcessingTinInputLayersModel::setProject( QgsProject *project )
{
  mProject = project;
}

QWidget *QgsProcessingTinInputLayersDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( option );
  Q_UNUSED( index );
  QComboBox *comboType = new QComboBox( parent );
  comboType->addItem( tr( "Vertices" ), QgsProcessingParameterTinInputLayers::Vertices );
  comboType->addItem( tr( "Break Lines" ), QgsProcessingParameterTinInputLayers::BreakLines );
  return comboType;
}

void QgsProcessingTinInputLayersDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  QComboBox *comboType = qobject_cast<QComboBox *>( editor );
  Q_ASSERT( comboType );
  const QgsProcessingParameterTinInputLayers::Type type =
    static_cast<QgsProcessingParameterTinInputLayers::Type>( index.data( QgsProcessingTinInputLayersModel::Type ).toInt() );
  const int comboIndex = comboType->findData( type );
  if ( comboIndex >= 0 )
    comboType->setCurrentIndex( comboIndex );
  else
    comboType->setCurrentIndex( 0 );
}

void QgsProcessingTinInputLayersDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QComboBox *comboType = qobject_cast<QComboBox *>( editor );
  Q_ASSERT( comboType );
  model->setData( index, comboType->currentData(), Qt::EditRole );
}

QgsProcessingTinInputLayersWidgetWrapper::QgsProcessingTinInputLayersWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent ):
  QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{}

QString QgsProcessingTinInputLayersWidgetWrapper::parameterType() const
{
  return QStringLiteral( "tininputlayers" );
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingTinInputLayersWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingTinInputLayersWidgetWrapper( parameter, type );
}

QStringList QgsProcessingTinInputLayersWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterTinInputLayers::typeName();
}

QStringList QgsProcessingTinInputLayersWidgetWrapper::compatibleOutputTypes() const {return QStringList();}

QWidget *QgsProcessingTinInputLayersWidgetWrapper::createWidget()
{
  mWidget = new QgsProcessingTinInputLayersWidget( widgetContext().project() );
  connect( mWidget, &QgsProcessingTinInputLayersWidget::changed, this, [ = ]
  {
    emit widgetValueHasChanged( this );
  } );

  return mWidget;
}

void QgsProcessingTinInputLayersWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  if ( !mWidget )
    return;
  mWidget->setValue( value );
  mWidget->setProject( context.project() );
}

QVariant QgsProcessingTinInputLayersWidgetWrapper::widgetValue() const
{
  if ( mWidget )
    return mWidget->value();
  else
    return QVariant();
}

/// @endcond PRIVATE
