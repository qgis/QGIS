/***************************************************************************
                         qgslabelsink.h
                         -------------------
    begin                : January 2014
    copyright            : (C) 2014 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLABELSINK_H
#define QGSLABELSINK_H

#define SIP_NO_FILE

#include "qgsvectorlayerlabelprovider.h"
#include "qgsrulebasedlabeling.h"

class QgsPalLayerSettings;
class QgsRuleBasedLabeling;

/**
 * \ingroup core
 * \brief Abstract base class that can be used to intercept rendered labels from
 * a labeling / rendering job.
 *
 * \note not available in Python bindings
 * \since QGIS 3.14
 */
class QgsLabelSink
{
  public:
    virtual ~QgsLabelSink() = default;

    /**
     * The drawLabel method is called for each label that is being drawn.
     * Every subclass must implement this method to draw the label or send the information from \a label
     * to another desired location.
     */
    virtual void drawLabel( const QString &layerId, QgsRenderContext &context, pal::LabelPosition *label, const QgsPalLayerSettings &settings ) = 0;

    /**
     * The drawLabel method is called for each unplaced label.
     * \param layerId The layer ID associated to the label
     * \param context The render context object
     * \param label The label object
     * \param settings The layer labeling settings
     * \since QGIS 3.24
     */
    virtual void drawUnplacedLabel( const QString &layerId, QgsRenderContext &context, pal::LabelPosition *label, const QgsPalLayerSettings &settings )
    {
      Q_UNUSED( layerId );
      Q_UNUSED( context )
      Q_UNUSED( label );
      Q_UNUSED( settings );
      return;
    }
};

/**
 * \ingroup core
 * \brief Implements a derived label provider for use with QgsLabelSink.
 *
 * \note not available in Python bindings
 * \since QGIS 3.14
 */
class QgsLabelSinkProvider : public QgsVectorLayerLabelProvider
{
  public:
    //! Creates a rule based label sink provider which will draw/register labels in \a sink.
    explicit QgsLabelSinkProvider( QgsVectorLayer *layer, const QString &providerId, QgsLabelSink *sink, const QgsPalLayerSettings *settings );

    void drawLabel( QgsRenderContext &context, pal::LabelPosition *label ) const override;
    void drawUnplacedLabel( QgsRenderContext &context, pal::LabelPosition *label ) const override;

  private:
    QgsLabelSink *mLabelSink = nullptr;
};

/**
 * \ingroup core
 * \brief Implements a derived label provider for rule based labels for use with QgsLabelSink.
 *
 * \note not available in Python bindings
 * \since QGIS 3.14
 */
class QgsRuleBasedLabelSinkProvider : public QgsRuleBasedLabelProvider
{
  public:
    //! Creates a rule based label sink provider which will draw/register labels in \a sink.
    explicit QgsRuleBasedLabelSinkProvider( const QgsRuleBasedLabeling &rules, QgsVectorLayer *layer, QgsLabelSink *sink );

    /**
     * Reinitialize the subproviders with QgsLabelSinkProviders
     * \deprecated since QGIS 3.12
     */
    Q_DECL_DEPRECATED void reinit( QgsVectorLayer *layer );

    void drawLabel( QgsRenderContext &context, pal::LabelPosition *label ) const override;
    void drawUnplacedLabel( QgsRenderContext &context, pal::LabelPosition *label ) const override;

    //! Creates a  QgsRuleBasedLabelSinkProvider
    QgsVectorLayerLabelProvider *createProvider( QgsVectorLayer *layer, const QString &providerId, bool withFeatureLoop, const QgsPalLayerSettings *settings ) override;

  private:
    QgsLabelSink *mLabelSink = nullptr;
};



#endif // QGSLABELSINK_H
