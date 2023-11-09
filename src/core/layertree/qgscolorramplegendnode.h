/***************************************************************************
  qgscolorramplegendnode.h
  --------------------------------------
  Date                 : December 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOLORRAMPLEGENDNODE_H
#define QGSCOLORRAMPLEGENDNODE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgslayertreemodellegendnode.h"
#include "qgscolorramplegendnodesettings.h"

class QgsColorRamp;

/**
 * \ingroup core
 * \brief A legend node which renders a color ramp.
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsColorRampLegendNode : public QgsLayerTreeModelLegendNode
{
    Q_OBJECT


  public:

    /**
     * Constructor for QgsColorRampLegendNode.
     * \param nodeLayer layer node
     * \param ramp color ramp to render in node. Ownership is transferred to the node.
     * \param minimumLabel label text to render for the minimum value in the ramp
     * \param maximumLabel label text to render for the maximum value in the ramp
     * \param parent attach a parent QObject to the legend node.
     */
    QgsColorRampLegendNode( QgsLayerTreeLayer *nodeLayer, QgsColorRamp *ramp SIP_TRANSFER,
                            const QString &minimumLabel, const QString &maximumLabel, QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Constructor for QgsColorRampLegendNode.
     * \param nodeLayer layer node
     * \param ramp color ramp to render in node. Ownership is transferred to the node.
     * \param settings node settings
     * \param minimumValue value associated with minimum of ramp
     * \param maximumValue value associated with maximum of ramp
     * \param parent attach a parent QObject to the legend node.
     */
    QgsColorRampLegendNode( QgsLayerTreeLayer *nodeLayer, QgsColorRamp *ramp SIP_TRANSFER,
                            const QgsColorRampLegendNodeSettings &settings, double minimumValue,
                            double maximumValue, QObject *parent SIP_TRANSFERTHIS = nullptr );


    QVariant data( int role ) const override;
    QSizeF drawSymbol( const QgsLegendSettings &settings, ItemContext *ctx, double itemHeight ) const override;
    QSizeF drawSymbolText( const QgsLegendSettings &settings, ItemContext *ctx, QSizeF symbolSize ) const override;

    /**
     * Set the icon \a size, which controls how large the ramp will render in a layer tree widget.
     *
     * \see iconSize()
     */
    void setIconSize( QSize size ) { mIconSize = size; }

    /**
     * Returns the icon size, which is how large the ramp will render in a layer tree widget.
     *
     * \see setIconSize()
     */
    QSize iconSize() const { return mIconSize; }

    /**
     * Returns the color ramp used by the node.
     */
    const QgsColorRamp *ramp() const;

    /**
     * Returns the node's settings.
     *
     * \see setSettings()
     */
    QgsColorRampLegendNodeSettings settings() const;

    /**
     * Sets the node's \a settings.
     *
     * \see settings()
     */
    void setSettings( const QgsColorRampLegendNodeSettings &settings );

  private:
    void init( QgsLayerTreeLayer *nodeLayer );

    QString labelForMinimum() const;
    QString labelForMaximum() const;

    std::unique_ptr< QgsColorRamp > mRamp;

    mutable QPixmap mPixmap; // cached symbol preview
    QSize mIconSize;

    QgsColorRampLegendNodeSettings mSettings;
    double mMinimumValue = 0;
    double mMaximumValue = 0;

};



#endif // QGSCOLORRAMPLEGENDNODE_H
