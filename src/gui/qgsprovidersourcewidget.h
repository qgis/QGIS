/***************************************************************************
                             qgsprovidersourcewidget.h
                             ---------------
    begin                : December 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPROVIDERSOURCEWIDGET_H
#define QGSPROVIDERSOURCEWIDGET_H

#include <QWidget>
#include "qgis_gui.h"
#include "qgis_sip.h"

class QgsMapCanvas;

/**
 * Base class for widgets which allow customization of a provider's source URI.
 *
 * \ingroup gui
 * \since QGIS 3.18
 */
class GUI_EXPORT QgsProviderSourceWidget : public QWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProviderSourceWidget with the specified \a parent widget.
     */
    QgsProviderSourceWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the source \a uri to show in the widget.
     *
     * \see sourceUri()
     */
    virtual void setSourceUri( const QString &uri ) = 0;

    /**
     * Returns the source URI as currently defined by the widget.
     *
     * \see setSourceUri()
     */
    virtual QString sourceUri() const = 0;

    /**
     * Sets a map \a canvas associated with the widget.
     *
     * \since QGIS 3.26
     */
    virtual void setMapCanvas( QgsMapCanvas *mapCanvas ) { mMapCanvas = mapCanvas; }

    /**
     * Returns the map canvas associated with the widget.
     *
     * \since QGIS 3.26
     */
    virtual QgsMapCanvas *mapCanvas() {return mMapCanvas; }

  signals:

    /**
     * Emitted whenever the validation status of the widget changes.
     *
     * If \a isValid is FALSE then the widget is not valid, and any dialog using the widget should be prevented from
     * being accepted.
     */
    void validChanged( bool isValid );

  private:
    QgsMapCanvas *mMapCanvas = nullptr;

};
#endif //QGSPROVIDERSOURCEWIDGET_H
