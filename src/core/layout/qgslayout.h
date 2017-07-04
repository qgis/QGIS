/***************************************************************************
                              qgslayout.h
                             -------------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
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
#ifndef QGSLAYOUT_H
#define QGSLAYOUT_H

#include "qgis_core.h"
#include <QGraphicsScene>

/**
 * \ingroup core
 * \class QgsLayout
 * \brief Base class for layouts, which can contain items such as maps, labels, scalebars, etc.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayout : public QGraphicsScene
{
    Q_OBJECT

  public:

    //! Preset item z-values, to ensure correct stacking
    enum ZValues
    {
      ZMapTool = 10000, //!< Z-Value for temporary map tool items
    };

    QgsLayout();

};

#endif //QGSLAYOUT_H



