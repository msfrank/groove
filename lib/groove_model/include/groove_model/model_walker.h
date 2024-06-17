#ifndef GROOVE_MODEL_MODEL_WALKER_H
#define GROOVE_MODEL_MODEL_WALKER_H

#include <tempo_utils/attr.h>

#include "column_walker.h"
#include "model_result.h"
#include "model_types.h"
#include "schema_attr_parser.h"

namespace groove_model {

    class ModelWalker {

    public:
        ModelWalker();
        ModelWalker(const ModelWalker &other);

        bool isValid() const;

        std::string getModelId() const;
        ModelKeyType getKeyType() const;
        ModelKeyCollation getKeyCollation() const;

        ColumnWalker getColumn(tu_uint32 index) const;
        ColumnWalker findColumn(const std::string &columnId) const;
        int numColumns() const;

    private:
        std::shared_ptr<const internal::SchemaReader> m_reader;
        void *m_modelDescriptor;

        ModelWalker(std::shared_ptr<const internal::SchemaReader> reader, void *modelDescriptor);
        friend class SchemaWalker;

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
                return ModelStatus::forCondition(ModelCondition::kModelInvariant, "missing attr in model");
            SchemaAttrParser parser(m_reader);
            return attr.parseAttr(index, &parser, value);
        }
    };
}

#endif // GROOVE_MODEL_MODEL_WALKER_H
