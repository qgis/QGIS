/***************************************************************************
    qgsannotationpictureitem.h
    ----------------
    begin                : July 2024
    copyright            : (C) 2024 by Nyall Dawson
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

#ifndef QGSANNOTATIONPICTUREITEM_H
#define QGSANNOTATIONPICTUREITEM_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsannotationrectitem.h"

/**
 * \ingroup core
 * \brief An annotation item which renders a picture.
 *
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsAnnotationPictureItem : public QgsAnnotationRectItem
{
  public:

    /**
     * Constructor for QgsAnnotationPictureItem, rendering the specified image \a path
     * within the specified \a bounds geometry.
     */
    QgsAnnotationPictureItem( Qgis::PictureFormat format, const QString &path, const QgsRectangle &bounds );
    ~QgsAnnotationPictureItem() override;

    QString type() const override;
    bool writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const override;
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    QgsAnnotationPictureItem *clone() const override SIP_FACTORY;

    /**
     * Creates a new polygon annotation item.
     */
    static QgsAnnotationPictureItem *create() SIP_FACTORY;

    /**
     * Returns the path of the image used to render the item.
     *
     * \see setPath()
     */
    QString path() const { return mPath; }

    /**
     * Returns the picture format.
     */
    Qgis::PictureFormat format() const { return mFormat; }

    /**
     * Sets the \a format and \a path of the image used to render the item.
     *
     * \see path()
     * \see format()
     */
    void setPath( Qgis::PictureFormat format, const QString &path );

    /**
     * Returns TRUE if the aspect ratio of the picture will be retained.
     *
     * \see setLockAspectRatio()
     */
    bool lockAspectRatio() const;

    /**
     * Sets whether the aspect ratio of the picture will be retained.
     *
     * \see lockAspectRatio()
     */
    void setLockAspectRatio( bool locked );

  protected:

    void renderInBounds( QgsRenderContext &context, const QRectF &painterBounds, QgsFeedback *feedback ) override;

  private:

    QString mPath;
    Qgis::PictureFormat mFormat = Qgis::PictureFormat::Unknown;
    bool mLockAspectRatio = true;

#ifdef SIP_RUN
    QgsAnnotationPictureItem( const QgsAnnotationPictureItem &other );
#endif

};
#endif // QGSANNOTATIONPICTUREITEM_H
