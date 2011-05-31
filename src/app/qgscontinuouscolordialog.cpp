/***************************************************************************
                          qgscontinuouscolordialog.cpp
 Continuous color renderer dialog
                             -------------------
    begin                : 2004-02-11
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscontinuouscolordialog.h"
#include "qgscontinuouscolorrenderer.h"
#include "qgis.h"
#include "qgsfield.h"
#include "qgssymbol.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"

#include <QColorDialog>
#include <QKeyEvent>

QgsContinuousColorDialog::QgsContinuousColorDialog( QgsVectorLayer * layer )
    : QDialog(), mVectorLayer( layer )
{
  setupUi( this );
  QgsDebugMsg( "entered." );

  QObject::connect( btnMinValue, SIGNAL( clicked() ), this, SLOT( selectMinimumColor() ) );
  QObject::connect( btnMaxValue, SIGNAL( clicked() ), this, SLOT( selectMaximumColor() ) );

  //find out the numerical fields of mVectorLayer
  const QgsFieldMap & fields = mVectorLayer->pendingFields();
  QString displayName;

  for ( QgsFieldMap::const_iterator it = fields.begin(); it != fields.end(); ++it )
  {
    QVariant::Type type = it->type();
    if ( type == QVariant::Int || type == QVariant::Double || type == QVariant::LongLong )
    {
      displayName = mVectorLayer->attributeDisplayName( it.key() );
      classificationComboBox->addItem( displayName, it.key() );
    }
  }

  //restore the correct colors for minimum and maximum values

  const QgsContinuousColorRenderer* renderer = dynamic_cast<const QgsContinuousColorRenderer *>( layer->renderer() );;

  if ( renderer )
  {
    classificationComboBox->setCurrentIndex( classificationComboBox->findData( renderer->classificationField() ) );

    const QgsSymbol* minsymbol = renderer->minimumSymbol();
    const QgsSymbol* maxsymbol = renderer->maximumSymbol();

    if ( mVectorLayer->geometryType() == QGis::Line || mVectorLayer->geometryType() == QGis::Point )
    {
      btnMinValue->setColor( minsymbol->pen().color() );
      btnMaxValue->setColor( maxsymbol->pen().color() );
    }
    else
    {
      btnMinValue->setColor( minsymbol->brush().color() );
      btnMaxValue->setColor( maxsymbol->brush().color() );
    }

    outlinewidthspinbox->setMinimum( 0 );
    outlinewidthspinbox->setValue( minsymbol->pen().widthF() );

    if ( renderer->drawPolygonOutline() )
    {
      cb_polygonOutline->setCheckState( Qt::Checked );
    }
    else
    {
      cb_polygonOutline->setCheckState( Qt::Unchecked );
    }

    if ( mVectorLayer->geometryType() != QGis::Polygon )
    {
      cb_polygonOutline->setVisible( false );
    }
  }
  else
  {
    cb_polygonOutline->setCheckState( Qt::Checked );
    outlinewidthspinbox->setValue( DEFAULT_LINE_WIDTH );
    if ( mVectorLayer->geometryType() != QGis::Polygon )
      cb_polygonOutline->setVisible( false );

    btnMinValue->setColor( Qt::black );
    btnMaxValue->setColor( Qt::white );

  }
  // Ensure that the state of other widgets is appropriate for the
  // state of the polygonoutline checkbox.
  on_cb_polygonOutline_clicked();
}

QgsContinuousColorDialog::QgsContinuousColorDialog()
{
  setupUi( this );
  QgsDebugMsg( "entered." );
}

QgsContinuousColorDialog::~QgsContinuousColorDialog()
{
  QgsDebugMsg( "entered." );
}

void QgsContinuousColorDialog::apply()
{
  int classfield = classificationComboBox->itemData( classificationComboBox->currentIndex() ).toInt();

  //find the minimum and maximum for the classification variable
  double minimum, maximum;
  QgsVectorDataProvider *provider = dynamic_cast<QgsVectorDataProvider *>( mVectorLayer->dataProvider() );
  if ( provider )
  {
    minimum = provider->minimumValue( classfield ).toDouble();
    maximum = provider->maximumValue( classfield ).toDouble();
  }
  else
  {
    QgsDebugMsg( "Warning, provider is null" );
    return;
  }


  //create the render items for minimum and maximum value
  QgsSymbol* minsymbol = new QgsSymbol( mVectorLayer->geometryType(), QVariant( minimum ).toString(), "", "" );
  QPen minPen;
  minPen.setColor( btnMinValue->color() );
  minPen.setWidthF( outlinewidthspinbox->value() );
  if ( mVectorLayer->geometryType() == QGis::Line || mVectorLayer->geometryType() == QGis::Point )
  {
    minsymbol->setPen( minPen );
  }
  else
  {
    minsymbol->setBrush( QBrush( btnMinValue->color() ) );
    minsymbol->setPen( minPen );
  }

  QgsSymbol* maxsymbol = new QgsSymbol( mVectorLayer->geometryType(), QVariant( maximum ).toString(), "", "" );
  QPen maxPen;
  maxPen.setColor( btnMaxValue->color() );
  maxPen.setWidthF( outlinewidthspinbox->value() );
  if ( mVectorLayer->geometryType() == QGis::Line || mVectorLayer->geometryType() == QGis::Point )
  {
    maxsymbol->setPen( maxPen );
  }
  else
  {
    maxsymbol->setBrush( QBrush( btnMaxValue->color() ) );
    maxsymbol->setPen( maxPen );
  }

  QgsContinuousColorRenderer* renderer = new QgsContinuousColorRenderer( mVectorLayer->geometryType() );
  mVectorLayer->setRenderer( renderer );

  renderer->setMinimumSymbol( minsymbol );
  renderer->setMaximumSymbol( maxsymbol );
  renderer->setClassificationField( classfield );
  bool drawOutline = ( cb_polygonOutline->checkState() == Qt::Checked ) ? true : false;
  renderer->setDrawPolygonOutline( drawOutline );
}

void QgsContinuousColorDialog::selectMinimumColor()
{
  QColor mincolor = QColorDialog::getColor( btnMinValue->color(), this );
  if ( mincolor.isValid() )
  {
    btnMinValue->setColor( mincolor );
  }
  activateWindow();
}

void QgsContinuousColorDialog::selectMaximumColor()
{
  QColor maxcolor = QColorDialog::getColor( btnMaxValue->color(), this );
  if ( maxcolor.isValid() )
  {
    btnMaxValue->setColor( maxcolor );
  }
  activateWindow();
}

void QgsContinuousColorDialog::on_cb_polygonOutline_clicked()
{
  if ( cb_polygonOutline->checkState() == Qt::Checked )
    outlinewidthspinbox->setEnabled( true );
  else
    outlinewidthspinbox->setEnabled( false );
}

void QgsContinuousColorDialog::keyPressEvent( QKeyEvent * e )
{
  // Ignore the ESC key to avoid close the dialog without the properties window
  if ( e->key() == Qt::Key_Escape )
  {
    e->ignore();
  }
}
