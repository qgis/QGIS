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

#include "qgis_gui.h"
#include "qgsrasterattributetable.h"
#include "qgis_sip.h"

#include <QAbstractTableModel>
#include <QObject>

/**
 * \ingroup core
 * \brief The QgsRasterAttributeTableModel class manages a QgsRasterAttributeTable
 * \since QGIS 3.30
 */
class GUI_EXPORT QgsRasterAttributeTableModel : public QAbstractTableModel
{
    Q_OBJECT

  public:

    /**
     * Creates a new QgsRasterAttributeTableModel from raster attribute table \a rat and optional \a parent.
     */
    explicit QgsRasterAttributeTableModel( QgsRasterAttributeTable *rat, QObject *parent  SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns true if the Raster Attribute Table is editable.
     */
    bool editable() const;

    /**
     * Sets the Raster Attribute Table editable state to \a editable.
     */
    void setEditable( bool editable );

    /**
     * Returns TRUE if the Raster Attribute Table has color information.
     */
    bool hasColor() const;

    /**
     * Returns TRUE if the Raster Attribute Table has ramp information.
     */
    bool hasRamp() const;

    /**
     * Returns all the header names, including the "virtual" color header if the Raster Attribute Table has color or ramp.
     */
    QStringList headerNames( ) const;

    /**
     * Returns the tooltip for the given \a section.
     */
    QString headerTooltip( const int section ) const;

    /**
     * Checks if the Raster Attribute Table is valid, optionally returns validation errors in \a errorMessage.
     */
    bool isValid( QString *errorMessage SIP_OUT = nullptr );

    /**
     * Returns TRUE if the Raster Attribute Table was modified since it was last saved or read.
     */
    bool isDirty( );

    // Raster Attribute Table operations

    /**
     * Inserts a field at the given position.
     * \param name field name
     * \param usage field usage
     * \param type field type
     * \param position insertion point (before)
     * \param errorMessage optional error message
     * \returns true on success
     */
    bool insertField( const int position, const QString &name, const Qgis::RasterAttributeTableFieldUsage usage, const QVariant::Type type, QString *errorMessage SIP_OUT = nullptr );

    /**
     * Remove the field at given \a position, optionally reporting any error in \a errorMessage, returns TRUE on success.
     */
    bool removeField( const int position, QString *errorMessage SIP_OUT = nullptr );

    /**
     * Removes all color or ramp information, optionally reporting any error in \a errorMessage, returns TRUE on success.
     */
    bool removeColorOrRamp( QString *errorMessage SIP_OUT = nullptr );

    /**
     * Inserts a new row before \a position, optionally reporting any error in \a errorMessage, returns TRUE on success.
     */
    bool insertRow( const int position, const QVariantList &rowData, QString *errorMessage SIP_OUT = nullptr );

    /**
     * Create RGBA fields and inserts them at \a position, optionally reporting any error in \a errorMessage, returns TRUE on success.
     */
    bool insertColor( int position, QString *errorMessage SIP_OUT = nullptr );

    /**
     * Create RGBA minimum and maximum fields and inserts them at \a position, optionally reporting any error in \a errorMessage, returns TRUE on success.
     */
    bool insertRamp( int position, QString *errorMessage SIP_OUT = nullptr );

    /**
     * Removes the row at \a position, optionally reporting any error in \a errorMessage, returns TRUE on success.
     */
    bool removeRow( const int position, QString *errorMessage SIP_OUT = nullptr );

  private:

    QgsRasterAttributeTable *mRat = nullptr;
    bool mEditable = false;

    // Checks for rat not nullptr and editable state
    bool editChecks( QString *errorMessage = nullptr );

    QString ratColorHeaderName( ) const;

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
