#ifndef GROOVE_MODEL_COLUMN_TRAITS_H
#define GROOVE_MODEL_COLUMN_TRAITS_H

#include <groove_data/category_double_vector.h>
#include <groove_data/category_frame.h>
#include <groove_data/category_int64_vector.h>
#include <groove_data/category_string_vector.h>
#include <groove_data/double_double_vector.h>
#include <groove_data/double_frame.h>
#include <groove_data/double_int64_vector.h>
#include <groove_data/double_string_vector.h>
#include <groove_data/int64_double_vector.h>
#include <groove_data/int64_frame.h>
#include <groove_data/int64_int64_vector.h>
#include <groove_data/int64_string_vector.h>

#include "category_column_iterator.h"
#include "double_column_iterator.h"
#include "int64_column_iterator.h"
#include "model_types.h"

namespace groove_model {

    template <>
    struct ColumnTraits<CategoryDouble, groove_data::CollationMode::COLLATION_INDEXED> {
        using DefType = CategoryDouble;
        using DatumType = groove_data::CategoryDoubleDatum;
        using IteratorType = CategoryDoubleColumnIterator;
        using RangeType = groove_data::CategoryRange;
        using VectorType = groove_data::CategoryDoubleVector;
        using FrameType = groove_data::CategoryFrame;
    };

    template <>
    struct ColumnTraits<CategoryInt64, groove_data::CollationMode::COLLATION_INDEXED> {
        using DefType = CategoryInt64;
        using DatumType = groove_data::CategoryInt64Datum;
        using IteratorType = CategoryInt64ColumnIterator;
        using RangeType = groove_data::CategoryRange;
        using VectorType = groove_data::CategoryInt64Vector;
        using FrameType = groove_data::CategoryFrame;
    };

    template <>
    struct ColumnTraits<CategoryString, groove_data::CollationMode::COLLATION_INDEXED> {
        using DefType = CategoryString;
        using DatumType = groove_data::CategoryStringDatum;
        using IteratorType = CategoryStringColumnIterator;
        using RangeType = groove_data::CategoryRange;
        using VectorType = groove_data::CategoryStringVector;
        using FrameType = groove_data::CategoryFrame;
    };

    template <>
    struct ColumnTraits<DoubleDouble, groove_data::CollationMode::COLLATION_INDEXED> {
        using DefType = DoubleDouble;
        using DatumType = groove_data::DoubleDoubleDatum;
        using IteratorType = DoubleDoubleColumnIterator;
        using RangeType = groove_data::DoubleRange;
        using VectorType = groove_data::DoubleDoubleVector;
        using FrameType = groove_data::DoubleFrame;
    };

    template <>
    struct ColumnTraits<DoubleInt64, groove_data::CollationMode::COLLATION_INDEXED> {
        using DefType = DoubleInt64;
        using DatumType = groove_data::DoubleInt64Datum;
        using IteratorType = DoubleInt64ColumnIterator;
        using RangeType = groove_data::DoubleRange;
        using VectorType = groove_data::DoubleInt64Vector;
        using FrameType = groove_data::DoubleFrame;
    };

    template <>
    struct ColumnTraits<DoubleString, groove_data::CollationMode::COLLATION_INDEXED> {
        using DefType = DoubleString;
        using DatumType = groove_data::DoubleStringDatum;
        using IteratorType = DoubleStringColumnIterator;
        using RangeType = groove_data::DoubleRange;
        using VectorType = groove_data::DoubleStringVector;
        using FrameType = groove_data::DoubleFrame;
    };

    template <>
    struct ColumnTraits<Int64Double, groove_data::CollationMode::COLLATION_INDEXED> {
        using DefType = Int64Double;
        using DatumType = groove_data::Int64DoubleDatum;
        using IteratorType = Int64DoubleColumnIterator;
        using RangeType = groove_data::Int64Range;
        using VectorType = groove_data::Int64DoubleVector;
        using FrameType = groove_data::Int64Frame;
    };

    template <>
    struct ColumnTraits<Int64Int64, groove_data::CollationMode::COLLATION_INDEXED> {
        using DefType = Int64Int64;
        using DatumType = groove_data::Int64Int64Datum;
        using IteratorType = Int64Int64ColumnIterator;
        using RangeType = groove_data::Int64Range;
        using VectorType = groove_data::Int64Int64Vector;
        using FrameType = groove_data::Int64Frame;
    };

    template <>
    struct ColumnTraits<Int64String, groove_data::CollationMode::COLLATION_INDEXED> {
        using DefType = Int64String;
        using DatumType = groove_data::Int64StringDatum;
        using IteratorType = Int64StringColumnIterator;
        using RangeType = groove_data::Int64Range;
        using VectorType = groove_data::Int64StringVector;
        using FrameType = groove_data::Int64Frame;
    };
}

#endif // GROOVE_MODEL_COLUMN_TRAITS_H
