/***************************************************************************
    qgsnetworkloggerwidgetfactory.h
    -------------------------
    begin                : March 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSNETWORKLOGGERWIDGETFACTORY_H
#define QGSNETWORKLOGGERWIDGETFACTORY_H

#define SIP_NO_FILE

#include "qgis_gui.h"
#include "qgsdevtoolwidgetfactory.h"

class QgsNetworkLogger;

/**
 * \ingroup gui
 * \class QgsNetworkLoggerWidgetFactory
 * \brief Factory class for creating network logger debugging page.
 * \since QGIS 4.0
 */
class GUI_EXPORT QgsNetworkLoggerWidgetFactory : public QgsDevToolWidgetFactory
{
  public:
    /**
     * Constructor for a QgsNetworkLoggerWidgetFactory.
     */
    QgsNetworkLoggerWidgetFactory( QgsNetworkLogger *logger );

    /**
     * Factory function to create the widget on demand as needed by the dock.
     *
     * The \a parent argument gives the correct parent for the newly created widget.
     */
    QgsDevToolWidget *createWidget( QWidget *parent = nullptr ) const override;

  private:
    QgsNetworkLogger *mLogger = nullptr;
};


#endif // QGSNETWORKLOGGERWIDGETFACTORY_H
