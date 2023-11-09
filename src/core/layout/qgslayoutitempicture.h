/***************************************************************************
                         qgslayoutitempicture.h
                         -------------------
begin                : October 2017
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
#ifndef QGSLAYOUTITEMPICTURE_H
#define QGSLAYOUTITEMPICTURE_H

#include "qgis_core.h"
#include "qgslayoutitem.h"
#include <QFile>
#include <QImage>
#include <QSvgRenderer>

class QgsLayoutItemMap;
class QgsExpression;
class QgsLayoutNorthArrowHandler;

/**
 * \ingroup core
 * \brief A layout item subclass that displays SVG files or raster format images (jpg, png, ...).
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutItemPicture: public QgsLayoutItem
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
    enum Format
    {
      FormatSVG, //!< SVG image
      FormatRaster, //!< Raster image
      FormatUnknown, //!< Invalid or unknown image type
    };

    //! Method for syncing rotation to a map's North direction
    enum NorthMode
    {
      GridNorth = 0, //!< Align to grid north
      TrueNorth, //!< Align to true north
    };

    /**
     * Constructor for QgsLayoutItemPicture, with the specified parent \a layout.
     */
    QgsLayoutItemPicture( QgsLayout *layout );

    int type() const override;
    QIcon icon() const override;

    /**
     * Returns a new picture item for the specified \a layout.
     *
     * The caller takes responsibility for deleting the returned object.
     */
    static QgsLayoutItemPicture *create( QgsLayout *layout ) SIP_FACTORY;


    /**
     * Sets the source \a path of the image (may be svg or a raster format). Data defined
     * picture source may override this value. The path can either be a local path
     * or a remote (http) path.
     *
     * Ideally, the \a format argument should specify the image format.
     *
     * \see picturePath()
     */
    void setPicturePath( const QString &path, Format format = FormatUnknown );

    /**
     * Returns the path of the source image. Data defined picture source may override
     * this value. The path can either be a local path or a remote (http) path.
     * \returns path for the source image
     * \see setPicturePath()
     * \see evaluatedPath()
     */
    QString picturePath() const;

    /**
     * Returns the rotation used for drawing the picture within the item's frame,
     * in degrees clockwise.
     * \see setPictureRotation()
     * \see linkedMap()
     */
    double pictureRotation() const { return mPictureRotation; }

    /**
     * Sets the \a map object for rotation.
     *
     * If this is set then the picture will be rotated by the same
     * amount as the specified map object. This is useful especially for
     * syncing north arrows with a map item.
     *
     * \see setPictureRotation()
     * \see linkedMap()
     */
    void setLinkedMap( QgsLayoutItemMap *map );

    /**
     * Returns the linked rotation map, if set. An NULLPTR means map rotation is
     * disabled.  If this is set then the picture is rotated by the same amount
     * as the specified map object.
     * \see setLinkedMap()
     */
    QgsLayoutItemMap *linkedMap() const;

    /**
     * Returns the mode used to align the picture to a map's North.
     * \see setNorthMode()
     * \see northOffset()
     */
    NorthMode northMode() const;

    /**
     * Sets the \a mode used to align the picture to a map's North.
     * \see northMode()
     * \see setNorthOffset()
     */
    void setNorthMode( NorthMode mode );

    /**
     * Returns the offset added to the picture's rotation from a map's North.
     * \see setNorthOffset()
     * \see northMode()
     */
    double northOffset() const;

    /**
     * Sets the \a offset added to the picture's rotation from a map's North.
     * \see northOffset()
     * \see setNorthMode()
     */
    void setNorthOffset( double offset );

    /**
     * Returns the resize mode used for drawing the picture within the layout
     * item's frame.
     * \see setResizeMode()
     */
    ResizeMode resizeMode() const { return mResizeMode; }

    /**
     * Sets the picture's \a anchor point, which controls how it is placed
     * within the picture item's frame.
     * \see pictureAnchor()
     */
    void setPictureAnchor( QgsLayoutItem::ReferencePoint anchor );

    /**
     * Returns the picture's current anchor, which controls how it is placed
     * within the picture item's frame.
     * \see setPictureAnchor()
     */
    QgsLayoutItem::ReferencePoint pictureAnchor() const { return mPictureAnchor; }

    /**
     * Returns the fill color used for parametrized SVG files.
     * \see setSvgFillColor()
     * \see svgStrokeColor()
     */
    QColor svgFillColor() const { return mSvgFillColor; }

    /**
     * Sets the fill \a color used for parametrized SVG files.
     * \note This setting only has an effect on parametrized SVG files, and is ignored for
     * non-parametrized SVG files.
     * \see svgFillColor()
     * \see setSvgStrokeColor()
     */
    void setSvgFillColor( const QColor &color );

    /**
     * Returns the stroke color used for parametrized SVG files.
     * \see setSvgStrokeColor()
     * \see svgFillColor()
     */
    QColor svgStrokeColor() const { return mSvgStrokeColor; }

    /**
     * Sets the stroke \a color used for parametrized SVG files.
     * \param color stroke color.
     * \note This setting only has an effect on parametrized SVG files, and is ignored for
     * non-parametrized SVG files.
     * \see svgStrokeColor()
     * \see setSvgFillColor()
     */
    void setSvgStrokeColor( const QColor &color );

    /**
     * Returns the stroke width (in layout units) used for parametrized SVG files.
     * \see setSvgStrokeWidth()
     * \see svgStrokeColor()
     */
    double svgStrokeWidth() const { return mSvgStrokeWidth; }

    /**
     * Sets the stroke \a width (in layout units) used for parametrized SVG files.
     * \note This setting only has an effect on parametrized SVG files, and is ignored for
     * non-parametrized SVG files.
     * \see svgStrokeWidth()
     * \see setSvgStrokeColor()
     */
    void setSvgStrokeWidth( double width );

    /**
     * Returns the current picture mode (image format), FormatUnknown if given
     * picture format is unknown
     * \see setMode() originalMode()
     */
    Format mode() const { return mMode; }

    /**
     * Returns the original set picture mode (image format).
     * It could differ from mode() if given picture format is unknown
     * \see setMode() mode()
     * \since 3.22
     */
    Format originalMode() const { return mOriginalMode; }

    /**
     * Sets the current picture \a mode (image format).
     * \see mode()
     * \since QGIS 3.14
     */
    void setMode( Format mode );

    void finalizeRestoreFromXml() override;

    /**
     * Returns TRUE if the source image is missing and the picture
     * cannot be rendered.
     *
     * \since QGIS 3.6
     */
    bool isMissingImage() const;

    /**
     * Returns the current evaluated picture path, which includes
     * the result of data defined path overrides.
     *
     * \see picturePath()
     * \since QGIS 3.6
     */
    QString evaluatedPath() const;

    /**
     * Returns the SVG dynamic parameters
     * \since QGIS 3.20
     */
    QMap<QString, QgsProperty> svgDynamicParameters() const;

    /**
     * Sets the SVG dynamic parameters
     * \since QGIS 3.20
     */
    void setSvgDynamicParameters( const QMap<QString, QgsProperty> &parameters );

  public slots:

    /**
     * Sets the picture \a rotation within the item bounds, in degrees clockwise. This does not affect
     * the item's frame, only the way the picture is drawn within the item.
     * \see pictureRotation()
     */
    void setPictureRotation( double rotation );

    /**
     * Sets the resize \a mode used for drawing the picture within the item bounds.
     * \param mode ResizeMode to use for image file
     * \see resizeMode
     */
    void setResizeMode( QgsLayoutItemPicture::ResizeMode mode );

    /**
     * Recalculates the source image (if using an expression for picture's source)
     * and reloads and redraws the picture.
     * \param context expression context for evaluating data defined picture sources
     */
    void refreshPicture( const QgsExpressionContext *context = nullptr );

    /**
     * Forces a recalculation of the picture's frame size
     */
    void recalculateSize();

    void refreshDataDefinedProperty( QgsLayoutObject::DataDefinedProperty property = QgsLayoutObject::AllProperties ) override;

  signals:
    //! Emitted on picture rotation change
    void pictureRotationChanged( double newRotation );

  protected:

    void draw( QgsLayoutItemRenderContext &context ) override;
    QSizeF applyItemSizeConstraint( QSizeF targetSize ) override;
    bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const override;
    bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context ) override;

  private:

    QgsLayoutItemPicture() = delete;

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
    Format mMode = FormatUnknown;
    Format mOriginalMode = FormatUnknown;

    QSize mDefaultSvgSize;

    //! Image rotation
    double mPictureRotation = 0;

    QString mRotationMapUuid;

    //! Width of the picture (in mm)
    double mPictureWidth = 0.0;
    //! Height of the picture (in mm)
    double mPictureHeight = 0.0;

    ResizeMode mResizeMode = QgsLayoutItemPicture::Zoom;
    QgsLayoutItem::ReferencePoint mPictureAnchor = UpperLeft;

    QColor mSvgFillColor = QColor( 255, 255, 255 );
    QColor mSvgStrokeColor = QColor( 0, 0, 0 );
    double mSvgStrokeWidth = 0.2;

    bool mHasExpressionError = false;
    bool mLoaded = false;
    bool mLoadingSvg = false;
    bool mIsMissingImage = false;
    QString mEvaluatedPath;

    QgsLayoutNorthArrowHandler *mNorthArrowHandler = nullptr;

    //! Loads an image file into the picture item and redraws the item
    void loadPicture( const QVariant &data );

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

    void loadPictureUsingCache( const QString &path );

    QgsLayoutItemPicture( const QgsLayoutItemPicture & ) = delete;
    QgsLayoutItemPicture &operator=( const QgsLayoutItemPicture & ) = delete;

  private slots:

    void updateNorthArrowRotation( double rotation );

    void shapeChanged();

    friend class QgsCompositionConverter;
    friend class TestQgsCompositionConverter;

};

#endif // QGSLAYOUTITEMPICTURE_H
