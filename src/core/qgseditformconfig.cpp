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
#include "qgsproject.h"
#include "qgsrelationmanager.h"

//#include "qgseditorwidgetregistry.h"

QgsAttributeEditorContainer::~QgsAttributeEditorContainer()
{
  qDeleteAll( mChildren );
}

QgsEditFormConfig::QgsEditFormConfig()
    : d( new QgsEditFormConfigPrivate() )
{
}

QString QgsEditFormConfig::widgetType( const QString& fieldName ) const
{
  return d->mEditorWidgetTypes.value( fieldName );
}

QgsEditorWidgetConfig QgsEditFormConfig::widgetConfig( const QString& fieldName ) const
{
  return d->mWidgetConfigs.value( fieldName );
}

void QgsEditFormConfig::setFields( const QgsFields& fields )
{
  d.detach();
  d->mFields = fields;

  if ( !d->mConfiguredRootContainer )
  {
    d->mInvisibleRootContainer->clear();
    for ( int i = 0; i < d->mFields.size(); ++i )
    {
      QgsAttributeEditorField* field = new QgsAttributeEditorField( d->mFields.at( i ).name(), i, d->mInvisibleRootContainer );
      d->mInvisibleRootContainer->addChildElement( field );
    }
  }
}

void QgsEditFormConfig::onRelationsLoaded()
{
  QList<QgsAttributeEditorElement*> relations = d->mInvisibleRootContainer->findElements( QgsAttributeEditorElement::AeTypeRelation );

  Q_FOREACH ( QgsAttributeEditorElement* relElem, relations )
  {
    QgsAttributeEditorRelation* rel = dynamic_cast< QgsAttributeEditorRelation* >( relElem );
    if ( !rel )
      continue;

    rel->init( QgsProject::instance()->relationManager() );
  }
}

void QgsEditFormConfig::setWidgetType( const QString& widgetName, const QString& widgetType )
{
  d->mEditorWidgetTypes[widgetName] = widgetType;
}

void QgsEditFormConfig::setWidgetConfig( const QString& widgetName, const QgsEditorWidgetConfig& config )
{
  d.detach();
  d->mWidgetConfigs[widgetName] = config;
}

bool QgsEditFormConfig::removeWidgetConfig( const QString& widgetName )
{
  d.detach();
  return d->mWidgetConfigs.remove( widgetName ) != 0;
}

QgsEditFormConfig::QgsEditFormConfig( const QgsEditFormConfig& o )
    : d( o.d )
{
}

QgsEditFormConfig& QgsEditFormConfig::operator=( const QgsEditFormConfig & o )
{
  d = o.d;
  return *this;
}

bool QgsEditFormConfig::operator==( const QgsEditFormConfig& o )
{
  return d == o.d;
}

QgsEditFormConfig::~QgsEditFormConfig()
{

}

void QgsEditFormConfig::addTab( QgsAttributeEditorElement* data )
{
  d.detach();
  d->mInvisibleRootContainer->addChildElement( data );
}

QList<QgsAttributeEditorElement*> QgsEditFormConfig::tabs() const
{
  return d->mInvisibleRootContainer->children();
}

void QgsEditFormConfig::clearTabs()
{
  d.detach();
  d->mInvisibleRootContainer->clear();
}

QgsAttributeEditorContainer* QgsEditFormConfig::invisibleRootContainer()
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

