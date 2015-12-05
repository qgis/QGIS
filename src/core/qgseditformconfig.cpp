#include "qgseditformconfig.h"
#include "qgsproject.h"

QgsEditFormConfig::QgsEditFormConfig( QObject* parent )
    : QObject( parent )
    , mEditorLayout( GeneratedLayout )
    , mInitCodeSource( CodeSourceNone )
    , mFeatureFormSuppress( SuppressDefault )
{
  connect( QgsProject::instance()->relationManager(), SIGNAL( relationsLoaded() ), this, SLOT( onRelationsLoaded() ) );
}

QString QgsEditFormConfig::widgetType( int fieldIdx ) const
{
  if ( fieldIdx < 0 || fieldIdx >= mFields.count() )
    return "TextEdit";

  return mEditorWidgetV2Types.value( mFields[fieldIdx].name(), "TextEdit" );
}

QString QgsEditFormConfig::widgetType( const QString& fieldName ) const
{
  return mEditorWidgetV2Types.value( fieldName, "TextEdit" );
}

QgsEditorWidgetConfig QgsEditFormConfig::widgetConfig( int fieldIdx ) const
{
  if ( fieldIdx < 0 || fieldIdx >= mFields.count() )
    return QgsEditorWidgetConfig();

  return mEditorWidgetV2Configs.value( mFields[fieldIdx].name() );
}

QgsEditorWidgetConfig QgsEditFormConfig::widgetConfig( const QString& fieldName ) const
{
  return mEditorWidgetV2Configs.value( fieldName );
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
    mEditorWidgetV2Configs[ mFields.at( attrIdx ).name()] = config;
}

QString QgsEditFormConfig::uiForm() const
{
  return mEditForm;
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
  mEditForm = ui;
}

bool QgsEditFormConfig::readOnly( int idx )
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

bool QgsEditFormConfig::labelOnTop( int idx )
{
  if ( idx >= 0 && idx < mFields.count() )
    return mLabelOnTop.value( mFields.at( idx ).name(), false );
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
