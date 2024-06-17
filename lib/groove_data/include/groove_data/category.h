#ifndef GROOVE_DATA_CATEGORY_H
#define GROOVE_DATA_CATEGORY_H

#include <string>
#include <vector>

#include <absl/strings/string_view.h>

#include <tempo_utils/log_stream.h>

namespace groove_data {

    class Category {
    public:
        Category();
        Category(const std::vector<std::shared_ptr<const std::string>> sv);
        Category(const std::vector<std::string> &sv);
        Category(std::initializer_list<std::string> initalizer_list);
        Category(const Category &other);

        bool isEmpty() const;
        std::shared_ptr<const std::string> at(int i) const;
        std::vector<std::shared_ptr<const std::string>>::const_iterator cbegin() const;
        std::vector<std::shared_ptr<const std::string>>::const_iterator cend() const;
        int size() const;

        std::string toString(std::string_view separator = "/") const;

        bool operator<(const Category &other) const;
        bool operator<=(const Category &other) const;
        bool operator>(const Category &other) const;
        bool operator>=(const Category &other) const;
        bool operator==(const Category &other) const;
        bool operator!=(const Category &other) const;
        int compare(const Category &other) const;

        template <typename H>
        friend H AbslHashValue(H h, const Category &category) {
            TU_ASSERT (bool(category.m_category));
            for (auto iterator = category.cbegin(); iterator != category.cend(); iterator++) {
                auto part = *iterator;
                h = H::combine_contiguous(std::move(h), part->data(), part->size());
            }
            return h;
        }

    private:
        std::shared_ptr<std::vector<std::shared_ptr<const std::string>>> m_category;
    };
}

#endif // GROOVE_DATA_CATEGORY_H