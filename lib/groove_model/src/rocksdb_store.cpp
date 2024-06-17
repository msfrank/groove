
#include <rocksdb/slice.h>

#include <groove_model/rocksdb_store.h>
#include <tempo_utils/log_stream.h>

groove_model::RocksDbStore::RocksDbStore(const std::filesystem::path &dbPath)
    : RocksDbStore(dbPath, rocksdb::Options())
{
}

groove_model::RocksDbStore::RocksDbStore(const std::filesystem::path &dbPath, const rocksdb::Options &options)
    : m_dbPath(dbPath),
      m_options(options)
{
    TU_ASSERT (!m_dbPath.empty());
    m_options.create_if_missing = true;
}

groove_model::RocksDbStore::~RocksDbStore()
{
    delete m_rocksDb;
}

std::filesystem::path
groove_model::RocksDbStore::getDbPath() const
{
    return m_dbPath;
}

rocksdb::Status
groove_model::RocksDbStore::open()
{
    return rocksdb::DB::Open(m_options, m_dbPath, &m_rocksDb);
}

inline rocksdb::Slice
make_slice(const std::string &bytes)
{
    return rocksdb::Slice(bytes.data(), bytes.size());
}

bool
groove_model::RocksDbStore::isEmpty()
{
    return valueCount() == 0;
}

tempo_utils::Result<groove_model::PageId>
groove_model::RocksDbStore::getPageIdBefore(const PageId &pageId, bool exclusive)
{
    rocksdb::Status status;
    auto key = getKeyBefore(&status, pageId.getBytes(), pageId.getPrefix(), exclusive);
    if (status.IsNotFound())
        return ModelStatus::forCondition(ModelCondition::kPageNotFound);
    if (!status.ok())
        return ModelStatus::forCondition(ModelCondition::kModelInvariant, status.ToString());
    if (key == nullptr)
        return ModelStatus::forCondition(ModelCondition::kPageNotFound);
    return PageId::fromString(std::string_view(key->data(), key->size()));
}

tempo_utils::Result<groove_model::PageId>
groove_model::RocksDbStore::getPageIdAfter(const PageId &pageId, bool exclusive)
{
    rocksdb::Status status;
    auto key = getKeyAfter(&status, pageId.getBytes(), pageId.getPrefix(), exclusive);
    if (status.IsNotFound())
        return ModelStatus::forCondition(ModelCondition::kPageNotFound);
    if (!status.ok())
        return ModelStatus::forCondition(ModelCondition::kModelInvariant, status.ToString());
    if (key == nullptr)
        return ModelStatus::forCondition(ModelCondition::kPageNotFound);
    return PageId::fromString(std::string_view(key->data(), key->size()));
}

tempo_utils::Result<std::shared_ptr<arrow::Buffer>>
groove_model::RocksDbStore::getPageData(const PageId &pageId)
{
    rocksdb::Status status;
    auto value = getValue(&status, pageId.getBytes());
    if (status.IsNotFound())
        return ModelStatus::forCondition(ModelCondition::kPageNotFound);
    if (!status.ok())
        return ModelStatus::forCondition(ModelCondition::kModelInvariant, status.ToString());
    if (value == nullptr)
        return ModelStatus::forCondition(ModelCondition::kPageNotFound);
    return value;
}

tempo_utils::Status
groove_model::RocksDbStore::pageExists(const PageId &pageId)
{
    auto getPageDataResult = getPageData(pageId);
    if (getPageDataResult.isStatus())
        return getPageDataResult.getStatus();
    return ModelStatus::ok();
}

groove_model::AbstractPageStoreTransaction *
groove_model::RocksDbStore::startTransaction()
{
    auto store = shared_from_this();
    auto *txn = new RocksDbTransaction(store);
    return txn;
}

std::shared_ptr<const std::string>
groove_model::RocksDbStore::getKeyBefore(
    rocksdb::Status *status,
    const std::string &key,
    const std::string &prefix,
    bool exclusive)
{
    const std::string fullKey = absl::StrCat("/v/", key);
    const std::string fullPrefix = absl::StrCat("/v/", prefix);

    auto iterator = std::unique_ptr<rocksdb::Iterator>(m_rocksDb->NewIterator(rocksdb::ReadOptions()));

    iterator->SeekForPrev(make_slice(fullKey));             // find the key equal to or less than key
    if (status)
        *status = iterator->status();
    if (!iterator->Valid())
        return {};                                          // returns an empty shared_ptr on failure

    auto slice = iterator->key();
    std::string result(slice.data(), slice.size());         // this does not copy the bytes from slice
    if (result == fullKey && exclusive) {
        iterator->Prev();                                   // if the result is equal to key, then find the previous key
        if (status)
            *status = iterator->status();
        if (!iterator->Valid())
            return {};
        slice = iterator->key();
        result = std::string(slice.data(), slice.size());    // this does not copy the bytes from slice
    }

    if (exclusive) {
        // result must be less than key and start with prefix
        if (result < fullKey && result.starts_with(fullPrefix)) {
            auto keyBefore = result.substr(3);
            return std::make_shared<const std::string>(std::move(keyBefore));
        }
    } else {
        // result must be equal or less than key and start with prefix
        if (result <= fullKey && result.starts_with(fullPrefix)) {
            auto keyBefore = result.substr(3);
            return std::make_shared<const std::string>(std::move(keyBefore));
        }
    }

    return {};   // returns an empty shared_ptr on failure
}

