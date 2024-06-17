//#include <tempo_utils/logging.h>
//#include <QDateTime>
//
//#include "fair_queuing_scheduler.h"
//#include "tempo_utils/date_time.h"
//
//#define JITTER_MIN_IN_MILLIS    50
//#define JITTER_MAX_IN_MILLIS    150
//
//FairQueuingScheduler::FairQueuingScheduler(QObject *parent) : QObject(parent)
//{
//    m_timer = new QTimer(this);
//    m_timer->setTimerType(Qt::CoarseTimer);
//    connect(m_timer, &QTimer::timeout, this, &FairQueuingScheduler::onScanQueue);
//    m_distribution = new std::uniform_int_distribution<int>(JITTER_MIN_IN_MILLIS, JITTER_MAX_IN_MILLIS);
//}
//
//FairQueuingScheduler::~FairQueuingScheduler()
//{
//    delete m_distribution;
//}
//
//groove_sync::SyncStatus
//FairQueuingScheduler::enqueue(
//    const tempo_utils::RequestId &requestId,
//    std::shared_ptr<BaseDataStream> stream,
//    int delayMillis)
//{
//    // if delay is negative, then generate a random delay between JITTER_MIN and JITTER_MAX
//    if (delayMillis < 0)
//        delayMillis = (*m_distribution)(m_generator);
//
//    auto deadline = tempo_utils::millis_since_epoch() + delayMillis;
//    m_pending.push(Pending(deadline, requestId, stream));
//
//    if (!m_timer->isActive() || deadline <= m_pending.top().ts) {
//        m_timer->start(delayMillis);
//    }
//
//    return groove_sync::SyncStatus::ok();
//}
//
//groove_sync::SyncStatus
//FairQueuingScheduler::enqueue(
//    const tempo_utils::RequestId &requestId,
//    std::shared_ptr<BaseDataStream> stream,
//    const QDateTime &deadline)
//{
//    if (!deadline.isValid())
//        return groove_sync::SyncStatus::internalViolation("deadline is invalid");
//    qint64 delayMillis = deadline.currentMSecsSinceEpoch() - QDateTime::currentMSecsSinceEpoch();
//    if (delayMillis > std::numeric_limits<int>::max())
//        return enqueue(requestId, stream, std::numeric_limits<int>::max());
//    return enqueue(requestId, stream, static_cast<int>(delayMillis));
//}
//
//void
//FairQueuingScheduler::onScanQueue()
//{
//    if (m_pending.empty())
//        return;
//
//    // process each timeslice that has a deadline in the past
//    auto earliest = m_pending.top();
//    auto now = tempo_utils::millis_since_epoch();
//    do {
//        m_pending.pop();
//        earliest.stream->notifyScheduled(earliest.id);
//        if (m_pending.empty())
//            return;
//        earliest = m_pending.top();
//    } while (earliest.ts < now);
//
//    // schedule a new timeout for the earliest deadline in the pending queue
//    auto delayMillis = earliest.ts - tempo_utils::millis_since_epoch();
//    if (delayMillis > std::numeric_limits<int>::max())
//        return m_timer->start(std::numeric_limits<int>::max());
//    return m_timer->start(static_cast<int>(delayMillis));
//}
