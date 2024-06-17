//
//#include <groove_model/persistent_caching_page_store.h>
//
//groove_model::PersistentCachingPageStore::PersistentCachingPageStore(std::shared_ptr<RocksDbStore> store)
//    : m_store(store)
//{
//    TU_ASSERT (m_store != nullptr);
//}
//
//std::shared_ptr<groove_model::RocksDbStore>
//groove_model::PersistentCachingPageStore::getStore() const
//{
//    return m_store;
//}