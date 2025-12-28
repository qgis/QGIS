/***************************************************************************
     qgsgmlschema.cpp
     --------------------------------------
    Date                 : February 2013
    Copyright            : (C) 2013 by Radim Blazek
    Email                : radim.blazek@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsgmlschema.h"

#include <limits>

#include "qgscoordinatereferencesystem.h"
#include "qgserror.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsrectangle.h"

#include <QBuffer>
#include <QList>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProgressDialog>
#include <QSet>
#include <QSettings>
#include <QUrl>

#include "moc_qgsgmlschema.cpp"

#ifndef NS_SEPARATOR_DEFINED
#define NS_SEPARATOR_DEFINED
static const char NS_SEPARATOR = '?';
#endif

#define GML_NAMESPACE u"http://www.opengis.net/gml"_s


QgsGmlFeatureClass::QgsGmlFeatureClass( const QString &name, const QString &path )
  : mName( name )
  , mPath( path )
{
}

int QgsGmlFeatureClass::fieldIndex( const QString &name )
{
  for ( int i = 0; i < mFields.size(); i++ )
  {
    if ( mFields[i].name() == name ) return i;
  }
  return -1;
}

// --------------------------- QgsGmlSchema -------------------------------
QgsGmlSchema::QgsGmlSchema()
  : mSkipLevel( std::numeric_limits<int>::max() )
{
  mGeometryTypes << u"Point"_s << u"MultiPoint"_s
                 << u"LineString"_s << u"MultiLineString"_s
                 << u"Polygon"_s << u"MultiPolygon"_s;
}

QString QgsGmlSchema::readAttribute( const QString &attributeName, const XML_Char **attr ) const
{
  int i = 0;
  while ( attr[i] )
  {
    if ( attributeName.compare( attr[i] ) == 0 )
    {
      return QString( attr[i + 1] );
    }
    i += 2;
  }
  return QString();
}

bool QgsGmlSchema::parseXSD( const QByteArray &xml )
{
  QDomDocument dom;
  QString errorMsg;
  int errorLine;
  int errorColumn;
  if ( !dom.setContent( xml, false, &errorMsg, &errorLine, &errorColumn ) )
  {
    // TODO: error
    return false;
  }

  const QDomElement docElem = dom.documentElement();

  const QList<QDomElement> elementElements = domElements( docElem, u"element"_s );

  //QgsDebugMsgLevel( u"%1 elements read"_s.arg( elementElements.size() ), 2);

  const auto constElementElements = elementElements;
  for ( const QDomElement &elementElement : constElementElements )
  {
    const QString name = elementElement.attribute( u"name"_s );
    const QString type = elementElement.attribute( u"type"_s );

    const QString gmlBaseType = xsdComplexTypeGmlBaseType( docElem, stripNS( type ) );
    //QgsDebugMsgLevel( u"gmlBaseType = %1"_s.arg( gmlBaseType ), 2 );
    //QgsDebugMsgLevel( u"name = %1 gmlBaseType = %2"_s.arg( name ).arg( gmlBaseType ), 2 );
    // We should only use gml:AbstractFeatureType descendants which have
    // ancestor listed in gml:FeatureAssociationType (featureMember) descendant
    // But we could only loose some data if XSD was not correct, I think.

    if ( gmlBaseType == "AbstractFeatureType"_L1 )
    {
      // Get feature type definition
      QgsGmlFeatureClass featureClass( name, QString() );
      xsdFeatureClass( docElem, stripNS( type ), featureClass );
      mFeatureClassMap.insert( name, featureClass );
    }
    // A feature may have more geometries, we take just the first one
  }

  return true;
}

bool QgsGmlSchema::xsdFeatureClass( const QDomElement &element, const QString &typeName, QgsGmlFeatureClass &featureClass )
{
  //QgsDebugMsgLevel("typeName = " + typeName, 2 );
  const QDomElement complexTypeElement = domElement( element, u"complexType"_s, u"name"_s, typeName );
  if ( complexTypeElement.isNull() ) return false;

  // extension or restriction
  QDomElement extrest = domElement( complexTypeElement, u"complexContent.extension"_s );
  if ( extrest.isNull() )
  {
    extrest = domElement( complexTypeElement, u"complexContent.restriction"_s );
  }
  if ( extrest.isNull() ) return false;

  const QString extrestName = extrest.attribute( u"base"_s );
  if ( extrestName == "gml:AbstractFeatureType"_L1 )
  {
    // In theory we should add gml:AbstractFeatureType default attributes gml:description
    // and gml:name but it does not seem to be a common practice and we would probably
    // confuse most users
  }
  else
  {
    // Get attributes from extrest
    if ( !xsdFeatureClass( element, stripNS( extrestName ), featureClass ) ) return false;
  }

  // Supported geometry types
  QStringList geometryPropertyTypes;
  const auto constMGeometryTypes = mGeometryTypes;
  for ( const QString &geom : constMGeometryTypes )
  {
    geometryPropertyTypes << geom + "PropertyType";
  }

  QStringList geometryAliases;
  geometryAliases << u"location"_s << u"centerOf"_s << u"position"_s << u"extentOf"_s
                  << u"coverage"_s << u"edgeOf"_s << u"centerLineOf"_s << u"multiLocation"_s
                  << u"multiCenterOf"_s << u"multiPosition"_s << u"multiCenterLineOf"_s
                  << u"multiEdgeOf"_s << u"multiCoverage"_s << u"multiExtentOf"_s;

  // Add attributes from current comple type
  const QList<QDomElement> sequenceElements = domElements( extrest, u"sequence.element"_s );
  const auto constSequenceElements = sequenceElements;
  for ( const QDomElement &sequenceElement : constSequenceElements )
  {
    const QString fieldName = sequenceElement.attribute( u"name"_s );
    QString fieldTypeName = stripNS( sequenceElement.attribute( u"type"_s ) );
    const QString ref = sequenceElement.attribute( u"ref"_s );
    //QgsDebugMsg ( QString("fieldName = %1 fieldTypeName = %2 ref = %3").arg(fieldName).arg(fieldTypeName).arg(ref) );

    if ( !ref.isEmpty() )
    {
      if ( ref.startsWith( "gml:"_L1 ) )
      {
        if ( geometryAliases.contains( stripNS( ref ) ) )
        {
          featureClass.geometryAttributes().append( stripNS( ref ) );
        }
        else
        {
          QgsDebugError( u"Unknown referenced GML element: %1"_s.arg( ref ) );
        }
      }
      else
      {
        // TODO: get type from referenced element
        QgsDebugError( u"field %1.%2 is referencing %3 - not supported"_s.arg( typeName, fieldName ) );
      }
      continue;
    }

    if ( fieldName.isEmpty() )
    {
      QgsDebugError( u"field in %1 without name"_s.arg( typeName ) );
      continue;
    }

    // type is either type attribute
    if ( fieldTypeName.isEmpty() )
    {
      // or type is inheriting from xs:simpleType
      const QDomElement sequenceElementRestriction = domElement( sequenceElement, u"simpleType.restriction"_s );
      fieldTypeName = stripNS( sequenceElementRestriction.attribute( u"base"_s ) );
    }

    QMetaType::Type fieldType = QMetaType::Type::QString;
    if ( fieldTypeName.isEmpty() )
    {
      QgsDebugError( u"Cannot get %1.%2 field type"_s.arg( typeName, fieldName ) );
    }
    else
    {
      if ( geometryPropertyTypes.contains( fieldTypeName ) )
      {
        // Geometry attribute
        featureClass.geometryAttributes().append( fieldName );
        continue;
      }

      if ( fieldTypeName == "decimal"_L1 )
      {
        fieldType = QMetaType::Type::Double;
      }
      else if ( fieldTypeName == "integer"_L1 )
      {
        fieldType = QMetaType::Type::Int;
      }
    }

    const QgsField field( fieldName, fieldType, fieldTypeName );
    featureClass.fields().append( field );
  }

  return true;
}

QString QgsGmlSchema::xsdComplexTypeGmlBaseType( const QDomElement &element, const QString &name )
{
  //QgsDebugMsgLevel("name = " + name, 2 );
  const QDomElement complexTypeElement = domElement( element, u"complexType"_s, u"name"_s, name );
  if ( complexTypeElement.isNull() ) return QString();

  QDomElement extrest = domElement( complexTypeElement, u"complexContent.extension"_s );
  if ( extrest.isNull() )
  {
    extrest = domElement( complexTypeElement, u"complexContent.restriction"_s );
  }
  if ( extrest.isNull() ) return QString();

  const QString extrestName = extrest.attribute( u"base"_s );
  if ( extrestName.startsWith( "gml:"_L1 ) )
  {
    // GML base type found
    return stripNS( extrestName );
  }
  // Continue recursively until GML base type is reached
  return xsdComplexTypeGmlBaseType( element, stripNS( extrestName ) );
}

QString QgsGmlSchema::stripNS( const QString &name )
{
  return name.contains( ':' ) ? name.section( ':', 1 ) : name;
}

QList<QDomElement> QgsGmlSchema::domElements( const QDomElement &element, const QString &path )
{
  QList<QDomElement> list;

  QStringList names = path.split( '.' );
  if ( names.isEmpty() ) return list;
  const QString name = names.value( 0 );
  names.removeFirst();

  QDomNode n1 = element.firstChild();
  while ( !n1.isNull() )
  {
    const QDomElement el = n1.toElement();
    if ( !el.isNull() )
    {
      const QString tagName = stripNS( el.tagName() );
      if ( tagName == name )
      {
        if ( names.isEmpty() )
        {
          list.append( el );
        }
        else
        {
          list.append( domElements( el, names.join( QLatin1Char( '.' ) ) ) );
        }
      }
    }
    n1 = n1.nextSibling();
  }

  return list;
}

QDomElement QgsGmlSchema::domElement( const QDomElement &element, const QString &path )
{
  return domElements( element, path ).value( 0 );
}

QList<QDomElement> QgsGmlSchema::domElements( QList<QDomElement> &elements, const QString &attr, const QString &attrVal )
{
  QList<QDomElement> list;
  const auto constElements = elements;
  for ( const QDomElement &el : constElements )
  {
    if ( el.attribute( attr ) == attrVal )
    {
      list << el;
    }
  }
  return list;
}

QDomElement QgsGmlSchema::domElement( const QDomElement &element, const QString &path, const QString &attr, const QString &attrVal )
{
  QList<QDomElement> list = domElements( element, path );
  return domElements( list, attr, attrVal ).value( 0 );
}

bool QgsGmlSchema::guessSchema( const QByteArray &data )
{
  mLevel = 0;
  mSkipLevel = std::numeric_limits<int>::max();
  XML_Parser p = XML_ParserCreateNS( nullptr, NS_SEPARATOR );
  XML_SetUserData( p, this );
  XML_SetElementHandler( p, QgsGmlSchema::start, QgsGmlSchema::end );
  XML_SetCharacterDataHandler( p, QgsGmlSchema::chars );
  const int atEnd = 1;
  const int res = XML_Parse( p, data.constData(), data.size(), atEnd );

  if ( res == 0 )
  {
    const QString err = QString( XML_ErrorString( XML_GetErrorCode( p ) ) );
    QgsDebugError( u"XML_Parse returned %1 error %2"_s.arg( res ).arg( err ) );
    mError = QgsError( err, u"GML schema"_s );
    mError.append( tr( "Cannot guess schema" ) );
  }

  return res != 0;
}

void QgsGmlSchema::startElement( const XML_Char *el, const XML_Char **attr )
{
  Q_UNUSED( attr )
  mLevel++;

  const QString elementName = QString::fromUtf8( el );
  QgsDebugMsgLevel( u"-> %1 %2 %3"_s.arg( mLevel ).arg( elementName, mLevel >= mSkipLevel ? "skip" : "" ), 5 );

  if ( mLevel >= mSkipLevel )
  {
    //QgsDebugMsgLevel( u"skip level %1"_s.arg( mLevel ), 2 );
    return;
  }

  mParsePathStack.append( elementName );
  const QString path = mParsePathStack.join( QLatin1Char( '.' ) );

  QStringList splitName = elementName.split( NS_SEPARATOR );
  const QString localName = splitName.last();
  const QString ns = splitName.size() > 1 ? splitName.first() : QString();
  //QgsDebugMsgLevel( "ns = " + ns + " localName = " + localName, 2 );

  const ParseMode parseMode = modeStackTop();
  //QgsDebugMsgLevel( QString("localName = %1 parseMode = %2").arg(localName).arg(parseMode), 2 );

  if ( ns == GML_NAMESPACE && localName == "boundedBy"_L1 )
  {
    // gml:boundedBy in feature or feature collection -> skip
    mSkipLevel = mLevel + 1;
  }
  else if ( localName.compare( "featureMembers"_L1, Qt::CaseInsensitive ) == 0 )
  {
    mParseModeStack.push( QgsGmlSchema::FeatureMembers );
  }
  // GML does not specify that gml:FeatureAssociationType elements should end
  // with 'Member' apart standard gml:featureMember, but it is quite usual to
  // that the names ends with 'Member', e.g.: osgb:topographicMember, cityMember,...
  // so this is really fail if the name does not contain 'Member'

  else if ( localName.endsWith( "member"_L1, Qt::CaseInsensitive ) )
  {
    mParseModeStack.push( QgsGmlSchema::FeatureMember );
  }
  // UMN Mapserver simple GetFeatureInfo response layer element (ends with _layer)
  else if ( elementName.endsWith( "_layer"_L1 ) )
  {
    // do nothing, we catch _feature children
  }
  // UMN Mapserver simple GetFeatureInfo response feature element (ends with _feature)
  // or featureMember children.
  // QGIS mapserver 2.2 GetFeatureInfo is using <Feature id="###"> for feature member,
  // without any feature class distinction.
  else if ( elementName.endsWith( "_feature"_L1 )
            || parseMode == QgsGmlSchema::FeatureMember
            || parseMode == QgsGmlSchema::FeatureMembers
            || localName.compare( "feature"_L1, Qt::CaseInsensitive ) == 0 )
  {
    QgsDebugMsgLevel( "is feature path = " + path, 2 );
    if ( mFeatureClassMap.count( localName ) == 0 )
    {
      mFeatureClassMap.insert( localName, QgsGmlFeatureClass( localName, path ) );
    }
    mCurrentFeatureName = localName;
    mParseModeStack.push( QgsGmlSchema::Feature );
  }
  else if ( parseMode == QgsGmlSchema::Attribute && ns == GML_NAMESPACE && mGeometryTypes.indexOf( localName ) >= 0 )
  {
    // Geometry (Point,MultiPoint,...) in geometry attribute
    QStringList &geometryAttributes = mFeatureClassMap[mCurrentFeatureName].geometryAttributes();
    if ( geometryAttributes.count( mAttributeName ) == 0 )
    {
      geometryAttributes.append( mAttributeName );
    }
    mSkipLevel = mLevel + 1; // no need to parse children
  }
  else if ( parseMode == QgsGmlSchema::Feature )
  {
    // An element in feature should be ordinary or geometry attribute
    //QgsDebugMsgLevel( "is attribute", 2);

    // Usually localName is attribute name, e.g.
    // <gml:desc>My description</gml:desc>
    // but QGIS server (2.2) is using:
    // <Attribute value="My description" name="desc"/>
    const QString name = readAttribute( u"name"_s, attr );
    //QgsDebugMsg ( "attribute name = " + name );
    if ( localName.compare( "attribute"_L1, Qt::CaseInsensitive ) == 0
         && !name.isEmpty() )
    {
      const QString value = readAttribute( u"value"_s, attr );
      //QgsDebugMsg ( "attribute value = " + value );
      addAttribute( name, value );
    }
    else
    {
      mAttributeName = localName;
      mParseModeStack.push( QgsGmlSchema::Attribute );
      mStringCash.clear();
    }
  }
}

void QgsGmlSchema::endElement( const XML_Char *el )
{
  const QString elementName = QString::fromUtf8( el );
  QgsDebugMsgLevel( u"<- %1 %2"_s.arg( mLevel ).arg( elementName ), 5 );

  if ( mLevel >= mSkipLevel )
  {
    //QgsDebugMsgLevel( u"skip level %1"_s.arg( mLevel ), 2 );
    mLevel--;
    return;
  }
  else
  {
    // clear possible skip level
    mSkipLevel = std::numeric_limits<int>::max();
  }

  QStringList splitName = elementName.split( NS_SEPARATOR );
  const QString localName = splitName.last();
  const QString ns = splitName.size() > 1 ? splitName.first() : QString();

  const QgsGmlSchema::ParseMode parseMode = modeStackTop();

  if ( parseMode == QgsGmlSchema::FeatureMembers )
  {
    modeStackPop();
  }
  else if ( parseMode == QgsGmlSchema::Attribute && localName == mAttributeName )
  {
    // End of attribute
    //QgsDebugMsgLevel("end attribute", 2);
    modeStackPop(); // go up to feature

    if ( mFeatureClassMap[mCurrentFeatureName].geometryAttributes().count( mAttributeName ) == 0 )
    {
      addAttribute( mAttributeName, mStringCash );
    }
  }
  else if ( ns == GML_NAMESPACE && localName == "boundedBy"_L1 )
  {
    // was skipped
  }
  else if ( localName.endsWith( "member"_L1, Qt::CaseInsensitive ) )
  {
    modeStackPop();
  }
  mParsePathStack.removeLast();
  mLevel--;
}

void QgsGmlSchema::characters( const XML_Char *chars, int len )
{
  //QgsDebugMsgLevel( u"level %1 : %2"_s.arg( mLevel ).arg( QString::fromUtf8( chars, len ) ), 2 );
  if ( mLevel >= mSkipLevel )
  {
    //QgsDebugMsgLevel( u"skip level %1"_s.arg( mLevel ),2 );
    return;
  }

  //save chars in mStringCash attribute mode for value type analysis
  if ( modeStackTop() == QgsGmlSchema::Attribute )
  {
    mStringCash.append( QString::fromUtf8( chars, len ) );
  }
}

void QgsGmlSchema::addAttribute( const QString &name, const QString &value )
{
  // It is not geometry attribute -> analyze value
  bool ok;
  ( void ) value.toInt( &ok );
  QMetaType::Type type = QMetaType::Type::QString;
  if ( ok )
  {
    type = QMetaType::Type::Int;
  }
  else
  {
    ( void ) value.toDouble( &ok );
    if ( ok )
    {
      type = QMetaType::Type::Double;
    }
  }
  //QgsDebugMsgLevel( "mStringCash = " + mStringCash + " type = " + QVariant::typeToName( type ),2 );
  //QMap<QString, QgsField> & fields = mFeatureClassMap[mCurrentFeatureName].fields();
  QList<QgsField> &fields = mFeatureClassMap[mCurrentFeatureName].fields();
  const int fieldIndex = mFeatureClassMap[mCurrentFeatureName].fieldIndex( name );
  if ( fieldIndex == -1 )
  {
    const QgsField field( name, type );
    fields.append( field );
  }
  else
  {
    QgsField &field = fields[fieldIndex];
    // check if type is sufficient
    if ( ( field.type() == QMetaType::Type::Int && ( type == QMetaType::Type::QString || type == QMetaType::Type::Double ) ) ||
         ( field.type() == QMetaType::Type::Double && type == QMetaType::Type::QString ) )
    {
      field.setType( type );
    }
  }
}

QStringList QgsGmlSchema::typeNames() const
{
  return mFeatureClassMap.keys();
}

QList<QgsField> QgsGmlSchema::fields( const QString &typeName )
{
  if ( mFeatureClassMap.count( typeName ) == 0 ) return QList<QgsField>();
  return mFeatureClassMap[typeName].fields();
}

QStringList QgsGmlSchema::geometryAttributes( const QString &typeName )
{
  if ( mFeatureClassMap.count( typeName ) == 0 ) return QStringList();
  return mFeatureClassMap[typeName].geometryAttributes();
}
