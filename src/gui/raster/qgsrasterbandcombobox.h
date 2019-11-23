/***************************************************************************
    qgsrasterbandcombobox.h
    -----------------------
    begin                : May 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERBANDCOMBOBOX_H
#define QGSRASTERBANDCOMBOBOX_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "raster/qgsrasterlayer.h"
#include <QComboBox>

class QgsMapLayer;
class QgsRasterDataProvider;

/**
 * \class QgsRasterBandComboBox
 * \ingroup gui
 * A combobox widget which displays the bands present in a raster layer.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsRasterBandComboBox : public QComboBox
{
    Q_OBJECT
    Q_PROPERTY( int band READ currentBand WRITE setBand NOTIFY bandChanged )

  public:

    /**
     * Constructor for QgsRasterBandComboBox.
     */
    QgsRasterBandComboBox( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the layer currently associated with the combobox.
     * \see setLayer()
     */
    QgsRasterLayer *layer() const;

    /**
     * Returns the current band number selected in the combobox, or -1
     * if no band is selected.
     * \see setBand()
     */
    int currentBand() const;

    /**
     * Returns TRUE if the combo box is showing the "not set" option.
     * \see setShowNotSetOption()
     */
    bool isShowingNotSetOption() const;

    /**
     * Sets whether the combo box should show the "not set" option.
     * Optionally the built in "not set" text can be overridden by specifying
     * a \a string.
     * \see setShowNotSetOption()
     */
    void setShowNotSetOption( bool show, const QString &string = QString() );

  public slots:

    /**
     * Sets the raster \a layer for which the bands are listed in the combobox. If no layer is set
     * or a non-raster layer is set then the combobox will be empty.
     * \see layer()
     */
    void setLayer( QgsMapLayer *layer );

    /**
     * Sets the current \a band number selected in the combobox.
     * \see band()
     */
    void setBand( int band );

  signals:

    /**
     * Emitted when the currently selected band changes.
     */
    void bandChanged( int band );

  private:

    QPointer< QgsRasterLayer > mLayer;

    bool mShowNotSet = false;
    QString mNotSetString;
    int mPrevBand = -1;

    QString displayBandName( QgsRasterDataProvider *provider, int band ) const;


};

#endif // QGSRASTERBANDCOMBOBOX_H
