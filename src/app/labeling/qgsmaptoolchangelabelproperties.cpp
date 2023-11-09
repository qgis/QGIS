/***************************************************************************
                          qgsmaptoolchangelabelproperties.cpp
                          ---------------------------------
    begin                : 2010-11-11
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolchangelabelproperties.h"
#include "qgslabelpropertydialog.h"
#include "qgsmapcanvas.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgsmapmouseevent.h"
#include "qgisapp.h"
#include "qgsmessagebar.h"

QgsMapToolChangeLabelProperties::QgsMapToolChangeLabelProperties( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDock )
  : QgsMapToolLabel( canvas, cadDock )
{
  mPalProperties << QgsPalLayerSettings::PositionX;
  mPalProperties << QgsPalLayerSettings::PositionY;
  mPalProperties << QgsPalLayerSettings::Show;
  mPalProperties << QgsPalLayerSettings::LabelRotation;
  mPalProperties << QgsPalLayerSettings::Family;
  mPalProperties << QgsPalLayerSettings::FontStyle;
  mPalProperties << QgsPalLayerSettings::Size;
  mPalProperties << QgsPalLayerSettings::Bold;
  mPalProperties << QgsPalLayerSettings::Italic;
  mPalProperties << QgsPalLayerSettings::Underline;
  mPalProperties << QgsPalLayerSettings::Color;
  mPalProperties << QgsPalLayerSettings::Strikeout;
  mPalProperties << QgsPalLayerSettings::MultiLineAlignment;
  mPalProperties << QgsPalLayerSettings::BufferSize;
  mPalProperties << QgsPalLayerSettings::BufferColor;
  mPalProperties << QgsPalLayerSettings::BufferDraw;
  mPalProperties << QgsPalLayerSettings::LabelDistance;
  mPalProperties << QgsPalLayerSettings::Hali;
  mPalProperties << QgsPalLayerSettings::Vali;
  mPalProperties << QgsPalLayerSettings::ScaleVisibility;
  mPalProperties << QgsPalLayerSettings::MinScale;
  mPalProperties << QgsPalLayerSettings::MaxScale;
  mPalProperties << QgsPalLayerSettings::AlwaysShow;
  mPalProperties << QgsPalLayerSettings::CalloutDraw;
  mPalProperties << QgsPalLayerSettings::LabelAllParts;
}

void QgsMapToolChangeLabelProperties::canvasPressEvent( QgsMapMouseEvent *e )
{
  deleteRubberBands();

  QgsLabelPosition labelPos;
  if ( !labelAtPosition( e, labelPos ) || labelPos.isDiagram )
  {
    mCurrentLabel = LabelDetails();
    return;
  }

  mCurrentLabel = LabelDetails( labelPos, canvas() );
  if ( !mCurrentLabel.valid || !mCurrentLabel.layer )
  {
    return;
  }

  createRubberBands();

  if ( !mCurrentLabel.layer->isEditable() )
  {
    QgsPalIndexes indexes;
    const bool newAuxiliaryLayer = createAuxiliaryFields( indexes );

    if ( !newAuxiliaryLayer && !mCurrentLabel.layer->auxiliaryLayer() )
    {
      deleteRubberBands();
      return;
    }

    // in case of a new auxiliary layer, a dialog window is displayed and the
    // canvas release event is lost.
    if ( newAuxiliaryLayer )
    {
      canvasReleaseEvent( e );
    }
  }
}

void QgsMapToolChangeLabelProperties::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e )
  if ( mLabelRubberBand && mCurrentLabel.valid )
  {
    QString labeltext = QString(); // NULL QString signifies no expression
    if ( mCurrentLabel.settings.isExpression )
    {
      labeltext = mCurrentLabel.pos.labelText;
    }

    QgsLabelPropertyDialog d( mCurrentLabel.pos.layerID,
                              mCurrentLabel.pos.providerID,
                              mCurrentLabel.pos.featureId,
                              mCurrentLabel.pos.labelFont,
                              labeltext,
                              mCurrentLabel.pos.isPinned,
                              mCurrentLabel.settings,
                              mCanvas,
                              nullptr );
    d.setMapCanvas( canvas() );

    connect( &d, &QgsLabelPropertyDialog::applied, this, &QgsMapToolChangeLabelProperties::dialogPropertiesApplied );
    if ( d.exec() == QDialog::Accepted )
    {
      applyChanges( d.changedProperties() );
    }

    deleteRubberBands();
  }
}

void QgsMapToolChangeLabelProperties::cadCanvasMoveEvent( QgsMapMouseEvent *e )
{
  updateHoveredLabel( e );
}

void QgsMapToolChangeLabelProperties::applyChanges( const QgsAttributeMap &changes )
{
  QgsVectorLayer *vlayer = mCurrentLabel.layer;
  if ( !vlayer )
    return;

  if ( !changes.isEmpty() )
  {
    if ( !vlayer->isEditable() )
    {
      bool needsEdit = false;
      for ( auto it = changes.constBegin(); it != changes.constEnd(); ++it )
      {
        if ( vlayer->fields().fieldOrigin( it.key() ) != QgsFields::OriginJoin )
        {
          needsEdit = true;
          break;
        }
      }
      if ( needsEdit )
      {
        if ( vlayer->startEditing() )
        {
          QgisApp::instance()->messageBar()->pushInfo( tr( "Change Label" ), tr( "Layer “%1” was made editable" ).arg( vlayer->name() ) );
        }
        else
        {
          QgisApp::instance()->messageBar()->pushWarning( tr( "Change Label" ), tr( "Cannot change “%1” — the layer “%2” could not be made editable" ).arg( mCurrentLabel.pos.labelText, vlayer->name() ) );
          return;
        }
      }
    }

    vlayer->beginEditCommand( tr( "Changed properties for label" ) + QStringLiteral( " '%1'" ).arg( currentLabelText( 24 ) ) );

    QgsAttributeMap::const_iterator changeIt = changes.constBegin();
    for ( ; changeIt != changes.constEnd(); ++changeIt )
    {
      vlayer->changeAttributeValue( mCurrentLabel.pos.featureId, changeIt.key(), changeIt.value() );
    }

    vlayer->endEditCommand();
    vlayer->triggerRepaint();
  }
}

void QgsMapToolChangeLabelProperties::dialogPropertiesApplied()
{
  QgsLabelPropertyDialog *dlg = qobject_cast<QgsLabelPropertyDialog *>( sender() );
  if ( !dlg )
    return;

  applyChanges( dlg->changedProperties() );
}

