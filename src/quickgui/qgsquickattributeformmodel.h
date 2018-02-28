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

    Q_PROPERTY( QgsQuickFeatureModel *featureModel READ featureModel WRITE setFeatureModel NOTIFY featureModelChanged )
    Q_PROPERTY( bool hasTabs READ hasTabs WRITE setHasTabs NOTIFY hasTabsChanged )
    Q_PROPERTY( bool constraintsValid READ constraintsValid NOTIFY constraintsValidChanged )

  public:
    enum FeatureRoles
    {
      ElementType = Qt::UserRole + 1,
      Name,
      AttributeValue,
      AttributeEditable,
      EditorWidget,
      EditorWidgetConfig,
      RememberValue,
      Field,
      FieldIndex,
      Group,
      AttributeEditorElement,
      CurrentlyVisible,
      ConstraintValid,
      ConstraintDescription
    };

    Q_ENUM( FeatureRoles )

    QgsQuickAttributeFormModel( QObject *parent = nullptr );

    bool hasTabs() const;
    void setHasTabs( bool hasTabs );

    QgsQuickFeatureModel *featureModel() const;
    void setFeatureModel( QgsQuickFeatureModel *featureModel );

    bool constraintsValid() const;

    Q_INVOKABLE void save();
    Q_INVOKABLE void create();
    Q_INVOKABLE QVariant attribute( const QString &name );

  signals:
    void featureModelChanged();
    void hasTabsChanged();
    void featureChanged();
    void constraintsValidChanged();

  protected:
    virtual bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const override;

  private:
    QgsQuickAttributeFormModelBase *mSourceModel;
};

#endif // QGSQUICKATTRIBUTEFORMMODEL_H
