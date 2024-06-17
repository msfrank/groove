//#include <tempo_utils/logging.h>
//
//#include "multi_pull_request.h"
//#include "double_data_stream.h"
//#include "int64_data_stream.h"
//#include "category_data_stream.h"
//
//MultiPullRequest::MultiPullRequest(
//    const tempo_utils::RequestId &requestId,
//    const SourceDataRanges &ranges,
//    const StreamSourceCollection &sources,
//    QObject *parent)
//    : QObject(parent)
//{
//    m_id = requestId;
//    m_ranges = ranges;
//    m_sources = sources;
//    m_running = false;
//    m_complete = false;
//    m_timerId = 0;
//
//    for (const auto &sourceId : sources.listSourceIds()) {
//        auto source = sources.getSource(sourceId);
//        auto stream = source.getStream();
//        switch (stream->getStreamType()) {
//            case streams::DataStreamType::STREAM_DOUBLE: {
//                auto dbl = std::static_pointer_cast<DoubleDataStream>(stream);
//                connect(dbl.get(), &DoubleDataStream::getDoubleDataResult,
//                    this, &MultiPullRequest::onGetDoubleDataResult);
//                connect(dbl.get(), &DoubleDataStream::getDoubleDataError,
//                    this, &MultiPullRequest::onGetDataError);
//                break;
//            }
//            case streams::DataStreamType::STREAM_INT64: {
//                auto i64 = std::static_pointer_cast<Int64DataStream>(stream);
//                connect(i64.get(), &Int64DataStream::getInt64DataResult,
//                    this, &MultiPullRequest::onGetInt64DataResult);
//                connect(i64.get(), &Int64DataStream::getInt64DataError,
//                    this, &MultiPullRequest::onGetDataError);
//                break;
//            }
//            case streams::DataStreamType::STREAM_CATEGORY: {
//                auto cat = std::static_pointer_cast<CategoryDataStream>(stream);
//                connect(cat.get(), &CategoryDataStream::getCategoryDataResult,
//                    this, &MultiPullRequest::onGetCategoryDataResult);
//                connect(cat.get(), &CategoryDataStream::getCategoryDataError,
//                    this, &MultiPullRequest::onGetDataError);
//                break;
//            }
//            default:
//                TU_LOG_WARN << this << " ignoring source stream " << stream->getId().toStdString() << " with unknown type";
//                break;
//        }
//    }
//}
//
//MultiPullRequest::MultiPullRequest(
//    const tempo_utils::RequestId &requestId,
//    const DoubleRange &requestRange,
//    const SourceDataRanges &ranges,
//    const StreamSourceCollection &sources,
//    QObject *parent)
//    : MultiPullRequest(requestId, ranges, sources, parent)
//{
//    m_dbl = requestRange;
//}
//
//MultiPullRequest::MultiPullRequest(
//    const tempo_utils::RequestId &requestId,
//    const Int64Range &requestRange,
//    const SourceDataRanges &ranges,
//    const StreamSourceCollection &sources,
//    QObject *parent)
//    : MultiPullRequest(requestId, ranges, sources, parent)
//{
//    m_i64 = requestRange;
//}
//
//MultiPullRequest::MultiPullRequest(
//    const tempo_utils::RequestId &requestId,
//    const CategoryRange &requestRange,
//    const SourceDataRanges &ranges,
//    const StreamSourceCollection &sources,
//    QObject *parent)
//    : MultiPullRequest(requestId, ranges, sources, parent)
//{
//    m_cat = requestRange;
//}
//
//void
//MultiPullRequest::start()
//{
//    if (m_running || m_complete)
//        return;
//
//    for (const auto &sourceId : m_ranges.listDoubleSourceRanges()) {
//        if (!m_sources.hasSource(sourceId))
//            continue;
//        const auto &range = m_ranges.getDoubleSourceRange(sourceId);
//        auto source = m_sources.getSource(sourceId);
//        auto stream = source.getStream();
//        if (stream->getStreamType() != streams::DataStreamType::STREAM_DOUBLE)
//            continue;
//        auto dbl = std::static_pointer_cast<DoubleDataStream>(stream);
//        if (dbl->isComplete())
//            continue;
//        auto result = dbl->getDoubleData(range);
//        if (result.isResult())
//            m_inflight.insert(result.getResult());
//        else
//            TU_LOG_ERROR << this << " rejected pull for double data: " << result.getStatus();
//    }
//
//    for (const auto &sourceId : m_ranges.listInt64SourceRanges()) {
//        if (!m_sources.hasSource(sourceId))
//            continue;
//        const auto &range = m_ranges.getInt64SourceRange(sourceId);
//        auto source = m_sources.getSource(sourceId);
//        auto stream = source.getStream();
//        if (stream->getStreamType() != streams::DataStreamType::STREAM_INT64)
//            continue;
//        auto i64 = std::static_pointer_cast<Int64DataStream>(stream);
//        if (i64->isComplete())
//            continue;
//        auto result = i64->getInt64Data(range);
//        if (result.isResult())
//            m_inflight.insert(result.getResult());
//        else
//            TU_LOG_ERROR << this << " rejected pull for int64 data: " << result.getStatus();
//    }
//
//    for (const auto &sourceId : m_ranges.listCategorySourceRanges()) {
//        if (!m_sources.hasSource(sourceId))
//            continue;
//        const auto &range = m_ranges.getCategorySourceRange(sourceId);
//        auto source = m_sources.getSource(sourceId);
//        auto stream = source.getStream();
//        if (stream->getStreamType() != streams::DataStreamType::STREAM_CATEGORY)
//            continue;
//        auto cat = std::static_pointer_cast<CategoryDataStream>(stream);
//        if (cat->isComplete())
//            continue;
//        auto result = cat->getCategoryData(range);
//        if (result.isResult())
//            m_inflight.insert(result.getResult());
//        else
//            TU_LOG_ERROR << this << " rejected pull for category data: " << result.getStatus();
//    }
//
//    if (m_inflight.empty()) {
//        m_complete = true;
//        TU_LOG_INFO << this << "no pulls required";
//        m_timerId = startTimer(0, Qt::CoarseTimer);
//    } else {
//        m_running = true;
//        TU_LOG_INFO << this << "starting pull";
//    }
//}
//
//bool
//MultiPullRequest::isComplete() const
//{
//    return m_complete;
//}
//
//bool
//MultiPullRequest::isRunning() const
//{
//    return m_running;
//}
//
//tempo_utils::RequestId
//MultiPullRequest::getRequestId() const
//{
//    return m_id;
//}
//
//SourceDataRanges
//MultiPullRequest::getSourceRanges() const
//{
//    return m_ranges;
//}
//
//StreamSourceCollection
//MultiPullRequest::getSelectionSet() const
//{
//    return m_sources;
//}
//
//DoubleRange
//MultiPullRequest::getDoubleRequestRange() const
//{
//    return m_dbl;
//}
//
//Int64Range
//MultiPullRequest::getInt64RequestRange() const
//{
//    return m_i64;
//}
//
//CategoryRange
//MultiPullRequest::getCategoryRequestRange() const
//{
//    return m_cat;
//}
//
//void
//MultiPullRequest::timerEvent(QTimerEvent *event)
//{
//    killTimer(event->timerId());
//    emit complete();
//}
//
//void
//MultiPullRequest::onGetDoubleDataResult(tempo_utils::RequestId requestId,
//                                        double smallestKey,
//                                        double largestKey,
//                                        bool complete)
//{
//    TU_LOG_INFO << this << "received result from request" << requestId.toString();
//    if (complete) {
//        m_inflight.erase(requestId);
//        if (m_inflight.empty()) {
//            m_running = false;
//            m_complete =  true;
//            emit MultiPullRequest::complete();
//        }
//    }
//}
//
//void
//MultiPullRequest::onGetInt64DataResult(tempo_utils::RequestId requestId,
//                                       qint64 smallestKey,
//                                       qint64 largestKey,
//                                       bool complete)
//{
//    TU_LOG_INFO << this << "received result from request" << requestId.toString();
//    if (complete) {
//        m_inflight.erase(requestId);
//        if (m_inflight.empty()) {
//            m_running = false;
//            m_complete =  true;
//            emit MultiPullRequest::complete();
//        }
//    }
//}
//
//void
//MultiPullRequest::onGetCategoryDataResult(tempo_utils::RequestId requestId,
//                                          QStringList smallestKey,
//                                          QStringList largestKey,
//                                          bool complete)
//{
//    TU_LOG_INFO << this << "received result from request" << requestId.toString();
//    if (complete) {
//        m_inflight.erase(requestId);
//        if (m_inflight.empty()) {
//            m_running = false;
//            m_complete =  true;
//            emit MultiPullRequest::complete();
//        }
//    }
//}
//
//void
//MultiPullRequest::onGetDataError(tempo_utils::RequestId requestId, groove_sync::SyncStatus status)
//{
//    TU_LOG_INFO << this << "received error: " << status << "from request " << requestId.toString();
//    m_inflight.erase(requestId);
//    if (m_inflight.empty()) {
//        m_running = false;
//        m_complete =  true;
//        TU_LOG_INFO << this << "pull complete";
//        emit complete();
//    }
//}
