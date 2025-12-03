/***************************************************************************
   qgsvirtualrasterprovider.h - QgsRasterDataProvider
     --------------------------------------
    Date                 : June 10, 2021
    Copyright            : (C) 2021 by Francesco Bursi
    email                : francesco dot bursi at hotmail dot it
***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSVIRTUALRASTERPROVIDER_H
#define QGSVIRTUALRASTERPROVIDER_H

#include "qgsprovidermetadata.h"
#include "qgsrastercalculator.h"
#include "qgsrasterdataprovider.h"

#include <QObject>

/**
 * The QgsVirtualRasterProvider class implements a raster data provider that enable the use of the raster calculator
 * without the need of save on the disk a derived file.
 */

class QgsVirtualRasterProvider : public QgsRasterDataProvider
{
    Q_OBJECT
  public:
    QgsVirtualRasterProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions );
    ~QgsVirtualRasterProvider() override;

    QgsVirtualRasterProvider &operator=( QgsVirtualRasterProvider other ) = delete;

    QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = nullptr ) override;
    bool readBlock( int bandNo, QgsRectangle const &viewExtent, int width, int height, void *data, QgsRasterBlockFeedback *feedback = nullptr ) override
    {
      Q_UNUSED( bandNo )
      Q_UNUSED( viewExtent );
      Q_UNUSED( width );
      Q_UNUSED( height );
      Q_UNUSED( data );
      Q_UNUSED( feedback );
      return true;
    }

    // QgsDataProvider interface
    Qgis::DataProviderFlags flags() const override;
    bool isValid() const override;
    QgsCoordinateReferenceSystem crs() const override;
    QgsRectangle extent() const override;
    QString name() const override;
    QString description() const override;

    // QgsRasterInterface interface
    int xBlockSize() const override;
    int yBlockSize() const override;
    int bandCount() const override;
    QgsVirtualRasterProvider *clone() const override;
    Qgis::DataType dataType( int bandNo ) const override;
    Qgis::DataType sourceDataType( int bandNo ) const override;

    int xSize() const override;
    int ySize() const override;

    // QgsRasterDataProvider interface
    QString htmlMetadata() const override;
    QString lastErrorTitle() override;
    QString lastError() override;
    Qgis::RasterInterfaceCapabilities capabilities() const override;

    static QString providerKey();

    QString formulaString() const;

  private:
    QgsVirtualRasterProvider( const QgsVirtualRasterProvider &other );

    bool mValid = false;
    QgsCoordinateReferenceSystem mCrs;
    QgsRectangle mExtent;
    int mWidth = 0;
    int mHeight = 0;
    int mBandCount = 1;
    int mXBlockSize = 0;
    int mYBlockSize = 0;

    //from qgsrastercalculator
    QString mFormulaString;
    QVector<QgsRasterCalculatorEntry> mRasterEntries;
    QString mLastError;

    std::unique_ptr<QgsRasterCalcNode> mCalcNode;
    QVector<QgsRasterLayer *> mRasterLayers;
};

class QgsVirtualRasterProviderMetadata : public QgsProviderMetadata
{
    Q_OBJECT
  public:
    QgsVirtualRasterProviderMetadata();
    QIcon icon() const override;
    QgsVirtualRasterProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags = Qgis::DataProviderReadFlags() ) override;
    QString absoluteToRelativeUri( const QString &uri, const QgsReadWriteContext &context ) const override;
    QString relativeToAbsoluteUri( const QString &uri, const QgsReadWriteContext &context ) const override;
    QList<Qgis::LayerType> supportedLayerTypes() const override;
};

#endif // QGSVIRTUALRASTERPROVIDER_H
