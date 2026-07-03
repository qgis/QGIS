/***************************************************************************
    qgs3dpointcloudeditingtoolbar.cpp
    -------------------
    begin                : July 2026
    copyright            : (C) 2026 Oslandia, Belgacem Nedjima
    email                : benoit dot de dot mezzo at oslandia dot com
                         : belgacem dot nedjima at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dpointcloudeditingtoolbar.h"

#include <QString>

using namespace Qt::StringLiterals;

//#include "moc_qgs3dpointcloudeditingtoolbar.cpp"

#include <QAction>
#include <QShortcut>
#include <QWidget>
#include <QActionGroup>
#include <QComboBox>
#include <QMenu>
#include <QPointer>
#include <QLabel>

#include "qgsapplication.h"
#include "qgspointcloudlayer.h"
#include "qgisapp.h"
#include "qgsdoublespinbox.h"
#include "qgs3dmapcanvas.h"
#include "qgs3dmaptoolpointcloudchangeattributepaintbrush.h"
#include "qgs3dmaptoolpointcloudchangeattributepolygon.h"
#include "qgsmessagebar.h"
#include "qgspointcloudquerybuilder.h"
#include "qgspointclouddataprovider.h"
#include "qgspointcloudlayer3drenderer.h"

Qgs3DPointCloudEditingToolBar::Qgs3DPointCloudEditingToolBar( Qgs3DMapCanvasWidget *parent )
  : Qgs3DEditingToolBar( u"Point cloud editing"_s, parent )
{
  setObjectName( u"3DPointCloudEditingToolBar"_s );

  mEditingToolsMenu = new QMenu( this );
  mEditingToolsMenu->setObjectName( u"m3DEditingToolsMenu"_s );

  mEditingToolsAction = new QAction( QgsApplication::getThemeIcon( u"mActionSelectPolygon.svg"_s ), tr( "Select Editing Tool" ), this );
  mEditingToolsAction->setObjectName( u"m3DActionSelectEditingTool"_s );
  mEditingToolsAction->setMenu( mEditingToolsMenu );
  /*mEditingToolBar->*/ addAction( mEditingToolsAction );

  QToolButton *editingToolsButton = qobject_cast<QToolButton *>( /*mEditingToolBar->*/ widgetForAction( mEditingToolsAction ) );
  editingToolsButton->setPopupMode( QToolButton::ToolButtonPopupMode::InstantPopup );
  QAction *actionPointCloudChangeAttributeTool
    = mEditingToolsMenu->addAction( QIcon( QgsApplication::iconPath( u"mActionSelectPolygon.svg"_s ) ), tr( "Select by Polygon" ), this, &Qgs3DPointCloudEditingToolBar::changePointCloudAttributeByPolygon );
  actionPointCloudChangeAttributeTool->setObjectName( u"m3DActionSelectByPolygon"_s );
  QAction *actionPaintbrush
    = mEditingToolsMenu->addAction( QIcon( QgsApplication::iconPath( u"propertyicons/rendering.svg"_s ) ), tr( "Select by Paintbrush" ), this, &Qgs3DPointCloudEditingToolBar::changePointCloudAttributeByPaintbrush );
  actionPaintbrush->setObjectName( u"m3DActionSelectByPaintbrush"_s );
  QAction *actionAboveLineTool
    = mEditingToolsMenu->addAction( QIcon( QgsApplication::iconPath( u"mActionSelectAboveLine.svg"_s ) ), tr( "Select Above Line" ), this, &Qgs3DPointCloudEditingToolBar::changePointCloudAttributeByAboveLine );
  actionAboveLineTool->setObjectName( u"m3DActionSelectAboveLine"_s );
  QAction *actionBelowLineTool
    = mEditingToolsMenu->addAction( QIcon( QgsApplication::iconPath( u"mActionSelectBelowLine.svg"_s ) ), tr( "Select Below Line" ), this, &Qgs3DPointCloudEditingToolBar::changePointCloudAttributeByBelowLine );
  actionBelowLineTool->setObjectName( u"m3DActionSelectBelowLine"_s );

  mGroupActions << actionPointCloudChangeAttributeTool << actionPaintbrush << actionAboveLineTool << actionBelowLineTool;

  QAction *actionPointFilter
    = addAction( QIcon( QgsApplication::iconPath( "mIconExpressionFilter.svg" ) ), tr( "Filter Points" ), this, &Qgs3DPointCloudEditingToolBar::changePointCloudAttributePointFilter );
  actionPointFilter->setObjectName( u"m3DActionFilterPoints"_s );
  actionPointFilter->setCheckable( true );
  const QString tooltip
    = u"%1\n\n%2\n%3"_s.arg( tr( "Filter Points" ), tr( "Set an expression to filter points that should be edited." ), tr( "Points that do not satisfy the expression will not be modified." ) );
  actionPointFilter->setToolTip( tooltip );

  addWidget( new QLabel( tr( "Attribute" ) ) )->setObjectName( u"m3DActionAttributeLabelAction"_s );
  mCboChangeAttribute = new QComboBox();
  mCboChangeAttribute->setObjectName( u"m3DCboChangeAttribute"_s );
  addWidget( mCboChangeAttribute )->setObjectName( u"m3DCboChangeAttributeAction"_s );
  mSpinChangeAttributeValue = new QgsDoubleSpinBox();
  mSpinChangeAttributeValue->setObjectName( u"m3DSpinChangeAttributeValue"_s );
  mSpinChangeAttributeValue->setShowClearButton( false );
  addWidget( new QLabel( tr( "Value" ) ) )->setObjectName( u"m3DActionValueLabel"_s );
  mSpinChangeAttributeValueAction = addWidget( mSpinChangeAttributeValue );
  mSpinChangeAttributeValueAction->setObjectName( u"m3DActionChangeAttributeValueSpin"_s );
  mSpinChangeAttributeValueAction->setVisible( false );
  mCboChangeAttributeValue = new QComboBox();
  mCboChangeAttributeValue->setObjectName( u"m3DCboChangeAttributeValue"_s );
  mCboChangeAttributeValue->setEditable( true );
  mClassValidator = new ClassValidator( this );
  mCboChangeAttributeValueAction = addWidget( mCboChangeAttributeValue );
  mCboChangeAttributeValueAction->setObjectName( u"m3DActionChangeAttributeValueCombo"_s );

  connect( mCboChangeAttribute, qOverload<int>( &QComboBox::currentIndexChanged ), this, [this]( int ) { onPointCloudChangeAttributeSettingsChanged(); } );
  connect( mCboChangeAttributeValue, qOverload<const QString &>( &QComboBox::currentTextChanged ), this, [this]( const QString &text ) {
    double newValue = 0;
    if ( mCboChangeAttributeValue->isEditable() )
    {
      const QStringList split = text.split( ' ' );
      if ( !split.isEmpty() )
      {
        newValue = split.constFirst().toDouble();
      }
    }
    else
    {
      newValue = mCboChangeAttributeValue->currentData().toDouble();
    }
    mMapToolChangeAttribute->setNewValue( newValue );
  } );
  connect( mSpinChangeAttributeValue, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, [this]( double ) { mMapToolChangeAttribute->setNewValue( mSpinChangeAttributeValue->value() ); } );

  mMapToolChangeAttribute = new Qgs3DMapToolPointCloudChangeAttribute( mapCanvas3D() );
}

