/***************************************************************************
    qgsgeometrycheckfactory.cpp
    ---------------------
    begin                : September 2015
    copyright            : (C) 2014 by Sandro Mani / Sourcepole AG
    email                : smani at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsproject.h"
#include "qgssettings.h"
#include "qgsgeometrycheckfactory.h"

#include "qgsgeometryanglecheck.h"
#include "qgsgeometryareacheck.h"
#include "qgsgeometrycontainedcheck.h"
#include "qgsgeometrydanglecheck.h"
#include "qgsgeometrydegeneratepolygoncheck.h"
#include "qgsgeometryduplicatecheck.h"
#include "qgsgeometryduplicatenodescheck.h"
#include "qgsgeometryfollowboundariescheck.h"
#include "qgsgeometrygapcheck.h"
#include "qgsgeometryholecheck.h"
#include "qgsgeometrylineintersectioncheck.h"
#include "qgsgeometrylinelayerintersectioncheck.h"
#include "qgsgeometrymultipartcheck.h"
#include "qgsgeometryoverlapcheck.h"
#include "qgsgeometrypointcoveredbylinecheck.h"
#include "qgsgeometrypointinpolygoncheck.h"
#include "qgsgeometrysegmentlengthcheck.h"
#include "qgsgeometryselfcontactcheck.h"
#include "qgsgeometryselfintersectioncheck.h"
#include "qgsgeometrysliverpolygoncheck.h"
#include "qgsgeometrytypecheck.h"

#include "qgsfeaturepool.h"


QString QgsGeometryCheckFactory::sSettingsGroup = QStringLiteral( "/geometry_checker/previous_values/" );


template<> void QgsGeometryCheckFactoryT<QgsGeometryAngleCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  ui.checkBoxAngle->setChecked( QgsSettings().value( sSettingsGroup + "checkAngle" ).toBool() );
  ui.doubleSpinBoxAngle->setValue( QgsSettings().value( sSettingsGroup + "minimalAngle" ).toDouble() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometryAngleCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab &ui, int /*nPoint*/, int nLineString, int nPolygon ) const
{
  ui.checkBoxAngle->setEnabled( nPolygon > 0 || nLineString > 0 );
  ui.doubleSpinBoxAngle->setEnabled( ui.checkBoxAngle->isEnabled() );
  return ui.checkBoxAngle->isEnabled();
}

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometryAngleCheck>::createInstance( QgsGeometryCheckContext *context, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkAngle", ui.checkBoxAngle->isChecked() );
  QgsSettings().setValue( sSettingsGroup + "minimalAngle", ui.doubleSpinBoxAngle->value() );
  QVariantMap configuration;
  configuration.insert( "minAngle", ui.doubleSpinBoxAngle->value() );
  if ( ui.checkBoxAngle->isEnabled() && ui.checkBoxAngle->isChecked() )
  {
    return new QgsGeometryAngleCheck( context, configuration );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryAngleCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometryAreaCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  ui.checkBoxArea->setChecked( QgsSettings().value( sSettingsGroup + "checkArea" ).toBool() );
  ui.doubleSpinBoxArea->setValue( QgsSettings().value( sSettingsGroup + "minimalArea" ).toDouble() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometryAreaCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab &ui, int /*nPoint*/, int /*nLineString*/, int nPolygon ) const
{
  ui.checkBoxArea->setEnabled( nPolygon > 0 );
  ui.doubleSpinBoxArea->setEnabled( ui.checkBoxArea->isEnabled() );
  return ui.checkBoxArea->isEnabled();
}

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometryAreaCheck>::createInstance( QgsGeometryCheckContext *context, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkArea", ui.checkBoxArea->isChecked() );
  QgsSettings().setValue( sSettingsGroup + "minimalArea", ui.doubleSpinBoxArea->value() );
  QVariantMap configuration;
  configuration.insert( "areaThreshold", ui.doubleSpinBoxAngle->value() );
  if ( ui.checkBoxArea->isEnabled() && ui.checkBoxArea->isChecked() )
  {
    return new QgsGeometryAreaCheck( context, configuration );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryAreaCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometryContainedCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  ui.checkBoxCovered->setChecked( QgsSettings().value( sSettingsGroup + "checkCovers" ).toBool() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometryContainedCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab &ui, int /*nPoint*/, int /*nLineString*/, int nPolygon ) const
{
  ui.checkBoxCovered->setEnabled( nPolygon > 0 );
  return ui.checkBoxCovered->isEnabled();
}

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometryContainedCheck>::createInstance( QgsGeometryCheckContext *context, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkCovers", ui.checkBoxCovered->isChecked() );
  if ( ui.checkBoxCovered->isEnabled() && ui.checkBoxCovered->isChecked() )
  {
    return new QgsGeometryContainedCheck( context, QVariantMap() );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryContainedCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometryDangleCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  ui.checkBoxDangle->setChecked( QgsSettings().value( sSettingsGroup + "checkDangle" ).toBool() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometryDangleCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab &ui, int /*nPoint*/, int nLineString, int /*nPolygon*/ ) const
{
  ui.checkBoxDangle->setEnabled( nLineString > 0 );
  return ui.checkBoxDangle->isEnabled();
}

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometryDangleCheck>::createInstance( QgsGeometryCheckContext *context, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkDangle", ui.checkBoxDangle->isChecked() );
  if ( ui.checkBoxDangle->isEnabled() && ui.checkBoxDangle->isChecked() )
  {
    return new QgsGeometryDangleCheck( context, QVariantMap() );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryDangleCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometryDegeneratePolygonCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  ui.checkBoxDegeneratePolygon->setChecked( QgsSettings().value( sSettingsGroup + "checkDegeneratePolygon" ).toBool() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometryDegeneratePolygonCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab &ui, int /*nPoint*/, int /*nLineString*/, int nPolygon ) const
{
  ui.checkBoxDegeneratePolygon->setEnabled( nPolygon > 0 );
  return ui.checkBoxDegeneratePolygon->isEnabled();
}

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometryDegeneratePolygonCheck>::createInstance( QgsGeometryCheckContext *context, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkDegeneratePolygon", ui.checkBoxDegeneratePolygon->isChecked() );
  if ( ui.checkBoxDegeneratePolygon->isEnabled() && ui.checkBoxDegeneratePolygon->isChecked() )
  {
    return new QgsGeometryDegeneratePolygonCheck( context, QVariantMap() );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryDegeneratePolygonCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometryDuplicateCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  ui.checkBoxDuplicates->setChecked( QgsSettings().value( sSettingsGroup + "checkDuplicates" ).toBool() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometryDuplicateCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab &ui, int nPoint, int nLineString, int nPolygon ) const
{
  ui.checkBoxDuplicates->setEnabled( nPoint > 0 || nLineString > 0 || nPolygon > 0 );
  return ui.checkBoxDuplicates->isEnabled();
}

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometryDuplicateCheck>::createInstance( QgsGeometryCheckContext *context, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkDuplicates", ui.checkBoxDuplicates->isChecked() );
  if ( ui.checkBoxDuplicates->isEnabled() && ui.checkBoxDuplicates->isChecked() )
  {
    return new QgsGeometryDuplicateCheck( context, QVariantMap() );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryDuplicateCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometryDuplicateNodesCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  ui.checkBoxDuplicateNodes->setChecked( QgsSettings().value( sSettingsGroup + "checkDuplicateNodes" ).toBool() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometryDuplicateNodesCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab &ui, int /*nPoint*/, int nLineString, int nPolygon ) const
{
  ui.checkBoxDuplicateNodes->setEnabled( nPolygon > 0 || nLineString > 0 );
  return ui.checkBoxDuplicateNodes->isEnabled();
}

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometryDuplicateNodesCheck>::createInstance( QgsGeometryCheckContext *context, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkDuplicateNodes", ui.checkBoxDuplicateNodes->isChecked() );
  if ( ui.checkBoxDuplicateNodes->isEnabled() && ui.checkBoxDuplicateNodes->isChecked() )
  {
    return new QgsGeometryDuplicateNodesCheck( context, QVariantMap() );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryDuplicateNodesCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometryFollowBoundariesCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  ui.checkBoxFollowBoundaries->setChecked( QgsSettings().value( sSettingsGroup + "checkFollowBoundaries" ).toBool() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometryFollowBoundariesCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab &ui, int /*nPoint*/, int nLineString, int nPolygon ) const
{
  const bool enabled = nLineString + nPolygon > 0;
  if ( !enabled )
    ui.checkBoxFollowBoundaries->setChecked( false );
  ui.checkBoxFollowBoundaries->setEnabled( enabled );
  ui.comboBoxFollowBoundaries->setEnabled( enabled && ui.checkBoxFollowBoundaries->isChecked() );
  return enabled;
}

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometryFollowBoundariesCheck>::createInstance( QgsGeometryCheckContext *context, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkFollowBoundaries", ui.checkBoxFollowBoundaries->isChecked() );
  if ( ui.checkBoxFollowBoundaries->isEnabled() && ui.checkBoxFollowBoundaries->isChecked() )
  {
    QgsVectorLayer *checkLayer = QgsProject::instance()->mapLayer<QgsVectorLayer *>( ui.comboBoxFollowBoundaries->currentData().toString() );
    return new QgsGeometryFollowBoundariesCheck( context, QVariantMap(), checkLayer );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryFollowBoundariesCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometryGapCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  ui.checkBoxGaps->setChecked( QgsSettings().value( sSettingsGroup + "checkGaps" ).toBool() );
  ui.doubleSpinBoxGapArea->setValue( QgsSettings().value( sSettingsGroup + "maxGapArea" ).toDouble() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometryGapCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab &ui, int /*nPoint*/, int /*nLineString*/, int nPolygon ) const
{
  ui.checkBoxGaps->setEnabled( nPolygon > 0 );
  ui.doubleSpinBoxGapArea->setEnabled( ui.checkBoxGaps->isEnabled() );
  return ui.checkBoxGaps->isEnabled();
}

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometryGapCheck>::createInstance( QgsGeometryCheckContext *context, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkGaps", ui.checkBoxGaps->isChecked() );
  QgsSettings().setValue( sSettingsGroup + "maxGapArea", ui.doubleSpinBoxGapArea->value() );
  QVariantMap configuration;
  configuration.insert( "gapThreshold", ui.doubleSpinBoxGapArea->value() );

  if ( ui.checkBoxGaps->isEnabled() && ui.checkBoxGaps->isChecked() )
  {
    return new QgsGeometryGapCheck( context, configuration );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryGapCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometryHoleCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  ui.checkBoxNoHoles->setChecked( QgsSettings().value( sSettingsGroup + "checkHoles" ).toBool() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometryHoleCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab &ui, int /*nPoint*/, int /*nLineString*/, int nPolygon ) const
{
  ui.checkBoxNoHoles->setEnabled( nPolygon > 0 );
  return ui.checkBoxNoHoles->isEnabled();
}

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometryHoleCheck>::createInstance( QgsGeometryCheckContext *context, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkHoles", ui.checkBoxNoHoles->isChecked() );
  if ( ui.checkBoxNoHoles->isEnabled() && ui.checkBoxNoHoles->isChecked() )
  {
    return new QgsGeometryHoleCheck( context, QVariantMap() );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryHoleCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometryLineIntersectionCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  ui.checkLineIntersection->setChecked( QgsSettings().value( sSettingsGroup + "checkLineIntersection" ).toBool() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometryLineIntersectionCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab &ui, int /*nPoint*/, int nLineString, int /*nPolygon*/ ) const
{
  ui.checkLineIntersection->setEnabled( nLineString > 0 );
  return ui.checkLineIntersection->isEnabled();
}

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometryLineIntersectionCheck>::createInstance( QgsGeometryCheckContext *context, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkLineIntersection", ui.checkLineIntersection->isChecked() );
  if ( ui.checkLineIntersection->isEnabled() && ui.checkLineIntersection->isChecked() )
  {
    return new QgsGeometryLineIntersectionCheck( context, QVariantMap() );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryLineIntersectionCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometryLineLayerIntersectionCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  ui.checkLineLayerIntersection->setChecked( QgsSettings().value( sSettingsGroup + "checkLineLayerIntersection" ).toBool() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometryLineLayerIntersectionCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab &ui, int /*nPoint*/, int nLineString, int /*nPolygon*/ ) const
{
  const bool enabled = nLineString > 0;
  if ( !enabled )
    ui.checkLineLayerIntersection->setChecked( false );
  ui.checkLineLayerIntersection->setEnabled( enabled );
  ui.comboLineLayerIntersection->setEnabled( enabled && ui.checkLineLayerIntersection->isChecked() );
  return enabled;
}

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometryLineLayerIntersectionCheck>::createInstance( QgsGeometryCheckContext *context, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkLineLayerIntersection", ui.checkLineLayerIntersection->isChecked() );
  QVariantMap configuration;
  configuration.insert( "checkLayer", ui.comboLineLayerIntersection->currentData().toString() );
  if ( ui.checkLineLayerIntersection->isEnabled() && ui.checkLineLayerIntersection->isChecked() )
  {
    return new QgsGeometryLineLayerIntersectionCheck( context, configuration );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryLineLayerIntersectionCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometryMultipartCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  ui.checkBoxMultipart->setChecked( QgsSettings().value( sSettingsGroup + "checkMultipart" ).toBool() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometryMultipartCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab &ui, int nPoint, int nLineString, int nPolygon ) const
{
  ui.checkBoxMultipart->setEnabled( nPoint + nLineString + nPolygon > 0 );
  return ui.checkBoxMultipart->isEnabled();
}

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometryMultipartCheck>::createInstance( QgsGeometryCheckContext *context, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkMultipart", ui.checkBoxMultipart->isChecked() );
  if ( ui.checkBoxMultipart->isEnabled() && ui.checkBoxMultipart->isChecked() )
  {
    return new QgsGeometryMultipartCheck( context, QVariantMap() );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryMultipartCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometryOverlapCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  ui.checkBoxOverlaps->setChecked( QgsSettings().value( sSettingsGroup + "checkOverlaps" ).toBool() );
  ui.doubleSpinBoxOverlapArea->setValue( QgsSettings().value( sSettingsGroup + "maxOverlapArea" ).toDouble() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometryOverlapCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab &ui, int /*nPoint*/, int /*nLineString*/, int nPolygon ) const
{
  ui.checkBoxOverlaps->setEnabled( nPolygon > 0 );
  ui.doubleSpinBoxOverlapArea->setEnabled( ui.checkBoxOverlaps->isEnabled() );
  return ui.checkBoxOverlaps->isEnabled();
}

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometryOverlapCheck>::createInstance( QgsGeometryCheckContext *context, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkOverlaps", ui.checkBoxOverlaps->isChecked() );
  QgsSettings().setValue( sSettingsGroup + "maxOverlapArea", ui.doubleSpinBoxOverlapArea->value() );
  QVariantMap configuration;
  configuration.insert( "maxOverlapArea", ui.doubleSpinBoxOverlapArea->value() );
  if ( ui.checkBoxOverlaps->isEnabled() && ui.checkBoxOverlaps->isChecked() )
  {
    return new QgsGeometryOverlapCheck( context, configuration );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryOverlapCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometryPointCoveredByLineCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  ui.checkPointCoveredByLine->setChecked( QgsSettings().value( sSettingsGroup + "checkPointCoveredByLine" ).toBool() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometryPointCoveredByLineCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab &ui, int nPoint, int /*nLineString*/, int /*nPolygon*/ ) const
{
  ui.checkPointCoveredByLine->setEnabled( nPoint > 0 );
  return ui.checkPointCoveredByLine->isEnabled();
}

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometryPointCoveredByLineCheck>::createInstance( QgsGeometryCheckContext *context, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkPointCoveredByLine", ui.checkPointCoveredByLine->isChecked() );
  if ( ui.checkPointCoveredByLine->isEnabled() && ui.checkPointCoveredByLine->isChecked() )
  {
    return new QgsGeometryPointCoveredByLineCheck( context, QVariantMap() );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryPointCoveredByLineCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometryPointInPolygonCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  ui.checkPointCoveredByLine->setChecked( QgsSettings().value( sSettingsGroup + "checkPointInPolygon" ).toBool() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometryPointInPolygonCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab &ui, int nPoint, int /*nLineString*/, int /*nPolygon*/ ) const
{
  ui.checkPointInPolygon->setEnabled( nPoint > 0 );
  return ui.checkPointInPolygon->isEnabled();
}

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometryPointInPolygonCheck>::createInstance( QgsGeometryCheckContext *context, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkPointInPolygon", ui.checkPointInPolygon->isChecked() );
  if ( ui.checkPointInPolygon->isEnabled() && ui.checkPointInPolygon->isChecked() )
  {
    return new QgsGeometryPointInPolygonCheck( context, QVariantMap() );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryPointInPolygonCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometrySegmentLengthCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  ui.checkBoxSegmentLength->setChecked( QgsSettings().value( sSettingsGroup + "checkSegmentLength" ).toBool() );
  ui.doubleSpinBoxSegmentLength->setValue( QgsSettings().value( sSettingsGroup + "minSegmentLength" ).toDouble() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometrySegmentLengthCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab &ui, int /*nPoint*/, int nLineString, int nPolygon ) const
{
  ui.checkBoxSegmentLength->setEnabled( nPolygon > 0 || nLineString > 0 );
  ui.doubleSpinBoxSegmentLength->setEnabled( ui.checkBoxSegmentLength->isEnabled() );
  return ui.checkBoxSegmentLength->isEnabled();
}

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometrySegmentLengthCheck>::createInstance( QgsGeometryCheckContext *context, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkSegmentLength", ui.checkBoxSegmentLength->isChecked() );
  QgsSettings().setValue( sSettingsGroup + "minSegmentLength", ui.doubleSpinBoxSegmentLength->value() );
  QVariantMap configuration;
  configuration.insert( "minSegmentLength", ui.doubleSpinBoxSegmentLength->value() );
  if ( ui.checkBoxSegmentLength->isEnabled() && ui.checkBoxSegmentLength->isChecked() )
  {
    return new QgsGeometrySegmentLengthCheck( context, configuration );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometrySegmentLengthCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometrySelfContactCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  ui.checkBoxSelfContacts->setChecked( QgsSettings().value( sSettingsGroup + "checkSelfContacts" ).toBool() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometrySelfContactCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab &ui, int /*nPoint*/, int nLineString, int nPolygon ) const
{
  ui.checkBoxSelfContacts->setEnabled( nPolygon > 0 || nLineString > 0 );
  return ui.checkBoxSelfContacts->isEnabled();
}

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometrySelfContactCheck>::createInstance( QgsGeometryCheckContext *context, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkSelfContacts", ui.checkBoxSelfContacts->isChecked() );
  if ( ui.checkBoxSelfContacts->isEnabled() && ui.checkBoxSelfContacts->isChecked() )
  {
    return new QgsGeometrySelfContactCheck( context, QVariantMap() );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometrySelfContactCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometrySelfIntersectionCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  ui.checkBoxSelfIntersections->setChecked( QgsSettings().value( sSettingsGroup + "checkSelfIntersections" ).toBool() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometrySelfIntersectionCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab &ui, int /*nPoint*/, int nLineString, int nPolygon ) const
{
  ui.checkBoxSelfIntersections->setEnabled( nPolygon > 0 || nLineString > 0 );
  return ui.checkBoxSelfIntersections->isEnabled();
}

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometrySelfIntersectionCheck>::createInstance( QgsGeometryCheckContext *context, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkSelfIntersections", ui.checkBoxSelfIntersections->isChecked() );
  if ( ui.checkBoxSelfIntersections->isEnabled() && ui.checkBoxSelfIntersections->isChecked() )
  {
    return new QgsGeometrySelfIntersectionCheck( context, QVariantMap() );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometrySelfIntersectionCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometrySliverPolygonCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  ui.checkBoxSliverPolygons->setChecked( QgsSettings().value( sSettingsGroup + "checkSliverPolygons" ).toBool() );
  ui.checkBoxSliverArea->setChecked( QgsSettings().value( sSettingsGroup + "sliverPolygonsAreaThresholdEnabled" ).toBool() );
  ui.doubleSpinBoxSliverArea->setValue( QgsSettings().value( sSettingsGroup + "sliverPolygonsAreaThreshold" ).toDouble() );
  ui.doubleSpinBoxSliverThinness->setValue( QgsSettings().value( sSettingsGroup + "sliverPolygonsThinnessThreshold", 20 ).toDouble() );
  ui.checkBoxSliverPolygons->setChecked( QgsSettings().value( sSettingsGroup + "checkSliverPolygons" ).toBool() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometrySliverPolygonCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab &ui, int /*nPoint*/, int /*nLineString*/, int nPolygon ) const
{
  ui.checkBoxSliverPolygons->setEnabled( nPolygon > 0 );
  return ui.checkBoxSliverPolygons->isEnabled();
}

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometrySliverPolygonCheck>::createInstance( QgsGeometryCheckContext *context, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  const double threshold = ui.doubleSpinBoxSliverThinness->value();
  const double maxArea = ui.checkBoxSliverArea->isChecked() ? ui.doubleSpinBoxSliverArea->value() : 0.;
  QgsSettings().setValue( sSettingsGroup + "sliverPolygonsAreaThresholdEnabled", ui.checkBoxSliverArea->isChecked() );
  QgsSettings().setValue( sSettingsGroup + "sliverPolygonsAreaThreshold", ui.doubleSpinBoxSliverArea->value() );
  QgsSettings().setValue( sSettingsGroup + "sliverPolygonsThinnessThreshold", ui.doubleSpinBoxSliverThinness->value() );
  QgsSettings().setValue( sSettingsGroup + "checkSliverPolygons", ui.checkBoxSliverPolygons->isChecked() );
  QVariantMap configuration;
  configuration.insert( "threshold", threshold );
  configuration.insert( "maxArea", maxArea );

  if ( ui.checkBoxSliverPolygons->isEnabled() && ui.checkBoxSliverPolygons->isChecked() )
  {
    return new QgsGeometrySliverPolygonCheck( context, configuration );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometrySliverPolygonCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometryTypeCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  ui.checkBoxPoint->setChecked( QgsSettings().value( sSettingsGroup + "checkTypePoint" ).toBool() );
  ui.checkBoxMultipoint->setChecked( QgsSettings().value( sSettingsGroup + "checkTypeMultipoint" ).toBool() );
  ui.checkBoxLine->setChecked( QgsSettings().value( sSettingsGroup + "checkTypeLine" ).toBool() );
  ui.checkBoxMultiline->setChecked( QgsSettings().value( sSettingsGroup + "checkTypeMultiline" ).toBool() );
  ui.checkBoxPolygon->setChecked( QgsSettings().value( sSettingsGroup + "checkTypePolygon" ).toBool() );
  ui.checkBoxMultipolygon->setChecked( QgsSettings().value( sSettingsGroup + "checkTypeMultipolygon" ).toBool() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometryTypeCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab &ui, int nPoint, int nLineString, int nPolygon ) const
{
  ui.checkBoxPoint->setEnabled( nPoint > 0 );
  ui.checkBoxMultipoint->setEnabled( nPoint > 0 );
  ui.checkBoxLine->setEnabled( nLineString > 0 );
  ui.checkBoxMultiline->setEnabled( nLineString > 0 );
  ui.checkBoxPolygon->setEnabled( nPolygon > 0 );
  ui.checkBoxMultipolygon->setEnabled( nPolygon > 0 );
  return nPoint + nLineString + nPolygon > 0;
}

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometryTypeCheck>::createInstance( QgsGeometryCheckContext *context, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkTypePoint", ui.checkBoxPoint->isChecked() );
  QgsSettings().setValue( sSettingsGroup + "checkTypeMultipoint", ui.checkBoxMultipoint->isChecked() );
  QgsSettings().setValue( sSettingsGroup + "checkTypeLine", ui.checkBoxLine->isChecked() );
  QgsSettings().setValue( sSettingsGroup + "checkTypeMultiline", ui.checkBoxMultiline->isChecked() );
  QgsSettings().setValue( sSettingsGroup + "checkTypePolygon", ui.checkBoxPolygon->isChecked() );
  QgsSettings().setValue( sSettingsGroup + "checkTypeMultipolygon", ui.checkBoxMultipolygon->isChecked() );

  int allowedTypes = 0;
  if ( ui.checkBoxPoint->isEnabled() && ui.checkBoxPoint->isChecked() )
  {
    allowedTypes |= 1 << QgsWkbTypes::Point;
  }
  if ( ui.checkBoxMultipoint->isEnabled() && ui.checkBoxMultipoint->isChecked() )
  {
    allowedTypes |= 1 << QgsWkbTypes::MultiPoint;
  }
  if ( ui.checkBoxLine->isEnabled() && ui.checkBoxLine->isChecked() )
  {
    allowedTypes |= 1 << QgsWkbTypes::LineString;
  }
  if ( ui.checkBoxMultiline->isEnabled() && ui.checkBoxMultiline->isChecked() )
  {
    allowedTypes |= 1 << QgsWkbTypes::MultiLineString;
  }
  if ( ui.checkBoxPolygon->isEnabled() && ui.checkBoxPolygon->isChecked() )
  {
    allowedTypes |= 1 << QgsWkbTypes::Polygon;
  }
  if ( ui.checkBoxMultipolygon->isEnabled() && ui.checkBoxMultipolygon->isChecked() )
  {
    allowedTypes |= 1 << QgsWkbTypes::MultiPolygon;
  }
  if ( allowedTypes != 0 )
  {
    return new QgsGeometryTypeCheck( context, QVariantMap(), allowedTypes );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryTypeCheck> )
