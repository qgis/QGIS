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

#include "qgis_core.h"
#include "qgscomposeritem.h"
#include <QFile>
#include <QImage>
#include <QSvgRenderer>

class QgsComposerMap;
class QgsExpression;

/**
 * \ingroup core
 * A composer class that displays svg files or raster format (jpg, png, ...)
 * */
class CORE_EXPORT QgsComposerPicture: public QgsComposerItem
{
    Q_OBJECT
  public:

    /**
     * Controls how pictures are scaled within the item's frame
     */
    enum ResizeMode
    {
      Zoom, //!< Enlarges image to fit frame while maintaining aspect ratio of picture
      Stretch, //!< Stretches image to fit frame, ignores aspect ratio
      Clip, //!< Draws image at original size and clips any portion which falls outside frame
      ZoomResizeFrame, //!< Enlarges image to fit frame, then resizes frame to fit resultant image
      FrameToImageSize //!< Sets size of frame to match original size of image without scaling
    };

    /**
     * Format of source image
     */
    enum Mode
    {
      SVG,
      RASTER,
      Unknown
    };

    //! Method for syncing rotation to a map's North direction
    enum NorthMode
    {
      GridNorth = 0, //!< Align to grid north
      TrueNorth, //!< Align to true north
    };

    QgsComposerPicture( QgsComposition *composition SIP_TRANSFERTHIS );

    //! Return correct graphics item type.
    virtual int type() const override { return ComposerPicture; }