bool Qgs3DPointCloudEditingToolBar::accept( QgsMapLayer *layer ) const
{
  if ( layer != nullptr && layer->type() == Qgis::LayerType::PointCloud )
  {
    QgsPointCloudLayer *pcLayer = qobject_cast<QgsPointCloudLayer *>( layer );
    for ( QgsPointCloudSubIndex &subIndex : pcLayer->subIndexes() )
    {
      if ( !subIndex.index() || !subIndex.index().isValid() )
      {
        QgisApp::instance()
          ->messageBar()
          ->pushMessage( tr( "Virtual Point Cloud editing" ), tr( "Some of the files referenced by the VPC have not yet been loaded, selection of areas of a not yet loaded dataset will not cause any changes to its data. Only actually selected points will be edited." ) );
        break;
      }
    }
    return true;
  }
  return false;
}


void Qgs3DPointCloudEditingToolBar::changePointCloudAttributeByPaintbrush()
{
  const QAction *action = qobject_cast<QAction *>( sender() );
  if ( !action )
    return;

  mapCanvas3D()->requestActivate();
  mMapToolChangeAttribute->deleteLater();
  mMapToolChangeAttribute = new Qgs3DMapToolPointCloudChangeAttributePaintbrush( mapCanvas3D() );
  onPointCloudChangeAttributeSettingsChanged();
  mapCanvas3D()->setMapTool( mMapToolChangeAttribute );
  mEditingToolsAction->setIcon( action->icon() );
}

