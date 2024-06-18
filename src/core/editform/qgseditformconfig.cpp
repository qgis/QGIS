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

#include "qgseditformconfig_p.h"
#include "qgseditformconfig.h"
#include "qgsnetworkcontentfetcherregistry.h"
#include "qgspathresolver.h"
#include "qgsproject.h"
#include "qgsreadwritecontext.h"
#include "qgsrelationmanager.h"
#include "qgslogger.h"
#include "qgsxmlutils.h"
#include "qgsapplication.h"
#include "qgsmessagelog.h"
#include "qgsattributeeditorcontainer.h"
#include "qgsattributeeditorfield.h"
#include "qgsattributeeditorrelation.h"
#include <QUrl>

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

    rel->init( QgsProject::instance()->relationManager() );
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
          if ( config.contains( QStringLiteral( "nm-rel" ) ) )
          {
            relation->setNmRelationId( config[QStringLiteral( "nm-rel" )] );
          }
          if ( config.contains( QStringLiteral( "force-suppress-popup" ) ) )
          {
            relation->setForceSuppressFormPopup( config[QStringLiteral( "force-suppress-popup" )].toBool() );
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
    QgsDebugError( QStringLiteral( "Trying to set a widget config for a field on QgsEditFormConfig. Use layer->setEditorWidgetSetup() instead." ) );
    return false;
  }

  //for legacy use it writes the relation editor configuration into the first instance of the widget
  if ( config.contains( QStringLiteral( "force-suppress-popup" ) ) || config.contains( QStringLiteral( "nm-rel" ) ) )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Deprecation Warning: Trying to set a relation config directly on the relation %1. Relation settings should be done for the specific widget instance instead. Use attributeEditorRelation->setNmRelationId() or attributeEditorRelation->setForceSuppressFormPopup() instead." ).arg( widgetName ) );
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
    return d->mReuseLastValue.value( d->mFields.at( index ).name(), false );
  else
    return false;
}

