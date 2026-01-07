/***************************************************************************
    qgseditformconfig.cpp
    ---------------------
    begin                : November 2015
    copyright            : (C) 2015 by Matthias Kuhn
    email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgseditformconfig.h"

#include "qgsapplication.h"
#include "qgsattributeeditorcontainer.h"
#include "qgsattributeeditorfield.h"
#include "qgsattributeeditorrelation.h"
#include "qgseditformconfig_p.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsnetworkcontentfetcherregistry.h"
#include "qgspathresolver.h"
#include "qgsproject.h"
#include "qgsreadwritecontext.h"
#include "qgsrelationmanager.h"
#include "qgsxmlutils.h"

#include <QUrl>

#include "moc_qgseditformconfig.cpp"

QgsEditFormConfig::QgsEditFormConfig()
  : d( new QgsEditFormConfigPrivate() )
{
}

void QgsEditFormConfig::setDataDefinedFieldProperties( const QString &fieldName, const QgsPropertyCollection &properties )
{
  d.detach();
  d->mDataDefinedFieldProperties[ fieldName ] = properties;
}

QgsPropertyCollection QgsEditFormConfig::dataDefinedFieldProperties( const QString &fieldName ) const
{
  return d->mDataDefinedFieldProperties.value( fieldName );
}

const QgsPropertiesDefinition &QgsEditFormConfig::propertyDefinitions()
{
  return QgsEditFormConfigPrivate::propertyDefinitions();
}

QVariantMap QgsEditFormConfig::widgetConfig( const QString &widgetName ) const
{
  const int fieldIndex = d->mFields.indexOf( widgetName );
  if ( fieldIndex != -1 )
    return d->mFields.at( fieldIndex ).editorWidgetSetup().config();
  else
    return d->mWidgetConfigs.value( widgetName );
}

void QgsEditFormConfig::setFields( const QgsFields &fields )
{
  d.detach();
  d->mFields = fields;

  if ( !d->mConfiguredRootContainer )
  {
    d->mInvisibleRootContainer->clear();
    for ( int i = 0; i < d->mFields.size(); ++i )
    {
      QgsAttributeEditorField *field = new QgsAttributeEditorField( d->mFields.at( i ).name(), i, d->mInvisibleRootContainer );
      d->mInvisibleRootContainer->addChildElement( field );
    }
  }
}

void QgsEditFormConfig::onRelationsLoaded()
{
  const QList<QgsAttributeEditorElement *> relations = d->mInvisibleRootContainer->findElements( Qgis::AttributeEditorType::Relation );

  for ( QgsAttributeEditorElement *relElem : relations )
  {
    QgsAttributeEditorRelation *rel = dynamic_cast< QgsAttributeEditorRelation * >( relElem );
    if ( !rel )
      continue;

    rel->init( QgsProject::instance()->relationManager() ); // skip-keyword-check
  }
}

bool QgsEditFormConfig::legacyUpdateRelationWidgetInTabs( QgsAttributeEditorContainer *container,  const QString &widgetName, const QVariantMap &config )
{
  const QList<QgsAttributeEditorElement *> children = container->children();
  for ( QgsAttributeEditorElement *child : children )
  {
    if ( child->type() ==  Qgis::AttributeEditorType::Container )
    {
      QgsAttributeEditorContainer *container = dynamic_cast<QgsAttributeEditorContainer *>( child );
      if ( legacyUpdateRelationWidgetInTabs( container, widgetName, config ) )
      {
        //return when a relation has been set in a child or child child...
        return true;
      }
    }
    else if ( child->type() ==  Qgis::AttributeEditorType::Relation )
    {
      QgsAttributeEditorRelation *relation = dynamic_cast< QgsAttributeEditorRelation * >( child );
      if ( relation )
      {
        if ( relation->relation().id() == widgetName )
        {
          if ( config.contains( u"nm-rel"_s ) )
          {
            relation->setNmRelationId( config[u"nm-rel"_s] );
          }
          if ( config.contains( u"force-suppress-popup"_s ) )
          {
            relation->setForceSuppressFormPopup( config[u"force-suppress-popup"_s].toBool() );
          }
          return true;
        }
      }
    }
  }
  return false;
}

bool QgsEditFormConfig::setWidgetConfig( const QString &widgetName, const QVariantMap &config )
{
  if ( d->mFields.indexOf( widgetName ) != -1 )
  {
    QgsDebugError( u"Trying to set a widget config for a field on QgsEditFormConfig. Use layer->setEditorWidgetSetup() instead."_s );
    return false;
  }

  //for legacy use it writes the relation editor configuration into the first instance of the widget
  if ( config.contains( u"force-suppress-popup"_s ) || config.contains( u"nm-rel"_s ) )
  {
    QgsMessageLog::logMessage( u"Deprecation Warning: Trying to set a relation config directly on the relation %1. Relation settings should be done for the specific widget instance instead. Use attributeEditorRelation->setNmRelationId() or attributeEditorRelation->setForceSuppressFormPopup() instead."_s.arg( widgetName ) );
    legacyUpdateRelationWidgetInTabs( d->mInvisibleRootContainer, widgetName, config );
  }

  d.detach();
  d->mWidgetConfigs[widgetName] = config;
  return true;
}

bool QgsEditFormConfig::removeWidgetConfig( const QString &widgetName )
{
  d.detach();
  return d->mWidgetConfigs.remove( widgetName ) != 0;
}

QgsEditFormConfig::QgsEditFormConfig( const QgsEditFormConfig &o ) //NOLINT
  : d( o.d )
{
}

QgsEditFormConfig::~QgsEditFormConfig() //NOLINT
{}

QgsEditFormConfig &QgsEditFormConfig::operator=( const QgsEditFormConfig &o )  //NOLINT
{
  if ( &o == this )
    return *this;

  d = o.d;
  return *this;
}

bool QgsEditFormConfig::operator==( const QgsEditFormConfig &o ) const
{
  return d == o.d;
}

void QgsEditFormConfig::addTab( QgsAttributeEditorElement *data )
{
  d.detach();
  d->mInvisibleRootContainer->addChildElement( data );
}

QList<QgsAttributeEditorElement *> QgsEditFormConfig::tabs() const
{
  return d->mInvisibleRootContainer->children();
}

void QgsEditFormConfig::clearTabs()
{
  d.detach();
  d->mInvisibleRootContainer->clear();
}

QgsAttributeEditorContainer *QgsEditFormConfig::invisibleRootContainer()
{
  return d->mInvisibleRootContainer;
}

Qgis::AttributeFormLayout QgsEditFormConfig::layout() const
{
  return d->mEditorLayout;
}

void QgsEditFormConfig::setLayout( Qgis::AttributeFormLayout editorLayout )
{
  d.detach();
  d->mEditorLayout = editorLayout;

  if ( editorLayout == Qgis::AttributeFormLayout::DragAndDrop )
    d->mConfiguredRootContainer = true;
}

QString QgsEditFormConfig::uiForm() const
{
  return d->mUiFormPath;
}

void QgsEditFormConfig::setUiForm( const QString &ui )
{
  if ( !ui.isEmpty() && !QUrl::fromUserInput( ui ).isLocalFile() )
  {
    // any existing download will not be restarted!
    QgsApplication::networkContentFetcherRegistry()->fetch( ui, Qgis::ActionStart::Immediate );
  }

  if ( ui.isEmpty() )
  {
    setLayout( Qgis::AttributeFormLayout::AutoGenerated );
  }
  else
  {
    setLayout( Qgis::AttributeFormLayout::UiFile );
  }
  d->mUiFormPath = ui;
}

bool QgsEditFormConfig::readOnly( int idx ) const
{
  if ( idx >= 0 && idx < d->mFields.count() )
  {
    if ( d->mFields.fieldOrigin( idx ) == Qgis::FieldOrigin::Join
         || d->mFields.fieldOrigin( idx ) == Qgis::FieldOrigin::Expression )
      return true;
    if ( d->mFields.at( idx ).isReadOnly() )
      return true;
    return !d->mFieldEditables.value( d->mFields.at( idx ).name(), true );
  }
  else
    return false;
}

bool QgsEditFormConfig::labelOnTop( int idx ) const
{
  if ( idx >= 0 && idx < d->mFields.count() )
    return d->mLabelOnTop.value( d->mFields.at( idx ).name(), false );
  else
    return false;
}

void QgsEditFormConfig::setReadOnly( int idx, bool readOnly )
{
  if ( idx >= 0 && idx < d->mFields.count() )
  {
    d.detach();
    d->mFieldEditables[ d->mFields.at( idx ).name()] = !readOnly;
  }
}

void QgsEditFormConfig::setLabelOnTop( int idx, bool onTop )
{
  if ( idx >= 0 && idx < d->mFields.count() )
  {
    d.detach();
    d->mLabelOnTop[ d->mFields.at( idx ).name()] = onTop;
  }
}

bool QgsEditFormConfig::reuseLastValue( int index ) const
{
  if ( index >= 0 && index < d->mFields.count() )
    return d->mReuseLastValuePolicy.value( d->mFields.at( index ).name(), Qgis::AttributeFormReuseLastValuePolicy::NotAllowed ) != Qgis::AttributeFormReuseLastValuePolicy::NotAllowed;
  else
    return false;
}

void QgsEditFormConfig::setReuseLastValue( int index, bool reuse )
{
  if ( index >= 0 && index < d->mFields.count() )
  {
    d.detach();
    d->mReuseLastValuePolicy[ d->mFields.at( index ).name()] = reuse ? Qgis::AttributeFormReuseLastValuePolicy::AllowedDefaultOn : Qgis::AttributeFormReuseLastValuePolicy::NotAllowed;
  }
}

Qgis::AttributeFormReuseLastValuePolicy QgsEditFormConfig::reuseLastValuePolicy( int index ) const
{
  if ( index >= 0 && index < d->mFields.count() )
    return d->mReuseLastValuePolicy.value( d->mFields.at( index ).name(), Qgis::AttributeFormReuseLastValuePolicy::NotAllowed );
  else
    return Qgis::AttributeFormReuseLastValuePolicy::NotAllowed;
}

void QgsEditFormConfig::setReuseLastValuePolicy( int index, Qgis::AttributeFormReuseLastValuePolicy policy )
{
  if ( index >= 0 && index < d->mFields.count() )
  {
    d.detach();
    d->mReuseLastValuePolicy[ d->mFields.at( index ).name()] = policy;
  }
}

QString QgsEditFormConfig::initFunction() const
{
  return d->mInitFunction;
}

void QgsEditFormConfig::setInitFunction( const QString &function )
{
  d.detach();
  d->mInitFunction = function;
}

QString QgsEditFormConfig::initCode() const
{
  return d->mInitCode;
}

void QgsEditFormConfig::setInitCode( const QString &code )
{
  d.detach();
  d->mInitCode = code;
}

QString QgsEditFormConfig::initFilePath() const
{
  return d->mInitFilePath;
}

void QgsEditFormConfig::setInitFilePath( const QString &filePath )
{
  d.detach();
  d->mInitFilePath = filePath;

  // if this is an URL, download file as there is a good chance it will be used later
  if ( !filePath.isEmpty() && !QUrl::fromUserInput( filePath ).isLocalFile() )
  {
    // any existing download will not be restarted!
    QgsApplication::networkContentFetcherRegistry()->fetch( filePath, Qgis::ActionStart::Immediate );
  }
}

Qgis::AttributeFormPythonInitCodeSource QgsEditFormConfig::initCodeSource() const
{
  return d->mInitCodeSource;
}

void QgsEditFormConfig::setInitCodeSource( const Qgis::AttributeFormPythonInitCodeSource initCodeSource )
{
  d.detach();
  d->mInitCodeSource = initCodeSource;
}

Qgis::AttributeFormSuppression QgsEditFormConfig::suppress() const
{
  return d->mSuppressForm;
}

void QgsEditFormConfig::setSuppress( Qgis::AttributeFormSuppression s )
{
  d.detach();
  d->mSuppressForm = s;
}

void QgsEditFormConfig::readXml( const QDomNode &node, QgsReadWriteContext &context )
{
  const QgsReadWriteContextCategoryPopper p = context.enterCategory( QObject::tr( "Edit form config" ) );

  d.detach();

  const QDomNode editFormNode = node.namedItem( u"editform"_s );
  if ( !editFormNode.isNull() )
  {
    const QDomElement e = editFormNode.toElement();
    const bool tolerantRemoteUrls = e.hasAttribute( u"tolerant"_s );
    if ( !e.text().isEmpty() )
    {
      const QString uiFormPath = context.pathResolver().readPath( e.text() );
      // <= 3.2 had a bug where invalid ui paths would get written into projects on load
      // to avoid restoring these invalid paths, we take a less-tolerant approach for older (untrustworthy) projects
      // and only set ui forms paths IF they are local files OR start with "http(s)".
      const bool localFile = QFileInfo::exists( uiFormPath );
      if ( localFile || tolerantRemoteUrls || uiFormPath.startsWith( "http"_L1 ) )
        setUiForm( uiFormPath );
    }
  }

  const QDomNode editFormInitNode = node.namedItem( u"editforminit"_s );
  if ( !editFormInitNode.isNull() )
  {
    d->mInitFunction = editFormInitNode.toElement().text();
  }

  const QDomNode editFormInitCodeSourceNode = node.namedItem( u"editforminitcodesource"_s );
  if ( !editFormInitCodeSourceNode.isNull() && !editFormInitCodeSourceNode.toElement().text().isEmpty() )
  {
    setInitCodeSource( static_cast< Qgis::AttributeFormPythonInitCodeSource >( editFormInitCodeSourceNode.toElement().text().toInt() ) );
  }

  const QDomNode editFormInitCodeNode = node.namedItem( u"editforminitcode"_s );
  if ( !editFormInitCodeNode.isNull() )
  {
    setInitCode( editFormInitCodeNode.toElement().text() );
  }

  // Temporary < 2.12 b/w compatibility "dot" support patch
  // \see: https://github.com/qgis/QGIS/pull/2498
  // For b/w compatibility, check if there's a dot in the function name
  // and if yes, transform it in an import statement for the module
  // and set the PythonInitCodeSource to CodeSourceDialog
  const int dotPos = d->mInitFunction.lastIndexOf( '.' );
  if ( dotPos >= 0 ) // It's a module
  {
    setInitCodeSource( Qgis::AttributeFormPythonInitCodeSource::Dialog );
    setInitCode( u"from %1 import %2\n"_s.arg( d->mInitFunction.left( dotPos ), d->mInitFunction.mid( dotPos + 1 ) ) );
    setInitFunction( d->mInitFunction.mid( dotPos + 1 ) );
  }

  const QDomNode editFormInitFilePathNode = node.namedItem( u"editforminitfilepath"_s );
  if ( !editFormInitFilePathNode.isNull() && !editFormInitFilePathNode.toElement().text().isEmpty() )
  {
    setInitFilePath( context.pathResolver().readPath( editFormInitFilePathNode.toElement().text() ) );
  }

  const QDomNode fFSuppNode = node.namedItem( u"featformsuppress"_s );
  if ( fFSuppNode.isNull() )
  {
    d->mSuppressForm = Qgis::AttributeFormSuppression::Default;
  }
  else
  {
    const QDomElement e = fFSuppNode.toElement();
    d->mSuppressForm = static_cast< Qgis::AttributeFormSuppression >( e.text().toInt() );
  }

  // tab display
  const QDomNode editorLayoutNode = node.namedItem( u"editorlayout"_s );
  if ( editorLayoutNode.isNull() )
  {
    d->mEditorLayout = Qgis::AttributeFormLayout::AutoGenerated;
  }
  else
  {
    if ( editorLayoutNode.toElement().text() == "uifilelayout"_L1 )
    {
      d->mEditorLayout = Qgis::AttributeFormLayout::UiFile;
    }
    else if ( editorLayoutNode.toElement().text() == "tablayout"_L1 )
    {
      d->mEditorLayout = Qgis::AttributeFormLayout::DragAndDrop;
    }
    else
    {
      d->mEditorLayout = Qgis::AttributeFormLayout::AutoGenerated;
    }
  }

  d->mFieldEditables.clear();
  const QDomNodeList editableNodeList = node.namedItem( u"editable"_s ).toElement().childNodes();
  for ( int i = 0; i < editableNodeList.size(); ++i )
  {
    const QDomElement editableElement = editableNodeList.at( i ).toElement();
    d->mFieldEditables.insert( editableElement.attribute( u"name"_s ), static_cast< bool >( editableElement.attribute( u"editable"_s ).toInt() ) );
  }

  d->mLabelOnTop.clear();
  const QDomNodeList labelOnTopNodeList = node.namedItem( u"labelOnTop"_s ).toElement().childNodes();
  for ( int i = 0; i < labelOnTopNodeList.size(); ++i )
  {
    const QDomElement labelOnTopElement = labelOnTopNodeList.at( i ).toElement();
    d->mLabelOnTop.insert( labelOnTopElement.attribute( u"name"_s ), static_cast< bool >( labelOnTopElement.attribute( u"labelOnTop"_s ).toInt() ) );
  }

  d->mReuseLastValuePolicy.clear();
  // Compatibility with QGIS projects saved prior to 4.0
  const QDomNodeList reuseLastValueNodeList = node.namedItem( u"reuseLastValue"_s ).toElement().childNodes();
  for ( int i = 0; i < reuseLastValueNodeList.size(); ++i )
  {
    const QDomElement reuseLastValueElement = reuseLastValueNodeList.at( i ).toElement();
    d->mReuseLastValuePolicy.insert( reuseLastValueElement.attribute( u"name"_s ), static_cast< bool >( reuseLastValueElement.attribute( u"reuseLastValue"_s ).toInt() ) ? Qgis::AttributeFormReuseLastValuePolicy::AllowedDefaultOn : Qgis::AttributeFormReuseLastValuePolicy::NotAllowed );
  }
  const QDomNodeList reuseLastValuePolicyNodeList = node.namedItem( u"reuseLastValuePolicy"_s ).toElement().childNodes();
  for ( int i = 0; i < reuseLastValuePolicyNodeList.size(); ++i )
  {
    const QDomElement reuseLastValuePolicyElement = reuseLastValuePolicyNodeList.at( i ).toElement();
    d->mReuseLastValuePolicy.insert( reuseLastValuePolicyElement.attribute( u"name"_s ), qgsEnumKeyToValue( reuseLastValuePolicyElement.attribute( u"reuseLastValuePolicy"_s ), Qgis::AttributeFormReuseLastValuePolicy::NotAllowed ) );
  }

  // Read data defined field properties
  const QDomNodeList fieldDDPropertiesNodeList = node.namedItem( u"dataDefinedFieldProperties"_s ).toElement().childNodes();
  for ( int i = 0; i < fieldDDPropertiesNodeList.size(); ++i )
  {
    const QDomElement DDElement = fieldDDPropertiesNodeList.at( i ).toElement();
    QgsPropertyCollection collection;
    collection.readXml( DDElement, propertyDefinitions() );
    d->mDataDefinedFieldProperties.insert( DDElement.attribute( u"name"_s ), collection );
  }

  const QDomNodeList widgetsNodeList = node.namedItem( u"widgets"_s ).toElement().childNodes();

  for ( int i = 0; i < widgetsNodeList.size(); ++i )
  {
    const QDomElement widgetElement = widgetsNodeList.at( i ).toElement();
    const QVariant config = QgsXmlUtils::readVariant( widgetElement.firstChildElement( u"config"_s ) );

    d->mWidgetConfigs[widgetElement.attribute( u"name"_s )] = config.toMap();
  }

  // tabs and groups display info
  const QDomNode attributeEditorFormNode = node.namedItem( u"attributeEditorForm"_s );
  if ( !attributeEditorFormNode.isNull() )
  {
    const QDomNodeList attributeEditorFormNodeList = attributeEditorFormNode.toElement().childNodes();

    if ( attributeEditorFormNodeList.size() )
    {
      d->mConfiguredRootContainer = true;
      clearTabs();

      for ( int i = 0; i < attributeEditorFormNodeList.size(); i++ )
      {
        QDomElement elem = attributeEditorFormNodeList.at( i ).toElement();

        fixLegacyConfig( elem );

        const QString layerId = node.namedItem( u"id"_s ).toElement().text();
        QgsAttributeEditorElement *attributeEditorWidget = QgsAttributeEditorElement::create( elem, layerId, d->mFields, context, nullptr );
        if ( attributeEditorWidget )
          addTab( attributeEditorWidget );
      }

      onRelationsLoaded();
    }
  }
}

void QgsEditFormConfig::fixLegacyConfig( QDomElement &el ) const
{
  // recursive method to move widget config into attribute element config

  if ( el.tagName() == "attributeEditorRelation"_L1 )
  {
    if ( !el.hasAttribute( u"forceSuppressFormPopup"_s ) )
    {
      // pre QGIS 3.16 compatibility - the widgets section is read before
      const bool forceSuppress = widgetConfig( el.attribute( u"relation"_s ) ).value( u"force-suppress-popup"_s, false ).toBool();
      el.setAttribute( u"forceSuppressFormPopup"_s, forceSuppress ? 1 : 0 );
    }
    if ( !el.hasAttribute( u"nmRelationId"_s ) )
    {
      // pre QGIS 3.16 compatibility - the widgets section is read before
      el.setAttribute( u"nmRelationId"_s, widgetConfig( el.attribute( u"relation"_s ) ).value( u"nm-rel"_s ).toString() );
    }
  }

  const QDomNodeList children = el.childNodes();
  for ( int i = 0; i < children.size(); i++ )
  {
    QDomElement child = children.at( i ).toElement();
    fixLegacyConfig( child );
    el.replaceChild( child, children.at( i ) );
  }
}

void QgsEditFormConfig::writeXml( QDomNode &node, const QgsReadWriteContext &context ) const
{
  QDomDocument doc( node.ownerDocument() );

  QDomElement efField  = doc.createElement( u"editform"_s );
  efField.setAttribute( u"tolerant"_s, u"1"_s );
  const QDomText efText = doc.createTextNode( context.pathResolver().writePath( uiForm() ) );
  efField.appendChild( efText );
  node.appendChild( efField );

  QDomElement efiField  = doc.createElement( u"editforminit"_s );
  if ( !initFunction().isEmpty() )
    efiField.appendChild( doc.createTextNode( initFunction() ) );
  node.appendChild( efiField );

  QDomElement eficsField  = doc.createElement( u"editforminitcodesource"_s );
  eficsField.appendChild( doc.createTextNode( QString::number( static_cast< int >( initCodeSource() ) ) ) );
  node.appendChild( eficsField );

  QDomElement efifpField  = doc.createElement( u"editforminitfilepath"_s );
  efifpField.appendChild( doc.createTextNode( context.pathResolver().writePath( initFilePath() ) ) );
  node.appendChild( efifpField );

  QDomElement eficField  = doc.createElement( u"editforminitcode"_s );
  eficField.appendChild( doc.createCDATASection( initCode() ) );
  node.appendChild( eficField );

  QDomElement fFSuppElem  = doc.createElement( u"featformsuppress"_s );
  const QDomText fFSuppText = doc.createTextNode( QString::number( static_cast< int >( suppress() ) ) );
  fFSuppElem.appendChild( fFSuppText );
  node.appendChild( fFSuppElem );

  // tab display
  QDomElement editorLayoutElem  = doc.createElement( u"editorlayout"_s );
  switch ( layout() )
  {
    case Qgis::AttributeFormLayout::UiFile:
      editorLayoutElem.appendChild( doc.createTextNode( u"uifilelayout"_s ) );
      break;

    case Qgis::AttributeFormLayout::DragAndDrop:
      editorLayoutElem.appendChild( doc.createTextNode( u"tablayout"_s ) );
      break;

    case Qgis::AttributeFormLayout::AutoGenerated:
    default:
      editorLayoutElem.appendChild( doc.createTextNode( u"generatedlayout"_s ) );
      break;
  }

  node.appendChild( editorLayoutElem );

  // tabs and groups of edit form
  if ( !tabs().empty() && d->mConfiguredRootContainer )
  {
    QDomElement tabsElem = doc.createElement( u"attributeEditorForm"_s );
    const QDomElement rootElem = d->mInvisibleRootContainer->toDomElement( doc );
    const QDomNodeList elemList = rootElem.childNodes();
    while ( !elemList.isEmpty() )
    {
      tabsElem.appendChild( elemList.at( 0 ) );
    }
    node.appendChild( tabsElem );
  }

  QDomElement editableElem = doc.createElement( u"editable"_s );
  for ( auto editIt = d->mFieldEditables.constBegin(); editIt != d->mFieldEditables.constEnd(); ++editIt )
  {
    QDomElement fieldElem = doc.createElement( u"field"_s );
    fieldElem.setAttribute( u"name"_s, editIt.key() );
    fieldElem.setAttribute( u"editable"_s, editIt.value() ? u"1"_s : u"0"_s );
    editableElem.appendChild( fieldElem );
  }
  node.appendChild( editableElem );

  QDomElement labelOnTopElem = doc.createElement( u"labelOnTop"_s );
  for ( auto labelOnTopIt = d->mLabelOnTop.constBegin(); labelOnTopIt != d->mLabelOnTop.constEnd(); ++labelOnTopIt )
  {
    QDomElement fieldElem = doc.createElement( u"field"_s );
    fieldElem.setAttribute( u"name"_s, labelOnTopIt.key() );
    fieldElem.setAttribute( u"labelOnTop"_s, labelOnTopIt.value() ? u"1"_s : u"0"_s );
    labelOnTopElem.appendChild( fieldElem );
  }
  node.appendChild( labelOnTopElem );

  QDomElement reuseLastValuePolicyElem = doc.createElement( u"reuseLastValuePolicy"_s );
  for ( auto reuseLastValuePolicyIt = d->mReuseLastValuePolicy.constBegin(); reuseLastValuePolicyIt != d->mReuseLastValuePolicy.constEnd(); ++reuseLastValuePolicyIt )
  {
    QDomElement fieldElem = doc.createElement( u"field"_s );
    fieldElem.setAttribute( u"name"_s, reuseLastValuePolicyIt.key() );
    fieldElem.setAttribute( u"reuseLastValuePolicy"_s, qgsEnumValueToKey( reuseLastValuePolicyIt.value() ) );
    reuseLastValuePolicyElem.appendChild( fieldElem );
  }
  node.appendChild( reuseLastValuePolicyElem );

  // Store data defined field properties
  QDomElement ddFieldPropsElement = doc.createElement( u"dataDefinedFieldProperties"_s );
  for ( auto it = d->mDataDefinedFieldProperties.constBegin(); it != d->mDataDefinedFieldProperties.constEnd(); ++it )
  {
    QDomElement ddPropsElement = doc.createElement( u"field"_s );
    ddPropsElement.setAttribute( u"name"_s, it.key() );
    it.value().writeXml( ddPropsElement, propertyDefinitions() );
    ddFieldPropsElement.appendChild( ddPropsElement );
  }
  node.appendChild( ddFieldPropsElement );

  QDomElement widgetsElem = doc.createElement( u"widgets"_s );

  QMap<QString, QVariantMap >::ConstIterator configIt( d->mWidgetConfigs.constBegin() );

  while ( configIt != d->mWidgetConfigs.constEnd() )
  {
    QDomElement widgetElem = doc.createElement( u"widget"_s );
    widgetElem.setAttribute( u"name"_s, configIt.key() );
    // widgetElem.setAttribute( "notNull",  );

    QDomElement configElem = QgsXmlUtils::writeVariant( configIt.value(), doc );
    configElem.setTagName( u"config"_s );
    widgetElem.appendChild( configElem );
    widgetsElem.appendChild( widgetElem );
    ++configIt;
  }

  node.appendChild( widgetsElem );
}

QgsAttributeEditorElement *QgsEditFormConfig::attributeEditorElementFromDomElement( QDomElement &elem, QgsAttributeEditorElement *parent, const QString &layerId, const QgsReadWriteContext &context )
{
  return QgsAttributeEditorElement::create( elem, layerId, d->mFields, context, parent );
}
