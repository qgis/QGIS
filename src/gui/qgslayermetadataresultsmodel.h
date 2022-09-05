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
#include <QThread>
#include "qgis_gui.h"
#include "qgsabstractlayermetadataprovider.h"

class QgsFeedback;


///@cond private

#ifndef SIP_RUN

/**
 * The QgsMetadataResultFetcher class fetches query results from a separate thread
 * WARNING: this class is an implementation detail and it is not part of public API!
 */
class QgsMetadataResultsFetcher: public QObject
{
    Q_OBJECT

  public:

    //! Constructs a metadata result fetcher.
    QgsMetadataResultsFetcher( const QgsAbstractLayerMetadataProvider *metadataProvider, const QgsMetadataSearchContext &searchContext, QgsFeedback *feedback );

    //! Start fetching.
    void fetchMetadata( );

  signals:

    //! Emitted when \a results have been fetched
    void resultsReady( const QgsLayerMetadataSearchResults &results );

  private:

    const QgsAbstractLayerMetadataProvider *mLayerMetadataProvider = nullptr;
    QgsMetadataSearchContext mSearchContext;
    QgsFeedback *mFeedback;
};

#endif

///@endcond private

class GUI_EXPORT QgsLayerMetadataResultsModel : public QAbstractTableModel
{
    Q_OBJECT

  public:
    explicit QgsLayerMetadataResultsModel( const QgsMetadataSearchContext &searchContext, QObject *parent = nullptr );

    ~QgsLayerMetadataResultsModel();


    // QAbstractTableModel interface

    int rowCount( const QModelIndex &parent ) const override;
    int columnCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;

    //! Load/Reload model data
    void reload( );

    //! Load/Reload model data
    void reloadAsync( );

    enum Roles
    {
      Metadata = Qt::ItemDataRole::UserRole,
    };

    enum Sections
    {
      Identifier,
      Title,
      Abstract,
      DataProviderName,
      GeometryType,
    };

  public slots:

    /**
     * Triggered when \a results have been fetched and can be added to the model.
     */
    void resultsReady( const QgsLayerMetadataSearchResults &results );

    /**
     * Cancels the results fetching.
     */
    void cancel();

  signals:

    void progressChanged( int progress );

  private:

    std::unique_ptr<QgsFeedback> mFeedback;
    QgsLayerMetadataSearchResults mResult;
    QgsMetadataSearchContext mSearchContext;
    std::vector<std::unique_ptr<QgsMetadataResultsFetcher>> mWorkers;
    std::vector<std::unique_ptr<QThread>> mWorkerThreads;

};

#endif // QGSLAYERMETADATARESULTSMODEL_H
