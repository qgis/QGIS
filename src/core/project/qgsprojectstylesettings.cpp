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

#include "qgscolorutils.h"
#include "qgsprojectstylesettings.h"
#include "moc_qgsprojectstylesettings.cpp"
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
#include "qgscombinedstylemodel.h"
#include "qgsxmlutils.h"

#include <QDomElement>

QgsProjectStyleSettings::QgsProjectStyleSettings( QgsProject *project )
  : QObject( project )
  , mProject( project )
{
  mCombinedStyleModel = new QgsCombinedStyleModel( this );
}

QgsProjectStyleSettings::~QgsProjectStyleSettings()
{
  if ( mProjectStyle )
  {
    mProjectStyle->deleteLater();
    mProjectStyle = nullptr;
  }
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
      if ( mDefaultMarkerSymbol.get() == symbol )
        return;

      mDefaultMarkerSymbol.reset( symbol ? symbol->clone() : nullptr );

      makeDirty();
      break;

    case Qgis::SymbolType::Line:
      if ( mDefaultLineSymbol.get() == symbol )
        return;

      mDefaultLineSymbol.reset( symbol ? symbol->clone() : nullptr );

      makeDirty();
      break;

    case Qgis::SymbolType::Fill:
      if ( mDefaultFillSymbol.get() == symbol )
        return;

      mDefaultFillSymbol.reset( symbol ? symbol->clone() : nullptr );

      makeDirty();
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
  if ( mDefaultColorRamp.get() == colorRamp )
    return;

  mDefaultColorRamp.reset( colorRamp ? colorRamp->clone() : nullptr );

  makeDirty();
}

QgsTextFormat QgsProjectStyleSettings::defaultTextFormat() const
{
  return mDefaultTextFormat;
}

void QgsProjectStyleSettings::setDefaultTextFormat( const QgsTextFormat &textFormat )
{
  if ( mDefaultTextFormat == textFormat )
    return;

  mDefaultTextFormat = textFormat;

  makeDirty();
}

void QgsProjectStyleSettings::setDefaultSymbolOpacity( double opacity )
{
  if ( qgsDoubleNear( mDefaultSymbolOpacity, opacity ) )
    return;

  mDefaultSymbolOpacity = opacity;

  makeDirty();
}

void QgsProjectStyleSettings::setRandomizeDefaultSymbolColor( bool randomized )
{
  if ( mRandomizeDefaultSymbolColor == randomized )
    return;

  mRandomizeDefaultSymbolColor = randomized;

  makeDirty();
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

  if ( mProject && ( mProject->capabilities() & Qgis::ProjectCapability::ProjectStyles ) )
  {
    const QString stylePath = mProject->createAttachedFile( QStringLiteral( "styles.db" ) );
    QgsStyle *style = new QgsStyle();
    style->createDatabase( stylePath );
    style->setName( tr( "Project Style" ) );
    style->setFileName( stylePath );
    setProjectStyle( style );
  }

  emit styleDatabasesChanged();
}

void QgsProjectStyleSettings::removeProjectStyle()
{
  if ( mProjectStyle )
  {
    mCombinedStyleModel->removeStyle( mProjectStyle );
    delete mProjectStyle;
    mProjectStyle = nullptr;
  }
}

void QgsProjectStyleSettings::setProjectStyle( QgsStyle *style )
{
  if ( mProjectStyle )
  {
    mCombinedStyleModel->removeStyle( mProjectStyle );
    mProjectStyle->deleteLater();
  }
  mProjectStyle = style;
  mProjectStyle->setName( tr( "Project Styles" ) );

  // if project color scheme changes, we need to redraw symbols - they may use project colors and accordingly
  // need updating to reflect the new colors
  if ( mProject )
  {
    connect( mProject, &QgsProject::projectColorsChanged, mProjectStyle, &QgsStyle::triggerIconRebuild );
  }
  mCombinedStyleModel->addStyle( mProjectStyle );

  emit projectStyleChanged();
}

QgsStyle *QgsProjectStyleSettings::projectStyle()
{
  return mProjectStyle;
}

