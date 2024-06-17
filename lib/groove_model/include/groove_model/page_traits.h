#ifndef GROOVE_MODEL_PAGE_TRAITS_H
#define GROOVE_MODEL_PAGE_TRAITS_H

#include <groove_data/category_double_vector.h>
#include <groove_data/category_int64_vector.h>
#include <groove_data/category_string_vector.h>
#include <groove_data/double_double_vector.h>
#include <groove_data/double_int64_vector.h>
#include <groove_data/double_string_vector.h>
#include <groove_data/int64_double_vector.h>
#include <groove_data/int64_int64_vector.h>
#include <groove_data/int64_string_vector.h>

#include "model_types.h"

namespace groove_model {

    template<>
    struct PageTraits<CategoryDouble, groove_data::CollationMode::COLLATION_INDEXED> {
        using DefType = CategoryDouble;
        using DatumType = groove_data::CategoryDoubleDatum;
        using IteratorType = groove_data::CategoryDoubleDatumIterator;
        using VectorType = groove_data::CategoryDoubleVector;
    };

    template<>
    struct PageTraits<CategoryInt64, groove_data::CollationMode::COLLATION_INDEXED> {
        using DefType = CategoryInt64;
        using DatumType = groove_data::CategoryInt64Datum;
        using IteratorType = groove_data::CategoryInt64DatumIterator;
        using VectorType = groove_data::CategoryInt64Vector;
    };

    template<>
    struct PageTraits<CategoryString, groove_data::CollationMode::COLLATION_INDEXED> {
        using DefType = CategoryString;
        using DatumType = groove_data::CategoryStringDatum;
        using IteratorType = groove_data::CategoryStringDatumIterator;
        using VectorType = groove_data::CategoryStringVector;
    };

    template<>
    struct PageTraits<DoubleDouble, groove_data::CollationMode::COLLATION_INDEXED> {
        using DefType = DoubleDouble;
        using DatumType = groove_data::DoubleDoubleDatum;
        using IteratorType = groove_data::DoubleDoubleDatumIterator;
        using VectorType = groove_data::DoubleDoubleVector;
    };

    template<>
    struct PageTraits<DoubleInt64, groove_data::CollationMode::COLLATION_INDEXED> {
        using DefType = DoubleInt64;
        using DatumType = groove_data::DoubleInt64Datum;
        using IteratorType = groove_data::DoubleInt64DatumIterator;
        using VectorType = groove_data::DoubleInt64Vector;
    };

    template<>
    struct PageTraits<DoubleString, groove_data::CollationMode::COLLATION_INDEXED> {
        using DefType = DoubleString;
        using DatumType = groove_data::DoubleStringDatum;
        using IteratorType = groove_data::DoubleStringDatumIterator;
        using VectorType = groove_data::DoubleStringVector;
    };

    template<>
    struct PageTraits<Int64Double, groove_data::CollationMode::COLLATION_INDEXED> {
        using DefType = Int64Double;
        using DatumType = groove_data::Int64DoubleDatum;
        using IteratorType = groove_data::Int64DoubleDatumIterator;
        using VectorType = groove_data::Int64DoubleVector;
    };

    template<>
    struct PageTraits<Int64Int64, groove_data::CollationMode::COLLATION_INDEXED> {
        using DefType = Int64Int64;
        using DatumType = groove_data::Int64Int64Datum;
        using IteratorType = groove_data::Int64Int64DatumIterator;
        using VectorType = groove_data::Int64Int64Vector;
    };

    template<>
    struct PageTraits<Int64String, groove_data::CollationMode::COLLATION_INDEXED> {
        using DefType = Int64String;
        using DatumType = groove_data::Int64StringDatum;
        using IteratorType = groove_data::Int64StringDatumIterator;
        using VectorType = groove_data::Int64StringVector;
    };
}

#endif // GROOVE_MODEL_PAGE_TRAITS_H