void QgsEditFormConfig::setUiForm( const QString& ui )
{
  if ( ui.isEmpty() || ui.isNull() )
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

QString QgsEditFormConfig::constraintExpression( int idx ) const
{
  QString expr;

  if ( idx >= 0 && idx < d->mFields.count() )
    expr = d->mConstraints.value( d->mFields.at( idx ).name(), QString() );

  return expr;
}

void QgsEditFormConfig::setConstraintExpression( int idx, const QString& expression )
{
  if ( idx >= 0 && idx < d->mFields.count() )
  {
    d.detach();
    d->mConstraints[ d->mFields.at( idx ).name()] = expression;
  }
}

QString QgsEditFormConfig::constraintDescription( int idx ) const
{
  QString description;

  if ( idx >= 0 && idx < d->mFields.count() )
    description = d->mConstraintsDescription[ d->mFields.at( idx ).name()];

  return description;
}

void QgsEditFormConfig::setContraintDescription( int idx, const QString &descr )
{
  if ( idx >= 0 && idx < d->mFields.count() )
  {
    d.detach();
    d->mConstraintsDescription[ d->mFields.at( idx ).name()] = descr;
  }
}

bool QgsEditFormConfig::notNull( int idx ) const
{
  if ( idx >= 0 && idx < d->mFields.count() )
    return d->mNotNull.value( d->mFields.at( idx ).name(), false );
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

void QgsEditFormConfig::setInitFunction( const QString& function )
{
  d.detach();
  d->mInitFunction = function;
}

QString QgsEditFormConfig::initCode() const
{
  return d->mInitCode;
}

void QgsEditFormConfig::setInitCode( const QString& code )
{
  d.detach();
  d->mInitCode = code;
}

QString QgsEditFormConfig::initFilePath() const
{
  return d->mInitFilePath;
}

void QgsEditFormConfig::setInitFilePath( const QString& filePath )
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

void QgsEditFormConfig::setNotNull( int idx, bool notnull )
{
  if ( idx >= 0 && idx < d->mFields.count() )
  {
    d.detach();
    d->mNotNull[ d->mFields.at( idx ).name()] = notnull;
  }
}

void QgsEditFormConfig::readXml( const QDomNode& node )
{
  d.detach();
  QDomNode editFormNode = node.namedItem( "editform" );
  if ( !editFormNode.isNull() )
  {
    QDomElement e = editFormNode.toElement();
    d->mUiFormPath = QgsProject::instance()->readPath( e.text() );
  }

  QDomNode editFormInitNode = node.namedItem( "editforminit" );
  if ( !editFormInitNode.isNull() )
  {
    d->mInitFunction = editFormInitNode.toElement().text();
  }

  QDomNode editFormInitCodeSourceNode = node.namedItem( "editforminitcodesource" );
  if ( !editFormInitCodeSourceNode.isNull() || ( !editFormInitCodeSourceNode.isNull() && !editFormInitCodeSourceNode.toElement().text().isEmpty() ) )
  {
    setInitCodeSource( static_cast< QgsEditFormConfig::PythonInitCodeSource >( editFormInitCodeSourceNode.toElement().text().toInt() ) );
  }

  QDomNode editFormInitCodeNode = node.namedItem( "editforminitcode" );
  if ( !editFormInitCodeNode.isNull() )
  {
    setInitCode( editFormInitCodeNode.toElement().text() );
  }

  // Temporary < 2.12 b/w compatibility "dot" support patch
  // @see: https://github.com/qgis/QGIS/pull/2498
  // For b/w compatibility, check if there's a dot in the function name
  // and if yes, transform it in an import statement for the module
  // and set the PythonInitCodeSource to CodeSourceDialog
  int dotPos = d->mInitFunction.lastIndexOf( '.' );
  if ( dotPos >= 0 ) // It's a module
  {
    setInitCodeSource( QgsEditFormConfig::CodeSourceDialog );
    setInitCode( QString( "from %1 import %2\n" ).arg( d->mInitFunction.left( dotPos ), d->mInitFunction.mid( dotPos + 1 ) ) );
    setInitFunction( d->mInitFunction.mid( dotPos + 1 ) );
  }

  QDomNode editFormInitFilePathNode = node.namedItem( "editforminitfilepath" );
  if ( !editFormInitFilePathNode.isNull() || ( !editFormInitFilePathNode.isNull() && !editFormInitFilePathNode.toElement().text().isEmpty() ) )
  {
    setInitFilePath( QgsProject::instance()->readPath( editFormInitFilePathNode.toElement().text() ) );
  }

  QDomNode fFSuppNode = node.namedItem( "featformsuppress" );
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
  QDomNode editorLayoutNode = node.namedItem( "editorlayout" );
  if ( editorLayoutNode.isNull() )
  {
    d->mEditorLayout = QgsEditFormConfig::GeneratedLayout;
  }
  else
  {
    if ( editorLayoutNode.toElement().text() == "uifilelayout" )
    {
      d->mEditorLayout = QgsEditFormConfig::UiFileLayout;
    }
    else if ( editorLayoutNode.toElement().text() == "tablayout" )
    {
      d->mEditorLayout = QgsEditFormConfig::TabLayout;
    }
    else
    {
      d->mEditorLayout = QgsEditFormConfig::GeneratedLayout;
    }
  }

  // tabs and groups display info
  QDomNode attributeEditorFormNode = node.namedItem( "attributeEditorForm" );
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

        QgsAttributeEditorElement* attributeEditorWidget = attributeEditorElementFromDomElement( elem, nullptr );
        addTab( attributeEditorWidget );
      }
    }
  }


  QDomElement widgetsElem = node.namedItem( "widgets" ).toElement();

  QDomNodeList widgetConfigsElems = widgetsElem.childNodes();

  for ( int i = 0; i < widgetConfigsElems.size(); ++i )
  {
    const QDomElement wdgElem = widgetConfigsElems.at( i ).toElement();
    const QDomElement cfgElem = wdgElem.namedItem( "config" ).toElement();
    const QgsEditorWidgetConfig widgetConfig = parseEditorWidgetConfig( cfgElem );
    setWidgetConfig( wdgElem.attribute( "name" ), widgetConfig );
  }
}

