#ifndef GROOVE_MODEL_COLUMN_WALKER_H
#define GROOVE_MODEL_COLUMN_WALKER_H

#include "model_result.h"
#include "model_types.h"
#include "schema_attr_parser.h"

namespace groove_model {

    class ColumnWalker {

    public:
        ColumnWalker();
        ColumnWalker(const ColumnWalker &other);

        bool isValid() const;

        std::string getColumnId() const;
        ColumnValueType getValueType() const;
        ColumnValueFidelity getValueFidelity() const;

    private:
        std::shared_ptr<const internal::SchemaReader> m_reader;
        void *m_columnDescriptor;

        ColumnWalker(std::shared_ptr<const internal::SchemaReader> reader, void *columnDescriptor);
        friend class ModelWalker;

        tu_uint32 findIndexForAttr(const tempo_utils::AttrKey &key) const;

    public:
        /**
         *
         * @tparam AttrType
         * @tparam SerdeType
         * @param attr
         * @param value
         * @return
         */
        template<class AttrType,
            typename SerdeType = typename AttrType::SerdeType>
        tempo_utils::Status
        parseAttr(const AttrType &attr, SerdeType &value) const
        {
            auto index = findIndexForAttr(attr.getKey());
            if (index == kInvalidOffsetU32)
                return ModelStatus::forCondition(ModelCondition::kModelInvariant, "missing attr in column");
            SchemaAttrParser parser(m_reader);
            return attr.parseAttr(index, &parser, value);
        }
    };
}

#endif // GROOVE_MODEL_COLUMN_WALKER_H
