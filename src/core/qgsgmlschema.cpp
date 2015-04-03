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
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgserror.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsnetworkaccessmanager.h"
#include <QBuffer>
#include <QList>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QProgressDialog>
#include <QSet>
#include <QSettings>
#include <QUrl>

#include <limits>

const char NS_SEPARATOR = '?';
const QString GML_NAMESPACE = "http://www.opengis.net/gml";

QgsGmlFeatureClass::QgsGmlFeatureClass()
{
}

QgsGmlFeatureClass::QgsGmlFeatureClass( QString name, QString path )
    : mName( name )
    , mPath( path )
{
}

QgsGmlFeatureClass::~QgsGmlFeatureClass()
{
}

int QgsGmlFeatureClass::fieldIndex( const QString & name )
{
  for ( int i = 0; i < mFields.size(); i++ )
  {
    if ( mFields[i].name() == name ) return i;
  }
  return -1;
}

// --------------------------- QgsGmlSchema -------------------------------
QgsGmlSchema::QgsGmlSchema()
    : QObject()
    , mCurrentFeature( NULL )
    , mFeatureCount( 0 )
    , mLevel( 0 )
    , mSkipLevel( std::numeric_limits<int>::max() )
{
  mGeometryTypes << "Point" << "MultiPoint"
  << "LineString" << "MultiLineString"
  << "Polygon" << "MultiPolygon";
}

QgsGmlSchema::~QgsGmlSchema()
{

}

