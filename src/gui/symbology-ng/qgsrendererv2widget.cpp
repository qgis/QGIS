#include "qgsrendererv2widget.h"


QgsRendererV2Widget::QgsRendererV2Widget( QgsVectorLayer* layer, QgsStyleV2* style )
    : QWidget(), mLayer( layer ), mStyle( style )
{
}



////////////

//#include <QAction>
#include "qgsfield.h"
#include <QMenu>

QgsRendererV2DataDefinedMenus::QgsRendererV2DataDefinedMenus( QMenu* menu, const QgsFieldMap& flds, QString rotationField, QString sizeScaleField )
    : QObject( menu ), mFlds( flds )
{
  mRotationMenu = new QMenu( tr( "Rotation field" ) );
  mSizeScaleMenu = new QMenu( tr( "Size scale field" ) );

  populateMenu( mRotationMenu, SLOT( rotationFieldSelected() ), rotationField );
  populateMenu( mSizeScaleMenu, SLOT( sizeScaleFieldSelected() ), sizeScaleField );

  menu->addMenu( mRotationMenu );
  menu->addMenu( mSizeScaleMenu );
}

void QgsRendererV2DataDefinedMenus::populateMenu( QMenu* menu, const char* slot, QString fieldName )
{
  QAction* aNo = menu->addAction( tr( "- no field -" ), this, slot );
  aNo->setCheckable( true );
  menu->addSeparator();

  bool hasField = false;
  //const QgsFieldMap& flds = mLayer->pendingFields();
  for ( QgsFieldMap::const_iterator it = mFlds.begin(); it != mFlds.end(); ++it )
  {
    const QgsField& fld = it.value();
    if ( fld.type() == QVariant::Int || fld.type() == QVariant::Double )
    {
      QAction* a = menu->addAction( fld.name(), this, slot );
      a->setCheckable( true );
      if ( fieldName == fld.name() )
      {
        a->setChecked( true );
        hasField = true;
      }
    }
  }

  if ( !hasField )
    aNo->setChecked( true );
}

void QgsRendererV2DataDefinedMenus::rotationFieldSelected()
{
  QObject* s = sender();
  if ( s == NULL )
    return;

  QAction* a = qobject_cast<QAction*>( s );
  if ( a == NULL )
    return;

  QString fldName = a->text();

  updateMenu( mRotationMenu, fldName );

  if ( fldName == tr( "- no field -" ) )
    fldName = QString();

  emit rotationFieldChanged( fldName );
}

void QgsRendererV2DataDefinedMenus::sizeScaleFieldSelected()
{
  QObject* s = sender();
  if ( s == NULL )
    return;

  QAction* a = qobject_cast<QAction*>( s );
  if ( a == NULL )
    return;

  QString fldName = a->text();

  updateMenu( mSizeScaleMenu, fldName );

  if ( fldName == tr( "- no field -" ) )
    fldName = QString();

  emit sizeScaleFieldChanged( fldName );
}

void QgsRendererV2DataDefinedMenus::updateMenu( QMenu* menu, QString fieldName )
{
  foreach( QAction* a, menu->actions() )
  {
    a->setChecked( a->text() == fieldName );
  }
}
