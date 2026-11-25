/***************************************************************************
  qgspicture.h
  --------------------------------------
  Date                 : November 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPICTURE_H
#define QGSPICTURE_H

#include <memory>

#include "qgis_core.h"
#include "qgis_sip.h"

#include <QExplicitlySharedDataPointer>
#include <QPaintDevice>
#include <QPointF>
#include <QRect>

class QPainter;
class QgsPicturePaintEngine;
class QgsPicturePrivate;

/**
 * \ingroup core
 * \brief A replacement for QPicture which focuses on optimised in-memory QPainter replays only.
 *
 * In contrast to QPicture, the QgsPicture class offers:
 *
 * - proper thread safety, and no crashes when replaying shared picture copies
 * - images are always stored as lossless QImage versions (as opposed to PNG encoded versions of images, which are costly to record and replay)
 * - the focus is on optimised recording and replay, as opposed to QPicture's focus on writing and replaying from a stable binary format
 *
 * \since QGIS 4.4.1
 */
class CORE_EXPORT QgsPicture : public QPaintDevice
{
    Q_DECLARE_PRIVATE( QgsPicture )

  public:
    /**
     * Constructor for an empty picture.
     */
    explicit QgsPicture();
    ~QgsPicture() override;

    QgsPicture( const QgsPicture &other );
    QgsPicture &operator=( const QgsPicture &other );
    //! Move constructor
    QgsPicture( QgsPicture &&other ) noexcept SIP_SKIP;
    //! Move assignment operator
    QgsPicture &operator=( QgsPicture &&other ) noexcept SIP_SKIP;

    int devType() const override;
    QPaintEngine *paintEngine() const override;

    /**
     * Returns TRUE if the picture contains no data.
     */
    bool isNull() const;

    /**
     * Replays the picture using \a painter.
     *
     * \returns TRUE if the replay was successful.
     *
     * \note This method is const and thread-safe, so multiple threads can concurrently
     * replay shared copies of the same QgsPicture onto separate painters.
     */
    bool play( QPainter *painter ) const;

    /**
     * Returns the picture's bounding rectangle or an invalid rectangle
     * if the picture contains no data.
     *
     * \see setBoundingRect()
     */
    QRect boundingRect() const;

    /**
     * Sets the picture's bounding rectangle to \a rect.
     *
     * The automatically calculated value is overridden.
     *
     * \see boundingRect()
     */
    void setBoundingRect( const QRect &rect );

    /**
     * Detaches this picture instance from the shared picture data.
     *
     * \see isDetached()
     */
    void detach();

    /**
     * Returns TRUE if this instance does not share its internal data with any other QgsPicture.
     *
     * \see detach()
     */
    bool isDetached() const;

#ifndef SIP_RUN
    typedef QExplicitlySharedDataPointer<QgsPicturePrivate> DataPtr;

    /**
     * Returns the internal shared data pointer.
     */
    DataPtr &data_ptr() { return d_ptr; }
#endif

  protected:
    int metric( PaintDeviceMetric m ) const override;

  private:
    QExplicitlySharedDataPointer<QgsPicturePrivate> d_ptr;
    mutable std::unique_ptr<QPaintEngine> mPaintEngine;

    friend class QgsPicturePaintEngine;
};

#endif // QGSPICTURE_H
