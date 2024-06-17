#ifndef GROOVE_DATA_CATEGORY_BUILDER_H
#define GROOVE_DATA_CATEGORY_BUILDER_H

#include <arrow/builder.h>

#include "category.h"

namespace groove_data {

    class CategoryBuilder {

    public:
        CategoryBuilder();

        arrow::Status Append(const Category category);
        arrow::Result<std::shared_ptr<arrow::Array>> Finish();

        static std::shared_ptr<arrow::DataType> makeDatatype();

    private:
        std::shared_ptr<arrow::StringBuilder> m_str;
        std::unique_ptr<arrow::ListBuilder> m_cat;
    };

}

#endif // GROOVE_DATA_CATEGORY_BUILDER_H