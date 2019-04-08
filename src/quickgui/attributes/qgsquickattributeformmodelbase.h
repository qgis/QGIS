/***************************************************************************
 qgsquickattributemodelbase.h
  --------------------------------------
  Date                 : 16.8.2016
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

#ifndef QGSQUICKATTRIBUTEFORMMODELBASE_H
#define QGSQUICKATTRIBUTEFORMMODELBASE_H

/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include <QStandardItemModel>
#include <QStack>

#include "qgseditformconfig.h"
#include "qgsexpressioncontext.h"

#include "qgis_quick.h"
#include "qgsquickattributemodel.h"

class QVariant;

/**
 * \ingroup quick
 * This is an internal (implementation) class used as the source model for QgsQuickAttributeFormModel.
 *
 * \sa QgsQuickAttributeFormModel
 *
 * \since QGIS 3.4
 */
class QgsQuickAttributeFormModelBase : public QStandardItemModel
{
    Q_OBJECT

    //! Feature model with attributes
    Q_PROPERTY( QgsQuickAttributeModel *attributeModel READ attributeModel WRITE setAttributeModel NOTIFY attributeModelChanged )

    //! Whether use tabs layout
    Q_PROPERTY( bool hasTabs READ hasTabs WRITE setHasTabs NOTIFY hasTabsChanged )

    //! Returns TRUE if all constraints defined on fields are satisfied with the current attribute values
    Q_PROPERTY( bool constraintsValid READ constraintsValid NOTIFY constraintsValidChanged )

  public:
    //! Constructor
    explicit QgsQuickAttributeFormModelBase( QObject *parent = nullptr );
    //! Destructor
    ~QgsQuickAttributeFormModelBase() = default;
    QHash<int, QByteArray> roleNames() const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;

    //! \copydoc QgsQuickAttributeFormModelBase::attributeModel
    QgsQuickAttributeModel *attributeModel() const;
    //! \copydoc QgsQuickAttributeFormModelBase::attributeModel
    void setAttributeModel( QgsQuickAttributeModel *attributeModel );

    //! \copydoc QgsQuickAttributeFormModelBase::hasTabs
    bool hasTabs() const;
    //! \copydoc QgsQuickAttributeFormModelBase::hasTabs
    void setHasTabs( bool hasTabs );

    //! Saves changes
    void save();

    //! Creates a new feature
    void create();

    //! \copydoc QgsQuickAttributeFormModelBase::constraintsValid
    bool constraintsValid() const;

    /**
     * Gets the value of attribute of the feature in the model
     *
     * \param name QString name of the wanted attribute
     */
    QVariant attribute( const QString &name ) const;

  signals:
    //! \copydoc QgsQuickAttributeFormModelBase::attributeModel
    void attributeModelChanged();
    //! \copydoc QgsQuickAttributeFormModelBase::hasTabs
    void hasTabsChanged();
    //! \copydoc QgsQuickAttributeFormModelBase::constraintsValid
    void constraintsValidChanged();

  private slots:
    void onFeatureChanged();
    void onLayerChanged();

  private:

    QgsAttributeEditorContainer *generateRootContainer() const;  //#spellok
    QgsAttributeEditorContainer *invisibleRootContainer() const;
    void updateAttributeValue( QStandardItem *item );
    void flatten( QgsAttributeEditorContainer *container, QStandardItem *parent, const QString &parentVisibilityExpressions, QVector<QStandardItem *> &items );
    void updateVisibility( int fieldIndex = -1 );
    void setConstraintsValid( bool constraintsValid );

    QgsQuickAttributeModel *mAttributeModel = nullptr; // not owned
    QgsVectorLayer *mLayer = nullptr; // not owned
    std::unique_ptr<QgsAttributeEditorContainer> mTemporaryContainer;
    bool mHasTabs = false;

    typedef QPair<QgsExpression, QVector<QStandardItem *> > VisibilityExpression;
    QList<VisibilityExpression> mVisibilityExpressions;
    QMap<QStandardItem *, QgsExpression> mConstraints;

    QgsExpressionContext mExpressionContext;
    bool mConstraintsValid = false;
};

/// @endcond

#endif // QGSQUICKATTRIBUTEFORMMODELBASE_H