bool QgsProjectStyleSettings::readXml( const QDomElement &element, const QgsReadWriteContext &context, Qgis::ProjectReadFlags )
{
  mRandomizeDefaultSymbolColor = element.attribute( QStringLiteral( "RandomizeDefaultSymbolColor" ), QStringLiteral( "0" ) ).toInt();
  mDefaultSymbolOpacity = element.attribute( QStringLiteral( "DefaultSymbolOpacity" ), QStringLiteral( "1.0" ) ).toDouble();
  mColorModel = qgsEnumKeyToValue( element.attribute( QStringLiteral( "colorModel" ) ), Qgis::ColorModel::Rgb );

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
    if ( !mProject || ( mProject->capabilities() & Qgis::ProjectCapability::ProjectStyles ) )
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

      if ( mProject && ( mProject->capabilities() & Qgis::ProjectCapability::ProjectStyles ) )
      {
        const QString projectStyleId = element.attribute( QStringLiteral( "projectStyleId" ) );
        const QString projectStyleFile = mProject->resolveAttachmentIdentifier( projectStyleId );
        QgsStyle *style = new QgsStyle();
        if ( !projectStyleFile.isEmpty() && QFile::exists( projectStyleFile ) )
        {
          style->load( projectStyleFile );
          style->setFileName( projectStyleFile );
        }
        else
        {
          const QString stylePath = mProject->createAttachedFile( QStringLiteral( "styles.db" ) );
          style->createDatabase( stylePath );
          style->setFileName( stylePath );
        }
        style->setName( tr( "Project Style" ) );
        setProjectStyle( style );
      }
    }
  }

  const QString iccProfileId = element.attribute( QStringLiteral( "iccProfileId" ) );
  mIccProfileFilePath = mProject ? mProject->resolveAttachmentIdentifier( iccProfileId ) : QString();
  if ( !mIccProfileFilePath.isEmpty() )
  {
    QString errorMsg;
    QColorSpace colorSpace = QgsColorUtils::iccProfile( mIccProfileFilePath, errorMsg );
    if ( !errorMsg.isEmpty() )
      context.pushMessage( errorMsg );

    setColorSpace( colorSpace );
  }

  emit styleDatabasesChanged();

  return true;
}

QDomElement QgsProjectStyleSettings::writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement element = doc.createElement( QStringLiteral( "ProjectStyleSettings" ) );

  element.setAttribute( QStringLiteral( "RandomizeDefaultSymbolColor" ), mRandomizeDefaultSymbolColor ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  element.setAttribute( QStringLiteral( "DefaultSymbolOpacity" ), QString::number( mDefaultSymbolOpacity ) );

  element.setAttribute( QStringLiteral( "colorModel" ), qgsEnumValueToKey( mColorModel ) );

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

  if ( mProject && mProjectStyle )
  {
    element.setAttribute( QStringLiteral( "projectStyleId" ), mProject->attachmentIdentifier( mProjectStyle->fileName() ) );
  }

  if ( mProject )
  {
    element.setAttribute( QStringLiteral( "iccProfileId" ), mProject->attachmentIdentifier( mIccProfileFilePath ) );
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

  if ( mProjectStyle && path == mProjectStyle->fileName() )
    return mProjectStyle;

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
  mCombinedStyleModel->addStyle( style );

  if ( mProject )
  {
    // if project color scheme changes, we need to redraw symbols - they may use project colors and accordingly
    // need updating to reflect the new colors
    connect( mProject, &QgsProject::projectColorsChanged, style, &QgsStyle::triggerIconRebuild );
  }
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
      mCombinedStyleModel->removeStyle( style );
      style->deleteLater();
      mStyles.removeAll( style );
    }
    emit styleDatabaseRemoved( path );
  }

  // should already be empty, but play it safe..!
  for ( QgsStyle *style : std::as_const( mStyles ) )
  {
    mCombinedStyleModel->removeStyle( style );
  }
  qDeleteAll( mStyles );
  mStyles.clear();
}

QgsCombinedStyleModel *QgsProjectStyleSettings::combinedStyleModel()
{
  return mCombinedStyleModel;
}

void QgsProjectStyleSettings::setColorModel( Qgis::ColorModel colorModel )
{
  if ( mColorModel == colorModel )
    return;

  mColorModel = colorModel;

  makeDirty();

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
  if ( mColorSpace.isValid() && QgsColorUtils::toColorModel( mColorSpace.colorModel() ) != colorModel )
  {
    setColorSpace( QColorSpace() );
  }
#endif
}