void Qgs3DPointCloudEditingToolBar::changePointCloudAttributeByPolygon()
{
  const QAction *action = qobject_cast<QAction *>( sender() );
  if ( !action )
    return;

  mMapToolChangeAttribute->deleteLater();
  mMapToolChangeAttribute = new Qgs3DMapToolPointCloudChangeAttributePolygon( mapCanvas3D(), Qgs3DMapToolPointCloudChangeAttributePolygon::Polygon );
  onPointCloudChangeAttributeSettingsChanged();
  mapCanvas3D()->setMapTool( mMapToolChangeAttribute );
  mEditingToolsAction->setIcon( action->icon() );
}

void Qgs3DPointCloudEditingToolBar::changePointCloudAttributeByAboveLine()
{
  const QAction *action = qobject_cast<QAction *>( sender() );
  if ( !action )
    return;

  mMapToolChangeAttribute->deleteLater();
  mMapToolChangeAttribute = new Qgs3DMapToolPointCloudChangeAttributePolygon( mapCanvas3D(), Qgs3DMapToolPointCloudChangeAttributePolygon::AboveLine );
  onPointCloudChangeAttributeSettingsChanged();
  mapCanvas3D()->setMapTool( mMapToolChangeAttribute );
  mEditingToolsAction->setIcon( action->icon() );
}

void Qgs3DPointCloudEditingToolBar::changePointCloudAttributeByBelowLine()
{
  const QAction *action = qobject_cast<QAction *>( sender() );
  if ( !action )
    return;

  mMapToolChangeAttribute->deleteLater();
  mMapToolChangeAttribute = new Qgs3DMapToolPointCloudChangeAttributePolygon( mapCanvas3D(), Qgs3DMapToolPointCloudChangeAttributePolygon::BelowLine );
  onPointCloudChangeAttributeSettingsChanged();
  mapCanvas3D()->setMapTool( mMapToolChangeAttribute );
  mEditingToolsAction->setIcon( action->icon() );
}

void Qgs3DPointCloudEditingToolBar::changePointCloudAttributePointFilter()
{
  QAction *action = qobject_cast<QAction *>( sender() );
  if ( !action )
    return;

  QgsPointCloudLayer *layer = qobject_cast<QgsPointCloudLayer *>( QgisApp::instance()->activeLayer() );
  if ( !layer )
    return;

  QgsPointCloudQueryBuilder qb( layer, this );
  qb.setSubsetString( mChangeAttributePointFilter );
  if ( qb.exec() )
  {
    mChangeAttributePointFilter = qb.subsetString();
    mMapToolChangeAttribute->setPointFilter( mChangeAttributePointFilter );
  }
  action->setChecked( !mChangeAttributePointFilter.isEmpty() );
  QString tooltip
    = u"%1\n\n%2\n%3"_s.arg( tr( "Filter Points" ), tr( "Set an expression to filter points that should be edited." ), tr( "Points that do not satisfy the expression will not be modified." ) );
  if ( !mChangeAttributePointFilter.isEmpty() )
    tooltip.append( u"\n%1\n%2"_s.arg( tr( "Current filter expression: " ), mChangeAttributePointFilter ) );
  action->setToolTip( tooltip );
}

void Qgs3DPointCloudEditingToolBar::deactivate()
{
  for ( auto action : findChildren<QAction *>() )
    action->setVisible( false );

  mEditingToolsAction->setEnabled( false );
  qDebug() << __FUNCTION__ << __LINE__ << "visible:" << isVisible();
  setEnabled( false );

  if ( mapCanvas3D()->mapTool() && mapCanvas3D()->mapTool() == mMapToolChangeAttribute )
    mapCanvas3D()->setMapTool( nullptr );
}

