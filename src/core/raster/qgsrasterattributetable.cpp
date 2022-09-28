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

bool QgsRasterAttributeTable::hasColor()
{
  QList<Qgis::RasterAttributeTableFieldUsage> usages;
  for ( const QgsRasterAttributeTable::Field &field : std::as_const( mFields ) )
  {
    usages.push_back( field.usage );
  }
  return ( usages.contains( Qgis::RasterAttributeTableFieldUsage::Red ) && usages.contains( Qgis::RasterAttributeTableFieldUsage::Green ) && usages.contains( Qgis::RasterAttributeTableFieldUsage::Blue ) ) ||
         ( usages.contains( Qgis::RasterAttributeTableFieldUsage::RedMin ) && usages.contains( Qgis::RasterAttributeTableFieldUsage::GreenMin ) && usages.contains( Qgis::RasterAttributeTableFieldUsage::BlueMin ) && usages.contains( Qgis::RasterAttributeTableFieldUsage::RedMax ) && usages.contains( Qgis::RasterAttributeTableFieldUsage::GreenMax ) && usages.contains( Qgis::RasterAttributeTableFieldUsage::BlueMax ) );
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

bool QgsRasterAttributeTable::insertField( const Field &field, int position )
{
  if ( position < 0 )
  {
    return false;
  }

  int realPos { std::min( static_cast<int>( mFields.count() ), position ) };

  mFields.insert( realPos, field );

  for ( auto it = mData.begin(); it != mData.end(); ++it )
  {
    it->insert( realPos, QVariant( field.type ) );
  }

  setIsDirty( true );

  return true;
}

bool QgsRasterAttributeTable::insertField( const QString &name, Qgis::RasterAttributeTableFieldUsage usage, QVariant::Type type, int position )
{
  return insertField( { name, usage, type}, position );
}

bool QgsRasterAttributeTable::appendField( const QString &name, Qgis::RasterAttributeTableFieldUsage usage, QVariant::Type type )
{
  return insertField( name, usage, type, mFields.count() );
}

bool QgsRasterAttributeTable::appendField( const Field &field )
{
  return insertField( field, mFields.count() );
}

bool QgsRasterAttributeTable::removeField( const QString &name )
{
  const auto toRemove { std::find_if( mFields.begin(), mFields.end(), [ &name ]( Field & f ) -> bool {
      return f.name == name;
    } )};

  if ( toRemove != mFields.end() )
  {
    const int idx { static_cast<int>( std::distance( mFields.begin(), toRemove ) ) };
    mFields.erase( toRemove, mFields.end() );
    for ( auto it = mData.begin(); it != mData.end(); ++it )
    {
      it->removeAt( idx );
    }
    setIsDirty( true );
    return true;
  }

  return false;
}

bool QgsRasterAttributeTable::insertRow( const QVariantList data, int position )
{
  if ( position < 0 )
  {
    return false;
  }

  QVariantList dataValid;

  if ( dataValid.size() > mFields.size() )
  {
    for ( int colIdx = 0; colIdx < mFields.size(); colIdx++ )
    {
      dataValid.append( data[ colIdx ] );
    }
  }
  else
  {
    dataValid = data;
  }
  mData.insert( position, dataValid );
  setIsDirty( true );
  return true;
}

bool QgsRasterAttributeTable::appendRow( const QVariantList data )
{
  return insertRow( data, mData.count() );
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
      *errorMessage = QObject::tr( "Error creating RAT table: %1." ).arg( writer->errorMessage() );
    }
    return false;
  }

  QgsFeatureList features { qgisFeatures() };
  bool result { writer->addFeatures( features ) };

  if ( ! result )
  {
    if ( errorMessage )
    {
      *errorMessage = QObject::tr( "Error creating RAT table: could not add rows." );
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
  QgsOgrProvider rat { path, QgsDataProvider::ProviderOptions() };
  if ( ! rat.isValid() )
  {
    if ( errorMessage )
    {
      *errorMessage = QObject::tr( "Error reading RAT table from file: invalid layer." );
    }
    return false;
  }

  QList<Field> oldFields = mFields;
  QList<QVariantList> oldData = mData;

  mFields.clear();
  mData.clear();

  bool hasValueField { false };
  for ( const QgsField &field : rat.fields() )
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
    appendField( ratField );
  }

  // Do we have a value field? If not, try to guess one
  if ( ! hasValueField && mFields.count() > 1 && ( mFields.at( 0 ).type == QVariant::Int || mFields.at( 0 ).type == QVariant::Char || mFields.at( 0 ).type == QVariant::UInt || mFields.at( 0 ).type == QVariant::LongLong || mFields.at( 0 ).type == QVariant::ULongLong ) )
  {
    mFields[0].usage = Qgis::RasterAttributeTableFieldUsage::MinMax;
  }

  const int fieldCount { static_cast<int>( fields().count( ) ) };
  QgsFeature f;
  QgsFeatureIterator fit { rat.getFeatures( QgsFeatureRequest() ) };
  while ( fit.nextFeature( f ) )
  {
    if ( f.attributeCount() != fieldCount )
    {
      if ( errorMessage )
      {
        *errorMessage = QObject::tr( "Error reading RAT table from file: number of fields and number of attributes do not match." );
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


bool QgsRasterAttributeTable::isValid() const
{
  // TODO: check for mandatory fields
  return mFields.count() > 0 && mData.count( ) > 0;
}

const QList<QList<QVariant> > QgsRasterAttributeTable::data() const
{
  return mData;
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
    else if ( fieldLower.contains( "red" ) || fieldLower == QStringLiteral( "r" ) )
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
    else if ( fieldLower == QStringLiteral( "value" ) )
    {
      return Qgis::RasterAttributeTableFieldUsage::MinMax;
    }
    else if ( fieldLower == QStringLiteral( "count" ) )
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