QgsEditorWidgetConfig QgsEditFormConfig::parseEditorWidgetConfig( const QDomElement& cfgElem )
{
  QgsEditorWidgetConfig cfg;
  //// TODO: MAKE THIS MORE GENERIC, SO INDIVIDUALL WIDGETS CAN NOT ONLY SAVE STRINGS
  /// SEE QgsEditorWidgetFactory::writeConfig
  for ( int j = 0; j < cfgElem.attributes().size(); ++j )
  {
    const QDomAttr attr = cfgElem.attributes().item( j ).toAttr();
    cfg.insert( attr.name(), attr.value() );
  }

  const QDomNodeList optionElements = cfgElem.elementsByTagName( "option" );
  for ( int j = 0; j < optionElements.size(); ++j )
  {
    const QDomElement option = optionElements.at( j ).toElement();
    const QString key = option.attribute( "key" );
    const QString value = option.attribute( "value" );
    cfg.insert( key, value );
  }
  //// END TODO
  return cfg;
}

void QgsEditFormConfig::writeXml( QDomNode& node ) const
{
  QDomDocument doc( node.ownerDocument() );

  QDomElement efField  = doc.createElement( "editform" );
  QDomText efText = doc.createTextNode( QgsProject::instance()->writePath( uiForm() ) );
  efField.appendChild( efText );
  node.appendChild( efField );

  QDomElement efiField  = doc.createElement( "editforminit" );
  if ( !initFunction().isEmpty() )
    efiField.appendChild( doc.createTextNode( initFunction() ) );
  node.appendChild( efiField );

  QDomElement eficsField  = doc.createElement( "editforminitcodesource" );
  eficsField.appendChild( doc.createTextNode( QString::number( initCodeSource() ) ) );
  node.appendChild( eficsField );

  QDomElement efifpField  = doc.createElement( "editforminitfilepath" );
  efifpField.appendChild( doc.createTextNode( QgsProject::instance()->writePath( initFilePath() ) ) );
  node.appendChild( efifpField );

  QDomElement eficField  = doc.createElement( "editforminitcode" );
  eficField.appendChild( doc.createCDATASection( initCode() ) );
  node.appendChild( eficField );

  QDomElement fFSuppElem  = doc.createElement( "featformsuppress" );
  QDomText fFSuppText = doc.createTextNode( QString::number( suppress() ) );
  fFSuppElem.appendChild( fFSuppText );
  node.appendChild( fFSuppElem );

  // tab display
  QDomElement editorLayoutElem  = doc.createElement( "editorlayout" );
  switch ( layout() )
  {
    case QgsEditFormConfig::UiFileLayout:
      editorLayoutElem.appendChild( doc.createTextNode( "uifilelayout" ) );
      break;

    case QgsEditFormConfig::TabLayout:
      editorLayoutElem.appendChild( doc.createTextNode( "tablayout" ) );
      break;

    case QgsEditFormConfig::GeneratedLayout:
    default:
      editorLayoutElem.appendChild( doc.createTextNode( "generatedlayout" ) );
      break;
  }

  node.appendChild( editorLayoutElem );

  // tabs and groups of edit form
  if ( tabs().size() > 0 && d->mConfiguredRootContainer )
  {
    QDomElement tabsElem = doc.createElement( "attributeEditorForm" );

    QDomElement rootElem = d->mInvisibleRootContainer->toDomElement( doc );
    QDomNodeList elemList = rootElem.childNodes();

    while ( !elemList.isEmpty() )
    {
      tabsElem.appendChild( elemList.at( 0 ) );
    }

    node.appendChild( tabsElem );
  }

  //// TODO: MAKE THIS MORE GENERIC, SO INDIVIDUALL WIDGETS CAN NOT ONLY SAVE STRINGS
  /// SEE QgsEditorWidgetFactory::writeConfig

  QDomElement widgetsElem = doc.createElement( "widgets" );

  QMap<QString, QgsEditorWidgetConfig >::ConstIterator configIt( d->mWidgetConfigs.constBegin() );

  while ( configIt != d->mWidgetConfigs.constEnd() )
  {
    if ( d->mFields.indexFromName( configIt.key() ) == -1 )
    {
      QDomElement widgetElem = doc.createElement( "widget" );
      widgetElem.setAttribute( "name", configIt.key() );
      // widgetElem.setAttribute( "notNull",  );

      QDomElement configElem = doc.createElement( "config" );
      widgetElem.appendChild( configElem );

      QgsEditorWidgetConfig::ConstIterator cfgIt( configIt.value().constBegin() );

      while ( cfgIt != configIt.value().constEnd() )
      {
        QDomElement optionElem = doc.createElement( "option" );
        optionElem.setAttribute( "key", cfgIt.key() );
        optionElem.setAttribute( "value", cfgIt.value().toString() );
        configElem.appendChild( optionElem );
        ++cfgIt;
      }

      widgetsElem.appendChild( widgetElem );
    }
    ++configIt;
  }

  node.appendChild( widgetsElem );

  //// END TODO
}

