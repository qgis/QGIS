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
#include "qgsproject.h"

QgsEditFormConfig::QgsEditFormConfig( QObject* parent )
    : QObject( parent )
    , mEditorLayout( GeneratedLayout )
    , mInitCodeSource( CodeSourceNone )
    , mSuppressForm( SuppressDefault )
{
  connect( QgsProject::instance()->relationManager(), SIGNAL( relationsLoaded() ), this, SLOT( onRelationsLoaded() ) );
}

QString QgsEditFormConfig::widgetType( int fieldIdx ) const
{
  if ( fieldIdx < 0 || fieldIdx >= mFields.count() )
    return "TextEdit";

  return mEditorWidgetV2Types.value( mFields.at( fieldIdx ).name(), "TextEdit" );
}

QString QgsEditFormConfig::widgetType( const QString& fieldName ) const
{
  return mEditorWidgetV2Types.value( fieldName, "TextEdit" );
}

QgsEditorWidgetConfig QgsEditFormConfig::widgetConfig( int fieldIdx ) const
{
  if ( fieldIdx < 0 || fieldIdx >= mFields.count() )
    return QgsEditorWidgetConfig();

  return mWidgetConfigs.value( mFields.at( fieldIdx ).name() );
}

QgsEditorWidgetConfig QgsEditFormConfig::widgetConfig( const QString& widgetName ) const
{
  return mWidgetConfigs.value( widgetName );
}

void QgsEditFormConfig::setFields( const QgsFields& fields )
{
  mFields = fields;
}

void QgsEditFormConfig::setWidgetType( int attrIdx, const QString& widgetType )
{
  if ( attrIdx >= 0 && attrIdx < mFields.count() )
    mEditorWidgetV2Types[ mFields.at( attrIdx ).name()] = widgetType;
}

void QgsEditFormConfig::setWidgetConfig( int attrIdx, const QgsEditorWidgetConfig& config )
{
  if ( attrIdx >= 0 && attrIdx < mFields.count() )
    mWidgetConfigs[ mFields.at( attrIdx ).name()] = config;
}

void QgsEditFormConfig::setWidgetConfig( const QString& widgetName, const QgsEditorWidgetConfig& config )
{
  mWidgetConfigs[widgetName] = config;
}

bool QgsEditFormConfig::removeWidgetConfig( const QString &widgetName )
{
  return mWidgetConfigs.remove( widgetName ) != 0;
}

bool QgsEditFormConfig::removeWidgetConfig( int fieldIdx )
{
  if ( fieldIdx < 0 || fieldIdx >= mFields.count() )
    return false;

  return mWidgetConfigs.remove( mFields.at( fieldIdx ).name() );
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
  mUiFormPath = ui;
}

bool QgsEditFormConfig::readOnly( int idx ) const
{
  if ( idx >= 0 && idx < mFields.count() )
  {
    if ( mFields.fieldOrigin( idx ) == QgsFields::OriginJoin
         || mFields.fieldOrigin( idx ) == QgsFields::OriginExpression )
      return true;
    return !mFieldEditables.value( mFields.at( idx ).name(), true );
  }
  else
    return false;
}

bool QgsEditFormConfig::labelOnTop( int idx ) const
{
  if ( idx >= 0 && idx < mFields.count() )
    return mLabelOnTop.value( mFields.at( idx ).name(), false );
  else
    return false;
}

QString QgsEditFormConfig::expression( int idx ) const
{
  QString expr;

  if ( idx >= 0 && idx < mFields.count() )
    expr = mConstraints.value( mFields.at( idx ).name(), QString() );

  return expr;
}

void QgsEditFormConfig::setExpression( int idx, const QString& str )
{
  if ( idx >= 0 && idx < mFields.count() )
    mConstraints[ mFields.at( idx ).name()] = str;
}

QString QgsEditFormConfig::expressionDescription( int idx ) const
{
  QString description;

  if ( idx >= 0 && idx < mFields.count() )
    description = mConstraintsDescription[ mFields.at( idx ).name()];

  return description;
}

