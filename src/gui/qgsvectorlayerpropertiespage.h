/***************************************************************************
    qgsvectorlayerpropertiespage.h
     --------------------------------------
    Date                 : 8.7.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORLAYERPROPERTIESPAGE_H
#define QGSVECTORLAYERPROPERTIESPAGE_H

#include <QWidget>

class QgsVectorLayer;

/** \ingroup gui
 * \class QgsVectorLayerPropertiesPage
 * \note added in QGIS 2.16
 * Base class for custom map layer property pages
 */
class GUI_EXPORT QgsMapLayerPropertiesPage : public QWidget
{
    Q_OBJECT
  public:

    /** Constructor for QgsMapLayerPropertiesPage.
     * @param parent parent widget
    */
    explicit QgsMapLayerPropertiesPage( QWidget *parent = nullptr );

  public slots:
    /** Apply changes */
    virtual void apply() = 0;
};

#endif // QGSVECTORLAYERPROPERTIESPAGE_H
