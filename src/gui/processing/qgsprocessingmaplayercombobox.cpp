/***************************************************************************
    qgsprocessingmaplayercombobox.cpp
    -------------------------------
    begin                : June 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingmaplayercombobox.h"
#include "qgsmaplayercombobox.h"
#include "qgsmimedatautils.h"
#include "qgsprocessingparameters.h"
#include "qgssettings.h"
#include "qgsvectorlayer.h"
#include "qgsfeatureid.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>
#include <QCheckBox>
#include <QDragEnterEvent>

///@cond PRIVATE

QgsProcessingMapLayerComboBox::QgsProcessingMapLayerComboBox( const QgsProcessingParameterDefinition *parameter, QWidget *parent )
  : QWidget( parent )
  , mParameter( parameter->clone() )
{
  QHBoxLayout *layout = new QHBoxLayout();
  layout->setMargin( 0 );
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->setSpacing( 6 );

  mCombo = new QgsMapLayerComboBox();
  layout->addWidget( mCombo );
  layout->setAlignment( mCombo, Qt::AlignTop );

  mSelectButton = new QToolButton();
  mSelectButton->setText( QStringLiteral( "…" ) );
  mSelectButton->setToolTip( tr( "Select file" ) );
  connect( mSelectButton, &QToolButton::clicked, this, &QgsProcessingMapLayerComboBox::triggerFileSelection );
  layout->addWidget( mSelectButton );
  layout->setAlignment( mSelectButton, Qt::AlignTop );

  QVBoxLayout *vl = new QVBoxLayout();
  vl->setMargin( 0 );
  vl->setContentsMargins( 0, 0, 0, 0 );
  vl->setSpacing( 6 );
  vl->addLayout( layout );

  QgsMapLayerProxyModel::Filters filters = nullptr;

  if ( mParameter->type() == QgsProcessingParameterFeatureSource::typeName() )
  {
    mUseSelectionCheckBox = new QCheckBox( tr( "Selected features only" ) );
    mUseSelectionCheckBox->setChecked( false );
    mUseSelectionCheckBox->setEnabled( false );
    vl->addWidget( mUseSelectionCheckBox );
  }

  if ( mParameter->type() == QgsProcessingParameterFeatureSource::typeName() || mParameter->type() == QgsProcessingParameterVectorLayer::typeName() )
  {
    QList<int> dataTypes;
    if ( mParameter->type() == QgsProcessingParameterFeatureSource::typeName() )
      dataTypes = static_cast< QgsProcessingParameterFeatureSource *>( mParameter.get() )->dataTypes();
    else if ( mParameter->type() == QgsProcessingParameterVectorLayer::typeName() )
      dataTypes = static_cast< QgsProcessingParameterVectorLayer *>( mParameter.get() )->dataTypes();

    if ( dataTypes.contains( QgsProcessing::TypeVectorAnyGeometry ) || dataTypes.isEmpty() )
      filters = QgsMapLayerProxyModel::HasGeometry;
    if ( dataTypes.contains( QgsProcessing::TypeVectorPoint ) )
      filters |= QgsMapLayerProxyModel::PointLayer;
    if ( dataTypes.contains( QgsProcessing::TypeVectorLine ) )
      filters |= QgsMapLayerProxyModel::LineLayer;
    if ( dataTypes.contains( QgsProcessing::TypeVectorPolygon ) )
      filters |= QgsMapLayerProxyModel::PolygonLayer;
    if ( !filters )
      filters = QgsMapLayerProxyModel::VectorLayer;
  }
  else if ( mParameter->type() == QgsProcessingParameterRasterLayer::typeName() )
  {
    filters = QgsMapLayerProxyModel::RasterLayer;
  }
  else if ( mParameter->type() == QgsProcessingParameterMeshLayer::typeName() )
  {
    filters = QgsMapLayerProxyModel::MeshLayer;
  }

  QgsSettings settings;
  if ( settings.value( QStringLiteral( "Processing/Configuration/SHOW_CRS_DEF" ), true ).toBool() )
    mCombo->setShowCrs( true );

  if ( filters )
    mCombo->setFilters( filters );
  mCombo->setExcludedProviders( QStringList() << QStringLiteral( "grass" ) ); // not sure if this is still required...

  if ( mParameter->flags() & QgsProcessingParameterDefinition::FlagOptional )
  {
    mCombo->setAllowEmptyLayer( true );
    mCombo->setLayer( nullptr );
  }

  connect( mCombo, &QgsMapLayerComboBox::layerChanged, this, &QgsProcessingMapLayerComboBox::onLayerChanged );
  if ( mUseSelectionCheckBox )
    connect( mUseSelectionCheckBox, &QCheckBox::toggled, this, [ = ]
  {
    if ( !mBlockChangedSignal )
      emit valueChanged();
  } );

  setLayout( vl );

  setAcceptDrops( true );

  onLayerChanged( mCombo->currentLayer() );
}

QgsProcessingMapLayerComboBox::~QgsProcessingMapLayerComboBox() = default;

void QgsProcessingMapLayerComboBox::setLayer( QgsMapLayer *layer )
{
  if ( layer || mParameter->flags() & QgsProcessingParameterDefinition::FlagOptional )
    mCombo->setLayer( layer );
}

QgsMapLayer *QgsProcessingMapLayerComboBox::currentLayer()
{
  return mCombo->currentLayer();
}

QString QgsProcessingMapLayerComboBox::currentText()
{
  return mCombo->currentText();
}

void QgsProcessingMapLayerComboBox::setValue( const QVariant &value, QgsProcessingContext &context )
{
  QVariant val = value;
  bool found = false;
  bool selectedOnly = false;
  if ( val.canConvert<QgsProcessingFeatureSourceDefinition>() )
  {
    QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( val );
    val = fromVar.source;
    selectedOnly = fromVar.selectedFeaturesOnly;
  }

  if ( val.canConvert<QgsProperty>() )
  {
    if ( val.value< QgsProperty >().propertyType() == QgsProperty::StaticProperty )
    {
      val = val.value< QgsProperty >().staticValue();
    }
    else
    {
      val = val.value< QgsProperty >().valueAsString( context.expressionContext(), mParameter->defaultValue().toString() );
    }
  }

  QgsMapLayer *layer = qobject_cast< QgsMapLayer * >( val.value< QObject * >() );
  if ( !layer && val.type() == QVariant::String )
  {
    layer = QgsProcessingUtils::mapLayerFromString( val.toString(), context, false );
  }

  if ( layer )
  {
    mBlockChangedSignal++;
    QgsMapLayer *prevLayer = currentLayer();
    setLayer( layer );
    found = static_cast< bool >( currentLayer() );
    bool changed = found && ( currentLayer() != prevLayer );
    if ( found && mUseSelectionCheckBox )
    {
      const bool hasSelection = qobject_cast< QgsVectorLayer * >( layer ) && qobject_cast< QgsVectorLayer * >( layer )->selectedFeatureCount() > 0;
      changed = changed | ( ( hasSelection && selectedOnly ) != mUseSelectionCheckBox->isChecked() );
      if ( hasSelection )
      {
        mUseSelectionCheckBox->setEnabled( true );
        mUseSelectionCheckBox->setChecked( selectedOnly );
      }
      else
      {
        mUseSelectionCheckBox->setChecked( false );
        mUseSelectionCheckBox->setEnabled( false );
      }
    }
    mBlockChangedSignal--;
    if ( changed )
      emit valueChanged(); // and ensure we only ever raise one
  }

  if ( !found )
  {
    const QString string = val.toString();
    if ( mUseSelectionCheckBox )
    {
      mUseSelectionCheckBox->setChecked( false );
      mUseSelectionCheckBox->setEnabled( false );
    }
    if ( !string.isEmpty() )
    {
      mBlockChangedSignal++;
      if ( mCombo->findText( string ) < 0 )
      {
        QStringList additional = mCombo->additionalItems();
        additional.append( string );
        mCombo->setAdditionalItems( additional );
      }
      mCombo->setCurrentIndex( mCombo->findText( string ) ); // this may or may not throw a signal, so let's block it..
      mBlockChangedSignal--;
      if ( !mBlockChangedSignal )
        emit valueChanged(); // and ensure we only ever raise one
    }
    else if ( mParameter->flags() & QgsProcessingParameterDefinition::FlagOptional )
    {
      mCombo->setLayer( nullptr );
    }
  }
}

QVariant QgsProcessingMapLayerComboBox::value() const
{
  if ( QgsMapLayer *layer = mCombo->currentLayer() )
  {
    if ( mUseSelectionCheckBox && mUseSelectionCheckBox->isChecked() )
      return QgsProcessingFeatureSourceDefinition( layer->id(), true );
    else
      return layer->id();
  }
  else
  {
    if ( !mCombo->currentText().isEmpty() )
    {
      if ( mUseSelectionCheckBox && mUseSelectionCheckBox->isChecked() )
        return QgsProcessingFeatureSourceDefinition( mCombo->currentText(), true );
      else
        return mCombo->currentText();
    }
  }
  return QVariant();
}


QgsMapLayer *QgsProcessingMapLayerComboBox::compatibleMapLayerFromMimeData( const QMimeData *data, bool &incompatibleLayerSelected ) const
{
  incompatibleLayerSelected = false;
  const QgsMimeDataUtils::UriList uriList = QgsMimeDataUtils::decodeUriList( data );
  for ( const QgsMimeDataUtils::Uri &u : uriList )
  {
    // is this uri from the current project?
    if ( QgsMapLayer *layer = u.mapLayer() )
    {
      if ( mCombo->mProxyModel->acceptsLayer( layer ) )
        return layer;
      else
      {
        incompatibleLayerSelected = true;
        return nullptr;
      }
    }
  }
  return nullptr;
}


QString QgsProcessingMapLayerComboBox::compatibleUriFromMimeData( const QMimeData *data ) const
{
  const QgsMimeDataUtils::UriList uriList = QgsMimeDataUtils::decodeUriList( data );
  for ( const QgsMimeDataUtils::Uri &u : uriList )
  {
    if ( ( mParameter->type() == QgsProcessingParameterFeatureSource::typeName()
           || mParameter->type() == QgsProcessingParameterVectorLayer::typeName()
           || mParameter->type() == QgsProcessingParameterMapLayer::typeName() )
         && u.layerType == QLatin1String( "vector" ) && u.providerKey == QLatin1String( "ogr" ) )
    {
      QList< int > dataTypes =  mParameter->type() == QgsProcessingParameterFeatureSource::typeName() ? static_cast< QgsProcessingParameterFeatureSource * >( mParameter.get() )->dataTypes()
                                : ( mParameter->type() == QgsProcessingParameterVectorLayer::typeName() ? static_cast<QgsProcessingParameterVectorLayer *>( mParameter.get() )->dataTypes()
                                    : QList< int >() );
      switch ( QgsWkbTypes::geometryType( u.wkbType ) )
      {
        case QgsWkbTypes::UnknownGeometry:
          return u.uri;

        case QgsWkbTypes::PointGeometry:
          if ( dataTypes.isEmpty() || dataTypes.contains( QgsProcessing::TypeVector ) || dataTypes.contains( QgsProcessing::TypeVectorAnyGeometry ) || dataTypes.contains( QgsProcessing::TypeVectorPoint ) )
            return u.uri;
          break;

        case QgsWkbTypes::LineGeometry:
          if ( dataTypes.isEmpty() || dataTypes.contains( QgsProcessing::TypeVector ) || dataTypes.contains( QgsProcessing::TypeVectorAnyGeometry ) || dataTypes.contains( QgsProcessing::TypeVectorLine ) )
            return u.uri;
          break;

        case QgsWkbTypes::PolygonGeometry:
          if ( dataTypes.isEmpty() || dataTypes.contains( QgsProcessing::TypeVector ) || dataTypes.contains( QgsProcessing::TypeVectorAnyGeometry ) || dataTypes.contains( QgsProcessing::TypeVectorPolygon ) )
            return u.uri;
          break;

        case QgsWkbTypes::NullGeometry:
          if ( dataTypes.contains( QgsProcessing::TypeVector ) )
            return u.uri;
          break;
      }
    }
    else if ( ( mParameter->type() == QgsProcessingParameterRasterLayer::typeName()
                || mParameter->type() == QgsProcessingParameterMapLayer::typeName() )
              && u.layerType == QLatin1String( "raster" ) && u.providerKey == QLatin1String( "gdal" ) )
      return u.uri;
    else if ( ( mParameter->type() == QgsProcessingParameterMeshLayer::typeName()
                || mParameter->type() == QgsProcessingParameterMapLayer::typeName() )
              && u.layerType == QLatin1String( "mesh" ) && u.providerKey == QLatin1String( "mdal" ) )
      return u.uri;
  }
  if ( !uriList.isEmpty() )
    return QString();

  // second chance -- files dragged from file explorer, outside of QGIS
  QStringList rawPaths;
  if ( data->hasUrls() )
  {
    const QList< QUrl > urls = data->urls();
    rawPaths.reserve( urls.count() );
    for ( const QUrl &url : urls )
    {
      const QString local =  url.toLocalFile();
      if ( !rawPaths.contains( local ) )
        rawPaths.append( local );
    }
  }
  if ( !data->text().isEmpty() && !rawPaths.contains( data->text() ) )
    rawPaths.append( data->text() );

  for ( const QString &path : qgis::as_const( rawPaths ) )
  {
    QFileInfo file( path );
    if ( file.isFile() )
    {
      // TODO - we should check to see if it's a valid extension for the parameter, but that's non-trivial
      return path;
    }
  }

  return QString();
}

void QgsProcessingMapLayerComboBox::dragEnterEvent( QDragEnterEvent *event )
{
  if ( !( event->possibleActions() & Qt::CopyAction ) )
    return;

  bool incompatibleLayerSelected = false;
  QgsMapLayer *layer = compatibleMapLayerFromMimeData( event->mimeData(), incompatibleLayerSelected );
  const QString uri = compatibleUriFromMimeData( event->mimeData() );
  if ( layer || ( !incompatibleLayerSelected && !uri.isEmpty() ) )
  {
    // dragged an acceptable layer, phew
    event->setDropAction( Qt::CopyAction );
    event->accept();
    mDragActive = true;
    mCombo->mHighlight = true;
    update();
  }
}

void QgsProcessingMapLayerComboBox::dragLeaveEvent( QDragLeaveEvent *event )
{
  QWidget::dragLeaveEvent( event );
  if ( mDragActive )
  {
    event->accept();
    mDragActive = false;
    mCombo->mHighlight = false;
    update();
  }
}

void QgsProcessingMapLayerComboBox::dropEvent( QDropEvent *event )
{
  if ( !( event->possibleActions() & Qt::CopyAction ) )
    return;

  bool incompatibleLayerSelected = false;
  QgsMapLayer *layer = compatibleMapLayerFromMimeData( event->mimeData(), incompatibleLayerSelected );
  const QString uri = compatibleUriFromMimeData( event->mimeData() );
  if ( layer || ( !incompatibleLayerSelected && !uri.isEmpty() ) )
  {
    // dropped an acceptable layer, phew
    setFocus( Qt::MouseFocusReason );
    event->setDropAction( Qt::CopyAction );
    event->accept();
    QgsProcessingContext context;
    setValue( layer ? QVariant::fromValue( layer ) : QVariant::fromValue( uri ), context );
  }
  mDragActive = false;
  mCombo->mHighlight = false;
  update();
}

void QgsProcessingMapLayerComboBox::onLayerChanged( QgsMapLayer *layer )
{
  if ( mParameter->type() == QgsProcessingParameterFeatureSource::typeName() )
  {
    if ( QgsVectorLayer *vl = qobject_cast< QgsVectorLayer * >( layer ) )
    {
      if ( QgsVectorLayer *prevLayer = qobject_cast< QgsVectorLayer * >( mPrevLayer ) )
      {
        disconnect( prevLayer, &QgsVectorLayer::selectionChanged, this, &QgsProcessingMapLayerComboBox::selectionChanged );
      }
      if ( vl->selectedFeatureCount() == 0 )
        mUseSelectionCheckBox->setChecked( false );
      mUseSelectionCheckBox->setEnabled( vl->selectedFeatureCount() > 0 );
      connect( vl, &QgsVectorLayer::selectionChanged, this, &QgsProcessingMapLayerComboBox::selectionChanged );
    }
  }

  mPrevLayer = layer;
  if ( !mBlockChangedSignal )
    emit valueChanged();
}

void QgsProcessingMapLayerComboBox::selectionChanged( const QgsFeatureIds &selected, const QgsFeatureIds &, bool )
{
  if ( selected.isEmpty() )
    mUseSelectionCheckBox->setChecked( false );
  mUseSelectionCheckBox->setEnabled( !selected.isEmpty() );
}



///@endcond