void QgsEditFormConfig::setReuseLastValue( int index, bool reuse )
{
  if ( index >= 0 && index < d->mFields.count() )
  {
    d.detach();
    d->mReuseLastValue[ d->mFields.at( index ).name()] = reuse;
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

  const QDomNode editFormNode = node.namedItem( QStringLiteral( "editform" ) );
  if ( !editFormNode.isNull() )
  {
    const QDomElement e = editFormNode.toElement();
    const bool tolerantRemoteUrls = e.hasAttribute( QStringLiteral( "tolerant" ) );
    if ( !e.text().isEmpty() )
    {
      const QString uiFormPath = context.pathResolver().readPath( e.text() );
      // <= 3.2 had a bug where invalid ui paths would get written into projects on load
      // to avoid restoring these invalid paths, we take a less-tolerant approach for older (untrustworthy) projects
      // and only set ui forms paths IF they are local files OR start with "http(s)".
      const bool localFile = QFileInfo::exists( uiFormPath );
      if ( localFile || tolerantRemoteUrls || uiFormPath.startsWith( QLatin1String( "http" ) ) )
        setUiForm( uiFormPath );
    }
  }

  const QDomNode editFormInitNode = node.namedItem( QStringLiteral( "editforminit" ) );
  if ( !editFormInitNode.isNull() )
  {
    d->mInitFunction = editFormInitNode.toElement().text();
  }

  const QDomNode editFormInitCodeSourceNode = node.namedItem( QStringLiteral( "editforminitcodesource" ) );
  if ( !editFormInitCodeSourceNode.isNull() && !editFormInitCodeSourceNode.toElement().text().isEmpty() )
  {
    setInitCodeSource( static_cast< Qgis::AttributeFormPythonInitCodeSource >( editFormInitCodeSourceNode.toElement().text().toInt() ) );
  }

  const QDomNode editFormInitCodeNode = node.namedItem( QStringLiteral( "editforminitcode" ) );
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
    setInitCode( QStringLiteral( "from %1 import %2\n" ).arg( d->mInitFunction.left( dotPos ), d->mInitFunction.mid( dotPos + 1 ) ) );
    setInitFunction( d->mInitFunction.mid( dotPos + 1 ) );
  }

  const QDomNode editFormInitFilePathNode = node.namedItem( QStringLiteral( "editforminitfilepath" ) );
  if ( !editFormInitFilePathNode.isNull() && !editFormInitFilePathNode.toElement().text().isEmpty() )
  {
    setInitFilePath( context.pathResolver().readPath( editFormInitFilePathNode.toElement().text() ) );
  }

  const QDomNode fFSuppNode = node.namedItem( QStringLiteral( "featformsuppress" ) );
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
  const QDomNode editorLayoutNode = node.namedItem( QStringLiteral( "editorlayout" ) );
  if ( editorLayoutNode.isNull() )
  {
    d->mEditorLayout = Qgis::AttributeFormLayout::AutoGenerated;
  }
  else
  {
    if ( editorLayoutNode.toElement().text() == QLatin1String( "uifilelayout" ) )
    {
      d->mEditorLayout = Qgis::AttributeFormLayout::UiFile;
    }
    else if ( editorLayoutNode.toElement().text() == QLatin1String( "tablayout" ) )
    {
      d->mEditorLayout = Qgis::AttributeFormLayout::DragAndDrop;
    }
    else
    {
      d->mEditorLayout = Qgis::AttributeFormLayout::AutoGenerated;
    }
  }

  d->mFieldEditables.clear();
  const QDomNodeList editableNodeList = node.namedItem( QStringLiteral( "editable" ) ).toElement().childNodes();
  for ( int i = 0; i < editableNodeList.size(); ++i )
  {
    const QDomElement editableElement = editableNodeList.at( i ).toElement();
    d->mFieldEditables.insert( editableElement.attribute( QStringLiteral( "name" ) ), static_cast< bool >( editableElement.attribute( QStringLiteral( "editable" ) ).toInt() ) );
  }

  d->mLabelOnTop.clear();
  const QDomNodeList labelOnTopNodeList = node.namedItem( QStringLiteral( "labelOnTop" ) ).toElement().childNodes();
  for ( int i = 0; i < labelOnTopNodeList.size(); ++i )
  {
    const QDomElement labelOnTopElement = labelOnTopNodeList.at( i ).toElement();
    d->mLabelOnTop.insert( labelOnTopElement.attribute( QStringLiteral( "name" ) ), static_cast< bool >( labelOnTopElement.attribute( QStringLiteral( "labelOnTop" ) ).toInt() ) );
  }

  d->mReuseLastValue.clear();
  const QDomNodeList reuseLastValueNodeList = node.namedItem( QStringLiteral( "reuseLastValue" ) ).toElement().childNodes();
  for ( int i = 0; i < reuseLastValueNodeList.size(); ++i )
  {
    const QDomElement reuseLastValueElement = reuseLastValueNodeList.at( i ).toElement();
    d->mReuseLastValue.insert( reuseLastValueElement.attribute( QStringLiteral( "name" ) ), static_cast< bool >( reuseLastValueElement.attribute( QStringLiteral( "reuseLastValue" ) ).toInt() ) );
  }

  // Read data defined field properties
  const QDomNodeList fieldDDPropertiesNodeList = node.namedItem( QStringLiteral( "dataDefinedFieldProperties" ) ).toElement().childNodes();
  for ( int i = 0; i < fieldDDPropertiesNodeList.size(); ++i )
  {
    const QDomElement DDElement = fieldDDPropertiesNodeList.at( i ).toElement();
    QgsPropertyCollection collection;
    collection.readXml( DDElement, propertyDefinitions() );
    d->mDataDefinedFieldProperties.insert( DDElement.attribute( QStringLiteral( "name" ) ), collection );
  }

  const QDomNodeList widgetsNodeList = node.namedItem( QStringLiteral( "widgets" ) ).toElement().childNodes();

  for ( int i = 0; i < widgetsNodeList.size(); ++i )
  {
    const QDomElement widgetElement = widgetsNodeList.at( i ).toElement();
    const QVariant config = QgsXmlUtils::readVariant( widgetElement.firstChildElement( QStringLiteral( "config" ) ) );

    d->mWidgetConfigs[widgetElement.attribute( QStringLiteral( "name" ) )] = config.toMap();
  }

  // tabs and groups display info
  const QDomNode attributeEditorFormNode = node.namedItem( QStringLiteral( "attributeEditorForm" ) );
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

        const QString layerId = node.namedItem( QStringLiteral( "id" ) ).toElement().text();
        QgsAttributeEditorElement *attributeEditorWidget = QgsAttributeEditorElement::create( elem, layerId, d->mFields, context, nullptr );
        if ( attributeEditorWidget )
          addTab( attributeEditorWidget );
      }

      onRelationsLoaded();
    }
  }
}

