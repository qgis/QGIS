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
/* $Id$ */

#include "qgscontinuouscolordialog.h"
#include "qgscontinuouscolorrenderer.h"
#include "qgsfield.h"
#include "qgssymbol.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"

#include <QColorDialog>


QgsContinuousColorDialog::QgsContinuousColorDialog( QgsVectorLayer * layer )
    : QDialog(), mVectorLayer( layer )
{
  setupUi( this );
#ifdef QGISDEBUG
  qWarning( "constructor QgsContinuousColorDialog" );
#endif

  QObject::connect( btnMinValue, SIGNAL( clicked() ), this, SLOT( selectMinimumColor() ) );
  QObject::connect( btnMaxValue, SIGNAL( clicked() ), this, SLOT( selectMaximumColor() ) );

  //find out the numerical fields of mVectorLayer
  QgsVectorDataProvider *provider = mVectorLayer->dataProvider();
  if ( provider )
  {
    const QgsFieldMap & fields = provider->fields();
    int fieldnumber( 0 ), combonumber( 0 );
    QString str;

    for ( QgsFieldMap::const_iterator it = fields.begin(); it != fields.end(); ++it )
    {
      QVariant::Type type = ( *it ).type();
      if ( type == QVariant::Int || type == QVariant::Double )
      {
        str = ( *it ).name();
        classificationComboBox->addItem( str );
        mFieldMap.insert( std::make_pair( combonumber, fieldnumber ) );
        combonumber++;
      }
      fieldnumber++;
    }
  }
  else
  {
    qWarning( "Warning, data provider is null in QgsContinuousColorDialog::QgsContinuousColorDialog(...)" );
    return;
  }

  //restore the correct colors for minimum and maximum values

  const QgsContinuousColorRenderer* renderer = dynamic_cast < const QgsContinuousColorRenderer * >( layer->renderer() );;

  if ( renderer )
  {
    // Awkard - here we want to search through mFieldMap for a
    // particular value, while elsewhere in this code we need to search
    // for a particular key, so one or the other loses out, which is here.

    std::map<int, int>::const_iterator iter = mFieldMap.begin();
    while ( iter != mFieldMap.end() )
    {
      if ( iter->second == renderer->classificationField() )
        break;
      iter++;
    }
    if ( iter != mFieldMap.end() )
      classificationComboBox->setCurrentIndex( iter->first );
    else
      classificationComboBox->setCurrentIndex( -1 );

    const QgsSymbol* minsymbol = renderer->minimumSymbol();
    const QgsSymbol* maxsymbol = renderer->maximumSymbol();

    if ( mVectorLayer->vectorType() == QGis::Line || mVectorLayer->vectorType() == QGis::Point )
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

    if ( mVectorLayer->vectorType() != QGis::Polygon )
    {
      cb_polygonOutline->setVisible( false );
    }
  }
  else
  {
    cb_polygonOutline->setCheckState( Qt::Checked );
    outlinewidthspinbox->setValue( 0.4 );
    if ( mVectorLayer->vectorType() != QGis::Polygon )
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
#ifdef QGISDEBUG
  qWarning( "constructor QgsContinuousColorDialog" );
#endif
}

QgsContinuousColorDialog::~QgsContinuousColorDialog()
{
#ifdef QGISDEBUG
  qWarning( "destructor QgsContinuousColorDialog" );
#endif
}

void QgsContinuousColorDialog::apply()
{
  int comboIndex = classificationComboBox->currentIndex();
  if ( comboIndex == -1 )  //don't do anything, if there is no classification field
  {
    return;
  }
  std::map < int, int >::iterator iter = mFieldMap.find( comboIndex );
  // Should never happen...
  assert( iter != mFieldMap.end() );

  int classfield = iter->second;

  //find the minimum and maximum for the classification variable
  double minimum, maximum;
  QgsVectorDataProvider *provider = dynamic_cast<QgsVectorDataProvider*>( mVectorLayer->dataProvider() );
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
  QgsSymbol* minsymbol = new QgsSymbol( mVectorLayer->vectorType(), QString::number( minimum, 'f' ), "", "" );
  QPen minPen;
  minPen.setColor( btnMinValue->color() );
  minPen.setWidthF( outlinewidthspinbox->value() );
  if ( mVectorLayer->vectorType() == QGis::Line || mVectorLayer->vectorType() == QGis::Point )
  {
    minsymbol->setPen( minPen );
  }
  else
  {
    minsymbol->setBrush( QBrush( btnMinValue->color() ) );
    minsymbol->setPen( minPen );
  }

  QgsSymbol* maxsymbol = new QgsSymbol( mVectorLayer->vectorType(), QString::number( maximum, 'f' ), "", "" );
  QPen maxPen;
  maxPen.setColor( btnMaxValue->color() );
  maxPen.setWidthF( outlinewidthspinbox->value() );
  if ( mVectorLayer->vectorType() == QGis::Line || mVectorLayer->vectorType() == QGis::Point )
  {
    maxsymbol->setPen( maxPen );
  }
  else
  {
    maxsymbol->setBrush( QBrush( btnMaxValue->color() ) );
    maxsymbol->setPen( maxPen );
  }

  QgsContinuousColorRenderer* renderer = new QgsContinuousColorRenderer( mVectorLayer->vectorType() );
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
