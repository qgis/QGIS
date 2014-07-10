#include "qgslegendsettings.h"

QgsLegendSettings::QgsLegendSettings()
    : mTitle( QObject::tr( "Legend" ) )
    , mTitleAlignment( Qt::AlignLeft )
    , mWrapChar( "" )
    , mFontColor( QColor( 0, 0, 0 ) )
    , mBoxSpace( 2 )
    , mSymbolSize( 7, 4 )
    , mWmsLegendSize( 50, 25 )
    , mLineSpacing( 1.5 )
    , mColumnSpace( 2 )
    , mColumnCount( 1 )
    , mSplitLayer( false )
    , mEqualColumnWidth( false )
    , mMmPerMapUnit( 1 )
    , mUseAdvancedEffects( true )
{
  rstyle( QgsComposerLegendStyle::Title ).setMargin( QgsComposerLegendStyle::Bottom, 2 );
  rstyle( QgsComposerLegendStyle::Group ).setMargin( QgsComposerLegendStyle::Top, 2 );
  rstyle( QgsComposerLegendStyle::Subgroup ).setMargin( QgsComposerLegendStyle::Top, 2 );
  rstyle( QgsComposerLegendStyle::Symbol ).setMargin( QgsComposerLegendStyle::Top, 2 );
  rstyle( QgsComposerLegendStyle::SymbolLabel ).setMargin( QgsComposerLegendStyle::Top, 2 );
  rstyle( QgsComposerLegendStyle::SymbolLabel ).setMargin( QgsComposerLegendStyle::Left, 2 );
  rstyle( QgsComposerLegendStyle::Title ).rfont().setPointSizeF( 16.0 );
  rstyle( QgsComposerLegendStyle::Group ).rfont().setPointSizeF( 14.0 );
  rstyle( QgsComposerLegendStyle::Subgroup ).rfont().setPointSizeF( 12.0 );
  rstyle( QgsComposerLegendStyle::SymbolLabel ).rfont().setPointSizeF( 12.0 );
}
