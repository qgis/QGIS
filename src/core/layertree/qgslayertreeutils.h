#ifndef QGSLAYERTREEUTILS_H
#define QGSLAYERTREEUTILS_H

#include <qnamespace.h>

class QDomElement;

class QgsLayerTreeGroup;

class CORE_EXPORT QgsLayerTreeUtils
{
public:

  // return a new instance - or null on error
  static QgsLayerTreeGroup* readOldLegend(const QDomElement& legendElem);

  static QString checkStateToXml(Qt::CheckState state);
  static Qt::CheckState checkStateFromXml(QString txt);



protected:
  static void addLegendGroupToTreeWidget( const QDomElement& groupElem, QgsLayerTreeGroup* parent );
  static void addLegendLayerToTreeWidget( const QDomElement& layerElem, QgsLayerTreeGroup* parent );

};

#endif // QGSLAYERTREEUTILS_H
