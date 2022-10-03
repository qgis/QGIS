/***************************************************************************
  qgsrasterattributetablemodel.h - QgsRasterAttributeTableModel

 ---------------------
 begin                : 29.9.2022
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
#ifndef QGSRASTERATTRIBUTETABLEMODEL_H
#define QGSRASTERATTRIBUTETABLEMODEL_H

#include <QAbstractTableModel>
#include <QObject>
#include "qgis_core.h"
#include "qgsrasterattributetable.h"
#include "qgis_sip.h"

/**
 * \ingroup core
 * \brief The QgsRasterAttributeTableModel class manages a QgsRasterAttributeTable
 * \since QGIS 3.30
 */
class CORE_EXPORT QgsRasterAttributeTableModel : public QAbstractTableModel
{
    Q_OBJECT

  public:

    /**
     * Creates a new QgsRasterAttributeTableModel from raster attribute table \a rat and optional \a parent.
     */
    explicit QgsRasterAttributeTableModel( QgsRasterAttributeTable *rat, QObject *parent = nullptr );

    /**
     * Returns true if the RAT is editable.
     */
    bool editable() const;

    /**
     * Sets the RAT editable state to \a editable.
     */
    void setEditable( bool editable );

    /**
     * Returns TRUE if the RAT has color information.
     */
    bool hasColor() const;

    /**
     * Returns TRUE if the RAT has ramp information.
     */
    bool hasRamp() const;

    /**
     * Returns all the header names, including the "virtual" color header if the RAT has color or ramp.
     */
    QStringList headerNames( ) const;

    /**
     * Returns the tooltip for the given \a section.
     */
    QString headerTooltip( const int section ) const;

    // RAT operations

    /**
     * Inserts a field at the given position.
     * \param name field name
     * \param usage field usage
     * \param type field type
     * \param position insertion point (before)
     * \param errorMessage optional error message
     * \returns true on success
     */
    bool insertField( const QString &name, const Qgis::RasterAttributeTableFieldUsage usage, const QVariant::Type type, const int position, QString *errorMessage SIP_OUT = nullptr );
    bool removeField( const int position, QString *errorMessage SIP_OUT = nullptr );

    /**
     * Removes all color or ramp information, returns TRUE on success.
     */
    bool removeColorOrRamp( QString *errorMessage SIP_OUT = nullptr );

    /**
     * Inserts a new row before \a position, returns TRUE on success.
     */
    bool insertRow( const QVariantList &rowData, const int position, QString *errorMessage SIP_OUT = nullptr );

    /**
     * Removes the row at \a position, returns TRUE on success.
     */
    bool removeRow( const int position, QString *errorMessage SIP_OUT = nullptr );

    //! Translatable name of the "color" header.
    static QString RAT_COLOR_HEADER_NAME;

  private:

    QgsRasterAttributeTable *mRat = nullptr;
    bool mEditable = false;

    // Checks for rat not nullptr and editable state
    bool editChecks( QString *errorMessage = nullptr );

    // QAbstractItemModel interface
  public:
    int rowCount( const QModelIndex &parent ) const override;
    int columnCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role ) override;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;

    Qt::ItemFlags flags( const QModelIndex &index ) const override;


};

#endif // QGSRASTERATTRIBUTETABLEMODEL_H
