# The following has been generated automatically from src/core/qgsvectorfilewriter.h
QgsVectorFileWriter.Set = QgsVectorFileWriter.OptionType.Set
QgsVectorFileWriter.String = QgsVectorFileWriter.OptionType.String
QgsVectorFileWriter.Int = QgsVectorFileWriter.OptionType.Int
QgsVectorFileWriter.Hidden = QgsVectorFileWriter.OptionType.Hidden
QgsVectorFileWriter.NoError = QgsVectorFileWriter.WriterError.NoError
QgsVectorFileWriter.ErrDriverNotFound = QgsVectorFileWriter.WriterError.ErrDriverNotFound
QgsVectorFileWriter.ErrCreateDataSource = QgsVectorFileWriter.WriterError.ErrCreateDataSource
QgsVectorFileWriter.ErrCreateLayer = QgsVectorFileWriter.WriterError.ErrCreateLayer
QgsVectorFileWriter.ErrAttributeTypeUnsupported = QgsVectorFileWriter.WriterError.ErrAttributeTypeUnsupported
QgsVectorFileWriter.ErrAttributeCreationFailed = QgsVectorFileWriter.WriterError.ErrAttributeCreationFailed
QgsVectorFileWriter.ErrProjection = QgsVectorFileWriter.WriterError.ErrProjection
QgsVectorFileWriter.ErrFeatureWriteFailed = QgsVectorFileWriter.WriterError.ErrFeatureWriteFailed
QgsVectorFileWriter.ErrInvalidLayer = QgsVectorFileWriter.WriterError.ErrInvalidLayer
QgsVectorFileWriter.ErrSavingMetadata = QgsVectorFileWriter.WriterError.ErrSavingMetadata
QgsVectorFileWriter.Canceled = QgsVectorFileWriter.WriterError.Canceled
QgsVectorFileWriter.Original = QgsVectorFileWriter.FieldNameSource.Original
QgsVectorFileWriter.PreferAlias = QgsVectorFileWriter.FieldNameSource.PreferAlias
QgsVectorFileWriter.SortRecommended = QgsVectorFileWriter.VectorFormatOption.SortRecommended
QgsVectorFileWriter.SkipNonSpatialFormats = QgsVectorFileWriter.VectorFormatOption.SkipNonSpatialFormats
QgsVectorFileWriter.SupportsMultipleLayers = QgsVectorFileWriter.VectorFormatOption.SupportsMultipleLayers
QgsVectorFileWriter.VectorFormatOptions = lambda flags=0: QgsVectorFileWriter.VectorFormatOption(flags)
QgsVectorFileWriter.CanAddNewLayer = QgsVectorFileWriter.EditionCapability.CanAddNewLayer
QgsVectorFileWriter.CanAppendToExistingLayer = QgsVectorFileWriter.EditionCapability.CanAppendToExistingLayer
QgsVectorFileWriter.CanAddNewFieldsToExistingLayer = QgsVectorFileWriter.EditionCapability.CanAddNewFieldsToExistingLayer
QgsVectorFileWriter.CanDeleteLayer = QgsVectorFileWriter.EditionCapability.CanDeleteLayer
QgsVectorFileWriter.EditionCapabilities = lambda flags=0: QgsVectorFileWriter.EditionCapability(flags)
QgsVectorFileWriter.CreateOrOverwriteFile = QgsVectorFileWriter.ActionOnExistingFile.CreateOrOverwriteFile
QgsVectorFileWriter.CreateOrOverwriteLayer = QgsVectorFileWriter.ActionOnExistingFile.CreateOrOverwriteLayer
QgsVectorFileWriter.AppendToLayerNoNewFields = QgsVectorFileWriter.ActionOnExistingFile.AppendToLayerNoNewFields
QgsVectorFileWriter.AppendToLayerAddFields = QgsVectorFileWriter.ActionOnExistingFile.AppendToLayerAddFields
from enum import Enum


def _force_int(v): return int(v.value) if isinstance(v, Enum) else v


QgsVectorFileWriter.EditionCapability.__bool__ = lambda flag: bool(_force_int(flag))
QgsVectorFileWriter.EditionCapability.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsVectorFileWriter.EditionCapability.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsVectorFileWriter.EditionCapability.__or__ = lambda flag1, flag2: QgsVectorFileWriter.EditionCapability(_force_int(flag1) | _force_int(flag2))
QgsVectorFileWriter.VectorFormatOption.__bool__ = lambda flag: bool(_force_int(flag))
QgsVectorFileWriter.VectorFormatOption.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsVectorFileWriter.VectorFormatOption.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsVectorFileWriter.VectorFormatOption.__or__ = lambda flag1, flag2: QgsVectorFileWriter.VectorFormatOption(_force_int(flag1) | _force_int(flag2))
