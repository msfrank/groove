#ifndef GROOVE_SHAPES_SUMMARY_GROUP_SHAPE_H
#define GROOVE_SHAPES_SUMMARY_GROUP_SHAPE_H

#include <absl/container/flat_hash_map.h>

#include <groove_data/data_types.h>
#include <groove_math/math_types.h>
#include <groove_math/reducer_traits.h>
#include <groove_model/column_traits.h>
#include <groove_model/model_types.h>
#include <groove_units/unit_dimension.h>
#include <tempo_utils/iterator_template.h>

#include "base_shape.h"
#include "shapes_result.h"
#include "groove_math/reducer_template.h"

namespace groove_shapes {

    struct SummaryGroupDatum {
        groove_data::DoubleRange range;
        double value;
        groove_data::DatumFidelity fidelity;

        SummaryGroupDatum();
        SummaryGroupDatum(const groove_data::DoubleRange &range, double value, groove_data::DatumFidelity fidelity);
    };

    class SummaryGroupShape : public BaseShape {

    public:
        SummaryGroupShape(
            std::shared_ptr<groove_model::GrooveDatabase> database,
            const std::string &shapeId);

        tempo_utils::Status configure(const SourceDescriptor &source) override;

        bool isValid() const;

        groove_shapes::DataShapeType getShapeType() const override;

        absl::flat_hash_set<std::string>::const_iterator itemsBegin() const;
        absl::flat_hash_set<std::string>::const_iterator itemsEnd() const;

        groove_data::DataValueType getSummaryGroupColumnValueType(const std::string &itemId) const;

    private:
        absl::flat_hash_set<std::string> m_items;
        absl::flat_hash_map<
            std::string,
            std::shared_ptr<groove_model::IndexedColumn<groove_model::DoubleDouble>>> m_columns;

        tempo_utils::Result<std::shared_ptr<Iterator<double>>> getItemValues(
            const std::string &itemId,
            const groove_data::DoubleRange &range);

    public:

        template <typename ReducerType,
            typename IdentityType = typename groove_math::ReducerTraits<ReducerType>::OutputType>
        tempo_utils::Result<SummaryGroupDatum> getSummaryGroupValue(
            const std::string &itemId,
            const groove_data::DoubleRange &range,
            const IdentityType &identity)
        {
            auto getItemValuesResult = getItemValues(itemId, range);
            if (getItemValuesResult.isStatus())
                return getItemValuesResult.getStatus();
            auto input = getItemValuesResult.getResult();

            groove_math::Reducer<ReducerType> reducer(identity);
            auto reduceResult = reducer.reduce(input);
            if (reduceResult.isStatus())
                return reduceResult.getStatus();
            auto value = reduceResult.getResult();

            return SummaryGroupDatum(range, value, groove_data::DatumFidelity::FIDELITY_VALID);
        }

    };
}

#endif // GROOVE_SHAPES_SUMMARY_GROUP_SHAPE_H