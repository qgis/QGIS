/***************************************************************************
 qgsquickattributeformmodel.h
  --------------------------------------
  Date                 : 22.9.2016
  Copyright            : (C) 2016 by Matthias Kuhn
  Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSQUICKATTRIBUTEFORMMODEL_H
#define QGSQUICKATTRIBUTEFORMMODEL_H

#include <QSortFilterProxyModel>

#include "qgis_quick.h"

class QgsQuickAttributeFormModelBase;
class QgsQuickFeatureModel;

/**
 * \ingroup quick
 * This is a model implementation for attribute form of a feature from a vector layer.
 *
 * The model is based on vector layer's edit form config (QgsEditFormConfig). It supports
 * auto-generated editor layouts and "tab" layout (layout defined with groups and tabs).
 * The form layout gets flattened into a list, each row has a bunch of roles with values
 * extracted from the edit form config.
 *
 * It also adds filtering of attribute (attributes may be visible or hidden based on expressions).
 *
 * \note QML Type: AttributeFormModel
 *
 * \since QGIS 3.2
 */
class QUICK_EXPORT QgsQuickAttributeFormModel : public QSortFilterProxyModel
{
    Q_OBJECT

    //! Feature model with attributes
    Q_PROPERTY( QgsQuickFeatureModel *featureModel READ featureModel WRITE setFeatureModel NOTIFY featureModelChanged )
    //! Whether use tabs layout
    Q_PROPERTY( bool hasTabs READ hasTabs WRITE setHasTabs NOTIFY hasTabsChanged )

    //! Returns true if all constraints defined on fields are satisfied with the current attribute values
    Q_PROPERTY( bool constraintsValid READ constraintsValid NOTIFY constraintsValidChanged )

  public:

    //! Feature fields's roles
    enum FeatureRoles
    {
      ElementType = Qt::UserRole + 1, //!< Element type
      Name, //!< Field Name
      AttributeValue, //!< Field Value
      AttributeEditable,  //!< Field editable
      EditorWidget, //!< Widget
      EditorWidgetConfig, //!< Config
      RememberValue, //!< Remember value
      Field, //!< Field
      FieldIndex, //!< Index
      Group, //!< Group
      AttributeEditorElement, //!< Attribute editor element
      CurrentlyVisible, //!< Field visible
      ConstraintValid, //!< Contraint valid
      ConstraintDescription //!< Contraint description
    };

    Q_ENUM( FeatureRoles )

    //! Create new attribute form model
    QgsQuickAttributeFormModel( QObject *parent = nullptr );

    //! Return whether model has tabs layout
    bool hasTabs() const;

    //! Set tabs layout
    void setHasTabs( bool hasTabs );

    //! Return feature model associated
    QgsQuickFeatureModel *featureModel() const;

    //! Set feature model
    void setFeatureModel( QgsQuickFeatureModel *featureModel );

    //! Whether all constraints are valid for feature model
    bool constraintsValid() const;

    //! Update QgsFeature based on changes
    Q_INVOKABLE void save();

    //! Create new QgsFeature
    Q_INVOKABLE void create();

    //! Return attribute value with name
    Q_INVOKABLE QVariant attribute( const QString &name ) const;

  signals:
    //! feature model changed
    void featureModelChanged();

    //! has tabs changed
    void hasTabsChanged();

    //! feature changed
    void featureChanged();

    //! constraints valid changed
    void constraintsValidChanged();

  protected:
    virtual bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const override;

  private:
    QgsQuickAttributeFormModelBase *mSourceModel;
};

#endif // QGSQUICKATTRIBUTEFORMMODEL_H