void QgsEditFormConfig::setExpressionDescription( int idx, const QString &descr )
{
  if ( idx >= 0 && idx < mFields.count() )
    mConstraintsDescription[ mFields.at( idx ).name()] = descr;
}

bool QgsEditFormConfig::notNull( int idx ) const
{
  if ( idx >= 0 && idx < mFields.count() )
    return mNotNull.value( mFields.at( idx ).name(), false );
  else
    return false;
}

void QgsEditFormConfig::setReadOnly( int idx, bool readOnly )
{
  if ( idx >= 0 && idx < mFields.count() )
    mFieldEditables[ mFields.at( idx ).name()] = !readOnly;
}

void QgsEditFormConfig::setLabelOnTop( int idx, bool onTop )
{
  if ( idx >= 0 && idx < mFields.count() )
    mLabelOnTop[ mFields.at( idx ).name()] = onTop;
}

void QgsEditFormConfig::setNotNull( int idx, bool notnull )
{
  if ( idx >= 0 && idx < mFields.count() )
    mNotNull[ mFields.at( idx ).name()] = notnull;
}

void QgsEditFormConfig::readXml( const QDomNode& node )
{
  QDomNode editFormNode = node.namedItem( "editform" );
  if ( !editFormNode.isNull() )
  {
    QDomElement e = editFormNode.toElement();
    mUiFormPath = QgsProject::instance()->readPath( e.text() );
  }

  QDomNode editFormInitNode = node.namedItem( "editforminit" );
  if ( !editFormInitNode.isNull() )
  {
    mInitFunction = editFormInitNode.toElement().text();
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
  int dotPos = mInitFunction.lastIndexOf( '.' );
  if ( dotPos >= 0 ) // It's a module
  {
    setInitCodeSource( QgsEditFormConfig::CodeSourceDialog );
    setInitCode( QString( "from %1 import %2\n" ).arg( mInitFunction.left( dotPos ), mInitFunction.mid( dotPos + 1 ) ) );
    setInitFunction( mInitFunction.mid( dotPos + 1 ) );
  }

  QDomNode editFormInitFilePathNode = node.namedItem( "editforminitfilepath" );
  if ( !editFormInitFilePathNode.isNull() || ( !editFormInitFilePathNode.isNull() && !editFormInitFilePathNode.toElement().text().isEmpty() ) )
  {
    setInitFilePath( QgsProject::instance()->readPath( editFormInitFilePathNode.toElement().text() ) );
  }

  QDomNode fFSuppNode = node.namedItem( "featformsuppress" );
  if ( fFSuppNode.isNull() )
  {
    mSuppressForm = QgsEditFormConfig::SuppressDefault;
  }
  else
  {
    QDomElement e = fFSuppNode.toElement();
    mSuppressForm = static_cast< QgsEditFormConfig::FeatureFormSuppress >( e.text().toInt() );
  }

  // tab display
  QDomNode editorLayoutNode = node.namedItem( "editorlayout" );
  if ( editorLayoutNode.isNull() )
  {
    mEditorLayout = QgsEditFormConfig::GeneratedLayout;
  }
  else
  {
    if ( editorLayoutNode.toElement().text() == "uifilelayout" )
    {
      mEditorLayout = QgsEditFormConfig::UiFileLayout;
    }
    else if ( editorLayoutNode.toElement().text() == "tablayout" )
    {
      mEditorLayout = QgsEditFormConfig::TabLayout;
    }
    else
    {
      mEditorLayout = QgsEditFormConfig::GeneratedLayout;
    }
  }

  // tabs and groups display info
  clearTabs();
  QDomNode attributeEditorFormNode = node.namedItem( "attributeEditorForm" );
  QDomNodeList attributeEditorFormNodeList = attributeEditorFormNode.toElement().childNodes();

  for ( int i = 0; i < attributeEditorFormNodeList.size(); i++ )
  {
    QDomElement elem = attributeEditorFormNodeList.at( i ).toElement();

    QgsAttributeEditorElement *attributeEditorWidget = attributeEditorElementFromDomElement( elem, this );
    addTab( attributeEditorWidget );
  }


  //// TODO: MAKE THIS MORE GENERIC, SO INDIVIDUALL WIDGETS CAN NOT ONLY SAVE STRINGS
  /// SEE QgsEditorWidgetFactory::writeConfig

  QDomElement widgetsElem = node.namedItem( "widgets" ).toElement();

  QDomNodeList widgetConfigsElems = widgetsElem.childNodes();

  for ( int i = 0; i < widgetConfigsElems.size(); ++i )
  {
    QgsEditorWidgetConfig cfg;

    QDomElement wdgElem = widgetConfigsElems.at( i ).toElement();

    QDomElement cfgElem = wdgElem.namedItem( "config" ).toElement();

    for ( int j = 0; j < cfgElem.attributes().size(); ++j )
    {
      QDomAttr attr = cfgElem.attributes().item( j ).toAttr();
      cfg.insert( attr.name(), attr.value() );
    }

    QDomNodeList optionElements = cfgElem.elementsByTagName( "option" );
    for ( int j = 0; j < optionElements.size(); ++j )
    {
      QString key = optionElements.at( j ).toElement().attribute( "key" );
      QString value = optionElements.at( j ).toElement().attribute( "value" );
      cfg.insert( key, value );
    }

    setWidgetConfig( wdgElem.attribute( "name" ), cfg );
  }
  //// END TODO
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
  if ( tabs().size() > 0 )
  {
    QDomElement tabsElem = doc.createElement( "attributeEditorForm" );

    for ( QList< QgsAttributeEditorElement* >::const_iterator it = mAttributeEditorElements.constBegin(); it != mAttributeEditorElements.constEnd(); ++it )
    {
      QDomElement attributeEditorWidgetElem = ( *it )->toDomElement( doc );
      tabsElem.appendChild( attributeEditorWidgetElem );
    }

    node.appendChild( tabsElem );
  }

  //// TODO: MAKE THIS MORE GENERIC, SO INDIVIDUALL WIDGETS CAN NOT ONLY SAVE STRINGS
  /// SEE QgsEditorWidgetFactory::writeConfig

  QDomElement widgetsElem = doc.createElement( "widgets" );

  QMap<QString, QgsEditorWidgetConfig >::ConstIterator configIt( mWidgetConfigs.constBegin() );

  while ( configIt != mWidgetConfigs.constEnd() )
  {
    if ( mFields.indexFromName( configIt.key() ) == -1 )
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

QgsAttributeEditorElement* QgsEditFormConfig::attributeEditorElementFromDomElement( QDomElement &elem, QObject* parent )
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
      container->setIsGroupBox( qobject_cast<QgsAttributeEditorContainer*>( parent ) );

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
    int idx = mFields.fieldNameIndex( name );
    newElement = new QgsAttributeEditorField( name, idx, parent );
  }
  else if ( elem.tagName() == "attributeEditorRelation" )
  {
    // At this time, the relations are not loaded
    // So we only grab the id and delegate the rest to onRelationsLoaded()
    QString name = elem.attribute( "name" );
    newElement = new QgsAttributeEditorRelation( name, elem.attribute( "relation", "[None]" ), parent );
  }
  return newElement;
}

void QgsEditFormConfig::onRelationsLoaded()
{
  Q_FOREACH ( QgsAttributeEditorElement* elem, mAttributeEditorElements )
  {
    if ( elem->type() == QgsAttributeEditorElement::AeTypeContainer )
    {
      QgsAttributeEditorContainer* cont = dynamic_cast< QgsAttributeEditorContainer* >( elem );
      if ( !cont )
        continue;

      QList<QgsAttributeEditorElement*> relations = cont->findElements( QgsAttributeEditorElement::AeTypeRelation );
      Q_FOREACH ( QgsAttributeEditorElement* relElem, relations )
      {
        QgsAttributeEditorRelation* rel = dynamic_cast< QgsAttributeEditorRelation* >( relElem );
        if ( !rel )
          continue;

        rel->init( QgsProject::instance()->relationManager() );
      }
    }
  }
}

int QgsAttributeEditorContainer::columnCount() const
{
  return mColumnCount;
}

void QgsAttributeEditorContainer::setColumnCount( int columnCount )
{
  mColumnCount = columnCount;
}
