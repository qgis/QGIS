/***************************************************************************
                              qgsannotationwidget.cpp
                              ------------------------
  begin                : February 25, 2010
  copyright            : (C) 2010 by Marco Hugentobler
  email                : marco dot hugentobler at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsannotationwidget.h"
#include "moc_qgsannotationwidget.cpp"
#include "qgsmapcanvasannotationitem.h"
#include "qgsannotation.h"
#include "qgsstyle.h"
#include "qgssymbollayerutils.h"
#include "qgssymbol.h"
#include "qgssymbolselectordialog.h"
#include "qgisapp.h"
#include "qgsfillsymbol.h"
#include "qgsmarkersymbol.h"
#include "qgsdoublespinbox.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingstree.h"

#include <QColorDialog>


const QgsSettingsEntryBool *QgsAnnotationWidget::settingLiveUpdate = new QgsSettingsEntryBool( QStringLiteral( "live-update" ), QgsSettingsTree::sTreeAnnotations, false, QObject::tr( "Whether the annotations are dynamically updated while they are edited" ) );


QgsAnnotationWidget::QgsAnnotationWidget( QgsMapCanvasAnnotationItem *item, QWidget *parent, Qt::WindowFlags f )
  : QWidget( parent, f )
  , mItem( item )
{
  setupUi( this );
  mLayerComboBox->setAllowEmptyLayer( true );

  mMapMarkerButton->setSymbolType( Qgis::SymbolType::Marker );
  mFrameStyleButton->setSymbolType( Qgis::SymbolType::Fill );

  if ( mItem && mItem->annotation() )
  {
    QgsAnnotation *annotation = mItem->annotation();
    blockAllSignals( true );

    if ( annotation->hasFixedMapPosition() )
    {
      mMapPositionFixedCheckBox->setCheckState( Qt::Checked );
    }
    else
    {
      mMapPositionFixedCheckBox->setCheckState( Qt::Unchecked );
    }

    whileBlocking( mSpinTopMargin )->setValue( annotation->contentsMargin().top() );
    whileBlocking( mSpinLeftMargin )->setValue( annotation->contentsMargin().left() );
    whileBlocking( mSpinRightMargin )->setValue( annotation->contentsMargin().right() );
    whileBlocking( mSpinBottomMargin )->setValue( annotation->contentsMargin().bottom() );

    mLayerComboBox->setLayer( annotation->mapLayer() );

    const QgsMarkerSymbol *symbol = annotation->markerSymbol();
    if ( symbol )
    {
      mMapMarkerButton->setSymbol( symbol->clone() );
    }
    const QgsFillSymbol *fill = annotation->fillSymbol();
    if ( fill )
    {
      mFrameStyleButton->setSymbol( fill->clone() );
    }

    blockAllSignals( false );
  }
  mMapMarkerButton->setMapCanvas( QgisApp::instance()->mapCanvas() );
  mMapMarkerButton->setMessageBar( QgisApp::instance()->messageBar() );
  mFrameStyleButton->setMapCanvas( QgisApp::instance()->mapCanvas() );
  mFrameStyleButton->setMessageBar( QgisApp::instance()->messageBar() );

  connect( mFrameStyleButton, &QgsSymbolButton::changed, this, &QgsAnnotationWidget::frameStyleChanged );

  // connect to the changed signal
  connect( mFrameStyleButton, &QgsSymbolButton::changed, this, &QgsAnnotationWidget::changed );
  connect( mMapMarkerButton, &QgsSymbolButton::changed, this, &QgsAnnotationWidget::changed );
  connect( mLayerComboBox, &QgsMapLayerComboBox::layerChanged, this, &QgsAnnotationWidget::changed );
  connect( mSpinTopMargin, static_cast<void ( QgsDoubleSpinBox::* )( double )>( &QgsDoubleSpinBox::valueChanged ), this, &QgsAnnotationWidget::changed );
  connect( mSpinLeftMargin, static_cast<void ( QgsDoubleSpinBox::* )( double )>( &QgsDoubleSpinBox::valueChanged ), this, &QgsAnnotationWidget::changed );
  connect( mSpinRightMargin, static_cast<void ( QgsDoubleSpinBox::* )( double )>( &QgsDoubleSpinBox::valueChanged ), this, &QgsAnnotationWidget::changed );
  connect( mSpinBottomMargin, static_cast<void ( QgsDoubleSpinBox::* )( double )>( &QgsDoubleSpinBox::valueChanged ), this, &QgsAnnotationWidget::changed );
  connect( mMapPositionFixedCheckBox, &QCheckBox::stateChanged, this, &QgsAnnotationWidget::changed );
}

QColor QgsAnnotationWidget::backgroundColor()
{
  return mFrameStyleButton->symbol() ? mFrameStyleButton->symbol()->color() : QColor();
}

void QgsAnnotationWidget::frameStyleChanged()
{
  emit backgroundColorChanged( backgroundColor() );
}

void QgsAnnotationWidget::apply()
{
  if ( mItem )
  {
    QgsAnnotation *annotation = mItem->annotation();
    if ( annotation )
    {
      annotation->setHasFixedMapPosition( mMapPositionFixedCheckBox->checkState() == Qt::Checked );
      annotation->setFillSymbol( mFrameStyleButton->clonedSymbol<QgsFillSymbol>() );
      annotation->setMarkerSymbol( mMapMarkerButton->clonedSymbol<QgsMarkerSymbol>() );
      annotation->setMapLayer( mLayerComboBox->currentLayer() );
      annotation->setContentsMargin( QgsMargins( mSpinLeftMargin->value(), mSpinTopMargin->value(), mSpinRightMargin->value(), mSpinBottomMargin->value() ) );
    }
    mItem->update();
  }
}

void QgsAnnotationWidget::blockAllSignals( bool block )
{
  mMapPositionFixedCheckBox->blockSignals( block );
  mMapMarkerButton->blockSignals( block );
  mLayerComboBox->blockSignals( block );
}