QString QgsGmlSchema::readAttribute( const QString& attributeName, const XML_Char** attr ) const
{
  int i = 0;
  while ( attr[i] != NULL )
  {
    if ( attributeName.compare( attr[i] ) == 0 )
    {
      return QString( attr[i+1] );
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

  QDomElement docElem = dom.documentElement();

  QList<QDomElement> elementElements = domElements( docElem, "element" );

  //QgsDebugMsg( QString( "%1 elemets read" ).arg( elementElements.size() ) );

  foreach ( QDomElement elementElement, elementElements )
  {
    QString name = elementElement.attribute( "name" );
    QString type = elementElement.attribute( "type" );

    QString gmlBaseType = xsdComplexTypeGmlBaseType( docElem, stripNS( type ) );
    //QgsDebugMsg( QString( "gmlBaseType = %1" ).arg( gmlBaseType ) );
    //QgsDebugMsg( QString( "name = %1 gmlBaseType = %2" ).arg( name ).arg( gmlBaseType ) );
    // We should only use gml:AbstractFeatureType descendants which have
    // ancestor listed in gml:FeatureAssociationType (featureMember) descendant
    // But we could only loose some data if XSD was not correct, I think.

    if ( gmlBaseType == "AbstractFeatureType" )
    {
      // Get feature type definition
      QgsGmlFeatureClass featureClass( name, "" );
      xsdFeatureClass( docElem, stripNS( type ), featureClass );
      mFeatureClassMap.insert( name, featureClass );
    }
    // A feature may have more geometries, we take just the first one
  }

  return true;
}

bool QgsGmlSchema::xsdFeatureClass( const QDomElement &element, const QString & typeName, QgsGmlFeatureClass & featureClass )
{
  //QgsDebugMsg("typeName = " + typeName );
  QDomElement complexTypeElement = domElement( element, "complexType", "name", typeName );
  if ( complexTypeElement.isNull() ) return false;

  // extension or restriction
  QDomElement extrest = domElement( complexTypeElement, "complexContent.extension" );
  if ( extrest.isNull() )
  {
    extrest = domElement( complexTypeElement, "complexContent.restriction" );
  }
  if ( extrest.isNull() ) return false;

  QString extrestName = extrest.attribute( "base" );
  if ( extrestName == "gml:AbstractFeatureType" )
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
  foreach ( QString geom, mGeometryTypes )
  {
    geometryPropertyTypes << geom + "PropertyType";
  }

  QStringList geometryAliases;
  geometryAliases << "location" << "centerOf" << "position" << "extentOf"
  << "coverage" << "edgeOf" << "centerLineOf" << "multiLocation"
  << "multiCenterOf" << "multiPosition" << "multiCenterLineOf"
  << "multiEdgeOf" << "multiCoverage" << "multiExtentOf";

  // Add attributes from current comple type
  QList<QDomElement> sequenceElements = domElements( extrest, "sequence.element" );
  foreach ( QDomElement sequenceElement, sequenceElements )
  {
    QString fieldName = sequenceElement.attribute( "name" );
    QString fieldTypeName = stripNS( sequenceElement.attribute( "type" ) );
    QString ref = sequenceElement.attribute( "ref" );
    //QgsDebugMsg ( QString("fieldName = %1 fieldTypeName = %2 ref = %3").arg(fieldName).arg(fieldTypeName).arg(ref) );

    if ( !ref.isEmpty() )
    {
      if ( ref.startsWith( "gml:" ) )
      {
        if ( geometryAliases.contains( stripNS( ref ) ) )
        {
          featureClass.geometryAttributes().append( stripNS( ref ) );
        }
        else
        {
          QgsDebugMsg( QString( "Unknown referenced GML element: %1" ).arg( ref ) );
        }
      }
      else
      {
        // TODO: get type from referenced element
        QgsDebugMsg( QString( "field %1.%2 is referencing %3 - not supported" ).arg( typeName ).arg( fieldName ) );
      }
      continue;
    }

    if ( fieldName.isEmpty() )
    {
      QgsDebugMsg( QString( "field in %1 without name" ).arg( typeName ) );
      continue;
    }

    // type is either type attribute
    if ( fieldTypeName.isEmpty() )
    {
      // or type is inheriting from xs:simpleType
      QDomElement sequenceElementRestriction = domElement( sequenceElement, "simpleType.restriction" );
      fieldTypeName = stripNS( sequenceElementRestriction.attribute( "base" ) );
    }

    QVariant::Type fieldType = QVariant::String;
    if ( fieldTypeName.isEmpty() )
    {
      QgsDebugMsg( QString( "Cannot get %1.%2 field type" ).arg( typeName ).arg( fieldName ) );
    }
    else
    {
      if ( geometryPropertyTypes.contains( fieldTypeName ) )
      {
        // Geometry attribute
        featureClass.geometryAttributes().append( fieldName );
        continue;
      }

      if ( fieldTypeName == "decimal" )
      {
        fieldType = QVariant::Double;
      }
      else if ( fieldTypeName == "integer" )
      {
        fieldType = QVariant::Int;
      }
    }

    QgsField field( fieldName, fieldType, fieldTypeName );
    featureClass.fields().append( field );
  }

  return true;
}

QString QgsGmlSchema::xsdComplexTypeGmlBaseType( const QDomElement &element, const QString & name )
{
  //QgsDebugMsg("name = " + name );
  QDomElement complexTypeElement = domElement( element, "complexType", "name", name );
  if ( complexTypeElement.isNull() ) return "";

  QDomElement extrest = domElement( complexTypeElement, "complexContent.extension" );
  if ( extrest.isNull() )
  {
    extrest = domElement( complexTypeElement, "complexContent.restriction" );
  }
  if ( extrest.isNull() ) return "";

  QString extrestName = extrest.attribute( "base" );
  if ( extrestName.startsWith( "gml:" ) )
  {
    // GML base type found
    return stripNS( extrestName );
  }
  // Continue recursively until GML base type is reached
  return xsdComplexTypeGmlBaseType( element, stripNS( extrestName ) );
}

QString QgsGmlSchema::stripNS( const QString & name )
{
  return name.contains( ":" ) ? name.section( ':', 1 ) : name;
}

QList<QDomElement> QgsGmlSchema::domElements( const QDomElement &element, const QString & path )
{
  QList<QDomElement> list;

  QStringList names = path.split( "." );
  if ( names.size() == 0 ) return list;
  QString name = names.value( 0 );
  names.removeFirst();

  QDomNode n1 = element.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement el = n1.toElement();
    if ( !el.isNull() )
    {
      QString tagName = stripNS( el.tagName() );
      if ( tagName == name )
      {
        if ( names.size() == 0 )
        {
          list.append( el );
        }
        else
        {
          list.append( domElements( el, names.join( "." ) ) );
        }
      }
    }
    n1 = n1.nextSibling();
  }

  return list;
}

QDomElement QgsGmlSchema::domElement( const QDomElement &element, const QString & path )
{
  return domElements( element, path ).value( 0 );
}

QList<QDomElement> QgsGmlSchema::domElements( QList<QDomElement> &elements, const QString & attr, const QString & attrVal )
{
  QList<QDomElement> list;
  foreach ( QDomElement el, elements )
  {
    if ( el.attribute( attr ) == attrVal )
    {
      list << el;
    }
  }
  return list;
}

QDomElement QgsGmlSchema::domElement( const QDomElement &element, const QString & path, const QString & attr, const QString & attrVal )
{
  QList<QDomElement> list = domElements( element, path );
  return domElements( list, attr, attrVal ).value( 0 );
}

bool QgsGmlSchema::guessSchema( const QByteArray &data )
{
  QgsDebugMsg( "Entered" );
  mLevel = 0;
  mSkipLevel = std::numeric_limits<int>::max();
  XML_Parser p = XML_ParserCreateNS( NULL, NS_SEPARATOR );
  XML_SetUserData( p, this );
  XML_SetElementHandler( p, QgsGmlSchema::start, QgsGmlSchema::end );
  XML_SetCharacterDataHandler( p, QgsGmlSchema::chars );
  int atEnd = 1;
  int res = XML_Parse( p, data.constData(), data.size(), atEnd );

  if ( res == 0 )
  {
    QString err = QString( XML_ErrorString( XML_GetErrorCode( p ) ) );
    QgsDebugMsg( QString( "XML_Parse returned %1 error %2" ).arg( res ).arg( err ) );
    mError = QgsError( err, "GML schema" );
    mError.append( tr( "Cannot guess schema" ) );
  }

  return res != 0;
}

void QgsGmlSchema::startElement( const XML_Char* el, const XML_Char** attr )
{
  Q_UNUSED( attr );
  mLevel++;

  QString elementName = QString::fromUtf8( el );
  QgsDebugMsgLevel( QString( "-> %1 %2 %3" ).arg( mLevel ).arg( elementName ).arg( mLevel >= mSkipLevel ? "skip" : "" ), 5 );

  if ( mLevel >= mSkipLevel )
  {
    //QgsDebugMsg( QString("skip level %1").arg( mLevel ) );
    return;
  }

  mParsePathStack.append( elementName );
  QString path = mParsePathStack.join( "." );

  QStringList splitName =  elementName.split( NS_SEPARATOR );
  QString localName = splitName.last();
  QString ns = splitName.size() > 1 ? splitName.first() : "";
  //QgsDebugMsg( "ns = " + ns + " localName = " + localName );

  ParseMode parseMode = modeStackTop();
  //QgsDebugMsg ( QString("localName = %1 parseMode = %2").arg(localName).arg(parseMode) );

  if ( ns == GML_NAMESPACE && localName == "boundedBy" )
  {
    // gml:boundedBy in feature or feature collection -> skip
    mSkipLevel = mLevel + 1;
  }
  else if ( localName.compare( "featureMembers", Qt::CaseInsensitive ) == 0 )
  {
    mParseModeStack.push( QgsGmlSchema::featureMembers );
  }
  // GML does not specify that gml:FeatureAssociationType elements should end
  // with 'Member' apart standard gml:featureMember, but it is quite usual to
  // that the names ends with 'Member', e.g.: osgb:topographicMember, cityMember,...
  // so this is really fail if the name does not contain 'Member'

  else if ( localName.endsWith( "member", Qt::CaseInsensitive ) )
  {
    mParseModeStack.push( QgsGmlSchema::featureMember );
  }
  // UMN Mapserver simple GetFeatureInfo response layer element (ends with _layer)
  else if ( elementName.endsWith( "_layer" ) )
  {
    // do nothing, we catch _feature children
  }
  // UMN Mapserver simple GetFeatureInfo response feature element (ends with _feature)
  // or featureMember children.
  // QGIS mapserver 2.2 GetFeatureInfo is using <Feature id="###"> for feature member,
  // without any feature class distinction.
  else if ( elementName.endsWith( "_feature" )
            || parseMode == QgsGmlSchema::featureMember
            || parseMode == QgsGmlSchema::featureMembers
            || localName.compare( "feature", Qt::CaseInsensitive ) == 0 )
  {
    QgsDebugMsg( "is feature path = " + path );
    if ( mFeatureClassMap.count( localName ) == 0 )
    {
      mFeatureClassMap.insert( localName, QgsGmlFeatureClass( localName, path ) );
    }
    mCurrentFeatureName = localName;
    mParseModeStack.push( QgsGmlSchema::feature );
  }
  else if ( parseMode == QgsGmlSchema::attribute && ns == GML_NAMESPACE && mGeometryTypes.indexOf( localName ) >= 0 )
  {
    // Geometry (Point,MultiPoint,...) in geometry attribute
    QStringList &geometryAttributes = mFeatureClassMap[mCurrentFeatureName].geometryAttributes();
    if ( geometryAttributes.count( mAttributeName ) == 0 )
    {
      geometryAttributes.append( mAttributeName );
    }
    mSkipLevel = mLevel + 1; // no need to parse children
  }
  else if ( parseMode == QgsGmlSchema::feature )
  {
    // An element in feature should be ordinary or geometry attribute
    //QgsDebugMsg( "is attribute");

    // Usually localName is attribute name, e.g.
    // <gml:desc>My description</gml:desc>
    // but QGIS server (2.2) is using:
    // <Attribute value="My description" name="desc"/>
    QString name = readAttribute( "name", attr );
    //QgsDebugMsg ( "attribute name = " + name );
    if ( localName.compare( "attribute", Qt::CaseInsensitive ) == 0
         && !name.isEmpty() )
    {
      QString value = readAttribute( "value", attr );
      //QgsDebugMsg ( "attribute value = " + value );
      addAttribute( name, value );
    }
    else
    {
      mAttributeName = localName;
      mParseModeStack.push( QgsGmlSchema::attribute );
      mStringCash.clear();
    }
  }
}

void QgsGmlSchema::endElement( const XML_Char* el )
{
  QString elementName = QString::fromUtf8( el );
  QgsDebugMsgLevel( QString( "<- %1 %2" ).arg( mLevel ).arg( elementName ), 5 );

  if ( mLevel >= mSkipLevel )
  {
    //QgsDebugMsg( QString("skip level %1").arg( mLevel ) );
    mLevel--;
    return;
  }
  else
  {
    // clear possible skip level
    mSkipLevel = std::numeric_limits<int>::max();
  }

  QStringList splitName =  elementName.split( NS_SEPARATOR );
  QString localName = splitName.last();
  QString ns = splitName.size() > 1 ? splitName.first() : "";

  QgsGmlSchema::ParseMode parseMode = modeStackTop();

  if ( parseMode == QgsGmlSchema::featureMembers )
  {
    modeStackPop();
  }
  else if ( parseMode == QgsGmlSchema::attribute && localName == mAttributeName )
  {
    // End of attribute
    //QgsDebugMsg("end attribute");
    modeStackPop(); // go up to feature

    if ( mFeatureClassMap[mCurrentFeatureName].geometryAttributes().count( mAttributeName ) == 0 )
    {
      addAttribute( mAttributeName, mStringCash );
    }
  }
  else if ( ns == GML_NAMESPACE && localName == "boundedBy" )
  {
    // was skipped
  }
  else if ( localName.endsWith( "member", Qt::CaseInsensitive ) )
  {
    modeStackPop();
  }
  mParsePathStack.removeLast();
  mLevel--;
}

void QgsGmlSchema::characters( const XML_Char* chars, int len )
{
  //QgsDebugMsg( QString("level %1 : %2").arg( mLevel ).arg( QString::fromUtf8( chars, len ) ) );
  if ( mLevel >= mSkipLevel )
  {
    //QgsDebugMsg( QString("skip level %1").arg( mLevel ) );
    return;
  }

  //save chars in mStringCash attribute mode for value type analysis
  if ( modeStackTop() == QgsGmlSchema::attribute )
  {
    mStringCash.append( QString::fromUtf8( chars, len ) );
  }
}

void QgsGmlSchema::addAttribute( const QString& name, const QString& value )
{
  // It is not geometry attribute -> analyze value
  bool ok;
  value.toInt( &ok );
  QVariant::Type type = QVariant::String;
  if ( ok )
  {
    type = QVariant::Int;
  }
  else
  {
    value.toDouble( &ok );
    if ( ok )
    {
      type = QVariant::Double;
    }
  }
  //QgsDebugMsg( "mStringCash = " + mStringCash + " type = " + QVariant::typeToName( type )  );
  //QMap<QString, QgsField> & fields = mFeatureClassMap[mCurrentFeatureName].fields();
  QList<QgsField> & fields = mFeatureClassMap[mCurrentFeatureName].fields();
  int fieldIndex = mFeatureClassMap[mCurrentFeatureName].fieldIndex( name );
  if ( fieldIndex == -1 )
  {
    QgsField field( name, type );
    fields.append( field );
  }
  else
  {
    QgsField &field = fields[fieldIndex];
    // check if type is sufficient
    if (( field.type() == QVariant::Int && ( type == QVariant::String || type == QVariant::Double ) ) ||
        ( field.type() == QVariant::Double && type == QVariant::String ) )
    {
      field.setType( type );
    }
  }
}

QStringList QgsGmlSchema::typeNames() const
{
  return mFeatureClassMap.keys();
}

QList<QgsField> QgsGmlSchema::fields( const QString & typeName )
{
  if ( mFeatureClassMap.count( typeName ) == 0 ) return QList<QgsField>();
  return mFeatureClassMap[typeName].fields();
}

QStringList QgsGmlSchema::geometryAttributes( const QString & typeName )
{
  if ( mFeatureClassMap.count( typeName ) == 0 ) return QStringList();
  return mFeatureClassMap[typeName].geometryAttributes();
}
