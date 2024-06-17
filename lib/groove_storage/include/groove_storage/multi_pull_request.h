//#ifndef MULTI_PULL_REQUEST_H
//#define MULTI_PULL_REQUEST_H
//
//#include <QObject>
//#include <QMap>
//#include <QTimer>
//#include <QTimerEvent>
//
//#include <groove_math/source_data_ranges.h>
//
//#include "base_data_stream.h"
//#include "stream_source.h"
//#include "stream_source_collection.h"
//
//class MultiPullRequest : public QObject {
//
//    Q_OBJECT
//
//public:
//    MultiPullRequest(
//        const tempo_utils::RequestId &requestId,
//        const DoubleRange &requestRange,
//        const SourceDataRanges &ranges,
//        const StreamSourceCollection &sources,
//        QObject *parent = Q_NULLPTR);
//    MultiPullRequest(
//        const tempo_utils::RequestId &requestId,
//        const Int64Range &requestRange,
//        const SourceDataRanges &ranges,
//        const StreamSourceCollection &sources,
//        QObject *parent = Q_NULLPTR);
//    MultiPullRequest(
//        const tempo_utils::RequestId &requestId,
//        const CategoryRange &requestRange,
//        const SourceDataRanges &ranges,
//        const StreamSourceCollection &sources,
//        QObject *parent = Q_NULLPTR);
//
//    tempo_utils::RequestId getRequestId() const;
//    SourceDataRanges getSourceRanges() const;
//    StreamSourceCollection getSelectionSet() const;
//
//    DoubleRange getDoubleRequestRange() const;
//    Int64Range getInt64RequestRange() const;
//    CategoryRange getCategoryRequestRange() const;
//
//    bool isComplete() const;
//    bool isRunning() const;
//
//    void start();
//
//protected:
//    void timerEvent(QTimerEvent *event) override;
//
//signals:
//    void complete();
//
//private slots:
//    void onGetDoubleDataResult(tempo_utils::RequestId requestId, double smallestKey, double largestKey, bool complete);
//    void onGetInt64DataResult(tempo_utils::RequestId requestId, qint64 smallestKey, qint64 largestKey, bool complete);
//    void onGetCategoryDataResult(tempo_utils::RequestId requestId, QStringList smallestKey, QStringList largestKey, bool complete);
//    void onGetDataError(tempo_utils::RequestId requestId, groove_sync::SyncStatus status);
//
//private:
//    tempo_utils::RequestId m_id;
//    SourceDataRanges m_ranges;
//    StreamSourceCollection m_sources;
//    DoubleRange m_dbl;
//    Int64Range m_i64;
//    CategoryRange m_cat;
//    absl::flat_hash_set<tempo_utils::RequestId> m_inflight;
//    bool m_running;
//    bool m_complete;
//    int m_timerId;
//
//    MultiPullRequest(
//        const tempo_utils::RequestId &requestId,
//        const SourceDataRanges &ranges,
//        const StreamSourceCollection &sources,
//        QObject *parent);
//};
//
//#endif // MULTI_PULL_REQUEST_H
