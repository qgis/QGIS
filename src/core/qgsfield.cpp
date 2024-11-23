#include "moc_qgsfield.cpp"
/***************************************************************************
       qgsfield.cpp - Describes a field in a layer or table
        --------------------------------------
       Date                 : 01-Jan-2004
       Copyright            : (C) 2004 by Gary E.Sherman
       email                : sherman at mrcc.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfield_p.h"
#include "qgis.h"
#include "qgsapplication.h"
#include "qgsreferencedgeometry.h"
#include "qgsvariantutils.h"

#include <QDataStream>
#include <QIcon>
#include <QLocale>
#include <QJsonDocument>

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfield.cpp.
 * See details in QEP #17
 ****************************************************************************/

#if 0
QgsField::QgsField( QString nam, QString typ, int len, int prec, bool num,
                    QString comment )
  : mName( nam ), mType( typ ), mLength( len ), mPrecision( prec ), mNumeric( num )
  , mComment( comment )
{
  // This function used to lower case the field name since some stores
  // use upper case (e.g., shapefiles), but that caused problems with
  // attribute actions getting confused between uppercase and
  // lowercase versions of the attribute names, so just leave the
  // names how they are now.
}
#endif
QgsField::QgsField( const QString &name, QMetaType::Type type,
                    const QString &typeName, int len, int prec, const QString &comment, QMetaType::Type subType )
{
  d = new QgsFieldPrivate( name, type, subType, typeName, len, prec, comment );
}

QgsField::QgsField( const QString &name, QVariant::Type type,
                    const QString &typeName, int len, int prec, const QString &comment, QVariant::Type subType )
  : QgsField( name, QgsVariantUtils::variantTypeToMetaType( type ), typeName, len, prec, comment, QgsVariantUtils::variantTypeToMetaType( subType ) )
{
}

QgsField::QgsField( const QgsField &other ) //NOLINT
  : d( other.d )
{

}

QgsField::~QgsField() = default;

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfield.cpp.
 * See details in QEP #17
 ****************************************************************************/

QgsField &QgsField::operator =( const QgsField &other )  //NOLINT
{
  d = other.d;
  return *this;
}

bool QgsField::operator==( const QgsField &other ) const
{
  return *( other.d ) == *d;
}

bool QgsField::operator!=( const QgsField &other ) const
{
  return !( *this == other );
}

QString QgsField::name() const
{
  return d->name;
}

QString QgsField::displayName() const
{
  if ( !d->alias.isEmpty() )
    return d->alias;
  else
    return d->name;
}

QString QgsField::displayNameWithAlias() const
{
  if ( alias().isEmpty() )
  {
    return name();
  }
  return QStringLiteral( "%1 (%2)" ).arg( name(), alias() );
}

QString QgsField::displayType( const bool showConstraints ) const
{
  QString typeStr = typeName();
  if ( typeStr.isEmpty() )
  {
    typeStr = QgsVariantUtils::typeToDisplayString( type(), subType() );
  }

  if ( length() > 0 && precision() > 0 )
    typeStr += QStringLiteral( "(%1, %2)" ).arg( length() ).arg( precision() );
  else if ( length() > 0 )
    typeStr += QStringLiteral( "(%1)" ).arg( length() );

  if ( showConstraints )
  {
    typeStr += ( constraints().constraints() & QgsFieldConstraints::ConstraintNotNull )
               ? QStringLiteral( " NOT NULL" )
               : QStringLiteral( " NULL" );

    typeStr += ( constraints().constraints() & QgsFieldConstraints::ConstraintUnique )
               ? QStringLiteral( " UNIQUE" )
               : QString();
  }

  return typeStr;
}

QString QgsField::friendlyTypeString() const
{
  if ( d->type == QMetaType::Type::User )
  {
    if ( d->typeName.compare( QLatin1String( "geometry" ), Qt::CaseInsensitive ) == 0 )
    {
      return QObject::tr( "Geometry" );
    }
  }
  return QgsVariantUtils::typeToDisplayString( d->type, d->subType );
}

