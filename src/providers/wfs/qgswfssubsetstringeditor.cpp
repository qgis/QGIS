/***************************************************************************
    qgswfssubsetstringeditor.cpp
     --------------------------------------
    Date                 : 15-Nov-2020
    Copyright            : (C) 2020 by Even Rouault
    Email                : even.rouault at spatials.com
****************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswfssubsetstringeditor.h"
#include "moc_qgswfssubsetstringeditor.cpp"
#include "qgswfsprovider.h"
#include "qgswfsshareddata.h"
#include "qgswfsutils.h"
#include "qgssqlstatement.h"

QgsSubsetStringEditorInterface *QgsWfsSubsetStringEditor::create( QgsVectorLayer *layer, QgsWFSProvider *provider, QWidget *parent, Qt::WindowFlags fl )
{
  Q_ASSERT( provider );
  const auto caps = provider->sharedData()->capabilities();

  QgsWFSDataSourceURI uri( provider->uri().uri( false ) );
  const QString typeName( uri.typeName() );
  QString displayedTypeName( typeName );
  if ( !caps.setAmbiguousUnprefixedTypename.contains( QgsWFSUtils::removeNamespacePrefix( typeName ) ) )
    displayedTypeName = QgsWFSUtils::removeNamespacePrefix( typeName );
  QString allSql( "SELECT * FROM " + QgsSQLStatement::quotedIdentifierIfNeeded( displayedTypeName ) );

  QgsSQLComposerDialog *d = new QgsSQLComposerDialog( layer, parent, fl );

  QgsWFSValidatorCallback *validatorCbk = new QgsWFSValidatorCallback( d, uri, allSql, caps );
  d->setSQLValidatorCallback( validatorCbk );

  QgsWFSTableSelectedCallback *tableSelectedCbk = new QgsWFSTableSelectedCallback( d, uri, caps );
  d->setTableSelectedCallback( tableSelectedCbk );

  const bool bSupportJoins = caps.featureTypes.size() > 1 && caps.supportsJoins;
  d->setSupportMultipleTables( bSupportJoins, QgsSQLStatement::quotedIdentifierIfNeeded( displayedTypeName ) );

  QMap<QString, QString> mapTypenameToTitle;
  for ( const QgsWfsCapabilities::FeatureType &f : std::as_const( caps.featureTypes ) )
    mapTypenameToTitle[f.name] = f.title;

  QList<QgsSQLComposerDialog::PairNameTitle> tablenames;
  tablenames << QgsSQLComposerDialog::PairNameTitle(
    QgsSQLStatement::quotedIdentifierIfNeeded( displayedTypeName ), mapTypenameToTitle[typeName]
  );
  if ( bSupportJoins )
  {
    for ( const auto &featureType : caps.featureTypes )
    {
      const QString iterTypename = featureType.name;
      if ( iterTypename != typeName )
      {
        QString displayedIterTypename( iterTypename );
        QString unprefixedIterTypename( QgsWFSUtils::removeNamespacePrefix( iterTypename ) );
        if ( !caps.setAmbiguousUnprefixedTypename.contains( unprefixedIterTypename ) )
          displayedIterTypename = unprefixedIterTypename;

        tablenames << QgsSQLComposerDialog::PairNameTitle(
          QgsSQLStatement::quotedIdentifierIfNeeded( displayedIterTypename ), mapTypenameToTitle[iterTypename]
        );
      }
    }
  }
  d->addTableNames( tablenames );

  QList<QgsSQLComposerDialog::Function> functionList;
  for ( const QgsWfsCapabilities::Function &f : std::as_const( caps.functionList ) )
  {
    QgsSQLComposerDialog::Function dialogF;
    dialogF.name = f.name;
    dialogF.returnType = f.returnType;
    dialogF.minArgs = f.minArgs;
    dialogF.maxArgs = f.maxArgs;
    for ( const QgsWfsCapabilities::Argument &arg : std::as_const( f.argumentList ) )
    {
      dialogF.argumentList << QgsSQLComposerDialog::Argument( arg.name, arg.type );
    }
    functionList << dialogF;
  }
  d->addFunctions( functionList );

  QList<QgsSQLComposerDialog::Function> spatialPredicateList;
  for ( const QgsWfsCapabilities::Function &f : std::as_const( caps.spatialPredicatesList ) )
  {
    QgsSQLComposerDialog::Function dialogF;
    dialogF.name = f.name;
    dialogF.returnType = f.returnType;
    dialogF.minArgs = f.minArgs;
    dialogF.maxArgs = f.maxArgs;
    for ( const QgsWfsCapabilities::Argument &arg : std::as_const( f.argumentList ) )
    {
      dialogF.argumentList << QgsSQLComposerDialog::Argument( arg.name, arg.type );
    }
    spatialPredicateList << dialogF;
  }
  d->addSpatialPredicates( spatialPredicateList );

  QList<QgsSQLComposerDialog::PairNameType> fieldList;
  QString fieldNamePrefix;
  if ( bSupportJoins )
  {
    fieldNamePrefix = QgsSQLStatement::quotedIdentifierIfNeeded( displayedTypeName ) + ".";
  }
  const auto constToList = provider->fields().toList();
  for ( const QgsField &field : constToList )
  {
    QString fieldName( fieldNamePrefix + QgsSQLStatement::quotedIdentifierIfNeeded( field.name() ) );
    fieldList << QgsSQLComposerDialog::PairNameType( fieldName, field.typeName() );
  }
  if ( !provider->geometryAttribute().isEmpty() )
  {
    QString fieldName( fieldNamePrefix + QgsSQLStatement::quotedIdentifierIfNeeded( provider->geometryAttribute() ) );
    fieldList << QgsSQLComposerDialog::PairNameType( fieldName, QStringLiteral( "geometry" ) );
  }
  fieldList << QgsSQLComposerDialog::PairNameType( fieldNamePrefix + "*", QString() );

  d->addColumnNames( fieldList, QgsSQLStatement::quotedIdentifierIfNeeded( displayedTypeName ) );

  return d;
}

QgsWFSValidatorCallback::QgsWFSValidatorCallback( QObject *parent, const QgsWFSDataSourceURI &uri, const QString &allSql, const QgsWfsCapabilities::Capabilities &caps )
  : QObject( parent )
  , mURI( uri )
  , mAllSql( allSql )
  , mCaps( caps )
{
}

bool QgsWFSValidatorCallback::isValid( const QString &sqlStr, QString &errorReason, QString &warningMsg )
{
  errorReason.clear();
  if ( sqlStr.isEmpty() || sqlStr == mAllSql )
    return true;

  QgsWFSDataSourceURI uri( mURI );
  uri.setSql( sqlStr );

  QgsDataProvider::ProviderOptions options;
  QgsWFSProvider p( uri.uri(), options, mCaps );
  if ( !p.isValid() )
  {
    errorReason = p.processSQLErrorMsg();
    return false;
  }
  warningMsg = p.processSQLWarningMsg();

  return true;
}

QgsWFSTableSelectedCallback::QgsWFSTableSelectedCallback( QgsSQLComposerDialog *dialog, const QgsWFSDataSourceURI &uri, const QgsWfsCapabilities::Capabilities &caps )
  : QObject( dialog )
  , mDialog( dialog )
  , mURI( uri )
  , mCaps( caps )
{
}

void QgsWFSTableSelectedCallback::tableSelected( const QString &name )
{
  QString typeName( QgsSQLStatement::stripQuotedIdentifier( name ) );
  QString prefixedTypename( mCaps.addPrefixIfNeeded( typeName ) );
  if ( prefixedTypename.isEmpty() )
    return;
  QgsWFSDataSourceURI uri( mURI );
  uri.setTypeName( prefixedTypename );

  QgsDataProvider::ProviderOptions providerOptions;
  QgsWFSProvider p( uri.uri(), providerOptions, mCaps );
  if ( !p.isValid() )
  {
    return;
  }

  QList<QgsSQLComposerDialog::PairNameType> fieldList;
  QString fieldNamePrefix( QgsSQLStatement::quotedIdentifierIfNeeded( typeName ) + "." );
  const auto constToList = p.fields().toList();
  for ( const QgsField &field : constToList )
  {
    QString fieldName( fieldNamePrefix + QgsSQLStatement::quotedIdentifierIfNeeded( field.name() ) );
    fieldList << QgsSQLComposerDialog::PairNameType( fieldName, field.typeName() );
  }
  if ( !p.geometryAttribute().isEmpty() )
  {
    QString fieldName( fieldNamePrefix + QgsSQLStatement::quotedIdentifierIfNeeded( p.geometryAttribute() ) );
    fieldList << QgsSQLComposerDialog::PairNameType( fieldName, QStringLiteral( "geometry" ) );
  }
  fieldList << QgsSQLComposerDialog::PairNameType( fieldNamePrefix + "*", QString() );

  mDialog->addColumnNames( fieldList, name );
}
