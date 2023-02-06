/***************************************************************************
  qgslayermetadataresultsproxymodel.h - QgsLayerMetadataResultsProxyModel

 ---------------------
 begin                : 1.9.2022
 copyright            : (C) 2022 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYERMETADATARESULTSPROXYMODEL_H
#define QGSLAYERMETADATARESULTSPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QObject>
#include "qgis_gui.h"
#include "qgsrectangle.h"
#include "qgswkbtypes.h"

/**
 * The QgsLayerMetadataResultsProxyModel class is a proxy model for QgsLayerMetadataResultsModel,
 * it handles text and extent filtering.
 * \ingroup gui
 * \since QGIS 3.28
 */
class GUI_EXPORT QgsLayerMetadataResultsProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

  public:

    /**
     * Constructs a QgsLayerMetadataResultsProxyModel with an optional \a parent.
     */
    explicit QgsLayerMetadataResultsProxyModel( QObject *parent = nullptr );

    /**
     * Returns the filter string.
     */
    const QString filterString() const;

    /**
     * Sets the geometry type filter status to \a enabled.
     */
    void setFilterGeometryTypeEnabled( bool enabled );

    /**
     * Sets the map layer type filter status to \a enabled.
     */
    void setFilterMapLayerTypeEnabled( bool enabled );

  public slots:

    /**
     * Sets the extent filter to \a extent.
     */
    void setFilterExtent( const QgsRectangle &extent );

    /**
     * Sets the geometry type filter to \a geometryType.
     */
    void setFilterGeometryType( const QgsWkbTypes::GeometryType geometryType );

    /**
     * Sets the text filter to \a filterString.
     */
    void setFilterString( const QString &filterString );

    /**
     * Sets the map layer type filter to \a mapLayerType.
     */
    void setFilterMapLayerType( const QgsMapLayerType mapLayerType );

    // QSortFilterProxyModel interface
  protected:
    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override;


  private:

    QgsRectangle mFilterExtent;
    QString mFilterString;
    QgsWkbTypes::GeometryType mFilterGeometryType = QgsWkbTypes::GeometryType::PointGeometry;
    QgsMapLayerType mFilterMapLayerType = QgsMapLayerType::VectorLayer;
    bool mFilterGeometryTypeEnabled = false;
    bool mFilterMapLayerTypeEnabled = false;
};

#endif // QGSLAYERMETADATARESULTSPROXYMODEL_H
