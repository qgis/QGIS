/***************************************************************************
                              qgslayouteffect.h
                             -------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall.dawson@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTEFFECT_H
#define QGSLAYOUTEFFECT_H

#include <QGraphicsEffect>
#include <QPainter>

#include "qgis_core.h"
#include "qgis_sip.h"

/**
 * \ingroup core
 * \class QgsLayoutEffect
 *
 * \brief A QGraphicsEffect subclass used for rendering layout items
 * onto a scene with custom composition modes.
 *
 * \deprecated QGIS 3.34. This class should not be used, it is non-stable and results in crashes. See https://bugreports.qt.io/browse/QTBUG-58501.
 */
class CORE_EXPORT QgsLayoutEffect : public QGraphicsEffect
{
    Q_OBJECT

  public:

    QgsLayoutEffect() = default;

    /**
     * Sets the composition (blending) \a mode used for rendering
     * the item.
     * \see compositionMode()
     *
     * \deprecated QGIS 3.34. This class should not be used, it is non-stable and results in crashes. See https://bugreports.qt.io/browse/QTBUG-58501.
     */
    Q_DECL_DEPRECATED void setCompositionMode( QPainter::CompositionMode mode ) SIP_DEPRECATED;

    /**
     * Returns the composition (blending) mode used for rendering
     * the item.
     * \see setCompositionMode()
     *
     * \deprecated QGIS 3.34. This class should not be used, it is non-stable and results in crashes. See https://bugreports.qt.io/browse/QTBUG-58501.
     */
    Q_DECL_DEPRECATED QPainter::CompositionMode compositionMode() const SIP_DEPRECATED { return mCompositionMode; }

  protected:

    void draw( QPainter *painter ) override;

  private:

    QPainter::CompositionMode mCompositionMode = QPainter::CompositionMode_SourceOver;
};

#endif // QGSLAYOUTEFFECT_H

