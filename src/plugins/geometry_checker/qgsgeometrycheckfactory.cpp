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

#include <QSettings>

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
#include "checks/qgsgeometryselfintersectioncheck.h"
#include "checks/qgsgeometrysliverpolygoncheck.h"
#include "checks/qgsgeometrytypecheck.h"
#include "utils/qgsfeaturepool.h"


QString QgsGeometryCheckFactory::sSettingsGroup = "/geometry_checker/previous_values/";


template<> void QgsGeometryCheckFactoryT<QgsGeometryAngleCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab& ui ) const
{
  ui.checkBoxAngle->setChecked( QSettings().value( sSettingsGroup + "checkAngle" ).toBool() );
  ui.doubleSpinBoxAngle->setValue( QSettings().value( sSettingsGroup + "minimalAngle" ).toDouble() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometryAngleCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab& ui, QGis::GeometryType geomType ) const
{
  ui.checkBoxAngle->setEnabled( geomType == QGis::Polygon || geomType == QGis::Line );
  return ui.checkBoxAngle->isEnabled();
}

template<> QgsGeometryCheck* QgsGeometryCheckFactoryT<QgsGeometryAngleCheck>::createInstance( QgsFeaturePool* featurePool, const Ui::QgsGeometryCheckerSetupTab& ui, double /*mapToLayerUnits*/ ) const
{
  QSettings().setValue( sSettingsGroup + "checkAngle", ui.checkBoxAngle->isChecked() );
  QSettings().setValue( sSettingsGroup + "minimalAngle", ui.doubleSpinBoxAngle->value() );
  if ( ui.checkBoxAngle->isEnabled() && ui.checkBoxAngle->isChecked() )
  {
    return new QgsGeometryAngleCheck( featurePool, ui.doubleSpinBoxAngle->value() );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryAngleCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometryAreaCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab& ui ) const
{
  ui.checkBoxArea->setChecked( QSettings().value( sSettingsGroup + "checkArea" ).toBool() );
  ui.doubleSpinBoxArea->setValue( QSettings().value( sSettingsGroup + "minimalArea" ).toDouble() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometryAreaCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab& ui, QGis::GeometryType geomType ) const
{
  ui.checkBoxArea->setEnabled( geomType == QGis::Polygon );
  return ui.checkBoxArea->isEnabled();
}

template<> QgsGeometryCheck* QgsGeometryCheckFactoryT<QgsGeometryAreaCheck>::createInstance( QgsFeaturePool* featurePool, const Ui::QgsGeometryCheckerSetupTab& ui, double mapToLayerUnits ) const
{
  QSettings().setValue( sSettingsGroup + "checkArea", ui.checkBoxArea->isChecked() );
  QSettings().setValue( sSettingsGroup + "minimalArea", ui.doubleSpinBoxArea->value() );
  if ( ui.checkBoxArea->isEnabled() && ui.checkBoxArea->isChecked() )
  {
    return new QgsGeometryAreaCheck( featurePool, ui.doubleSpinBoxArea->value() * mapToLayerUnits * mapToLayerUnits );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryAreaCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometryContainedCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab& ui ) const
{
  ui.checkBoxCovered->setChecked( QSettings().value( sSettingsGroup + "checkCovers" ).toBool() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometryContainedCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab& ui, QGis::GeometryType geomType ) const
{
  ui.checkBoxCovered->setEnabled( geomType == QGis::Polygon );
  return ui.checkBoxCovered->isEnabled();
}

template<> QgsGeometryCheck* QgsGeometryCheckFactoryT<QgsGeometryContainedCheck>::createInstance( QgsFeaturePool* featurePool, const Ui::QgsGeometryCheckerSetupTab& ui, double /*mapToLayerUnits*/ ) const
{
  QSettings().setValue( sSettingsGroup + "checkCovers", ui.checkBoxCovered->isChecked() );
  if ( ui.checkBoxCovered->isEnabled() && ui.checkBoxCovered->isChecked() )
  {
    return new QgsGeometryContainedCheck( featurePool );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryContainedCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometryDegeneratePolygonCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab& ui ) const
{
  ui.checkBoxDegeneratePolygon->setChecked( QSettings().value( sSettingsGroup + "checkDegeneratePolygon" ).toBool() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometryDegeneratePolygonCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab& ui, QGis::GeometryType geomType ) const
{
  ui.checkBoxDegeneratePolygon->setEnabled( geomType == QGis::Polygon );
  return ui.checkBoxDegeneratePolygon->isEnabled();
}

template<> QgsGeometryCheck* QgsGeometryCheckFactoryT<QgsGeometryDegeneratePolygonCheck>::createInstance( QgsFeaturePool* featurePool, const Ui::QgsGeometryCheckerSetupTab& ui, double /*mapToLayerUnits*/ ) const
{
  QSettings().setValue( sSettingsGroup + "checkDegeneratePolygon", ui.checkBoxDegeneratePolygon->isChecked() );
  if ( ui.checkBoxDegeneratePolygon->isEnabled() && ui.checkBoxDegeneratePolygon->isChecked() )
  {
    return new QgsGeometryDegeneratePolygonCheck( featurePool );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryDegeneratePolygonCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometryDuplicateCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab& ui ) const
{
  ui.checkBoxDuplicates->setChecked( QSettings().value( sSettingsGroup + "checkDuplicates" ).toBool() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometryDuplicateCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab& ui, QGis::GeometryType geomType ) const
{
  ui.checkBoxDuplicates->setEnabled( geomType == QGis::Polygon );
  return ui.checkBoxDuplicates->isEnabled();
}

template<> QgsGeometryCheck* QgsGeometryCheckFactoryT<QgsGeometryDuplicateCheck>::createInstance( QgsFeaturePool* featurePool, const Ui::QgsGeometryCheckerSetupTab& ui, double /*mapToLayerUnits*/ ) const
{
  QSettings().setValue( sSettingsGroup + "checkDuplicates", ui.checkBoxDuplicates->isChecked() );
  if ( ui.checkBoxDuplicates->isEnabled() && ui.checkBoxDuplicates->isChecked() )
  {
    return new QgsGeometryDuplicateCheck( featurePool );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryDuplicateCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometryDuplicateNodesCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab& ui ) const
{
  ui.checkBoxDuplicateNodes->setChecked( QSettings().value( sSettingsGroup + "checkDuplicateNodes" ).toBool() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometryDuplicateNodesCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab& ui, QGis::GeometryType geomType ) const
{
  ui.checkBoxDuplicateNodes->setEnabled( geomType == QGis::Polygon || geomType == QGis::Line );
  return ui.checkBoxDuplicateNodes->isEnabled();
}

template<> QgsGeometryCheck* QgsGeometryCheckFactoryT<QgsGeometryDuplicateNodesCheck>::createInstance( QgsFeaturePool* featurePool, const Ui::QgsGeometryCheckerSetupTab& ui, double /*mapToLayerUnits*/ ) const
{
  QSettings().setValue( sSettingsGroup + "checkDuplicateNodes", ui.checkBoxDuplicateNodes->isChecked() );
  if ( ui.checkBoxDuplicateNodes->isEnabled() && ui.checkBoxDuplicateNodes->isChecked() )
  {
    return new QgsGeometryDuplicateNodesCheck( featurePool );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryDuplicateNodesCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometryGapCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab& ui ) const
{
  ui.checkBoxGaps->setChecked( QSettings().value( sSettingsGroup + "checkGaps" ).toBool() );
  ui.doubleSpinBoxGapArea->setValue( QSettings().value( sSettingsGroup + "maxGapArea" ).toDouble() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometryGapCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab& ui, QGis::GeometryType geomType ) const
{
  ui.checkBoxGaps->setEnabled( geomType == QGis::Polygon );
  ui.doubleSpinBoxGapArea->setEnabled( ui.checkBoxGaps->isEnabled() );
  return ui.checkBoxGaps->isEnabled();
}

template<> QgsGeometryCheck* QgsGeometryCheckFactoryT<QgsGeometryGapCheck>::createInstance( QgsFeaturePool* featurePool, const Ui::QgsGeometryCheckerSetupTab& ui, double mapToLayerUnits ) const
{
  QSettings().setValue( sSettingsGroup + "checkGaps", ui.checkBoxGaps->isChecked() );
  QSettings().setValue( sSettingsGroup + "maxGapArea", ui.doubleSpinBoxGapArea->value() );
  if ( ui.checkBoxGaps->isEnabled() && ui.checkBoxGaps->isChecked() )
  {
    return new QgsGeometryGapCheck( featurePool, ui.doubleSpinBoxGapArea->value() * mapToLayerUnits * mapToLayerUnits );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryGapCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometryHoleCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab& ui ) const
{
  ui.checkBoxNoHoles->setChecked( QSettings().value( sSettingsGroup + "checkHoles" ).toBool() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometryHoleCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab& ui, QGis::GeometryType geomType ) const
{
  ui.checkBoxNoHoles->setEnabled( geomType == QGis::Polygon );
  return ui.checkBoxNoHoles->isEnabled();
}

template<> QgsGeometryCheck* QgsGeometryCheckFactoryT<QgsGeometryHoleCheck>::createInstance( QgsFeaturePool* featurePool, const Ui::QgsGeometryCheckerSetupTab& ui, double /*mapToLayerUnits*/ ) const
{
  QSettings().setValue( sSettingsGroup + "checkHoles", ui.checkBoxNoHoles->isChecked() );
  if ( ui.checkBoxNoHoles->isEnabled() && ui.checkBoxNoHoles->isChecked() )
  {
    return new QgsGeometryHoleCheck( featurePool );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryHoleCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometryMultipartCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab& ui ) const
{
  ui.checkBoxMultipart->setChecked( QSettings().value( sSettingsGroup + "checkMultipart" ).toBool() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometryMultipartCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab& /*ui*/, QGis::GeometryType /*geomType*/ ) const
{
  return true;
}

template<> QgsGeometryCheck* QgsGeometryCheckFactoryT<QgsGeometryMultipartCheck>::createInstance( QgsFeaturePool* featurePool, const Ui::QgsGeometryCheckerSetupTab& ui, double /*mapToLayerUnits*/ ) const
{
  QSettings().setValue( sSettingsGroup + "checkMultipart", ui.checkBoxMultipart->isChecked() );
  if ( ui.checkBoxMultipart->isEnabled() && ui.checkBoxMultipart->isChecked() )
  {
    return new QgsGeometryMultipartCheck( featurePool );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryMultipartCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometryOverlapCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab& ui ) const
{
  ui.checkBoxOverlaps->setChecked( QSettings().value( sSettingsGroup + "checkOverlaps" ).toBool() );
  ui.doubleSpinBoxOverlapArea->setValue( QSettings().value( sSettingsGroup + "maxOverlapArea" ).toDouble() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometryOverlapCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab& ui, QGis::GeometryType geomType ) const
{
  ui.checkBoxOverlaps->setEnabled( geomType == QGis::Polygon );
  ui.doubleSpinBoxOverlapArea->setEnabled( ui.checkBoxOverlaps->isEnabled() );
  return ui.checkBoxOverlaps->isEnabled();
}

template<> QgsGeometryCheck* QgsGeometryCheckFactoryT<QgsGeometryOverlapCheck>::createInstance( QgsFeaturePool* featurePool, const Ui::QgsGeometryCheckerSetupTab& ui, double mapToLayerUnits ) const
{
  QSettings().setValue( sSettingsGroup + "checkOverlaps", ui.checkBoxOverlaps->isChecked() );
  QSettings().setValue( sSettingsGroup + "maxOverlapArea", ui.doubleSpinBoxOverlapArea->value() );
  if ( ui.checkBoxOverlaps->isEnabled() && ui.checkBoxOverlaps->isChecked() )
  {
    return new QgsGeometryOverlapCheck( featurePool, ui.doubleSpinBoxOverlapArea->value() * mapToLayerUnits * mapToLayerUnits );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryOverlapCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometrySegmentLengthCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab& ui ) const
{
  ui.checkBoxSegmentLength->setChecked( QSettings().value( sSettingsGroup + "checkSegmentLength" ).toBool() );
  ui.doubleSpinBoxSegmentLength->setValue( QSettings().value( sSettingsGroup + "minSegmentLength" ).toDouble() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometrySegmentLengthCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab& ui, QGis::GeometryType geomType ) const
{
  ui.checkBoxSegmentLength->setEnabled( geomType == QGis::Polygon || geomType == QGis::Line );
  ui.doubleSpinBoxSegmentLength->setEnabled( ui.checkBoxSegmentLength->isEnabled() );
  return ui.checkBoxSegmentLength->isEnabled();
}

template<> QgsGeometryCheck* QgsGeometryCheckFactoryT<QgsGeometrySegmentLengthCheck>::createInstance( QgsFeaturePool* featurePool, const Ui::QgsGeometryCheckerSetupTab& ui, double mapToLayerUnits ) const
{
  QSettings().setValue( sSettingsGroup + "checkSegmentLength", ui.checkBoxSegmentLength->isChecked() );
  QSettings().setValue( sSettingsGroup + "minSegmentLength", ui.doubleSpinBoxSegmentLength->value() );
  if ( ui.checkBoxSegmentLength->isEnabled() && ui.checkBoxSegmentLength->isChecked() )
  {
    return new QgsGeometrySegmentLengthCheck( featurePool, ui.doubleSpinBoxSegmentLength->value() * mapToLayerUnits );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometrySegmentLengthCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometrySelfIntersectionCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab& ui ) const
{
  ui.checkBoxSelfIntersections->setChecked( QSettings().value( sSettingsGroup + "checkSelfIntersections" ).toBool() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometrySelfIntersectionCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab& ui, QGis::GeometryType geomType ) const
{
  ui.checkBoxSelfIntersections->setEnabled( geomType == QGis::Polygon || geomType == QGis::Line );
  return ui.checkBoxSelfIntersections->isEnabled();
}

template<> QgsGeometryCheck* QgsGeometryCheckFactoryT<QgsGeometrySelfIntersectionCheck>::createInstance( QgsFeaturePool* featurePool, const Ui::QgsGeometryCheckerSetupTab& ui, double /*mapToLayerUnits*/ ) const
{
  QSettings().setValue( sSettingsGroup + "checkSelfIntersections", ui.checkBoxSelfIntersections->isChecked() );
  if ( ui.checkBoxSelfIntersections->isEnabled() && ui.checkBoxSelfIntersections->isChecked() )
  {
    return new QgsGeometrySelfIntersectionCheck( featurePool );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometrySelfIntersectionCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometrySliverPolygonCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab& ui ) const
{
  ui.checkBoxSliverPolygons->setChecked( QSettings().value( sSettingsGroup + "checkSliverPolygons" ).toBool() );
  ui.checkBoxSliverArea->setChecked( QSettings().value( sSettingsGroup + "sliverPolygonsAreaThresholdEnabled" ).toBool() );
  ui.doubleSpinBoxSliverArea->setValue( QSettings().value( sSettingsGroup + "sliverPolygonsAreaThreshold" ).toDouble() );
  ui.doubleSpinBoxSliverThinness->setValue( QSettings().value( sSettingsGroup + "sliverPolygonsThinnessThreshold" ).toDouble() );
  ui.checkBoxSliverPolygons->setChecked( QSettings().value( sSettingsGroup + "checkSliverPolygons" ).toBool() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometrySliverPolygonCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab& ui, QGis::GeometryType geomType ) const
{
  ui.checkBoxSliverPolygons->setEnabled( geomType == QGis::Polygon );
  return ui.checkBoxSliverPolygons->isEnabled();
}

template<> QgsGeometryCheck* QgsGeometryCheckFactoryT<QgsGeometrySliverPolygonCheck>::createInstance( QgsFeaturePool* featurePool, const Ui::QgsGeometryCheckerSetupTab& ui, double mapToLayerUnits ) const
{
  double threshold = ui.doubleSpinBoxSliverThinness->value();
  double maxArea = ui.checkBoxSliverArea->isChecked() ? ui.doubleSpinBoxSliverArea->value() : 0.;
  QSettings().setValue( sSettingsGroup + "sliverPolygonsAreaThresholdEnabled", ui.checkBoxSliverArea->isChecked() );
  QSettings().setValue( sSettingsGroup + "sliverPolygonsAreaThreshold", ui.doubleSpinBoxSliverArea->value() );
  QSettings().setValue( sSettingsGroup + "sliverPolygonsThinnessThreshold", ui.doubleSpinBoxSliverThinness->value() );
  QSettings().setValue( sSettingsGroup + "checkSliverPolygons", ui.checkBoxSliverPolygons->isChecked() );
  if ( ui.checkBoxSliverPolygons->isEnabled() && ui.checkBoxSliverPolygons->isChecked() )
  {
    return new QgsGeometrySliverPolygonCheck( featurePool, threshold, maxArea * mapToLayerUnits * mapToLayerUnits );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometrySliverPolygonCheck> )

///////////////////////////////////////////////////////////////////////////////

template<> void QgsGeometryCheckFactoryT<QgsGeometryTypeCheck>::restorePrevious( Ui::QgsGeometryCheckerSetupTab& ui ) const
{
  ui.checkBoxPoint->setChecked( QSettings().value( sSettingsGroup + "checkTypePoint" ).toBool() );
  ui.checkBoxMultipoint->setChecked( QSettings().value( sSettingsGroup + "checkTypeMultipoint" ).toBool() );
  ui.checkBoxLine->setChecked( QSettings().value( sSettingsGroup + "checkTypeLine" ).toBool() );
  ui.checkBoxMultiline->setChecked( QSettings().value( sSettingsGroup + "checkTypeMultiline" ).toBool() );
  ui.checkBoxPolygon->setChecked( QSettings().value( sSettingsGroup + "checkTypePolygon" ).toBool() );
  ui.checkBoxMultipolygon->setChecked( QSettings().value( sSettingsGroup + "checkTypeMultipolygon" ).toBool() );
}

template<> bool QgsGeometryCheckFactoryT<QgsGeometryTypeCheck>::checkApplicability( Ui::QgsGeometryCheckerSetupTab& /*ui*/, QGis::GeometryType /*geomType*/ ) const
{
  return true;
}

template<> QgsGeometryCheck* QgsGeometryCheckFactoryT<QgsGeometryTypeCheck>::createInstance( QgsFeaturePool* featurePool, const Ui::QgsGeometryCheckerSetupTab& ui, double /*mapToLayerUnits*/ ) const
{
  QSettings().setValue( sSettingsGroup + "checkTypePoint", ui.checkBoxPoint->isChecked() );
  QSettings().setValue( sSettingsGroup + "checkTypeMultipoint", ui.checkBoxMultipoint->isChecked() );
  QSettings().setValue( sSettingsGroup + "checkTypeLine", ui.checkBoxLine->isChecked() );
  QSettings().setValue( sSettingsGroup + "checkTypeMultiline", ui.checkBoxMultiline->isChecked() );
  QSettings().setValue( sSettingsGroup + "checkTypePolygon", ui.checkBoxPolygon->isChecked() );
  QSettings().setValue( sSettingsGroup + "checkTypeMultipolygon", ui.checkBoxMultipolygon->isChecked() );

  int allowedTypes = 0;
  if ( ui.checkBoxPoint->isChecked() )
  {
    allowedTypes |= 1 << QGis::WKBPoint;
  }
  if ( ui.checkBoxMultipoint->isChecked() )
  {
    allowedTypes |= 1 << QGis::WKBMultiPoint;
  }
  if ( ui.checkBoxLine->isChecked() )
  {
    allowedTypes |= 1 << QGis::WKBLineString;
  }
  if ( ui.checkBoxMultiline->isChecked() )
  {
    allowedTypes |= 1 << QGis::WKBMultiLineString;
  }
  if ( ui.checkBoxPolygon->isChecked() )
  {
    allowedTypes |= 1 << QGis::WKBPolygon;
  }
  if ( ui.checkBoxMultipolygon->isChecked() )
  {
    allowedTypes |= 1 << QGis::WKBMultiPolygon;
  }
  if ( allowedTypes != 0 )
  {
    return new QgsGeometryTypeCheck( featurePool, allowedTypes );
  }
  else
  {
    return nullptr;
  }
}

REGISTER_QGS_GEOMETRY_CHECK_FACTORY( QgsGeometryCheckFactoryT<QgsGeometryTypeCheck> )
