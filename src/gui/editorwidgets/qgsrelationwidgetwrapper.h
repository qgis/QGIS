/***************************************************************************
    qgsrelationwidgetwrapper.h
     --------------------------------------
    Date                 : 14.5.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRELATIONWIDGETWRAPPER_H
#define QGSRELATIONWIDGETWRAPPER_H

#include "qgswidgetwrapper.h"
#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgsattributeeditorrelation.h"


class QgsAbstractRelationEditorWidget;

/**
 * \ingroup gui
 * \class QgsRelationWidgetWrapper
 */

class GUI_EXPORT QgsRelationWidgetWrapper : public QgsWidgetWrapper
{
    Q_OBJECT

  public:

    //! Constructor for QgsRelationWidgetWrapper
    QgsRelationWidgetWrapper(
      QgsVectorLayer *vl,
      const QgsRelation &relation,
      QWidget *editor SIP_CONSTRAINED = nullptr,
      QWidget *parent SIP_TRANSFERTHIS SIP_CONSTRAINED  = nullptr
    );

    //! Constructor for QgsRelationWidgetWrapper
    QgsRelationWidgetWrapper(
      const QString &relationEditorName,
      QgsVectorLayer *vl,
      const QgsRelation &relation,
      QWidget *editor = nullptr,
      QWidget *parent SIP_TRANSFERTHIS = nullptr
    );

    /**
     * Defines if a title label should be shown for this widget.
     * Only has an effect after widget() has been called at least once.
     *
     * \since QGIS 2.18
     * \deprecated since QGIS 3.20 label is handled directly in QgsAttributeForm.
     */
    Q_DECL_DEPRECATED bool showLabel() const SIP_DEPRECATED;

    /**
     * Defines if a title label should be shown for this widget.
     * Only has an effect after widget() has been called at least once.
     *
     * \since QGIS 2.18
     * \deprecated since QGIS 3.20 label is handled directly in QgsAttributeForm.
     */
    Q_DECL_DEPRECATED void setShowLabel( bool showLabel ) SIP_DEPRECATED;

    /**
     * Determines if the "link feature" button should be shown
     * \since QGIS 2.18
     * \deprecated since QGIS 3.16 use visibleButtons() instead
     */
    Q_DECL_DEPRECATED bool showLinkButton() const SIP_DEPRECATED;

    /**
     * Determines if the "link feature" button should be shown
     * \since QGIS 2.18
     * \deprecated since QGIS 3.16 use setVisibleButtons() instead
     */
    Q_DECL_DEPRECATED void setShowLinkButton( bool showLinkButton ) SIP_DEPRECATED;

    /**
     * Determines if the "unlink feature" button should be shown
     * \since QGIS 2.18
     * \deprecated since QGIS 3.16 use visibleButtons() instead
     */
    Q_DECL_DEPRECATED bool showUnlinkButton() const SIP_DEPRECATED;

    /**
     * Determines if the "unlink feature" button should be shown
     * \since QGIS 2.18
     * \deprecated since QGIS 3.16 use setVisibleButtons() instead
     */
    Q_DECL_DEPRECATED void setShowUnlinkButton( bool showUnlinkButton ) SIP_DEPRECATED;

    /**
     * Determines if the "Save child layer edits" button should be shown
     * \since QGIS 3.14
     * \deprecated since QGIS 3.16 use setVisibleButtons() instead
     */
    Q_DECL_DEPRECATED void setShowSaveChildEditsButton( bool showChildEdits ) SIP_DEPRECATED;

    /**
     * Determines if the "Save child layer edits" button should be shown
     * \since QGIS 3.14
     * \deprecated since QGIS 3.16 use visibleButtons() instead
     */
    Q_DECL_DEPRECATED bool showSaveChildEditsButton() const SIP_DEPRECATED;

    /**
     * Defines the buttons which are shown
     * \since QGIS 3.16
     * \deprecated since QGIS 3.18 use setWidgetConfig() instead
     */
    Q_DECL_DEPRECATED void setVisibleButtons( const QgsAttributeEditorRelation::Buttons &buttons ) SIP_DEPRECATED;

