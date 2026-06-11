/***************************************************************************
    qgsaiembeddingprovider.cpp
    --------------------------
    begin                : June 2026
    copyright            : (C) 2026 by Francesco Mazzi
    email                : francemazzi at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsaiembeddingprovider.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "qgsaiembeddingclient.h"
#include "qgsapplication.h"
#include "qgssettings.h"

#include <QString>

#ifdef HAVE_AI_E5_EMBEDDINGS
#include <onnxruntime_cxx_api.h>
#include <sentencepiece_processor.h>
#endif

#include <QByteArray>
#include <QChar>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileDevice>
#include <QFileInfo>
#include <QMutexLocker>
#include <QObject>
#include <QThread>

using namespace Qt::StringLiterals;

namespace
{
  constexpr int LOCAL_DIMENSION = 384;
#ifdef HAVE_AI_E5_EMBEDDINGS
  constexpr int E5_DIMENSION = 384;
  constexpr int E5_MAX_SEQUENCE_LENGTH = 512;
#endif
  constexpr qint64 E5_ONNX_SIZE = 118346824;
  constexpr qint64 E5_SENTENCEPIECE_SIZE = 5069051;
  constexpr const char *INDEX_PROVIDER_SETTING = "strata/index/embedding_provider";
  constexpr const char *LEGACY_EMBEDDINGS_PROVIDER_SETTING = "ai/embeddings/provider";
  constexpr const char *E5_PROVIDER_ID = "local:e5-small-int8-384";
  constexpr const char *E5_MODEL_ID = "intfloat/multilingual-e5-small";
  constexpr const char *E5_MODEL_REVISION = "614241f6-qint8-e5-prefix-mean-l2-v1";
  constexpr const char *E5_HF_REVISION = "614241f622f53c4eeff9890bdc4f31cfecc418b3";
  constexpr const char *E5_ONNX_RELATIVE_PATH = "onnx/model_qint8_avx512_vnni.onnx";
  constexpr const char *E5_ONNX_FILE_NAME = "model_qint8_avx512_vnni.onnx";
  constexpr const char *E5_SENTENCEPIECE_RELATIVE_PATH = "sentencepiece.bpe.model";
  constexpr const char *E5_SENTENCEPIECE_ONNX_RELATIVE_PATH = "onnx/sentencepiece.bpe.model";
  constexpr const char *E5_ONNX_SHA256 = "dd476dd0c2514e9b9be83aeb3853fac0763e0bdf4a71645407587d77c48a2d88";
  constexpr const char *E5_SENTENCEPIECE_SHA256 = "cfc8146abe2a0488e9e2a0c56de7952f7c11ab059eca145a0a727afce0db2865";

  quint64 stableHash64( const QString &value )
  {
    const QByteArray digest = QCryptographicHash::hash( value.toUtf8(), QCryptographicHash::Sha1 );
    quint64 out = 0;
    const int bytes = std::min<int>( 8, digest.size() );
    for ( int i = 0; i < bytes; ++i )
      out = ( out << 8 ) | static_cast<unsigned char>( digest.at( i ) );
    return out;
  }

  void addFeature( QVector<float> &vector, const QString &feature, float weight )
  {
    const quint64 h = stableHash64( feature );
    const int idx = static_cast<int>( h % static_cast<quint64>( vector.size() ) );
    const float sign = ( h & 0x100000000ULL ) ? -1.0f : 1.0f;
    vector[idx] += sign * weight;
  }

  QStringList tokenize( const QString &text )
  {
    QString normalized = text.toLower().normalized( QString::NormalizationForm_KD );
    QStringList tokens;
    QString current;
    for ( const QChar ch : normalized )
    {
      if ( ch.isLetterOrNumber() )
      {
        current.append( ch );
      }
      else if ( !current.isEmpty() )
      {
        tokens.append( current );
        current.clear();
      }
    }
    if ( !current.isEmpty() )
      tokens.append( current );
    return tokens;
  }

  QString cleanExistingE5Prefix( const QString &text )
  {
    QString cleaned = text.trimmed();
    while ( true )
    {
      if ( cleaned.startsWith( u"query:"_s, Qt::CaseInsensitive ) )
      {
        cleaned = cleaned.mid( 6 ).trimmed();
      }
      else if ( cleaned.startsWith( u"passage:"_s, Qt::CaseInsensitive ) )
      {
        cleaned = cleaned.mid( 8 ).trimmed();
      }
      else
      {
        break;
      }
    }
    return cleaned;
  }

  QString modelDownloadUrl( const QString &relativePath )
  {
    return u"https://huggingface.co/intfloat/multilingual-e5-small/resolve/%1/%2"_s.arg( QString::fromLatin1( E5_HF_REVISION ), relativePath );
  }

#ifdef HAVE_AI_E5_EMBEDDINGS
  std::string pathForOrt( const QString &path )
  {
    return QFile::encodeName( QDir::toNativeSeparators( path ) ).toStdString();
  }
#endif

  class RemoteEmbeddingProvider final : public QgsAiEmbeddingProvider
  {
    public:
      explicit RemoteEmbeddingProvider( QgsAiEmbeddingClient::Provider provider, QObject *parent )
        : mProvider( provider )
      {
        Q_UNUSED( parent )
        mClient.setProvider( provider );
      }

      QString providerId() const override { return mProvider == QgsAiEmbeddingClient::Provider::OpenRouter ? u"openrouter"_s : u"openai"_s; }

      QString displayName() const override { return mProvider == QgsAiEmbeddingClient::Provider::OpenRouter ? u"OpenRouter"_s : u"OpenAI"_s; }

      QString modelId() const override { return mClient.model(); }
      QString modelRevision() const override { return u"remote"_s; }
      bool isRemote() const override { return true; }

      bool isAvailable( QString *errorMessage = nullptr ) const override
      {
        if ( mClient.hasApiKey() )
          return true;

        if ( errorMessage )
        {
          *errorMessage = mProvider == QgsAiEmbeddingClient::Provider::OpenRouter ? u"OpenRouter embedding provider is selected, but no OpenRouter API key is configured."_s
                                                                                  : u"OpenAI embedding provider is selected, but no OpenAI API key is configured."_s;
        }
        return false;
      }

      bool embed( const QStringList &texts, QList<QVector<float>> &out, QString *errorMessage = nullptr, int maxBatch = 64 ) override { return mClient.embed( texts, out, errorMessage, maxBatch ); }

      bool embed( const QStringList &texts, QgsAiEmbeddingRole role, QList<QVector<float>> &out, QString *errorMessage = nullptr, const QgsAiEmbeddingOptions &options = QgsAiEmbeddingOptions() ) override
      {
        Q_UNUSED( role )
        return embed( texts, out, errorMessage, options.maxBatch );
      }

    private:
      mutable QgsAiEmbeddingClient mClient;
      QgsAiEmbeddingClient::Provider mProvider = QgsAiEmbeddingClient::Provider::OpenAi;
  };
} // namespace

#ifdef HAVE_AI_E5_EMBEDDINGS
struct QgsAiE5EmbeddingProvider::Runtime
{
    Runtime()
      : env( ORT_LOGGING_LEVEL_WARNING, "StrataE5Embeddings" )
    {}

    Ort::Env env;
    std::unique_ptr<Ort::Session> session;
    std::unique_ptr<sentencepiece::SentencePieceProcessor> tokenizer;
    std::vector<std::string> inputNames;
    std::vector<std::string> outputNames;
    int outputIndex = 0;
};
#else
struct QgsAiE5EmbeddingProvider::Runtime
{};
#endif

QgsAiE5EmbeddingProvider::QgsAiE5EmbeddingProvider() = default;

QgsAiE5EmbeddingProvider::~QgsAiE5EmbeddingProvider() = default;

QString QgsAiE5EmbeddingProvider::staticProviderId()
{
  return QString::fromLatin1( E5_PROVIDER_ID );
}

QString QgsAiE5EmbeddingProvider::modelName()
{
  return QString::fromLatin1( E5_MODEL_ID );
}

QString QgsAiE5EmbeddingProvider::pinnedModelRevision()
{
  return QString::fromLatin1( E5_MODEL_REVISION );
}

QString QgsAiE5EmbeddingProvider::developerModelDirectory()
{
  const QString dir = qEnvironmentVariable( "STRATA_AI_EMBEDDING_MODEL_DIR" ).trimmed();
  return dir.isEmpty() ? QString() : QDir::cleanPath( dir );
}

QString QgsAiE5EmbeddingProvider::userModelDirectory()
{
  return QDir( QgsApplication::qgisSettingsDirPath() + u"ai_models"_s ).filePath( u"multilingual-e5-small-int8"_s );
}

QString QgsAiE5EmbeddingProvider::packagedModelDirectory()
{
  return QDir( QgsApplication::pkgDataPath() ).filePath( u"resources/ai_models/multilingual-e5-small-int8"_s );
}

QString QgsAiE5EmbeddingProvider::activeModelDirectory()
{
  const QString developerDir = developerModelDirectory();
  if ( !developerDir.isEmpty() )
    return developerDir;

  const QString userDir = userModelDirectory();
  if ( modelFilesAvailable( userDir ) )
    return userDir;

  const QString packagedDir = packagedModelDirectory();
  if ( modelFilesAvailable( packagedDir ) )
    return packagedDir;

  return userDir;
}

QString QgsAiE5EmbeddingProvider::modelPath( const QString &modelDirectory )
{
  const QDir dir( modelDirectory );
  const QString nested = dir.filePath( QString::fromLatin1( E5_ONNX_RELATIVE_PATH ) );
  if ( QFileInfo::exists( nested ) )
    return nested;
  return dir.filePath( QString::fromLatin1( E5_ONNX_FILE_NAME ) );
}

QString QgsAiE5EmbeddingProvider::tokenizerPath( const QString &modelDirectory )
{
  const QDir dir( modelDirectory );
  const QString root = dir.filePath( QString::fromLatin1( E5_SENTENCEPIECE_RELATIVE_PATH ) );
  if ( QFileInfo::exists( root ) )
    return root;
  return dir.filePath( QString::fromLatin1( E5_SENTENCEPIECE_ONNX_RELATIVE_PATH ) );
}

bool QgsAiE5EmbeddingProvider::modelFilesAvailable( const QString &modelDirectory, QString *errorMessage )
{
  if ( modelDirectory.trimmed().isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = u"Local E5 embedding model directory is not configured."_s;
    return false;
  }

  const QString onnxPath = modelPath( modelDirectory );
  const QString spPath = tokenizerPath( modelDirectory );
  if ( !QFileInfo::exists( onnxPath ) )
  {
    if ( errorMessage )
      *errorMessage = u"Local E5 embedding model is missing ONNX file: %1"_s.arg( onnxPath );
    return false;
  }
  if ( !QFileInfo::exists( spPath ) )
  {
    if ( errorMessage )
      *errorMessage = u"Local E5 embedding model is missing SentencePiece file: %1"_s.arg( spPath );
    return false;
  }
  return true;
}

QList<QgsAiEmbeddingModelDownloadFile> QgsAiE5EmbeddingProvider::downloadFiles()
{
  return {
    { QString::fromLatin1( E5_ONNX_RELATIVE_PATH ), modelDownloadUrl( QString::fromLatin1( E5_ONNX_RELATIVE_PATH ) ), QString::fromLatin1( E5_ONNX_SHA256 ), E5_ONNX_SIZE },
    { QString::fromLatin1( E5_SENTENCEPIECE_RELATIVE_PATH ),
      modelDownloadUrl( QString::fromLatin1( E5_SENTENCEPIECE_RELATIVE_PATH ) ),
      QString::fromLatin1( E5_SENTENCEPIECE_SHA256 ),
      E5_SENTENCEPIECE_SIZE },
  };
}

qint64 QgsAiE5EmbeddingProvider::downloadSize()
{
  qint64 size = 0;
  for ( const QgsAiEmbeddingModelDownloadFile &file : downloadFiles() )
    size += file.size;
  return size;
}

QString QgsAiE5EmbeddingProvider::formatInputForRole( const QString &text, QgsAiEmbeddingRole role )
{
  return ( role == QgsAiEmbeddingRole::Query ? u"query: "_s : u"passage: "_s ) + cleanExistingE5Prefix( text );
}

QVector<qint64> QgsAiE5EmbeddingProvider::tokenIdsWithSpecials( const QVector<int> &pieceIds, int maxSequenceLength )
{
  const int sequenceLength = std::max( 2, maxSequenceLength );
  QVector<qint64> ids;
  ids.reserve( sequenceLength );
  ids.append( 0 );

  const int pieceLimit = std::max( 0, sequenceLength - 2 );
  for ( int i = 0; i < pieceIds.size() && i < pieceLimit; ++i )
    ids.append( static_cast<qint64>( pieceIds.at( i ) ) );

  ids.append( 2 );
  return ids;
}

QVector<float> QgsAiE5EmbeddingProvider::meanPoolAndNormalize( const QVector<float> &lastHiddenStates, const QVector<qint64> &attentionMask, int hiddenSize )
{
  if ( hiddenSize <= 0 )
    return {};

  QVector<float> pooled( hiddenSize );
  if ( attentionMask.isEmpty() || lastHiddenStates.size() < attentionMask.size() * hiddenSize )
    return pooled;

  int activeTokens = 0;
  for ( int token = 0; token < attentionMask.size(); ++token )
  {
    if ( attentionMask.at( token ) == 0 )
      continue;

    ++activeTokens;
    const int offset = token * hiddenSize;
    for ( int dim = 0; dim < hiddenSize; ++dim )
      pooled[dim] += lastHiddenStates.at( offset + dim );
  }

  if ( activeTokens > 0 )
  {
    const float inv = 1.0f / static_cast<float>( activeTokens );
    for ( float &value : pooled )
      value *= inv;
  }

  double norm = 0.0;
  for ( float value : pooled )
    norm += static_cast<double>( value ) * static_cast<double>( value );

  if ( norm > 0.0 )
  {
    const float invNorm = static_cast<float>( 1.0 / std::sqrt( norm ) );
    for ( float &value : pooled )
      value *= invNorm;
  }

  return pooled;
}

QString QgsAiE5EmbeddingProvider::fileSha256( const QString &path, QString *errorMessage )
{
  QFile file( path );
  if ( !file.open( QIODevice::ReadOnly ) )
  {
    if ( errorMessage )
      *errorMessage = u"Cannot open %1 for SHA-256 verification."_s.arg( path );
    return QString();
  }

  QCryptographicHash hash( QCryptographicHash::Sha256 );
  while ( !file.atEnd() )
  {
    const QByteArray chunk = file.read( 1024 * 1024 );
    if ( chunk.isEmpty() && file.error() != QFileDevice::NoError )
    {
      if ( errorMessage )
        *errorMessage = u"Cannot read %1 for SHA-256 verification: %2"_s.arg( path, file.errorString() );
      return QString();
    }
    hash.addData( chunk );
  }

  return QString::fromLatin1( hash.result().toHex() );
}

bool QgsAiE5EmbeddingProvider::fileMatchesSha256( const QString &path, const QString &expectedSha256, QString *errorMessage )
{
  QString hashError;
  const QString actual = fileSha256( path, &hashError );
  if ( actual.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = hashError;
    return false;
  }

  if ( actual.compare( expectedSha256, Qt::CaseInsensitive ) != 0 )
  {
    if ( errorMessage )
      *errorMessage = u"SHA-256 mismatch for %1: expected %2, got %3."_s.arg( path, expectedSha256, actual );
    return false;
  }

  return true;
}

bool QgsAiE5EmbeddingProvider::ensureRuntime( QString *errorMessage ) const
{
  QMutexLocker locker( &mRuntimeMutex );

#ifndef HAVE_AI_E5_EMBEDDINGS
  mRuntimeError = u"Local multilingual E5 embedding support was not compiled because ONNX Runtime and/or SentencePiece were not found."_s;
  mRuntimeLoadAttempted = true;
  if ( errorMessage )
    *errorMessage = mRuntimeError;
  return false;
#else
  if ( mRuntime )
    return true;

  if ( mRuntimeLoadAttempted )
  {
    if ( errorMessage )
      *errorMessage = mRuntimeError;
    return false;
  }

  mRuntimeLoadAttempted = true;

  const QString modelDir = activeModelDirectory();
  QString filesError;
  if ( !modelFilesAvailable( modelDir, &filesError ) )
  {
    const QString developerDir = developerModelDirectory();
    mRuntimeError
      = developerDir.isEmpty()
          ? u"Local multilingual E5 embedding model is not installed. Download it from the AI settings dialog or set STRATA_AI_EMBEDDING_MODEL_DIR for development builds. Expected cache: %1"_s.arg(
              userModelDirectory()
            )
          : u"STRATA_AI_EMBEDDING_MODEL_DIR is set to %1, but the local multilingual E5 files are not usable: %2"_s.arg( developerDir, filesError );
    if ( errorMessage )
      *errorMessage = mRuntimeError;
    return false;
  }

  const QString onnxPath = modelPath( modelDir );
  const QString spPath = tokenizerPath( modelDir );

  try
  {
    auto runtime = std::make_unique<Runtime>();

    runtime->tokenizer = std::make_unique<sentencepiece::SentencePieceProcessor>();
    const auto tokenizerStatus = runtime->tokenizer->Load( QFile::encodeName( spPath ).toStdString() );
    if ( !tokenizerStatus.ok() )
    {
      mRuntimeError = u"Failed to load multilingual E5 SentencePiece tokenizer: %1"_s.arg( QString::fromStdString( tokenizerStatus.ToString() ) );
      if ( errorMessage )
        *errorMessage = mRuntimeError;
      return false;
    }

    Ort::SessionOptions sessionOptions;
    sessionOptions.SetGraphOptimizationLevel( GraphOptimizationLevel::ORT_ENABLE_ALL );
    sessionOptions.SetIntraOpNumThreads( std::max( 1, std::min( 4, QThread::idealThreadCount() ) ) );

#ifdef _WIN32
    const std::wstring ortModelPath = QDir::toNativeSeparators( onnxPath ).toStdWString();
    runtime->session = std::make_unique<Ort::Session>( runtime->env, ortModelPath.c_str(), sessionOptions );
#else
    const std::string ortModelPath = pathForOrt( onnxPath );
    runtime->session = std::make_unique<Ort::Session>( runtime->env, ortModelPath.c_str(), sessionOptions );
#endif

    Ort::AllocatorWithDefaultOptions allocator;
    const size_t inputCount = runtime->session->GetInputCount();
    for ( size_t i = 0; i < inputCount; ++i )
    {
      Ort::AllocatedStringPtr name = runtime->session->GetInputNameAllocated( i, allocator );
      runtime->inputNames.emplace_back( name.get() );
    }

    const size_t outputCount = runtime->session->GetOutputCount();
    for ( size_t i = 0; i < outputCount; ++i )
    {
      Ort::AllocatedStringPtr name = runtime->session->GetOutputNameAllocated( i, allocator );
      runtime->outputNames.emplace_back( name.get() );
      if ( runtime->outputNames.back() == "last_hidden_state" )
        runtime->outputIndex = static_cast<int>( i );
    }

    if ( runtime->inputNames.empty() || runtime->outputNames.empty() )
    {
      mRuntimeError = u"Failed to load multilingual E5 ONNX model: model has no usable inputs or outputs."_s;
      if ( errorMessage )
        *errorMessage = mRuntimeError;
      return false;
    }

    mRuntime = std::move( runtime );
    mRuntimeError.clear();
    return true;
  }
  catch ( const Ort::Exception &e )
  {
    mRuntimeError = u"Failed to load multilingual E5 ONNX model: %1"_s.arg( QString::fromUtf8( e.what() ) );
  }
  catch ( const std::exception &e )
  {
    mRuntimeError = u"Failed to load multilingual E5 embedding model: %1"_s.arg( QString::fromUtf8( e.what() ) );
  }

  if ( errorMessage )
    *errorMessage = mRuntimeError;
  return false;
#endif
}

bool QgsAiE5EmbeddingProvider::isAvailable( QString *errorMessage ) const
{
  return ensureRuntime( errorMessage );
}

bool QgsAiE5EmbeddingProvider::embed( const QStringList &texts, QList<QVector<float>> &out, QString *errorMessage, int maxBatch )
{
  QgsAiEmbeddingOptions options;
  options.maxBatch = maxBatch;
  return embed( texts, QgsAiEmbeddingRole::Passage, out, errorMessage, options );
}

bool QgsAiE5EmbeddingProvider::embed( const QStringList &texts, QgsAiEmbeddingRole role, QList<QVector<float>> &out, QString *errorMessage, const QgsAiEmbeddingOptions &options )
{
  out.clear();
  if ( texts.isEmpty() )
    return true;

  if ( !ensureRuntime( errorMessage ) )
    return false;

#ifndef HAVE_AI_E5_EMBEDDINGS
  Q_UNUSED( role )
  Q_UNUSED( options )
  return false;
#else
  QMutexLocker locker( &mRuntimeMutex );
  const int requestedBatch = options.maxBatch > 0 ? options.maxBatch : 1;
  const int batchLimit = std::max( 1, requestedBatch );
  const int textCount = static_cast<int>( texts.size() );
  out.reserve( texts.size() );

  Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu( OrtArenaAllocator, OrtMemTypeDefault );

  for ( int start = 0; start < textCount; start += batchLimit )
  {
    const int batchSize = std::min( batchLimit, textCount - start );
    QVector<QVector<qint64>> batchTokenIds;
    batchTokenIds.reserve( batchSize );
    int sequenceLength = 0;

    for ( int i = 0; i < batchSize; ++i )
    {
      std::vector<int> pieceIds;
      const QString formatted = formatInputForRole( texts.at( start + i ), role );
      const auto encodeStatus = mRuntime->tokenizer->Encode( formatted.toStdString(), &pieceIds );
      if ( !encodeStatus.ok() )
      {
        if ( errorMessage )
          *errorMessage = u"Failed to tokenize text for multilingual E5 embeddings: %1"_s.arg( QString::fromStdString( encodeStatus.ToString() ) );
        return false;
      }

      QVector<int> qPieceIds;
      qPieceIds.reserve( static_cast<int>( pieceIds.size() ) );
      for ( const int pieceId : pieceIds )
        qPieceIds.append( pieceId );
      batchTokenIds.append( tokenIdsWithSpecials( qPieceIds, E5_MAX_SEQUENCE_LENGTH ) );
      sequenceLength = std::max( sequenceLength, static_cast<int>( batchTokenIds.constLast().size() ) );
    }

    std::vector<int64_t> inputIds( static_cast<size_t>( batchSize * sequenceLength ), 1 );
    std::vector<int64_t> attentionMask( static_cast<size_t>( batchSize * sequenceLength ), 0 );
    std::vector<int64_t> tokenTypeIds( static_cast<size_t>( batchSize * sequenceLength ), 0 );

    for ( int row = 0; row < batchSize; ++row )
    {
      const QVector<qint64> &ids = batchTokenIds.at( row );
      for ( int col = 0; col < ids.size(); ++col )
      {
        const size_t offset = static_cast<size_t>( row * sequenceLength + col );
        inputIds[offset] = static_cast<int64_t>( ids.at( col ) );
        attentionMask[offset] = 1;
      }
    }

    const std::array<int64_t, 2> inputShape = { static_cast<int64_t>( batchSize ), static_cast<int64_t>( sequenceLength ) };
    std::vector<Ort::Value> inputTensors;
    std::vector<const char *> inputNamePtrs;
    inputTensors.reserve( mRuntime->inputNames.size() );
    inputNamePtrs.reserve( mRuntime->inputNames.size() );

    for ( const std::string &inputName : mRuntime->inputNames )
    {
      inputNamePtrs.push_back( inputName.c_str() );
      if ( inputName == "attention_mask" )
      {
        inputTensors.push_back( Ort::Value::CreateTensor<int64_t>( memoryInfo, attentionMask.data(), attentionMask.size(), inputShape.data(), inputShape.size() ) );
      }
      else if ( inputName == "token_type_ids" )
      {
        inputTensors.push_back( Ort::Value::CreateTensor<int64_t>( memoryInfo, tokenTypeIds.data(), tokenTypeIds.size(), inputShape.data(), inputShape.size() ) );
      }
      else
      {
        inputTensors.push_back( Ort::Value::CreateTensor<int64_t>( memoryInfo, inputIds.data(), inputIds.size(), inputShape.data(), inputShape.size() ) );
      }
    }

    const std::string &outputName = mRuntime->outputNames.at( mRuntime->outputIndex );
    const char *outputNamePtr = outputName.c_str();
    std::vector<Ort::Value> outputTensors;
    try
    {
      outputTensors = mRuntime->session->Run( Ort::RunOptions { nullptr }, inputNamePtrs.data(), inputTensors.data(), inputTensors.size(), &outputNamePtr, 1 );
    }
    catch ( const Ort::Exception &e )
    {
      if ( errorMessage )
        *errorMessage = u"Multilingual E5 ONNX inference failed: %1"_s.arg( QString::fromUtf8( e.what() ) );
      return false;
    }

    if ( outputTensors.empty() || !outputTensors.front().IsTensor() )
    {
      if ( errorMessage )
        *errorMessage = u"Multilingual E5 ONNX inference returned no tensor output."_s;
      return false;
    }

    Ort::TensorTypeAndShapeInfo outputInfo = outputTensors.front().GetTensorTypeAndShapeInfo();
    const std::vector<int64_t> outputShape = outputInfo.GetShape();
    if ( outputShape.size() != 3 || outputShape.at( 0 ) != batchSize || outputShape.at( 1 ) <= 0 || outputShape.at( 2 ) <= 0 )
    {
      if ( errorMessage )
        *errorMessage = u"Multilingual E5 ONNX output has unexpected shape."_s;
      return false;
    }

    const int outputSequenceLength = static_cast<int>( outputShape.at( 1 ) );
    const int hiddenSize = static_cast<int>( outputShape.at( 2 ) );
    if ( hiddenSize != E5_DIMENSION )
    {
      if ( errorMessage )
        *errorMessage = u"Multilingual E5 ONNX output dimension mismatch: expected %1 got %2."_s.arg( E5_DIMENSION ).arg( hiddenSize );
      return false;
    }

    const float *outputData = outputTensors.front().GetTensorData<float>();
    for ( int row = 0; row < batchSize; ++row )
    {
      QVector<float> sampleStates( outputSequenceLength * hiddenSize );
      const float *rowStart = outputData + static_cast<size_t>( row * outputSequenceLength * hiddenSize );
      std::copy( rowStart, rowStart + sampleStates.size(), sampleStates.begin() );

      QVector<qint64> sampleMask( outputSequenceLength );
      for ( int col = 0; col < outputSequenceLength && col < sequenceLength; ++col )
        sampleMask[col] = static_cast<qint64>( attentionMask.at( static_cast<size_t>( row * sequenceLength + col ) ) );

      out.append( meanPoolAndNormalize( sampleStates, sampleMask, hiddenSize ) );
    }
  }

  return out.size() == texts.size();
#endif
}

bool QgsAiLocalEmbeddingProvider::isAvailable( QString *errorMessage ) const
{
  Q_UNUSED( errorMessage )
  return true;
}

bool QgsAiLocalEmbeddingProvider::embed( const QStringList &texts, QList<QVector<float>> &out, QString *errorMessage, int maxBatch )
{
  QgsAiEmbeddingOptions options;
  options.maxBatch = maxBatch;
  return embed( texts, QgsAiEmbeddingRole::Passage, out, errorMessage, options );
}

bool QgsAiLocalEmbeddingProvider::embed( const QStringList &texts, QgsAiEmbeddingRole role, QList<QVector<float>> &out, QString *errorMessage, const QgsAiEmbeddingOptions &options )
{
  Q_UNUSED( errorMessage )
  Q_UNUSED( options )
  out.clear();
  out.reserve( texts.size() );
  for ( const QString &text : texts )
    out.append( embedOne( text, role ) );
  return true;
}

QVector<float> QgsAiLocalEmbeddingProvider::embedOne( const QString &text, QgsAiEmbeddingRole role ) const
{
  QVector<float> vector( LOCAL_DIMENSION );
  const QStringList tokens = tokenize( text );

  addFeature( vector, role == QgsAiEmbeddingRole::Query ? u"role:query"_s : u"role:passage"_s, 0.05f );

  for ( int i = 0; i < tokens.size(); ++i )
  {
    const QString &token = tokens.at( i );
    addFeature( vector, u"w:"_s + token, 1.0f );

    if ( i + 1 < tokens.size() )
      addFeature( vector, u"b:"_s + token + u" "_s + tokens.at( i + 1 ), 0.8f );

    if ( token.size() >= 3 )
    {
      for ( int j = 0; j <= token.size() - 3; ++j )
        addFeature( vector, u"c:"_s + token.mid( j, 3 ), 0.25f );
    }
  }

  if ( tokens.isEmpty() )
  {
    const QString trimmed = text.trimmed().toLower();
    for ( int i = 0; i < trimmed.size(); ++i )
      addFeature( vector, u"u:"_s + trimmed.mid( i, 1 ), 0.2f );
  }

  double norm = 0.0;
  for ( float v : vector )
    norm += static_cast<double>( v ) * static_cast<double>( v );
  if ( norm > 0.0 )
  {
    const float inv = static_cast<float>( 1.0 / std::sqrt( norm ) );
    for ( float &v : vector )
      v *= inv;
  }
  return vector;
}

QString QgsAiEmbeddingProviderRegistry::defaultProviderId()
{
#ifdef HAVE_AI_E5_EMBEDDINGS
  return QgsAiE5EmbeddingProvider::staticProviderId();
#else
  return u"local:minihash-384"_s;
#endif
}

QString QgsAiEmbeddingProviderRegistry::configuredProviderId()
{
  const QgsSettings settings;
  const QString configured = settings.value( QString::fromLatin1( INDEX_PROVIDER_SETTING ), defaultProviderId() ).toString().trimmed().toLower();
  if ( providerIds().contains( configured ) )
    return configured;

  // Legacy remote embedding settings must not implicitly enable remote
  // indexing. They are ignored unless copied to the new Strata index key.
  Q_UNUSED( LEGACY_EMBEDDINGS_PROVIDER_SETTING )
  return defaultProviderId();
}

void QgsAiEmbeddingProviderRegistry::setConfiguredProviderId( const QString &providerId )
{
  QgsSettings settings;
  const QString normalized = providerId.trimmed().toLower();
  settings.setValue( QString::fromLatin1( INDEX_PROVIDER_SETTING ), providerIds().contains( normalized ) ? normalized : defaultProviderId() );
}

QStringList QgsAiEmbeddingProviderRegistry::providerIds()
{
  QStringList ids;
#ifdef HAVE_AI_E5_EMBEDDINGS
  ids << QgsAiE5EmbeddingProvider::staticProviderId();
#endif
  ids << u"local:minihash-384"_s << u"openai"_s << u"openrouter"_s;
  return ids;
}

QList<QgsAiEmbeddingProviderUiEntry> QgsAiEmbeddingProviderRegistry::providerUiEntries()
{
  QList<QgsAiEmbeddingProviderUiEntry> entries;

#ifdef HAVE_AI_E5_EMBEDDINGS
  entries.append( { QgsAiE5EmbeddingProvider::staticProviderId(), u"Local multilingual E5 small (recommended)"_s, true, QString() } );
#else
  entries.append( {
    QgsAiE5EmbeddingProvider::staticProviderId(),
    u"Local multilingual E5 small (not compiled)"_s,
    false,
    u"Local multilingual E5 requires ONNX Runtime and SentencePiece support in this build."_s,
  } );
#endif

  entries.append( { u"local:minihash-384"_s, u"Local MinHash fallback"_s, true, QString() } );
  entries.append( { u"openai"_s, u"OpenAI"_s, true, QString() } );
  entries.append( { u"openrouter"_s, u"OpenRouter"_s, true, QString() } );
  return entries;
}

QString QgsAiEmbeddingProviderRegistry::displayNameForProviderId( const QString &providerId )
{
  const QString normalized = providerId.trimmed().toLower();
  if ( normalized == QgsAiE5EmbeddingProvider::staticProviderId() )
  {
#ifdef HAVE_AI_E5_EMBEDDINGS
    return u"Local multilingual E5 small (recommended)"_s;
#else
    return u"Local multilingual E5 small (not compiled)"_s;
#endif
  }
  if ( normalized == "local:minihash-384"_L1 )
    return u"Local MinHash fallback"_s;
  if ( normalized == "openai"_L1 )
    return u"OpenAI"_s;
  if ( normalized == "openrouter"_L1 )
    return u"OpenRouter"_s;
  return u"Local multilingual E5 small (recommended)"_s;
}

bool QgsAiEmbeddingProviderRegistry::isRemoteProviderId( const QString &providerId )
{
  const QString normalized = providerId.trimmed().toLower();
  return normalized == "openai"_L1 || normalized == "openrouter"_L1;
}

std::unique_ptr<QgsAiEmbeddingProvider> QgsAiEmbeddingProviderRegistry::createProviderFromSettings( QObject *parent )
{
  return createProvider( configuredProviderId(), parent );
}

std::unique_ptr<QgsAiEmbeddingProvider> QgsAiEmbeddingProviderRegistry::createProvider( const QString &providerId, QObject *parent )
{
  const QString normalized = providerId.trimmed().toLower();
  if ( normalized == "openai"_L1 )
    return std::make_unique<RemoteEmbeddingProvider>( QgsAiEmbeddingClient::Provider::OpenAi, parent );
  if ( normalized == "openrouter"_L1 )
    return std::make_unique<RemoteEmbeddingProvider>( QgsAiEmbeddingClient::Provider::OpenRouter, parent );
  if ( normalized == QgsAiE5EmbeddingProvider::staticProviderId() )
  {
#ifdef HAVE_AI_E5_EMBEDDINGS
    Q_UNUSED( parent )
    return std::make_unique<QgsAiE5EmbeddingProvider>();
#else
    Q_UNUSED( parent )
    return std::make_unique<QgsAiLocalEmbeddingProvider>();
#endif
  }
  if ( normalized == "local:minihash-384"_L1 )
    return std::make_unique<QgsAiLocalEmbeddingProvider>();
  Q_UNUSED( parent )
  return std::make_unique<QgsAiLocalEmbeddingProvider>();
}