std::shared_ptr<const std::string>
groove_model::RocksDbStore::getKeyAfter(
    rocksdb::Status *status,
    const std::string &key,
    const std::string &prefix,
    bool exclusive)
{
    const std::string fullKey = absl::StrCat("/v/", key);
    const std::string fullPrefix = absl::StrCat("/v/", prefix);

    auto iterator = std::unique_ptr<rocksdb::Iterator>(m_rocksDb->NewIterator(rocksdb::ReadOptions()));

    iterator->Seek(make_slice(fullKey));                    // find the key equal to or greater than key
    if (status)
        *status = iterator->status();
    if (!iterator->Valid())
        return {};                                          // returns an empty string on failure

    auto slice = iterator->key();
    std::string result(slice.data(), slice.size());         // this does not copy the bytes from slice
    if (result == fullKey && exclusive) {
        iterator->Next();                                   // if the result is equal to key, then find the next key
        if (status)
            *status = iterator->status();
        if (!iterator->Valid())
            return {};                                      // returns an empty string on failure
        slice = iterator->key();
        result = std::string(slice.data(), slice.size());   // this does not copy the bytes from slice
    }
    if (exclusive) {
        // result must be greater than key and start with prefix
        if (result > fullKey && result.starts_with(fullPrefix)) {
            auto keyAfter = result.substr(3);
            return std::make_shared<const std::string>(std::move(keyAfter));
        }
    } else {
        // result must be equal or greater than key and start with prefix
        if (result >= fullKey && result.starts_with(fullPrefix)) {
            auto keyAfter = result.substr(3);
            return std::make_shared<const std::string>(std::move(keyAfter));
        }
    }

    return {};   // returns an empty string on failure
}

std::shared_ptr<arrow::Buffer>
groove_model::RocksDbStore::getValue(rocksdb::Status *status, const std::string &key)
{
    const std::string fullKey = absl::StrCat("/v/", key);

    auto value = std::make_unique<rocksdb::PinnableSlice>();
    auto ret = m_rocksDb->Get(
        rocksdb::ReadOptions(),
        m_rocksDb->DefaultColumnFamily(),
        make_slice(fullKey),
        value.get());
    if (status)
        *status = ret;
    if (!ret.ok())
        return {};                                      // returns an empty string on failure
    if (value->size() == 0)                             // value exists but has length 0
        return {};                                      // returns an empty string but status is ok

    return std::make_shared<RocksdbPageData>(value.release());
}

rocksdb::Status
groove_model::RocksDbStore::applyBatch(rocksdb::WriteBatch *batch)
{
    TU_ASSERT(batch != nullptr);
    return m_rocksDb->Write(rocksdb::WriteOptions(), batch);
}

void
groove_model::RocksDbStore::setValue(
    rocksdb::Status *status,
    const std::string &key,
    std::shared_ptr<const arrow::Buffer> value,
    rocksdb::WriteBatch *batch)
{
    const std::string fullKey = absl::StrCat("/v/", key);
    rocksdb::Slice valueSlice((const char *)value->data(), value->size());

    if (batch) {    // if WriteBatch is specified, then append operation to the batch
        batch->Put(make_slice(fullKey), valueSlice);
        if (status)
            *status = rocksdb::Status::OK();    // writing to the batch always succeeds
    } else {        // otherwise write directly to the db
        auto ret = m_rocksDb->Put(rocksdb::WriteOptions(), make_slice(fullKey), valueSlice);
        if (status)
            *status = ret;
    }
}

void
groove_model::RocksDbStore::removeValue(rocksdb::Status *status, const std::string &key, rocksdb::WriteBatch *batch)
{
    const std::string fullKey = absl::StrCat("/v/", key);

    if (batch) {
        batch->Delete(make_slice(fullKey));
        if (status)
            *status = rocksdb::Status::OK();    // writing to the batch always succeeds
    } else {
        auto ret = m_rocksDb->Delete(rocksdb::WriteOptions(), make_slice(fullKey));
        if (status)
            *status = ret;
    }
}

