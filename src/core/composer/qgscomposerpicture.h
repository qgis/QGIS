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
class QgsExpression;

/** \ingroup core
 * A composer class that displays svg files or raster format (jpg, png, ...)
 * */
class CORE_EXPORT QgsComposerPicture: public QgsComposerItem
{
    Q_OBJECT
  public:

    /** Controls how pictures are scaled within the item's frame
     */
    enum ResizeMode
    {
      Zoom, /*!< enlarges image to fit frame while maintaining aspect ratio of picture */
      Stretch, /*!< stretches image to fit frame, ignores aspect ratio */
      Clip, /*!< draws image at original size and clips any portion which falls outside frame */
      ZoomResizeFrame, /*!< enlarges image to fit frame, then resizes frame to fit resultant image */
      FrameToImageSize /*!< sets size of frame to match original size of image without scaling */
    };

    /** Format of source image
     */
    enum Mode
    {
      SVG,
      RASTER,
      Unknown
    };

    QgsComposerPicture( QgsComposition *composition );
    ~QgsComposerPicture();

    /** Return correct graphics item type. */
    virtual int type() const override { return ComposerPicture; }

    /** Reimplementation of QCanvasItem::paint*/
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget ) override;

    /** Sets the source path of the image (may be svg or a raster format). Data defined
     * picture source may override this value. The path can either be a local path
     * or a remote (http) path.
     * @param path path for the source image
     * @see usePictureExpression
     * @see picturePath
     * @note added in QGIS 2.5
     */
    void setPicturePath( const QString& path );

    /** Returns the path of the source image. Data defined picture source may override
     * this value. The path can either be a local path or a remote (http) path.
     * @returns path for the source image
     * @see usePictureExpression
     * @see setPicturePath
     * @note added in QGIS 2.5
     */
    QString picturePath() const;

    /** Sets this items bound in scene coordinates such that 1 item size units
     * corresponds to 1 scene size unit and resizes the svg symbol / image
     */
    void setSceneRect( const QRectF& rectangle ) override;

    /** Stores state in Dom element
     * @param elem is Dom element corresponding to 'Composer' tag
     * @param doc is Dom document
     */
    bool writeXml( QDomElement& elem, QDomDocument & doc ) const override;

    /** Sets state from Dom document
     * @param itemElem is Dom node corresponding to item tag
     * @param doc is Dom document
     */
    bool readXml( const QDomElement& itemElem, const QDomDocument& doc ) override;

    /** Returns the rotation used for drawing the picture within the item's frame
     * @returns picture rotation in degrees
     * @note added in 2.2
     * @see setPictureRotation
     * @see rotationMap
     */
    double pictureRotation() const { return mPictureRotation; }

    /** Sets the map object for rotation (by id). A value of -1 disables the map
     * rotation.  If this is set then the picture will be rotated by the same
     * amount as the specified map object. This is useful especially for
     * syncing north arrows with a map item.
     * @param composerMapId composer map id to sync rotation with
     * @see setPictureRotation
     * @see rotationMap
     */
    void setRotationMap( int composerMapId );

    /** Returns the id of the rotation map.  A value of -1 means map rotation is
     * disabled.  If this is set then the picture is rotated by the same amount
     * as the specified map object.
     * @returns id of map object
     * @see setRotationMap
     * @see useRotationMap
     */
    int rotationMap() const;

    /** True if the picture rotation is matched to a map item.
     * @returns true if rotation map is in use
     * @see rotationMap
     * @see setRotationMap
     */
    bool useRotationMap() const { return mRotationMap; }

    /** Returns the resize mode used for drawing the picture within the composer
     * item's frame.
     * @returns resize mode of picture
     * @note added in 2.3
     * @see setResizeMode
     */
    ResizeMode resizeMode() const { return mResizeMode; }

    /** Sets the picture's anchor point, which controls how it is placed
     * within the picture item's frame.
     * @param anchor anchor point for picture
     * @note added in 2.3
     * @see pictureAnchor
     */
    void setPictureAnchor( QgsComposerItem::ItemPositionMode anchor );

    /** Returns the picture's current anchor, which controls how it is placed
     * within the picture item's frame.
     * @returns anchor point for picture
     * @note added in 2.3
     * @see setPictureAnchor
     */
    ItemPositionMode pictureAnchor() const { return mPictureAnchor; }

    /** Returns the fill color used for parameterized SVG files.
     * @see setSvgFillColor()
     * @see svgBorderColor()
     * @note added in QGIS 2.14.1
     */
    QColor svgFillColor() const { return mSvgFillColor; }

    /** Sets the fill color used for parameterized SVG files.
     * @param color fill color.
     * @note this setting only has an effect on parameterized SVG files, and is ignored for
     * non-parameterized SVG files.
     * @see svgFillColor()
     * @see setSvgBorderColor()
     * @note added in QGIS 2.14.1
     */
    void setSvgFillColor( const QColor& color );

    /** Returns the border color used for parameterized SVG files.
     * @see setSvgBorderColor()
     * @see svgFillColor()
     * @note added in QGIS 2.14.1
     */
    QColor svgBorderColor() const { return mSvgBorderColor; }

    /** Sets the border color used for parameterized SVG files.
     * @param color border color.
     * @note this setting only has an effect on parameterized SVG files, and is ignored for
     * non-parameterized SVG files.
     * @see svgBorderlColor()
     * @see setSvgFillColor()
     * @note added in QGIS 2.14.1
     */
    void setSvgBorderColor( const QColor& color );

    /** Returns the border width (in mm) used for parameterized SVG files.
     * @see setSvgBorderWidth()
     * @see svgBorderColor()
     * @note added in QGIS 2.14.1
     */
    double svgBorderWidth() const { return mSvgBorderWidth; }

    /** Sets the border width used for parameterized SVG files.
     * @param width border width in mm
     * @note this setting only has an effect on parameterized SVG files, and is ignored for
     * non-parameterized SVG files.
     * @see svgBorderWidth()
     * @see setSvgBorderColor()
     * @note added in QGIS 2.14.1
     */
    void setSvgBorderWidth( double width );

    /** Returns the current picture mode (image format).
     * @returns picture mode
     * @note added in 2.3
     */
    Mode mode() const { return mMode; }

  public slots:

    /** Sets the picture rotation within the item bounds. This does not affect
     * the item's frame, only the way the picture is drawn within the item.
     * @param r rotation in degrees clockwise
     * @see pictureRotation
     * @note added in 2.2
     */
    virtual void setPictureRotation( double r );

    /** Sets the resize mode used for drawing the picture within the item bounds.
     * @param mode ResizeMode to use for image file
     * @note added in 2.3
     * @see resizeMode
     */
    virtual void setResizeMode( ResizeMode mode );

    /** Recalculates the source image (if using an expression for picture's source)
     * and reloads and redraws the picture.
     * @param context expression context for evaluating data defined picture sources
     * @note added in 2.3
     */
    void refreshPicture( const QgsExpressionContext* context = nullptr );

    /** Forces a recalculation of the picture's frame size
     * @note added in 2.3
     */
    void recalculateSize();

    virtual void refreshDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property = QgsComposerObject::AllProperties, const QgsExpressionContext *context = nullptr ) override;

  signals:
    /** Is emitted on picture rotation change*/
    void pictureRotationChanged( double newRotation );

  private:

    //default constructor is forbidden
    QgsComposerPicture();
    /** Calculates bounding rect for svg file (mSourcefile) such that aspect ratio is correct*/
    QRectF boundedSVGRect( double deviceWidth, double deviceHeight );
    /** Calculates bounding rect for image such that aspect ratio is correct*/
    QRectF boundedImageRect( double deviceWidth, double deviceHeight );

    /** Returns size of current raster or svg picture */
    QSizeF pictureSize();

    QImage mImage;
    QSvgRenderer mSVG;
    QString mSourcePath;
    Mode mMode;

    QSize mDefaultSvgSize;

    /** Image rotation*/
    double mPictureRotation;
    /** Map that sets the rotation (or 0 if this picture uses map independent rotation)*/
    const QgsComposerMap* mRotationMap;
    /** Width of the picture (in mm)*/
    double mPictureWidth;
    /** Height of the picture (in mm)*/
    double mPictureHeight;

    ResizeMode mResizeMode;
    QgsComposerItem::ItemPositionMode mPictureAnchor;

    QColor mSvgFillColor;
    QColor mSvgBorderColor;
    double mSvgBorderWidth;

    bool mHasExpressionError;
    bool mLoaded;
    bool mLoadingSvg;

    /** Loads an image file into the picture item and redraws the item*/
    void loadPicture( const QString &path );

    /** Sets up the picture item and connects to relevant signals*/
    void init();

    /** Returns part of a raster image which will be shown, given current picture
     * anchor settings
     */
    QRect clippedImageRect( double &boundRectWidthMM, double &boundRectHeightMM, QSize imageRectPixels );

    /** Loads a remote picture for the item
     */
    void loadRemotePicture( const QString &url );

    /** Loads a local picture for the item
     */
    void loadLocalPicture( const QString &path );

  private slots:

    void remotePictureLoaded();
};

#endif
