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

/** \ingroup MapComposer
 * A composer class that displays svg files or raster format (jpg, png, ...)
 * */
class CORE_EXPORT QgsComposerPicture: public QgsComposerItem
{
    Q_OBJECT
  public:
    QgsComposerPicture( QgsComposition *composition );
    ~QgsComposerPicture();

    /**Reimplementation of QCanvasItem::paint*/
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    /**Sets the source file of the image (may be svg or a raster format)*/
    void setPictureFile( const QString& path );
    QString pictureFile() const;

    /**Sets this items bound in scene coordinates such that 1 item size units
       corresponds to 1 scene size unit and resizes the svg symbol / image*/
    void setSceneRect( const QRectF& rectangle );

    /** stores state in Dom node
       * @param node is Dom node corresponding to 'Composer' tag
       * @param temp write template file
       */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** sets state from Dom document
      * @param itemElem is Dom node corresponding to item tag
      */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    /**Sets the map object for rotation (by id). A value of -1 disables the map rotation*/
    void setRotationMap( int composerMapId );
    /**Returns the id of the rotation map*/
    int rotationMap() const;
    /**True if the rotation is taken from a map item*/
    bool useRotationMap() const {return mRotationMap;}

  public slots:
    /**Sets the rotation and adapts the item rect*/
    virtual void setRotation( double r );

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

    /**Updates content of mImage using svg generator
    @param out: boundWidth width of mImage that is used by the svg renderer. May different from mImage.width() to preserve aspect ratio
    @param out: boundHeight height of mImage that is used by the svg renderer*/
    void updateImageFromSvg();


    QImage mImage;
    QFile mSourceFile;
    Mode mMode;
    /**False if image needs to be rendered from svg*/
    bool mSvgCacheUpToDate;
    int mCachedDpi; //store dpis for which the svg cache is valid
    double mCachedRotation; //store last rotation value to generate new pixmap from svg on change
    double mCachedViewScaleFactor;

    QSize mDefaultSvgSize;
    /**Map that sets the rotation (or 0 if this picture uses map independent rotation)*/
    const QgsComposerMap* mRotationMap;
    /**Width of the picture (in mm)*/
    double mPictureWidth;
    /**Height of the picture (in mm)*/
    double mPictureHeight;

  signals:
    /**Tell the configuration widget that the settings need to be updated*/
    void settingsChanged();
};

#endif
