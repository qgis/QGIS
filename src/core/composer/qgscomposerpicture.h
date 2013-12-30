/***************************************************************************
                         qgscomposerpicture.h
                             -------------------
    begin                : September 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOMPOSERPICTURE_H
#define QGSCOMPOSERPICTURE_H

#include "qgscomposeritem.h"
#include <QFile>
#include <QImage>
#include <QSvgRenderer>

class QgsComposerMap;

/** \ingroup MapComposer
 * A composer class that displays svg files or raster format (jpg, png, ...)
 * */
class CORE_EXPORT QgsComposerPicture: public QgsComposerItem
{
    Q_OBJECT
  public:
    QgsComposerPicture( QgsComposition *composition );
    ~QgsComposerPicture();

    /** return correct graphics item type. Added in v1.7 */
    virtual int type() const { return ComposerPicture; }

    /**Reimplementation of QCanvasItem::paint*/
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    /**Sets the source file of the image (may be svg or a raster format)*/
    void setPictureFile( const QString& path );
    QString pictureFile() const;

    /**Sets this items bound in scene coordinates such that 1 item size units
       corresponds to 1 scene size unit and resizes the svg symbol / image*/
    void setSceneRect( const QRectF& rectangle );

    /** stores state in Dom element
       * @param elem is Dom element corresponding to 'Composer' tag
       * @param doc is Dom document
       */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** sets state from Dom document
      * @param itemElem is Dom node corresponding to item tag
      * @param doc is Dom document
      */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    /**Returns the rotation used for drawing the picture within the composer item
     * @deprecated Use pictureRotation() instead
     */
    double rotation() const { return mPictureRotation;};

    /**Returns the rotation used for drawing the picture within the item
      @note this function was added in version 2.1*/
    double pictureRotation() const { return mPictureRotation;};

    /**Sets the map object for rotation (by id). A value of -1 disables the map rotation*/
    void setRotationMap( int composerMapId );
    /**Returns the id of the rotation map*/
    int rotationMap() const;
    /**True if the rotation is taken from a map item*/
    bool useRotationMap() const {return mRotationMap;}

    /**Calculates width and hight of the picture (in mm) such that it fits into the item frame with the given rotation
     * @deprecated Use bool QgsComposerItem::imageSizeConsideringRotation( double& width, double& height, double rotation )
     * instead
     */
    bool imageSizeConsideringRotation( double& width, double& height ) const;
    /**Calculates corner point after rotation and scaling
     * @deprecated Use QgsComposerItem::cornerPointOnRotatedAndScaledRect( double& x, double& y, double width, double height, double rotation )
     * instead
     */
    bool cornerPointOnRotatedAndScaledRect( double& x, double& y, double width, double height ) const;
    /**Calculates width / height of the bounding box of a rotated rectangle
    * @deprecated Use QgsComposerItem::sizeChangedByRotation( double& width, double& height, double rotation )
    * instead
    */
    void sizeChangedByRotation( double& width, double& height );

  public slots:
    /**Sets the picture rotation within the item bounds. This does not affect the item rectangle,
      only the way the picture is drawn within the item.
     * @deprecated Use setPictureRotation( double rotation ) instead
     */
    virtual void setRotation( double r );

    /**Sets the picture rotation within the item bounds. This does not affect the item rectangle,
      only the way the picture is drawn within the item.
      @note this function was added in version 2.1*/
    virtual void setPictureRotation( double r );

  signals:
    /**Is emitted on picture rotation change*/
    void pictureRotationChanged( double newRotation );

  private:

    enum Mode //SVG or raster graphic format
    {
      SVG,
      RASTER,
      Unknown
    };

    //default constructor is forbidden
    QgsComposerPicture();
    /**Calculates bounding rect for svg file (mSourcefile) such that aspect ratio is correct*/
    QRectF boundedSVGRect( double deviceWidth, double deviceHeight );
    /**Calculates bounding rect for image such that aspect ratio is correct*/
    QRectF boundedImageRect( double deviceWidth, double deviceHeight );

    /**Returns size of current raster or svg picture */
    QSizeF pictureSize();

    QImage mImage;
    QSvgRenderer mSVG;
    QFile mSourceFile;
    Mode mMode;

    QSize mDefaultSvgSize;

    /**Image rotation*/
    double mPictureRotation;
    /**Map that sets the rotation (or 0 if this picture uses map independent rotation)*/
    const QgsComposerMap* mRotationMap;
    /**Width of the picture (in mm)*/
    double mPictureWidth;
    /**Height of the picture (in mm)*/
    double mPictureHeight;
};

#endif
