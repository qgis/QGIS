/***************************************************************************
    qgspastetransformations.cpp - set up how source fields are transformed to
                                  destination fields in copy/paste operations
                             -------------------
    begin                : 8 July 2005
    copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au
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


#include <QComboBox>
#include <QSettings>

#include "qgsfield.h"
#include "qgscontexthelp.h"
#include "qgspastetransformations.h"
#include "qgsmaplayerregistry.h"
#include "qgslogger.h"

QgsPasteTransformations::QgsPasteTransformations()
    : QgsPasteTransformationsBase()
{
  setupUi( this );
  connect( buttonBox, SIGNAL( helpRequested() ),this,SLOT( help() ) );

  mAddTransferButton = new QPushButton( tr( "&Add New Transfer" ) );
  buttonBox->addButton( mAddTransferButton, QDialogButtonBox::ActionRole );
  connect( mAddTransferButton,SIGNAL( clicked() ), this, SLOT( addNewTransfer() ) );

  // Populate the dialog with the loaded layers
  QMap<QString, QgsMapLayer*> mapLayers =
    QgsMapLayerRegistry::instance()->mapLayers();

  for ( QMap<QString, QgsMapLayer*>::iterator it  = mapLayers.begin();
        it != mapLayers.end();
        ++it )
  {
    QgsDebugMsg( QString( "QgsMapLayerRegistry has %1." ).arg( it.value()->name() ) );

    // TODO: Test if a VECTOR or DATABASE layer only (not RASTER)

    sourceLayerComboBox     ->addItem( it.value()->name() );
    destinationLayerComboBox->addItem( it.value()->name() );

    // store the lookup from the name to the map layer object
    mMapNameLookup[ it.value()->name()] = it.value();
  }
}

QgsPasteTransformations::~QgsPasteTransformations()
{

}

void QgsPasteTransformations::accept()
{
  QSettings settings;
  QString baseKey        = "/Qgis/paste-transformations";             // TODO: promote to static member
  QString sourceKey      = sourceLayerComboBox     ->currentText();
  QString destinationKey = destinationLayerComboBox->currentText();

  for ( uint i = 0; i < mSourceTransfers.size(); i++ )
  {
    settings.setValue(
      baseKey + "/" + sourceKey + "/" + destinationKey + "/" +
      mSourceTransfers[i]     ->currentText(),
      mDestinationTransfers[i]->currentText()
    );
  }
  QDialog::accept();
}

void QgsPasteTransformations::help()
{
  helpInfo();
}

void QgsPasteTransformations::helpInfo()
{
  QgsContextHelp::run( context_id );
}

void QgsPasteTransformations::addNewTransfer()
{
  // This ends up being a wrapper for addTransfer, but with no preselected fields
  addTransfer();
}

void QgsPasteTransformations::sourceChanged( const QString& layerName )
{
  QgsDebugMsg( QString( "Source changed to %1." ).arg( layerName ) );
  layerChanged( layerName, &mSourceFields );
}


void QgsPasteTransformations::destinationChanged( const QString& layerName )
{
  QgsDebugMsg( QString( "Destination changed to %1." ).arg( layerName ) );
  layerChanged( layerName, &mDestinationFields );
}

void QgsPasteTransformations::addTransfer( const QString& sourceSelectedFieldName,
    const QString& destinationSelectedFieldName )
{
  QgsDebugMsg( QString( "From %1 to %2." ).arg( sourceSelectedFieldName ).arg( destinationSelectedFieldName ) );

  int newRow = gridLayout->rowCount();

// TODO: Do not add the transfer if neither the sourceSelectedFieldName nor the destinationSelectedFieldName could be found.

  // For some reason Qt4's uic3 only outputs generic names for layout items
  QComboBox* newSourceFields      = new QComboBox( gridLayout->parentWidget() );
  QComboBox* newDestinationFields = new QComboBox( gridLayout->parentWidget() );

  int count = 0;

  // Populate source fields
  for ( std::vector<QString>::iterator it  = mSourceFields.begin();
        it != mSourceFields.end();
        ++it )
  {
    newSourceFields->addItem(( *it ) );

    // highlight this item if appropriate
    if ( sourceSelectedFieldName == ( *it ) )
    {
      newSourceFields->setCurrentIndex( count );
    }

    count++;
  }

  count = 0;

  // Populate destination fields
  for ( std::vector<QString>::iterator it  = mDestinationFields.begin();
        it != mDestinationFields.end();
        ++it )
  {
    newDestinationFields->addItem(( *it ) );

    // highlight this item if appropriate
    if ( destinationSelectedFieldName == ( *it ) )
    {
      newDestinationFields->setCurrentIndex( count );
    }

    count++;
  }

  // Append to dialog layout
  gridLayout->addWidget( newSourceFields,      newRow, 0 );
  gridLayout->addWidget( newDestinationFields, newRow, 1 );

  // Keep a reference to them so that we can read from them
  // when the dialog is dismissed
  mSourceTransfers     .push_back( newSourceFields );
  mDestinationTransfers.push_back( newDestinationFields );

  // Reveal the new sub-widgets
  newSourceFields->show();
  newDestinationFields->show();

}


void QgsPasteTransformations::layerChanged( const QString& layerName, std::vector<QString>* fields )
{
  // Fetch the fields that will be populated into the Transfer rows.
  QgsDebugMsg( QString( "Layer changed to %1." ).arg( layerName ) );

  restoreTransfers( sourceLayerComboBox     ->currentText(),
                    destinationLayerComboBox->currentText() );
}

void QgsPasteTransformations::restoreTransfers( const QString& sourceLayerName,
    const QString& destinationLayerName )
{
  QgsDebugMsg( "entered." );
  QSettings settings;
  QString baseKey = "/Qgis/paste-transformations";             // TODO: promote to static member

  settings.beginGroup( baseKey );
  QStringList sourceLayers = settings.childGroups();

  for ( QStringList::Iterator it  = sourceLayers.begin();
        it != sourceLayers.end();
        ++it )
  {
    QgsDebugMsg( QString( "testing source '%1' with '%2'." ).arg(( *it ) ).arg( sourceLayerName ) );
    if (( sourceLayerName == ( *it ) ) )
    {
      // Go through destination layers defined for this source layer.
      settings.beginGroup( *it );
      QStringList destinationLayers = settings.childGroups();
      for ( QStringList::Iterator it2  = destinationLayers.begin();
            it2 != destinationLayers.end();
            ++it2 )
      {
        QgsDebugMsg( QString( "testing destination '%1' with '%2'." ).arg(( *it2 ) ).arg( destinationLayerName ) );
        if (( destinationLayerName == ( *it2 ) ) )
        {
          QgsDebugMsg( "going through transfers." );
          // Go through Transfers for this source/destination layer pair.
          settings.beginGroup( *it2 );
          QStringList transfers = settings.childKeys();
          for ( QStringList::Iterator it3  = transfers.begin();
                it3 != transfers.end();
                ++it3 )
          {
            QgsDebugMsg( QString( "setting transfer for %1." ).arg(( *it3 ) ) );
            QString destinationField = settings.value( *it3 ).toString();
            addTransfer(( *it3 ), destinationField );
          }
          settings.endGroup();
        }
      }
      settings.endGroup();
    }
  }
  settings.endGroup();
}


QString QgsPasteTransformations::pasteTo( const QString& sourceLayerName,
    const QString& destinationLayerName,
    const QString& sourceFieldName )
{

// TODO: Adjust QgsVectorLayer::addFeature to complete the usefulness of this function
// TODO: Cache previous results as this will be called once per pasted feature.

  QgsDebugMsg( "entered." );
  QSettings settings;
  QString baseKey = "/Qgis/paste-transformations";             // TODO: promote to static member

  QString destinationField =
    settings.value( baseKey + "/" + sourceLayerName + "/" + destinationLayerName + "/" + sourceFieldName ).toString();

  if ( QString::null == destinationField )
  {
    destinationField = sourceFieldName;
  }

  QgsDebugMsg( QString( "returning '%1'." ).arg( destinationField ) );

  return destinationField;
}

