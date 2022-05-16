/***************************************************************************
    qgsprojectstylesettings.cpp
    ---------------------------
    begin                : May 2022
    copyright            : (C) 2022 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprojectstylesettings.h"
#include "qgis.h"
#include "qgsproject.h"
#include "qgssymbol.h"
#include "qgssymbollayerutils.h"
#include "qgsmarkersymbol.h"
#include "qgslinesymbol.h"
#include "qgsfillsymbol.h"
#include "qgscolorramp.h"
#include "qgstextformat.h"
#include "qgsstyle.h"

#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
#include "qgscombinedstylemodel.h"
#endif

#include <QDomElement>

QgsProjectStyleSettings::QgsProjectStyleSettings( QgsProject *project )
  : QObject( project )
  , mProject( project )
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
  mCombinedStyleModel = new QgsCombinedStyleModel( this );
#endif
}

QgsSymbol *QgsProjectStyleSettings::defaultSymbol( Qgis::SymbolType symbolType ) const
{
  switch ( symbolType )
  {
    case Qgis::SymbolType::Marker:
      return mDefaultMarkerSymbol ? mDefaultMarkerSymbol->clone() : nullptr;

    case Qgis::SymbolType::Line:
      return mDefaultLineSymbol ? mDefaultLineSymbol->clone() : nullptr;

    case Qgis::SymbolType::Fill:
      return mDefaultFillSymbol ? mDefaultFillSymbol->clone() : nullptr;

    case Qgis::SymbolType::Hybrid:
      break;
  }

  return nullptr;
}

void QgsProjectStyleSettings::setDefaultSymbol( Qgis::SymbolType symbolType, QgsSymbol *symbol )
{
  switch ( symbolType )
  {
    case Qgis::SymbolType::Marker:
      mDefaultMarkerSymbol.reset( symbol ? symbol->clone() : nullptr );
      break;

    case Qgis::SymbolType::Line:
      mDefaultLineSymbol.reset( symbol ? symbol->clone() : nullptr );
      break;

    case Qgis::SymbolType::Fill:
      mDefaultFillSymbol.reset( symbol ? symbol->clone() : nullptr );
      break;

    case Qgis::SymbolType::Hybrid:
      break;
  }
}

QgsColorRamp *QgsProjectStyleSettings::defaultColorRamp() const
{
  return mDefaultColorRamp ? mDefaultColorRamp->clone() : nullptr;
}

void QgsProjectStyleSettings::setDefaultColorRamp( QgsColorRamp *colorRamp )
{
  mDefaultColorRamp.reset( colorRamp ? colorRamp->clone() : nullptr );
}

QgsTextFormat QgsProjectStyleSettings::defaultTextFormat() const
{
  return mDefaultTextFormat;
}

void QgsProjectStyleSettings::setDefaultTextFormat( const QgsTextFormat &textFormat )
{
  mDefaultTextFormat = textFormat;
}

void QgsProjectStyleSettings::reset()
{
  mDefaultMarkerSymbol.reset();
  mDefaultLineSymbol.reset();
  mDefaultFillSymbol.reset();
  mDefaultColorRamp.reset();
  mDefaultTextFormat = QgsTextFormat();
  mRandomizeDefaultSymbolColor = true;
  mDefaultSymbolOpacity = 1.0;

  clearStyles();
  emit styleDatabasesChanged();
}

bool QgsProjectStyleSettings::readXml( const QDomElement &element, const QgsReadWriteContext &context, Qgis::ProjectReadFlags flags )
{
  mRandomizeDefaultSymbolColor = element.attribute( QStringLiteral( "RandomizeDefaultSymbolColor" ), QStringLiteral( "0" ) ).toInt();
  mDefaultSymbolOpacity = element.attribute( QStringLiteral( "DefaultSymbolOpacity" ), QStringLiteral( "1.0" ) ).toDouble();

  QDomElement elem = element.firstChildElement( QStringLiteral( "markerSymbol" ) );
  if ( !elem.isNull() )
  {
    QDomElement symbolElem = elem.firstChildElement( QStringLiteral( "symbol" ) );
    mDefaultMarkerSymbol.reset( !symbolElem.isNull() ? QgsSymbolLayerUtils::loadSymbol<QgsMarkerSymbol>( symbolElem, context ) : nullptr );
  }
  else
  {
    mDefaultMarkerSymbol.reset();
  }

  elem = element.firstChildElement( QStringLiteral( "lineSymbol" ) );
  if ( !elem.isNull() )
  {
    QDomElement symbolElem = elem.firstChildElement( QStringLiteral( "symbol" ) );
    mDefaultLineSymbol.reset( !symbolElem.isNull() ? QgsSymbolLayerUtils::loadSymbol<QgsLineSymbol>( symbolElem, context ) : nullptr );
  }
  else
  {
    mDefaultLineSymbol.reset();
  }

  elem = element.firstChildElement( QStringLiteral( "fillSymbol" ) );
  if ( !elem.isNull() )
  {
    QDomElement symbolElem = elem.firstChildElement( QStringLiteral( "symbol" ) );
    mDefaultFillSymbol.reset( !symbolElem.isNull() ? QgsSymbolLayerUtils::loadSymbol<QgsFillSymbol>( symbolElem, context ) : nullptr );
  }
  else
  {
    mDefaultFillSymbol.reset();
  }

  elem = element.firstChildElement( QStringLiteral( "colorramp" ) );
  mDefaultColorRamp.reset( !elem.isNull() ? QgsSymbolLayerUtils::loadColorRamp( elem ) : nullptr );

  elem = element.firstChildElement( QStringLiteral( "text-style" ) );
  if ( !elem.isNull() )
  {
    mDefaultTextFormat.readXml( elem, context );
  }
  else
  {
    mDefaultTextFormat = QgsTextFormat();
  }

  {
    clearStyles();
    if ( !( flags & Qgis::ProjectReadFlag::DontLoadProjectStyles ) )
    {
      const QDomElement styleDatabases = element.firstChildElement( QStringLiteral( "databases" ) );
      if ( !styleDatabases.isNull() )
      {
        const QDomNodeList styleEntries = styleDatabases.childNodes();
        for ( int i = 0; i < styleEntries.count(); ++i )
        {
          const QDomElement styleElement = styleEntries.at( i ).toElement();
          const QString path = styleElement.attribute( QStringLiteral( "path" ) );
          const QString fullPath = context.pathResolver().readPath( path );
          emit styleDatabaseAboutToBeAdded( fullPath );
          mStyleDatabases.append( fullPath );
          loadStyleAtPath( fullPath );
          emit styleDatabaseAdded( fullPath );
        }
      }
    }
  }
  emit styleDatabasesChanged();

  return true;
}

QDomElement QgsProjectStyleSettings::writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement element = doc.createElement( QStringLiteral( "ProjectStyleSettings" ) );

  element.setAttribute( QStringLiteral( "RandomizeDefaultSymbolColor" ), mRandomizeDefaultSymbolColor ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  element.setAttribute( QStringLiteral( "DefaultSymbolOpacity" ), QString::number( mDefaultSymbolOpacity ) );

  if ( mDefaultMarkerSymbol )
  {
    QDomElement markerSymbolElem = doc.createElement( QStringLiteral( "markerSymbol" ) );
    markerSymbolElem.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mDefaultMarkerSymbol.get(), doc, context ) );
    element.appendChild( markerSymbolElem );
  }

  if ( mDefaultLineSymbol )
  {
    QDomElement lineSymbolElem = doc.createElement( QStringLiteral( "lineSymbol" ) );
    lineSymbolElem.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mDefaultLineSymbol.get(), doc, context ) );
    element.appendChild( lineSymbolElem );
  }

  if ( mDefaultFillSymbol )
  {
    QDomElement fillSymbolElem = doc.createElement( QStringLiteral( "fillSymbol" ) );
    fillSymbolElem.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mDefaultFillSymbol.get(), doc, context ) );
    element.appendChild( fillSymbolElem );
  }

  if ( mDefaultColorRamp )
  {
    QDomElement colorRampElem = QgsSymbolLayerUtils::saveColorRamp( QString(), mDefaultColorRamp.get(), doc );
    element.appendChild( colorRampElem );
  }

  if ( mDefaultTextFormat.isValid() )
  {
    QDomElement textFormatElem = mDefaultTextFormat.writeXml( doc, context );
    element.appendChild( textFormatElem );
  }

  {
    QDomElement styleDatabases = doc.createElement( QStringLiteral( "databases" ) );
    for ( const QString &db : mStyleDatabases )
    {
      QDomElement dbElement = doc.createElement( QStringLiteral( "db" ) );
      dbElement.setAttribute( QStringLiteral( "path" ), context.pathResolver().writePath( db ) );
      styleDatabases.appendChild( dbElement );
    }
    element.appendChild( styleDatabases );
  }

  return element;
}

QList<QgsStyle *> QgsProjectStyleSettings::styles() const
{
  QList< QgsStyle * > res;
  res.reserve( mStyles.size() );
  for ( QgsStyle *style : mStyles )
  {
    if ( style )
      res.append( style );
  }
  return res;
}

QgsStyle *QgsProjectStyleSettings::styleAtPath( const QString &path )
{
  if ( path == QgsStyle::defaultStyle()->fileName() )
    return QgsStyle::defaultStyle();

  for ( QgsStyle *style : std::as_const( mStyles ) )
  {
    if ( style->fileName() == path )
      return style;
  }

  return nullptr;
}

void QgsProjectStyleSettings::addStyleDatabasePath( const QString &path )
{
  if ( mStyleDatabases.contains( path ) )
    return;

  emit styleDatabaseAboutToBeAdded( path );
  mStyleDatabases.append( path );
  loadStyleAtPath( path );
  emit styleDatabaseAdded( path );

  emit styleDatabasesChanged();
}

void QgsProjectStyleSettings::setStyleDatabasePaths( const QStringList &paths )
{
  if ( paths == mStyleDatabases )
    return;

  clearStyles();

  for ( const QString &path : paths )
  {
    emit styleDatabaseAboutToBeAdded( path );
    mStyleDatabases.append( path );
    loadStyleAtPath( path );
    emit styleDatabaseAdded( path );
  }
  emit styleDatabasesChanged();
}

void QgsProjectStyleSettings::loadStyleAtPath( const QString &path )
{
  QgsStyle *style = new QgsStyle( this );

  const QFileInfo fileInfo( path );
  if ( fileInfo.suffix().compare( QLatin1String( "xml" ), Qt::CaseInsensitive ) == 0 )
  {
    style->createMemoryDatabase();
    style->importXml( path );
    style->setFileName( path );
    style->setReadOnly( true );
  }
  else
  {
    style->load( path );
  }
  style->setName( fileInfo.completeBaseName() );
  mStyles.append( style );
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
  mCombinedStyleModel->addStyle( style );
#endif
}

void QgsProjectStyleSettings::clearStyles()
{
  const QStringList pathsToRemove = mStyleDatabases;
  for ( const QString &path : pathsToRemove )
  {
    emit styleDatabaseAboutToBeRemoved( path );
    mStyleDatabases.removeAll( path );
    if ( QgsStyle *style = styleAtPath( path ) )
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
      mCombinedStyleModel->removeStyle( style );
#endif
      style->deleteLater();
      mStyles.removeAll( style );
    }
    emit styleDatabaseRemoved( path );
  }

  // should already be empty, but play it safe..!
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
  for ( QgsStyle *style : std::as_const( mStyles ) )
  {
    mCombinedStyleModel->removeStyle( style );
  }
#endif
  qDeleteAll( mStyles );
  mStyles.clear();
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
QgsCombinedStyleModel *QgsProjectStyleSettings::combinedStyleModel()
{
  return mCombinedStyleModel;
}
#endif




//
// QgsProjectStyleDatabaseModel
//

QgsProjectStyleDatabaseModel::QgsProjectStyleDatabaseModel( QgsProjectStyleSettings *settings, QObject *parent )
  : QAbstractListModel( parent )
  , mSettings( settings )
{
  connect( mSettings, &QgsProjectStyleSettings::styleDatabaseAboutToBeAdded, this, &QgsProjectStyleDatabaseModel::styleDatabaseAboutToBeAdded );
  connect( mSettings, &QgsProjectStyleSettings::styleDatabaseAdded, this, &QgsProjectStyleDatabaseModel::styleDatabaseAdded );
  connect( mSettings, &QgsProjectStyleSettings::styleDatabaseAboutToBeRemoved, this, &QgsProjectStyleDatabaseModel::styleDatabaseAboutToBeRemoved );
  connect( mSettings, &QgsProjectStyleSettings::styleDatabaseRemoved, this, &QgsProjectStyleDatabaseModel::styleDatabaseRemoved );
}

int QgsProjectStyleDatabaseModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return ( mSettings ? mSettings->styleDatabasePaths().count() : 0 ) + ( mShowDefault ? 1 : 0 );
}

QVariant QgsProjectStyleDatabaseModel::data( const QModelIndex &index, int role ) const
{
  if ( index.row() < 0 || index.row() >= rowCount( QModelIndex() ) )
    return QVariant();

  const bool isDefault = index.row() == 0 && mShowDefault;
  const int styleRow = mShowDefault ? index.row() - 1 : index.row();

  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::EditRole:
      if ( isDefault )
        return QgsStyle::defaultStyle()->name();
      else
        return mSettings ? mSettings->styles().at( styleRow )->name() : QVariant();

    case Qt::ToolTipRole:
      if ( isDefault )
        return QDir::toNativeSeparators( QgsStyle::defaultStyle()->fileName() );
      else
        return mSettings ? QDir::toNativeSeparators( mSettings->styles().at( styleRow )->fileName() ) : QVariant();

    case StyleRole:
    {
      if ( isDefault )
        return QVariant::fromValue( QgsStyle::defaultStyle() );
      else if ( QgsStyle *style = mSettings->styles().value( styleRow ) )
        return QVariant::fromValue( style );
      else
        return QVariant();
    }

    case PathRole:
      if ( isDefault )
        return QgsStyle::defaultStyle()->fileName();
      else
        return mSettings ? mSettings->styles().at( styleRow )->fileName() : QVariant();

    default:
      return QVariant();
  }
}

QgsStyle *QgsProjectStyleDatabaseModel::styleFromIndex( const QModelIndex &index ) const
{
  if ( index.row() == 0 && mShowDefault )
    return QgsStyle::defaultStyle();

  if ( QgsStyle *style = qobject_cast< QgsStyle * >( qvariant_cast<QObject *>( data( index, StyleRole ) ) ) )
    return style;
  else
    return nullptr;
}

QModelIndex QgsProjectStyleDatabaseModel::indexFromStyle( QgsStyle *style ) const
{
  if ( style == QgsStyle::defaultStyle() && mShowDefault )
    return index( 0, 0, QModelIndex() );

  if ( !mSettings )
  {
    return QModelIndex();
  }

  const int r = mSettings->styles().indexOf( style );
  if ( r < 0 )
    return QModelIndex();

  QModelIndex idx = index( mShowDefault ? r + 1 : r, 0, QModelIndex() );
  if ( idx.isValid() )
  {
    return idx;
  }

  return QModelIndex();
}

void QgsProjectStyleDatabaseModel::setShowDefaultStyle( bool show )
{
  if ( show == mShowDefault )
    return;

  if ( show )
  {
    beginInsertRows( QModelIndex(), 0, 0 );
    mShowDefault = true;
    endInsertRows();
  }
  else
  {
    beginRemoveRows( QModelIndex(), 0, 0 );
    mShowDefault = false;
    endRemoveRows();
  }
}

void QgsProjectStyleDatabaseModel::styleDatabaseAboutToBeAdded( const QString & )
{
  int row = mSettings->styles().count() + ( mShowDefault ? 1 : 0 );
  beginInsertRows( QModelIndex(), row, row );
}

void QgsProjectStyleDatabaseModel::styleDatabaseAboutToBeRemoved( const QString &path )
{
  QgsStyle *style = mSettings->styleAtPath( path );
  int row = mSettings->styles().indexOf( style ) + ( mShowDefault ? 1 : 0 );
  if ( row >= 0 )
    beginRemoveRows( QModelIndex(), row, row );
}

void QgsProjectStyleDatabaseModel::styleDatabaseAdded( const QString & )
{
  endInsertRows();
}

void QgsProjectStyleDatabaseModel::styleDatabaseRemoved( const QString & )
{
  endRemoveRows();
}

//
// QgsProjectStyleDatabaseProxyModel
//

QgsProjectStyleDatabaseProxyModel::QgsProjectStyleDatabaseProxyModel( QgsProjectStyleDatabaseModel *model, QObject *parent )
  : QSortFilterProxyModel( parent )
{
  setSourceModel( model );
  setDynamicSortFilter( true );
}

bool QgsProjectStyleDatabaseProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  if ( mFilters & Filter::FilterHideReadOnly )
  {
    if ( const QgsStyle *style = qobject_cast< QgsStyle * >( sourceModel()->data( sourceModel()->index( sourceRow, 0, sourceParent ), QgsProjectStyleDatabaseModel::Role::StyleRole ).value< QObject * >() ) )
    {
      if ( style->isReadOnly() )
        return false;
    }
  }

  return true;
}

QgsProjectStyleDatabaseProxyModel::Filters QgsProjectStyleDatabaseProxyModel::filters() const
{
  return mFilters;
}

void QgsProjectStyleDatabaseProxyModel::setFilters( QgsProjectStyleDatabaseProxyModel::Filters filters )
{
  mFilters = filters;
  invalidateFilter();
}
