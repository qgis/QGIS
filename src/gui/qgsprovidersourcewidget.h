/***************************************************************************
                             qgsprovidersourcewidget.h
                             ---------------
    begin                : August 2018
    copyright            : (C) 2018 by Nyall Dawson
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

/**
 * Base class for widgets which allow customisation of a provider's source URI.
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

};
#endif //QGSPROVIDERSOURCEWIDGET_H
