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

#include "qgsattributeform.h"
#include "qgsproject.h"
#include "qgsrelationmanager.h"
#include "qgsrelationreferencewidget.h"
#include "qgsvaluerelationfieldformatter.h"

#include "moc_qgsrelationreferencewidgetwrapper.cpp"

QgsRelationReferenceWidgetWrapper::QgsRelationReferenceWidgetWrapper( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QgsMapCanvas *canvas, QgsMessageBar *messageBar, QWidget *parent )
  : QgsEditorWidgetWrapper( vl, fieldIdx, editor, parent )
  , mCanvas( canvas )
  , mMessageBar( messageBar )

{
}

QWidget *QgsRelationReferenceWidgetWrapper::createWidget( QWidget *parent )
{
  QgsAttributeForm *form = qobject_cast<QgsAttributeForm *>( parent );
  if ( form )
    connect( form, &QgsAttributeForm::widgetValueChanged, this, &QgsRelationReferenceWidgetWrapper::widgetValueChanged );

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

  const bool showForm = config( u"ShowForm"_s, false ).toBool();
  const bool mapIdent = config( u"MapIdentification"_s, false ).toBool();
  const bool readOnlyWidget = config( u"ReadOnly"_s, false ).toBool();
  const bool showOpenFormButton = config( u"ShowOpenFormButton"_s, true ).toBool();

  mWidget->setEmbedForm( showForm );
  mWidget->setReadOnlySelector( readOnlyWidget );
  mWidget->setAllowMapIdentification( mapIdent );
  mWidget->setOpenFormButtonVisible( showOpenFormButton );

  const bool fetchLimitActive = config( u"FetchLimitActive"_s, QgsSettings().value( u"maxEntriesRelationWidget"_s, 100, QgsSettings::Gui ).toInt() > 0 ).toBool();
  if ( fetchLimitActive )
  {
    mWidget->setFetchLimit( config( u"FetchLimitNumber"_s, QgsSettings().value( u"maxEntriesRelationWidget"_s, 100, QgsSettings::Gui ) ).toInt() );
  }

  if ( config( u"FilterFields"_s, QVariant() ).isValid() )
  {
    mWidget->setFilterFields( config( u"FilterFields"_s ).toStringList() );
    mWidget->setChainFilters( config( u"ChainFilters"_s ).toBool() );
  }
  if ( !config( u"FilterExpression"_s ).toString().isEmpty() )
  {
    mWidget->setFilterExpression( config( u"FilterExpression"_s ).toString() );
    mWidget->setFormFeature( formFeature() );
    mWidget->setParentFormFeature( ctx->parentFormFeature() );
  }
  mWidget->setAllowAddFeatures( config( u"AllowAddFeatures"_s, false ).toBool() );

  mWidget->setOrderExpression( config( u"OrderExpression"_s ).toString() );
  mWidget->setSortOrder( config( u"OrderDescending"_s, false ).toBool() ? Qt::DescendingOrder : Qt::AscendingOrder );

  const QVariant relationName = config( u"Relation"_s );

  // Store relation data source and provider key
  mWidget->setReferencedLayerDataSource( config( u"ReferencedLayerDataSource"_s ).toString() );
  mWidget->setReferencedLayerProviderKey( config( u"ReferencedLayerProviderKey"_s ).toString() );
  mWidget->setReferencedLayerId( config( u"ReferencedLayerId"_s ).toString() );
  mWidget->setReferencedLayerName( config( u"ReferencedLayerName"_s ).toString() );

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
  } while ( ctx );

  // If AllowNULL is not set in the config, provide a default value based on the
  // constraints of the referencing fields
  if ( !config( u"AllowNULL"_s ).isValid() )
  {
    mWidget->setRelation( relation, relation.referencingFieldsAllowNull() );
  }
  else
  {
    mWidget->setRelation( relation, config( u"AllowNULL"_s ).toBool() );
  }

  connect( mWidget, &QgsRelationReferenceWidget::foreignKeysChanged, this, &QgsRelationReferenceWidgetWrapper::foreignKeysChanged );
}

void QgsRelationReferenceWidgetWrapper::aboutToSave()
{
  // Save changes in the embedded form
  mWidget->saveReferencedAttributeForm();
}

QVariant QgsRelationReferenceWidgetWrapper::value() const
{
  if ( !mWidget )
    return QgsVariantUtils::createNullVariant( field().type() );

  const QVariantList fkeys = mWidget->foreignKeys();

  if ( fkeys.isEmpty() )
  {
    return QgsVariantUtils::createNullVariant( field().type() );
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
    return QgsVariantUtils::createNullVariant( field().type() ); // should not happen
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
  if ( !mWidget )
    return {};

  if ( !mWidget->relation().isValid() )
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

  QVariant mainValue = QgsVariantUtils::createNullVariant( field().type() );

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
          mWidget->setStyleSheet( u".QComboBox { background-color: #dd7777; }"_s );
          break;

        case ConstraintResultFailSoft:
          mWidget->setStyleSheet( u".QComboBox { background-color: #ffd85d; }"_s );
          break;
      }
    }
  }
}

void QgsRelationReferenceWidgetWrapper::parentFormValueChanged( const QString &attribute, const QVariant &value )
{
  // Update the parent feature in the context ( which means to replace the whole context :/ )
  QgsAttributeEditorContext ctx { context() };
  QgsFeature feature { context().parentFormFeature() };
  feature.setAttribute( attribute, value );
  ctx.setParentFormFeature( feature );
  setContext( ctx );

  // Check if the change might affect the filter expression and the cache needs updates
  if ( QgsValueRelationFieldFormatter::expressionRequiresParentFormScope( mWidget->filterExpression() )
       && QgsValueRelationFieldFormatter::expressionParentFormAttributes( mExpression ).contains( attribute ) )
  {
    mWidget->setParentFormFeature( context().parentFormFeature() );
  }
}

void QgsRelationReferenceWidgetWrapper::widgetValueChanged( const QString &attribute, const QVariant &newValue, bool attributeChanged )
{
  if ( attributeChanged )
  {
    setFormFeatureAttribute( attribute, newValue );
    if ( QgsValueRelationFieldFormatter::expressionRequiresFormScope( mWidget->filterExpression() )
         && QgsValueRelationFieldFormatter::expressionFormAttributes( mWidget->filterExpression() ).contains( attribute ) )
    {
      mWidget->setFormFeature( formFeature() );
    }
  }
}
