#ifndef PALLABELING_H
#define PALLABELING_H

class QPainter;
class QgsMapRenderer;
class QgsRectangle;
class QgsCoordinateTransform;

#include <QString>
#include <QFont>
#include <QColor>
#include <QList>
#include <QRectF>

namespace pal
{
  class Pal;
  class Layer;
  class LabelPosition;
}

class QgsMapToPixel;
class QgsFeature;
#include "qgspoint.h"

#include "qgsvectorlayer.h" // definition of QgsLabelingEngineInterface

class MyLabel;

class LayerSettings
{
  public:
    LayerSettings();
    LayerSettings( const LayerSettings& s );
    ~LayerSettings();

    enum Placement
    {
      AroundPoint, // Point / Polygon
      OverPoint, // Point / Polygon
      Line, // Line / Polygon
      Curved, // Line
      Horizontal, // Polygon
      Free // Polygon
    };

    enum LinePlacementFlags
    {
      OnLine    = 1,
      AboveLine = 2,
      BelowLine = 4,
      MapOrientation = 8
    };

    QString fieldName;
    Placement placement;
    unsigned int placementFlags;
    QFont textFont;
    QColor textColor;
    bool enabled;
    int priority; // 0 = low, 10 = high
    bool obstacle; // whether it's an obstacle
    double dist; // distance from the feature (in pixels)
    int scaleMin, scaleMax; // disabled if both are zero
    int bufferSize;
    QColor bufferColor;
    bool labelPerPart; // whether to label every feature's part or only the biggest one
    bool mergeLines;

    // called from register feature hook
    void calculateLabelSize( QString text, double& labelX, double& labelY );

    // implementation of register feature hook
    void registerFeature( QgsFeature& f );

    void readFromLayer( QgsVectorLayer* layer );
    void writeToLayer( QgsVectorLayer* layer );

    // temporary stuff: set when layer gets prepared
    pal::Layer* palLayer;
    int fieldIndex;
    QFontMetrics* fontMetrics;
    int fontBaseline;
    const QgsMapToPixel* xform;
    const QgsCoordinateTransform* ct;
    QgsPoint ptZero, ptOne;
    QList<MyLabel*> geometries;
};

class LabelCandidate
{
  public:
    LabelCandidate( QRectF r, double c ): rect( r ), cost( c ) {}

    QRectF rect;
    double cost;
};

class PalLabeling : public QgsLabelingEngineInterface
{
  public:
    PalLabeling( QgsMapRenderer* renderer );
    ~PalLabeling();

    LayerSettings& layer( const char* layerName );

    void doLabeling( QPainter* painter, QgsRectangle extent );

    void numCandidatePositions( int& candPoint, int& candLine, int& candPolygon );
    void setNumCandidatePositions( int candPoint, int candLine, int candPolygon );

    enum Search { Chain, Popmusic_Tabu, Popmusic_Chain, Popmusic_Tabu_Chain, Falp };

    void setSearchMethod( Search s );
    Search searchMethod() const;

    bool isShowingCandidates() const { return mShowingCandidates; }
    void setShowingCandidates( bool showing ) { mShowingCandidates = showing; }
    const QList<LabelCandidate>& candidates() { return mCandidates; }

    bool isShowingAllLabels() const { return mShowingAllLabels; }
    void setShowingAllLabels( bool showing ) { mShowingAllLabels = showing; }

    // implemented methods from labeling engine interface

    //! hook called when drawing layer before issuing select()
    virtual int prepareLayer( QgsVectorLayer* layer, int& attrIndex );
    //! hook called when drawing for every feature in a layer
    virtual void registerFeature( QgsVectorLayer* layer, QgsFeature& feat );


    void drawLabelCandidateRect( pal::LabelPosition* lp, QPainter* painter, const QgsMapToPixel* xform );
    void drawLabel( pal::LabelPosition* label, QPainter* painter, const QgsMapToPixel* xform, bool drawBuffer = false );
    static void drawLabelBuffer( QPainter* p, QString text, const QFont& font, int size, QColor color );

  protected:

    void initPal();

  protected:
    // temporary hashtable of layer settings, being filled during labeling, cleared once labeling's done
    QHash<QgsVectorLayer*, LayerSettings> mActiveLayers;
    LayerSettings mInvalidLayerSettings;

    QgsMapRenderer* mMapRenderer;
    int mCandPoint, mCandLine, mCandPolygon;
    Search mSearch;

    pal::Pal* mPal;

    // list of candidates from last labeling
    QList<LabelCandidate> mCandidates;
    bool mShowingCandidates;

    bool mShowingAllLabels; // whether to avoid collisions or not
};

#endif // PALLABELING_H
