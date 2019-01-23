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

//#include "qgseditorwidgetregistry.h"

QgsAttributeEditorContainer::~QgsAttributeEditorContainer()
{
  qDeleteAll( mChildren );
}

QgsEditFormConfig::QgsEditFormConfig()
  : d( new QgsEditFormConfigPrivate() )
{
}

QVariantMap QgsEditFormConfig::widgetConfig( const QString &widgetName ) const
{
  int fieldIndex = d->mFields.indexOf( widgetName );
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
  const QList<QgsAttributeEditorElement *> relations = d->mInvisibleRootContainer->findElements( QgsAttributeEditorElement::AeTypeRelation );

  for ( QgsAttributeEditorElement *relElem : relations )
  {
    QgsAttributeEditorRelation *rel = dynamic_cast< QgsAttributeEditorRelation * >( relElem );
    if ( !rel )
      continue;

    rel->init( QgsProject::instance()->relationManager() );
  }
}

bool QgsEditFormConfig::setWidgetConfig( const QString &widgetName, const QVariantMap &config )
{
  if ( d->mFields.indexOf( widgetName ) != -1 )
  {
    QgsDebugMsg( QStringLiteral( "Trying to set a widget config for a field on QgsEditFormConfig. Use layer->setEditorWidgetSetup() instead." ) );
    return false;
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

bool QgsEditFormConfig::operator==( const QgsEditFormConfig &o )
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

QgsEditFormConfig::EditorLayout QgsEditFormConfig::layout() const
{
  return d->mEditorLayout;
}

void QgsEditFormConfig::setLayout( QgsEditFormConfig::EditorLayout editorLayout )
{
  d.detach();
  d->mEditorLayout = editorLayout;

  if ( editorLayout == TabLayout )
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
    QgsApplication::instance()->networkContentFetcherRegistry()->fetch( ui, QgsNetworkContentFetcherRegistry::DownloadImmediately );
  }

  if ( ui.isEmpty() )
  {
    setLayout( GeneratedLayout );
  }
  else
  {
    setLayout( UiFileLayout );
  }
  d->mUiFormPath = ui;
}

bool QgsEditFormConfig::readOnly( int idx ) const
{
  if ( idx >= 0 && idx < d->mFields.count() )
  {
    if ( d->mFields.fieldOrigin( idx ) == QgsFields::OriginJoin
         || d->mFields.fieldOrigin( idx ) == QgsFields::OriginExpression )
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
}

QgsEditFormConfig::PythonInitCodeSource QgsEditFormConfig::initCodeSource() const
{
  return d->mInitCodeSource;
}

void QgsEditFormConfig::setInitCodeSource( const QgsEditFormConfig::PythonInitCodeSource initCodeSource )
{
  d.detach();
  d->mInitCodeSource = initCodeSource;
}

QgsEditFormConfig::FeatureFormSuppress QgsEditFormConfig::suppress() const
{
  return d->mSuppressForm;
}

void QgsEditFormConfig::setSuppress( QgsEditFormConfig::FeatureFormSuppress s )
{
  d.detach();
  d->mSuppressForm = s;
}

void QgsEditFormConfig::readXml( const QDomNode &node, QgsReadWriteContext &context )
{
  QgsReadWriteContextCategoryPopper p = context.enterCategory( QObject::tr( "Edit form config" ) );

  d.detach();

  QDomNode editFormNode = node.namedItem( QStringLiteral( "editform" ) );
  if ( !editFormNode.isNull() )
  {
    QDomElement e = editFormNode.toElement();
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

  QDomNode editFormInitNode = node.namedItem( QStringLiteral( "editforminit" ) );
  if ( !editFormInitNode.isNull() )
  {
    d->mInitFunction = editFormInitNode.toElement().text();
  }

  QDomNode editFormInitCodeSourceNode = node.namedItem( QStringLiteral( "editforminitcodesource" ) );
  if ( !editFormInitCodeSourceNode.isNull() && !editFormInitCodeSourceNode.toElement().text().isEmpty() )
  {
    setInitCodeSource( static_cast< QgsEditFormConfig::PythonInitCodeSource >( editFormInitCodeSourceNode.toElement().text().toInt() ) );
  }

  QDomNode editFormInitCodeNode = node.namedItem( QStringLiteral( "editforminitcode" ) );
  if ( !editFormInitCodeNode.isNull() )
  {
    setInitCode( editFormInitCodeNode.toElement().text() );
  }

  // Temporary < 2.12 b/w compatibility "dot" support patch
  // \see: https://github.com/qgis/QGIS/pull/2498
  // For b/w compatibility, check if there's a dot in the function name
  // and if yes, transform it in an import statement for the module
  // and set the PythonInitCodeSource to CodeSourceDialog
  int dotPos = d->mInitFunction.lastIndexOf( '.' );
  if ( dotPos >= 0 ) // It's a module
  {
    setInitCodeSource( QgsEditFormConfig::CodeSourceDialog );
    setInitCode( QStringLiteral( "from %1 import %2\n" ).arg( d->mInitFunction.left( dotPos ), d->mInitFunction.mid( dotPos + 1 ) ) );
    setInitFunction( d->mInitFunction.mid( dotPos + 1 ) );
  }

  QDomNode editFormInitFilePathNode = node.namedItem( QStringLiteral( "editforminitfilepath" ) );
  if ( !editFormInitFilePathNode.isNull() && !editFormInitFilePathNode.toElement().text().isEmpty() )
  {
    setInitFilePath( context.pathResolver().readPath( editFormInitFilePathNode.toElement().text() ) );
  }

  QDomNode fFSuppNode = node.namedItem( QStringLiteral( "featformsuppress" ) );
  if ( fFSuppNode.isNull() )
  {
    d->mSuppressForm = QgsEditFormConfig::SuppressDefault;
  }
  else
  {
    QDomElement e = fFSuppNode.toElement();
    d->mSuppressForm = static_cast< QgsEditFormConfig::FeatureFormSuppress >( e.text().toInt() );
  }

  // tab display
  QDomNode editorLayoutNode = node.namedItem( QStringLiteral( "editorlayout" ) );
  if ( editorLayoutNode.isNull() )
  {
    d->mEditorLayout = QgsEditFormConfig::GeneratedLayout;
  }
  else
  {
    if ( editorLayoutNode.toElement().text() == QLatin1String( "uifilelayout" ) )
    {
      d->mEditorLayout = QgsEditFormConfig::UiFileLayout;
    }
    else if ( editorLayoutNode.toElement().text() == QLatin1String( "tablayout" ) )
    {
      d->mEditorLayout = QgsEditFormConfig::TabLayout;
    }
    else
    {
      d->mEditorLayout = QgsEditFormConfig::GeneratedLayout;
    }
  }

  d->mFieldEditables.clear();
  QDomNodeList editableNodeList = node.namedItem( QStringLiteral( "editable" ) ).toElement().childNodes();
  for ( int i = 0; i < editableNodeList.size(); ++i )
  {
    QDomElement editableElement = editableNodeList.at( i ).toElement();
    d->mFieldEditables.insert( editableElement.attribute( QStringLiteral( "name" ) ), static_cast< bool >( editableElement.attribute( QStringLiteral( "editable" ) ).toInt() ) );
  }

  d->mLabelOnTop.clear();
  QDomNodeList labelOnTopNodeList = node.namedItem( QStringLiteral( "labelOnTop" ) ).toElement().childNodes();
  for ( int i = 0; i < labelOnTopNodeList.size(); ++i )
  {
    QDomElement labelOnTopElement = labelOnTopNodeList.at( i ).toElement();
    d->mLabelOnTop.insert( labelOnTopElement.attribute( QStringLiteral( "name" ) ), static_cast< bool >( labelOnTopElement.attribute( QStringLiteral( "labelOnTop" ) ).toInt() ) );
  }

  QDomNodeList widgetsNodeList = node.namedItem( QStringLiteral( "widgets" ) ).toElement().childNodes();

  for ( int i = 0; i < widgetsNodeList.size(); ++i )
  {
    QDomElement widgetElement = widgetsNodeList.at( i ).toElement();
    QVariant config = QgsXmlUtils::readVariant( widgetElement.firstChildElement( QStringLiteral( "config" ) ) );

    d->mWidgetConfigs[widgetElement.attribute( QStringLiteral( "name" ) )] = config.toMap();
  }

  // tabs and groups display info
  QDomNode attributeEditorFormNode = node.namedItem( QStringLiteral( "attributeEditorForm" ) );
  if ( !attributeEditorFormNode.isNull() )
  {
    QDomNodeList attributeEditorFormNodeList = attributeEditorFormNode.toElement().childNodes();

    if ( attributeEditorFormNodeList.size() )
    {
      d->mConfiguredRootContainer = true;
      clearTabs();

      for ( int i = 0; i < attributeEditorFormNodeList.size(); i++ )
      {
        QDomElement elem = attributeEditorFormNodeList.at( i ).toElement();

        QgsAttributeEditorElement *attributeEditorWidget = attributeEditorElementFromDomElement( elem, nullptr, node.namedItem( QStringLiteral( "id" ) ).toElement().text(), context );
        addTab( attributeEditorWidget );
      }

      onRelationsLoaded();
    }
  }
}

void QgsEditFormConfig::writeXml( QDomNode &node, const QgsReadWriteContext &context ) const
{
  QDomDocument doc( node.ownerDocument() );

  QDomElement efField  = doc.createElement( QStringLiteral( "editform" ) );
  efField.setAttribute( QStringLiteral( "tolerant" ), QStringLiteral( "1" ) );
  QDomText efText = doc.createTextNode( context.pathResolver().writePath( uiForm() ) );
  efField.appendChild( efText );
  node.appendChild( efField );

  QDomElement efiField  = doc.createElement( QStringLiteral( "editforminit" ) );
  if ( !initFunction().isEmpty() )
    efiField.appendChild( doc.createTextNode( initFunction() ) );
  node.appendChild( efiField );

  QDomElement eficsField  = doc.createElement( QStringLiteral( "editforminitcodesource" ) );
  eficsField.appendChild( doc.createTextNode( QString::number( initCodeSource() ) ) );
  node.appendChild( eficsField );

  QDomElement efifpField  = doc.createElement( QStringLiteral( "editforminitfilepath" ) );
  efifpField.appendChild( doc.createTextNode( context.pathResolver().writePath( initFilePath() ) ) );
  node.appendChild( efifpField );

  QDomElement eficField  = doc.createElement( QStringLiteral( "editforminitcode" ) );
  eficField.appendChild( doc.createCDATASection( initCode() ) );
  node.appendChild( eficField );

  QDomElement fFSuppElem  = doc.createElement( QStringLiteral( "featformsuppress" ) );
  QDomText fFSuppText = doc.createTextNode( QString::number( suppress() ) );
  fFSuppElem.appendChild( fFSuppText );
  node.appendChild( fFSuppElem );

  // tab display
  QDomElement editorLayoutElem  = doc.createElement( QStringLiteral( "editorlayout" ) );
  switch ( layout() )
  {
    case QgsEditFormConfig::UiFileLayout:
      editorLayoutElem.appendChild( doc.createTextNode( QStringLiteral( "uifilelayout" ) ) );
      break;

    case QgsEditFormConfig::TabLayout:
      editorLayoutElem.appendChild( doc.createTextNode( QStringLiteral( "tablayout" ) ) );
      break;

    case QgsEditFormConfig::GeneratedLayout:
    default:
      editorLayoutElem.appendChild( doc.createTextNode( QStringLiteral( "generatedlayout" ) ) );
      break;
  }

  node.appendChild( editorLayoutElem );

  // tabs and groups of edit form
  if ( !tabs().empty() && d->mConfiguredRootContainer )
  {
    QDomElement tabsElem = doc.createElement( QStringLiteral( "attributeEditorForm" ) );

    QDomElement rootElem = d->mInvisibleRootContainer->toDomElement( doc );
    QDomNodeList elemList = rootElem.childNodes();

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

  //// END TODO
}

QgsAttributeEditorElement *QgsEditFormConfig::attributeEditorElementFromDomElement( QDomElement &elem, QgsAttributeEditorElement *parent, const QString &layerId, const QgsReadWriteContext &context )
{
  QgsAttributeEditorElement *newElement = nullptr;

  if ( elem.tagName() == QLatin1String( "attributeEditorContainer" ) )
  {
    QgsAttributeEditorContainer *container = new QgsAttributeEditorContainer( context.projectTranslator()->translate( QStringLiteral( "project:layers:%1:formcontainers" ).arg( layerId ), elem.attribute( QStringLiteral( "name" ) ) ), parent );
    bool ok;
    int cc = elem.attribute( QStringLiteral( "columnCount" ) ).toInt( &ok );
    if ( !ok )
      cc = 0;
    container->setColumnCount( cc );

    bool isGroupBox = elem.attribute( QStringLiteral( "groupBox" ) ).toInt( &ok );
    if ( ok )
      container->setIsGroupBox( isGroupBox );
    else
      container->setIsGroupBox( parent );

    bool visibilityExpressionEnabled = elem.attribute( QStringLiteral( "visibilityExpressionEnabled" ) ).toInt( &ok );
    QgsOptionalExpression visibilityExpression;
    if ( ok )
    {
      visibilityExpression.setEnabled( visibilityExpressionEnabled );
      visibilityExpression.setData( QgsExpression( elem.attribute( QStringLiteral( "visibilityExpression" ) ) ) );
    }
    container->setVisibilityExpression( visibilityExpression );

    QDomNodeList childNodeList = elem.childNodes();

    for ( int i = 0; i < childNodeList.size(); i++ )
    {
      QDomElement childElem = childNodeList.at( i ).toElement();
      QgsAttributeEditorElement *myElem = attributeEditorElementFromDomElement( childElem, container, layerId, context );
      if ( myElem )
        container->addChildElement( myElem );
    }

    newElement = container;
  }
  else if ( elem.tagName() == QLatin1String( "attributeEditorField" ) )
  {
    QString name = elem.attribute( QStringLiteral( "name" ) );
    int idx = d->mFields.lookupField( name );
    newElement = new QgsAttributeEditorField( name, idx, parent );
  }
  else if ( elem.tagName() == QLatin1String( "attributeEditorRelation" ) )
  {
    // At this time, the relations are not loaded
    // So we only grab the id and delegate the rest to onRelationsLoaded()
    QgsAttributeEditorRelation *relElement = new QgsAttributeEditorRelation( elem.attribute( QStringLiteral( "relation" ), QStringLiteral( "[None]" ) ), parent );
    relElement->setShowLinkButton( elem.attribute( QStringLiteral( "showLinkButton" ), QStringLiteral( "1" ) ).toInt() );
    relElement->setShowUnlinkButton( elem.attribute( QStringLiteral( "showUnlinkButton" ), QStringLiteral( "1" ) ).toInt() );
    newElement = relElement;
  }
  else if ( elem.tagName() == QLatin1String( "attributeEditorQmlElement" ) )
  {
    QgsAttributeEditorQmlElement *qmlElement = new QgsAttributeEditorQmlElement( elem.attribute( QStringLiteral( "name" ) ), parent );
    qmlElement->setQmlCode( elem.text() );
    newElement = qmlElement;
  }

  if ( newElement )
  {
    if ( elem.hasAttribute( QStringLiteral( "showLabel" ) ) )
      newElement->setShowLabel( elem.attribute( QStringLiteral( "showLabel" ) ).toInt() );
    else
      newElement->setShowLabel( true );
  }

  return newElement;
}

int QgsAttributeEditorContainer::columnCount() const
{
  return mColumnCount;
}

void QgsAttributeEditorContainer::setColumnCount( int columnCount )
{
  mColumnCount = columnCount;
}

QgsAttributeEditorElement *QgsAttributeEditorContainer::clone( QgsAttributeEditorElement *parent ) const
{
  QgsAttributeEditorContainer *element = new QgsAttributeEditorContainer( name(), parent );

  const auto childElements = children();

  for ( QgsAttributeEditorElement *child : childElements )
  {
    element->addChildElement( child->clone( element ) );
  }
  element->mIsGroupBox = mIsGroupBox;
  element->mColumnCount = mColumnCount;
  element->mVisibilityExpression = mVisibilityExpression;

  return element;
}

void QgsAttributeEditorContainer::saveConfiguration( QDomElement &elem ) const
{
  elem.setAttribute( QStringLiteral( "columnCount" ), mColumnCount );
  elem.setAttribute( QStringLiteral( "groupBox" ), mIsGroupBox ? 1 : 0 );
  elem.setAttribute( QStringLiteral( "visibilityExpressionEnabled" ), mVisibilityExpression.enabled() ? 1 : 0 );
  elem.setAttribute( QStringLiteral( "visibilityExpression" ), mVisibilityExpression->expression() );

  Q_FOREACH ( QgsAttributeEditorElement *child, mChildren )
  {
    QDomDocument doc = elem.ownerDocument();
    elem.appendChild( child->toDomElement( doc ) );
  }
}

QString QgsAttributeEditorContainer::typeIdentifier() const
{
  return QStringLiteral( "attributeEditorContainer" );
}
