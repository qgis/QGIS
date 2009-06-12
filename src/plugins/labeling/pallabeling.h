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
  class Layer;
}

class QgsMapToPixel;
class QgsFeature;
#include "qgspoint.h"


class LayerSettings
{
public:
  LayerSettings();
  ~LayerSettings();

  enum Placement
  {
    AroundPoint, // Point / Polygon
    OnLine, // Line / Polygon
    AroundLine, // Line / Polygon
    Horizontal, // Polygon
    Free // Polygon
  };

  QString layerId;
  QString fieldName;
  Placement placement;
  QFont textFont;
  QColor textColor;
  bool enabled;
  int priority; // 0 = low, 10 = high
  bool obstacle; // whether it's an obstacle

  // called from register feature hook
  void calculateLabelSize(QString text, double& labelX, double& labelY);

  // implementation of register feature hook
  void registerFeature(QgsFeature& f);

  // temporary stuff: set when layer gets prepared
  pal::Layer* palLayer;
  int fieldIndex;
  QFontMetrics* fontMetrics;
  int fontBaseline;
  const QgsMapToPixel* xform;
  QgsPoint ptZero;
};

class PalLabeling
{
public:
    PalLabeling(QgsMapCanvas* mapCanvas);
    ~PalLabeling();

    void doLabeling(QPainter* painter);

    void addLayer(LayerSettings layerSettings);

    void removeLayer(QString layerId);

    LayerSettings layer(QString layerId);

    void numCandidatePositions(int& candPoint, int& candLine, int& candPolygon);
    void setNumCandidatePositions(int candPoint, int candLine, int candPolygon);

    enum Search { Chain, Popmusic_Tabu, Popmusic_Chain, Popmusic_Tabu_Chain };

    void setSearchMethod(Search s);
    Search searchMethod() const;


    //! hook called when drawing layer before issuing select()
    static int prepareLayerHook(void* context, void* layerContext, int& attrIndex);
    //! hook called when drawing for every feature in a layer
    static void registerFeatureHook(QgsFeature& f, void* layerContext);

protected:

    void initPal();

protected:
    QList<LayerSettings> mLayers;
    QgsMapCanvas* mMapCanvas;
    int mCandPoint, mCandLine, mCandPolygon;
    Search mSearch;

    pal::Pal* mPal;
};

#endif // PALLABELING_H