Qgis::ColorModel QgsProjectStyleSettings::colorModel() const
{
  return mColorModel;
}

void QgsProjectStyleSettings::setColorSpace( const QColorSpace &colorSpace )
{
  if ( mColorSpace == colorSpace )
    return;

  if ( !mProject )
  {
    QgsDebugError( "Impossible to attach ICC profile, no project defined" );
    return;
  }

  auto clearIccProfile = [this]()
  {
    mProject->removeAttachedFile( mIccProfileFilePath );
    mIccProfileFilePath.clear();
    mColorSpace = QColorSpace();
  };

  if ( !mIccProfileFilePath.isEmpty() )
    clearIccProfile();

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
  bool ok;
  Qgis::ColorModel colorModel = QgsColorUtils::toColorModel( colorSpace.colorModel(), &ok );
  mColorSpace = ok ? colorSpace : QColorSpace();
#else
  mColorSpace = colorSpace;
#endif

  makeDirty();

  if ( !mColorSpace.isValid() )
    return;

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
  if ( colorModel != mColorModel )
    mColorModel = colorModel;
#endif

  mIccProfileFilePath = mProject->createAttachedFile( QStringLiteral( "profile.icc" ) );
  QFile file( mIccProfileFilePath );
  if ( !file.open( QIODevice::WriteOnly ) || file.write( colorSpace.iccProfile() ) < 0 )
    clearIccProfile();
}

QColorSpace QgsProjectStyleSettings::colorSpace() const
{
  return mColorSpace;
}

void QgsProjectStyleSettings::makeDirty()
{
  if ( mProject )
    mProject->setDirty( true );
}

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

  if ( mSettings->projectStyle() )
    setProjectStyle( mSettings->projectStyle() );
  connect( mSettings, &QgsProjectStyleSettings::projectStyleChanged, this, &QgsProjectStyleDatabaseModel::projectStyleChanged );
}

int QgsProjectStyleDatabaseModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return ( mSettings ? mSettings->styleDatabasePaths().count() : 0 ) + ( mProjectStyle ? 1 : 0 ) + ( mShowDefault ? 1 : 0 );
}

QVariant QgsProjectStyleDatabaseModel::data( const QModelIndex &index, int role ) const
{
  if ( index.row() < 0 || index.row() >= rowCount( QModelIndex() ) )
    return QVariant();

  const bool isProjectStyle = index.row() == 0 && mProjectStyle;
  const bool isDefault = mShowDefault && ( ( index.row() == 0 && !mProjectStyle ) || ( index.row() == 1 && mProjectStyle ) );
  const int styleRow = index.row() - ( mShowDefault ? 1 : 0 ) - ( mProjectStyle ? 1 : 0 );

  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::EditRole:
      if ( isDefault )
        return QgsStyle::defaultStyle()->name();
      else if ( isProjectStyle )
        return mProjectStyle->name();
      else
        return mSettings ? mSettings->styles().at( styleRow )->name() : QVariant();

    case Qt::ToolTipRole:
      if ( isDefault )
        return QDir::toNativeSeparators( QgsStyle::defaultStyle()->fileName() );
      else if ( isProjectStyle )
        return mProjectStyle->name();
      else
        return mSettings ? QDir::toNativeSeparators( mSettings->styles().at( styleRow )->fileName() ) : QVariant();

    case static_cast< int >( CustomRole::Style ):
    {
      if ( isDefault )
        return QVariant::fromValue( QgsStyle::defaultStyle() );
      else if ( isProjectStyle )
        return QVariant::fromValue( mProjectStyle.data() );
      else if ( QgsStyle *style = mSettings->styles().value( styleRow ) )
        return QVariant::fromValue( style );
      else
        return QVariant();
    }

    case static_cast< int >( CustomRole::Path ):
      if ( isDefault )
        return QgsStyle::defaultStyle()->fileName();
      else if ( isProjectStyle )
        return mProjectStyle->fileName();
      else
        return mSettings ? mSettings->styles().at( styleRow )->fileName() : QVariant();

    default:
      return QVariant();
  }
}

