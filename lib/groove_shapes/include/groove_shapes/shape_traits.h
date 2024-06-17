#ifndef GROOVE_SHAPES_SHAPE_TRAITS_H
#define GROOVE_SHAPES_SHAPE_TRAITS_H

#include "bar_annotation_shape.h"
#include "bar_group_shape.h"
#include "bar_stack_shape.h"
#include "event_table_shape.h"
#include "pie_annotation_shape.h"
#include "pie_group_shape.h"
#include "series_annotation_shape.h"
#include "series_group_shape.h"
#include "series_stack_shape.h"
#include "shape_types.h"
#include "summary_group_shape.h"

namespace groove_shapes {

    template <>
    struct ShapeTraits<BarAnnotation> {
        using ShapeType = BarAnnotationShape;
        using DatumType = BarAnnotationDatum;
        using IteratorType = BarAnnotationDatumIterator;
    };

    template <>
    struct ShapeTraits<BarGroup> {
        using ShapeType = BarGroupShape;
        using DatumType = BarGroupDatum;
        using IteratorType = BarGroupDatumIterator;
    };

    template <>
    struct ShapeTraits<BarStack> {
        using ShapeType = BarStackShape;
        using DatumType = BarStackDatum;
        using IteratorType = BarStackDatumIterator;
    };

    template <>
    struct ShapeTraits<EventTable> {
        using ShapeType = EventTableShape;
        using DatumType = EventTableDatum;
        using IteratorType = EventTableDatumIterator;
    };

    template <>
    struct ShapeTraits<PieAnnotation> {
        using ShapeType = PieAnnotationShape;
        using DatumType = PieAnnotationDatum;
        using IteratorType = PieAnnotationDatumIterator;
    };

    template <>
    struct ShapeTraits<PieGroup> {
        using ShapeType = PieGroupShape;
        using DatumType = PieGroupDatum;
        using IteratorType = std::nullptr_t;
    };

    template <>
    struct ShapeTraits<SeriesAnnotation> {
        using ShapeType = SeriesAnnotationShape;
        using DatumType = SeriesAnnotationDatum;
        using IteratorType = SeriesAnnotationDatumIterator;
    };

    template <>
    struct ShapeTraits<SeriesGroup> {
        using ShapeType = SeriesGroupShape;
        using DatumType = SeriesGroupDatum;
        using IteratorType = SeriesGroupDatumIterator;
    };

    template <>
    struct ShapeTraits<SeriesStack> {
        using ShapeType = SeriesStackShape;
        using DatumType = SeriesStackDatum;
        using IteratorType = SeriesStackDatumIterator;
    };

    template <>
    struct ShapeTraits<SummaryGroup> {
        using ShapeType = SummaryGroupShape;
        using DatumType = SummaryGroupDatum;
        using IteratorType = std::nullptr_t;
    };

}

#endif // GROOVE_SHAPES_SHAPE_TRAITS_H