void QgsEditFormConfig::fixLegacyConfig( QDomElement &el )
{
  // recursive method to move widget config into attribute element config

  if ( el.tagName() == QLatin1String( "attributeEditorRelation" ) )
  {
    if ( !el.hasAttribute( QStringLiteral( "forceSuppressFormPopup" ) ) )
    {
      // pre QGIS 3.16 compatibility - the widgets section is read before
      const bool forceSuppress = widgetConfig( el.attribute( QStringLiteral( "relation" ) ) ).value( QStringLiteral( "force-suppress-popup" ), false ).toBool();
      el.setAttribute( QStringLiteral( "forceSuppressFormPopup" ), forceSuppress ? 1 : 0 );
    }
    if ( !el.hasAttribute( QStringLiteral( "nmRelationId" ) ) )
    {
      // pre QGIS 3.16 compatibility - the widgets section is read before
      el.setAttribute( QStringLiteral( "nmRelationId" ), widgetConfig( el.attribute( QStringLiteral( "relation" ) ) ).value( QStringLiteral( "nm-rel" ) ).toString() );
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

  QDomElement efField  = doc.createElement( QStringLiteral( "editform" ) );
  efField.setAttribute( QStringLiteral( "tolerant" ), QStringLiteral( "1" ) );
  const QDomText efText = doc.createTextNode( context.pathResolver().writePath( uiForm() ) );
  efField.appendChild( efText );
  node.appendChild( efField );

  QDomElement efiField  = doc.createElement( QStringLiteral( "editforminit" ) );
  if ( !initFunction().isEmpty() )
    efiField.appendChild( doc.createTextNode( initFunction() ) );
  node.appendChild( efiField );

  QDomElement eficsField  = doc.createElement( QStringLiteral( "editforminitcodesource" ) );
  eficsField.appendChild( doc.createTextNode( QString::number( static_cast< int >( initCodeSource() ) ) ) );
  node.appendChild( eficsField );

  QDomElement efifpField  = doc.createElement( QStringLiteral( "editforminitfilepath" ) );
  efifpField.appendChild( doc.createTextNode( context.pathResolver().writePath( initFilePath() ) ) );
  node.appendChild( efifpField );

  QDomElement eficField  = doc.createElement( QStringLiteral( "editforminitcode" ) );
  eficField.appendChild( doc.createCDATASection( initCode() ) );
  node.appendChild( eficField );

  QDomElement fFSuppElem  = doc.createElement( QStringLiteral( "featformsuppress" ) );
  const QDomText fFSuppText = doc.createTextNode( QString::number( static_cast< int >( suppress() ) ) );
  fFSuppElem.appendChild( fFSuppText );
  node.appendChild( fFSuppElem );

  // tab display
  QDomElement editorLayoutElem  = doc.createElement( QStringLiteral( "editorlayout" ) );
  switch ( layout() )
  {
    case Qgis::AttributeFormLayout::UiFile:
      editorLayoutElem.appendChild( doc.createTextNode( QStringLiteral( "uifilelayout" ) ) );
      break;

    case Qgis::AttributeFormLayout::DragAndDrop:
      editorLayoutElem.appendChild( doc.createTextNode( QStringLiteral( "tablayout" ) ) );
      break;

    case Qgis::AttributeFormLayout::AutoGenerated:
    default:
      editorLayoutElem.appendChild( doc.createTextNode( QStringLiteral( "generatedlayout" ) ) );
      break;
  }

  node.appendChild( editorLayoutElem );

  // tabs and groups of edit form
  if ( !tabs().empty() && d->mConfiguredRootContainer )
  {
    QDomElement tabsElem = doc.createElement( QStringLiteral( "attributeEditorForm" ) );
    const QDomElement rootElem = d->mInvisibleRootContainer->toDomElement( doc );
    const QDomNodeList elemList = rootElem.childNodes();
    while ( !elemList.isEmpty() )
    {
      tabsElem.appendChild( elemList.at( 0 ) );
    }
    node.appendChild( tabsElem );
  }

  QDomElement editableElem = doc.createElement( QStringLiteral( "editable" ) );
  for ( auto editIt = d->mFieldEditables.constBegin(); editIt != d->mFieldEditables.constEnd(); ++editIt )
  {
    QDomElement fieldElem = doc.createElement( QStringLiteral( "field" ) );
    fieldElem.setAttribute( QStringLiteral( "name" ), editIt.key() );
    fieldElem.setAttribute( QStringLiteral( "editable" ), editIt.value() ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
    editableElem.appendChild( fieldElem );
  }
  node.appendChild( editableElem );

  QDomElement labelOnTopElem = doc.createElement( QStringLiteral( "labelOnTop" ) );
  for ( auto labelOnTopIt = d->mLabelOnTop.constBegin(); labelOnTopIt != d->mLabelOnTop.constEnd(); ++labelOnTopIt )
  {
    QDomElement fieldElem = doc.createElement( QStringLiteral( "field" ) );
    fieldElem.setAttribute( QStringLiteral( "name" ), labelOnTopIt.key() );
    fieldElem.setAttribute( QStringLiteral( "labelOnTop" ), labelOnTopIt.value() ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
    labelOnTopElem.appendChild( fieldElem );
  }
  node.appendChild( labelOnTopElem );

  QDomElement reuseLastValueElem = doc.createElement( QStringLiteral( "reuseLastValue" ) );
  for ( auto reuseLastValueIt = d->mReuseLastValue.constBegin(); reuseLastValueIt != d->mReuseLastValue.constEnd(); ++reuseLastValueIt )
  {
    QDomElement fieldElem = doc.createElement( QStringLiteral( "field" ) );
    fieldElem.setAttribute( QStringLiteral( "name" ), reuseLastValueIt.key() );
    fieldElem.setAttribute( QStringLiteral( "reuseLastValue" ), reuseLastValueIt.value() ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
    reuseLastValueElem.appendChild( fieldElem );
  }
  node.appendChild( reuseLastValueElem );

  // Store data defined field properties
  QDomElement ddFieldPropsElement = doc.createElement( QStringLiteral( "dataDefinedFieldProperties" ) );
  for ( auto it = d->mDataDefinedFieldProperties.constBegin(); it != d->mDataDefinedFieldProperties.constEnd(); ++it )
  {
    QDomElement ddPropsElement = doc.createElement( QStringLiteral( "field" ) );
    ddPropsElement.setAttribute( QStringLiteral( "name" ), it.key() );
    it.value().writeXml( ddPropsElement, propertyDefinitions() );
    ddFieldPropsElement.appendChild( ddPropsElement );
  }
  node.appendChild( ddFieldPropsElement );

  QDomElement widgetsElem = doc.createElement( QStringLiteral( "widgets" ) );

  QMap<QString, QVariantMap >::ConstIterator configIt( d->mWidgetConfigs.constBegin() );

  while ( configIt != d->mWidgetConfigs.constEnd() )
  {
    QDomElement widgetElem = doc.createElement( QStringLiteral( "widget" ) );
    widgetElem.setAttribute( QStringLiteral( "name" ), configIt.key() );
    // widgetElem.setAttribute( "notNull",  );

    QDomElement configElem = QgsXmlUtils::writeVariant( configIt.value(), doc );
    configElem.setTagName( QStringLiteral( "config" ) );
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
