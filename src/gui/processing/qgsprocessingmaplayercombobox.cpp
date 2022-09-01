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
#include "qgsapplication.h"
#include "qgsguiutils.h"
#include "qgspanelwidget.h"
#include "qgsprocessingfeaturesourceoptionswidget.h"
#include "qgsdatasourceselectdialog.h"
#include "qgsprocessingwidgetwrapper.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>
#include <QCheckBox>
#include <QDragEnterEvent>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QUrl>

///@cond PRIVATE

QgsProcessingMapLayerComboBox::QgsProcessingMapLayerComboBox( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QWidget( parent )
  , mParameter( parameter->clone() )
{
  QHBoxLayout *layout = new QHBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->setSpacing( 6 );

  mCombo = new QgsMapLayerComboBox();
  layout->addWidget( mCombo );
  layout->setAlignment( mCombo, Qt::AlignTop );

  int iconSize = QgsGuiUtils::scaleIconSize( 24 );
  if ( mParameter->type() == QgsProcessingParameterFeatureSource::typeName() && type == QgsProcessingGui::Standard )
  {
    mIterateButton = new QToolButton();
    mIterateButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mIconIterate.svg" ) ) );
    mIterateButton->setToolTip( tr( "Iterate over this layer, creating a separate output for every feature in the layer" ) );
    mIterateButton->setCheckable( true );
    mIterateButton->setAutoRaise( true );

    // button width is 1.25 * icon size, height 1.1 * icon size. But we round to ensure even pixel sizes for equal margins
    mIterateButton->setFixedSize( 2 * static_cast< int >( 1.25 * iconSize / 2.0 ), 2 * static_cast< int >( iconSize * 1.1 / 2.0 ) );
    mIterateButton->setIconSize( QSize( iconSize, iconSize ) );

    layout->addWidget( mIterateButton );
    layout->setAlignment( mIterateButton, Qt::AlignTop );
  }

  if ( mParameter->type() == QgsProcessingParameterFeatureSource::typeName() )
  {
    mSettingsButton = new QToolButton();
    mSettingsButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionOptions.svg" ) ) );
    mSettingsButton->setToolTip( tr( "Advanced options" ) );

    // button width is 1.25 * icon size, height 1.1 * icon size. But we round to ensure even pixel sizes for equal margins
    mSettingsButton->setFixedSize( 2 * static_cast< int >( 1.25 * iconSize / 2.0 ), 2 * static_cast< int >( iconSize * 1.1 / 2.0 ) );
    mSettingsButton->setIconSize( QSize( iconSize, iconSize ) );
    mSettingsButton->setAutoRaise( true );

    connect( mSettingsButton, &QToolButton::clicked, this, &QgsProcessingMapLayerComboBox::showSourceOptions );
    layout->addWidget( mSettingsButton );
    layout->setAlignment( mSettingsButton, Qt::AlignTop );
  }

  mSelectButton = new QToolButton();
  mSelectButton->setText( QString( QChar( 0x2026 ) ) );
  mSelectButton->setToolTip( tr( "Select input" ) );
  layout->addWidget( mSelectButton );
  layout->setAlignment( mSelectButton, Qt::AlignTop );
  if ( mParameter->type() == QgsProcessingParameterFeatureSource::typeName() || mParameter->type() == QgsProcessingParameterVectorLayer::typeName() )
  {
    mFeatureSourceMenu = new QMenu( this );
    QAction *selectFromFileAction = new QAction( tr( "Select File…" ), mFeatureSourceMenu );
    connect( selectFromFileAction, &QAction::triggered, this, &QgsProcessingMapLayerComboBox::selectFromFile );
    mFeatureSourceMenu->addAction( selectFromFileAction );
    QAction *browseForLayerAction = new QAction( tr( "Browse for Layer…" ), mFeatureSourceMenu );
    connect( browseForLayerAction, &QAction::triggered, this, &QgsProcessingMapLayerComboBox::browseForLayer );
    mFeatureSourceMenu->addAction( browseForLayerAction );
    mSelectButton->setMenu( mFeatureSourceMenu );
    mSelectButton->setPopupMode( QToolButton::InstantPopup );
  }
  else
  {
    connect( mSelectButton, &QToolButton::clicked, this, &QgsProcessingMapLayerComboBox::selectFromFile );
  }

  QVBoxLayout *vl = new QVBoxLayout();
  vl->setContentsMargins( 0, 0, 0, 0 );
  vl->setSpacing( 6 );
  vl->addLayout( layout );

  QgsMapLayerProxyModel::Filters filters = QgsMapLayerProxyModel::Filters();

  if ( mParameter->type() == QgsProcessingParameterFeatureSource::typeName() && type == QgsProcessingGui::Standard )
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
  else if ( mParameter->type() == QgsProcessingParameterPointCloudLayer::typeName() )
  {
    filters = QgsMapLayerProxyModel::PointCloudLayer;
  }
  else if ( mParameter->type() == QgsProcessingParameterMapLayer::typeName() )
  {
    QList<int> dataTypes;
    dataTypes = static_cast< QgsProcessingParameterMapLayer *>( mParameter.get() )->dataTypes();

    if ( dataTypes.contains( QgsProcessing::TypeVectorAnyGeometry ) )
      filters |= QgsMapLayerProxyModel::HasGeometry;
    if ( dataTypes.contains( QgsProcessing::TypeVectorPoint ) )
      filters |= QgsMapLayerProxyModel::PointLayer;
    if ( dataTypes.contains( QgsProcessing::TypeVectorLine ) )
      filters |= QgsMapLayerProxyModel::LineLayer;
    if ( dataTypes.contains( QgsProcessing::TypeVectorPolygon ) )
      filters |= QgsMapLayerProxyModel::PolygonLayer;
    if ( dataTypes.contains( QgsProcessing::TypeRaster ) )
      filters |= QgsMapLayerProxyModel::RasterLayer;
    if ( dataTypes.contains( QgsProcessing::TypeMesh ) )
      filters |= QgsMapLayerProxyModel::MeshLayer;
    if ( dataTypes.contains( QgsProcessing::TypePointCloud ) )
      filters |= QgsMapLayerProxyModel::PointCloudLayer;
    if ( !filters )
      filters = QgsMapLayerProxyModel::All;
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
  if ( !value.isValid()  && mParameter->flags() & QgsProcessingParameterDefinition::FlagOptional )
  {
    setLayer( nullptr );
    return;
  }

  QVariant val = value;
  bool found = false;
  bool selectedOnly = false;
  bool iterate = false;
  if ( val.userType() == QMetaType::type( "QgsProcessingFeatureSourceDefinition" ) )
  {
    QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( val );
    val = fromVar.source;
    selectedOnly = fromVar.selectedFeaturesOnly;
    iterate = fromVar.flags & QgsProcessingFeatureSourceDefinition::Flag::FlagCreateIndividualOutputPerInputFeature;
    mFeatureLimit = fromVar.featureLimit;
    mIsOverridingDefaultGeometryCheck = fromVar.flags & QgsProcessingFeatureSourceDefinition::Flag::FlagOverrideDefaultGeometryCheck;
    mGeometryCheck = fromVar.geometryCheck;
  }
  else
  {
    mFeatureLimit = -1;
    mIsOverridingDefaultGeometryCheck = false;
    mGeometryCheck = QgsFeatureRequest::GeometryAbortOnInvalid;
  }

  if ( val.userType() == QMetaType::type( "QgsProperty" ) )
  {
    if ( val.value< QgsProperty >().propertyType() == QgsProperty::StaticProperty )
    {
      val = val.value< QgsProperty >().staticValue();
    }
    else
    {
      val = val.value< QgsProperty >().valueAsString( context.expressionContext(), mParameter->defaultValueForGui().toString() );
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

      if ( mIterateButton )
      {
        mIterateButton->setChecked( iterate );
      }
    }
    mBlockChangedSignal--;
    if ( changed )
      emit valueChanged(); // and ensure we only ever raise one
  }

  if ( !found )
  {
    const QString string = val.toString();
    if ( mIterateButton )
      mIterateButton->setChecked( iterate );

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
      if ( mUseSelectionCheckBox )
      {
        mUseSelectionCheckBox->setChecked( false );
        mUseSelectionCheckBox->setEnabled( false );
      }
      mBlockChangedSignal--;
      if ( !mBlockChangedSignal )
        emit valueChanged(); // and ensure we only ever raise one
    }
    else if ( mParameter->flags() & QgsProcessingParameterDefinition::FlagOptional )
    {
      mCombo->setLayer( nullptr );
      if ( mUseSelectionCheckBox )
      {
        mUseSelectionCheckBox->setChecked( false );
        mUseSelectionCheckBox->setEnabled( false );
      }
    }
  }
}

QVariant QgsProcessingMapLayerComboBox::value() const
{
  if ( isEditable() && mCombo->currentText() != mCombo->itemText( mCombo->currentIndex() ) )
    return mCombo->currentText();

  const bool iterate = mIterateButton && mIterateButton->isChecked();
  const bool selectedOnly = mUseSelectionCheckBox && mUseSelectionCheckBox->isChecked();
  if ( QgsMapLayer *layer = mCombo->currentLayer() )
  {
    if ( selectedOnly || iterate || mFeatureLimit != -1 || mIsOverridingDefaultGeometryCheck )
      return QgsProcessingFeatureSourceDefinition( layer->id(), selectedOnly, mFeatureLimit,
             ( iterate ? QgsProcessingFeatureSourceDefinition::Flag::FlagCreateIndividualOutputPerInputFeature : QgsProcessingFeatureSourceDefinition::Flags() )
             | ( mIsOverridingDefaultGeometryCheck ? QgsProcessingFeatureSourceDefinition::Flag::FlagOverrideDefaultGeometryCheck : QgsProcessingFeatureSourceDefinition::Flags() ),
             mGeometryCheck );
    else
      return layer->id();
  }
  else
  {
    if ( !mCombo->currentText().isEmpty() )
    {
      if ( selectedOnly || iterate || mFeatureLimit != -1 || mIsOverridingDefaultGeometryCheck )
        return QgsProcessingFeatureSourceDefinition( mCombo->currentText(), selectedOnly, mFeatureLimit,
               ( iterate ? QgsProcessingFeatureSourceDefinition::Flag::FlagCreateIndividualOutputPerInputFeature : QgsProcessingFeatureSourceDefinition::Flags() )
               | ( mIsOverridingDefaultGeometryCheck ? QgsProcessingFeatureSourceDefinition::Flag::FlagOverrideDefaultGeometryCheck : QgsProcessingFeatureSourceDefinition::Flags() ),
               mGeometryCheck );
      else
        return mCombo->currentText();
    }
  }
  return QVariant();
}

void QgsProcessingMapLayerComboBox::setWidgetContext( const QgsProcessingParameterWidgetContext &context )
{
  mBrowserModel = context.browserModel();
  mCombo->setProject( context.project() );
}

void QgsProcessingMapLayerComboBox::setEditable( bool editable )
{
  mCombo->setEditable( editable );
}

bool QgsProcessingMapLayerComboBox::isEditable() const
{
  return mCombo->isEditable();
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
           || mParameter->type() == QgsProcessingParameterVectorLayer::typeName() )
         && u.layerType == QLatin1String( "vector" ) )
    {
      QList< int > dataTypes =  mParameter->type() == QgsProcessingParameterFeatureSource::typeName() ? static_cast< QgsProcessingParameterFeatureSource * >( mParameter.get() )->dataTypes()
                                : ( mParameter->type() == QgsProcessingParameterVectorLayer::typeName() ? static_cast<QgsProcessingParameterVectorLayer *>( mParameter.get() )->dataTypes()
                                    : QList< int >() );
      bool acceptable = false;
      switch ( QgsWkbTypes::geometryType( u.wkbType ) )
      {
        case QgsWkbTypes::UnknownGeometry:
          acceptable = true;
          break;

        case QgsWkbTypes::PointGeometry:
          if ( dataTypes.isEmpty() || dataTypes.contains( QgsProcessing::TypeVector ) || dataTypes.contains( QgsProcessing::TypeVectorAnyGeometry ) || dataTypes.contains( QgsProcessing::TypeVectorPoint ) )
            acceptable = true;
          break;

        case QgsWkbTypes::LineGeometry:
          if ( dataTypes.isEmpty() || dataTypes.contains( QgsProcessing::TypeVector ) || dataTypes.contains( QgsProcessing::TypeVectorAnyGeometry ) || dataTypes.contains( QgsProcessing::TypeVectorLine ) )
            acceptable = true;
          break;

        case QgsWkbTypes::PolygonGeometry:
          if ( dataTypes.isEmpty() || dataTypes.contains( QgsProcessing::TypeVector ) || dataTypes.contains( QgsProcessing::TypeVectorAnyGeometry ) || dataTypes.contains( QgsProcessing::TypeVectorPolygon ) )
            acceptable = true;
          break;

        case QgsWkbTypes::NullGeometry:
          if ( dataTypes.contains( QgsProcessing::TypeVector ) )
            acceptable = true;
          break;
      }
      if ( acceptable )
        return u.providerKey != QLatin1String( "ogr" ) ? QgsProcessingUtils::encodeProviderKeyAndUri( u.providerKey, u.uri ) : u.uri;
    }
    else if ( mParameter->type() == QgsProcessingParameterRasterLayer::typeName()
              && u.layerType == QLatin1String( "raster" ) && u.providerKey == QLatin1String( "gdal" ) )
      return u.uri;
    else if ( mParameter->type() == QgsProcessingParameterMeshLayer::typeName()
              && u.layerType == QLatin1String( "mesh" ) && u.providerKey == QLatin1String( "mdal" ) )
      return u.uri;
    else if ( mParameter->type() == QgsProcessingParameterMapLayer::typeName() )
    {
      QList< int > dataTypes = static_cast< QgsProcessingParameterMapLayer * >( mParameter.get() )->dataTypes();
      if ( dataTypes.isEmpty() || dataTypes.contains( QgsProcessing::TypeMapLayer ) )
      {
        return u.uri;
      }

      if ( u.layerType == QLatin1String( "vector" ) && u.providerKey == QLatin1String( "ogr" ) )
      {
        switch ( QgsWkbTypes::geometryType( u.wkbType ) )
        {
          case QgsWkbTypes::UnknownGeometry:
            return u.uri;

          case QgsWkbTypes::PointGeometry:
            if ( dataTypes.contains( QgsProcessing::TypeVectorAnyGeometry ) || dataTypes.contains( QgsProcessing::TypeVectorPoint ) )
              return u.uri;
            break;

          case QgsWkbTypes::LineGeometry:
            if ( dataTypes.contains( QgsProcessing::TypeVectorAnyGeometry ) || dataTypes.contains( QgsProcessing::TypeVectorLine ) )
              return u.uri;
            break;

          case QgsWkbTypes::PolygonGeometry:
            if ( dataTypes.contains( QgsProcessing::TypeVectorAnyGeometry ) || dataTypes.contains( QgsProcessing::TypeVectorPolygon ) )
              return u.uri;
            break;

          case QgsWkbTypes::NullGeometry:
            return u.uri;
        }
      }
      else if ( u.layerType == QLatin1String( "raster" ) && u.providerKey == QLatin1String( "gdal" )
                && dataTypes.contains( QgsProcessing::TypeRaster ) )
        return u.uri;
      else if ( u.layerType == QLatin1String( "mesh" ) && u.providerKey == QLatin1String( "mdal" )
                && dataTypes.contains( QgsProcessing::TypeMesh ) )
        return u.uri;
    }
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

  for ( const QString &path : std::as_const( rawPaths ) )
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
  if ( mUseSelectionCheckBox && mParameter->type() == QgsProcessingParameterFeatureSource::typeName() )
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

