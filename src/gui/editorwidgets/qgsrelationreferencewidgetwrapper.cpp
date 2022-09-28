/***************************************************************************
    qgsrelationreferencewidgetwrapper.cpp
     --------------------------------------
    Date                 : 20.4.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsrelationreferencewidgetwrapper.h"
#include "qgsproject.h"
#include "qgsrelationmanager.h"
#include "qgsrelationreferencewidget.h"
#include "qgsattributeform.h"

QgsRelationReferenceWidgetWrapper::QgsRelationReferenceWidgetWrapper( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QgsMapCanvas *canvas, QgsMessageBar *messageBar, QWidget *parent )
  : QgsEditorWidgetWrapper( vl, fieldIdx, editor, parent )
  , mCanvas( canvas )
  , mMessageBar( messageBar )
  , mIndeterminateState( false )
{
}

QWidget *QgsRelationReferenceWidgetWrapper::createWidget( QWidget *parent )
{
  QgsRelationReferenceWidget *w = new QgsRelationReferenceWidget( parent );
  return w;
}

void QgsRelationReferenceWidgetWrapper::initWidget( QWidget *editor )
{
  QgsRelationReferenceWidget *w = qobject_cast<QgsRelationReferenceWidget *>( editor );
  if ( !w )
  {
    w = new QgsRelationReferenceWidget( editor );
  }

  mWidget = w;

  const QgsAttributeEditorContext *ctx = &context();

  mWidget->setEditorContext( *ctx, mCanvas, mMessageBar );

  const bool showForm = config( QStringLiteral( "ShowForm" ), false ).toBool();
  const bool mapIdent = config( QStringLiteral( "MapIdentification" ), false ).toBool();
  const bool readOnlyWidget = config( QStringLiteral( "ReadOnly" ), false ).toBool();
  const bool orderByValue = config( QStringLiteral( "OrderByValue" ), false ).toBool();
  const bool showOpenFormButton = config( QStringLiteral( "ShowOpenFormButton" ), true ).toBool();

  mWidget->setEmbedForm( showForm );
  mWidget->setReadOnlySelector( readOnlyWidget );
  mWidget->setAllowMapIdentification( mapIdent );
  mWidget->setOrderByValue( orderByValue );
  mWidget->setOpenFormButtonVisible( showOpenFormButton );
  if ( config( QStringLiteral( "FilterFields" ), QVariant() ).isValid() )
  {
    mWidget->setFilterFields( config( QStringLiteral( "FilterFields" ) ).toStringList() );
    mWidget->setChainFilters( config( QStringLiteral( "ChainFilters" ) ).toBool() );
  }
  if ( !config( QStringLiteral( "FilterExpression" ) ).toString().isEmpty() )
  {
    mWidget->setFilterExpression( config( QStringLiteral( "FilterExpression" ) ).toString() );
  }
  mWidget->setAllowAddFeatures( config( QStringLiteral( "AllowAddFeatures" ), false ).toBool() );

  const QVariant relationName = config( QStringLiteral( "Relation" ) );

  // Store relation data source and provider key
  mWidget->setReferencedLayerDataSource( config( QStringLiteral( "ReferencedLayerDataSource" ) ).toString() );
  mWidget->setReferencedLayerProviderKey( config( QStringLiteral( "ReferencedLayerProviderKey" ) ).toString() );
  mWidget->setReferencedLayerId( config( QStringLiteral( "ReferencedLayerId" ) ).toString() );
  mWidget->setReferencedLayerName( config( QStringLiteral( "ReferencedLayerName" ) ).toString() );

  QgsRelation relation; // invalid relation by default
  if ( relationName.isValid() )
    relation = QgsProject::instance()->relationManager()->relation( relationName.toString() );
  if ( !relation.isValid() && !layer()->referencingRelations( fieldIdx() ).isEmpty() )
    relation = layer()->referencingRelations( fieldIdx() )[0];

  // If this widget is already embedded by the same relation, reduce functionality
  do
  {
    if ( ctx->relation().id() == relation.id() )
    {
      mWidget->setEmbedForm( false );
      mWidget->setReadOnlySelector( true );
      mWidget->setAllowMapIdentification( false );
      mWidget->setOpenFormButtonVisible( false );
      mWidget->setAllowAddFeatures( false );
      break;
    }
    ctx = ctx->parentContext();
  }
  while ( ctx );

  mWidget->setRelation( relation, config( QStringLiteral( "AllowNULL" ) ).toBool() );

  connect( mWidget, &QgsRelationReferenceWidget::foreignKeysChanged, this, &QgsRelationReferenceWidgetWrapper::foreignKeysChanged );
}

QVariant QgsRelationReferenceWidgetWrapper::value() const
{
  if ( !mWidget )
    return QVariant( field().type() );

  const QVariantList fkeys = mWidget->foreignKeys();

  if ( fkeys.isEmpty() )
  {
    return QVariant( field().type() );
  }
  else
  {
    const QList<QgsRelation::FieldPair> fieldPairs = mWidget->relation().fieldPairs();
    Q_ASSERT( fieldPairs.count() == fkeys.count() );
    for ( int i = 0; i < fieldPairs.count(); i++ )
    {
      if ( fieldPairs.at( i ).referencingField() == field().name() )
        return fkeys.at( i );
    }
    return QVariant( field().type() ); // should not happen
  }
}

bool QgsRelationReferenceWidgetWrapper::valid() const
{
  return mWidget;
}

void QgsRelationReferenceWidgetWrapper::showIndeterminateState()
{
  if ( mWidget )
  {
    mWidget->showIndeterminateState();
  }
  mIndeterminateState = true;
}

QVariantList QgsRelationReferenceWidgetWrapper::additionalFieldValues() const
{
  if ( !mWidget || !mWidget->relation().isValid() )
  {
    QVariantList values;
    for ( int i = 0; i < mWidget->relation().fieldPairs().count(); i++ )
    {
      values << QVariant();
    }
    return values;
  }
  else
  {
    QVariantList values = mWidget->foreignKeys();
    const QList<QgsRelation::FieldPair> fieldPairs = mWidget->relation().fieldPairs();
    const int fieldCount = std::min( fieldPairs.count(), values.count() );
    for ( int i = 0; i < fieldCount; i++ )
    {
      if ( fieldPairs.at( i ).referencingField() == field().name() )
      {
        values.removeAt( i );
        break;
      }
    }
    return values;
  }
}

QStringList QgsRelationReferenceWidgetWrapper::additionalFields() const
{
  if ( !mWidget || !mWidget->relation().isValid() )
    return QStringList();

  QStringList fields;
  const QList<QgsRelation::FieldPair> fieldPairs = mWidget->relation().fieldPairs();
  for ( int i = 0; i < fieldPairs.count(); i++ )
  {
    if ( fieldPairs.at( i ).referencingField() == field().name() )
      continue;

    fields << fieldPairs.at( i ).referencingField();
  }
  return fields;
}

void QgsRelationReferenceWidgetWrapper::updateValues( const QVariant &val, const QVariantList &additionalValues )
{
  if ( !mWidget || ( !mIndeterminateState && val == value() && QgsVariantUtils::isNull( val ) == QgsVariantUtils::isNull( value() ) ) )
    return;

  mIndeterminateState = false;

  QVariantList values = additionalValues;
  const QList<QgsRelation::FieldPair> fieldPairs = mWidget->relation().fieldPairs();
  for ( int i = 0; i < fieldPairs.count(); i++ )
  {
    if ( fieldPairs.at( i ).referencingField() == field().name() )
    {
      values.insert( i, val );
      break;
    }
  }
  Q_ASSERT( values.count() == fieldPairs.count() );

  mBlockChanges++;
  mWidget->setForeignKeys( values );
  mWidget->setFormFeature( formFeature() );
  mBlockChanges--;
}

void QgsRelationReferenceWidgetWrapper::setEnabled( bool enabled )
{
  if ( !mWidget )
    return;

  mWidget->setRelationEditable( enabled );
}

void QgsRelationReferenceWidgetWrapper::foreignKeysChanged( const QVariantList &values )
{
  if ( mBlockChanges != 0 ) // initial value is being set, we can ignore this signal
    return;

  QVariant mainValue = QVariant( field().type() );

  if ( !mWidget || !mWidget->relation().isValid() )
  {
    Q_NOWARN_DEPRECATED_PUSH
    emit valueChanged( mainValue );
    Q_NOWARN_DEPRECATED_POP
    emit valuesChanged( mainValue );
    return;
  }

  QVariantList additionalValues = values;
  const QList<QgsRelation::FieldPair> fieldPairs = mWidget->relation().fieldPairs();
  for ( int i = 0; i < fieldPairs.count(); i++ )
  {
    if ( fieldPairs.at( i ).referencingField() == field().name() )
      mainValue = additionalValues.takeAt( i ); // additional values in field pair order remain
  }
  Q_ASSERT( additionalValues.count() == values.count() - 1 );

  Q_NOWARN_DEPRECATED_PUSH
  emit valueChanged( mainValue );
  Q_NOWARN_DEPRECATED_POP
  emit valuesChanged( mainValue, additionalValues );
}

void QgsRelationReferenceWidgetWrapper::updateConstraintWidgetStatus()
{
  if ( mWidget )
  {
    if ( !constraintResultVisible() )
    {
      widget()->setStyleSheet( QString() );
    }
    else
    {
      switch ( constraintResult() )
      {
        case ConstraintResultPass:
          mWidget->setStyleSheet( QString() );
          break;

        case ConstraintResultFailHard:
          mWidget->setStyleSheet( QStringLiteral( ".QComboBox { background-color: #dd7777; }" ) );
          break;

        case ConstraintResultFailSoft:
          mWidget->setStyleSheet( QStringLiteral( ".QComboBox { background-color: #ffd85d; }" ) );
          break;
      }
    }
  }
}
