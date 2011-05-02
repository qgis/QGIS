/***************************************************************************
                         qgsgraduatedsymboldialog.cpp  -  description
                             -------------------
    begin                : Oct 2003
    copyright            : (C) 2003 by Marco Hugentobler
    email                : mhugent@geo.unizh.ch
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

#include <algorithm>
#include <cmath>

#include "qgsgraduatedsymboldialog.h"
#include "qgsfield.h"
#include "qgsgraduatedsymbolrenderer.h"
#include "qgsludialog.h"
#include "qgssymbol.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"

#include <QKeyEvent>

QgsGraduatedSymbolDialog::QgsGraduatedSymbolDialog( QgsVectorLayer * layer ): QDialog(), mVectorLayer( layer ), sydialog( layer )
{
  setupUi( this );
  QgsDebugMsg( "entered." );

  setOrientation( Qt::Vertical );

  //find out the numerical fields of mVectorLayer
  const QgsFieldMap & fields = layer->pendingFields();
  QString displayName;

  for ( QgsFieldMap::const_iterator it = fields.begin(); it != fields.end(); ++it )
  {
    QVariant::Type type = ( *it ).type();
    if ( type == QVariant::Int || type == QVariant::Double || type == QVariant::LongLong )
    {
      displayName = layer->attributeDisplayName( it.key() );
      classificationComboBox->addItem( displayName );
      mFieldMap.insert( std::make_pair( displayName, it.key() ) );
    }
  }

  //restore the correct settings
  const QgsGraduatedSymbolRenderer* renderer = dynamic_cast<const QgsGraduatedSymbolRenderer *>( layer->renderer() );

  //
  // Set up the mode combo
  //
  modeComboBox->addItem( tr( "Equal Interval" ) );
  modeComboBox->addItem( tr( "Quantiles" ) );
  modeComboBox->addItem( tr( "Empty" ) );

  if ( renderer )
  {
    QString myMode = "";
    if ( renderer->mode() == QgsGraduatedSymbolRenderer::Empty )
    {
      myMode = tr( "Empty" );
    }
    else if ( renderer->mode() == QgsGraduatedSymbolRenderer::Quantile )
    {
      myMode = tr( "Quantiles" );
    }
    else
    {
      myMode = tr( "Equal Interval" );
    }
    modeComboBox->setCurrentIndex( modeComboBox->findText( myMode ) );
  }


  //
  // Set up the classfield combo
  //
  if ( renderer )
  {
    QList < QgsSymbol * >list = renderer->symbols();

    //display the classification field
    QString classfield = "";
    for ( std::map<QString, int>::iterator it = mFieldMap.begin(); it != mFieldMap.end(); ++it )
    {
      if ( it->second == renderer->classificationField() )
      {
        classfield = it->first;
        break;
      }
    }
    classificationComboBox->setCurrentIndex( classificationComboBox->findText( classfield ) );

    numberofclassesspinbox->setValue( list.size() );
    //fill the items of the renderer into mValues
    for ( QList<QgsSymbol*>::iterator it = list.begin(); it != list.end(); ++it )
    {
      //todo: make an assignment operator and a copy constructor for QgsSymbol
      QString classbreak = ( *it )->lowerValue() + " - " + ( *it )->upperValue();
      QgsSymbol* sym = new QgsSymbol( mVectorLayer->geometryType(), ( *it )->lowerValue(), ( *it )->upperValue(), ( *it )->label() );
      sym->setPen(( *it )->pen() );
      sym->setCustomTexture(( *it )->customTexture() );
      sym->setBrush(( *it )->brush() );
      sym->setNamedPointSymbol(( *it )->pointSymbolName() );
      sym->setPointSize(( *it )->pointSize() );
      sym->setPointSizeUnits(( *it )->pointSizeUnits() );
      sym->setScaleClassificationField(( *it )->scaleClassificationField() );
      sym->setRotationClassificationField(( *it )->rotationClassificationField() );
      mEntries.insert( std::make_pair( classbreak, sym ) );
      QListWidgetItem * mypItem = new QListWidgetItem( classbreak );
      updateEntryIcon( sym , mypItem );
      mClassListWidget->addItem( mypItem );
    }

  }

  //do the necessary signal/slot connections
  QObject::connect( mClassifyButton, SIGNAL( clicked() ), this, SLOT( adjustClassification() ) );
  QObject::connect( mClassListWidget, SIGNAL( currentItemChanged( QListWidgetItem*, QListWidgetItem* ) ), this, SLOT( changeCurrentValue() ) );
  QObject::connect( &sydialog, SIGNAL( settingsChanged() ), this, SLOT( applySymbologyChanges() ) );
  QObject::connect( mClassListWidget, SIGNAL( itemDoubleClicked( QListWidgetItem* ) ), this, SLOT( modifyClass( QListWidgetItem* ) ) );
  QObject::connect( mDeleteClassButton, SIGNAL( clicked() ), this, SLOT( deleteCurrentClass() ) );

  mSymbolWidgetStack->addWidget( &sydialog );
  mSymbolWidgetStack->setCurrentWidget( &sydialog );

  mClassListWidget->setCurrentRow( 0 );
}

QgsGraduatedSymbolDialog::QgsGraduatedSymbolDialog(): QDialog(), mVectorLayer( 0 ), sydialog( 0 )
{
  setupUi( this );
  QgsDebugMsg( "entered." );
}

QgsGraduatedSymbolDialog::~QgsGraduatedSymbolDialog()
{
  QgsDebugMsg( "entered." );
}

void QgsGraduatedSymbolDialog::adjustNumberOfClasses()
{
  //find out the number of the classification field
  QString fieldstring = classificationComboBox->currentText();

  if ( fieldstring.isEmpty() )  //don't do anything, it there is no classification field
  {
    show();
    return;
  }
}

void QgsGraduatedSymbolDialog::apply()
{
  if ( classificationComboBox->currentText().isEmpty() )  //don't do anything, it there is no classification field
  {
    return;
  }

  QgsGraduatedSymbolRenderer* renderer = new QgsGraduatedSymbolRenderer( mVectorLayer->geometryType() );

  //
  // First the mode
  //
  if ( modeComboBox->currentText() == tr( "Empty" ) )
  {
    renderer->setMode( QgsGraduatedSymbolRenderer::Empty );
  }
  else if ( modeComboBox->currentText() == tr( "Quantiles" ) )
  {
    renderer->setMode( QgsGraduatedSymbolRenderer::Quantile );
  }
  else //equal interval by default//equal interval by default
  {
    renderer->setMode( QgsGraduatedSymbolRenderer::EqualInterval );
  }
  //
  // Now the class breaks
  //
  for ( int item = 0; item < mClassListWidget->count(); ++item )
  {
    QString classbreak = mClassListWidget->item( item )->text();
    std::map<QString, QgsSymbol*>::iterator it = mEntries.find( classbreak );
    if ( it == mEntries.end() )
    {
      continue;
    }

    QString lower_bound = it->second->lowerValue();
    QString upper_bound = it->second->upperValue();
    QString label = it->second->label();

    QgsSymbol* sy = new QgsSymbol( mVectorLayer->geometryType(), lower_bound, upper_bound, label );

    sy->setColor( it->second->pen().color() );
    sy->setLineStyle( it->second->pen().style() );
    sy->setLineWidth( it->second->pen().widthF() );

    if ( mVectorLayer->geometryType() == QGis::Point )
    {
      sy->setNamedPointSymbol( it->second->pointSymbolName() );
      sy->setPointSize( it->second->pointSize() );
      sy->setPointSizeUnits( it->second->pointSizeUnits() );
      sy->setScaleClassificationField( it->second->scaleClassificationField() );
      sy->setRotationClassificationField( it->second->rotationClassificationField() );
    }

    if ( mVectorLayer->geometryType() != QGis::Line )
    {
      sy->setFillColor( it->second->brush().color() );
      sy->setCustomTexture( it->second->customTexture() );//necessary?
      sy->setFillStyle( it->second->brush().style() );
    }

    //test, if lower_bound is numeric or not (making a subclass of QString would be the proper solution)
    bool lbcontainsletter = false;
    for ( int j = 0; j < lower_bound.length(); j++ )
    {
      if ( lower_bound[j].isLetter() )
      {
        lbcontainsletter = true;
      }
    }

    //test, if upper_bound is numeric or not (making a subclass of QString would be the proper solution)
    bool ubcontainsletter = false;
    for ( int j = 0; j < upper_bound.length(); j++ )
    {
      if ( upper_bound[j].isLetter() )
      {
        ubcontainsletter = true;
      }
    }
    if ( !lbcontainsletter && !ubcontainsletter && lower_bound.length() > 0 && upper_bound.length() > 0 ) //only add the item if the value bounds do not contain letters and are not null strings
    {
      renderer->addSymbol( sy );
    }
    else
    {
      delete sy;
    }
  }
  renderer->updateSymbolAttributes();

  std::map<QString, int>::iterator iter = mFieldMap.find( classificationComboBox->currentText() );
  if ( iter != mFieldMap.end() )
  {
    renderer->setClassificationField( iter->second );
  }
  mVectorLayer->setRenderer( renderer );
}

void QgsGraduatedSymbolDialog::adjustClassification()
{
  mClassListWidget->clear();
  QGis::GeometryType m_type = mVectorLayer->geometryType();
  double minimum = 0;
  double maximum = 0;

  //delete all previous entries
  for ( std::map<QString, QgsSymbol*>::iterator it = mEntries.begin(); it != mEntries.end(); ++it )
  {
    delete it->second;
  }
  mEntries.clear();

  //find out the number of the classification field
  QString fieldstring = classificationComboBox->currentText();

  if ( fieldstring.isEmpty() )  //don't do anything, it there is no classification field
  {
    show();
    return;
  }

  std::map < QString, int >::iterator iter = mFieldMap.find( fieldstring );
  int field = iter->second;

  if ( modeComboBox->currentText() == tr( "Equal Interval" ) ||
       modeComboBox->currentText() == tr( "Quantiles" ) )
  {
    minimum = mVectorLayer->minimumValue( field ).toDouble();
    maximum = mVectorLayer->maximumValue( field ).toDouble();
  }
  else                    //don't waste performance if mMode is QgsGraduatedSymbolDialog::EMPTY
  {
    minimum = 0;
    maximum = 0;
  }

  //todo: setup a data structure which holds the symbols
  std::list<QgsSymbol*> symbolList;
  for ( int i = 0; i < numberofclassesspinbox->value(); ++i )
  {
    QgsSymbol* symbol = new QgsSymbol( m_type );
    symbol->setLabel( "" );
    QPen pen;
    QBrush brush;

    // todo: These color ramps should come from a dropdown list
    QString ramp;
    ramp = "red_to_green";
    if ( m_type == QGis::Line )
    {
      pen.setColor( getColorFromRamp( ramp, i, numberofclassesspinbox->value() ) );
    }
    else //point or polygon
    {
      brush.setColor( getColorFromRamp( ramp, i, numberofclassesspinbox->value() ) );
      pen.setColor( Qt::black );
    }

    pen.setWidthF( symbol->lineWidth() );
    brush.setStyle( Qt::SolidPattern );
    symbol->setPen( pen );
    symbol->setBrush( brush );
    symbolList.push_back( symbol );
  }

  QString listBoxText;
  QString lowerString, upperString;

  if ( modeComboBox->currentText() == tr( "Quantiles" ) )
  {
    //test: insert the values into mClassListWidget
    std::list<double> quantileBorders;
    quantilesFromVectorLayer( quantileBorders, field, numberofclassesspinbox->value() );

    std::list<double>::const_iterator it;
    std::list<double>::const_iterator last_it = quantileBorders.end();
    std::list<QgsSymbol*>::const_iterator symbol_it = symbolList.begin();
    for ( it = quantileBorders.begin(); symbol_it != symbolList.end() && it != quantileBorders.end(); ++it )
    {
      if ( last_it != quantileBorders.end() )
      {
        lowerString = QString::number( QVariant( *last_it ).toDouble(), 'f', 3 );
        upperString = QString::number( QVariant( *it ).toDouble(), 'f', 3 );
        ( *symbol_it )->setLowerValue( lowerString );
        ( *symbol_it )->setUpperValue( upperString );


        listBoxText = lowerString + " - " + upperString;
        mEntries.insert( std::make_pair( listBoxText, *symbol_it ) );
        QListWidgetItem *mypItem = new QListWidgetItem( listBoxText );
        mClassListWidget->addItem( mypItem );
        updateEntryIcon( *symbol_it, mypItem );
        ++symbol_it;
      }
      last_it = it;
    }
  }
  else if ( modeComboBox->currentText() == tr( "Equal Interval" ) )
  {
    std::list<QgsSymbol*>::const_iterator symbol_it = symbolList.begin();
    for ( int i = 0; i < numberofclassesspinbox->value(); ++i )
    {
      //switch if attribute is int or double
      double lower = minimum + ( maximum - minimum ) / numberofclassesspinbox->value() * i;
      double upper = minimum + ( maximum - minimum ) / numberofclassesspinbox->value() * ( i + 1 );
      lowerString = QString::number( lower, 'f', 3 );
      upperString = QString::number( upper, 'f', 3 );
      ( *symbol_it )->setLowerValue( lowerString );
      ( *symbol_it )->setUpperValue( upperString );
      listBoxText = lowerString + " - " + upperString;

      QListWidgetItem * mypItem = new QListWidgetItem( listBoxText );
      updateEntryIcon( *symbol_it, mypItem );
      mClassListWidget->addItem( mypItem );

      mEntries.insert( std::make_pair( listBoxText, *symbol_it ) );
      ++symbol_it;
    }
  }
  else if ( modeComboBox->currentText() == tr( "Empty" ) )
  {
    std::list<QgsSymbol*>::const_iterator symbol_it = symbolList.begin();
    for ( int i = 0; i < numberofclassesspinbox->value(); ++i )
    {
      listBoxText = "Empty" + QString::number( i + 1 );
      QListWidgetItem * mypItem = new QListWidgetItem( listBoxText );
      updateEntryIcon( *symbol_it, mypItem );
      mClassListWidget->addItem( mypItem );
      mEntries.insert( std::make_pair( listBoxText, *symbol_it ) );
      ++symbol_it;
    }
  }

  mClassListWidget->setCurrentRow( 0 );
}

void QgsGraduatedSymbolDialog::changeCurrentValue()
{
  sydialog.blockSignals( true );//block signals to prevent sydialog from changing the current QgsRenderItem
  QListWidgetItem* item = mClassListWidget->currentItem();
  if ( item )
  {
    QString value = item->text();
    std::map<QString, QgsSymbol*>::iterator it = mEntries.find( value );
    if ( it != mEntries.end() )
    {
      sydialog.set(( *it ).second );
      sydialog.setLabel(( *it ).second->label() );
      updateEntryIcon(( *it ).second, item );
    }
  }
  sydialog.blockSignals( false );
}

void QgsGraduatedSymbolDialog::applySymbologyChanges()
{
  QListWidgetItem* item = mClassListWidget->currentItem();
  if ( item )
  {
    QString value = item->text();
    std::map<QString, QgsSymbol*>::iterator it = mEntries.find( value );
    if ( it != mEntries.end() )
    {
      sydialog.apply(( *it ).second );
      it->second->setLabel(( *it ).second->label() );
      updateEntryIcon(( *it ).second, item );
    }
  }
}

void QgsGraduatedSymbolDialog::modifyClass( QListWidgetItem* item )
{
  QString currenttext = item->text();
  QgsSymbol* symbol = 0;
  std::map<QString, QgsSymbol*>::iterator iter = mEntries.find( currenttext );
  if ( iter != mEntries.end() )
  {
    symbol = iter->second;
  }
  QgsLUDialog dialog( this );

  if ( symbol )
  {
    dialog.setLowerValue( symbol->lowerValue() );
    dialog.setUpperValue( symbol->upperValue() );
  }

  if ( dialog.exec() == QDialog::Accepted )
  {
    if ( symbol )
    {
      mEntries.erase( currenttext );
      symbol->setLowerValue( dialog.lowerValue() );
      symbol->setUpperValue( dialog.upperValue() );
      QString newclass = dialog.lowerValue() + "-" + dialog.upperValue();
      mEntries.insert( std::make_pair( newclass, symbol ) );
      item->setText( newclass );
      updateEntryIcon( symbol, item );
    }
  }
}

void QgsGraduatedSymbolDialog::deleteCurrentClass()
{
  QListWidgetItem* currentItem = mClassListWidget->currentItem();
  if ( !currentItem )
  {
    return;
  }

  QString classValue = currentItem->text();
  int currentIndex = mClassListWidget->currentRow();
  mEntries.erase( classValue );
  delete( mClassListWidget->takeItem( currentIndex ) );
  QgsDebugMsg( QString( "numRows: %1" ).arg( mClassListWidget->count() ) );
  //
  if ( mClassListWidget->count() < ( currentIndex + 1 ) )
  {
    QgsDebugMsg( "selecting numRows - 1" );
    mClassListWidget->setCurrentRow( mClassListWidget->count() - 1 );
  }
  else
  {
    QgsDebugMsg( "selecting currentIndex" );
    mClassListWidget->setCurrentRow( currentIndex );
  }
}

int QgsGraduatedSymbolDialog::quantilesFromVectorLayer( std::list<double>& result, int attributeIndex, int numQuantiles ) const
{
  if ( mVectorLayer )
  {
    std::vector<double> attributeValues( mVectorLayer->featureCount() );
    QgsAttributeList attList;
    attList.push_back( attributeIndex );
    QgsFeature currentFeature;
    QgsAttributeMap currentAttributeMap;
    double currentValue;
    int index = 0;

    mVectorLayer->select( attList, QgsRectangle(), false );
    while ( mVectorLayer->nextFeature( currentFeature ) )
    {
      currentAttributeMap = currentFeature.attributeMap();
      currentValue = currentAttributeMap[attributeIndex].toDouble();
      attributeValues[index] = currentValue;
      ++index;
    }

    sort( attributeValues.begin(), attributeValues.end() );
    return calculateQuantiles( result, attributeValues, numQuantiles );
  }
  return 1;
}

int QgsGraduatedSymbolDialog::calculateQuantiles( std::list<double>& result, const std::vector<double>& values, int numQuantiles ) const
{
  result.clear();

  double q;
  double k;
  double intPart;

  result.push_back( values[0] );
  for ( int i = 0; i < ( numQuantiles - 1 ); ++i )
  {
    q = 100 / numQuantiles * ( i + 1 );
    k = values.size() * q / 100;
    if ( std::modf( k, &intPart ) < 0.000000001 )
    {
      result.push_back((( 100 - q ) * values[( int )( k - 1 )] + q * values[( int )k] ) / 100 );
    }
    else
    {
      result.push_back( values[( int )k] );
    }
  }
  result.push_back( values[values.size() - 1] );
  return 0;
}


QColor QgsGraduatedSymbolDialog::getColorFromRamp( QString ramp, int step, int totalSteps )
{
  QColor color;
  /* To do:
     Grab the ramp by name from a file or ramp registry
       and apply determine the color for the given step.
     Ideally there would be two types of ramps:
       - discrete colors: the number of steps would have to match totalSteps
       - continuous colors: (eg grass or gmt ramps) would need to code a method
          for determining an RGB color for any point along the continuum
     Color ramps should be plugin-able; should be defined in a simple text file format
       and read from a directory where users can add their own ramps.
  */
  if ( step == 0 )
  {
    color = QColor( 0, 255, 0 );
  }
  else
  {
    color = QColor( 0, 255 - (( 255 / totalSteps ) * step + 1 ), (( 255 / totalSteps ) * step + 1 ) );
  }
  return color;
}

void QgsGraduatedSymbolDialog::updateEntryIcon( QgsSymbol * thepSymbol,
    QListWidgetItem * thepItem )
{
  QGis::GeometryType myType = mVectorLayer->geometryType();
  switch ( myType )
  {
    case QGis::Point:
    {
      int myWidthScale = 4; //magick no to try to make vector props dialog preview look same as legend
      thepItem->setIcon( QIcon( QPixmap::fromImage( thepSymbol->getPointSymbolAsImage( myWidthScale ) ) ) );
    }
    break;
    case QGis::Line:
      thepItem->setIcon( QIcon( QPixmap::fromImage( thepSymbol->getLineSymbolAsImage() ) ) );
      break;
    case QGis::Polygon:
      thepItem->setIcon( QIcon( QPixmap::fromImage( thepSymbol->getPolygonSymbolAsImage() ) ) );
      break;
    default: //unknown
      //do nothing
      ;
  }
}

void QgsGraduatedSymbolDialog::keyPressEvent( QKeyEvent * e )
{
  // Ignore the ESC key to avoid close the dialog without the properties window
  if ( e->key() == Qt::Key_Escape )
  {
    e->ignore();
  }
}
