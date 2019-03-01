/***************************************************************************
  qgsdatadefinedsizelegend.h
  --------------------------------------
  Date                 : June 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDATADEFINEDSIZELEGEND_H
#define QGSDATADEFINEDSIZELEGEND_H

#include "qgslegendsymbolitem.h"

#include <QColor>
#include <QFont>

class QDomElement;
class QgsMarkerSymbol;
class QgsProperty;
class QgsReadWriteContext;
class QgsRenderContext;
class QgsSizeScaleTransformer;


/**
 * \ingroup core
 * Object that keeps configuration of appearance of marker symbol's data-defined size in legend.
 * For example: the list of classes (size values), whether the classes should appear in separate
 * legend nodes or whether to collapse them into one legend node.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsDataDefinedSizeLegend
{
  public:

    /**
     * Constructor for QgsDataDefinedSizeLegend.
     */
    QgsDataDefinedSizeLegend() = default;

    //! Copy constructor
    QgsDataDefinedSizeLegend( const QgsDataDefinedSizeLegend &other );
    QgsDataDefinedSizeLegend &operator=( const QgsDataDefinedSizeLegend &other );

    //! Determines how to display data-defined size legend
    enum LegendType
    {
      LegendSeparated,   //!< Each class (size value) has a separate legend node
      LegendCollapsed,   //!< All classes are rendered within one legend node
    };

    //! How to vertically align symbols when all classes go into one node
    enum VerticalAlignment
    {
      AlignCenter,      //!< Symbols are aligned to the center
      AlignBottom,      //!< Symbols are aligned to the bottom
    };

    //! Definition of one class for the legend
    struct SizeClass
    {
      SizeClass( double size, const QString &label ): size( size ), label( label ) {}

      double size;    //!< Marker size in units used by the symbol (usually millimeters). May be further scaled before rendering if size scale transformer is enabled.
      QString label;  //!< Label to be shown with the particular symbol size
    };

    //! Sets how the legend should be rendered
    void setLegendType( LegendType type ) { mType = type; }
    //! Returns how the legend should be rendered
    LegendType legendType() const { return mType; }

    //! Sets marker symbol that will be used to draw markers in legend
    void setSymbol( QgsMarkerSymbol *symbol SIP_TRANSFER );
    //! Returns marker symbol that will be used to draw markers in legend
    QgsMarkerSymbol *symbol() const;

    //! Sets transformer for scaling of symbol sizes. Takes ownership of the object. Accepts NULLPTR to set no transformer.
    void setSizeScaleTransformer( QgsSizeScaleTransformer *transformer SIP_TRANSFER );
    //! Returns transformer for scaling of symbol sizes. Returns NULLPTR if no transformer is defined.
    QgsSizeScaleTransformer *sizeScaleTransformer() const;

    //! Sets list of classes: each class is a pair of symbol size (in units used by the symbol) and label
    void setClasses( const QList< QgsDataDefinedSizeLegend::SizeClass > &classes ) { mSizeClasses = classes; }
    //! Returns list of classes: each class is a pair of symbol size (in units used by the symbol) and label
    QList< QgsDataDefinedSizeLegend::SizeClass > classes() const { return mSizeClasses; }

    //! Sets title label for data-defined size legend
    void setTitle( const QString &title ) { mTitleLabel = title; }
    //! Returns title label for data-defined size legend
    QString title() const { return mTitleLabel; }

    //! Sets vertical alignment of symbols - only valid for collapsed legend
    void setVerticalAlignment( VerticalAlignment vAlign ) { mVAlign = vAlign; }
    //! Returns vertical alignment of symbols - only valid for collapsed legend
    VerticalAlignment verticalAlignment() const { return mVAlign; }

    //! Sets font used for rendering of labels - only valid for collapsed legend
    void setFont( const QFont &font ) { mFont = font; }
    //! Returns font used for rendering of labels - only valid for collapsed legend
    QFont font() const { return mFont; }

    //! Sets text color for rendering of labels - only valid for collapsed legend
    void setTextColor( const QColor &color ) { mTextColor = color; }
    //! Returns text color for rendering of labels - only valid for collapsed legend
    QColor textColor() const { return mTextColor; }

    //! Sets horizontal text alignment for rendering of labels - only valid for collapsed legend
    void setTextAlignment( Qt::AlignmentFlag flag ) { mTextAlignment = flag; }
    //! Returns horizontal text alignment for rendering of labels - only valid for collapsed legend
    Qt::AlignmentFlag textAlignment() const { return mTextAlignment; }

    //

    //! Updates the list of classes, source symbol and title label from given symbol and property
    void updateFromSymbolAndProperty( const QgsMarkerSymbol *symbol, const QgsProperty &ddSize );

    //! Generates legend symbol items according to the configuration
    QgsLegendSymbolList legendSymbolList() const;

    /**
     * Draw the legend if using LegendOneNodeForAll and optionally output size of the legend and x offset of labels (in painter units).
     * If the painter in context is NULLPTR, it only does size calculation without actual rendering.
     * Does nothing if legend is not configured as collapsed.
     */
    void drawCollapsedLegend( QgsRenderContext &context, QSize *outputSize SIP_OUT = nullptr, int *labelXOffset SIP_OUT = nullptr ) const;

    //! Returns output image that would be shown in the legend. Returns invalid image if legend is not configured as collapsed.
    QImage collapsedLegendImage( QgsRenderContext &context, const QColor &backgroundColor = Qt::transparent, double paddingMM = 1 ) const;

    //! Creates instance from given element and returns it (caller takes ownership). Returns NULLPTR on error.
    static QgsDataDefinedSizeLegend *readXml( const QDomElement &elem, const QgsReadWriteContext &context ) SIP_FACTORY;

    //! Writes configuration to the given XML element.
    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const;

  private:
    LegendType mType = LegendSeparated;
    QString mTitleLabel;  //!< Title label for the following size-based item(s)
    QList< SizeClass > mSizeClasses;  //!< List of classes: symbol size (in whatever units symbol uses) + label
    std::unique_ptr<QgsMarkerSymbol> mSymbol;
    std::unique_ptr<QgsSizeScaleTransformer> mSizeScaleTransformer;  //!< Optional transformer for classes
    VerticalAlignment mVAlign = AlignBottom;
    QFont mFont;
    QColor mTextColor = Qt::black;
    Qt::AlignmentFlag mTextAlignment = Qt::AlignLeft;
};

#endif // QGSDATADEFINEDSIZELEGEND_H