QMetaType::Type QgsField::type() const
{
  return d->type;
}

QMetaType::Type QgsField::subType() const
{
  return d->subType;
}

QString QgsField::typeName() const
{
  return d->typeName;
}

int QgsField::length() const
{
  return d->length;
}

int QgsField::precision() const
{
  return d->precision;
}

QString QgsField::comment() const
{
  return d->comment;
}

QVariant QgsField::metadata( int property ) const
{
  return d->metadata.value( property );
}

QMap<int, QVariant> QgsField::metadata() const
{
  return d->metadata;
}

QVariant QgsField::metadata( Qgis::FieldMetadataProperty property ) const
{
  return d->metadata.value( static_cast< int >( property ) );
}

void QgsField::setMetadata( const QMap<int, QVariant> metadata )
{
  d->metadata = metadata;
}

void QgsField::setMetadata( Qgis::FieldMetadataProperty property, const QVariant &value )
{
  d->metadata[ static_cast< int >( property )] = value;
}

void QgsField::setMetadata( int property, const QVariant &value )
{
  d->metadata[ property ] = value;
}

bool QgsField::isNumeric() const
{
  return d->type == QMetaType::Type::Double || d->type == QMetaType::Type::Int || d->type == QMetaType::Type::UInt || d->type == QMetaType::Type::LongLong || d->type == QMetaType::Type::ULongLong;
}

