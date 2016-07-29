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

QgsMapToolChangeLabelProperties::QgsMapToolChangeLabelProperties( QgsMapCanvas* canvas ): QgsMapToolLabel( canvas )
{
}

QgsMapToolChangeLabelProperties::~QgsMapToolChangeLabelProperties()
{
}

void QgsMapToolChangeLabelProperties::canvasPressEvent( QgsMapMouseEvent* e )
{
  deleteRubberBands();

  QgsLabelPosition labelPos;
  if ( !labelAtPosition( e, labelPos ) || labelPos.isDiagram )
  {
    mCurrentLabel = LabelDetails();
    return;
  }

  mCurrentLabel = LabelDetails( labelPos );
  if ( !mCurrentLabel.valid || !mCurrentLabel.layer || !mCurrentLabel.layer->isEditable() )
  {
    return;
  }

  createRubberBands();
}

void QgsMapToolChangeLabelProperties::canvasReleaseEvent( QgsMapMouseEvent* e )
{
  Q_UNUSED( e );
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
                              labeltext, nullptr );

    connect( &d, SIGNAL( applied() ), this, SLOT( dialogPropertiesApplied() ) );
    if ( d.exec() == QDialog::Accepted )
    {
      applyChanges( d.changedProperties() );
    }

    deleteRubberBands();
  }
}

void QgsMapToolChangeLabelProperties::applyChanges( const QgsAttributeMap& changes )
{
  QgsVectorLayer* vlayer = mCurrentLabel.layer;
  if ( !vlayer )
    return;

  if ( !changes.isEmpty() )
  {
    vlayer->beginEditCommand( tr( "Changed properties for label" ) + QString( " '%1'" ).arg( currentLabelText( 24 ) ) );

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
  QgsLabelPropertyDialog* dlg = qobject_cast<QgsLabelPropertyDialog*>( sender() );
  if ( !dlg )
    return;

  applyChanges( dlg->changedProperties() );
}