    /**
     * Returns the buttons which are shown
     * \since QGIS 3.16
     * \deprecated since QGIS 3.18 use widgetConfig() instead
     */
    Q_DECL_DEPRECATED QgsAttributeEditorRelation::Buttons visibleButtons() const SIP_DEPRECATED;


    /**
     * Will set the config of this widget wrapper to the specified config.
     *
     * \param config The config for this wrapper
     * \since QGIS 3.18
     */
    void setWidgetConfig( const QVariantMap &config );

    /**
     * Returns the whole widget config
     * \since QGIS 3.18
     */
    QVariantMap widgetConfig() const;

    /**
     * Determines the force suppress form popup status that is configured for this widget
     * \since QGIS 3.16
     */
    bool forceSuppressFormPopup() const;

    /**
     * Sets force suppress form popup status to \a forceSuppressFormPopup for this widget
     * and for the vectorLayerTools (if TRUE).
     * This flag will override the layer and general settings regarding the automatic
     * opening of the attribute form dialog when digitizing is completed.
     * \since QGIS 3.16
     */
    void setForceSuppressFormPopup( bool forceSuppressFormPopup );

    /**
     * Determines the relation id of the second relation involved in an N:M relation.
    * \since QGIS 3.16
    */
    QVariant nmRelationId() const;

    /**
     * Sets \a nmRelationId for the relation id of the second relation involved in an N:M relation.
     * If it's empty, then it's considered as a 1:M relationship.
     * \since QGIS 3.16
     */
    void setNmRelationId( const QVariant &nmRelationId = QVariant() );

    /**
     * Determines the label of this element
     * \since QGIS 3.16
     * \deprecated since QGIS 3.20 label is handled directly in QgsAttributeForm.
     */
    Q_DECL_DEPRECATED QString label() const SIP_DEPRECATED;

    /**
     * Sets \a label for this element
     * If it's empty it takes the relation id as label
     * \since QGIS 3.16
     * \deprecated since QGIS 3.20 label is handled directly in QgsAttributeForm.
     */
    Q_DECL_DEPRECATED void setLabel( const QString &label = QString() ) SIP_DEPRECATED;

    /**
     * The relation for which this wrapper is created.
     *
     * \since QGIS 3.0
     */
    QgsRelation relation() const;

    /**
     * Will be called when a value in the current edited form or table row
     * changes
     *
     * Forward the signal to the embedded form
     *
     * \param attribute The name of the attribute that changed.
     * \param newValue     The new value of the attribute.
     * \param attributeChanged If TRUE, it corresponds to an actual change of the feature attribute
     * \since QGIS 3.14
     */
    void widgetValueChanged( const QString &attribute, const QVariant &newValue, bool attributeChanged );


  protected:
    QWidget *createWidget( QWidget *parent ) override;
    void initWidget( QWidget *editor ) override;
    bool valid() const override;

  signals:

    /**
     * Emit this signal, whenever the related features changed.
     * This happens for example when related features are added, removed,
     * linked or unlinked.
     *
     * \since QGIS 3.22
     */
    void relatedFeaturesChanged();

  public slots:
    void setFeature( const QgsFeature &feature ) override;

    /**
     * Set multiple feature to edit simultaneously.
     * \param fids Multiple Id of features to edit
     * \since QGIS 3.24
     */
    void setMultiEditFeatureIds( const QgsFeatureIds &fids );

    /**
     * Sets the visibility of the wrapper's widget.
     * \param visible set to TRUE to show widget, FALSE to hide widget
     * \since QGIS 2.16
     */
    void setVisible( bool visible );

  private:
    void aboutToSave() override;
    QgsRelation mRelation;
    QgsRelation mNmRelation;
    QString mRelationEditorId;
    QgsAbstractRelationEditorWidget *mWidget = nullptr;
};

#endif // QGSRELATIONWIDGETWRAPPER_H
