/***************************************************************************
    qgscrsdefinitionwidget.h
    ---------------------
    begin                : December 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCRSDEFINITIONWIDGET_H
#define QGSCRSDEFINITIONWIDGET_H

#include "ui_qgscrsdefinitionwidgetbase.h"
#include "qgscoordinatereferencesystem.h"

#include "qgis_gui.h"
#include "qgis_sip.h"

class QgsCoordinateReferenceSystem;

/**
 * \brief A widget for definition a custom coordinate reference system.
 * \ingroup gui
 * \since QGIS 3.24
*/
class GUI_EXPORT QgsCrsDefinitionWidget : public QWidget, private Ui::QgsCrsDefinitionWidgetBase
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsCrsDefinitionWidget, with the specified \a parent widget.
     */
    QgsCrsDefinitionWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the current CRS as defined in the widget.
     *
     * An invalid CRS may be returned if no CRS is defined in the widget.
     *
     * \see setCrs()
     */
    QgsCoordinateReferenceSystem crs() const;

    /**
     * Sets the current \a crs to display in the widget.
     *
     * \see crs()
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Sets the current \a crs to display in the widget.
     *
     * The \a nativeFormat argument specifies the format (e.g. WKT or PROJ) is natively associated
     * with the custom CRS.
     *
     * \see crs()
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs, Qgis::CrsDefinitionFormat nativeFormat );

    /**
     * Returns the selected CRS format.
     *
     * \see setFormat()
     */
    Qgis::CrsDefinitionFormat format() const;

    /**
     * Sets the CRS \a format.
     *
     * \see format()
     */
    void setFormat( Qgis::CrsDefinitionFormat format );

    /**
     * Returns the current definition string.
     *
     * This represents the unaltered user-entered definition string, which may represent
     * either a WKT or PROJ string (see format()), and may not represent a valid CRS definition.
     *
     * \see setDefinitionString()
     */
    QString definitionString() const;

    /**
     * Sets the current \a definition string.
     *
     * This represents the unaltered user-entered definition string, which may represent
     * either a WKT or PROJ string (see format()), and may not represent a valid CRS definition.
     *
     * \see definitionString()
     */
    void setDefinitionString( const QString &definition );

  signals:

    /**
     * Emitted when the CRS defined in the widget is changed.
     */
    void crsChanged();

  private slots:
    void pbnCalculate_clicked();
    void pbnCopyCRS_clicked();
    void validateCurrent();
    void formatChanged();

  private:

    QString multiLineWktToSingleLine( const QString &wkt );

};

#endif // QGSCRSDEFINITIONWIDGET_H
