#ifndef EVENT_TABLE_SHAPE_H
#define EVENT_TABLE_SHAPE_H

#include <absl/container/flat_hash_map.h>

#include <groove_data/data_types.h>
#include <groove_iterator/peek_iterator_template.h>
#include <groove_model/column_traits.h>
#include <groove_model/model_types.h>
#include <groove_model/variant_value.h>
#include <tempo_utils/iterator_template.h>

#include "base_shape.h"
#include "shape_types.h"
#include "groove_model/indexed_variant_column.h"

namespace groove_shapes {

    struct EventTableDatum {
        groove_data::Category key;
        absl::flat_hash_map<std::string, std::pair<groove_model::VariantValue,groove_data::DatumFidelity>> values;

        EventTableDatum();
        EventTableDatum(
            const groove_data::Category &key,
            const absl::flat_hash_map<
                std::string,
                std::pair<groove_model::VariantValue,groove_data::DatumFidelity>> &values);
    };

    class EventTableDatumIterator : public Iterator<EventTableDatum> {
    public:
        EventTableDatumIterator();
        EventTableDatumIterator(
            const std::vector<
                std::pair<
                    std::string,
                    groove_iterator::PeekIterator<
                        groove_model::VariantValueDatumIterator<groove_data::Category>,
                        groove_model::VariantValueDatum<groove_data::Category>>>> &inputs);
        bool getNext(EventTableDatum &datum) override;
    private:
        std::vector<
            std::pair<
                std::string,
                groove_iterator::PeekIterator<
                    groove_model::VariantValueDatumIterator<groove_data::Category>,
                    groove_model::VariantValueDatum<groove_data::Category>>>> m_inputs;
    };

    class EventTableShape : public BaseShape {

    public:
        EventTableShape(
            std::shared_ptr<groove_model::GrooveDatabase> database,
            const std::string &shapeId);

        tempo_utils::Status configure(const SourceDescriptor &source) override;

        bool isValid() const;

        groove_shapes::DataShapeType getShapeType() const override;

        absl::flat_hash_set<std::string>::const_iterator itemsBegin() const;
        absl::flat_hash_set<std::string>::const_iterator itemsEnd() const;

        groove_data::DataValueType getEventTableColumnValueType(const std::string &itemId) const;

        tempo_utils::Result<EventTableDatum> getEventTableValue(const groove_data::Category &key);

        tempo_utils::Result<EventTableDatumIterator> getEventTableValues(
            const groove_data::CategoryRange &range);

    private:
        absl::flat_hash_set<std::string> m_items;
        absl::flat_hash_map<
            std::string,
            std::shared_ptr<
                groove_model::AbstractIndexedVariantColumn<
                    groove_data::Category,groove_data::CategoryRange>>> m_columns;
    };
}

#endif // EVENT_TABLE_SHAPE_H