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

#include "qgssettings.h"
#include "qgsgeometrycheckfactory.h"

#include "checks/qgsgeometryanglecheck.h"
#include "checks/qgsgeometryareacheck.h"
#include "checks/qgsgeometrycontainedcheck.h"
#include "checks/qgsgeometrydegeneratepolygoncheck.h"
#include "checks/qgsgeometryduplicatecheck.h"
#include "checks/qgsgeometryduplicatenodescheck.h"
#include "checks/qgsgeometrygapcheck.h"
#include "checks/qgsgeometryholecheck.h"
#include "checks/qgsgeometrymultipartcheck.h"
#include "checks/qgsgeometryoverlapcheck.h"
#include "checks/qgsgeometrysegmentlengthcheck.h"
#include "checks/qgsgeometryselfcontactcheck.h"
#include "checks/qgsgeometryselfintersectioncheck.h"
#include "checks/qgsgeometrysliverpolygoncheck.h"
#include "checks/qgsgeometrytypecheck.h"

#include "utils/qgsfeaturepool.h"


QString QgsGeometryCheckFactory::sSettingsGroup = QStringLiteral( "/geometry_checker/previous_values/" );


template<> void QgsGeometryCheckFactoryT<QgsGeometryAngleCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  ui.checkBoxAngle->setChecked( QgsSettings().value( sSettingsGroup + "checkAngle" ).toBool() );
  ui.doubleSpinBoxAngle->setValue( QgsSettings().value( sSettingsGroup + "minimalAngle" ).toDouble() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometryAngleCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab &ui, int /*nPoint*/, int nLineString, int nPolygon ) const
{
  ui.checkBoxAngle->setEnabled( nPolygon > 0 || nLineString > 0 );
  return ui.checkBoxAngle->isEnabled();
}

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometryAngleCheck>::createInstance( const QMap<QString, QgsFeaturePool *> &featurePools, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkAngle", ui.checkBoxAngle->isChecked() );
  QgsSettings().setValue( sSettingsGroup + "minimalAngle", ui.doubleSpinBoxAngle->value() );
  if ( ui.checkBoxAngle->isEnabled() && ui.checkBoxAngle->isChecked() )
  {
    return new QgsGeometryAngleCheck( featurePools, ui.doubleSpinBoxAngle->value() );
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
  return ui.checkBoxArea->isEnabled();
}

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometryAreaCheck>::createInstance( const QMap<QString, QgsFeaturePool *> &featurePools, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkArea", ui.checkBoxArea->isChecked() );
  QgsSettings().setValue( sSettingsGroup + "minimalArea", ui.doubleSpinBoxArea->value() );
  if ( ui.checkBoxArea->isEnabled() && ui.checkBoxArea->isChecked() )
  {
    return new QgsGeometryAreaCheck( featurePools, ui.doubleSpinBoxArea->value() );
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

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometryContainedCheck>::createInstance( const QMap<QString, QgsFeaturePool *> &featurePools, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkCovers", ui.checkBoxCovered->isChecked() );
  if ( ui.checkBoxCovered->isEnabled() && ui.checkBoxCovered->isChecked() )
  {
    return new QgsGeometryContainedCheck( featurePools );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryContainedCheck> )

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

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometryDegeneratePolygonCheck>::createInstance( const QMap<QString, QgsFeaturePool *> &featurePools, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkDegeneratePolygon", ui.checkBoxDegeneratePolygon->isChecked() );
  if ( ui.checkBoxDegeneratePolygon->isEnabled() && ui.checkBoxDegeneratePolygon->isChecked() )
  {
    return new QgsGeometryDegeneratePolygonCheck( featurePools );
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

template<> bool QgsGeometryCheckFactoryT<QgsGeometryDuplicateCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab &ui, int /*nPoint*/, int /*nLineString*/, int nPolygon ) const
{
  ui.checkBoxDuplicates->setEnabled( nPolygon > 0 );
  return ui.checkBoxDuplicates->isEnabled();
}

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometryDuplicateCheck>::createInstance( const QMap<QString, QgsFeaturePool *> &featurePools, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkDuplicates", ui.checkBoxDuplicates->isChecked() );
  if ( ui.checkBoxDuplicates->isEnabled() && ui.checkBoxDuplicates->isChecked() )
  {
    return new QgsGeometryDuplicateCheck( featurePools );
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

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometryDuplicateNodesCheck>::createInstance( const QMap<QString, QgsFeaturePool *> &featurePools, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkDuplicateNodes", ui.checkBoxDuplicateNodes->isChecked() );
  if ( ui.checkBoxDuplicateNodes->isEnabled() && ui.checkBoxDuplicateNodes->isChecked() )
  {
    return new QgsGeometryDuplicateNodesCheck( featurePools );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryDuplicateNodesCheck> )

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

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometryGapCheck>::createInstance( const QMap<QString, QgsFeaturePool *> &featurePools, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkGaps", ui.checkBoxGaps->isChecked() );
  QgsSettings().setValue( sSettingsGroup + "maxGapArea", ui.doubleSpinBoxGapArea->value() );
  if ( ui.checkBoxGaps->isEnabled() && ui.checkBoxGaps->isChecked() )
  {
    return new QgsGeometryGapCheck( featurePools, ui.doubleSpinBoxGapArea->value() );
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

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometryHoleCheck>::createInstance( const QMap<QString, QgsFeaturePool *> &featurePools, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkHoles", ui.checkBoxNoHoles->isChecked() );
  if ( ui.checkBoxNoHoles->isEnabled() && ui.checkBoxNoHoles->isChecked() )
  {
    return new QgsGeometryHoleCheck( featurePools );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryHoleCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometryMultipartCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  ui.checkBoxMultipart->setChecked( QgsSettings().value( sSettingsGroup + "checkMultipart" ).toBool() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometryMultipartCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab & /*ui*/, int /*nPoint*/, int /*nLineString*/, int /*nPolygon*/ ) const
{
  return true;
}

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometryMultipartCheck>::createInstance( const QMap<QString, QgsFeaturePool *> &featurePools, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkMultipart", ui.checkBoxMultipart->isChecked() );
  if ( ui.checkBoxMultipart->isEnabled() && ui.checkBoxMultipart->isChecked() )
  {
    return new QgsGeometryMultipartCheck( featurePools );
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

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometryOverlapCheck>::createInstance( const QMap<QString, QgsFeaturePool *> &featurePools, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkOverlaps", ui.checkBoxOverlaps->isChecked() );
  QgsSettings().setValue( sSettingsGroup + "maxOverlapArea", ui.doubleSpinBoxOverlapArea->value() );
  if ( ui.checkBoxOverlaps->isEnabled() && ui.checkBoxOverlaps->isChecked() )
  {
    return new QgsGeometryOverlapCheck( featurePools, ui.doubleSpinBoxOverlapArea->value() );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryOverlapCheck> )

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

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometrySegmentLengthCheck>::createInstance( const QMap<QString, QgsFeaturePool *> &featurePools, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkSegmentLength", ui.checkBoxSegmentLength->isChecked() );
  QgsSettings().setValue( sSettingsGroup + "minSegmentLength", ui.doubleSpinBoxSegmentLength->value() );
  if ( ui.checkBoxSegmentLength->isEnabled() && ui.checkBoxSegmentLength->isChecked() )
  {
    return new QgsGeometrySegmentLengthCheck( featurePools, ui.doubleSpinBoxSegmentLength->value() );
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

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometrySelfContactCheck>::createInstance( const QMap<QString, QgsFeaturePool *> &featurePools, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkSelfContacts", ui.checkBoxSelfContacts->isChecked() );
  if ( ui.checkBoxSelfContacts->isEnabled() && ui.checkBoxSelfContacts->isChecked() )
  {
    return new QgsGeometrySelfContactCheck( featurePools );
  }
  else
  {
    return 0;
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

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometrySelfIntersectionCheck>::createInstance( const QMap<QString, QgsFeaturePool *> &featurePools, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkSelfIntersections", ui.checkBoxSelfIntersections->isChecked() );
  if ( ui.checkBoxSelfIntersections->isEnabled() && ui.checkBoxSelfIntersections->isChecked() )
  {
    return new QgsGeometrySelfIntersectionCheck( featurePools );
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

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometrySliverPolygonCheck>::createInstance( const QMap<QString, QgsFeaturePool *> &featurePools, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  double threshold = ui.doubleSpinBoxSliverThinness->value();
  double maxArea = ui.checkBoxSliverArea->isChecked() ? ui.doubleSpinBoxSliverArea->value() : 0.;
  QgsSettings().setValue( sSettingsGroup + "sliverPolygonsAreaThresholdEnabled", ui.checkBoxSliverArea->isChecked() );
  QgsSettings().setValue( sSettingsGroup + "sliverPolygonsAreaThreshold", ui.doubleSpinBoxSliverArea->value() );
  QgsSettings().setValue( sSettingsGroup + "sliverPolygonsThinnessThreshold", ui.doubleSpinBoxSliverThinness->value() );
  QgsSettings().setValue( sSettingsGroup + "checkSliverPolygons", ui.checkBoxSliverPolygons->isChecked() );
  if ( ui.checkBoxSliverPolygons->isEnabled() && ui.checkBoxSliverPolygons->isChecked() )
  {
    return new QgsGeometrySliverPolygonCheck( featurePools, threshold, maxArea );
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

template<> bool QgsGeometryCheckFactoryT<QgsGeometryTypeCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab & /*ui*/, int /*nPoint*/, int /*nLineString*/, int /*nPolygon*/ ) const
{
  return true;
}

template<> QgsGeometryCheck *QgsGeometryCheckFactoryT<QgsGeometryTypeCheck>::createInstance( const QMap<QString, QgsFeaturePool *> &featurePools, const Ui::QgsGeometryCheckerSetupTab &ui ) const
{
  QgsSettings().setValue( sSettingsGroup + "checkTypePoint", ui.checkBoxPoint->isChecked() );
  QgsSettings().setValue( sSettingsGroup + "checkTypeMultipoint", ui.checkBoxMultipoint->isChecked() );
  QgsSettings().setValue( sSettingsGroup + "checkTypeLine", ui.checkBoxLine->isChecked() );
  QgsSettings().setValue( sSettingsGroup + "checkTypeMultiline", ui.checkBoxMultiline->isChecked() );
  QgsSettings().setValue( sSettingsGroup + "checkTypePolygon", ui.checkBoxPolygon->isChecked() );
  QgsSettings().setValue( sSettingsGroup + "checkTypeMultipolygon", ui.checkBoxMultipolygon->isChecked() );

  int allowedTypes = 0;
  if ( ui.checkBoxPoint->isChecked() )
  {
    allowedTypes |= 1 << QgsWkbTypes::Point;
  }
  if ( ui.checkBoxMultipoint->isChecked() )
  {
    allowedTypes |= 1 << QgsWkbTypes::MultiPoint;
  }
  if ( ui.checkBoxLine->isChecked() )
  {
    allowedTypes |= 1 << QgsWkbTypes::LineString;
  }
  if ( ui.checkBoxMultiline->isChecked() )
  {
    allowedTypes |= 1 << QgsWkbTypes::MultiLineString;
  }
  if ( ui.checkBoxPolygon->isChecked() )
  {
    allowedTypes |= 1 << QgsWkbTypes::Polygon;
  }
  if ( ui.checkBoxMultipolygon->isChecked() )
  {
    allowedTypes |= 1 << QgsWkbTypes::MultiPolygon;
  }
  if ( allowedTypes != 0 )
  {
    return new QgsGeometryTypeCheck( featurePools, allowedTypes );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryTypeCheck> )
