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
     * Constructs a QgsLayerMetadataResultsProxyModel with an optional parent.
     * @param parent
     */
    explicit QgsLayerMetadataResultsProxyModel( QObject *parent = nullptr );

  public slots:

    /**
     * Sett the extent filter to \a extent.
     */
    void setFilterExtent( const QgsRectangle &extent );

    /**
     * Sets the text filter to \a filterString.
     */
    void setFilterString( const QString &filterString );

    // QSortFilterProxyModel interface
  protected:
    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override;

  private:

    QgsRectangle mFilterExtent;
    QString mFilterString;
};

#endif // QGSLAYERMETADATARESULTSPROXYMODEL_H
