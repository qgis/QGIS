/***************************************************************************
                          qgsmaptoollabel.h
                          --------------------
    begin                : 2010-11-03
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLLABEL_H
#define QGSMAPTOOLLABEL_H

#include "qgsmaptooladvanceddigitizing.h"
#include "qgspallabeling.h"
#include "qgsnewauxiliarylayerdialog.h"
#include "qgsauxiliarystorage.h"
#include "qgscalloutposition.h"
#include "qgscallout.h"
#include "qgis_app.h"

class QgsRubberBand;

typedef QMap<QgsPalLayerSettings::Property, int> QgsPalIndexes;
typedef QMap<QgsDiagramLayerSettings::Property, int> QgsDiagramIndexes;
typedef QMap<QgsCallout::Property, int> QgsCalloutIndexes;

//! Base class for map tools that modify label properties
class APP_EXPORT QgsMapToolLabel: public QgsMapToolAdvancedDigitizing
{
    Q_OBJECT

  public:
    QgsMapToolLabel( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDock );
    ~QgsMapToolLabel() override;



    void deactivate() override;

    /**
     * Returns true if diagram move can be applied to a layer
     * \param xCol out: index of the attribute for data defined x coordinate
     * \param yCol out: index of the attribute for data defined y coordinate
     * \returns TRUE if labels of layer can be moved
    */
    bool diagramMoveable( QgsVectorLayer *vlayer, int &xCol, int &yCol ) const;

    /**
     * Returns TRUE if layer has attribute field set up for diagrams
     * \param showCol out: attribute column for data defined diagram showing
     * \since QGIS 2.16
    */
    bool diagramCanShowHide( QgsVectorLayer *vlayer, int &showCol ) const;

    /**
     * Returns TRUE if layer has attribute field set up
     * \param showCol out: attribute column for data defined label showing
    */
    bool labelCanShowHide( QgsVectorLayer *vlayer, int &showCol ) const;

    enum class PropertyStatus
    {
      Valid,
      DoesNotExist,
      CurrentExpressionInvalid
    };

    /**
     * Checks if labels in a layer can be rotated
     * \param rotationCol out: attribute column for data defined label rotation
    */
    PropertyStatus labelRotatableStatus( QgsVectorLayer *layer, const QgsPalLayerSettings &settings, int &rotationCol ) const;

  protected:
    QgsRubberBand *mHoverRubberBand = nullptr;
    QgsRubberBand *mCalloutOtherPointsRubberBand = nullptr;
    QgsRubberBand *mLabelRubberBand = nullptr;
    QgsRubberBand *mFeatureRubberBand = nullptr;
    //! Shows label fixpoint (left/bottom by default)
    QgsRubberBand *mFixPointRubberBand = nullptr;

    struct APP_EXPORT LabelDetails
    {
      LabelDetails() = default;
      explicit LabelDetails( const QgsLabelPosition &p, QgsMapCanvas *canvas );
      bool valid = false;
      QgsLabelPosition pos;
      QgsVectorLayer *layer = nullptr;
      QgsPalLayerSettings settings;
    };

    //! Currently dragged label position
    LabelDetails mCurrentLabel;

    //! Currently hovered label position
    LabelDetails mCurrentHoverLabel;

    QgsCalloutPosition mCurrentCallout;

    /**
     * Returns label position for mouse click location
     * \param e mouse event
     * \param p out: label position
     * \returns TRUE in case of success, FALSE if no label at this location
    */
    bool labelAtPosition( QMouseEvent *e, QgsLabelPosition &p );

    /**
     * Returns callout position for a mouse event.
     * \param e mouse event
     * \param p out: callout position
     * \param isOrigin set to TRUE if the origin is at the position, or FALSE if the callout destination is at the position
     * \returns TRUE in case of success, FALSE if no callout at this location
    */
    bool calloutAtPosition( QMouseEvent *e, QgsCalloutPosition &p, bool &isOrigin );

    /**
     * Finds out rotation point of current label position
     * \param ignoreUpsideDown treat label as right-side-up
     * \returns TRUE in case of success
    */
    bool currentLabelRotationPoint( QgsPointXY &pos, bool ignoreUpsideDown = false );

    //! Creates label / feature / fixpoint rubber bands for the current label position
    void createRubberBands();

    //! Removes label / feature / fixpoint rubber bands
    virtual void deleteRubberBands();

    /**
     * Returns current label's text
     * \param trunc number of chars to truncate to, with ... added
    */
    QString currentLabelText( int trunc = 0 );

    void currentAlignment( QString &hali, QString &vali );

    /**
     * Gets vector feature for current label pos
     * \returns TRUE in case of success
    */
    bool currentFeature( QgsFeature &f, bool fetchGeom = false );

    //! Returns the font for the current feature (considering default font and data defined properties)
    QFont currentLabelFont();

    //! Returns a data defined attribute column name for particular property or empty string if not defined
    QString dataDefinedColumnName( QgsPalLayerSettings::Property p, const QgsPalLayerSettings &labelSettings, const QgsVectorLayer *layer, PropertyStatus &status ) const;

    /**
     * Returns a data defined attribute column index
     * \returns -1 if column does not exist or an expression is used instead
    */
    int dataDefinedColumnIndex( QgsPalLayerSettings::Property p, const QgsPalLayerSettings &labelSettings, const QgsVectorLayer *vlayer ) const;

    /**
     * Evaluates a labeling data defined property for the specified \a feature.
     */
    QVariant evaluateDataDefinedProperty( QgsPalLayerSettings::Property property, const QgsPalLayerSettings &labelSettings, const QgsFeature &feature, const QVariant &defaultValue ) const;

    //! Returns whether to preserve predefined rotation data during label pin/unpin operations
    bool currentLabelPreserveRotation();

    /**
     * Gets data defined position of current label
     * \param x out: data defined x-coordinate
     * \param xSuccess out: FALSE if attribute value is NULL
     * \param y out: data defined y-coordinate
     * \param ySuccess out: FALSE if attribute value is NULL
     * \param xCol out: index of the x position column
     * \param yCol out: index of the y position column
     * \param pointCol out: index of the point position column
     * \returns FALSE if layer does not have data defined label position enabled
    */
    bool currentLabelDataDefinedPosition( double &x, bool &xSuccess, double &y, bool &ySuccess, int &xCol, int &yCol, int &pointCol ) const;

    /**
     * Returns data defined rotation of current label
     * \param rotation out: rotation value
     * \param rotationSuccess out: FALSE if rotation value is NULL
     * \param rCol out: index of the rotation column
     * \param ignoreXY ignore that x and y are required to be data-defined
     * \returns TRUE if data defined rotation is enabled on the layer
     */
    bool currentLabelDataDefinedRotation( double &rotation, bool &rotationSuccess, int &rCol, bool ignoreXY = false ) const;

    /**
     * Change the data defined position of current label
     * \param rCol out: index of the rotation column
     * \param x data defined x-coordinate
     * \param y data defined y-coordinate
     * \returns TRUE if data defined position could be changed
     */
    bool changeCurrentLabelDataDefinedPosition( const QVariant &x, const QVariant &y );

    /**
     * Returns data defined show/hide of a feature.
     * \param vlayer vector layer
     * \param featureId feature identification integer
     * \param show out: show/hide value
     * \param showSuccess out: FALSE if show/hide value is NULL
     * \param showCol out: index of the show label column
     * \returns TRUE if data defined show/hide is enabled on the layer
    */
    bool dataDefinedShowHide( QgsVectorLayer *vlayer, QgsFeatureId featureId, int &show, bool &showSuccess, int &showCol ) const;

    /**
     * Returns the pin status for the current label/diagram
     * \returns TRUE if the label/diagram is pinned, FALSE otherwise
     * \since QGIS 2.16
    */
    bool isPinned();

    bool labelMoveable( QgsVectorLayer *vlayer, const QgsPalLayerSettings &settings, int &xCol, int &yCol, int &pointCol ) const;

    bool createAuxiliaryFields( QgsPalIndexes &palIndexes );
    bool createAuxiliaryFields( LabelDetails &details, QgsPalIndexes &palIndexes ) const;
    bool createAuxiliaryFields( QgsDiagramIndexes &diagIndexes );
    bool createAuxiliaryFields( LabelDetails &details, QgsDiagramIndexes &diagIndexes );
    bool createAuxiliaryFields( QgsCalloutIndexes &calloutIndexes );
    bool createAuxiliaryFields( QgsCalloutPosition &details, QgsCalloutIndexes &calloutIndexes );

    void updateHoveredLabel( QgsMapMouseEvent *e );
    void clearHoveredLabel();
    virtual bool canModifyLabel( const LabelDetails &label );
    virtual bool canModifyCallout( const QgsCalloutPosition &callout, bool isOrigin, int &xCol, int &yCol );

    QList<QgsPalLayerSettings::Property> mPalProperties;
    QList<QgsDiagramLayerSettings::Property> mDiagramProperties;
    QList<QgsCallout::Property> mCalloutProperties;

    friend class TestQgsMapToolLabel;
};

#endif // QGSMAPTOOLLABEL_H
