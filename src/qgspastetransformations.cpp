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

#include <iostream>

#include <qsettings.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>

#include "qgspastetransformations.h"
#include "qgsmaplayerregistry.h"

QgsPasteTransformations::QgsPasteTransformations()
  : QgsPasteTransformationsBase()
{

  // Populate the dialog with the loaded layers
  std::map<QString, QgsMapLayer*> mapLayers =
    QgsMapLayerRegistry::instance()->mapLayers();

  for (std::map<QString, QgsMapLayer*>::iterator it  = mapLayers.begin();
                                                 it != mapLayers.end();
                                               ++it )
  {
#ifdef QGISDEBUG
        std::cerr << "QgsPasteTransformations::QgsPasteTransformations: QgsMapLayerRegistry has "
          << it->second->name() << "."
          << std::endl;
#endif

    // TODO: Test if a VECTOR or DATABASE layer only (not RASTER)

    sourceLayerComboBox     ->insertItem( it->second->name() );
    destinationLayerComboBox->insertItem( it->second->name() );

    // store the lookup from the name to the map layer object
    mMapNameLookup[ it->second->name() ] = it->second;
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

  for (int i = 0; i < mSourceTransfers.size(); i++)
  {
    settings.writeEntry(
                        baseKey + "/" + sourceKey + "/" + destinationKey + "/" +
                        mSourceTransfers[i]     ->currentText(),
                        mDestinationTransfers[i]->currentText()
                       );
  }

  QDialog::accept();

}


void QgsPasteTransformations::addNewTransfer()
{
  // This ends up being a wrapper for addTransfer, but with no preselected fields
  addTransfer();
}


void QgsPasteTransformations::sourceChanged(const QString& layerName)
{
#ifdef QGISDEBUG
        std::cerr << "QgsPasteTransformations::sourceChanged: Source changed to "
          << layerName << "."
          << std::endl;
#endif

  layerChanged(layerName, &mSourceFields);

}


void QgsPasteTransformations::destinationChanged(const QString& layerName)
{
#ifdef QGISDEBUG
        std::cerr << "QgsPasteTransformations::destinationChanged: Destination changed to "
          << layerName << "."
          << std::endl;
#endif

  layerChanged(layerName, &mDestinationFields);

}


void QgsPasteTransformations::addTransfer(const QString& sourceSelectedFieldName,
                                          const QString& destinationSelectedFieldName)
{
#ifdef QGISDEBUG
        std::cerr << "QgsPasteTransformations::addTransfer: From " << sourceSelectedFieldName 
                                                         << " to " << destinationSelectedFieldName << "."
          << std::endl;
#endif
  int newRow = transferLayout->numRows();

// TODO: Do not add the transfer if neither the sourceSelectedFieldName nor the destinationSelectedFieldName could be found.

  // Build a Transfer row at the end of the grid layout.
  QComboBox* newSourceFields      = new QComboBox(FALSE, transferLayout->mainWidget() );
  QComboBox* newDestinationFields = new QComboBox(FALSE, transferLayout->mainWidget() );

  int count = 0;

  // Populate source fields
  for (std::vector<QString>::iterator it  = mSourceFields.begin();
                                      it != mSourceFields.end();
                                    ++it )
  {
    newSourceFields->insertItem( (*it) );

    // highlight this item if appropriate
    if (sourceSelectedFieldName == (*it))
    {
      newSourceFields->setCurrentItem(count);
    }

    count++;
  }

  count = 0;

  // Populate destination fields
  for (std::vector<QString>::iterator it  = mDestinationFields.begin();
                                      it != mDestinationFields.end();
                                    ++it )
  {
    newDestinationFields->insertItem( (*it) );

    // highlight this item if appropriate
    if (destinationSelectedFieldName == (*it))
    {
      newDestinationFields->setCurrentItem(count);
    }

    count++;
  }

  // Append to dialog layout
  transferLayout->addWidget(newSourceFields,      newRow, 0);
  transferLayout->addWidget(newDestinationFields, newRow, 1);

  // Keep a reference to them so that we can read from them
  // when the dialog is dismissed
  mSourceTransfers     .push_back(newSourceFields);
  mDestinationTransfers.push_back(newDestinationFields);

  // Reveal the new sub-widgets
  newSourceFields->show();
  newDestinationFields->show();

}


void QgsPasteTransformations::layerChanged(const QString& layerName, std::vector<QString>* fields)
{
  // Fetch the fields that will be populated into the Transfer rows.
#ifdef QGISDEBUG
        std::cerr << "QgsPasteTransformations::layerChanged: Layer changed to "
          << layerName << "."
          << std::endl;
#endif

  std::vector<QgsField> layerFields = 
    (mMapNameLookup[ layerName ])->fields();

  fields->clear();

  for (std::vector<QgsField>::iterator it  = layerFields.begin();
                                       it != layerFields.end();
                                     ++it )
  {
#ifdef QGISDEBUG
        std::cerr << "QgsPasteTransformations::layerChanged: Got field "
          << it->name() << "."
          << std::endl;
#endif

    fields->push_back(it->name());
  }

  restoreTransfers( sourceLayerComboBox     ->currentText(),
                    destinationLayerComboBox->currentText() );
}

void QgsPasteTransformations::restoreTransfers(const QString& sourceLayerName,
                                               const QString& destinationLayerName)
{
#ifdef QGISDEBUG
        std::cerr << "QgsPasteTransformations::restoreTransfers: Entered."
          << std::endl;
#endif
  QSettings settings;
  QString baseKey = "/Qgis/paste-transformations";             // TODO: promote to static member

  QStringList sourceLayers = settings.subkeyList(baseKey);

  for (QStringList::Iterator it  = sourceLayers.begin(); 
                             it != sourceLayers.end();
                           ++it )
  {
#ifdef QGISDEBUG
        std::cerr << "QgsPasteTransformations::restoreTransfers: Testing source '"
          << (*it) << "' with '"
          << sourceLayerName << "'."
          << std::endl;
#endif
    if ((sourceLayerName == (*it)))
    {
      // Go through destination layers defined for this source layer.
      QStringList destinationLayers = settings.subkeyList( baseKey + "/" + (*it) );
      for (QStringList::Iterator it2  = destinationLayers.begin(); 
                                 it2 != destinationLayers.end();
                               ++it2 )
      {
#ifdef QGISDEBUG
        std::cerr << "QgsPasteTransformations::restoreTransfers: Testing destination '"
          << (*it2) << "' with '"
          << destinationLayerName << "'."
          << std::endl;
#endif
        if ((destinationLayerName == (*it2)))
        {
#ifdef QGISDEBUG
        std::cerr << "QgsPasteTransformations::restoreTransfers:going through transfers."
          << std::endl;
#endif
          // Go through Transfers for this source/destination layer pair.
          QStringList transfers = settings.entryList( baseKey + "/" + (*it) + "/" + (*it2) );
          for (QStringList::Iterator it3  = transfers.begin(); 
                                     it3 != transfers.end();
                                   ++it3 )
          {
#ifdef QGISDEBUG
        std::cerr << "QgsPasteTransformations::restoreTransfers: setting transfer for "
          << (*it3) << "."
          << std::endl;
#endif
            QString destinationField = 
              settings.readEntry( baseKey + "/" + (*it) + "/" + (*it2) + "/" + (*it3) );
            addTransfer( (*it3), destinationField );
          }
        }
      }
    }
  }
}


QString QgsPasteTransformations::pasteTo(const QString& sourceLayerName,
                                         const QString& destinationLayerName,
                                         const QString& sourceFieldName)
{

// TODO: Adjust QgsVectorLayer::addFeature to complete the usefulness of this function
// TODO: Cache previous results as this will be called once per pasted feature.

#ifdef QGISDEBUG
        std::cerr << "QgsPasteTransformations::pasteTo: Entered."
          << std::endl;
#endif
  QSettings settings;
  QString baseKey = "/Qgis/paste-transformations";             // TODO: promote to static member

  QString destinationField = 
    settings.readEntry( baseKey + "/" + sourceLayerName + "/" + destinationLayerName + "/" + sourceFieldName );

  if (QString::null == destinationField)
  {
    destinationField = sourceFieldName;
  }

#ifdef QGISDEBUG
        std::cerr << "QgsPasteTransformations::pasteTo: Returning '" << destinationField << "'."
          << std::endl;
#endif

  return destinationField;
}

