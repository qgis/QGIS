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
class QgsQuickAttributeModel;
class QVariant;

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
 * \since QGIS 3.4
 */
class QUICK_EXPORT QgsQuickAttributeFormModel : public QSortFilterProxyModel
{
    Q_OBJECT

    //! Feature model with attributes
    Q_PROPERTY( QgsQuickAttributeModel *attributeModel READ attributeModel WRITE setAttributeModel NOTIFY attributeModelChanged )

    //! Whether use tabs layout
    Q_PROPERTY( bool hasTabs READ hasTabs WRITE setHasTabs NOTIFY hasTabsChanged )

    //! Returns true if all constraints defined on fields are satisfied with the current attribute values
    Q_PROPERTY( bool constraintsValid READ constraintsValid NOTIFY constraintsValidChanged )

  public:

    //! Feature fields's roles
    enum FeatureFieldRoles
    {
      ElementType = Qt::UserRole + 1, //!< User role used to identify either "field" or "container" type of item
      Name, //!< Field Name
      Alias, //!< Field Alias
      AttributeValue, //!< Field Value
      AttributeEditable,  //!< Whether is field editable
      EditorWidget, //!< Widget type to represent the data (text field, value map, ...)
      EditorWidgetConfig, //!< Widget configuration
      RememberValue, //!< Remember value (default value for next feature)
      Field, //!< Field
      FieldIndex, //!< Index
      Group, //!< Group
      AttributeEditorElement, //!< Attribute editor element
      CurrentlyVisible, //!< Field visible
      ConstraintValid, //!< Contraint valid
      ConstraintDescription //!< Contraint description
    };

    Q_ENUM( FeatureFieldRoles )

    //! Create new attribute form model
    QgsQuickAttributeFormModel( QObject *parent = nullptr );

    //! \copydoc QgsQuickAttributeFormModel::hasTabs
    bool hasTabs() const;

    //! \copydoc QgsQuickAttributeFormModel::hasTabs
    void setHasTabs( bool hasTabs );

    //! \copydoc QgsQuickAttributeFormModel::attributeModel
    QgsQuickAttributeModel *attributeModel() const;

    //! \copydoc QgsQuickAttributeFormModel::attributeModel
    void setAttributeModel( QgsQuickAttributeModel *attributeModel );

    //! \copydoc QgsQuickAttributeFormModel::constraintsValid
    bool constraintsValid() const;

    //! Updates QgsFeature based on changes
    Q_INVOKABLE void save();

    //! Creates new QgsFeature
    Q_INVOKABLE void create();

    //! Returns attribute value with name
    Q_INVOKABLE QVariant attribute( const QString &name ) const;

  signals:
    //! \copydoc QgsQuickAttributeFormModel::attributeModel
    void attributeModelChanged();

    //! \copydoc QgsQuickAttributeFormModel::hasTabs
    void hasTabsChanged();

    //! \copydoc QgsQuickAttributeFormModel::constraintsValid
    void constraintsValidChanged();

  protected:
    virtual bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const override;

  private:
    QgsQuickAttributeFormModelBase *mSourceModel = nullptr; //not owned
};

#endif // QGSQUICKATTRIBUTEFORMMODEL_H