int
groove_model::RocksDbStore::valueCount()
{
    int count = 0;
    auto iterator = m_rocksDb->NewIterator(rocksdb::ReadOptions());

    // seek to the first value
    iterator->Seek(make_slice(std::string("/v/")));
    while (iterator->Valid()) {
        count++;
        iterator->Next();
    }
    delete iterator;

    return count;
}

std::string
groove_model::RocksDbStore::iterateForward(
    rocksdb::Status *status,
    std::vector<std::string> &values,
    int maxValues,
    const std::string &token)
{
    auto iterator = std::unique_ptr<rocksdb::Iterator>(m_rocksDb->NewIterator(rocksdb::ReadOptions()));

    // move iterator to the first position
    std::string keyPrefix;
    if (token.empty()) {
        keyPrefix.append("/v/");
    } else {
        keyPrefix = token;
    }
    iterator->Seek(make_slice(keyPrefix));

    // fetch at most maxValues items from the store
    for (int i = 0; i < maxValues; i++) {
        if (status)
            *status = iterator->status();
        if (!iterator->Valid())
            return std::string();
        auto _key = iterator->key();
        const std::string key(_key.data(), _key.size());        // this does not copy bytes from _key
        auto _value = iterator->value();
        const std::string value(_value.data(), _value.size());  // this does not copy bytes from _value
        if (!key.starts_with("/v/"))
            return std::string();
        values.push_back(value);                                // const value triggers copy constructor
        iterator->Next();
    }

    // if we get here, there are strictly maxValues or greater items
    if (iterator->Valid()) {
        auto _key = iterator->key();
        const std::string key(_key.data(), _key.size());
        return key;
    }

    return std::string();
}

std::string
groove_model::RocksDbStore::iteratePrefix(
    rocksdb::Status *status,
    const std::string &prefix,
    std::vector<std::string> &values,
    int maxValues,
    const std::string &token)
{
    auto iterator = std::unique_ptr<rocksdb::Iterator>(m_rocksDb->NewIterator(rocksdb::ReadOptions()));

    // move iterator to the first position
    std::string keyPrefix;
    if (token.empty()) {
        keyPrefix = absl::StrCat("/v/", prefix);
    } else {
        keyPrefix = token;
    }
    //_iterator->Seek(make_slice(keyPrefix));

    // fetch at most maxValues items from the store
    for (int i = 0; i < maxValues; i++) {
        if (status)
            *status = iterator->status();
        if (!iterator->Valid())
            return std::string();
        auto _value = iterator->value();
        const std::string value(_value.data(), _value.size());  // this does not copy bytes from _value
        if (!value.starts_with(prefix))
            return std::string();
        values.push_back(value);                                // const value triggers copy constructor
        iterator->Next();
    }

    // if we get here, there are strictly maxValues or greater items
    if (iterator->Valid()) {
        auto _key = iterator->key();
        const std::string key(_key.data(), _key.size());
        return key;
    }

    return std::string();
}

std::string
groove_model::RocksDbStore::iterateBounds(
    rocksdb::Status *status,
    const std::string &start,
    const std::string &end,
    std::vector<std::string> &values,
    int maxValues,
    const std::string &token)
{
    auto iterator = std::unique_ptr<rocksdb::Iterator>(m_rocksDb->NewIterator(rocksdb::ReadOptions()));

    // move iterator to the first position
    std::string keyPrefix;
    if (token.empty()) {
        keyPrefix = absl::StrCat("/v/", start);
    } else {
        keyPrefix = token;
    }
    iterator->Seek(make_slice(keyPrefix));

    const std::string endKey = std::string("/v/").append(end);

    // fetch at most maxValues items from the store
    for (int i = 0; i < maxValues; i++) {
        if (status)
            *status = iterator->status();
        if (!iterator->Valid())
            return std::string();
        auto _key = iterator->key();
        const std::string key(_key.data(), _key.size());       // constructor makes a deep copy of the result
        if (key > endKey)
            return std::string();
        auto _value = iterator->value();
        const std::string value(_value.data(), _value.size()); // constructor makes a deep copy of the result
        values.push_back(value);
        iterator->Next();
    }

    // if we get here, there are strictly maxValues or greater items
    if (iterator->Valid()) {
        auto _key = iterator->key();
        const std::string key(_key.data(), _key.size());
        return key;
    }

    return std::string();
}

std::string
groove_model::RocksDbStore::getMeta(rocksdb::Status *status, const std::string &key)
{
    const std::string fullKey = absl::StrCat("/m/", key);
    std::string value;
    auto _status = m_rocksDb->Get(rocksdb::ReadOptions(), make_slice(fullKey), &value);
    if (status)
        *status = _status;
    if (!_status.ok())
        return std::string();                            // returns a null, empty byte array
    if (value.size() == 0)                              // value exists but has length 0
        return std::string("");                          // returns a non-null, empty byte array
    return std::string(value.data(), value.size());      // constructor makes a deep copy of the result
}

