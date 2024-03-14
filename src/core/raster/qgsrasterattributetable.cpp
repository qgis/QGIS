/***************************************************************************
  qgsrasterattributetable.cpp - QgsRasterAttributeTable

 ---------------------
 begin                : 3.12.2021
 copyright            : (C) 2021 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterattributetable.h"
#include "qgsvectorfilewriter.h"
#include "qgsogrprovider.h"
#include "qgsfileutils.h"
#include "qgsrasterlayer.h"
#include "qgspalettedrasterrenderer.h"
#include "qgssinglebandpseudocolorrenderer.h"
#include "qgsrastershader.h"
#include "qgsrastershaderfunction.h"

#include <QLocale>

#include <mutex>
#include <cmath>

///@cond private
std::once_flag usageInformationLoaderFlag;
QHash<Qgis::RasterAttributeTableFieldUsage, QgsRasterAttributeTable::UsageInformation> QgsRasterAttributeTable::sUsageInformation;
///@endcond private


Qgis::RasterAttributeTableType QgsRasterAttributeTable::type() const
{
  return mType;
}


bool QgsRasterAttributeTable::hasColor() const
{
  const QList<Qgis::RasterAttributeTableFieldUsage> fieldUsages { usages() };
  return fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::Red ) && fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::Green ) && fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::Blue );
}

bool QgsRasterAttributeTable::setColor( const int row, const QColor &color )
{
  if ( ! hasColor() || row < 0 || row >= mData.count() )
  {
    return false;
  }

  for ( int idx = 0; idx < mFields.count(); ++idx )
  {
    const Field f { mFields.at( idx ) };
    switch ( f.usage )
    {
      case Qgis::RasterAttributeTableFieldUsage::Red:
        setValue( row, idx, color.red() );
        break;
      case Qgis::RasterAttributeTableFieldUsage::Green:
        setValue( row, idx, color.green() );
        break;
      case Qgis::RasterAttributeTableFieldUsage::Blue:
        setValue( row, idx, color.blue() );
        break;
      case Qgis::RasterAttributeTableFieldUsage::Alpha:
        setValue( row, idx, color.alpha() );
        break;
      default:
        break;
    }
  }
  return true;
}


bool QgsRasterAttributeTable::setRamp( const int row, const QColor &colorMin, const QColor &colorMax )
{
  if ( ! hasRamp() || row < 0 || row >= mData.count() )
  {
    return false;
  }

  int idx = 0;
  for ( Field &f : mFields )
  {
    switch ( f.usage )
    {
      case Qgis::RasterAttributeTableFieldUsage::RedMin:
        setValue( row, idx, colorMin.red() );
        break;
      case Qgis::RasterAttributeTableFieldUsage::GreenMin:
        setValue( row, idx, colorMin.green() );
        break;
      case Qgis::RasterAttributeTableFieldUsage::BlueMin:
        setValue( row, idx, colorMin.blue() );
        break;
      case Qgis::RasterAttributeTableFieldUsage::AlphaMin:
        setValue( row, idx, colorMin.alpha() );
        break;
      case Qgis::RasterAttributeTableFieldUsage::RedMax:
        setValue( row, idx, colorMax.red() );
        break;
      case Qgis::RasterAttributeTableFieldUsage::GreenMax:
        setValue( row, idx, colorMax.green() );
        break;
      case Qgis::RasterAttributeTableFieldUsage::BlueMax:
        setValue( row, idx, colorMax.blue() );
        break;
      case Qgis::RasterAttributeTableFieldUsage::AlphaMax:
        setValue( row, idx, colorMax.alpha() );
        break;
      default:
        break;
    }
    idx++;
  }
  return true;
}

bool QgsRasterAttributeTable::hasRamp() const
{
  const QList<Qgis::RasterAttributeTableFieldUsage> fieldUsages { usages() };
  return fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::RedMin ) && fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::GreenMin ) && fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::BlueMin ) && fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::RedMax ) && fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::GreenMax ) && fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::BlueMax );
}


QList<Qgis::RasterAttributeTableFieldUsage> QgsRasterAttributeTable::usages() const
{
  QList<Qgis::RasterAttributeTableFieldUsage> usages;
  for ( const QgsRasterAttributeTable::Field &field : std::as_const( mFields ) )
  {
    usages.push_back( field.usage );
  }
  return usages;
}

///@cond private
QList<int> QgsRasterAttributeTable::intUsages( ) const
{
  QList<int> usages;
  for ( const QgsRasterAttributeTable::Field &field : std::as_const( mFields ) )
  {
    usages.push_back( static_cast<int>( field.usage ) );
  }
  return usages;
}
///@endcond private

QColor QgsRasterAttributeTable::color( int row ) const
{
  QList<Qgis::RasterAttributeTableFieldUsage> fieldUsages { usages() };
  // No ramps support here
  if ( hasColor() && row < mData.count( )
       && fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::Red )
       && fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::Green )
       && fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::Blue ) )
  {
    const QVariantList rowData = mData.at( row );
    QColor color { rowData.at( fieldUsages.indexOf( Qgis::RasterAttributeTableFieldUsage::Red ) ).toInt(),
                   rowData.at( fieldUsages.indexOf( Qgis::RasterAttributeTableFieldUsage::Green ) ).toInt(),
                   rowData.at( fieldUsages.indexOf( Qgis::RasterAttributeTableFieldUsage::Blue ) ).toInt() };
    if ( fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::Alpha ) )
    {
      color.setAlpha( rowData.at( fieldUsages.indexOf( Qgis::RasterAttributeTableFieldUsage::Alpha ) ).toInt() );
    }
    return color;
  }
  return QColor();
}

QgsGradientColorRamp QgsRasterAttributeTable::ramp( int row ) const
{
  if ( ! hasRamp() || row < 0 || row >= mData.count() )
  {
    return QgsGradientColorRamp();
  }
  QList<Qgis::RasterAttributeTableFieldUsage> fieldUsages { usages() };
  const QVariantList rowData = mData.at( row );
  QColor colorMin { rowData.at( fieldUsages.indexOf( Qgis::RasterAttributeTableFieldUsage::RedMin ) ).toInt(),
                    rowData.at( fieldUsages.indexOf( Qgis::RasterAttributeTableFieldUsage::GreenMin ) ).toInt(),
                    rowData.at( fieldUsages.indexOf( Qgis::RasterAttributeTableFieldUsage::BlueMin ) ).toInt() };
  if ( fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::AlphaMin ) )
  {
    colorMin.setAlpha( rowData.at( fieldUsages.indexOf( Qgis::RasterAttributeTableFieldUsage::AlphaMin ) ).toInt() );
  }
  QColor colorMax { rowData.at( fieldUsages.indexOf( Qgis::RasterAttributeTableFieldUsage::RedMax ) ).toInt(),
                    rowData.at( fieldUsages.indexOf( Qgis::RasterAttributeTableFieldUsage::GreenMax ) ).toInt(),
                    rowData.at( fieldUsages.indexOf( Qgis::RasterAttributeTableFieldUsage::BlueMax ) ).toInt() };
  if ( fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::AlphaMax ) )
  {
    colorMax.setAlpha( rowData.at( fieldUsages.indexOf( Qgis::RasterAttributeTableFieldUsage::AlphaMax ) ).toInt() );
  }
  return QgsGradientColorRamp( colorMin, colorMax );
}

QList<QgsRasterAttributeTable::Field> QgsRasterAttributeTable::fields() const
{
  return mFields;
}

QgsFields QgsRasterAttributeTable::qgisFields() const
{
  QgsFields qFields;

  for ( const QgsRasterAttributeTable::Field &field : std::as_const( mFields ) )
  {
    qFields.append( QgsField( field.name, field.type ) );
  }
  return qFields;
}

QgsFeatureList QgsRasterAttributeTable::qgisFeatures() const
{
  QgsFeatureList features;
  for ( const QVariantList &row : std::as_const( mData ) )
  {
    QgsAttributes attributes;
    for ( const auto &cell : std::as_const( row ) )
    {
      attributes.append( cell );
    }
    QgsFeature feature { qgisFields() };
    feature.setAttributes( attributes );
    features.append( feature );
  }
  return features;
}

bool QgsRasterAttributeTable::isDirty() const
{
  return mIsDirty;
}

void QgsRasterAttributeTable::setDirty( bool isDirty )
{
  mIsDirty = isDirty;
}

bool QgsRasterAttributeTable::insertField( int position, const Field &field, QString *errorMessage )
{

  const int realPos { std::clamp( position, 0, static_cast<int>( mFields.count() ) ) };

  if ( field.name.isEmpty() )
  {
    if ( errorMessage )
    {
      *errorMessage = tr( "Field name must not be empty." );
    }
    return false;
  }

  // Check for duplicate names
  bool ok;
  fieldByName( field.name, &ok );

  if ( ok )
  {
    if ( errorMessage )
    {
      *errorMessage = tr( "A field with name '%1' already exists." ).arg( field.name );
    }
    return false;
  }

  // Check for duplicate unique usages
  static const QList<Qgis::RasterAttributeTableFieldUsage> uniqueUsages {{
      Qgis::RasterAttributeTableFieldUsage::Red,
      Qgis::RasterAttributeTableFieldUsage::Green,
      Qgis::RasterAttributeTableFieldUsage::Blue,
      Qgis::RasterAttributeTableFieldUsage::Alpha,
      Qgis::RasterAttributeTableFieldUsage::RedMax,
      Qgis::RasterAttributeTableFieldUsage::GreenMax,
      Qgis::RasterAttributeTableFieldUsage::BlueMax,
      Qgis::RasterAttributeTableFieldUsage::AlphaMax,
      Qgis::RasterAttributeTableFieldUsage::RedMin,
      Qgis::RasterAttributeTableFieldUsage::GreenMin,
      Qgis::RasterAttributeTableFieldUsage::BlueMin,
      Qgis::RasterAttributeTableFieldUsage::AlphaMin,
      Qgis::RasterAttributeTableFieldUsage::MaxCount,
      Qgis::RasterAttributeTableFieldUsage::PixelCount,
      Qgis::RasterAttributeTableFieldUsage::MinMax,
      Qgis::RasterAttributeTableFieldUsage::Min,
      Qgis::RasterAttributeTableFieldUsage::Max
    }};

  if ( uniqueUsages.contains( field.usage ) && ! fieldsByUsage( field.usage ).isEmpty() )
  {
    if ( errorMessage )
    {
      *errorMessage = tr( "A field with unique usage '%1' already exists." ).arg( usageName( field.usage ) );
    }
    return false;
  }

  mFields.insert( realPos, field );

  for ( auto it = mData.begin(); it != mData.end(); ++it )
  {
    QVariant defaultValue( field.type );
    // Set default values
    switch ( field.type )
    {
      case QVariant::Type::Char:
      case QVariant::Type::Int:
      case QVariant::Type::UInt:
      case QVariant::Type::LongLong:
      case QVariant::Type::ULongLong:
      case QVariant::Type::Double:
        defaultValue = 0;
        break;
      default:
        defaultValue = QString();
    }
    it->insert( realPos, defaultValue );
  }

  // Set/change the table type from the value field type
  if ( field.usage == Qgis::RasterAttributeTableFieldUsage::MinMax )
  {
    mType = Qgis::RasterAttributeTableType::Thematic;
  }
  else if ( field.usage == Qgis::RasterAttributeTableFieldUsage::Max || field.usage == Qgis::RasterAttributeTableFieldUsage::Max )
  {
    mType = Qgis::RasterAttributeTableType::Athematic;
  }

  setType();
  setDirty( true );

  return true;
}

bool QgsRasterAttributeTable::insertField( int position, const QString &name, const Qgis::RasterAttributeTableFieldUsage usage, const QVariant::Type type, QString *errorMessage )
{
  return insertField( position, { name, usage, type}, errorMessage );
}

bool QgsRasterAttributeTable::insertColor( int position, QString *errorMessage )
{
  const QList<Qgis::RasterAttributeTableFieldUsage> colors {{ Qgis::RasterAttributeTableFieldUsage::Red, Qgis::RasterAttributeTableFieldUsage::Green, Qgis::RasterAttributeTableFieldUsage::Blue, Qgis::RasterAttributeTableFieldUsage::Alpha }};
  int idx { position };
  for ( const Qgis::RasterAttributeTableFieldUsage usage : std::as_const( colors ) )
  {
    if ( ! insertField( idx, usageName( usage ), usage, QVariant::Type::Int, errorMessage ) )
    {
      return false;
    }
    ++idx;
  }
  return true;
}

bool QgsRasterAttributeTable::setFieldUsage( int fieldIndex, const Qgis::RasterAttributeTableFieldUsage usage )
{
  if ( fieldIndex <  0 || fieldIndex >= fields().count( ) )
  {
    return false;
  }

  const Field field { fields().at( fieldIndex ) };
  if ( ! usageInformation()[ usage ].allowedTypes.contains( field.type ) )
  {
    return false;
  }

  mFields[ fieldIndex ].usage = usage;
  setType();

  return true;
}

bool QgsRasterAttributeTable::insertRamp( int position, QString *errorMessage )
{
  if ( mType != Qgis::RasterAttributeTableType::Athematic )
  {
    if ( errorMessage )
    {
      *errorMessage = tr( "A color ramp can only be added to an athematic attribute table." );
    }
  }
  const QList<Qgis::RasterAttributeTableFieldUsage> colors {{ Qgis::RasterAttributeTableFieldUsage::RedMin, Qgis::RasterAttributeTableFieldUsage::GreenMin, Qgis::RasterAttributeTableFieldUsage::BlueMin, Qgis::RasterAttributeTableFieldUsage::AlphaMin, Qgis::RasterAttributeTableFieldUsage::RedMax, Qgis::RasterAttributeTableFieldUsage::GreenMax, Qgis::RasterAttributeTableFieldUsage::BlueMax, Qgis::RasterAttributeTableFieldUsage::AlphaMax }};
  int idx { position };
  for ( const Qgis::RasterAttributeTableFieldUsage usage : std::as_const( colors ) )
  {
    if ( ! insertField( idx, usageName( usage ), usage, QVariant::Type::Int, errorMessage ) )
    {
      return false;
    }
    ++idx;
  }
  return true;
}

bool QgsRasterAttributeTable::appendField( const QString &name, const Qgis::RasterAttributeTableFieldUsage usage, const QVariant::Type type, QString *errorMessage )
{
  return insertField( mFields.count(), name, usage, type, errorMessage );
}

bool QgsRasterAttributeTable::appendField( const Field &field, QString *errorMessage )
{
  return insertField( mFields.count(), field, errorMessage );
}

bool QgsRasterAttributeTable::removeField( const QString &name, QString *errorMessage )
{
  const auto toRemove { std::find_if( mFields.begin(), mFields.end(), [ &name ]( Field & f ) -> bool {
      return f.name == name;
    } )};

  if ( toRemove != mFields.end() )
  {
    const int idx { static_cast<int>( std::distance( mFields.begin(), toRemove ) ) };
    mFields.erase( toRemove );
    for ( auto it = mData.begin(); it != mData.end(); ++it )
    {
      it->removeAt( idx );
    }
    setType();
    setDirty( true );
    return true;
  }

  if ( errorMessage )
  {
    *errorMessage = tr( "A field with name '%1' was not found." ).arg( name );
  }
  return false;
}

bool QgsRasterAttributeTable::insertRow( int position, const QVariantList &rowData, QString *errorMessage )
{

  const int realPos { std::clamp( position, 0, static_cast<int>( mData.count() ) ) };

  if ( rowData.size() != mFields.size() )
  {
    if ( errorMessage )
    {
      *errorMessage = tr( "Row element count differs from field count (%1)." ).arg( mFields.size() );
    }
    return false;
  }

  QVariantList dataValid;

  for ( int idx = 0; idx < mFields.count(); ++idx )
  {
    QVariant cell( rowData[ idx ] );
    if ( ! cell.canConvert( mFields.at( idx ).type ) || ! cell.convert( mFields.at( idx ).type ) )
    {
      if ( errorMessage )
      {
        *errorMessage = tr( "Row data at column %1 cannot be converted to field type (%2)." ).arg( idx ).arg( QVariant::typeToName( mFields.at( idx ).type ) );
      }
      return false;
    }
    else
    {
      dataValid.append( cell );
    }
  }

  mData.insert( realPos, dataValid );
  setDirty( true );
  return true;
}

bool QgsRasterAttributeTable::removeRow( int position, QString *errorMessage )
{
  if ( position >= mData.count() || position < 0 || mData.isEmpty() )
  {
    if ( errorMessage )
    {
      *errorMessage = tr( "Position is not valid or the table is empty." );
    }
    return false;
  }
  mData.removeAt( position );
  setDirty( true );
  return true;
}

bool QgsRasterAttributeTable::appendRow( const QVariantList &data, QString *errorMessage )
{
  return insertRow( mData.count(), data, errorMessage );
}

bool QgsRasterAttributeTable::writeToFile( const QString &path, QString *errorMessage )
{
  QgsVectorFileWriter::SaveVectorOptions options;
  options.actionOnExistingFile =  QgsVectorFileWriter::ActionOnExistingFile::CreateOrOverwriteFile;
  options.driverName = QStringLiteral( "ESRI Shapefile" );
  options.fileEncoding = QStringLiteral( "UTF-8" );
  options.layerOptions = QStringList() << QStringLiteral( "SHPT=NULL" );

  std::unique_ptr<QgsVectorFileWriter> writer;

  // Strip .dbf from path because OGR adds it back
  QString cleanedPath { path };
  if ( path.endsWith( QStringLiteral( ".dbf" ), Qt::CaseSensitivity::CaseInsensitive ) )
  {
    cleanedPath.chop( 4 );
  }

  cleanedPath = QgsFileUtils::ensureFileNameHasExtension( cleanedPath, {{ QStringLiteral( ".vat" ) } } );

  writer.reset( QgsVectorFileWriter::create( cleanedPath, qgisFields(), Qgis::WkbType::NoGeometry, QgsCoordinateReferenceSystem(), QgsCoordinateTransformContext(), options ) );

  cleanedPath.append( QStringLiteral( ".dbf" ) );

  const QgsVectorFileWriter::WriterError error { writer->hasError() };
  if ( error != QgsVectorFileWriter::WriterError::NoError )
  {
    if ( errorMessage )
    {
      *errorMessage = tr( "Error creating Raster Attribute Table table: %1." ).arg( writer->errorMessage() );
    }
    return false;
  }

  QgsFeatureList features { qgisFeatures() };
  bool result { writer->addFeatures( features ) };

  if ( ! result )
  {
    if ( errorMessage )
    {
      *errorMessage = tr( "Error creating Raster Attribute Table table: could not add rows." );
    }
    return false;
  }

  result = writer->flushBuffer();

  if ( result )
  {
    mFilePath = cleanedPath;
    setDirty( false );
  }

  return result;
}

bool QgsRasterAttributeTable::readFromFile( const QString &path, QString *errorMessage )
{
  QgsOgrProvider ratDbfSource { path, QgsDataProvider::ProviderOptions() };
  if ( ! ratDbfSource.isValid() )
  {
    if ( errorMessage )
    {
      *errorMessage = tr( "Error reading Raster Attribute Table table from file: invalid layer." );
    }
    return false;
  }

  QList<Field> oldFields = mFields;
  QList<QVariantList> oldData = mData;

  mFields.clear();
  mData.clear();

  bool hasValueField { false };
  for ( const QgsField &field : ratDbfSource.fields() )
  {
    const Qgis::RasterAttributeTableFieldUsage usage { guessFieldUsage( field.name(), field.type() ) };
    QVariant::Type type { field.type() };
    // DBF sets all int fields to long but for RGBA it doesn't make sense
    if ( type == QVariant::Type::LongLong &&
         ( usage == Qgis::RasterAttributeTableFieldUsage::Red || usage == Qgis::RasterAttributeTableFieldUsage::RedMax || usage == Qgis::RasterAttributeTableFieldUsage::RedMin ||
           usage == Qgis::RasterAttributeTableFieldUsage::Green || usage == Qgis::RasterAttributeTableFieldUsage::GreenMax || usage == Qgis::RasterAttributeTableFieldUsage::GreenMin ||
           usage == Qgis::RasterAttributeTableFieldUsage::Blue || usage == Qgis::RasterAttributeTableFieldUsage::BlueMax || usage == Qgis::RasterAttributeTableFieldUsage::BlueMin ||
           usage == Qgis::RasterAttributeTableFieldUsage::Alpha || usage == Qgis::RasterAttributeTableFieldUsage::AlphaMax || usage == Qgis::RasterAttributeTableFieldUsage::AlphaMin ) )
    {
      type = QVariant::Int;
    }

    if ( usage == Qgis::RasterAttributeTableFieldUsage::MinMax || usage == Qgis::RasterAttributeTableFieldUsage::Min || usage == Qgis::RasterAttributeTableFieldUsage::Max )
    {
      hasValueField = true;
    }

    QgsRasterAttributeTable::Field ratField { field.name(), usage, type };
    if ( ! appendField( ratField, errorMessage ) )
    {
      mFields = oldFields;
      mData = oldData;
      return false;
    }
  }

  // Do we have a value field? If not, try to guess one
  if ( ! hasValueField && mFields.count() > 1 && ( mFields.at( 0 ).type == QVariant::Int || mFields.at( 0 ).type == QVariant::Char || mFields.at( 0 ).type == QVariant::UInt || mFields.at( 0 ).type == QVariant::LongLong || mFields.at( 0 ).type == QVariant::ULongLong ) )
  {
    mFields[0].usage = Qgis::RasterAttributeTableFieldUsage::MinMax;
  }

  const int fieldCount { static_cast<int>( ratDbfSource.fields().count( ) ) };
  QgsFeature f;
  QgsFeatureIterator fit { ratDbfSource.getFeatures( QgsFeatureRequest() ) };
  while ( fit.nextFeature( f ) )
  {
    if ( f.attributeCount() != fieldCount )
    {
      if ( errorMessage )
      {
        *errorMessage = tr( "Error reading Raster Attribute Table table from file: number of fields and number of attributes do not match." );
      }
      mFields = oldFields;
      mData = oldData;
      return false;
    }
    appendRow( f.attributes().toList() );
  }

  mFilePath = path;
  setDirty( false );

  return true;
}


bool QgsRasterAttributeTable::isValid( QString *errorMessage ) const
{
  QStringList errors;

  if ( mFields.isEmpty() )
  {
    errors.push_back( tr( "The attribute table has no fields." ) );
  }

  if ( mData.isEmpty() )
  {
    errors.push_back( tr( "The attribute table has no rows." ) );
  }

  const QList<Qgis::RasterAttributeTableFieldUsage> fieldUsages { usages() };
  const bool isMinMax { fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::MinMax ) };
  const bool isValueRamp { fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::Min ) &&fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::Max ) };
  if ( ! isMinMax && ! isValueRamp )
  {
    errors.push_back( tr( "The attribute table has no MinMax nor a pair of Min and Max fields." ) );
  }

  // Check color
  if ( fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::Red ) || fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::Green ) || fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::Blue ) )
  {
    if ( !( fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::Red ) && fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::Green ) && fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::Blue ) ) )
    {
      errors.push_back( tr( "The attribute table has some but not all the fields required for color definition (Red, Green, Blue)." ) );
    }
  }

  // Check ramp
  if ( fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::RedMin ) || fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::GreenMin ) || fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::BlueMin ) || fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::RedMax ) || fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::GreenMax ) || fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::BlueMax ) )
  {
    if ( !( fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::RedMin ) && fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::GreenMin ) && fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::BlueMin ) && fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::RedMax ) && fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::GreenMax ) && fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::BlueMax ) ) )
    {
      errors.push_back( tr( "The attribute table has some but not all the fields required for color ramp definition (RedMin, GreenMin, BlueMin, RedMax, GreenMax, BlueMax)." ) );
    }
    else if ( ! isValueRamp )
    {
      errors.push_back( tr( "The attribute table has all the fields required for color ramp definition (RedMin, GreenMin, BlueMin, RedMax, GreenMax, BlueMax) but no Min and Max field." ) );
    }
  }

  if ( errorMessage && ! errors.isEmpty() )
  {
    *errorMessage = errors.join( QChar( '\n' ) );
  }

  return errors.isEmpty();
}

const QList<QList<QVariant> > QgsRasterAttributeTable::data() const
{
  return mData;
}

const QgsRasterAttributeTable::Field QgsRasterAttributeTable::fieldByName( const QString name, bool *ok ) const
{
  for ( const Field &f : std::as_const( mFields ) )
  {
    if ( f.name == name )
    {
      if ( ok )
      {
        *ok = true;
      }
      return f;
    }
  }
  if ( ok )
  {
    *ok = false;
  }
  return Field( QString(), Qgis::RasterAttributeTableFieldUsage::Generic, QVariant::String );
}

const QList<QgsRasterAttributeTable::Field> QgsRasterAttributeTable::fieldsByUsage( const Qgis::RasterAttributeTableFieldUsage fieldUsage ) const
{
  QList<QgsRasterAttributeTable::Field> result;
  for ( const Field &f : std::as_const( mFields ) )
  {
    if ( f.usage == fieldUsage )
    {
      result.push_back( f );
    }
  }
  return result;
}

bool QgsRasterAttributeTable::setValue( const int row, const int column, const QVariant &value )
{
  if ( row < 0 || row >= mData.count( ) || column < 0 || column >=  mData[ row ].count( ) )
  {
    return false;
  }

  QVariant newVal = value;
  if ( column >= mFields.length() || ! value.canConvert( mFields.at( column ).type ) || ! newVal.convert( mFields.at( column ).type ) )
  {
    return false;
  }

  const QVariant oldVal = mData[ row ][ column ];

  if ( newVal != oldVal )
  {
    mData[ row ][ column ] = newVal;
    setDirty( true );
  }

  return true;
}

QVariant QgsRasterAttributeTable::value( const int row, const int column ) const
{
  if ( row < 0 || row >= mData.count( ) || column < 0 || column >=  mData[ row ].count( ) )
  {
    return QVariant();
  }
  return mData[ row ][ column ];
}

double QgsRasterAttributeTable::minimumValue() const
{
  const QList<Qgis::RasterAttributeTableFieldUsage> fieldUsages { usages() };
  bool ok { false };
  int fieldIdx { -1 };

  if ( fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::MinMax ) )
  {
    fieldIdx = fieldUsages.indexOf( Qgis::RasterAttributeTableFieldUsage::MinMax );
  }
  else if ( fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::Min ) )
  {
    fieldIdx = fieldUsages.indexOf( Qgis::RasterAttributeTableFieldUsage::Min );
  }

  double min { std::numeric_limits<double>::max() };
  for ( int rowIdx = 0; rowIdx < mData.count(); ++rowIdx )
  {
    min = std::min( min, value( rowIdx, fieldIdx ).toDouble( &ok ) );
    if ( ! ok )
    {
      return std::numeric_limits<double>::quiet_NaN();
    }
  }

  if ( fieldIdx == -1 || ! ok )
  {
    return std::numeric_limits<double>::quiet_NaN();
  }
  else
  {
    return min;
  }
}

double QgsRasterAttributeTable::maximumValue() const
{
  const QList<Qgis::RasterAttributeTableFieldUsage> fieldUsages { usages() };
  bool ok { false };
  int fieldIdx { -1 };

  if ( fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::MinMax ) )
  {
    fieldIdx = fieldUsages.indexOf( Qgis::RasterAttributeTableFieldUsage::MinMax );
  }
  else if ( fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::Max ) )
  {
    fieldIdx = fieldUsages.indexOf( Qgis::RasterAttributeTableFieldUsage::Max );
  }

  double max { std::numeric_limits<double>::lowest() };
  for ( int rowIdx = 0; rowIdx < mData.count(); ++rowIdx )
  {
    max = std::max( max, value( rowIdx, fieldIdx ).toDouble( &ok ) );
    if ( ! ok )
    {
      return std::numeric_limits<double>::quiet_NaN();
    }
  }

  if ( fieldIdx == -1 || ! ok )
  {
    return std::numeric_limits<double>::quiet_NaN();
  }
  else
  {
    return max;
  }
}

QVariantList QgsRasterAttributeTable::row( const double matchValue ) const
{
  if ( ! isValid() )
  {
    return QVariantList();
  }

  const QList<Qgis::RasterAttributeTableFieldUsage> fieldUsages { usages() };

  if ( fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::MinMax ) )
  {
    const int colIdx { static_cast<int>( fieldUsages.indexOf( Qgis::RasterAttributeTableFieldUsage::MinMax ) ) };
    for ( int rowIdx = 0; rowIdx < mData.count(); ++rowIdx )
    {
      bool ok;
      if ( matchValue == value( rowIdx, colIdx ).toDouble( &ok ) && ok )
      {
        return mData.at( rowIdx );
      }
    }
  }
  else
  {
    const int minColIdx { static_cast<int>( fieldUsages.indexOf( Qgis::RasterAttributeTableFieldUsage::Min ) ) };
    const int maxColIdx { static_cast<int>( fieldUsages.indexOf( Qgis::RasterAttributeTableFieldUsage::Max ) ) };
    for ( int rowIdx = 0; rowIdx < mData.count(); ++rowIdx )
    {
      bool ok;
      if ( matchValue >= value( rowIdx, minColIdx ).toDouble( &ok ) && ok )
      {
        if ( matchValue < value( rowIdx, maxColIdx ).toDouble( &ok ) && ok )
        {
          return mData.at( rowIdx );
        }
      }
    }
  }
  return QVariantList();
}

Qgis::RasterAttributeTableFieldUsage QgsRasterAttributeTable::guessFieldUsage( const QString &name, const QVariant::Type type )
{
  static const QStringList minValueNames { {
      QStringLiteral( "min" ),
      QStringLiteral( "min_value" ),
      QStringLiteral( "min value" ),
      QStringLiteral( "value min" ),
      QStringLiteral( "value_min" ),
    } };

  static const QStringList maxValueNames { {
      QStringLiteral( "max" ),
      QStringLiteral( "max_value" ),
      QStringLiteral( "max value" ),
      QStringLiteral( "value max" ),
      QStringLiteral( "value_max" ),
    } };

  const QString fieldLower { name.toLower() };

  if ( type == QVariant::Double || type == QVariant::Int || type == QVariant::UInt || type == QVariant::LongLong || type == QVariant::ULongLong )
  {
    if ( minValueNames.contains( fieldLower ) )
    {
      return Qgis::RasterAttributeTableFieldUsage::Min;
    }
    else if ( maxValueNames.contains( fieldLower ) )
    {
      return Qgis::RasterAttributeTableFieldUsage::Max;
    }
    else if ( fieldLower == QLatin1String( "value" ) )
    {
      return Qgis::RasterAttributeTableFieldUsage::MinMax;
    }
    else if ( fieldLower == QLatin1String( "count" ) )
    {
      // This could really be max count but it's more likely pixel count
      return Qgis::RasterAttributeTableFieldUsage::PixelCount;
    }
    // Colors (not double)
    else if ( type != QVariant::Double )
    {
      if ( fieldLower.contains( "red" ) || fieldLower == QLatin1String( "r" ) )
      {
        if ( fieldLower.contains( "min" ) )
        {
          return Qgis::RasterAttributeTableFieldUsage::RedMin;
        }
        else if ( fieldLower.contains( "max" ) )
        {
          return Qgis::RasterAttributeTableFieldUsage::RedMax;
        }
        else
        {
          return Qgis::RasterAttributeTableFieldUsage::Red;
        }
      }
      else if ( fieldLower.contains( "green" ) || fieldLower == QLatin1String( "g" ) )
      {
        if ( fieldLower.contains( "min" ) )
        {
          return Qgis::RasterAttributeTableFieldUsage::GreenMin;
        }
        else if ( fieldLower.contains( "max" ) )
        {
          return Qgis::RasterAttributeTableFieldUsage::GreenMax;
        }
        else
        {
          return Qgis::RasterAttributeTableFieldUsage::Green;
        }
      }
      else if ( fieldLower.contains( "blue" ) || fieldLower == QLatin1String( "b" ) )
      {
        if ( fieldLower.contains( "min" ) )
        {
          return Qgis::RasterAttributeTableFieldUsage::BlueMin;
        }
        else if ( fieldLower.contains( "max" ) )
        {
          return Qgis::RasterAttributeTableFieldUsage::BlueMax;
        }
        else
        {
          return Qgis::RasterAttributeTableFieldUsage::Blue;
        }
      }
      else if ( fieldLower.contains( "alpha" ) || fieldLower == QLatin1String( "a" ) )
      {
        if ( fieldLower.contains( "min" ) )
        {
          return Qgis::RasterAttributeTableFieldUsage::AlphaMin;
        }
        else if ( fieldLower.contains( "max" ) )
        {
          return Qgis::RasterAttributeTableFieldUsage::AlphaMax;
        }
        else
        {
          return Qgis::RasterAttributeTableFieldUsage::Alpha;
        }
      }
    }
    // end colors
  }

  if ( type == QVariant::String )  // default to name for strings
  {
    return Qgis::RasterAttributeTableFieldUsage::Name;
  }

  // default to generic for all other cases
  return Qgis::RasterAttributeTableFieldUsage::Generic;

}

QString QgsRasterAttributeTable::usageName( const Qgis::RasterAttributeTableFieldUsage usage )
{
  switch ( usage )
  {
    case Qgis::RasterAttributeTableFieldUsage::Red:
      return tr( "Red" );
    case Qgis::RasterAttributeTableFieldUsage::Green:
      return tr( "Green" );
    case Qgis::RasterAttributeTableFieldUsage::Blue:
      return tr( "Blue" );
    case Qgis::RasterAttributeTableFieldUsage::Alpha:
      return tr( "Alpha" );
    case Qgis::RasterAttributeTableFieldUsage::RedMin:
      return tr( "Red Minimum" );
    case Qgis::RasterAttributeTableFieldUsage::GreenMin:
      return tr( "Green Minimum" );
    case Qgis::RasterAttributeTableFieldUsage::BlueMin:
      return tr( "Blue Minimum" );
    case Qgis::RasterAttributeTableFieldUsage::AlphaMin:
      return tr( "Alpha Minimum" );
    case Qgis::RasterAttributeTableFieldUsage::RedMax:
      return tr( "Red Maximum" );
    case Qgis::RasterAttributeTableFieldUsage::GreenMax:
      return tr( "Green Maximum" );
    case Qgis::RasterAttributeTableFieldUsage::BlueMax:
      return tr( "Blue Maximum" );
    case Qgis::RasterAttributeTableFieldUsage::AlphaMax:
      return tr( "Alpha Maximum" );
    case Qgis::RasterAttributeTableFieldUsage::Generic:
      return tr( "Generic" );
    case Qgis::RasterAttributeTableFieldUsage::Name:
      return tr( "Name" );
    case Qgis::RasterAttributeTableFieldUsage::PixelCount:
      return tr( "Pixel Count" );
    case Qgis::RasterAttributeTableFieldUsage::MaxCount:
      return tr( "Maximum Count" );
    case Qgis::RasterAttributeTableFieldUsage::MinMax:
      return tr( "Value" );
    case Qgis::RasterAttributeTableFieldUsage::Min:
      return tr( "Minimum Value" );
    case Qgis::RasterAttributeTableFieldUsage::Max:
      return tr( "Maximum Value" );
  }
  return QString();
}

QList<Qgis::RasterAttributeTableFieldUsage> QgsRasterAttributeTable::valueAndColorFieldUsages()
{
  static const QList<Qgis::RasterAttributeTableFieldUsage> valueColorUsages {{
      Qgis::RasterAttributeTableFieldUsage::MinMax,
      Qgis::RasterAttributeTableFieldUsage::Min,
      Qgis::RasterAttributeTableFieldUsage::Max,
      Qgis::RasterAttributeTableFieldUsage::Red,
      Qgis::RasterAttributeTableFieldUsage::Green,
      Qgis::RasterAttributeTableFieldUsage::Blue,
      Qgis::RasterAttributeTableFieldUsage::Alpha,
      Qgis::RasterAttributeTableFieldUsage::RedMin,
      Qgis::RasterAttributeTableFieldUsage::GreenMin,
      Qgis::RasterAttributeTableFieldUsage::BlueMin,
      Qgis::RasterAttributeTableFieldUsage::AlphaMin,
      Qgis::RasterAttributeTableFieldUsage::RedMax,
      Qgis::RasterAttributeTableFieldUsage::GreenMax,
      Qgis::RasterAttributeTableFieldUsage::BlueMax,
      Qgis::RasterAttributeTableFieldUsage::AlphaMax,
    }};
  return valueColorUsages;
}

QgsRasterAttributeTable *QgsRasterAttributeTable::createFromRaster( QgsRasterLayer *raster, int *bandNumber )
{

  if ( ! raster || ! raster->dataProvider() || ! raster->isValid() )
  {
    return nullptr;
  }

  const QgsRasterRenderer *renderer = raster->renderer();

  if ( ! renderer )
  {
    return nullptr;
  }

  if ( const QgsPalettedRasterRenderer *palettedRenderer = dynamic_cast<const QgsPalettedRasterRenderer *>( renderer ) )
  {
    QgsRasterAttributeTable *rat = new QgsRasterAttributeTable();
    rat->appendField( QStringLiteral( "Value" ), Qgis::RasterAttributeTableFieldUsage::MinMax, QVariant::Type::Double );
    rat->appendField( QStringLiteral( "Class" ), Qgis::RasterAttributeTableFieldUsage::Name, QVariant::Type::String );
    rat->appendField( QStringLiteral( "Red" ), Qgis::RasterAttributeTableFieldUsage::Red, QVariant::Type::Int );
    rat->appendField( QStringLiteral( "Green" ), Qgis::RasterAttributeTableFieldUsage::Green, QVariant::Type::Int );
    rat->appendField( QStringLiteral( "Blue" ), Qgis::RasterAttributeTableFieldUsage::Blue, QVariant::Type::Int );
    rat->appendField( QStringLiteral( "Alpha" ), Qgis::RasterAttributeTableFieldUsage::Alpha, QVariant::Type::Int );

    const QgsPalettedRasterRenderer::ClassData classes { palettedRenderer->classes() };

    for ( const QgsPalettedRasterRenderer::Class &klass : std::as_const( classes ) )
    {
      rat->appendRow( QVariantList() << klass.value << klass.label << 0 << 0 << 0 << 255 );
      rat->setColor( rat->data().length() - 1, klass.color );
    }

    if ( bandNumber )
    {
      *bandNumber = palettedRenderer->inputBand();
    }
    return rat;
  }
  else if ( const QgsSingleBandPseudoColorRenderer *pseudoColorRenderer = dynamic_cast<const QgsSingleBandPseudoColorRenderer *>( renderer ) )
  {
    if ( const QgsRasterShader *shader = pseudoColorRenderer->shader() )
    {
      if ( const QgsColorRampShader *shaderFunction = dynamic_cast<const QgsColorRampShader *>( shader->rasterShaderFunction() ) )
      {
        QgsRasterAttributeTable *rat = new QgsRasterAttributeTable();
        switch ( shaderFunction->colorRampType() )
        {

          case QgsColorRampShader::Type::Interpolated:
          {
            rat->appendField( QStringLiteral( "Min" ), Qgis::RasterAttributeTableFieldUsage::Min, QVariant::Type::Double );
            rat->appendField( QStringLiteral( "Max" ), Qgis::RasterAttributeTableFieldUsage::Max, QVariant::Type::Double );
            rat->appendField( QStringLiteral( "Class" ), Qgis::RasterAttributeTableFieldUsage::Name, QVariant::Type::String );
            rat->appendField( QStringLiteral( "RedMin" ), Qgis::RasterAttributeTableFieldUsage::RedMin, QVariant::Type::Int );
            rat->appendField( QStringLiteral( "GreenMin" ), Qgis::RasterAttributeTableFieldUsage::GreenMin, QVariant::Type::Int );
            rat->appendField( QStringLiteral( "BlueMin" ), Qgis::RasterAttributeTableFieldUsage::BlueMin, QVariant::Type::Int );
            rat->appendField( QStringLiteral( "AlphaMin" ), Qgis::RasterAttributeTableFieldUsage::AlphaMin, QVariant::Type::Int );
            rat->appendField( QStringLiteral( "RedMax" ), Qgis::RasterAttributeTableFieldUsage::RedMax, QVariant::Type::Int );
            rat->appendField( QStringLiteral( "GreenMax" ), Qgis::RasterAttributeTableFieldUsage::GreenMax, QVariant::Type::Int );
            rat->appendField( QStringLiteral( "BlueMax" ), Qgis::RasterAttributeTableFieldUsage::BlueMax, QVariant::Type::Int );
            rat->appendField( QStringLiteral( "AlphaMax" ), Qgis::RasterAttributeTableFieldUsage::AlphaMax, QVariant::Type::Int );
            const QList<QgsColorRampShader::ColorRampItem> rampItems { shaderFunction->colorRampItemList() };
            if ( rampItems.size() > 1 )
            {
              QColor color1 { rampItems.at( 0 ).color };
              QString label1 { rampItems.at( 0 ).label };
              QVariant value1( rampItems.at( 0 ).value );
              const int rampItemSize = rampItems.size();
              for ( int i = 1; i < rampItemSize; ++i )
              {
                const QgsColorRampShader::ColorRampItem &rampItem { rampItems.at( i )};
                rat->appendRow( QVariantList() << value1 << rampItem.value << QStringLiteral( "%1 - %2" ).arg( label1, rampItem.label ) << 0 << 0 << 0 << 255 << 0 << 0 << 0 << 255 );
                rat->setRamp( rat->data().length() - 1, color1, rampItem.color );
                label1 = rampItem.label;
                value1 = rampItem.value;
                color1 = rampItem.color;
              }
            }
            break;
          }

          case QgsColorRampShader::Type::Discrete:
          {
            rat->appendField( QStringLiteral( "Min" ), Qgis::RasterAttributeTableFieldUsage::Min, QVariant::Type::Double );
            rat->appendField( QStringLiteral( "Max" ), Qgis::RasterAttributeTableFieldUsage::Max, QVariant::Type::Double );
            rat->appendField( QStringLiteral( "Class" ), Qgis::RasterAttributeTableFieldUsage::Name, QVariant::Type::String );
            rat->appendField( QStringLiteral( "Red" ), Qgis::RasterAttributeTableFieldUsage::Red, QVariant::Type::Int );
            rat->appendField( QStringLiteral( "Green" ), Qgis::RasterAttributeTableFieldUsage::Green, QVariant::Type::Int );
            rat->appendField( QStringLiteral( "Blue" ), Qgis::RasterAttributeTableFieldUsage::Blue, QVariant::Type::Int );
            rat->appendField( QStringLiteral( "Alpha" ), Qgis::RasterAttributeTableFieldUsage::Alpha, QVariant::Type::Int );
            const QList<QgsColorRampShader::ColorRampItem> rampItems { shaderFunction->colorRampItemList() };
            if ( rampItems.size( ) > 1 )
            {
              QColor color1 { rampItems.at( 0 ).color };
              QString label1 { rampItems.at( 0 ).label };
              QVariant value1( rampItems.at( 0 ).value );
              const int rampItemSize = rampItems.size();
              for ( int i = 1; i < rampItemSize; ++i )
              {
                const QgsColorRampShader::ColorRampItem &rampItem { rampItems.at( i )};
                rat->appendRow( QVariantList() << value1 << rampItem.value << QStringLiteral( "%1 - %2" ).arg( label1, rampItem.label ) << 0 << 0 << 0 << 255 << 0 << 0 << 0 << 255 );
                rat->setRamp( rat->data().length() - 1, color1, rampItem.color );
                label1 = rampItem.label;
                value1 = rampItem.value;
                color1 = rampItem.color;
              }
            }
            break;
          }

          case QgsColorRampShader::Type::Exact:
          {
            rat->appendField( QStringLiteral( "Value" ), Qgis::RasterAttributeTableFieldUsage::MinMax, QVariant::Type::Double );
            rat->appendField( QStringLiteral( "Class" ), Qgis::RasterAttributeTableFieldUsage::Name, QVariant::Type::String );
            rat->appendField( QStringLiteral( "Red" ), Qgis::RasterAttributeTableFieldUsage::Red, QVariant::Type::Int );
            rat->appendField( QStringLiteral( "Green" ), Qgis::RasterAttributeTableFieldUsage::Green, QVariant::Type::Int );
            rat->appendField( QStringLiteral( "Blue" ), Qgis::RasterAttributeTableFieldUsage::Blue, QVariant::Type::Int );
            rat->appendField( QStringLiteral( "Alpha" ), Qgis::RasterAttributeTableFieldUsage::Alpha, QVariant::Type::Int );
            const QList<QgsColorRampShader::ColorRampItem> rampItems { shaderFunction->colorRampItemList() };
            for ( const QgsColorRampShader::ColorRampItem &rampItem : std::as_const( rampItems ) )
            {
              rat->appendRow( QVariantList() << rampItem.value << rampItem.label << 0 << 0 << 0 << 255 );
              rat->setColor( rat->data().length() - 1, rampItem.color );
            }
            break;
          }
        }

        if ( bandNumber )
        {
          *bandNumber = pseudoColorRenderer->inputBand();
        }

        return rat;
      }
      else
      {
        return nullptr;
      }
    }
    else
    {
      return nullptr;
    }
  }
  else
  {
    return nullptr;
  }
}

QHash<Qgis::RasterAttributeTableFieldUsage, QgsRasterAttributeTable::UsageInformation> QgsRasterAttributeTable::usageInformation()
{
  std::call_once( usageInformationLoaderFlag, [ ]
  {
    QgsRasterAttributeTable::sUsageInformation.insert( Qgis::RasterAttributeTableFieldUsage::Generic, { tr( "General Purpose Field" ), false, false, false, false, true, true, QList<QVariant::Type>() << QVariant::String << QVariant::Int << QVariant::LongLong << QVariant::Double } );
    QgsRasterAttributeTable::sUsageInformation.insert( Qgis::RasterAttributeTableFieldUsage::PixelCount, { tr( "Histogram Pixel Count" ), true, false, false, false, true, false, QList<QVariant::Type>() << QVariant::LongLong } );
    QgsRasterAttributeTable::sUsageInformation.insert( Qgis::RasterAttributeTableFieldUsage::Name, { tr( "Class Name" ), false, false, false, false, true, true, QList<QVariant::Type>() << QVariant::String } );
    QgsRasterAttributeTable::sUsageInformation.insert( Qgis::RasterAttributeTableFieldUsage::MinMax, { tr( "Class Value (min=max)" ), true, true, false, false, true, false, QList<QVariant::Type>() << QVariant::Int << QVariant::LongLong << QVariant::Double } );
    QgsRasterAttributeTable::sUsageInformation.insert( Qgis::RasterAttributeTableFieldUsage::Min, { tr( "Class Minimum Value" ), true, true, false, false, true, false, QList<QVariant::Type>() << QVariant::Int << QVariant::LongLong << QVariant::Double } );
    QgsRasterAttributeTable::sUsageInformation.insert( Qgis::RasterAttributeTableFieldUsage::Max, { tr( "Class Maximum Value" ), true, true, false, false, true, false, QList<QVariant::Type>() << QVariant::Int << QVariant::LongLong << QVariant::Double } );
    QgsRasterAttributeTable::sUsageInformation.insert( Qgis::RasterAttributeTableFieldUsage::Red, { tr( "Red Color Value (0-255)" ), true, false, true, false, true, false, QList<QVariant::Type>() << QVariant::Int } );
    QgsRasterAttributeTable::sUsageInformation.insert( Qgis::RasterAttributeTableFieldUsage::Green, { tr( "Green Color Value (0-255)" ), true, false, true, false, true, false, QList<QVariant::Type>() << QVariant::Int } );
    QgsRasterAttributeTable::sUsageInformation.insert( Qgis::RasterAttributeTableFieldUsage::Blue, { tr( "Blue Color Value (0-255)" ), true, false, true, false, true, false, QList<QVariant::Type>() << QVariant::Int } );
    QgsRasterAttributeTable::sUsageInformation.insert( Qgis::RasterAttributeTableFieldUsage::Alpha, { tr( "Alpha Color Value (0-255)" ), true, false, true, false, true, false, QList<QVariant::Type>() << QVariant::Int } );
    QgsRasterAttributeTable::sUsageInformation.insert( Qgis::RasterAttributeTableFieldUsage::RedMin, { tr( "Red Color Minimum Value (0-255)" ), true, false, false, true, true, false, QList<QVariant::Type>() << QVariant::Int } );
    QgsRasterAttributeTable::sUsageInformation.insert( Qgis::RasterAttributeTableFieldUsage::GreenMin, { tr( "Green Color Minimum Value (0-255)" ), true, false, false, true, true, false, QList<QVariant::Type>() << QVariant::Int } );
    QgsRasterAttributeTable::sUsageInformation.insert( Qgis::RasterAttributeTableFieldUsage::BlueMin, { tr( "Blue Color Minimum Value (0-255)" ), true, false, false, true, true, false, QList<QVariant::Type>() << QVariant::Int } );
    QgsRasterAttributeTable::sUsageInformation.insert( Qgis::RasterAttributeTableFieldUsage::AlphaMin, { tr( "Alpha Color Minimum Value (0-255)" ), true, false, false, true, true, false, QList<QVariant::Type>() << QVariant::Int } );
    QgsRasterAttributeTable::sUsageInformation.insert( Qgis::RasterAttributeTableFieldUsage::RedMax, { tr( "Red Color Minimum Value (0-255)" ), true, false, false, true, true, false, QList<QVariant::Type>() << QVariant::Int } );
    QgsRasterAttributeTable::sUsageInformation.insert( Qgis::RasterAttributeTableFieldUsage::GreenMax, { tr( "Green Color Minimum Value (0-255)" ), true, false, false, true, true, false, QList<QVariant::Type>() << QVariant::Int } );
    QgsRasterAttributeTable::sUsageInformation.insert( Qgis::RasterAttributeTableFieldUsage::BlueMax, { tr( "Blue Color Minimum Value (0-255)" ), true, false, false, true, true, false, QList<QVariant::Type>() << QVariant::Int } );
    QgsRasterAttributeTable::sUsageInformation.insert( Qgis::RasterAttributeTableFieldUsage::AlphaMax, { tr( "Alpha Color Minimum Value (0-255)" ), true, false, false, true, true, false, QList<QVariant::Type>() << QVariant::Int } );
    // Unsupported!!
    QgsRasterAttributeTable::sUsageInformation.insert( Qgis::RasterAttributeTableFieldUsage::MaxCount, { tr( "Maximum GFU value(equals to GFU_AlphaMax+1 currently)" ), true, false, false, true, false, false, QList<QVariant::Type>() << QVariant::Int } );
  } );
  return QgsRasterAttributeTable::sUsageInformation;
}

void QgsRasterAttributeTable::setType()
{
  const QList<Qgis::RasterAttributeTableFieldUsage> fieldUsages { usages() };
  mType =  fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::MinMax ) ? Qgis::RasterAttributeTableType::Thematic : Qgis::RasterAttributeTableType::Athematic;
}


///@cond PRIVATE
QHash<int, QgsRasterAttributeTable::UsageInformation> QgsRasterAttributeTable::usageInformationInt()
{
  QHash<int, QgsRasterAttributeTable::UsageInformation> usageInfoInt;
  const QHash<Qgis::RasterAttributeTableFieldUsage, QgsRasterAttributeTable::UsageInformation> usageInfo { QgsRasterAttributeTable::usageInformation() };
  for ( auto it = usageInfo.cbegin(); it != usageInfo.cend(); ++it )
  {
    usageInfoInt.insert( static_cast<int>( it.key() ), it.value() );
  }
  return usageInfoInt;
}
///@endcond PRIVATE

QString QgsRasterAttributeTable::filePath() const
{
  return mFilePath;
}

QList<QgsRasterAttributeTable::MinMaxClass> QgsRasterAttributeTable::minMaxClasses( const int classificationColumn ) const
{
  QList<QgsRasterAttributeTable::MinMaxClass> classes;
  if ( !isValid() )
  {
    QgsDebugError( "minMaxClasses was called on an invalid RAT" );
    return classes;
  }

  const QList<Qgis::RasterAttributeTableFieldUsage> fieldUsages { usages() };

  if ( ! fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::MinMax ) )
  {
    QgsDebugError( "minMaxClasses was called on a ramp raster" );
    return classes;
  }

  const int minMaxIndex { static_cast<int>( fieldUsages.indexOf( Qgis::RasterAttributeTableFieldUsage::MinMax ) ) };

  Q_ASSERT( minMaxIndex >= 0 );

  int classificationIndex = classificationColumn;
  if ( classificationIndex >= 0 && classificationIndex < mFields.count( ) )
  {
    const Field classificationField { mFields.at( classificationIndex ) };
    if ( ( classificationField.usage != Qgis::RasterAttributeTableFieldUsage::Name && classificationField.usage != Qgis::RasterAttributeTableFieldUsage::Generic ) )
    {
      QgsDebugError( "minMaxClasses was called with a classification column which is not suitable for classification" );
      return classes;
    }
  }
  else if ( classificationIndex == -1 )  // Special value for not-set
  {
    // Find first value or generic field
    if ( fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::Name ) )
    {
      classificationIndex = fieldUsages.indexOf( Qgis::RasterAttributeTableFieldUsage::Name );
    }
    else if ( fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::Generic ) )
    {
      classificationIndex = fieldUsages.indexOf( Qgis::RasterAttributeTableFieldUsage::Generic );
    }
    else
    {
      classificationIndex = minMaxIndex;
    }
  }
  else if ( classificationIndex >= mFields.count( ) )
  {
    QgsDebugError( "minMaxClasses was called with a classification column out of range" );
    return classes;
  }

  if ( classificationIndex >= 0 )
  {
    QStringList labels;
    int rowIdx { 0 };
    for ( const QVariantList &row : std::as_const( mData ) )
    {
      const QString label { row.at( classificationIndex ).toString() };
      bool ok;
      const double value { row.at( minMaxIndex ).toDouble( &ok ) };
      // This should never happen, could eventually become a Q_ASSERT
      if ( ! ok )
      {
        QgsDebugError( "minMaxClasses could not convert a MinMax value to double" );
        return classes;
      }
      if ( labels.contains( label ) )
      {
        classes[ labels.indexOf( label ) ].minMaxValues.push_back( value );
      }
      else
      {
        labels.push_back( label );
        classes.push_back( { label, { value }, color( rowIdx ) } );
      }
      rowIdx++;
    }
  }
  return classes;
}

QgsGradientColorRamp QgsRasterAttributeTable::colorRamp( QStringList &labels, const int labelColumn ) const
{
  QgsGradientColorRamp ramp{ Qt::GlobalColor::white, Qt::GlobalColor::black };
  const QList<Qgis::RasterAttributeTableFieldUsage> fieldUsages { usages() };
  const int minIdx { static_cast<int>( fieldUsages.indexOf( Qgis::RasterAttributeTableFieldUsage::Min ) ) };
  const int maxIdx { static_cast<int>( fieldUsages.indexOf( Qgis::RasterAttributeTableFieldUsage::Max ) ) };
  const bool isRange { minIdx >= 0 && maxIdx >= 0 };

  int labelIdx { labelColumn };
  if ( labelColumn < 0 || labelColumn >= fields().count( ) ||
       ( fieldUsages.at( labelColumn ) != Qgis::RasterAttributeTableFieldUsage::Name && fieldUsages.at( labelColumn ) != Qgis::RasterAttributeTableFieldUsage::Generic ) )
  {
    labelIdx = -1;
  }

  if ( ! mData.isEmpty() && ( minIdx >= 0 && maxIdx >= 0 ) )
  {
    QgsGradientStopsList stops;
    const bool hasColorOrRamp { hasColor() || hasRamp() };
    {

      const double min { minimumValue() };
      const double max { maximumValue() };
      const double range { max - min };

      if ( range != 0 )
      {

        if ( ! isnan( min ) && ! isnan( max ) )
        {
          const QList<QVariantList> dataCopy( orderedRows() );

          QgsRasterAttributeTable orderedRat;
          for ( const Field &f : std::as_const( mFields ) )
          {
            orderedRat.appendField( f );
          }
          for ( const QVariantList &r : std::as_const( dataCopy ) )
          {
            orderedRat.appendRow( r );
          }

          QColor lastColor { ramp.color1() };

          if ( hasColorOrRamp )
          {
            ramp.setColor1( orderedRat.hasColor() ? orderedRat.color( 0 ) : orderedRat.ramp( 0 ).color1() );
            ramp.setColor2( orderedRat.hasColor() ? orderedRat.color( orderedRat.data().count() - 1 ) : orderedRat.ramp( orderedRat.data().count() - 1 ).color2() );
            lastColor = orderedRat.hasColor() ? orderedRat.color( 0 ) : orderedRat.ramp( 0 ).color2();
          }

          auto labelFromField = [ & ]( int rowIdx ) -> QString
          {
            if ( labelIdx < 0 )
            {
              return QStringLiteral( "%L1 - %L2" ).arg( orderedRat.value( rowIdx, minIdx ).toDouble() ).arg( orderedRat.value( rowIdx, maxIdx ).toDouble() );
            }
            const QVariant val( orderedRat.value( rowIdx, labelIdx ) );
            bool ok { true };
            QString res;
            switch ( val.type() )
            {
              case QVariant::Type::Char:
                return QString( val.toChar() );
              case QVariant::Type::Int:
                res = QLocale().toString( val.toInt( &ok ) );
                break;
              case QVariant::Type::LongLong:
                res = QLocale().toString( val.toLongLong( &ok ) );
                break;
              case QVariant::Type::UInt:
                res = QLocale().toString( val.toUInt( &ok ) );
                break;
              case QVariant::Type::ULongLong:
                res = QLocale().toString( val.toULongLong( &ok ) );
                break;
              case QVariant::Type::Double:
                res = QLocale().toString( val.toDouble( &ok ), 'g' );
                break;
              case QVariant::Type::String:
              default:
                return val.toString( );
            }
            return ok ? res : val.toString();
          };

          // Case 1: range classes, discrete colors
          //    - create stops for the lower value of each class except for the first,
          //    - use the color from the previous class
          if ( orderedRat.hasColor() && isRange )
          {
            labels.push_back( labelFromField( 0 ) );

            for ( int rowIdx = 1; rowIdx < orderedRat.data().count(); ++rowIdx )
            {
              const double offset { ( orderedRat.value( rowIdx, minIdx ).toDouble( ) - min ) / range };
              const QColor color { orderedRat.color( rowIdx  - 1 ) };
              stops.append( QgsGradientStop( offset, color ) );
              labels.push_back( labelFromField( rowIdx ) );
            }
          }
          // Case 2: range classes, gradients colors
          // Take the class bounds (average value between max of previous class and min of the next)
          // to avoid potential overlapping or gaps between classes.
          // Create stop:
          //   - first stop at value taking the max color of the previous class
          //   - second stop at value + epsilon taking the min color of the next class, unless colors and offset are equal
          else if ( orderedRat.hasRamp() && isRange )
          {
            double prevOffset { 0 };
            labels.push_back( labelFromField( 0 ) );
            for ( int rowIdx = 1; rowIdx < orderedRat.data().count(); ++rowIdx )
            {
              labels.push_back( labelFromField( rowIdx ) );
              const int prevRowIdx { rowIdx - 1 };
              const double offset { ( ( orderedRat.value( rowIdx, minIdx ).toDouble( ) + orderedRat.value( prevRowIdx, maxIdx ).toDouble( ) ) / 2.0  - min ) / range };
              const QgsGradientColorRamp previousRamp { orderedRat.ramp( prevRowIdx ) };
              stops.append( QgsGradientStop( offset, previousRamp.color2() ) );

              const QgsGradientColorRamp currentRamp { orderedRat.ramp( rowIdx ) };
              // An additional stop is added if the colors are different or offsets are different by 1e-6 (offset varies from 0 to 1).
              if ( currentRamp.color1() != previousRamp.color2() && qgsDoubleNear( offset, prevOffset, 1e-6 ) )
              {
                stops.append( QgsGradientStop( offset + std::numeric_limits<double>::epsilon(), currentRamp.color1() ) );
              }
              prevOffset = offset;
            }
          }
          // Case 3: range classes but no colors at all
          // Take the class borders (average value between max of previous class and min of the next)
          // Create stop for the lower class, actually skipping the upper bound of the last class
          else
          {
            labels.push_back( labelFromField( 0 ) );

            for ( int rowIdx = 1; rowIdx < orderedRat.data().count(); ++rowIdx )
            {
              const int prevRowIdx { rowIdx - 1 };
              const double offset { ( ( orderedRat.value( rowIdx, minIdx ).toDouble( ) + orderedRat.value( prevRowIdx, maxIdx ).toDouble( ) ) / 2.0  - min ) / range };
              stops.append( QgsGradientStop( offset, ramp.color( offset ) ) );
              labels.push_back( labelFromField( rowIdx ) );
            }
          }
        }
      }
    }

    ramp.setStops( stops );
    ramp.setDiscrete( hasColor() );

  }

  return ramp;
}

QgsRasterRenderer *QgsRasterAttributeTable::createRenderer( QgsRasterDataProvider *provider, const int bandNumber, const int classificationColumn )
{
  if ( ! provider )
  {
    return nullptr;
  }

  std::unique_ptr<QgsRasterRenderer> renderer;

  if ( type() == Qgis::RasterAttributeTableType::Thematic )
  {
    std::unique_ptr<QgsColorRamp> ramp;
    if ( ! hasColor() )
    {
      ramp.reset( new QgsRandomColorRamp() );
    }
    const QgsPalettedRasterRenderer::MultiValueClassData classes = QgsPalettedRasterRenderer::rasterAttributeTableToClassData( this, classificationColumn, ramp.get() );
    if ( classes.isEmpty() )
      return nullptr;

    renderer = std::make_unique<QgsPalettedRasterRenderer>( provider,
               bandNumber,
               classes );
  }
  else
  {
    std::unique_ptr<QgsSingleBandPseudoColorRenderer> pseudoColorRenderer = std::make_unique<QgsSingleBandPseudoColorRenderer>( provider, bandNumber );
    QStringList labels;
    // Athematic classification is not supported, but the classificationColumn will be used for labels.
    // if it's not specified, try to guess it here.
    int labelColumn { classificationColumn };
    if ( labelColumn < 0 )
    {
      const QList<Qgis::RasterAttributeTableFieldUsage> fieldUsages { usages() };
      labelColumn = fieldUsages.indexOf( Qgis::RasterAttributeTableFieldUsage::Name );
      if ( labelColumn < 0 )
      {
        labelColumn = fieldUsages.indexOf( Qgis::RasterAttributeTableFieldUsage::Generic );
      }
    }
    QgsGradientColorRamp *ramp { colorRamp( labels, labelColumn ).clone() };
    pseudoColorRenderer->setClassificationMin( minimumValue() );
    pseudoColorRenderer->setClassificationMax( maximumValue() );
    // Use discrete for single colors, interpolated for ramps
    pseudoColorRenderer->createShader( ramp, hasRamp() ? QgsColorRampShader::Type::Interpolated : QgsColorRampShader::Type::Discrete, QgsColorRampShader::ClassificationMode::Continuous, ramp->stops().count() + 2, true );
    if ( pseudoColorRenderer->shader() )
    {
      pseudoColorRenderer->shader()->setMaximumValue( maximumValue() );
      pseudoColorRenderer->shader()->setMinimumValue( minimumValue() );
      // Set labels
      if ( QgsColorRampShader *shaderFunction = static_cast<QgsColorRampShader *>( pseudoColorRenderer->shader()->rasterShaderFunction() ) )
      {
        shaderFunction->setMinimumValue( minimumValue() );
        shaderFunction->setMaximumValue( maximumValue() );
        const bool labelsAreUsable { ramp->count() > 2 && labels.count() == ramp->count() - 1 };

        if ( labelsAreUsable )
        {
          QList<QgsColorRampShader::ColorRampItem> newItemList;
          const double range { maximumValue() - minimumValue() };
          int stopIdx { 0 };
          for ( const QString &label : std::as_const( labels ) )
          {
            if ( stopIdx >= ramp->count() - 2 )
            {
              break;
            }
            double value { minimumValue() + ramp->stops().at( stopIdx ).offset * range };
            QgsColorRampShader::ColorRampItem item { value, ramp->stops().at( stopIdx ).color, label };
            newItemList.push_back( item );
            stopIdx++;
          }

          QgsColorRampShader::ColorRampItem item { maximumValue(), ramp->color2(), labels.last() };
          newItemList.push_back( item );
          shaderFunction->setColorRampItemList( newItemList );
        }
      }
    }
    renderer.reset( pseudoColorRenderer.release() );
  }

  return renderer.release();
}

QList<QList<QVariant> > QgsRasterAttributeTable::orderedRows() const
{
  const QList<Qgis::RasterAttributeTableFieldUsage> fieldUsages { usages() };
  const int minIdx { static_cast<int>( fieldUsages.indexOf( Qgis::RasterAttributeTableFieldUsage::Min ) ) };
  const int maxIdx { static_cast<int>( fieldUsages.indexOf( Qgis::RasterAttributeTableFieldUsage::Max ) ) };
  const bool isRange { minIdx >= 0 && maxIdx >= 0 };
  QList<QVariantList> dataCopy( mData );

  if ( isRange )
  {
    std::sort( dataCopy.begin(), dataCopy.end(), [ & ]( const QVariantList & first, const QVariantList & second ) -> bool
    {
      return ( first.at( maxIdx ).toDouble() + first.at( minIdx ).toDouble() )  < ( second.at( maxIdx ).toDouble() + second.at( minIdx ).toDouble() );
    } );
  }
  else
  {
    const int minMaxIdx { static_cast<int>( fieldUsages.indexOf( Qgis::RasterAttributeTableFieldUsage::MinMax ) ) };
    if ( minMaxIdx < 0 )
    {
      return dataCopy;
    }
    else
    {
      std::sort( dataCopy.begin(), dataCopy.end(), [ & ]( const QVariantList & first, const QVariantList & second ) -> bool
      {
        return first.at( minMaxIdx ).toDouble() < second.at( minMaxIdx ).toDouble();
      } );
    }
  }

  return dataCopy;
}

bool QgsRasterAttributeTable::Field::isColor() const
{
  return usage == Qgis::RasterAttributeTableFieldUsage::Red || usage == Qgis::RasterAttributeTableFieldUsage::Green || usage == Qgis::RasterAttributeTableFieldUsage::Blue || usage == Qgis::RasterAttributeTableFieldUsage::Alpha;
}

bool QgsRasterAttributeTable::Field::isRamp() const
{
  return usage == Qgis::RasterAttributeTableFieldUsage::RedMin || usage == Qgis::RasterAttributeTableFieldUsage::GreenMin || usage == Qgis::RasterAttributeTableFieldUsage::BlueMin || usage == Qgis::RasterAttributeTableFieldUsage::AlphaMin || usage == Qgis::RasterAttributeTableFieldUsage::RedMax || usage == Qgis::RasterAttributeTableFieldUsage::GreenMax || usage == Qgis::RasterAttributeTableFieldUsage::BlueMax || usage == Qgis::RasterAttributeTableFieldUsage::AlphaMax;
}