void QgsProcessingMapLayerComboBox::showSourceOptions()
{
  if ( QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this ) )
  {
    QgsProcessingFeatureSourceOptionsWidget *widget = new QgsProcessingFeatureSourceOptionsWidget();
    widget->setPanelTitle( tr( "%1 Options" ).arg( mParameter->description() ) );

    widget->setGeometryCheckMethod( mIsOverridingDefaultGeometryCheck, mGeometryCheck );
    widget->setFeatureLimit( mFeatureLimit );

    panel->openPanel( widget );

    connect( widget, &QgsPanelWidget::widgetChanged, this, [ = ]
    {
      bool changed = false;
      changed = changed | ( widget->featureLimit() != mFeatureLimit );
      changed = changed | ( widget->isOverridingInvalidGeometryCheck() != mIsOverridingDefaultGeometryCheck );
      changed = changed | ( widget->geometryCheckMethod() != mGeometryCheck );

      mFeatureLimit = widget->featureLimit();
      mIsOverridingDefaultGeometryCheck = widget->isOverridingInvalidGeometryCheck();
      mGeometryCheck = widget->geometryCheckMethod();

      if ( changed )
        emit valueChanged();
    } );
  }
}

void QgsProcessingMapLayerComboBox::selectFromFile()
{
  QgsSettings settings;
  const QString initialValue = currentText();
  QString path;

  if ( QFileInfo( initialValue ).isDir() && QFileInfo::exists( initialValue ) )
    path = initialValue;
  else if ( QFileInfo::exists( QFileInfo( initialValue ).path() ) && QFileInfo( initialValue ).path() != '.' )
    path = QFileInfo( initialValue ).path();
  else if ( settings.contains( QStringLiteral( "/Processing/LastInputPath" ) ) )
    path = settings.value( QStringLiteral( "/Processing/LastInputPath" ) ).toString();

  QString filter;
  if ( const QgsFileFilterGenerator *generator = dynamic_cast< const QgsFileFilterGenerator * >( mParameter.get() ) )
    filter = generator->createFileFilter();
  else
    filter = QObject::tr( "All files (*.*)" );

  const QString filename = QFileDialog::getOpenFileName( this, tr( "Select File" ), path, filter );
  if ( filename.isEmpty() )
    return;

  settings.setValue( QStringLiteral( "/Processing/LastInputPath" ), QFileInfo( filename ).path() );
  QgsProcessingContext context;
  setValue( filename, context );
}

void QgsProcessingMapLayerComboBox::browseForLayer()
{
  if ( QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this ) )
  {
    QgsDataSourceSelectWidget *widget = new QgsDataSourceSelectWidget( mBrowserModel, true, QgsMapLayerType::VectorLayer );
    widget->setPanelTitle( tr( "Browse for \"%1\"" ).arg( mParameter->description() ) );

    panel->openPanel( widget );

    connect( widget, &QgsDataSourceSelectWidget::itemTriggered, this, [ = ]( const QgsMimeDataUtils::Uri & )
    {
      widget->acceptPanel();
    } );
    connect( widget, &QgsPanelWidget::panelAccepted, this, [ = ]()
    {
      QgsProcessingContext context;
      if ( widget->uri().uri.isEmpty() )
        setValue( QVariant(), context );
      else if ( widget->uri().providerKey == QLatin1String( "ogr" ) )
        setValue( widget->uri().uri, context );
      else
        setValue( QgsProcessingUtils::encodeProviderKeyAndUri( widget->uri().providerKey, widget->uri().uri ), context );
    } );
  }
}



///@endcond
