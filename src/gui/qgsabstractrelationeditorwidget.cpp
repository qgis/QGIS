/***************************************************************************
                         qgsabstractrelationeditorwidget.cpp
                         ----------------------
    begin                : October 2020
    copyright            : (C) 2020 by Ivan Ivanov
    email                : ivan@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsabstractrelationeditorwidget.h"

#include "qgsfeatureiterator.h"
#include "qgsexpression.h"
#include "qgsfeature.h"
#include "qgsfeatureselectiondlg.h"
#include "qgsrelation.h"
#include "qgsrelationmanager.h"
#include "qgspolymorphicrelation.h"
#include "qgsvectorlayertools.h"
#include "qgsproject.h"
#include "qgstransactiongroup.h"
#include "qgsvectorlayerutils.h"

#include <QMessageBox>
#include <QPushButton>

QgsAbstractRelationEditorWidget::QgsAbstractRelationEditorWidget( const QVariantMap &config, QWidget *parent )
  : QWidget( parent )
{
  Q_UNUSED( config );
}

void QgsAbstractRelationEditorWidget::setRelationFeature( const QgsRelation &relation, const QgsFeature &feature )
{
  beforeSetRelationFeature( relation, feature );

  mRelation = relation;
  mFeature = feature;

  setObjectName( QStringLiteral( "referenced/" ) + mRelation.name() );

  afterSetRelationFeature();
  updateUi();
}

void QgsAbstractRelationEditorWidget::setRelations( const QgsRelation &relation, const QgsRelation &nmrelation )
{

  beforeSetRelations( relation, nmrelation );

  mRelation = relation;
  mNmRelation = nmrelation;

  if ( !mRelation.isValid() )
  {
    afterSetRelations();
    return;
  }

  mLayerInSameTransactionGroup = false;

  const auto transactionGroups = QgsProject::instance()->transactionGroups();
  for ( auto it = transactionGroups.constBegin(); it != transactionGroups.constEnd(); ++it )
  {
    if ( mNmRelation.isValid() )
    {
      if ( it.value()->layers().contains( mRelation.referencedLayer() ) &&
           it.value()->layers().contains( mRelation.referencingLayer() ) &&
           it.value()->layers().contains( mNmRelation.referencedLayer() ) )
        mLayerInSameTransactionGroup = true;
    }
    else
    {
      if ( it.value()->layers().contains( mRelation.referencedLayer() ) &&
           it.value()->layers().contains( mRelation.referencingLayer() ) )
        mLayerInSameTransactionGroup = true;
    }
  }

  setObjectName( QStringLiteral( "referenced/" ) + mRelation.name() );

  afterSetRelations();
  updateUi();
}

void QgsAbstractRelationEditorWidget::setEditorContext( const QgsAttributeEditorContext &context )
{
  mEditorContext = context;
}

QgsAttributeEditorContext QgsAbstractRelationEditorWidget::editorContext() const
{
  return mEditorContext;
}

void QgsAbstractRelationEditorWidget::setFeature( const QgsFeature &feature, bool update )
{
  mFeature = feature;

  mEditorContext.setFormFeature( feature );

  if ( update )
    updateUi();
}

void QgsAbstractRelationEditorWidget::setNmRelationId( const QVariant &nmRelationId )
{
  QgsRelation nmrelation = QgsProject::instance()->relationManager()->relation( nmRelationId.toString() );
  beforeSetRelations( mRelation, nmrelation );
  mNmRelation = nmrelation;
  afterSetRelations();
  updateUi();
}

QVariant QgsAbstractRelationEditorWidget::nmRelationId() const
{
  return mNmRelation.id();
}

QString QgsAbstractRelationEditorWidget::label() const
{
  return QString();
}

void QgsAbstractRelationEditorWidget::setLabel( const QString &label )
{
  Q_UNUSED( label )
}

bool QgsAbstractRelationEditorWidget::showLabel() const
{
  return false;
}

void QgsAbstractRelationEditorWidget::setShowLabel( bool showLabel )
{
  Q_UNUSED( showLabel )
}

void QgsAbstractRelationEditorWidget::setForceSuppressFormPopup( bool forceSuppressFormPopup )
{
  mForceSuppressFormPopup = forceSuppressFormPopup;
}

bool QgsAbstractRelationEditorWidget::forceSuppressFormPopup() const
{
  return mForceSuppressFormPopup;
}

void QgsAbstractRelationEditorWidget::updateTitle()
{
}

QgsFeature QgsAbstractRelationEditorWidget::feature() const
{
  return mFeature;
}

void QgsAbstractRelationEditorWidget::toggleEditing( bool state )
{
  if ( state )
  {
    mEditorContext.vectorLayerTools()->startEditing( mRelation.referencingLayer() );
    if ( mNmRelation.isValid() )
      mEditorContext.vectorLayerTools()->startEditing( mNmRelation.referencedLayer() );
  }
  else
  {
    mEditorContext.vectorLayerTools()->stopEditing( mRelation.referencingLayer() );
    if ( mNmRelation.isValid() )
      mEditorContext.vectorLayerTools()->stopEditing( mNmRelation.referencedLayer() );
  }
}

void QgsAbstractRelationEditorWidget::saveEdits()
{
  mEditorContext.vectorLayerTools()->saveEdits( mRelation.referencingLayer() );
  if ( mNmRelation.isValid() )
    mEditorContext.vectorLayerTools()->saveEdits( mNmRelation.referencedLayer() );
}

void QgsAbstractRelationEditorWidget::addFeature( const QgsGeometry &geometry )
{
  QgsAttributeMap keyAttrs;

  const QgsVectorLayerTools *vlTools = mEditorContext.vectorLayerTools();

  // Fields of the linking table
  const QgsFields fields = mRelation.referencingLayer()->fields();

  // For generated relations insert the referenced layer field
  if ( mRelation.type() == QgsRelation::Generated )
  {
    QgsPolymorphicRelation polyRel = mRelation.polymorphicRelation();
    keyAttrs.insert( fields.indexFromName( polyRel.referencedLayerField() ), polyRel.layerRepresentation( mRelation.referencedLayer() ) );
  }

  if ( mNmRelation.isValid() )
  {
    // only normal relations support m:n relation
    Q_ASSERT( mNmRelation.type() == QgsRelation::Normal );

    // n:m Relation: first let the user create a new feature on the other table
    // and autocreate a new linking feature.
    QgsFeature f;
    if ( !vlTools->addFeature( mNmRelation.referencedLayer(), QgsAttributeMap(), geometry, &f ) )
      return;

    // Expression context for the linking table
    QgsExpressionContext context = mRelation.referencingLayer()->createExpressionContext();

    QgsAttributeMap linkAttributes = keyAttrs;
    const auto constFieldPairs = mRelation.fieldPairs();
    for ( const QgsRelation::FieldPair &fieldPair : constFieldPairs )
    {
      int index = fields.indexOf( fieldPair.first );
      linkAttributes.insert( index,  mFeature.attribute( fieldPair.second ) );
    }

    const auto constNmFieldPairs = mNmRelation.fieldPairs();
    for ( const QgsRelation::FieldPair &fieldPair : constNmFieldPairs )
    {
      int index = fields.indexOf( fieldPair.first );
      linkAttributes.insert( index, f.attribute( fieldPair.second ) );
    }
    QgsFeature linkFeature = QgsVectorLayerUtils::createFeature( mRelation.referencingLayer(), QgsGeometry(), linkAttributes, &context );

    mRelation.referencingLayer()->addFeature( linkFeature );
  }
  else
  {
    const auto constFieldPairs = mRelation.fieldPairs();
    for ( const QgsRelation::FieldPair &fieldPair : constFieldPairs )
    {
      keyAttrs.insert( fields.indexFromName( fieldPair.referencingField() ), mFeature.attribute( fieldPair.referencedField() ) );
    }

    if ( !vlTools->addFeature( mRelation.referencingLayer(), keyAttrs, geometry ) )
      return;
  }

  updateUi();
}

void QgsAbstractRelationEditorWidget::deleteFeature( const QgsFeatureId fid )
{
  deleteFeatures( QgsFeatureIds() << fid );
}

void QgsAbstractRelationEditorWidget::deleteFeatures( const QgsFeatureIds &fids )
{
  bool deleteFeatures = true;

  QgsVectorLayer *layer;
  if ( mNmRelation.isValid() )
  {
    // only normal relations support m:n relation
    Q_ASSERT( mNmRelation.type() == QgsRelation::Normal );

    layer = mNmRelation.referencedLayer();

    // When deleting a linked feature within an N:M relation,
    // check if the feature is linked to more than just one feature.
    // In case it is linked more than just once, ask the user for confirmation
    // as it is likely he was not aware of the implications and might delete
    // there may be several linking entries deleted along.

    QgsFeatureRequest deletedFeaturesRequest;
    deletedFeaturesRequest.setFilterFids( fids );
    deletedFeaturesRequest.setFlags( QgsFeatureRequest::NoGeometry );
    deletedFeaturesRequest.setSubsetOfAttributes( QgsAttributeList() << mNmRelation.referencedFields().first() );

    QgsFeatureIterator deletedFeatures = layer->getFeatures( deletedFeaturesRequest );
    QStringList deletedFeaturesPks;
    QgsFeature feature;
    while ( deletedFeatures.nextFeature( feature ) )
    {
      deletedFeaturesPks.append( QgsExpression::quotedValue( feature.attribute( mNmRelation.referencedFields().first() ) ) );
    }

    QgsFeatureRequest linkingFeaturesRequest;
    linkingFeaturesRequest.setFlags( QgsFeatureRequest::NoGeometry );
    linkingFeaturesRequest.setNoAttributes();

    QString linkingFeaturesRequestExpression;
    if ( !deletedFeaturesPks.empty() )
    {
      linkingFeaturesRequestExpression = QStringLiteral( "%1 IN (%2)" ).arg( QgsExpression::quotedColumnRef( mNmRelation.fieldPairs().first().first ), deletedFeaturesPks.join( ',' ) );
      linkingFeaturesRequest.setFilterExpression( linkingFeaturesRequestExpression );

      QgsFeatureIterator relatedLinkingFeatures = mNmRelation.referencingLayer()->getFeatures( linkingFeaturesRequest );

      int relatedLinkingFeaturesCount = 0;
      while ( relatedLinkingFeatures.nextFeature( feature ) )
      {
        relatedLinkingFeaturesCount++;
      }

      if ( deletedFeaturesPks.size() == 1 && relatedLinkingFeaturesCount > 1 )
      {
        QMessageBox messageBox( QMessageBox::Question, tr( "Really delete entry?" ), tr( "The entry on %1 is still linked to %2 features on %3. Do you want to delete it?" ).arg( mNmRelation.referencedLayer()->name(), QLocale().toString( relatedLinkingFeaturesCount ), mRelation.referencedLayer()->name() ), QMessageBox::NoButton, this );
        messageBox.addButton( QMessageBox::Cancel );
        QAbstractButton *deleteButton = messageBox.addButton( tr( "Delete" ),  QMessageBox::AcceptRole );

        messageBox.exec();
        if ( messageBox.clickedButton() != deleteButton )
          deleteFeatures = false;
      }
      else if ( deletedFeaturesPks.size() > 1 && relatedLinkingFeaturesCount > deletedFeaturesPks.size() )
      {
        QMessageBox messageBox( QMessageBox::Question, tr( "Really delete entries?" ), tr( "The %1 entries on %2 are still linked to %3 features on %4. Do you want to delete them?" ).arg( QLocale().toString( deletedFeaturesPks.size() ), mNmRelation.referencedLayer()->name(), QLocale().toString( relatedLinkingFeaturesCount ), mRelation.referencedLayer()->name() ), QMessageBox::NoButton, this );
        messageBox.addButton( QMessageBox::Cancel );
        QAbstractButton *deleteButton = messageBox.addButton( tr( "Delete" ), QMessageBox::AcceptRole );

        messageBox.exec();
        if ( messageBox.clickedButton() != deleteButton )
          deleteFeatures = false;
      }
    }
  }
  else
  {
    layer = mRelation.referencingLayer();
  }

  QgsVectorLayerUtils::QgsDuplicateFeatureContext infoContext;
  if ( QgsVectorLayerUtils::impactsCascadeFeatures( layer, fids, QgsProject::instance(), infoContext ) )
  {
    QString childrenInfo;
    int childrenCount = 0;
    const auto infoContextLayers = infoContext.layers();
    for ( QgsVectorLayer *chl : infoContextLayers )
    {
      childrenCount += infoContext.duplicatedFeatures( chl ).size();
      childrenInfo += ( tr( "%1 feature(s) on layer \"%2\", " ).arg( infoContext.duplicatedFeatures( chl ).size() ).arg( chl->name() ) );
    }

    // for extra safety to make sure we know that the delete can have impact on children and joins
    int res = QMessageBox::question( this, tr( "Delete at least %1 feature(s) on other layer(s)" ).arg( childrenCount ),
                                     tr( "Delete %1 feature(s) on layer \"%2\", %3 as well\nand all of its other descendants.\nDelete these features?" ).arg( fids.count() ).arg( layer->name() ).arg( childrenInfo ),
                                     QMessageBox::Yes | QMessageBox::No );
    if ( res != QMessageBox::Yes )
      deleteFeatures = false;
  }

  if ( deleteFeatures )
  {
    QgsVectorLayer::DeleteContext context( true, QgsProject::instance() );
    layer->deleteFeatures( fids, &context );
    const auto contextLayers = context.handledLayers();
    if ( contextLayers.size() > 1 )
    {
      int deletedCount = 0;
      QString feedbackMessage;
      for ( QgsVectorLayer *contextLayer : contextLayers )
      {
        feedbackMessage += tr( "%1 on layer %2. " ).arg( context.handledFeatures( contextLayer ).size() ).arg( contextLayer->name() );
        deletedCount += context.handledFeatures( contextLayer ).size();
      }
      mEditorContext.mainMessageBar()->pushMessage( tr( "%1 features deleted: %2" ).arg( deletedCount ).arg( feedbackMessage ), Qgis::MessageLevel::Success );
    }

    updateUi();
  }
}

void QgsAbstractRelationEditorWidget::linkFeature()
{
  QgsVectorLayer *layer = nullptr;

  if ( mNmRelation.isValid() )
  {
    // only normal relations support m:n relation
    Q_ASSERT( mNmRelation.type() == QgsRelation::Normal );

    layer = mNmRelation.referencedLayer();
  }
  else
    layer = mRelation.referencingLayer();

  QgsFeatureSelectionDlg *selectionDlg = new QgsFeatureSelectionDlg( layer, mEditorContext, this );
  selectionDlg->setAttribute( Qt::WA_DeleteOnClose );

  const QString displayString = QgsVectorLayerUtils::getFeatureDisplayString( mRelation.referencedLayer(), mFeature );
  selectionDlg->setWindowTitle( tr( "Link existing child features for parent %1 \"%2\"" ).arg( mRelation.referencedLayer()->name(), displayString ) );

  connect( selectionDlg, &QDialog::accepted, this, &QgsAbstractRelationEditorWidget::onLinkFeatureDlgAccepted );
  selectionDlg->show();
}

void QgsAbstractRelationEditorWidget::onLinkFeatureDlgAccepted()
{
  QgsFeatureSelectionDlg *selectionDlg = qobject_cast<QgsFeatureSelectionDlg *>( sender() );
  if ( mNmRelation.isValid() )
  {
    // only normal relations support m:n relation
    Q_ASSERT( mNmRelation.type() == QgsRelation::Normal );

    QgsFeatureIterator it = mNmRelation.referencedLayer()->getFeatures(
                              QgsFeatureRequest()
                              .setFilterFids( selectionDlg->selectedFeatures() )
                              .setSubsetOfAttributes( mNmRelation.referencedFields() ) );

    QgsFeature relatedFeature;

    QgsFeatureList newFeatures;

    // Fields of the linking table
    const QgsFields fields = mRelation.referencingLayer()->fields();

    // Expression context for the linking table
    QgsExpressionContext context = mRelation.referencingLayer()->createExpressionContext();

    QgsAttributeMap linkAttributes;

    if ( mRelation.type() == QgsRelation::Generated )
    {
      QgsPolymorphicRelation polyRel = mRelation.polymorphicRelation();
      Q_ASSERT( polyRel.isValid() );

      linkAttributes.insert( fields.indexFromName( polyRel.referencedLayerField() ),
                             polyRel.layerRepresentation( mRelation.referencedLayer() ) );
    }

    const auto constFieldPairs = mRelation.fieldPairs();
    for ( const QgsRelation::FieldPair &fieldPair : constFieldPairs )
    {
      int index = fields.indexOf( fieldPair.first );
      linkAttributes.insert( index,  mFeature.attribute( fieldPair.second ) );
    }

    while ( it.nextFeature( relatedFeature ) )
    {
      const auto constFieldPairs = mNmRelation.fieldPairs();
      for ( const QgsRelation::FieldPair &fieldPair : constFieldPairs )
      {
        int index = fields.indexOf( fieldPair.first );
        linkAttributes.insert( index, relatedFeature.attribute( fieldPair.second ) );
      }
      const QgsFeature linkFeature = QgsVectorLayerUtils::createFeature( mRelation.referencingLayer(), QgsGeometry(), linkAttributes, &context );

      newFeatures << linkFeature;
    }

    mRelation.referencingLayer()->addFeatures( newFeatures );
    QgsFeatureIds ids;
    const auto constNewFeatures = newFeatures;
    for ( const QgsFeature &f : constNewFeatures )
      ids << f.id();
    mRelation.referencingLayer()->selectByIds( ids );
  }
  else
  {
    QMap<int, QVariant> keys;
    const auto constFieldPairs = mRelation.fieldPairs();
    for ( const QgsRelation::FieldPair &fieldPair : constFieldPairs )
    {
      int idx = mRelation.referencingLayer()->fields().lookupField( fieldPair.referencingField() );
      QVariant val = mFeature.attribute( fieldPair.referencedField() );
      keys.insert( idx, val );
    }

    const auto constSelectedFeatures = selectionDlg->selectedFeatures();
    for ( QgsFeatureId fid : constSelectedFeatures )
    {
      QgsVectorLayer *referencingLayer = mRelation.referencingLayer();
      if ( mRelation.type() == QgsRelation::Generated )
      {
        QgsPolymorphicRelation polyRel = mRelation.polymorphicRelation();

        Q_ASSERT( polyRel.isValid() );

        mRelation.referencingLayer()->changeAttributeValue( fid,
            referencingLayer->fields().indexFromName( polyRel.referencedLayerField() ),
            polyRel.layerRepresentation( mRelation.referencedLayer() ) );
      }

      QMapIterator<int, QVariant> it( keys );
      while ( it.hasNext() )
      {
        it.next();
        referencingLayer->changeAttributeValue( fid, it.key(), it.value() );
      }
    }
  }

  updateUi();
}

void QgsAbstractRelationEditorWidget::unlinkFeature( const QgsFeatureId fid )
{
  unlinkFeatures( QgsFeatureIds() << fid );
}

void QgsAbstractRelationEditorWidget::unlinkFeatures( const QgsFeatureIds &fids )
{
  if ( mNmRelation.isValid() )
  {
    // only normal relations support m:n relation
    Q_ASSERT( mNmRelation.type() == QgsRelation::Normal );

    QgsFeatureIterator selectedIterator = mNmRelation.referencedLayer()->getFeatures(
                                            QgsFeatureRequest()
                                            .setFilterFids( fids )
                                            .setSubsetOfAttributes( mNmRelation.referencedFields() ) );

    QgsFeature f;

    QStringList filters;

    while ( selectedIterator.nextFeature( f ) )
    {
      filters << '(' + mNmRelation.getRelatedFeaturesRequest( f ).filterExpression()->expression() + ')';
    }

    QString filter = QStringLiteral( "(%1) AND (%2)" ).arg(
                       mRelation.getRelatedFeaturesRequest( mFeature ).filterExpression()->expression(),
                       filters.join( QLatin1String( " OR " ) ) );

    QgsFeatureIterator linkedIterator = mRelation.referencingLayer()->getFeatures( QgsFeatureRequest()
                                        .setNoAttributes()
                                        .setFilterExpression( filter ) );

    QgsFeatureIds fids;

    while ( linkedIterator.nextFeature( f ) )
    {
      fids << f.id();
      QgsDebugMsgLevel( FID_TO_STRING( f.id() ), 4 );
    }

    mRelation.referencingLayer()->deleteFeatures( fids );
  }
  else
  {
    QMap<int, QgsField> keyFields;
    const auto constFieldPairs = mRelation.fieldPairs();
    for ( const QgsRelation::FieldPair &fieldPair : constFieldPairs )
    {
      int idx = mRelation.referencingLayer()->fields().lookupField( fieldPair.referencingField() );
      if ( idx < 0 )
      {
        QgsDebugMsg( QStringLiteral( "referencing field %1 not found" ).arg( fieldPair.referencingField() ) );
        return;
      }
      QgsField fld = mRelation.referencingLayer()->fields().at( idx );
      keyFields.insert( idx, fld );
    }

    const auto constFeatureids = fids;
    for ( QgsFeatureId fid : constFeatureids )
    {
      QgsVectorLayer *referencingLayer = mRelation.referencingLayer();
      if ( mRelation.type() == QgsRelation::Generated )
      {
        QgsPolymorphicRelation polyRel = mRelation.polymorphicRelation();

        Q_ASSERT( mRelation.polymorphicRelation().isValid() );

        mRelation.referencingLayer()->changeAttributeValue( fid,
            referencingLayer->fields().indexFromName( polyRel.referencedLayerField() ),
            referencingLayer->fields().field( polyRel.referencedLayerField() ).type() );
      }

      QMapIterator<int, QgsField> it( keyFields );
      while ( it.hasNext() )
      {
        it.next();
        mRelation.referencingLayer()->changeAttributeValue( fid, it.key(), QVariant( it.value().type() ) );
      }
    }
  }

  updateUi();
}

void QgsAbstractRelationEditorWidget::updateUi()
{}

void QgsAbstractRelationEditorWidget::setTitle( const QString &title )
{
  Q_UNUSED( title )
}

void QgsAbstractRelationEditorWidget::beforeSetRelationFeature( const QgsRelation &newRelation, const QgsFeature &newFeature )
{
  Q_UNUSED( newRelation )
  Q_UNUSED( newFeature )
}

void QgsAbstractRelationEditorWidget::afterSetRelationFeature()
{}

void QgsAbstractRelationEditorWidget::beforeSetRelations( const QgsRelation &newRelation, const QgsRelation &newNmRelation )
{
  Q_UNUSED( newRelation )
  Q_UNUSED( newNmRelation )
}

void QgsAbstractRelationEditorWidget::afterSetRelations()
{}

void QgsAbstractRelationEditorWidget::duplicateFeature( const QgsFeatureId &fid )
{
  duplicateFeatures( QgsFeatureIds() << fid );
}

void QgsAbstractRelationEditorWidget::duplicateFeatures( const QgsFeatureIds &fids )
{
  QgsVectorLayer *layer = mRelation.referencingLayer();

  QgsFeatureIterator fit = layer->getFeatures( QgsFeatureRequest().setFilterFids( fids ) );
  QgsFeature f;
  while ( fit.nextFeature( f ) )
  {
    QgsVectorLayerUtils::QgsDuplicateFeatureContext duplicatedFeatureContext;
    QgsVectorLayerUtils::duplicateFeature( layer, f, QgsProject::instance(), duplicatedFeatureContext );
  }
}

void QgsAbstractRelationEditorWidget::showEvent( QShowEvent * )
{
  updateUi();
}


///////////////////////////////////////////////////////////////////////////////


QgsAbstractRelationEditorConfigWidget::QgsAbstractRelationEditorConfigWidget( const QgsRelation &relation, QWidget *parent )
  : QWidget( parent )
  , mRelation( relation )
{
}

QgsVectorLayer *QgsAbstractRelationEditorConfigWidget::layer()
{
  return mLayer;
}

QgsRelation QgsAbstractRelationEditorConfigWidget::relation() const
{
  return mRelation;
}

void QgsAbstractRelationEditorConfigWidget::setNmRelation( const QgsRelation &nmRelation )
{
  mNmRelation = nmRelation;
}

QgsRelation QgsAbstractRelationEditorConfigWidget::nmRelation() const
{
  return mNmRelation;
}


///////////////////////////////////////////////////////////////////////////////


QgsAbstractRelationEditorWidgetFactory::QgsAbstractRelationEditorWidgetFactory()
{
}