void
groove_model::RocksDbStore::setMeta(
    rocksdb::Status *status,
    const std::string &key,
    const std::string &value,
    rocksdb::WriteBatch *batch)
{
    const std::string fullKey = absl::StrCat("/m/", key);
    if (batch) {    // if WriteBatch is specified, then append operation to the batch
        batch->Put(make_slice(fullKey), make_slice(value));
        if (status)
            *status = rocksdb::Status::OK();    // writing to the batch always succeeds
    } else {        // otherwise write directly to the db
        auto ret = m_rocksDb->Put(rocksdb::WriteOptions(), make_slice(fullKey), make_slice(value));
        if (status)
            *status = ret;
    }
}

void
groove_model::RocksDbStore::removeMeta(rocksdb::Status *status, const std::string &key, rocksdb::WriteBatch *batch)
{
    const std::string fullKey = absl::StrCat("/m/", key);
    if (batch) {
        batch->Delete(make_slice(fullKey));
        if (status)
            *status = rocksdb::Status::OK();    // writing to the batch always succeeds
    } else {
        auto ret = m_rocksDb->Delete(rocksdb::WriteOptions(), make_slice(fullKey));
        if (status)
            *status = ret;
    }
}

bool
groove_model::RocksDbStore::metaExists(rocksdb::Status *status, const std::string &key)
{
    rocksdb::Status ret;
    const auto value = getMeta(&ret, key);
    if (status)
        *status = ret;
    if (ret.ok() && !value.empty())
        return true;
    return false;
}

std::shared_ptr<groove_model::RocksDbStore>
groove_model::RocksDbStore::create(const std::filesystem::path &dbPath)
{
    return std::shared_ptr<RocksDbStore>(new RocksDbStore(dbPath));
}

std::shared_ptr<groove_model::RocksDbStore>
groove_model::RocksDbStore::create(const std::filesystem::path &dbPath, const rocksdb::Options &options)
{
    return std::shared_ptr<RocksDbStore>(new RocksDbStore(dbPath, options));
}

groove_model::RocksdbPageData::RocksdbPageData(rocksdb::PinnableSlice *slice)
    : arrow::Buffer((const tu_uint8 *) slice->data(), slice->size()),
      m_slice(slice)
{
    TU_ASSERT (m_slice && !m_slice->empty());
}

groove_model::RocksdbPageData::~RocksdbPageData()
{
    TU_ASSERT (m_slice != nullptr);
    delete m_slice;
}

groove_model::RocksDbTransaction::RocksDbTransaction(std::shared_ptr<RocksDbStore> store)
    : m_store(store)
{
    TU_ASSERT (store != nullptr);
    m_batch = new rocksdb::WriteBatch();
}

groove_model::RocksDbTransaction::~RocksDbTransaction()
{
    // if transaction is destructed without calling apply or abort, then abort the batch
    if (m_batch != nullptr) {
        delete m_batch;
    }
}

tempo_utils::Status
groove_model::RocksDbTransaction::removePage(const PageId &pageId)
{
    if (m_batch == nullptr)
        return ModelStatus::forCondition(ModelCondition::kModelInvariant, "invalid write batch");

    rocksdb::Status status;
    m_store->removeValue(&status, pageId.getBytes(), m_batch);
    if (!status.ok())
        return ModelStatus::forCondition(ModelCondition::kModelInvariant, status.ToString());
    return ModelStatus::ok();
}

tempo_utils::Status
groove_model::RocksDbTransaction::writePage(const PageId &pageId, std::shared_ptr<const arrow::Buffer> pageBytes)
{
    if (m_batch == nullptr)
        return ModelStatus::forCondition(ModelCondition::kModelInvariant, "invalid write batch");

    rocksdb::Status status;
    m_store->setValue(&status, pageId.getBytes(), pageBytes, m_batch);
    if (!status.ok())
        return ModelStatus::forCondition(ModelCondition::kModelInvariant, status.ToString());
    return ModelStatus::ok();
}

tempo_utils::Status
groove_model::RocksDbTransaction::apply()
{
    if (m_batch == nullptr)
        return ModelStatus::forCondition(ModelCondition::kModelInvariant, "invalid write batch");
    m_store->applyBatch(m_batch);
    delete m_batch;
    m_batch = nullptr;
    return ModelStatus::ok();
}

tempo_utils::Status
groove_model::RocksDbTransaction::abort()
{
    if (m_batch == nullptr)
        return ModelStatus::forCondition(ModelCondition::kModelInvariant, "invalid write batch");
    delete m_batch;
    m_batch = nullptr;
    return ModelStatus::ok();
}