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
      int priority; // 0 = low, 10 = high
      bool obstacle; // whether it's an obstacle
    };

    void doLabeling(QPainter* painter);

    void addLayer(LayerSettings layerSettings);

    void removeLayer(QString layerId);

    LayerSettings layer(QString layerId);

    void numCandidatePositions(int& candPoint, int& candLine, int& candPolygon);
    void setNumCandidatePositions(int candPoint, int candLine, int candPolygon);

    enum Search { Chain, Popmusic_Tabu, Popmusic_Chain, Popmusic_Tabu_Chain };

    void setSearchMethod(Search s);
    Search searchMethod() const;

protected:
    int prepareLayer(pal::Pal& pal, const LayerSettings& lyr);

protected:
    QList<LayerSettings> mLayers;
    QgsMapCanvas* mMapCanvas;
    int mCandPoint, mCandLine, mCandPolygon;
    Search mSearch;
};

#endif // PALLABELING_H
