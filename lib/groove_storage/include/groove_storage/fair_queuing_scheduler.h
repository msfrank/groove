//#ifndef FAIR_QUEUING_SCHEDULER_H
//#define FAIR_QUEUING_SCHEDULER_H
//
//#include <random>
//#include <queue>
//
//#include <QObject>
//#include <QMap>
//#include <QHash>
//#include <QTimer>
//#include <QDateTime>
//
//#include <tempo_utils/request_id.h>
//
//#include "base_data_stream.h"
//
//class FairQueuingScheduler : public QObject {
//
//    Q_OBJECT
//
//public:
//    explicit FairQueuingScheduler(QObject *parent = Q_NULLPTR);
//    ~FairQueuingScheduler() override;
//
//    groove_sync::SyncStatus enqueue(
//        const tempo_utils::RequestId &requestId,
//        std::shared_ptr<BaseDataStream> stream,
//        int delayMillis = -1);
//    groove_sync::SyncStatus enqueue(
//        const tempo_utils::RequestId &requestId,
//        std::shared_ptr<BaseDataStream> stream,
//        const QDateTime &deadline);
//
//private slots:
//    void onScanQueue();
//
//private:
//    struct Pending {
//        int64_t ts;
//        tempo_utils::RequestId id;
//        std::shared_ptr<BaseDataStream> stream;
//        Pending(
//            int64_t ts,
//            const tempo_utils::RequestId &id,
//            std::shared_ptr<BaseDataStream> stream)
//            : ts(ts), id(id), stream(stream)
//        {};
//        bool operator<(const Pending &other) const { return ts < other.ts; };
//        bool operator>(const Pending &other) const { return ts > other.ts; };
//    };
//    std::priority_queue<Pending,std::vector<Pending>,std::greater<Pending>> m_pending;
//    QTimer *m_timer;
//    std::default_random_engine m_generator;
//    std::uniform_int_distribution<int> *m_distribution;
//};
//
//#endif // FAIR_QUEUING_SCHEDULER_H
