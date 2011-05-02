/***************************************************************************
                         qgsdiagramdialog.cpp  -  description
                         --------------------
    begin                : January 2007
    copyright            : (C) 2007 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdiagramdialog.h"
#include "qgsdiagramrenderer.h"
#include "qgsdiagramoverlay.h"
#include "qgsfield.h"
#include "qgslinearlyscalingdialog.h"
#include "qgssvgdiagramfactory.h"
#include "qgssvgdiagramfactorywidget.h"
#include "qgsvectordataprovider.h"
#include "qgswkndiagramfactory.h"
#include "qgswkndiagramfactorywidget.h"
#include <QColorDialog>


QgsDiagramDialog::QgsDiagramDialog( QgsVectorLayer* vl ): mVectorLayer( vl )
{
  setupUi( this );

  mDiagramTypeComboBox->insertItem( 0, tr( "Pie chart" ) );
  mDiagramTypeComboBox->insertItem( 1, tr( "Bar chart" ) );
  mDiagramTypeComboBox->insertItem( 2, tr( "Proportional SVG symbols" ) );

  if ( !mVectorLayer )
  {
    return;
  }

  //insert attributes into combo box
  QgsVectorDataProvider *provider;
  if (( provider = dynamic_cast<QgsVectorDataProvider *>( mVectorLayer->dataProvider() ) ) )
  {
    const QgsFieldMap & fields = provider->fields();
    QString str;

    int comboIndex = 0;
    for ( QgsFieldMap::const_iterator it = fields.begin(); it != fields.end(); ++it )
    {
      str = ( *it ).name();
      mClassificationComboBox->insertItem( comboIndex, str );
      ++comboIndex;
    }
  }

  mClassificationTypeComboBox->insertItem( 0, tr( "linearly scaling" ) );

  //if mVectorLayer already has a diagram overlay, apply its settings to this dialog
  const QgsVectorOverlay* previousOverlay = mVectorLayer->findOverlayByType( "diagram" );
  if ( previousOverlay )
  {
    restoreSettings( previousOverlay );
  }
  setGuiElementsEnabled( mDisplayDiagramsCheckBox->checkState() == Qt::Checked );
}

QgsDiagramDialog::QgsDiagramDialog(): mVectorLayer( 0 )
{

}

QgsDiagramDialog::~QgsDiagramDialog()
{

}

void QgsDiagramDialog::on_mClassificationTypeComboBox_currentIndexChanged( const QString& newType )
{
  if ( newType == tr( "linearly scaling" ) )
  {
    QWidget* currentWidget = mWidgetStackRenderers->currentWidget();
    if ( currentWidget )
    {
      mWidgetStackRenderers->removeWidget( currentWidget );
    }
    QWidget* newWidget = new QgsLinearlyScalingDialog( mVectorLayer );
    mWidgetStackRenderers->addWidget( newWidget );
    mWidgetStackRenderers->setCurrentWidget( newWidget );
    on_mClassificationComboBox_currentIndexChanged( mClassificationComboBox->currentText() );
  }
}

void QgsDiagramDialog::on_mClassificationComboBox_currentIndexChanged( const QString& newAttribute )
{
  int attributeIndex = QgsDiagramOverlay::indexFromAttributeName( newAttribute, mVectorLayer );
  if ( attributeIndex == -1 )
  {
    return;
  }
  QgsDiagramRendererWidget* rendererWidget = dynamic_cast<QgsDiagramRendererWidget *>( mWidgetStackRenderers->currentWidget() );
  if ( !rendererWidget )
  {
    return;
  }
  rendererWidget->changeClassificationField( attributeIndex );
}

void QgsDiagramDialog::on_mDiagramTypeComboBox_currentIndexChanged( const QString& text )
{
  //remove old widget
  QWidget* currentWidget = mDiagramFactoryStackedWidget->currentWidget();
  mDiagramFactoryStackedWidget->removeWidget( currentWidget );
  delete currentWidget;

  //and create a new one
  QgsDiagramFactoryWidget* newWidget = 0;
  if ( text == tr( "Pie chart" ) )
  {
    newWidget = new QgsWKNDiagramFactoryWidget( mVectorLayer, "Pie" );
  }
  else if ( text == tr( "Bar chart" ) )
  {
    newWidget = new QgsWKNDiagramFactoryWidget( mVectorLayer, "Bar" );
  }
  else if ( text == ( tr( "Proportional SVG symbols" ) ) )
  {
    newWidget = new QgsSVGDiagramFactoryWidget();
  }

  if ( newWidget )
  {
    mDiagramFactoryStackedWidget->addWidget( newWidget );
    mDiagramFactoryStackedWidget->setCurrentWidget( newWidget );
    newWidget->show();
  }
}

void QgsDiagramDialog::on_mDisplayDiagramsCheckBox_stateChanged( int state )
{
  //activate / deactivate all gui items depending on the checkbox state
  if ( state == Qt::Checked )
  {
    setGuiElementsEnabled( true );
  }
  else
  {
    setGuiElementsEnabled( false );
  }
}

void QgsDiagramDialog::apply() const
{
  if ( !mVectorLayer )
  {
    return;
  }

  //create diagram factory
  QgsDiagramFactory* diagramFactory = 0;
  QWidget* factoryWidget = mDiagramFactoryStackedWidget->currentWidget();

  if ( factoryWidget )
  {
    QgsDiagramFactoryWidget* diagramFactoryWidget = dynamic_cast<QgsDiagramFactoryWidget *>( factoryWidget );
    if ( factoryWidget )
    {
      diagramFactory = diagramFactoryWidget->createFactory();
    }
  }

  if ( !diagramFactory )
  {
    return;
  }

  //and diagram renderer

  //classAttr comes from the gui
  int classAttr =  QgsDiagramOverlay::indexFromAttributeName( mClassificationComboBox->currentText(), mVectorLayer );
  if ( classAttr == -1 )
  {
    return;
  }

  //attList contains the category attributes
  QgsAttributeList attList;
  QgsWKNDiagramFactory* wknDiagramFactory = dynamic_cast<QgsWKNDiagramFactory *>( diagramFactory );
  if ( wknDiagramFactory )
  {
    attList += wknDiagramFactory->categoryAttributes();
  }


  QgsDiagramRenderer* diagramRenderer = 0;
  QgsDiagramFactory::SizeUnit diagramSizeUnit = QgsDiagramFactory::MM; //mm on output medium is default

  QWidget* rendererWidget = mWidgetStackRenderers->currentWidget();
  if ( rendererWidget )
  {
    QgsDiagramRendererWidget* diagramRendererWidget = dynamic_cast<QgsDiagramRendererWidget *>( rendererWidget );
    if ( diagramRendererWidget )
    {
      diagramRenderer = diagramRendererWidget->createRenderer( classAttr, attList );
      diagramSizeUnit = diagramRendererWidget->sizeUnit();
    }
  }

  if ( !diagramRenderer )
  {
    return;
  }

  diagramRenderer->setFactory( diagramFactory );
  QgsAttributeList scalingAttributeList;
  scalingAttributeList.push_back( classAttr );
  diagramFactory->setScalingAttributes( scalingAttributeList );
  //also set units to the diagram factory
  diagramFactory->setSizeUnit( diagramSizeUnit );

  //the overlay needs to fetch scaling attributes and category attributes
  if ( !attList.contains( classAttr ) )
  {
    attList.push_back( classAttr );
  }
  QgsDiagramOverlay* diagramOverlay = new QgsDiagramOverlay( mVectorLayer );
  diagramOverlay->setDiagramRenderer( diagramRenderer );
  diagramOverlay->setAttributes( attList );

  //display flag
  if ( mDisplayDiagramsCheckBox->checkState() == Qt::Checked )
  {
    diagramOverlay->setDisplayFlag( true );
  }
  if ( mDisplayDiagramsCheckBox->checkState() == Qt::Unchecked )
  {
    diagramOverlay->setDisplayFlag( false );
  }

  //remove already existing diagram overlays
  mVectorLayer->removeOverlay( "diagram" );

  //finally add the new overlay to the vector layer
  mVectorLayer->addOverlay( diagramOverlay );
}

void QgsDiagramDialog::restoreSettings( const QgsVectorOverlay* overlay )
{
  const QgsDiagramOverlay* previousDiagramOverlay = dynamic_cast<const QgsDiagramOverlay *>( overlay );
  if ( overlay )
  {
    //set check state according to QgsDiagramOverlay
    if ( previousDiagramOverlay->displayFlag() )
    {
      mDisplayDiagramsCheckBox->setCheckState( Qt::Checked );
    }
    else
    {
      mDisplayDiagramsCheckBox->setCheckState( Qt::Unchecked );
    }
    const QgsDiagramRenderer* previousDiagramRenderer = dynamic_cast<const QgsDiagramRenderer *>( previousDiagramOverlay->diagramRenderer() );

    if ( previousDiagramRenderer && previousDiagramRenderer->factory() )
    {
      QgsDiagramFactory* theFactory = previousDiagramRenderer->factory();
      QgsDiagramFactoryWidget* newWidget = 0;


      QgsWKNDiagramFactory* theWKNFactory = dynamic_cast<QgsWKNDiagramFactory *>( theFactory );
      if ( theWKNFactory )
      {
        QString wknType = theWKNFactory->diagramType();
        if ( wknType == ( "Pie" ) )
        {
          newWidget = new QgsWKNDiagramFactoryWidget( mVectorLayer, "Pie" );
          mDiagramTypeComboBox->setCurrentIndex( mDiagramTypeComboBox->findText( tr( "Pie chart" ) ) );
        }
        else
        {
          newWidget = new QgsWKNDiagramFactoryWidget( mVectorLayer, "Bar" );
          mDiagramTypeComboBox->setCurrentIndex( mDiagramTypeComboBox->findText( tr( "Bar chart" ) ) );
        }
        newWidget->setExistingFactory( theWKNFactory );
      }

      QgsSVGDiagramFactory* theSVGFactory = dynamic_cast<QgsSVGDiagramFactory *>( theFactory );
      if ( theSVGFactory )
      {
        mDiagramTypeComboBox->setCurrentIndex( mDiagramTypeComboBox->findText( tr( "Proportional SVG symbols" ) ) );
        newWidget = new QgsSVGDiagramFactoryWidget();
      }

      newWidget->setExistingFactory( theFactory );
      //remove old widget
      QWidget* currentWidget = mDiagramFactoryStackedWidget->currentWidget();
      mDiagramFactoryStackedWidget->removeWidget( currentWidget );
      delete currentWidget;

      if ( newWidget )
      {
        mDiagramFactoryStackedWidget->addWidget( newWidget );
        mDiagramFactoryStackedWidget->setCurrentWidget( newWidget );
        newWidget->show();
      }


      //classification attribute
      QString classFieldName;
      QgsAttributeList attList = previousDiagramRenderer->classificationAttributes();
      if ( attList.size() > 0 )
      {
        classFieldName  = QgsDiagramOverlay::attributeNameFromIndex( attList.first(), mVectorLayer );
        mClassificationComboBox->setCurrentIndex( mClassificationComboBox->findText( classFieldName ) );

        //classification type (specific for renderer subclass)
        mClassificationTypeComboBox->setCurrentIndex( mClassificationTypeComboBox->findText( tr( "linearly scaling" ) ) );
      }

      //apply the renderer settings to the renderer specific dialog
      if ( mWidgetStackRenderers->count() > 0 )
      {
        QgsDiagramRendererWidget* rendererWidget = dynamic_cast<QgsDiagramRendererWidget *>( mWidgetStackRenderers->currentWidget() );
        if ( rendererWidget )
        {
          rendererWidget->applySettings( previousDiagramRenderer );
        }
      }
    }
  }
}

void QgsDiagramDialog::setGuiElementsEnabled( bool enabled )
{
  mDiagramTypeComboBox->setEnabled( enabled );
  mTypeLabel->setEnabled( enabled );
  mDiagramFactoryStackedWidget->setEnabled( enabled );
  mClassificationTypeLabel->setEnabled( enabled );
  mClassificationTypeComboBox->setEnabled( enabled );
  mClassificationLabel->setEnabled( enabled );
  mClassificationComboBox->setEnabled( enabled );
  mWidgetStackRenderers->setEnabled( enabled );
}



