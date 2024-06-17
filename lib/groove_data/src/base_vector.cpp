
#include <arrow/scalar.h>

#include <groove_data/base_vector.h>
#include <tempo_utils/log_message.h>

groove_data::BaseVector::BaseVector(
    std::shared_ptr<arrow::Table> table,
    int keyFieldIndex,
    int valFieldIndex,
    int fidFieldIndex)
    : m_table(table),
      m_keyFieldIndex(keyFieldIndex),
      m_valFieldIndex(valFieldIndex),
      m_fidFieldIndex(fidFieldIndex)
{
    TU_ASSERT (m_table != nullptr);
}

std::shared_ptr<arrow::Table>
groove_data::BaseVector::getTable() const
{
    return m_table;
}

std::shared_ptr<arrow::Schema>
groove_data::BaseVector::getSchema() const
{
    return m_table->schema();
}

std::string
groove_data::BaseVector::getColumnId() const
{
    auto field = m_table->field(m_valFieldIndex);
    return field->name();
}

int
groove_data::BaseVector::getKeyFieldIndex() const
{
    return m_keyFieldIndex;
}

int
groove_data::BaseVector::getValFieldIndex() const
{
    return m_valFieldIndex;
}

int
groove_data::BaseVector::getFidFieldIndex() const
{
    return m_fidFieldIndex;
}

bool
groove_data::BaseVector::isEmpty() const
{
    return m_table->num_rows() == 0;
}

int
groove_data::BaseVector::getSize() const
{
    return m_table->num_rows();
}

std::string
groove_data::BaseVector::toString() const
{
    std::string s;
    absl::StrAppend(&s,
        m_table->num_rows(), " rows, ",
        "key index: ", m_keyFieldIndex,
        ", val index: ", m_valFieldIndex,
        ", fid index: ", m_fidFieldIndex, "\n");
    absl::StrAppend(&s, "----\n");

    auto keyArray = m_table->column(m_keyFieldIndex);
    auto valArray = m_table->column(m_valFieldIndex);
    auto fidArray = m_table->column(m_fidFieldIndex);

    for (int i = 0; i < m_table->num_rows(); i++) {
        auto getKeyResult = keyArray->GetScalar(i);
        if (!getKeyResult.ok())
            return {};
        auto key = *getKeyResult;
        auto getValResult = valArray->GetScalar(i);
        if (!getValResult.ok())
            return {};
        auto val = *getValResult;
        auto getFidResult = fidArray->GetScalar(i);
        if (!getFidResult.ok())
            return {};
        auto fid = *getFidResult;
        absl::StrAppend(&s,
            i, "\t",
            "key: ", key->ToString(),
            ", val: ", val->ToString(),
            ", fid: ", fid->ToString(), "\n");
    }

    return s;
}

void
groove_data::BaseVector::print() const
{
    TU_CONSOLE_OUT << toString();
}
