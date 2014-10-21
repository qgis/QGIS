#ifndef QGSPALGEOMETRY_H
#define QGSPALGEOMETRY_H

#include "qgsgeometry.h"
#include <pal/feature.h>
#include <pal/palgeometry.h>

using namespace pal;

class QgsPalGeometry : public PalGeometry
{
  public:
    QgsPalGeometry( QgsFeatureId id, QString text, GEOSGeometry* g,
                    qreal ltrSpacing = 0.0, qreal wordSpacing = 0.0, bool curvedLabeling = false )
        : mG( g )
        , mText( text )
        , mId( id )
        , mInfo( NULL )
        , mIsDiagram( false )
        , mIsPinned( false )
        , mFontMetrics( NULL )
        , mLetterSpacing( ltrSpacing )
        , mWordSpacing( wordSpacing )
        , mCurvedLabeling( curvedLabeling )
    {
      mStrId = FID_TO_STRING( mId ).toAscii();
      mDefinedFont = QFont();
    }

    ~QgsPalGeometry()
    {
      if ( mG )
        GEOSGeom_destroy_r( QgsGeometry::getGEOSHandler(), mG );
      delete mInfo;
      delete mFontMetrics;
    }

    // getGeosGeometry + releaseGeosGeometry is called twice: once when adding, second time when labeling

    const GEOSGeometry* getGeosGeometry()
    {
      return mG;
    }
    void releaseGeosGeometry( const GEOSGeometry* /*geom*/ )
    {
      // nothing here - we'll delete the geometry in destructor
    }

    const char* strId() { return mStrId.data(); }
    QString text() { return mText; }

    pal::LabelInfo* info( QFontMetricsF* fm, const QgsMapToPixel* xform, double fontScale, double maxinangle, double maxoutangle )
    {
      if ( mInfo )
        return mInfo;

      mFontMetrics = new QFontMetricsF( *fm ); // duplicate metrics for when drawing label

      // max angle between curved label characters (20.0/-20.0 was default in QGIS <= 1.8)
      if ( maxinangle < 20.0 )
        maxinangle = 20.0;
      if ( 60.0 < maxinangle )
        maxinangle = 60.0;
      if ( maxoutangle > -20.0 )
        maxoutangle = -20.0;
      if ( -95.0 > maxoutangle )
        maxoutangle = -95.0;

      // create label info!
      QgsPoint ptZero = xform->toMapCoordinates( 0, 0 );
      QgsPoint ptSize = xform->toMapCoordinatesF( 0.0, -fm->height() / fontScale );

      // mLetterSpacing/mWordSpacing = 0.0 is default for non-curved labels
      // (non-curved spacings handled by Qt in QgsPalLayerSettings/QgsPalLabeling)
      qreal charWidth;
      qreal wordSpaceFix;
      mInfo = new pal::LabelInfo( mText.count(), ptSize.y() - ptZero.y(), maxinangle, maxoutangle );
      for ( int i = 0; i < mText.count(); i++ )
      {
        mInfo->char_info[i].chr = mText[i].unicode();

        // reconstruct how Qt creates word spacing, then adjust per individual stored character
        // this will allow PAL to create each candidate width = character width + correct spacing
        charWidth = fm->width( mText[i] );
        if ( mCurvedLabeling )
        {
          wordSpaceFix = qreal( 0.0 );
          if ( mText[i] == QString( " " )[0] )
          {
            // word spacing only gets added once at end of consecutive run of spaces, see QTextEngine::shapeText()
            int nxt = i + 1;
            wordSpaceFix = ( nxt < mText.count() && mText[nxt] != QString( " " )[0] ) ? mWordSpacing : qreal( 0.0 );
          }
          if ( fm->width( QString( mText[i] ) ) - fm->width( mText[i] ) - mLetterSpacing != qreal( 0.0 ) )
          {
            // word spacing applied when it shouldn't be
            wordSpaceFix -= mWordSpacing;
          }
          charWidth = fm->width( QString( mText[i] ) ) + wordSpaceFix;
        }

        ptSize = xform->toMapCoordinatesF((( double ) charWidth ) / fontScale, 0.0 );
        mInfo->char_info[i].width = ptSize.x() - ptZero.x();
      }
      return mInfo;
    }

    const QMap< QgsPalLayerSettings::DataDefinedProperties, QVariant >& dataDefinedValues() const { return mDataDefinedValues; }
    void addDataDefinedValue( QgsPalLayerSettings::DataDefinedProperties p, QVariant v ) { mDataDefinedValues.insert( p, v ); }

    void setIsDiagram( bool d ) { mIsDiagram = d; }
    bool isDiagram() const { return mIsDiagram; }

    void setIsPinned( bool f ) { mIsPinned = f; }
    bool isPinned() const { return mIsPinned; }

    void setDefinedFont( QFont f ) { mDefinedFont = QFont( f ); }
    QFont definedFont() { return mDefinedFont; }

    QFontMetricsF* getLabelFontMetrics() { return mFontMetrics; }

    void setDiagramAttributes( const QgsAttributes& attrs ) { mDiagramAttributes = attrs; }
    const QgsAttributes& diagramAttributes() { return mDiagramAttributes; }

    void feature( QgsFeature& feature )
    {
      feature.setFeatureId( mId );
      feature.setAttributes( mDiagramAttributes );
      feature.setValid( true );
    }

    void setDxfLayer( QString dxfLayer ) { mDxfLayer = dxfLayer; }
    QString dxfLayer() const { return mDxfLayer; }

  protected:
    GEOSGeometry* mG;
    QString mText;
    QByteArray mStrId;
    QgsFeatureId mId;
    LabelInfo* mInfo;
    bool mIsDiagram;
    bool mIsPinned;
    QFont mDefinedFont;
    QFontMetricsF* mFontMetrics;
    qreal mLetterSpacing; // for use with curved labels
    qreal mWordSpacing; // for use with curved labels
    bool mCurvedLabeling; // whether the geometry is to be used for curved labeling placement
    /**Stores attribute values for data defined properties*/
    QMap< QgsPalLayerSettings::DataDefinedProperties, QVariant > mDataDefinedValues;

    /**Stores attribute values for diagram rendering*/
    QgsAttributes mDiagramAttributes;

    QString mDxfLayer;
};

#endif //QGSPALGEOMETRY_H
