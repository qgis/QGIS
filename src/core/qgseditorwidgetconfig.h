/***************************************************************************
    qgseditorwidgetconfig.h
    ---------------------
    begin                : September 2013
    copyright            : (C) 2013 by Matthias Kuhn
    email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QMap>
#include <QString>
#include <QVariant>

#ifndef QGSEDITORWIDGETCONFIG_H
#define QGSEDITORWIDGETCONFIG_H

/** \ingroup core
 * Holds a set of configuration parameters for a editor widget wrapper.
 * It's basically a set of key => value pairs.
 *
 * If you need more advanced structures than a simple key => value pair,
 * you can use a value to hold any structure a QVariant can handle (and that's
 * about anything you get through your compiler)
 *
 * These are the user configurable options in the field properties tab of the
 * vector layer properties. They are saved in the project file per layer and field.
 * You get these passed, for every new widget wrapper.
 */

typedef QVariantMap QgsEditorWidgetConfig;

#endif // QGSEDITORWIDGETCONFIG_H
