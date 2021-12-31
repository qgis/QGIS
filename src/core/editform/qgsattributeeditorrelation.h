/***************************************************************************
  qgsattributeeditorrelation.h - QgsAttributeEditorElement

 ---------------------
 begin                : 12.01.2021
 copyright            : (C) 2021 by Denis Rouzaud
 email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSATTRIBUTEEDITORRELATION_H
#define QGSATTRIBUTEEDITORRELATION_H

#include "qgis_core.h"
#include "qgsattributeeditorelement.h"
#include "qgsrelation.h"
#include "qgsoptionalexpression.h"
#include "qgspropertycollection.h"
#include <QColor>

class QgsRelationManager;

/**
 * \ingroup core
 * \brief This element will load a relation editor onto the form.
 */
class CORE_EXPORT QgsAttributeEditorRelation : public QgsAttributeEditorElement
{
    Q_GADGET
  public:

    /**
       * Possible buttons shown in the relation editor
       * \deprecated since QGIS 3.18 use QgsRelationEditorWidget::Button instead
       * \since QGIS 3.16
       */
    enum Button
    {
      Link = 1 << 1, //!< Link button
      Unlink = 1 << 2, //!< Unlink button
      SaveChildEdits = 1 << 3, //!< Save child edits button
      AddChildFeature = 1 << 4, //!< Add child feature (as in some projects we only want to allow linking/unlinking existing features)
      DuplicateChildFeature = 1 << 5, //!< Duplicate child feature
      DeleteChildFeature = 1 << 6, //!< Delete child feature button
      ZoomToChildFeature = 1 << 7, //!< Zoom to child feature
      AllButtons = Link | Unlink | SaveChildEdits | AddChildFeature | DuplicateChildFeature | DeleteChildFeature | ZoomToChildFeature //!< All buttons
    };
    // TODO QGIS 4: remove
    // this could not be tagged with Q_DECL_DEPRECATED due to Doxygen warning

    Q_ENUM( Button )
    Q_DECLARE_FLAGS( Buttons, Button )
    Q_FLAG( Buttons )

    /**
     * \deprecated since QGIS 3.0.2. The name parameter is not used for anything and overwritten by the relationId internally.
     */
    Q_DECL_DEPRECATED QgsAttributeEditorRelation( const QString &name, const QString &relationId, QgsAttributeEditorElement *parent )
      : QgsAttributeEditorElement( AeTypeRelation, name, parent )
      , mRelationId( relationId )
    {}

    /**
     * \deprecated since QGIS 3.0.2. The name parameter is not used for anything and overwritten by the relationId internally.
     */
    Q_DECL_DEPRECATED QgsAttributeEditorRelation( const QString &name, const QgsRelation &relation, QgsAttributeEditorElement *parent )
      : QgsAttributeEditorElement( AeTypeRelation, name, parent )
      , mRelationId( relation.id() )
      , mRelation( relation )
    {}

    /**
     * Creates a new element which embeds a relation.
     *
     * \param relationId   The id of the relation to embed
     * \param parent       The parent (used as container)
     */
    QgsAttributeEditorRelation( const QString &relationId, QgsAttributeEditorElement *parent )
      : QgsAttributeEditorElement( AeTypeRelation, relationId, parent )
      , mRelationId( relationId )
    {}

    /**
     * Creates a new element which embeds a relation.
     *
     * \param relation     The relation to embed
     * \param parent       The parent (used as container)
     */
    QgsAttributeEditorRelation( const QgsRelation &relation, QgsAttributeEditorElement *parent )
      : QgsAttributeEditorElement( AeTypeRelation, relation.id(), parent )
      , mRelationId( relation.id() )
      , mRelation( relation )
    {}


    /**
     * Gets the id of the relation which shall be embedded
     *
     * \returns the id
     */
    const QgsRelation &relation() const { return mRelation; }

    /**
     * Initializes the relation from the id
     *
     * \param relManager The relation manager to use for the initialization
     * \returns TRUE if the relation was found in the relationmanager
     */
    bool init( QgsRelationManager *relManager );

    QgsAttributeEditorElement *clone( QgsAttributeEditorElement *parent ) const override SIP_FACTORY;

    /**
     * Determines the force suppress form popup status.
     * \since QGIS 3.16
     */
    bool forceSuppressFormPopup() const;

    /**
     * Sets force suppress form popup status to \a forceSuppressFormPopup.
     * This flag is to override the layer and general settings regarding the automatic
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
     */
    QString label() const;

    /**
     * Sets \a label for this element
     * If it's empty it takes the relation id as label
     * \since QGIS 3.16
     */
    void setLabel( const QString &label = QString() );

    /**
     * Returns the current relation widget type id
     * \since QGIS 3.18
     */
    QString relationWidgetTypeId() const;

    /**
     * Sets the relation widget type
     * \since QGIS 3.18
     */
    void setRelationWidgetTypeId( const QString &relationWidgetTypeId );

    /**
     * Returns the relation editor widget configuration
     *
     * \since QGIS 3.18
     */
    QVariantMap relationEditorConfiguration() const;

    /**
     * Sets the relation editor configuration
     *
     * \since QGIS 3.18
     */
    void setRelationEditorConfiguration( const QVariantMap &config );

  private:
    void saveConfiguration( QDomElement &elem, QDomDocument &doc ) const override;
    void loadConfiguration( const QDomElement &element, const QString &layerId, const QgsReadWriteContext &context, const QgsFields &fields ) override;
    QString typeIdentifier() const override;
    QString mRelationId;
    QgsRelation mRelation;
    Q_NOWARN_DEPRECATED_PUSH
    Buttons mButtons = Buttons( Button::AllButtons );
    Q_NOWARN_DEPRECATED_POP
    bool mForceSuppressFormPopup = false;
    QVariant mNmRelationId;
    QString mLabel;
    QString mRelationWidgetTypeId;
    QVariantMap mRelationEditorConfig;
};

Q_NOWARN_DEPRECATED_PUSH
Q_DECLARE_OPERATORS_FOR_FLAGS( QgsAttributeEditorRelation::Buttons )
Q_NOWARN_DEPRECATED_POP

#endif // QGSATTRIBUTEEDITORRELATION_H
