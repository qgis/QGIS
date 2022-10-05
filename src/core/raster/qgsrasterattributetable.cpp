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

Qgis::RasterAttributeTableType QgsRasterAttributeTable::type() const
{
  return mType;
}

void QgsRasterAttributeTable::setType( const Qgis::RasterAttributeTableType type )
{
  mType = type;
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

QgsRasterAttributeTable::Ramp QgsRasterAttributeTable::ramp( int row ) const
{
  if ( ! hasRamp() || row < 0 || row >= mData.count() )
  {
    return Ramp();
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
  return Ramp{ colorMin, colorMax };
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

void QgsRasterAttributeTable::setIsDirty( bool isDirty )
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

  setIsDirty( true );

  return true;
}

bool QgsRasterAttributeTable::insertField( int position, const QString &name, const Qgis::RasterAttributeTableFieldUsage usage, const QVariant::Type type, QString *errorMessage )
{
  return insertField( position, { name, usage, type}, errorMessage );
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
    setIsDirty( true );
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
      *errorMessage = tr( "Row element count differs from field count (%1)'." ).arg( mFields.size() );
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
  setIsDirty( true );
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

  writer.reset( QgsVectorFileWriter::create( cleanedPath, qgisFields(), QgsWkbTypes::Type::NoGeometry, QgsCoordinateReferenceSystem(), QgsCoordinateTransformContext(), options ) );

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
    setIsDirty( false );
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

  setIsDirty( false );

  return true;
}


bool QgsRasterAttributeTable::isValid( QString *errorMessage ) const
{
  QStringList errors;

  if ( mFields.isEmpty() )
  {
    errors.push_back( tr( "RAT has no fields." ) );
  }

  if ( mData.isEmpty() )
  {
    errors.push_back( tr( "RAT has no rows." ) );
  }

  const QList<Qgis::RasterAttributeTableFieldUsage> fieldUsages { usages() };
  const bool isMinMax { fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::MinMax ) };
  const bool isValueRamp { fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::Min ) &&fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::Max ) };
  if ( ! isMinMax && ! isValueRamp )
  {
    errors.push_back( tr( "RAT has no MinMax nor a pair of Min and Max fields." ) );
  }

  // Check color
  if ( fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::Red ) || fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::Green ) || fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::Blue ) )
  {
    if ( !( fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::Red ) && fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::Green ) && fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::Blue ) ) )
    {
      errors.push_back( tr( "RAT has some but not all the fields required for color definition (Red, Green, Blue)." ) );
    }
    else if ( ! isMinMax )
    {
      errors.push_back( tr( "RAT has all the fields required for color definition (Red, Green, Blue) but no MinMax field." ) );
    }
  }

  // Check ramp
  if ( fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::RedMin ) || fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::GreenMin ) || fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::BlueMin ) || fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::RedMax ) || fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::GreenMax ) || fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::BlueMax ) )
  {
    if ( !( fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::RedMin ) && fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::GreenMin ) && fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::BlueMin ) && fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::RedMax ) && fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::GreenMax ) && fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::BlueMax ) ) )
    {
      errors.push_back( tr( "RAT has some but not all the fields required for color ramp definition (RedMin, GreenMin, BlueMin, RedMax, GreenMax, BlueMax)." ) );
    }
    else if ( ! isValueRamp )
    {
      errors.push_back( tr( "RAT has all the fields required for color ramp definition (RedMin, GreenMin, BlueMin, RedMax, GreenMax, BlueMax) but no Min and Max field." ) );
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

  mData[ row ][ column ] = newVal;

  return true;
}

QVariant QgsRasterAttributeTable::value( const int row, const int column )
{
  if ( row < 0 || row >= mData.count( ) || column < 0 || column >=  mData[ row ].count( ) )
  {
    return QVariant();
  }
  return mData[ row ][ column ];
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
    // Colors (not double)
    else if ( type != QVariant::Double )
    {
      if ( fieldLower.contains( "red" ) || fieldLower == QStringLiteral( "r" ) )
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
      else if ( fieldLower.contains( "green" ) || fieldLower == QStringLiteral( "g" ) )
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
      else if ( fieldLower.contains( "blue" ) || fieldLower == QStringLiteral( "b" ) )
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
      else if ( fieldLower.contains( "alpha" ) || fieldLower == QStringLiteral( "a" ) )
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
    else if ( fieldLower == QStringLiteral( "value" ) )
    {
      return Qgis::RasterAttributeTableFieldUsage::MinMax;
    }
    else if ( fieldLower == QLatin1String( "count" ) )
    {
      // This could really be max count but it's more likely pixel count
      return Qgis::RasterAttributeTableFieldUsage::PixelCount;
    }
  }
  else if ( type == QVariant::String )  // default to name for strings
  {
    return Qgis::RasterAttributeTableFieldUsage::Name;
  }

  // default to generic for not strings
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

QList<QgsRasterAttributeTable::MinMaxClass> QgsRasterAttributeTable::minMaxClasses( const int classificationColumn ) const
{
  QList<QgsRasterAttributeTable::MinMaxClass> classes;
  if ( !isValid() )
  {
    QgsDebugMsg( "minMaxClasses was called on an invalid RAT" );
    return classes;
  }

  const QList<Qgis::RasterAttributeTableFieldUsage> fieldUsages { usages() };
  if ( ! fieldUsages.contains( Qgis::RasterAttributeTableFieldUsage::MinMax ) )
  {
    QgsDebugMsg( "minMaxClasses was called on a ramp raster" );
    return classes;
  }

  const int minMaxIndex { fieldUsages.indexOf( Qgis::RasterAttributeTableFieldUsage::MinMax ) };

  Q_ASSERT( minMaxIndex >= 0 );

  int classificationIndex = classificationColumn;
  if ( classificationIndex >= 0 && classificationIndex < mFields.count( ) )
  {
    const Field classificationField { mFields.at( classificationIndex ) };
    if ( ( classificationField.usage != Qgis::RasterAttributeTableFieldUsage::Name && classificationField.usage != Qgis::RasterAttributeTableFieldUsage::Generic ) )
    {
      QgsDebugMsg( "minMaxClasses was called with a classification column which is not suitable for classification" );
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
    else
    {
      classificationIndex = fieldUsages.indexOf( Qgis::RasterAttributeTableFieldUsage::Generic );
      Q_ASSERT( classificationIndex >= 0 );
    }
  }
  else if ( classificationIndex >= mFields.count( ) )
  {
    QgsDebugMsg( "minMaxClasses was called with a classification column out of range" );
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
        QgsDebugMsg( "minMaxClasses could not convert a MinMax value to double" );
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


bool QgsRasterAttributeTable::Field::isColor() const
{
  return usage == Qgis::RasterAttributeTableFieldUsage::Red || usage == Qgis::RasterAttributeTableFieldUsage::Green || usage == Qgis::RasterAttributeTableFieldUsage::Blue || usage == Qgis::RasterAttributeTableFieldUsage::Alpha;
}

bool QgsRasterAttributeTable::Field::isRamp() const
{
  return usage == Qgis::RasterAttributeTableFieldUsage::RedMin || usage == Qgis::RasterAttributeTableFieldUsage::GreenMin || usage == Qgis::RasterAttributeTableFieldUsage::BlueMin || usage == Qgis::RasterAttributeTableFieldUsage::AlphaMin || usage == Qgis::RasterAttributeTableFieldUsage::RedMax || usage == Qgis::RasterAttributeTableFieldUsage::GreenMax || usage == Qgis::RasterAttributeTableFieldUsage::BlueMax || usage == Qgis::RasterAttributeTableFieldUsage::AlphaMax;
}
