/***************************************************************************
 qgsquickattributemodelbase.cpp
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

#include "qgseditorwidgetsetup.h"
#include "qgsvectorlayer.h"

#include "qgsquickattributeformmodelbase.h"
#include "qgsquickattributeformmodel.h"

/// @cond PRIVATE

QgsQuickAttributeFormModelBase::QgsQuickAttributeFormModelBase( QObject *parent )
  : QStandardItemModel( 0, 1, parent )
{
}


QHash<int, QByteArray> QgsQuickAttributeFormModelBase::roleNames() const
{
  QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();

  roles[QgsQuickAttributeFormModel::ElementType]  = QByteArray( "Type" );
  roles[QgsQuickAttributeFormModel::Name]  = QByteArray( "Name" );
  roles[QgsQuickAttributeFormModel::AttributeValue] = QByteArray( "AttributeValue" );
  roles[QgsQuickAttributeFormModel::AttributeEditable] = QByteArray( "AttributeEditable" );
  roles[QgsQuickAttributeFormModel::EditorWidget] = QByteArray( "EditorWidget" );
  roles[QgsQuickAttributeFormModel::EditorWidgetConfig] = QByteArray( "EditorWidgetConfig" );
  roles[QgsQuickAttributeFormModel::RememberValue] = QByteArray( "RememberValue" );
  roles[QgsQuickAttributeFormModel::Field] = QByteArray( "Field" );
  roles[QgsQuickAttributeFormModel::Group] = QByteArray( "Group" );
  roles[QgsQuickAttributeFormModel::ConstraintValid] = QByteArray( "ConstraintValid" );
  roles[QgsQuickAttributeFormModel::ConstraintDescription] = QByteArray( "ConstraintDescription" );

  return roles;
}

bool QgsQuickAttributeFormModelBase::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( data( index, role ) != value )
  {
    switch ( role )
    {
      case QgsQuickAttributeFormModel::RememberValue:
      {
        QStandardItem *item = itemFromIndex( index );
        int fieldIndex = item->data( QgsQuickAttributeFormModel::FieldIndex ).toInt();
        mAttributeModel->setData( mAttributeModel->index( fieldIndex ), value, QgsQuickAttributeModel::RememberAttribute );
        item->setData( value, QgsQuickAttributeFormModel::RememberValue );
        break;
      }

      case QgsQuickAttributeFormModel::AttributeValue:
      {
        QStandardItem *item = itemFromIndex( index );
        int fieldIndex = item->data( QgsQuickAttributeFormModel::FieldIndex ).toInt();
        bool changed = mAttributeModel->setData( mAttributeModel->index( fieldIndex ), value, QgsQuickAttributeModel::AttributeValue );
        if ( changed )
        {
          item->setData( value, QgsQuickAttributeFormModel::AttributeValue );
          emit dataChanged( index, index, QVector<int>() << role );
        }
        updateVisibility( fieldIndex );
        return changed;
        break;
      }
    }
  }
  return false;
}

QgsQuickAttributeModel *QgsQuickAttributeFormModelBase::attributeModel() const
{
  return mAttributeModel;
}

void QgsQuickAttributeFormModelBase::setAttributeModel( QgsQuickAttributeModel *attributeModel )
{
  if ( mAttributeModel == attributeModel )
    return;

  if ( mAttributeModel )
  {
    disconnect( mAttributeModel, &QgsQuickAttributeModel::layerChanged, this, &QgsQuickAttributeFormModelBase::onLayerChanged );
    disconnect( mAttributeModel, &QgsQuickAttributeModel::featureChanged, this, &QgsQuickAttributeFormModelBase::onFeatureChanged );
    disconnect( mAttributeModel, &QgsQuickAttributeModel::modelReset, this, &QgsQuickAttributeFormModelBase::onFeatureChanged );
  }

  mAttributeModel = attributeModel;

  if ( mAttributeModel )
  {
    connect( mAttributeModel, &QgsQuickAttributeModel::layerChanged, this, &QgsQuickAttributeFormModelBase::onLayerChanged );
    connect( mAttributeModel, &QgsQuickAttributeModel::featureChanged, this, &QgsQuickAttributeFormModelBase::onFeatureChanged );
    connect( mAttributeModel, &QgsQuickAttributeModel::modelReset, this, &QgsQuickAttributeFormModelBase::onFeatureChanged );
  }

  emit attributeModelChanged();
}

void QgsQuickAttributeFormModelBase::onLayerChanged()
{
  clear();

  mLayer = mAttributeModel->featureLayerPair().layer();
  mVisibilityExpressions.clear();
  mConstraints.clear();

  if ( mLayer )
  {
    QgsAttributeEditorContainer *root = nullptr;
    mTemporaryContainer = nullptr;

    if ( mLayer->editFormConfig().layout() == QgsEditFormConfig::TabLayout )
    {
      root = mLayer->editFormConfig().invisibleRootContainer();
    }
    else
    {
      root = generateRootContainer();  //#spellok
      mTemporaryContainer.reset( root );
    }

    setHasTabs( !root->children().isEmpty() && QgsAttributeEditorElement::AeTypeContainer == root->children().first()->type() );

    invisibleRootItem()->setColumnCount( 1 );
    if ( mHasTabs )
    {
      for ( QgsAttributeEditorElement *element : root->children() )
      {
        if ( element->type() == QgsAttributeEditorElement::AeTypeContainer )
        {
          QgsAttributeEditorContainer *container = static_cast<QgsAttributeEditorContainer *>( element );

          QStandardItem *item = new QStandardItem();
          item->setData( element->name(), QgsQuickAttributeFormModel::Name );
          item->setData( QStringLiteral( "container" ), QgsQuickAttributeFormModel::ElementType );
          item->setData( true, QgsQuickAttributeFormModel::CurrentlyVisible );
          invisibleRootItem()->appendRow( item );

          if ( container->visibilityExpression().enabled() )
          {
            mVisibilityExpressions.append( qMakePair( container->visibilityExpression().data(), QVector<QStandardItem *>() << item ) );
          }

          QVector<QStandardItem *> dummy;
          flatten( container, item, QString(), dummy );
        }
      }
    }
    else
    {
      QVector<QStandardItem *> dummy;
      flatten( invisibleRootContainer(), invisibleRootItem(), QString(), dummy );
    }

    mExpressionContext = mLayer->createExpressionContext();
  }
}

void QgsQuickAttributeFormModelBase::onFeatureChanged()
{
  for ( int i = 0 ; i < invisibleRootItem()->rowCount(); ++i )
  {
    updateAttributeValue( invisibleRootItem()->child( i ) );
  }

  updateVisibility();
}

QgsAttributeEditorContainer *QgsQuickAttributeFormModelBase::generateRootContainer() const  //#spellok
{
  QgsAttributeEditorContainer *root = new QgsAttributeEditorContainer( QString(), nullptr );
  QgsFields fields = mLayer->fields();
  for ( int i = 0; i < fields.size(); ++i )
  {
    if ( fields.at( i ).editorWidgetSetup().type() != QStringLiteral( "Hidden" ) )
    {
      QgsAttributeEditorField *field = new QgsAttributeEditorField( fields.at( i ).name(), i, root );
      root->addChildElement( field );
    }
  }
  return root;
}

QgsAttributeEditorContainer *QgsQuickAttributeFormModelBase::invisibleRootContainer() const
{
  return mTemporaryContainer ? mTemporaryContainer.get() : mLayer->editFormConfig().invisibleRootContainer();
}

void QgsQuickAttributeFormModelBase::updateAttributeValue( QStandardItem *item )
{
  if ( item->data( QgsQuickAttributeFormModel::ElementType ) == QStringLiteral( "field" ) )
  {
    item->setData( mAttributeModel->featureLayerPair().feature().attribute( item->data( QgsQuickAttributeFormModel::FieldIndex ).toInt() ), QgsQuickAttributeFormModel::AttributeValue );
  }
  else
  {
    for ( int i = 0; i < item->rowCount(); ++i )
    {
      updateAttributeValue( item->child( i ) );
    }
  }
}

void QgsQuickAttributeFormModelBase::flatten( QgsAttributeEditorContainer *container, QStandardItem *parent, const QString &parentVisibilityExpressions, QVector<QStandardItem *> &items )
{
  for ( QgsAttributeEditorElement *element : container->children() )
  {
    switch ( element->type() )
    {
      case QgsAttributeEditorElement::AeTypeContainer:
      {
        QString visibilityExpression = parentVisibilityExpressions;
        QgsAttributeEditorContainer *container = static_cast<QgsAttributeEditorContainer *>( element );
        if ( container->visibilityExpression().enabled() )
        {
          if ( visibilityExpression.isNull() )
            visibilityExpression = container->visibilityExpression().data().expression();
          else
            visibilityExpression += " AND " + container->visibilityExpression().data().expression();
        }

        QVector<QStandardItem *> newItems;
        flatten( container, parent, visibilityExpression, newItems );
        if ( !visibilityExpression.isEmpty() )
          mVisibilityExpressions.append( qMakePair( QgsExpression( visibilityExpression ), newItems ) );
        break;
      }

      case QgsAttributeEditorElement::AeTypeField:
      {
        QgsAttributeEditorField *editorField = static_cast<QgsAttributeEditorField *>( element );
        int fieldIndex = editorField->idx();
        if ( fieldIndex < 0 || fieldIndex >= mLayer->fields().size() )
          continue;

        QgsField field = mLayer->fields().at( fieldIndex );

        QStandardItem *item = new QStandardItem();
        item->setData( mLayer->attributeDisplayName( fieldIndex ), QgsQuickAttributeFormModel::Name );
        item->setData( mAttributeModel->featureLayerPair().feature().attribute( fieldIndex ), QgsQuickAttributeFormModel::AttributeValue );
        item->setData( !mLayer->editFormConfig().readOnly( fieldIndex ), QgsQuickAttributeFormModel::AttributeEditable );
        QgsEditorWidgetSetup setup = mLayer->editorWidgetSetup( fieldIndex );
        item->setData( setup.type(), QgsQuickAttributeFormModel::EditorWidget );
        item->setData( setup.config(), QgsQuickAttributeFormModel::EditorWidgetConfig );
        item->setData( mAttributeModel->rememberedAttributes().at( fieldIndex ) ? Qt::Checked : Qt::Unchecked, QgsQuickAttributeFormModel::RememberValue );
        item->setData( mLayer->fields().at( fieldIndex ), QgsQuickAttributeFormModel::Field );
        item->setData( QStringLiteral( "field" ), QgsQuickAttributeFormModel::ElementType );
        item->setData( fieldIndex, QgsQuickAttributeFormModel::FieldIndex );
        item->setData( container->isGroupBox() ? container->name() : QString(), QgsQuickAttributeFormModel::Group );
        item->setData( true, QgsQuickAttributeFormModel::CurrentlyVisible );
        item->setData( true, QgsQuickAttributeFormModel::ConstraintValid );
        item->setData( field.constraints().constraintDescription(), QgsQuickAttributeFormModel::ConstraintDescription );

        if ( !field.constraints().constraintExpression().isEmpty() )
        {
          mConstraints.insert( item, field.constraints().constraintExpression() );
        }

        items.append( item );

        parent->appendRow( item );
        break;
      }

      case QgsAttributeEditorElement::AeTypeRelation:
        // todo
        break;

      case QgsAttributeEditorElement::AeTypeInvalid:
        // todo
        break;

      case QgsAttributeEditorElement::AeTypeQmlElement:
        // todo
        break;

      case QgsAttributeEditorElement::AeTypeHtmlElement:
        // todo
        break;
    }
  }
}

void QgsQuickAttributeFormModelBase::updateVisibility( int fieldIndex )
{
  QgsFields fields = mAttributeModel->featureLayerPair().feature().fields();
  mExpressionContext.setFields( fields );
  mExpressionContext.setFeature( mAttributeModel->featureLayerPair().feature() );

  for ( const VisibilityExpression &it : mVisibilityExpressions )
  {
    if ( fieldIndex == -1 || it.first.referencedAttributeIndexes( fields ).contains( fieldIndex ) )
    {
      QgsExpression exp = it.first;
      exp.prepare( &mExpressionContext );

      bool visible = exp.evaluate( &mExpressionContext ).toInt();
      for ( QStandardItem *item : it.second )
      {
        if ( item->data( QgsQuickAttributeFormModel::CurrentlyVisible ).toBool() != visible )
        {
          item->setData( visible, QgsQuickAttributeFormModel::CurrentlyVisible );
        }
      }
    }
  }

  bool allConstraintsValid = true;
  QMap<QStandardItem *, QgsExpression>::ConstIterator constraintIterator( mConstraints.constBegin() );
  for ( ; constraintIterator != mConstraints.constEnd(); ++constraintIterator )
  {
    QStandardItem *item = constraintIterator.key();
    QgsExpression exp = constraintIterator.value();
    exp.prepare( &mExpressionContext );
    bool constraintSatisfied = exp.evaluate( &mExpressionContext ).toBool();

    if ( constraintSatisfied != item->data( QgsQuickAttributeFormModel::ConstraintValid ).toBool() )
    {
      item->setData( constraintSatisfied, QgsQuickAttributeFormModel::ConstraintValid );
    }

    if ( !item->data( QgsQuickAttributeFormModel::ConstraintValid ).toBool() )
    {
      allConstraintsValid = false;
    }
  }

  setConstraintsValid( allConstraintsValid );
}

bool QgsQuickAttributeFormModelBase::constraintsValid() const
{
  return mConstraintsValid;
}

QVariant QgsQuickAttributeFormModelBase::attribute( const QString &name ) const
{
  if ( !mLayer )
    return QVariant();

  int idx = mLayer->fields().indexOf( name );
  return mAttributeModel->featureLayerPair().feature().attribute( idx );
}

void QgsQuickAttributeFormModelBase::setConstraintsValid( bool constraintsValid )
{
  if ( constraintsValid == mConstraintsValid )
    return;

  mConstraintsValid = constraintsValid;
  emit constraintsValidChanged();
}

bool QgsQuickAttributeFormModelBase::hasTabs() const
{
  return mHasTabs;
}

void QgsQuickAttributeFormModelBase::setHasTabs( bool hasTabs )
{
  if ( hasTabs == mHasTabs )
    return;

  mHasTabs = hasTabs;
  emit hasTabsChanged();
}

void QgsQuickAttributeFormModelBase::save()
{
  mAttributeModel->save();
}

void QgsQuickAttributeFormModelBase::create()
{
  mAttributeModel->create();
}

/// @endcond
