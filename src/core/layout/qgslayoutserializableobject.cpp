/***************************************************************************
                          qgslayoutserializableobject.cpp
                          -------------------------------
    begin                : July 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutserializableobject.h"
#include "qgsreadwritecontext.h"
#include "qgslayout.h"
#include "qgsproject.h"

///@cond PRIVATE
class QgsLayoutSerializableObjectUndoCommand: public QgsAbstractLayoutUndoCommand
{
  public:

    QgsLayoutSerializableObjectUndoCommand( QgsLayoutSerializableObject *object, const QString &text, int id, QUndoCommand *parent SIP_TRANSFERTHIS = nullptr )
      : QgsAbstractLayoutUndoCommand( text, id, parent )
      , mObject( object )
    {}

    bool mergeWith( const QUndoCommand *command ) override
    {
      if ( command->id() == 0 )
        return false;

      const QgsLayoutSerializableObjectUndoCommand *c = dynamic_cast<const QgsLayoutSerializableObjectUndoCommand *>( command );
      if ( !c )
      {
        return false;
      }

      if ( mObject->stringType() != c->mObject->stringType() )
        return false;

      setAfterState( c->afterState() );
      return true;
    }

  protected:

    void saveState( QDomDocument &stateDoc ) const override
    {
      stateDoc.clear();
      QDomElement documentElement = stateDoc.createElement( QStringLiteral( "UndoState" ) );
      mObject->writeXml( documentElement, stateDoc, QgsReadWriteContext() );
      stateDoc.appendChild( documentElement );
    }
    void restoreState( QDomDocument &stateDoc ) override
    {
      if ( !mObject )
      {
        return;
      }

      mObject->readXml( stateDoc.documentElement().firstChild().toElement(), stateDoc, QgsReadWriteContext() );
      mObject->layout()->project()->setDirty( true );
    }

  private:

    QgsLayoutSerializableObject *mObject = nullptr;
};
///@endcond


QgsAbstractLayoutUndoCommand *QgsLayoutSerializableObject::createCommand( const QString &text, int id, QUndoCommand *parent )
{
  return new QgsLayoutSerializableObjectUndoCommand( this, text, id, parent );
}
