/***************************************************************************
  qgslayermetadataresultsmodel.h - QgsLayerMetadataResultsModel

 ---------------------
 begin                : 1.9.2022
 copyright            : (C) 2022 by ale
 email                : [your-email-here]
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYERMETADATARESULTSMODEL_H
#define QGSLAYERMETADATARESULTSMODEL_H

#include <QAbstractTableModel>
#include <QObject>
#include "qgis_gui.h"
#include "qgsabstractlayermetadataprovider.h"

class QgsFeedback;

class GUI_EXPORT QgsLayerMetadataResultsModel : public QAbstractTableModel
{
  public:
    explicit QgsLayerMetadataResultsModel( const QgsMetadataSearchContext &searchContext, QgsFeedback *feedback, QObject *parent = nullptr );

    // QAbstractTableModel interface
  public:

    int rowCount( const QModelIndex &parent ) const override;
    int columnCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;

    //! Reload model data
    void reload( );

  private:

    QgsFeedback *mFeedback;
    QgsLayerMetadataSearchResults mResult;
    QgsMetadataSearchContext mSearchContext;
};

#endif // QGSLAYERMETADATARESULTSMODEL_H