QgsAttributeEditorElement* QgsEditFormConfig::attributeEditorElementFromDomElement( QDomElement &elem, QgsAttributeEditorElement* parent )
{
  QgsAttributeEditorElement* newElement = nullptr;

  if ( elem.tagName() == "attributeEditorContainer" )
  {
    QgsAttributeEditorContainer* container = new QgsAttributeEditorContainer( elem.attribute( "name" ), parent );
    bool ok;
    int cc = elem.attribute( "columnCount" ).toInt( &ok );
    if ( !ok )
      cc = 0;
    container->setColumnCount( cc );

    bool isGroupBox = elem.attribute( "groupBox" ).toInt( &ok );
    if ( ok )
      container->setIsGroupBox( isGroupBox );
    else
      container->setIsGroupBox( parent );

    bool visibilityExpressionEnabled = elem.attribute( "visibilityExpressionEnabled" ).toInt( &ok );
    QgsOptionalExpression visibilityExpression;
    if ( ok )
    {
      visibilityExpression.setEnabled( visibilityExpressionEnabled );
      visibilityExpression.setData( QgsExpression( elem.attribute( "visibilityExpression" ) ) );
    }
    container->setVisibilityExpression( visibilityExpression );

    QDomNodeList childNodeList = elem.childNodes();

    for ( int i = 0; i < childNodeList.size(); i++ )
    {
      QDomElement childElem = childNodeList.at( i ).toElement();
      QgsAttributeEditorElement *myElem = attributeEditorElementFromDomElement( childElem, container );
      if ( myElem )
        container->addChildElement( myElem );
    }

    newElement = container;
  }
  else if ( elem.tagName() == "attributeEditorField" )
  {
    QString name = elem.attribute( "name" );
    int idx = d->mFields.lookupField( name );
    newElement = new QgsAttributeEditorField( name, idx, parent );
  }
  else if ( elem.tagName() == "attributeEditorRelation" )
  {
    // At this time, the relations are not loaded
    // So we only grab the id and delegate the rest to onRelationsLoaded()
    QString name = elem.attribute( "name" );
    QgsAttributeEditorRelation* relElement = new QgsAttributeEditorRelation( name, elem.attribute( "relation", "[None]" ), parent );
    relElement->setShowLinkButton( elem.attribute( "showLinkButton", "1" ).toInt() );
    relElement->setShowUnlinkButton( elem.attribute( "showUnlinkButton", "1" ).toInt() );
    newElement = relElement;
  }

  if ( elem.hasAttribute( "showLabel" ) )
    newElement->setShowLabel( elem.attribute( "showLabel" ).toInt() );
  else
    newElement->setShowLabel( true );

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

QgsAttributeEditorElement* QgsAttributeEditorContainer::clone( QgsAttributeEditorElement* parent ) const
{
  QgsAttributeEditorContainer* element = new QgsAttributeEditorContainer( name(), parent );

  Q_FOREACH ( QgsAttributeEditorElement* child, children() )
  {
    element->addChildElement( child->clone( element ) );
  }
  element->mIsGroupBox = mIsGroupBox;
  element->mColumnCount = mColumnCount;
  element->mVisibilityExpression = mVisibilityExpression;

  return element;
}

void QgsAttributeEditorContainer::saveConfiguration( QDomElement& elem ) const
{
  elem.setAttribute( "columnCount", mColumnCount );
  elem.setAttribute( "groupBox", mIsGroupBox ? 1 : 0 );
  elem.setAttribute( "visibilityExpressionEnabled", mVisibilityExpression.enabled() ? 1 : 0 );
  elem.setAttribute( "visibilityExpression", mVisibilityExpression->expression() );

  Q_FOREACH ( QgsAttributeEditorElement* child, mChildren )
  {
    QDomDocument doc = elem.ownerDocument();
    elem.appendChild( child->toDomElement( doc ) );
  }
}

QString QgsAttributeEditorContainer::typeIdentifier() const
{
  return "attributeEditorContainer";
}