QgsStyle *QgsProjectStyleDatabaseModel::styleFromIndex( const QModelIndex &index ) const
{
  if ( index.row() == 0 && mProjectStyle )
    return mProjectStyle;
  else if ( mShowDefault && ( ( index.row() == 0 && !mProjectStyle ) || ( index.row() == 1 && mProjectStyle ) ) )
    return QgsStyle::defaultStyle();
  else if ( QgsStyle *style = qobject_cast< QgsStyle * >( qvariant_cast<QObject *>( data( index, static_cast< int >( CustomRole::Style ) ) ) ) )
    return style;
  else
    return nullptr;
}

QModelIndex QgsProjectStyleDatabaseModel::indexFromStyle( QgsStyle *style ) const
{
  if ( style == mProjectStyle )
    return index( 0, 0, QModelIndex() );
  else if ( style == QgsStyle::defaultStyle() && mShowDefault )
    return index( mProjectStyle ? 1 : 0, 0, QModelIndex() );

  if ( !mSettings )
  {
    return QModelIndex();
  }

  const int r = mSettings->styles().indexOf( style );
  if ( r < 0 )
    return QModelIndex();

  QModelIndex idx = index( r + ( mShowDefault ? 1 : 0 ) + ( mProjectStyle ? 1 : 0 ), 0, QModelIndex() );
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

  const int row = mProjectStyle ? 1 : 0;
  if ( show )
  {
    beginInsertRows( QModelIndex(), row, row );
    mShowDefault = true;
    endInsertRows();
  }
  else
  {
    beginRemoveRows( QModelIndex(), row, row );
    mShowDefault = false;
    endRemoveRows();
  }
}

void QgsProjectStyleDatabaseModel::setProjectStyle( QgsStyle *style )
{
  if ( style == mProjectStyle )
    return;

  if ( mProjectStyle )
  {
    disconnect( mProjectStyle, &QgsStyle::aboutToBeDestroyed, this, &QgsProjectStyleDatabaseModel::projectStyleAboutToBeDestroyed );
    disconnect( mProjectStyle, &QgsStyle::destroyed, this, &QgsProjectStyleDatabaseModel::projectStyleDestroyed );
    beginRemoveRows( QModelIndex(), 0, 0 );
    mProjectStyle = nullptr;
    endRemoveRows();
  }

  if ( style )
  {
    beginInsertRows( QModelIndex(), 0, 0 );
    mProjectStyle = style;
    endInsertRows();

    connect( mProjectStyle, &QgsStyle::aboutToBeDestroyed, this, &QgsProjectStyleDatabaseModel::projectStyleAboutToBeDestroyed );
    connect( mProjectStyle, &QgsStyle::destroyed, this, &QgsProjectStyleDatabaseModel::projectStyleDestroyed );
  }
}

void QgsProjectStyleDatabaseModel::styleDatabaseAboutToBeAdded( const QString & )
{
  int row = mSettings->styles().count() + ( mShowDefault ? 1 : 0 ) + ( mProjectStyle ? 1 : 0 );
  beginInsertRows( QModelIndex(), row, row );
}

void QgsProjectStyleDatabaseModel::styleDatabaseAboutToBeRemoved( const QString &path )
{
  QgsStyle *style = mSettings->styleAtPath( path );
  int row = mSettings->styles().indexOf( style ) + ( mShowDefault ? 1 : 0 ) + ( mProjectStyle ? 1 : 0 );
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

void QgsProjectStyleDatabaseModel::projectStyleAboutToBeDestroyed()
{
  beginRemoveRows( QModelIndex(), 0, 0 );
}

void QgsProjectStyleDatabaseModel::projectStyleDestroyed()
{
  endRemoveRows();
}

void QgsProjectStyleDatabaseModel::projectStyleChanged()
{
  setProjectStyle( mSettings->projectStyle() );
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
    if ( const QgsStyle *style = qobject_cast< QgsStyle * >( sourceModel()->data( sourceModel()->index( sourceRow, 0, sourceParent ), static_cast< int >( QgsProjectStyleDatabaseModel::CustomRole::Style ) ).value< QObject * >() ) )
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