    //! Reimplementation of QCanvasItem::paint
    void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget ) override;

    /**
     * Sets the source path of the image (may be svg or a raster format). Data defined
     * picture source may override this value. The path can either be a local path
     * or a remote (http) path.
     * \param path path for the source image
     * \see usePictureExpression
     * \see picturePath
     * \since QGIS 2.5
     */
    void setPicturePath( const QString &path );

    /**
     * Returns the path of the source image. Data defined picture source may override
     * this value. The path can either be a local path or a remote (http) path.
     * \returns path for the source image
     * \see usePictureExpression
     * \see setPicturePath
     * \since QGIS 2.5
     */
    QString picturePath() const;

    /**
     * Sets this items bound in scene coordinates such that 1 item size units
     * corresponds to 1 scene size unit and resizes the svg symbol / image
     */
    void setSceneRect( const QRectF &rectangle ) override;

    /**
     * Stores state in Dom element
     * \param elem is Dom element corresponding to 'Composer' tag
     * \param doc is Dom document
     */
    bool writeXml( QDomElement &elem, QDomDocument &doc ) const override;

    /**
     * Sets state from Dom document
     * \param itemElem is Dom node corresponding to item tag
     * \param doc is Dom document
     */
    bool readXml( const QDomElement &itemElem, const QDomDocument &doc ) override;

    /**
     * Returns the rotation used for drawing the picture within the item's frame,
     * in degrees clockwise.
     * \since QGIS 2.2
     * \see setPictureRotation()
     * \see rotationMap()
     */
    double pictureRotation() const { return mPictureRotation; }

    /**
     * Sets the map object for rotation (by id). A value of -1 disables the map
     * rotation.  If this is set then the picture will be rotated by the same
     * amount as the specified map object. This is useful especially for
     * syncing north arrows with a map item.
     * \param composerMapId composer map id to sync rotation with
     * \see setPictureRotation
     * \see rotationMap
     */
    void setRotationMap( int composerMapId );

    /**
     * Returns the id of the rotation map.  A value of -1 means map rotation is
     * disabled.  If this is set then the picture is rotated by the same amount
     * as the specified map object.
     * \returns id of map object
     * \see setRotationMap
     * \see useRotationMap
     */
    int rotationMap() const;

    /**
     * True if the picture rotation is matched to a map item.
     * \returns true if rotation map is in use
     * \see rotationMap
     * \see setRotationMap
     */
    bool useRotationMap() const { return mRotationMap; }

    /**
     * Returns the mode used to align the picture to a map's North.
     * \see setNorthMode()
     * \see northOffset()
     * \since QGIS 2.18
     */
    NorthMode northMode() const { return mNorthMode; }

    /**
     * Sets the mode used to align the picture to a map's North.
     * \see northMode()
     * \see setNorthOffset()
     * \since QGIS 2.18
     */
    void setNorthMode( NorthMode mode );

    /**
     * Returns the offset added to the picture's rotation from a map's North.
     * \see setNorthOffset()
     * \see northMode()
     * \since QGIS 2.18
     */
    double northOffset() const { return mNorthOffset; }

    /**
     * Sets the offset added to the picture's rotation from a map's North.
     * \see northOffset()
     * \see setNorthMode()
     * \since QGIS 2.18
     */
    void setNorthOffset( double offset );

    /**
     * Returns the resize mode used for drawing the picture within the composer
     * item's frame.
     * \returns resize mode of picture
     * \since QGIS 2.3
     * \see setResizeMode
     */
    ResizeMode resizeMode() const { return mResizeMode; }

    /**
     * Sets the picture's anchor point, which controls how it is placed
     * within the picture item's frame.
     * \param anchor anchor point for picture
     * \since QGIS 2.3
     * \see pictureAnchor
     */
    void setPictureAnchor( QgsComposerItem::ItemPositionMode anchor );

    /**
     * Returns the picture's current anchor, which controls how it is placed
     * within the picture item's frame.
     * \returns anchor point for picture
     * \since QGIS 2.3
     * \see setPictureAnchor
     */
    ItemPositionMode pictureAnchor() const { return mPictureAnchor; }

    /**
     * Returns the fill color used for parametrized SVG files.
     * \see setSvgFillColor()
     * \see svgStrokeColor()
     * \since QGIS 2.14.1
     */
    QColor svgFillColor() const { return mSvgFillColor; }

    /**
     * Sets the fill color used for parametrized SVG files.
     * \param color fill color.
     * \note this setting only has an effect on parametrized SVG files, and is ignored for
     * non-parametrized SVG files.
     * \see svgFillColor()
     * \see setSvgStrokeColor()
     * \since QGIS 2.14.1
     */
    void setSvgFillColor( const QColor &color );

    /**
     * Returns the stroke color used for parametrized SVG files.
     * \see setSvgStrokeColor()
     * \see svgFillColor()
     * \since QGIS 2.14.1
     */
    QColor svgStrokeColor() const { return mSvgStrokeColor; }

    /**
     * Sets the stroke color used for parametrized SVG files.
     * \param color stroke color.
     * \note this setting only has an effect on parametrized SVG files, and is ignored for
     * non-parametrized SVG files.
     * \see svgStrokelColor()
     * \see setSvgFillColor()
     * \since QGIS 2.14.1
     */
    void setSvgStrokeColor( const QColor &color );

    /**
     * Returns the stroke width (in mm) used for parametrized SVG files.
     * \see setSvgStrokeWidth()
     * \see svgStrokeColor()
     * \since QGIS 2.14.1
     */
    double svgStrokeWidth() const { return mSvgStrokeWidth; }

    /**
     * Sets the stroke width used for parametrized SVG files.
     * \param width stroke width in mm
     * \note this setting only has an effect on parametrized SVG files, and is ignored for
     * non-parametrized SVG files.
     * \see svgStrokeWidth()
     * \see setSvgStrokeColor()
     * \since QGIS 2.14.1
     */
    void setSvgStrokeWidth( double width );

    /**
     * Returns the current picture mode (image format).
     * \returns picture mode
     * \since QGIS 2.3
     */
    Mode mode() const { return mMode; }

  public slots:

    /**
     * Sets the picture \a rotation within the item bounds, in degrees clockwise. This does not affect
     * the item's frame, only the way the picture is drawn within the item.
     * \see pictureRotation()
     * \since QGIS 2.2
     */
    virtual void setPictureRotation( double rotation );

    /**
     * Sets the resize mode used for drawing the picture within the item bounds.
     * \param mode ResizeMode to use for image file
     * \since QGIS 2.3
     * \see resizeMode
     */
    virtual void setResizeMode( ResizeMode mode );

    /**
     * Recalculates the source image (if using an expression for picture's source)
     * and reloads and redraws the picture.
     * \param context expression context for evaluating data defined picture sources
     * \since QGIS 2.3
     */
    void refreshPicture( const QgsExpressionContext *context = nullptr );

    /**
     * Forces a recalculation of the picture's frame size
     * \since QGIS 2.3
     */
    void recalculateSize();

    virtual void refreshDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property = QgsComposerObject::AllProperties, const QgsExpressionContext *context = nullptr ) override;

  signals:
    //! Is emitted on picture rotation change
    void pictureRotationChanged( double newRotation );

  private:

    //default constructor is forbidden
    QgsComposerPicture();
    //! Calculates bounding rect for svg file (mSourcefile) such that aspect ratio is correct
    QRectF boundedSVGRect( double deviceWidth, double deviceHeight );
    //! Calculates bounding rect for image such that aspect ratio is correct
    QRectF boundedImageRect( double deviceWidth, double deviceHeight );

    //! Returns size of current raster or svg picture
    QSizeF pictureSize();

    QImage mImage;
    QSvgRenderer mSVG;
    //! Absolute path to the image (may be also HTTP URL)
    QString mSourcePath;
    Mode mMode = Unknown;

    QSize mDefaultSvgSize;

    //! Image rotation
    double mPictureRotation = 0;
    //! Map that sets the rotation (or 0 if this picture uses map independent rotation)
    const QgsComposerMap *mRotationMap = nullptr;

    //! Mode used to align to North
    NorthMode mNorthMode = GridNorth;
    //! Offset for north arrow
    double mNorthOffset = 0.0;

    //! Width of the picture (in mm)
    double mPictureWidth;
    //! Height of the picture (in mm)
    double mPictureHeight;

    ResizeMode mResizeMode = QgsComposerPicture::Zoom;
    QgsComposerItem::ItemPositionMode mPictureAnchor = UpperLeft;

    QColor mSvgFillColor = QColor( 255, 255, 255 );
    QColor mSvgStrokeColor = QColor( 0, 0, 0 );
    double mSvgStrokeWidth = 0.2;

    bool mHasExpressionError = false;
    bool mLoaded;
    bool mLoadingSvg = false;

    //! Loads an image file into the picture item and redraws the item
    void loadPicture( const QString &path );

    //! Sets up the picture item and connects to relevant signals
    void init();

    /**
     * Returns part of a raster image which will be shown, given current picture
     * anchor settings
     */
    QRect clippedImageRect( double &boundRectWidthMM, double &boundRectHeightMM, QSize imageRectPixels );

    /**
     * Loads a remote picture for the item
     */
    void loadRemotePicture( const QString &url );

    /**
     * Loads a local picture for the item
     */
    void loadLocalPicture( const QString &path );

  private slots:

    void remotePictureLoaded();

    void updateMapRotation();
};

#endif
