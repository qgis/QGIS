#ifndef QGSLAYERTREEUTILS_H
#define QGSLAYERTREEUTILS_H

#include <qnamespace.h>
#include <QList>

class QDomElement;

class QgsLayerTreeGroup;
class QgsLayerTreeLayer;

class CORE_EXPORT QgsLayerTreeUtils
{
public:

  // return a new instance - or null on error
  static bool readOldLegend(QgsLayerTreeGroup* root, const QDomElement& legendElem);

  static QString checkStateToXml(Qt::CheckState state);
  static Qt::CheckState checkStateFromXml(QString txt);

  static bool layersEditable( const QList<QgsLayerTreeLayer*>& layerNodes );
  static bool layersModified( const QList<QgsLayerTreeLayer*>& layerNodes );


protected:
  static void addLegendGroupToTreeWidget( const QDomElement& groupElem, QgsLayerTreeGroup* parent );
  static void addLegendLayerToTreeWidget( const QDomElement& layerElem, QgsLayerTreeGroup* parent );

};

#endif // QGSLAYERTREEUTILS_H
