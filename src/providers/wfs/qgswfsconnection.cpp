#include "qgswfsconnection.h"

#include <QSettings>
#include <QStringList>

QgsWFSConnection::QgsWFSConnection(QObject *parent) :
    QObject(parent)
{
}


QStringList QgsWFSConnection::connectionList()
{
  QSettings settings;
  settings.beginGroup( "/Qgis/connections-wfs" );
  return settings.childGroups();
}

QString QgsWFSConnection::selectedConnection()
{
  QSettings settings;
  return settings.value( "/Qgis/connections-wfs/selected" ).toString();
}

void QgsWFSConnection::setSelectedConnection( QString name )
{
  QSettings settings;
  settings.setValue( "/Qgis/connections-wfs/selected", name );
}

void QgsWFSConnection::deleteConnection( QString name )
{
  QSettings settings;
  settings.remove( "/Qgis/connections-wfs/" + name );
}


// TODO:
// - get URI
// - get capabilities