void Qgs3DPointCloudEditingToolBar::activate( QgsMapLayer *layer )
{
  QgsPointCloudLayer *pcLayer = qobject_cast<QgsPointCloudLayer *>( layer );
  const QVector<QgsPointCloudAttribute> attributes = pcLayer->attributes().attributes();
  const QString previousAttribute = mCboChangeAttribute->currentText();
  whileBlocking( mCboChangeAttribute )->clear();
  for ( const QgsPointCloudAttribute &attribute : attributes )
  {
    if ( attribute.name() == "X"_L1 || attribute.name() == "Y"_L1 || attribute.name() == "Z"_L1 )
      continue;

    whileBlocking( mCboChangeAttribute )->addItem( attribute.name() );
  }

  int index = mCboChangeAttribute->findText( previousAttribute );
  if ( index < 0 )
    index = mCboChangeAttribute->findText( u"Classification"_s );
  mCboChangeAttribute->setCurrentIndex( std::max( index, 0 ) );

  // setEnabled( pcLayer->isEditable() );
  mEditingToolsAction->setEnabled( pcLayer->isEditable() );
  // Reparse the class values when the renderer changes - renderer3DChanged() is not fired when only the renderer symbol is changed
  connect( pcLayer, &QgsMapLayer::request3DUpdate, this, &Qgs3DPointCloudEditingToolBar::onPointCloudChangeAttributeSettingsChanged );

  setEnabled( true );
  for ( auto action : findChildren<QAction *>() )
    action->setVisible( true );
  onPointCloudChangeAttributeSettingsChanged(); // be sure to have the good input fields displayed
  qDebug() << __FUNCTION__ << __LINE__ << "visible:" << isVisible();
}