bool QgsField::isDateOrTime() const
{
  return d->type == QMetaType::Type::QDate || d->type == QMetaType::Type::QTime || d->type == QMetaType::Type::QDateTime;
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfield.cpp.
 * See details in QEP #17
 ****************************************************************************/

void QgsField::setName( const QString &name )
{
  d->name = name;
}

void QgsField::setType( QMetaType::Type type )
{
  d->type = type;
}

void QgsField::setType( QVariant::Type type )
{
  setType( QgsVariantUtils::variantTypeToMetaType( type ) );
}

void QgsField::setSubType( QMetaType::Type subType )
{
  d->subType = subType;
}

void QgsField::setSubType( QVariant::Type subType )
{
  setSubType( QgsVariantUtils::variantTypeToMetaType( subType ) );
}

void QgsField::setTypeName( const QString &typeName )
{
  d->typeName = typeName;
}

void QgsField::setLength( int len )
{
  d->length = len;
}
void QgsField::setPrecision( int precision )
{
  d->precision = precision;
}

void QgsField::setComment( const QString &comment )
{
  d->comment = comment;
}

QgsDefaultValue QgsField::defaultValueDefinition() const
{
  return d->defaultValueDefinition;
}

void QgsField::setDefaultValueDefinition( const QgsDefaultValue &defaultValueDefinition )
{
  d->defaultValueDefinition = defaultValueDefinition;
}

void QgsField::setConstraints( const QgsFieldConstraints &constraints )
{
  d->constraints = constraints;
}

const QgsFieldConstraints &QgsField::constraints() const
{
  return d->constraints;
}

QString QgsField::alias() const
{
  return d->alias;
}

void QgsField::setAlias( const QString &alias )
{
  d->alias = alias;
}

Qgis::FieldConfigurationFlags QgsField::configurationFlags() const
{
  return d->flags;
}

void QgsField::setConfigurationFlags( Qgis::FieldConfigurationFlags flags )
{
  d->flags = flags;
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfield.cpp.
 * See details in QEP #17
 ****************************************************************************/

QString QgsField::displayString( const QVariant &v ) const
{
  if ( QgsVariantUtils::isNull( v ) )
  {
    return QgsApplication::nullRepresentation();
  }

  if ( v.userType() == qMetaTypeId<QgsReferencedGeometry>() )
  {
    QgsReferencedGeometry geom = qvariant_cast<QgsReferencedGeometry>( v );
    if ( geom.isNull() )
      return QgsApplication::nullRepresentation();
    else
    {
      QString wkt = geom.asWkt();
      if ( wkt.length() >= 1050 )
      {
        wkt = wkt.left( MAX_WKT_LENGTH ) + QChar( 0x2026 );
      }
      QString formattedText = QStringLiteral( "%1 [%2]" ).arg( wkt, geom.crs().userFriendlyIdentifier() );
      return formattedText;
    }
  }

  // Special treatment for numeric types if group separator is set or decimalPoint is not a dot
  if ( d->type == QMetaType::Type::Double )
  {
    // if value doesn't contain a double (a default value expression for instance),
    // apply no transformation
    bool ok;
    v.toDouble( &ok );
    if ( !ok )
      return v.toString();

    // Locales with decimal point != '.' or that require group separator: use QLocale
    if ( QLocale().decimalPoint() != '.' ||
         !( QLocale().numberOptions() & QLocale::NumberOption::OmitGroupSeparator ) )
    {
      if ( d->precision > 0 )
      {
        if ( -1 < v.toDouble() && v.toDouble() < 1 )
        {
          return QLocale().toString( v.toDouble(), 'g', d->precision );
        }
        else
        {
          return QLocale().toString( v.toDouble(), 'f', d->precision );
        }
      }
      else
      {
        // Precision is not set, let's guess it from the
        // standard conversion to string
        const QString s( v.toString() );
        const int dotPosition( s.indexOf( '.' ) );
        int precision;
        if ( dotPosition < 0 && s.indexOf( 'e' ) < 0 )
        {
          precision = 0;
          return QLocale().toString( v.toDouble(), 'f', precision );
        }
        else
        {
          if ( dotPosition < 0 ) precision = 0;
          else precision = s.length() - dotPosition - 1;

          if ( -1 < v.toDouble() && v.toDouble() < 1 )
          {
            return QLocale().toString( v.toDouble(), 'g', precision );
          }
          else
          {
            return QLocale().toString( v.toDouble(), 'f', precision );
          }
        }
      }
    }
    // Default for doubles with precision
    else if ( d->precision > 0 )
    {
      if ( -1 < v.toDouble() && v.toDouble() < 1 )
      {
        return QString::number( v.toDouble(), 'g', d->precision );
      }
      else
      {
        return QString::number( v.toDouble(), 'f', d->precision );
      }
    }
    else
    {
      const double vDouble = v.toDouble();
      // mimic Qt 5 handling of when to switch to exponential forms
      if ( std::fabs( vDouble ) < 1e-04 )
        return QString::number( vDouble, 'g', QLocale::FloatingPointShortest );
      else
        return QString::number( vDouble, 'f', QLocale::FloatingPointShortest );
    }
  }
  // Other numeric types than doubles
  else if ( isNumeric() &&
            !( QLocale().numberOptions() & QLocale::NumberOption::OmitGroupSeparator ) )
  {
    bool ok;
    const qlonglong converted( v.toLongLong( &ok ) );
    if ( ok )
      return QLocale().toString( converted );
  }
  else if ( d->typeName.compare( QLatin1String( "json" ), Qt::CaseInsensitive ) == 0 || d->typeName == QLatin1String( "jsonb" ) )
  {
    const QJsonDocument doc = QJsonDocument::fromVariant( v );
    return QString::fromUtf8( doc.toJson().constData() );
  }
  else if ( d->type == QMetaType::Type::QByteArray )
  {
    return QObject::tr( "BLOB" );
  }
  else if ( d->type == QMetaType::Type::QStringList || d->type == QMetaType::Type::QVariantList )
  {
    QString result;
    const QVariantList list = v.toList();
    for ( const QVariant &var : list )
    {
      if ( !result.isEmpty() )
        result.append( QStringLiteral( ", " ) );
      result.append( var.toString() );
    }
    return result;
  }

  // Fallback if special rules do not apply
  return v.toString();
}

QString QgsField::readableConfigurationFlag( Qgis::FieldConfigurationFlag flag )
{
  switch ( flag )
  {
    case Qgis::FieldConfigurationFlag::NoFlag:
      return QObject::tr( "None" );
    case Qgis::FieldConfigurationFlag::NotSearchable:
      return QObject::tr( "Not searchable" );
    case Qgis::FieldConfigurationFlag::HideFromWms:
      return QObject::tr( "Do not expose via WMS" );
    case Qgis::FieldConfigurationFlag::HideFromWfs:
      return QObject::tr( "Do not expose via WFS" );
  }
  return QString();
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfield.cpp.
 * See details in QEP #17
 ****************************************************************************/

bool QgsField::convertCompatible( QVariant &v, QString *errorMessage ) const
{
  const QVariant original = v;
  if ( errorMessage )
    errorMessage->clear();

  if ( QgsVariantUtils::isNull( v ) )
  {
    v.convert( d->type );
    return true;
  }

  if ( d->type == QMetaType::Type::Int && v.toInt() != v.toLongLong() )
  {
    v = QgsVariantUtils::createNullVariant( d->type );
    if ( errorMessage )
      *errorMessage = QObject::tr( "Value \"%1\" is too large for integer field" ).arg( original.toLongLong() );
    return false;
  }

  // Give it a chance to convert to double since for not '.' locales
  // we accept both comma and dot as decimal point
  if ( d->type == QMetaType::Type::Double && v.userType() == QMetaType::Type::QString )
  {
    QVariant tmp( v );
    if ( !tmp.convert( d->type ) )
    {
      // This might be a string with thousand separator: use locale to convert
      bool ok = false;
      double d = qgsPermissiveToDouble( v.toString(), ok );
      if ( ok )
      {
        v = QVariant( d );
        return true;
      }
      // For not 'dot' locales, we also want to accept '.'
      if ( QLocale().decimalPoint() != '.' )
      {
        d = QLocale( QLocale::C ).toDouble( v.toString(), &ok );
        if ( ok )
        {
          v = QVariant( d );
          return true;
        }
      }
    }
  }

  // For string representation of an int we also might have thousand separator
  if ( d->type == QMetaType::Type::Int && v.userType() == QMetaType::Type::QString )
  {
    QVariant tmp( v );
    if ( !tmp.convert( d->type ) )
    {
      // This might be a string with thousand separator: use locale to convert
      bool ok;
      const int i = qgsPermissiveToInt( v.toString(), ok );
      if ( ok )
      {
        v = QVariant( i );
        return true;
      }
    }
  }

  // For string representation of a long we also might have thousand separator
  if ( d->type == QMetaType::Type::LongLong && v.userType() == QMetaType::Type::QString )
  {
    QVariant tmp( v );
    if ( !tmp.convert( d->type ) )
    {
      // This might be a string with thousand separator: use locale to convert
      bool ok;
      const qlonglong l = qgsPermissiveToLongLong( v.toString(), ok );
      if ( ok )
      {
        v = QVariant( l );
        return true;
      }
    }
  }

  //String representations of doubles in QVariant will return false to convert( QVariant::Int )
  //work around this by first converting to double, and then checking whether the double is convertible to int
  if ( d->type == QMetaType::Type::Int && v.canConvert( QMetaType::Type::Double ) )
  {
    bool ok = false;
    const double dbl = v.toDouble( &ok );
    if ( !ok )
    {
      //couldn't convert to number
      v = QgsVariantUtils::createNullVariant( d->type );

      if ( errorMessage )
        *errorMessage = QObject::tr( "Value \"%1\" is not a number" ).arg( original.toString() );

      return false;
    }

    const double round = std::round( dbl );
    if ( round  > std::numeric_limits<int>::max() || round < -std::numeric_limits<int>::max() )
    {
      //double too large to fit in int
      v = QgsVariantUtils::createNullVariant( d->type );

      if ( errorMessage )
        *errorMessage = QObject::tr( "Value \"%1\" is too large for integer field" ).arg( original.toDouble() );

      return false;
    }
    v = QVariant( static_cast< int >( std::round( dbl ) ) );
    return true;
  }

  //String representations of doubles in QVariant will return false to convert( QVariant::LongLong )
  //work around this by first converting to double, and then checking whether the double is convertible to longlong
  if ( d->type == QMetaType::Type::LongLong && v.canConvert( QMetaType::Type::Double ) )
  {
    //firstly test the conversion to longlong because conversion to double will rounded the value
    QVariant tmp( v );
    if ( !tmp.convert( d->type ) )
    {
      bool ok = false;
      const double dbl = v.toDouble( &ok );
      if ( !ok )
      {
        //couldn't convert to number
        v = QgsVariantUtils::createNullVariant( d->type );

        if ( errorMessage )
          *errorMessage = QObject::tr( "Value \"%1\" is not a number" ).arg( original.toString() );

        return false;
      }

      const double round = std::round( dbl );
      if ( round  > static_cast<double>( std::numeric_limits<long long>::max() ) || round < static_cast<double>( -std::numeric_limits<long long>::max() ) )
      {
        //double too large to fit in longlong
        v = QgsVariantUtils::createNullVariant( d->type );

        if ( errorMessage )
          *errorMessage = QObject::tr( "Value \"%1\" is too large for long long field" ).arg( original.toDouble() );

        return false;
      }
      v = QVariant( static_cast< long long >( std::round( dbl ) ) );
      return true;
    }
  }

  if ( d->typeName.compare( QLatin1String( "json" ), Qt::CaseInsensitive ) == 0 || d->typeName.compare( QLatin1String( "jsonb" ), Qt::CaseInsensitive ) == 0 )
  {
    if ( d->type == QMetaType::Type::QString )
    {
      const QJsonDocument doc = QJsonDocument::fromVariant( v );
      if ( !doc.isNull() )
      {
        v = QString::fromUtf8( doc.toJson( QJsonDocument::Compact ).constData() );
        return true;
      }
      v = QgsVariantUtils::createNullVariant( d->type );
      return false;
    }
    else if ( d->type == QMetaType::Type::QVariantMap )
    {
      if ( v.userType() == QMetaType::Type::QStringList || v.userType() == QMetaType::Type::QVariantList || v.userType() == QMetaType::Type::QVariantMap )
      {
        return true;
      }
      v = QgsVariantUtils::createNullVariant( d->type );
      return false;
    }
  }

  if ( ( d->type == QMetaType::Type::QStringList || ( d->type == QMetaType::Type::QVariantList && d->subType == QMetaType::Type::QString ) )
       && ( v.userType() == QMetaType::Type::QString ) )
  {
    v = QStringList( { v.toString() } );
    return true;
  }

  if ( ( d->type == QMetaType::Type::QStringList || d->type == QMetaType::Type::QVariantList ) && !( v.userType() == QMetaType::Type::QStringList || v.userType() == QMetaType::Type::QVariantList ) )
  {
    v = QgsVariantUtils::createNullVariant( d->type );

    if ( errorMessage )
      *errorMessage = QObject::tr( "Could not convert value \"%1\" to target list type" ).arg( original.toString() );

    return false;
  }

  // Handle referenced geometries (e.g. from additional geometry fields)
  if ( d->type == QMetaType::Type::QString && v.userType() == qMetaTypeId<QgsReferencedGeometry>() )
  {
    const QgsReferencedGeometry geom { v.value<QgsReferencedGeometry>( ) };
    if ( geom.isNull() )
    {
      v = QgsVariantUtils::createNullVariant( d->type );
    }
    else
    {
      v = QVariant( geom.asWkt() );
    }
    return true;
  }
  else if ( d->type == QMetaType::Type::User && d->typeName.compare( QLatin1String( "geometry" ), Qt::CaseInsensitive ) == 0 )
  {
    if ( v.userType() == qMetaTypeId<QgsReferencedGeometry>() || v.userType() == qMetaTypeId< QgsGeometry>() )
    {
      return true;
    }
    else if ( v.userType() == QMetaType::Type::QString )
    {
      const QgsGeometry geom = QgsGeometry::fromWkt( v.toString() );
      if ( !geom.isNull() )
      {
        v = QVariant::fromValue( geom );
        return true;
      }
    }
    return false;
  }
  else if ( !v.convert( d->type ) )
  {
    v = QgsVariantUtils::createNullVariant( d->type );

    if ( errorMessage )
      *errorMessage = QObject::tr( "Could not convert value \"%1\" to target type \"%2\"" )
                      .arg( original.toString(),
                            d->typeName );

    return false;
  }

  if ( d->type == QMetaType::Type::Double && d->precision > 0 )
  {
    const double s = std::pow( 10, d->precision );
    const double d = v.toDouble() * s;
    v = QVariant( ( d < 0 ? std::ceil( d - 0.5 ) : std::floor( d + 0.5 ) ) / s );
    return true;
  }

  if ( d->type == QMetaType::Type::QString && d->length > 0 && v.toString().length() > d->length )
  {
    const int length = v.toString().length();
    v = v.toString().left( d->length );

    if ( errorMessage )
      *errorMessage = QObject::tr( "String of length %1 exceeds maximum field length (%2)" ).arg( length ).arg( d->length );

    return false;
  }

  return true;
}

QgsField::operator QVariant() const
{
  return QVariant::fromValue( *this );
}

void QgsField::setEditorWidgetSetup( const QgsEditorWidgetSetup &v )
{
  d->editorWidgetSetup = v;
}

QgsEditorWidgetSetup QgsField::editorWidgetSetup() const
{
  return d->editorWidgetSetup;
}

void QgsField::setReadOnly( bool readOnly )
{
  d->isReadOnly = readOnly;
}

bool QgsField::isReadOnly() const
{
  return d->isReadOnly;
}

Qgis::FieldDomainSplitPolicy QgsField::splitPolicy() const
{
  return d->splitPolicy;
}

void QgsField::setSplitPolicy( Qgis::FieldDomainSplitPolicy policy )
{
  d->splitPolicy = policy;
}

Qgis::FieldDuplicatePolicy QgsField::duplicatePolicy() const
{
  return d->duplicatePolicy;
}

void QgsField::setDuplicatePolicy( Qgis::FieldDuplicatePolicy policy )
{
  d->duplicatePolicy = policy;
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfield.cpp.
 * See details in QEP #17
 ****************************************************************************/

QDataStream &operator<<( QDataStream &out, const QgsField &field )
{
  out << field.name();
  out << static_cast< quint32 >( field.type() );
  out << field.typeName();
  out << field.length();
  out << field.precision();
  out << field.comment();
  out << field.alias();
  out << field.defaultValueDefinition().expression();
  out << field.defaultValueDefinition().applyOnUpdate();
  out << field.constraints().constraints();
  out << static_cast< quint32 >( field.constraints().constraintOrigin( QgsFieldConstraints::ConstraintNotNull ) );
  out << static_cast< quint32 >( field.constraints().constraintOrigin( QgsFieldConstraints::ConstraintUnique ) );
  out << static_cast< quint32 >( field.constraints().constraintOrigin( QgsFieldConstraints::ConstraintExpression ) );
  out << static_cast< quint32 >( field.constraints().constraintStrength( QgsFieldConstraints::ConstraintNotNull ) );
  out << static_cast< quint32 >( field.constraints().constraintStrength( QgsFieldConstraints::ConstraintUnique ) );
  out << static_cast< quint32 >( field.constraints().constraintStrength( QgsFieldConstraints::ConstraintExpression ) );
  out << field.constraints().constraintExpression();
  out << field.constraints().constraintDescription();
  out << static_cast< quint32 >( field.subType() );
  out << static_cast< int >( field.splitPolicy() );
  out << static_cast< int >( field.duplicatePolicy() );
  out << field.metadata();
  return out;
}

QDataStream &operator>>( QDataStream &in, QgsField &field )
{
  quint32 type;
  quint32 subType;
  quint32 length;
  quint32 precision;
  quint32 constraints;
  quint32 originNotNull;
  quint32 originUnique;
  quint32 originExpression;
  quint32 strengthNotNull;
  quint32 strengthUnique;
  quint32 strengthExpression;
  int splitPolicy;
  int duplicatePolicy;

  bool applyOnUpdate;

  QString name;
  QString typeName;
  QString comment;
  QString alias;
  QString defaultValueExpression;
  QString constraintExpression;
  QString constraintDescription;
  QMap< int, QVariant > metadata;

  in >> name >> type >> typeName >> length >> precision >> comment >> alias
     >> defaultValueExpression >> applyOnUpdate >> constraints >> originNotNull >> originUnique >> originExpression >> strengthNotNull >> strengthUnique >> strengthExpression >>
     constraintExpression >> constraintDescription >> subType >> splitPolicy >> duplicatePolicy >> metadata;
  field.setName( name );
  field.setType( static_cast< QMetaType::Type >( type ) );
  field.setTypeName( typeName );
  field.setLength( static_cast< int >( length ) );
  field.setPrecision( static_cast< int >( precision ) );
  field.setComment( comment );
  field.setAlias( alias );
  field.setDefaultValueDefinition( QgsDefaultValue( defaultValueExpression, applyOnUpdate ) );
  field.setSplitPolicy( static_cast< Qgis::FieldDomainSplitPolicy >( splitPolicy ) );
  field.setDuplicatePolicy( static_cast< Qgis::FieldDuplicatePolicy >( duplicatePolicy ) );
  QgsFieldConstraints fieldConstraints;
  if ( constraints & QgsFieldConstraints::ConstraintNotNull )
  {
    fieldConstraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, static_cast< QgsFieldConstraints::ConstraintOrigin>( originNotNull ) );
    fieldConstraints.setConstraintStrength( QgsFieldConstraints::ConstraintNotNull, static_cast< QgsFieldConstraints::ConstraintStrength>( strengthNotNull ) );
  }
  else
    fieldConstraints.removeConstraint( QgsFieldConstraints::ConstraintNotNull );
  if ( constraints & QgsFieldConstraints::ConstraintUnique )
  {
    fieldConstraints.setConstraint( QgsFieldConstraints::ConstraintUnique, static_cast< QgsFieldConstraints::ConstraintOrigin>( originUnique ) );
    fieldConstraints.setConstraintStrength( QgsFieldConstraints::ConstraintUnique, static_cast< QgsFieldConstraints::ConstraintStrength>( strengthUnique ) );
  }
  else
    fieldConstraints.removeConstraint( QgsFieldConstraints::ConstraintUnique );
  if ( constraints & QgsFieldConstraints::ConstraintExpression )
  {
    fieldConstraints.setConstraint( QgsFieldConstraints::ConstraintExpression, static_cast< QgsFieldConstraints::ConstraintOrigin>( originExpression ) );
    fieldConstraints.setConstraintStrength( QgsFieldConstraints::ConstraintExpression, static_cast< QgsFieldConstraints::ConstraintStrength>( strengthExpression ) );
  }
  else
    fieldConstraints.removeConstraint( QgsFieldConstraints::ConstraintExpression );
  fieldConstraints.setConstraintExpression( constraintExpression, constraintDescription );
  field.setConstraints( fieldConstraints );
  field.setSubType( static_cast< QMetaType::Type >( subType ) );
  field.setMetadata( metadata );
  return in;
}
