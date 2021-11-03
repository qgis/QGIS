/***************************************************************************
                             qgslayoutrendercontext.h
                             -------------------
    begin                : July 2017
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
#ifndef QGSLAYOUTRENDERCONTEXT_H
#define QGSLAYOUTRENDERCONTEXT_H

#include "qgis_core.h"
#include "qgslayoutmeasurementconverter.h"
#include "qgsvectorsimplifymethod.h"
#include "qgis.h"
#include <QtGlobal>
#include <QColor>
#include <QVector>

class QgsLayout;
class QgsFeatureFilterProvider;

/**
 * \ingroup core
 * \class QgsLayoutRenderContext
 * \brief Stores information relating to the current rendering settings for a layout.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutRenderContext : public QObject
{

    Q_OBJECT

  public:

    //! Flags for controlling how a layout is rendered
    enum Flag
    {
      FlagDebug = 1 << 1,  //!< Debug/testing mode, items are drawn as solid rectangles.
      FlagOutlineOnly = 1 << 2, //!< Render items as outlines only.
      FlagAntialiasing = 1 << 3, //!< Use antialiasing when drawing items.
      FlagUseAdvancedEffects = 1 << 4, //!< Enable advanced effects such as blend modes.
      FlagForceVectorOutput = 1 << 5, //!< Force output in vector format where possible, even if items require rasterization to keep their correct appearance.
      FlagHideCoverageLayer = 1 << 6, //!< Hide coverage layer in outputs
      FlagDrawSelection = 1 << 7, //!< Draw selection
      FlagDisableTiledRasterLayerRenders = 1 << 8, //!< If set, then raster layers will not be drawn as separate tiles. This may improve the appearance in exported files, at the cost of much higher memory usage during exports.
      FlagRenderLabelsByMapLayer = 1 << 9, //!< When rendering map items to multi-layered exports, render labels belonging to different layers into separate export layers
      FlagLosslessImageRendering = 1 << 10, //!< Render images losslessly whenever possible, instead of the default lossy jpeg rendering used for some destination devices (e.g. PDF). This flag only works with builds based on Qt 5.13 or later.
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /**
     * Constructor for QgsLayoutRenderContext.
     */
    QgsLayoutRenderContext( QgsLayout *layout SIP_TRANSFERTHIS );

    /**
     * Sets the combination of \a flags that will be used for rendering the layout.
     * \see setFlag()
     * \see flags()
     * \see testFlag()
     */
    void setFlags( QgsLayoutRenderContext::Flags flags );

    /**
     * Enables or disables a particular rendering \a flag for the layout. Other existing
     * flags are not affected.
     * \see setFlags()
     * \see flags()
     * \see testFlag()
     */
    void setFlag( QgsLayoutRenderContext::Flag flag, bool on = true );

    /**
     * Returns the current combination of flags used for rendering the layout.
     * \see setFlags()
     * \see setFlag()
     * \see testFlag()
     */
    QgsLayoutRenderContext::Flags flags() const;

    /**
     * Check whether a particular rendering \a flag is enabled for the layout.
     * \see setFlags()
     * \see setFlag()
     * \see flags()
     */
    bool testFlag( Flag flag ) const;

    /**
     * Returns the combination of render context flags matched to the layout context's settings.
     */
    Qgis::RenderContextFlags renderContextFlags() const;

    /**
     * Sets the \a dpi for outputting the layout. This also sets the
     * corresponding DPI for the context's measurementConverter().
     * \see dpi()
     */
    void setDpi( double dpi );

    /**
     * Returns the \a dpi for outputting the layout.
     * \see setDpi()
     */
    double dpi() const;

    /**
     * Sets color that is used for drawing of selected vector features
     * \see selectionColor()
     * \since QGIS 3.4
     */
    void setSelectionColor( const QColor &color ) { mSelectionColor = color; }

    /**
     * Gets color that is used for drawing of selected vector features
     * \see setSelectionColor()
     * \since QGIS 3.4
     */
    QColor selectionColor() const { return mSelectionColor; }

    /**
     * Returns the layout measurement converter to be used in the layout. This converter is used
     * for translating between other measurement units and the layout's native unit.
     */
    SIP_SKIP const QgsLayoutMeasurementConverter &measurementConverter() const { return mMeasurementConverter; }

    /**
     * Returns the layout measurement converter to be used in the layout. This converter is used
     * for translating between other measurement units and the layout's native unit.
     */
    QgsLayoutMeasurementConverter &measurementConverter() { return mMeasurementConverter; }

    /**
     * Returns TRUE if the render current being conducted is a preview render,
     * i.e. it is being rendered inside a QGraphicsView widget as opposed to a destination
     * device (such as an image).
     */
    bool isPreviewRender() const { return mIsPreviewRender; }

    /**
     * Returns TRUE if the page grid should be drawn.
     * \see setGridVisible()
     */
    bool gridVisible() const;

    /**
     * Sets whether the page grid should be \a visible.
     * \see gridVisible()
     */
    void setGridVisible( bool visible );

    /**
     * Returns TRUE if the item bounding boxes should be drawn.
     * \see setBoundingBoxesVisible()
     */
    bool boundingBoxesVisible() const;

    /**
     * Sets whether the item bounding boxes should be \a visible.
     * \see boundingBoxesVisible()
     */
    void setBoundingBoxesVisible( bool visible );

    /**
     * Sets whether the page items should be \a visible in the layout. Removing
     * them will prevent both display of the page boundaries in layout views and
     * will also prevent them from being rendered in layout exports.
     * \see pagesVisible()
     */
    void setPagesVisible( bool visible );

    /**
     * Returns whether the page items are be visible in the layout. This setting
     * effects both display of the page boundaries in layout views and
     * whether they will be rendered in layout exports.
     * \see setPagesVisible()
     */
    bool pagesVisible() const { return mPagesVisible; }

    /**
     * Sets the current item \a layer to draw while exporting. QgsLayoutItem subclasses
     * which support multi-layer SVG exports must check the currentExportLayer()
     * and customize their rendering based on the layer.
     *
     * If \a layer is -1, all item layers will be rendered.
     *
     * \see currentExportLayer()
     * \deprecated Items should now handle this themselves, via QgsLayoutItem::exportLayerBehavior() and returning QgsLayoutItem::nextExportPart().
     */
    Q_DECL_DEPRECATED void setCurrentExportLayer( int layer = -1 ) SIP_DEPRECATED { mCurrentExportLayer = layer; }

    /**
     * Returns the current item layer to draw while exporting. QgsLayoutItem subclasses
     * which support multi-layer SVG exports must check this
     * and customize their rendering based on the layer.
     *
     * If \a layer is -1, all item layers should be rendered.
     *
     * \see setCurrentExportLayer()
     * \deprecated Items should now handle this themselves, via QgsLayoutItem::exportLayerBehavior() and returning QgsLayoutItem::nextExportPart().
     */
    Q_DECL_DEPRECATED int currentExportLayer() const SIP_DEPRECATED  { return mCurrentExportLayer; }

    /**
     * Returns the text render format, which dictates how text is rendered (e.g. as paths or real text objects).
     *
     * \see setTextRenderFormat()
     * \since QGIS 3.4.3
     */
    Qgis::TextRenderFormat textRenderFormat() const
    {
      return mTextRenderFormat;
    }

    /**
     * Sets the text render \a format, which dictates how text is rendered (e.g. as paths or real text objects).
     *
     * \see textRenderFormat()
     * \since QGIS 3.4.3
     */
    void setTextRenderFormat( Qgis::TextRenderFormat format )
    {
      mTextRenderFormat = format;
    }

    /**
     * Sets the simplification setting to use when rendering vector layers.
     *
     * If the simplify \a method is enabled, it apply to all vector layers rendered inside map items.
     *
     * This can be used to specify global simplification methods to apply during map exports,
     * e.g. to allow vector layers to be simplified to an appropriate maximum level of detail
     * during PDF exports (avoiding excessive PDF size due to huge numbers of vertices).
     *
     * The default is to use no simplification.
     *
     * \note This simplification method is only used during non-preview renders.
     *
     * \see simplifyMethod()
     *
     * \since QGIS 3.10
     */
    void setSimplifyMethod( const QgsVectorSimplifyMethod &method ) { mSimplifyMethod = method; }

    /**
     * Returns the simplification settings to use when rendering vector layers.
     *
     * If enabled, it will apply to all vector layers rendered for the map.
     *
     * The default is to use no simplification.
     *
     * \note This simplification method is only used during non-preview renders.
     *
     * \see setSimplifyMethod()
     * \since QGIS 3.10
     */
    const QgsVectorSimplifyMethod &simplifyMethod() const { return mSimplifyMethod; }

    /**
     * Returns a list of map themes to use during the export.
     *
     * Items which handle layered exports (e.g. maps) may utilize this list to export different
     * representations of the item as export layers, as they iterate through these included themes.
     *
     * \see setExportThemes()
     * \since QGIS 3.10
     */
    QStringList exportThemes() const;

    /**
     * Sets a list of map \a themes to use during the export.
     *
     * Items which handle layered exports (e.g. maps) may utilize this list to export different
     * representations of the item as export layers, as they iterate through these included themes.
     *
     * \see exportThemes()
     * \since QGIS 3.10
     */
    void setExportThemes( const QStringList &themes );

    /**
     * Sets the list of predefined \a scales to use with the layout. This is used
     * for maps which are set to the predefined atlas scaling mode.
     * \see predefinedScales()
     * \since QGIS 3.10
     */
    void setPredefinedScales( const QVector<qreal> &scales );

    /**
     * Returns the current list of predefined scales for use with the layout.
     * \see setPredefinedScales()
     * \since QGIS 3.10
     */
    QVector<qreal> predefinedScales() const { return mPredefinedScales; }

    /**
     * Returns the possibly NULL feature filter provider.
     *
     * A feature filter provider for filtering visible features or attributes.
     * It is currently used by QGIS Server Access Control Plugins.
     *
     * \since QGIS 3.18
     */
    QgsFeatureFilterProvider *featureFilterProvider() const;

    /**
     * Sets feature filter provider to \a featureFilterProvider.
     *
     * A feature filter provider for filtering visible features or attributes.
     * It is currently used by QGIS Server Access Control Plugins.
     *
     * \since QGIS 3.18
     */
    void setFeatureFilterProvider( QgsFeatureFilterProvider *featureFilterProvider );

  signals:

    /**
     * Emitted whenever the context's \a flags change.
     * \see setFlags()
     */
    void flagsChanged( QgsLayoutRenderContext::Flags flags );

    /**
     * Emitted when the context's DPI is changed.
     */
    void dpiChanged();

    /**
     * Emitted when the list of predefined scales changes.
     * \see predefinedScales()
     * \since QGIS 3.10
     */
    void predefinedScalesChanged();

  private:

    Flags mFlags = Flags();

    QgsLayout *mLayout = nullptr;

    int mCurrentExportLayer = -1;

    QColor mSelectionColor = Qt::yellow;

    QgsLayoutMeasurementConverter mMeasurementConverter;

    bool mIsPreviewRender = true;
    bool mGridVisible = false;
    bool mBoundingBoxesVisible = true;
    bool mPagesVisible = true;

    Qgis::TextRenderFormat mTextRenderFormat = Qgis::TextRenderFormat::AlwaysOutlines;

    QStringList mExportThemes;

    QgsVectorSimplifyMethod mSimplifyMethod;

    QVector<qreal> mPredefinedScales;

    QgsFeatureFilterProvider *mFeatureFilterProvider = nullptr;

    friend class QgsLayoutExporter;
    friend class TestQgsLayout;
    friend class LayoutContextPreviewSettingRestorer;
    friend class TestQgsLayoutMap;
    friend class TestQgsLayoutLabel;

};

Q_DECLARE_METATYPE( QgsLayoutRenderContext::Flags )

#endif //QGSLAYOUTRENDERCONTEXT_H