void Qgs3DPointCloudEditingToolBar::onPointCloudChangeAttributeSettingsChanged()
{
  const QString attributeName = mCboChangeAttribute->currentText();

  mSpinChangeAttributeValue->setSuffix( QString() );
  bool useComboBox = false;

  if ( attributeName == "Intensity"_L1 || attributeName == "PointSourceId"_L1 || attributeName == "Red"_L1 || attributeName == "Green"_L1 || attributeName == "Blue"_L1 || attributeName == "Infrared"_L1 )
  {
    mSpinChangeAttributeValue->setMinimum( 0 );
    mSpinChangeAttributeValue->setMaximum( 65535 );
    mSpinChangeAttributeValue->setDecimals( 0 );
  }
  else if ( attributeName == "ReturnNumber"_L1 || attributeName == "NumberOfReturns"_L1 )
  {
    mSpinChangeAttributeValue->setMinimum( 0 );
    mSpinChangeAttributeValue->setMaximum( 15 );
    mSpinChangeAttributeValue->setDecimals( 0 );
  }
  else if ( attributeName == "Synthetic"_L1
            || attributeName == "KeyPoint"_L1
            || attributeName == "Withheld"_L1
            || attributeName == "Overlap"_L1
            || attributeName == "ScanDirectionFlag"_L1
            || attributeName == "EdgeOfFlightLine"_L1 )
  {
    useComboBox = true;
    const int oldIndex = mCboChangeAttributeValue->currentIndex();
    QgsSignalBlocker< QComboBox > blocker( mCboChangeAttributeValue );
    mCboChangeAttributeValue->clear();
    mCboChangeAttributeValue->addItem( tr( "False" ), 0 );
    mCboChangeAttributeValue->addItem( tr( "True" ), 1 );
    mCboChangeAttributeValue->setEditable( false );
    mCboChangeAttributeValue->setCurrentIndex( std::min( oldIndex, 1 ) );
  }
  else if ( attributeName == "ScannerChannel"_L1 )
  {
    mSpinChangeAttributeValue->setMinimum( 0 );
    mSpinChangeAttributeValue->setMaximum( 3 );
    mSpinChangeAttributeValue->setDecimals( 0 );
  }
  else if ( attributeName == "Classification"_L1 )
  {
    useComboBox = true;
    const QStringList split = mCboChangeAttributeValue->currentText().split( ' ' );
    const int oldValue = split.isEmpty() ? 0 : split.constFirst().toInt();

    whileBlocking( mCboChangeAttributeValue )->clear();
    // We will fill the combobox with all available classes from the Classification renderer (may have changed names) and the layer statistics
    // Users will be able to manually type in any other class number too.
    QMap<int, QString> lasCodes = QgsPointCloudDataProvider::translatedLasClassificationCodes();
    QMap<int, QString> classes;

    QgsPointCloudLayer *layer = qobject_cast<QgsPointCloudLayer *>( QgisApp::instance()->activeLayer() );
    if ( layer )
    {
      QgsAbstract3DRenderer *r = layer->renderer3D();
      // if there's a classification renderer, let's use the classes labels
      if ( QgsPointCloudLayer3DRenderer *cr = dynamic_cast<QgsPointCloudLayer3DRenderer *>( r ) )
      {
        const QgsPointCloud3DSymbol *s = cr->symbol();
        if ( const QgsClassificationPointCloud3DSymbol *cs = dynamic_cast<const QgsClassificationPointCloud3DSymbol *>( s ) )
        {
          if ( cs->attribute() == "Classification"_L1 )
          {
            for ( const QgsPointCloudCategory &c : cs->categoriesList() )
            {
              classes[c.value()] = c.label();
            }
          }
        }
      }

      // then add missing classes from the layer stats too
      const QMap<int, int> statisticsClasses = layer->statistics().availableClasses( u"Classification"_s );
      for ( auto it = statisticsClasses.constBegin(); it != statisticsClasses.constEnd(); ++it )
      {
        if ( !classes.contains( it.key() ) )
          classes[it.key()] = lasCodes[it.key()];
      }
      for ( auto it = classes.constBegin(); it != classes.constEnd(); ++it )
      {
        // populate the combobox
        whileBlocking( mCboChangeAttributeValue )->addItem( u"%1 (%2)"_s.arg( it.key() ).arg( it.value() ), it.key() );
        // and also update the labels in the full list of classes, which will be used in the editable combobox validator.
        lasCodes[it.key()] = it.value();
      }
    }
    // new values (manually edited) will be added after a separator
    mCboChangeAttributeValue->insertSeparator( mCboChangeAttributeValue->count() );
    mClassValidator->setClasses( lasCodes );
    mCboChangeAttributeValue->setEditable( true );
    mCboChangeAttributeValue->setValidator( mClassValidator );
    mCboChangeAttributeValue->setCompleter( nullptr );

    // Try to reselect last selected value
    if ( classes.contains( oldValue ) )
    {
      for ( int i = 0; i < mCboChangeAttributeValue->count(); ++i )
      {
        if ( mCboChangeAttributeValue->itemText( i ).startsWith( u"%1 "_s.arg( oldValue ) ) )
        {
          mCboChangeAttributeValue->setCurrentIndex( i );
          break;
        }
      }
    }
    else
    {
      whileBlocking( mCboChangeAttributeValue )->addItem( u"%1 ()"_s.arg( oldValue ), oldValue );
      mCboChangeAttributeValue->setCurrentIndex( mCboChangeAttributeValue->count() - 1 );
    }
  }
  else if ( attributeName == "UserData"_L1 )
  {
    mSpinChangeAttributeValue->setMinimum( 0 );
    mSpinChangeAttributeValue->setMaximum( 255 );
    mSpinChangeAttributeValue->setDecimals( 0 );
  }
  else if ( attributeName == "ScanAngleRank"_L1 )
  {
    mSpinChangeAttributeValue->setMinimum( -180 );
    mSpinChangeAttributeValue->setMaximum( 180 );
    mSpinChangeAttributeValue->setDecimals( 3 );
    mSpinChangeAttributeValue->setSuffix( u" %1"_s.arg( tr( "degrees" ) ) );
  }
  else if ( attributeName == "GpsTime"_L1 )
  {
    mSpinChangeAttributeValue->setMinimum( 0 );
    mSpinChangeAttributeValue->setMaximum( std::numeric_limits<double>::max() );
    mSpinChangeAttributeValue->setDecimals( 42 );
  }

  mMapToolChangeAttribute->setAttribute( attributeName );
  double newValue = 0;
  if ( useComboBox && mCboChangeAttributeValue->isEditable() )
  {
    // read class integer
    const QStringList split = mCboChangeAttributeValue->currentText().split( ' ' );
    if ( !split.isEmpty() )
      newValue = split.constFirst().toDouble();
  }
  else if ( useComboBox )
  {
    // read true/false combo box
    newValue = mCboChangeAttributeValue->currentData().toDouble();
  }
  else
  {
    // read the spinbox value
    newValue = mSpinChangeAttributeValue->value();
  }
  mMapToolChangeAttribute->setNewValue( newValue );

  mCboChangeAttributeValueAction->setVisible( useComboBox );
  mSpinChangeAttributeValueAction->setVisible( !useComboBox );

  mMapToolChangeAttribute->setPointFilter( mChangeAttributePointFilter );
}

QList<QAction *> Qgs3DPointCloudEditingToolBar::groupActions() const
{
  return mGroupActions;
}
