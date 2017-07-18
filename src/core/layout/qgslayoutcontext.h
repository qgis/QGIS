/***************************************************************************
                             qgslayoutcontext.h
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
#ifndef QGSLAYOUTCONTEXT_H
#define QGSLAYOUTCONTEXT_H

#include "qgis_core.h"
#include "qgsfeature.h"
#include "qgsvectorlayer.h"
#include "qgslayoutmeasurementconverter.h"
#include <QtGlobal>

class QgsFeature;
class QgsVectorLayer;

/**
 * \ingroup core
 * \class QgsLayoutContext
 * \brief Stores information relating to the current context and rendering settings for a layout.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutContext
{

  public:

    //! Flags for controlling how a layout is rendered
    enum Flag
    {
      FlagDebug = 1 << 1,  //!< Debug/testing mode, items are drawn as solid rectangles.
      FlagOutlineOnly = 1 << 2, //!< Render items as outlines only.
      FlagAntialiasing = 1 << 3, //!< Use antialiasing when drawing items.
      FlagUseAdvancedEffects = 1 << 4, //!< Enable advanced effects such as blend modes.
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    QgsLayoutContext();

    /**
     * Sets the combination of \a flags that will be used for rendering the layout.
     * \see setFlag()
     * \see flags()
     * \see testFlag()
     */
    void setFlags( const QgsLayoutContext::Flags flags );

    /**
     * Enables or disables a particular rendering \a flag for the layout. Other existing
     * flags are not affected.
     * \see setFlags()
     * \see flags()
     * \see testFlag()
     */
    void setFlag( const QgsLayoutContext::Flag flag, const bool on = true );

    /**
     * Returns the current combination of flags used for rendering the layout.
     * \see setFlags()
     * \see setFlag()
     * \see testFlag()
     */
    QgsLayoutContext::Flags flags() const;

    /**
     * Check whether a particular rendering \a flag is enabled for the layout.
     * \see setFlags()
     * \see setFlag()
     * \see flags()
     */
    bool testFlag( const Flag flag ) const;

    /**
     * Sets the current \a feature for evaluating the layout. This feature may
     * be used for altering an item's content and appearance for a report
     * or atlas layout.
     * \see feature()
     */
    void setFeature( const QgsFeature &feature ) { mFeature = feature; }

    /**
     * Returns the current feature for evaluating the layout. This feature may
     * be used for altering an item's content and appearance for a report
     * or atlas layout.
     * \see setFeature()
     */
    QgsFeature feature() const { return mFeature; }

    /**
     * Returns the vector layer associated with the layout's context.
     * \see setLayer()
     */
    QgsVectorLayer *layer() const;

    /**
     * Sets the vector \a layer associated with the layout's context.
     * \see layer()
     */
    void setLayer( QgsVectorLayer *layer );

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
     * Returns the layout measurement converter to be used in the layout. This converter is used
     * for translating between other measurement units and the layout's native unit.
     */
    SIP_SKIP const QgsLayoutMeasurementConverter &measurementConverter() const { return mMeasurementConverter; }

    /**
     * Returns the layout measurement converter to be used in the layout. This converter is used
     * for translating between other measurement units and the layout's native unit.
     */
    QgsLayoutMeasurementConverter &measurementConverter() { return mMeasurementConverter; }

  private:

    Flags mFlags = 0;

    QgsFeature mFeature;
    QPointer< QgsVectorLayer > mLayer;

    QgsLayoutMeasurementConverter mMeasurementConverter;

};

#endif //QGSLAYOUTCONTEXT_H



