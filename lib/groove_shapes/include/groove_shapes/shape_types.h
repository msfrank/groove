#ifndef GROOVE_SHAPES_SHAPE_TYPES_H
#define GROOVE_SHAPES_SHAPE_TYPES_H

#include <string>

#include <tempo_config/config_types.h>
#include <tempo_utils/url.h>

namespace groove_shapes {

    enum class DataShapeType {
        INVALID,
        SHAPE_UNKNOWN,

        // series shapes
        SHAPE_SERIES_GROUP,           // individually display a set of one or more series
        SHAPE_SERIES_STACK,           // display a set of one or more series stacked
        SHAPE_SERIES_ANNOTATION,
        SHAPE_SERIES_INTERVAL,

        // bar shapes
        SHAPE_BAR_GROUP,
        SHAPE_BAR_STACK,
        SHAPE_BAR_ANNOTATION,
        SHAPE_BAR_INTERVAL,

        // pie shapes
        SHAPE_PIE_GROUP,
        SHAPE_PIE_STACK,
        SHAPE_PIE_ANNOTATION,
        SHAPE_PIE_INTERVAL,

        // spreadsheet shapes
        SHAPE_INFO_TABLE,             // generic table which displays any type of data
        SHAPE_EVENT_TABLE,
        SHAPE_CONTINGENCY_TABLE,
        SHAPE_FREQUENCY_TABLE,
        SHAPE_STEM_AND_LEAF_TABLE,

        // control shapes
        SHAPE_BOOLEAN_SELECTOR,       // special case of SINGLE, returns either true or false
        SHAPE_SINGLE_SELECTOR,        // selector returning a single scalar
        SHAPE_LIST_SELECTOR,          // multi selector structured as a list of attrs
        SHAPE_MAP_SELECTOR,           // multi selector structured as a map of key -> attr pairs
        SHAPE_TIMERANGE_SELECTOR,     // selector returning a time range structured as a map with 'start' and 'end' keys

        // summary shapes
        SHAPE_SUMMARY_GROUP,
    };

    enum class ColumnTargetType {
        INVALID,
        COLUMN_TARGET_KEY,
        COLUMN_TARGET_VALUE,
    };

    class SourceDescriptor {

    public:
        SourceDescriptor();
        SourceDescriptor(
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            const std::string &columnId);
        SourceDescriptor(const tempo_utils::Url &datasetUrl, const std::string &modelId);
        SourceDescriptor(const SourceDescriptor &other);

        bool isValid() const;

        tempo_utils::Url getDatasetUrl() const;
        std::string getModelId() const;
        std::string getColumnId() const;

    private:
        struct SourcePriv {
            tempo_utils::Url datasetUrl;
            std::string modelId;
            std::string columnId;
        };
        std::shared_ptr<const SourcePriv> m_priv;
    };

    template <typename DefType>
    struct ShapeTraits {};

    class ShapeDef {

    public:
        ShapeDef();
        ShapeDef(DataShapeType type);
        ShapeDef(const ShapeDef &other);

        DataShapeType getType() const;

    private:
        DataShapeType m_type;
    };

    struct BarAnnotation : public ShapeDef {
        BarAnnotation();
    };

    struct BarGroup : public ShapeDef {
        BarGroup();
    };

    struct BarStack : public ShapeDef {
        BarStack();
    };

    struct EventTable : public ShapeDef {
        EventTable();
    };

    struct InfoTable : public ShapeDef {
        InfoTable();
    };

    struct PieAnnotation : public ShapeDef {
        PieAnnotation();
    };

    struct PieGroup : public ShapeDef {
        PieGroup();
    };

    struct SeriesAnnotation : public ShapeDef {
        SeriesAnnotation();
    };

    struct SeriesGroup : public ShapeDef {
        SeriesGroup();
    };

    struct SeriesStack : public ShapeDef {
        SeriesStack();
    };

    struct SummaryGroup : public ShapeDef {
        SummaryGroup();
    };
}

#endif // GROOVE_SHAPES_SHAPE_TYPES_H