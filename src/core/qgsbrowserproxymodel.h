/***************************************************************************
    qgsbrowserproxymodel.h
    ---------------------
    begin                : October 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSBROWSERPROXYMODEL_H
#define QGSBROWSERPROXYMODEL_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsmaplayer.h"
#include <QSortFilterProxyModel>

class QgsBrowserModel;
class QgsDataItem;

/**
 * \class QgsBrowserProxyModel
 * \ingroup core
 * A QSortFilterProxyModel subclass for filtering and sorting browser model items.
 * \since QGIS 3.4
 */
class CORE_EXPORT QgsBrowserProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
  public:

    //! Filter syntax options
    enum FilterSyntax
    {
      Normal, //!< Standard string filtering
      Wildcards, //!< Wildcard filtering
      RegularExpression, //!< Regular expression filtering
    };

    /**
      * Constructor for QgsBrowserProxyModel, with the specified \a parent object.
      */
    explicit QgsBrowserProxyModel( QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the underlying browser \a model.
     *
     * \see browserModel()
     */
    void setBrowserModel( QgsBrowserModel *model );

    /**
     * Returns the underlying browser model.
     *
     * \see setBrowserModel()
     */
    QgsBrowserModel *browserModel() { return mModel; }

    /**
     * Returns the data item at the specified proxy \a index, or NULLPTR if no item
     * exists at the index.
     */
    QgsDataItem *dataItem( const QModelIndex &index ) const;

    /**
     * Sets the filter \a syntax.
     *
     * \see filterSyntax()
     */
    void setFilterSyntax( FilterSyntax syntax );

    /**
     * Returns the filter syntax.
     *
     * \see setFilterSyntax()
     */
    FilterSyntax filterSyntax() const;

    /**
     * Sets the \a filter string to use when filtering items in the model.
     *
     * \see filterString()
     */
    void setFilterString( const QString &filter );

    /**
     * Returns the filter string used when filtering items in the model.
     *
     * \see setFilterString()
     */
    QString filterString() const;

    /**
     * Sets whether item filtering should be case sensitive.
     *
     * \see caseSensitivity()
     */
    void setFilterCaseSensitivity( Qt::CaseSensitivity sensitivity );

    /**
     * Returns whether item filtering is case sensitive.
     *
     * \see setFilterCaseSensitivity()
     */
    Qt::CaseSensitivity caseSensitivity() const;

    /**
     * Returns TRUE if the model is filtered by map layer type.
     *
     * \see layerType()
     * \see setFilterByLayerType()
     */
    bool filterByLayerType() const { return mFilterByLayerType; }

    /**
     * Sets whether the model is filtered by map layer type.
     *
     * \see filterByLayerType()
     * \see setLayerType()
     */
    void setFilterByLayerType( bool enabled );

    /**
     * Returns the layer type to filter the model by. This is only used if
     * filterByLayerType() is TRUE.
     *
     * \see setLayerType()
     * \see filterByLayerType()
     */
    QgsMapLayer::LayerType layerType() const;

    /**
     * Sets the layer \a type to filter the model by. This is only used if
     * filterByLayerType() is TRUE.
     *
     * \see layerType()
     * \see setFilterByLayerType()
     */
    void setLayerType( QgsMapLayer::LayerType type );

  protected:

    // It would be better to apply the filer only to expanded (visible) items, but using mapFromSource() + view here was causing strange errors
    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override;

  private:

    QgsBrowserModel *mModel = nullptr;
    QString mFilter; //filter string provided
    QVector<QRegExp> mREList; //list of filters, separated by "|"
    FilterSyntax mPatternSyntax = Normal;
    Qt::CaseSensitivity mCaseSensitivity = Qt::CaseInsensitive;

    bool mFilterByLayerType = false;
    QgsMapLayer::LayerType mLayerType = QgsMapLayer::VectorLayer;

    //! Update filter
    void updateFilter();

    //! Filter accepts string
    bool filterAcceptsString( const QString &value ) const;

    //! Returns TRUE if at least one ancestor is accepted by filter
    bool filterAcceptsAncestor( const QModelIndex &sourceIndex ) const;

    //! Returns TRUE if at least one descendant s accepted by filter
    bool filterAcceptsDescendant( const QModelIndex &sourceIndex ) const;

    //! Filter accepts item name
    bool filterAcceptsItem( const QModelIndex &sourceIndex ) const;
};

#endif // QGSBROWSERPROXYMODEL_H
