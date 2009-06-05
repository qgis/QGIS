#ifndef PALLABELING_H
#define PALLABELING_H

class QPainter;
class QgsMapCanvas;

#include <QString>
#include <QFont>
#include <QColor>

namespace pal
{
  class Pal;
}

class PalLabeling
{
public:
    PalLabeling(QgsMapCanvas* mapCanvas);

    enum Placement
    {
      AroundPoint, // Point / Polygon
      OnLine, // Line / Polygon
      AroundLine, // Line / Polygon
      Horizontal, // Polygon
      Free // Polygon
    };

    struct LayerSettings
    {
      //LayerSettings()
      QString layerId;
      QString fieldName;
      Placement placement;
      QFont textFont;
      QColor textColor;
      bool enabled;
    };

    void doLabeling(QPainter* painter);

    void addLayer(LayerSettings layerSettings);

    void removeLayer(QString layerId);

    LayerSettings layer(QString layerId);

protected:
    int prepareLayer(pal::Pal& pal, const LayerSettings& lyr);

protected:
    QList<LayerSettings> mLayers;
    QgsMapCanvas* mMapCanvas;
};

#endif // PALLABELING_H
