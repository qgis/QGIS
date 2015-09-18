#ifndef QGSPALGEOMETRY_H
#define QGSPALGEOMETRY_H

#include "qgsgeometry.h"
#include "qgspallabeling.h"
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
      mStrId = FID_TO_STRING( mId );
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

    const GEOSGeometry* getGeosGeometry() override
    {
      return mG;
    }
    void releaseGeosGeometry( const GEOSGeometry* /*geom*/ ) override
    {
      // nothing here - we'll delete the geometry in destructor
    }

    QString strId() { return mStrId; }
    QString text() { return mText; }

    /** Returns the text component corresponding to a specified label part
     * @param partId Set to -1 for labels which are not broken into parts (eg, non-curved labels), or the required
     * part index for labels which are broken into parts (curved labels)
     * @note added in QGIS 2.10
     */
    QString text( int partId ) const
    {
      if ( partId == -1 )
        return mText;
      else
        return mClusters.at( partId );
    }

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
      double mapScale = xform->mapUnitsPerPixel();
      double labelHeight = mapScale * fm->height() / fontScale;

      // mLetterSpacing/mWordSpacing = 0.0 is default for non-curved labels
      // (non-curved spacings handled by Qt in QgsPalLayerSettings/QgsPalLabeling)
      qreal charWidth;
      qreal wordSpaceFix;

      //split string by valid grapheme boundaries - required for certain scripts (see #6883)
      mClusters = QgsPalLabeling::splitToGraphemes( mText );

      mInfo = new pal::LabelInfo( mClusters.count(), labelHeight, maxinangle, maxoutangle );
      for ( int i = 0; i < mClusters.count(); i++ )
      {
        //doesn't appear to be used anywhere:
        //mInfo->char_info[i].chr = textClusters[i].unicode();

        // reconstruct how Qt creates word spacing, then adjust per individual stored character
        // this will allow PAL to create each candidate width = character width + correct spacing
        charWidth = fm->width( mClusters[i] );
        if ( mCurvedLabeling )
        {
          wordSpaceFix = qreal( 0.0 );
          if ( mClusters[i] == QString( " " ) )
          {
            // word spacing only gets added once at end of consecutive run of spaces, see QTextEngine::shapeText()
            int nxt = i + 1;
            wordSpaceFix = ( nxt < mClusters.count() && mClusters[nxt] != QString( " " ) ) ? mWordSpacing : qreal( 0.0 );
          }
          // this workaround only works for clusters with a single character. Not sure how it should be handled
          // with multi-character clusters.
          if ( mClusters[i].length() == 1 &&
               !qgsDoubleNear( fm->width( QString( mClusters[i].at( 0 ) ) ), fm->width( mClusters[i].at( 0 ) ) + mLetterSpacing ) )
          {
            // word spacing applied when it shouldn't be
            wordSpaceFix -= mWordSpacing;
          }

          charWidth = fm->width( QString( mClusters[i] ) ) + wordSpaceFix;
        }

        double labelWidth = mapScale * charWidth / fontScale;
        mInfo->char_info[i].width = labelWidth;
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
    QStringList mClusters;
    QString mStrId;
    QgsFeatureId mId;
    LabelInfo* mInfo;
    bool mIsDiagram;
    bool mIsPinned;
    QFont mDefinedFont;
    QFontMetricsF* mFontMetrics;
    qreal mLetterSpacing; // for use with curved labels
    qreal mWordSpacing; // for use with curved labels
    bool mCurvedLabeling; // whether the geometry is to be used for curved labeling placement
    /** Stores attribute values for data defined properties*/
    QMap< QgsPalLayerSettings::DataDefinedProperties, QVariant > mDataDefinedValues;

    /** Stores attribute values for diagram rendering*/
    QgsAttributes mDiagramAttributes;

    QString mDxfLayer;
};

#endif //QGSPALGEOMETRY_H
