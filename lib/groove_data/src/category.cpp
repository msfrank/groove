
#include <absl/strings/string_view.h>
#include <absl/strings/str_join.h>

#include <groove_data/category.h>

groove_data::Category::Category()
    : m_category(std::make_shared<std::vector<std::shared_ptr<const std::string>>>())
{
}

groove_data::Category::Category(const std::vector<std::shared_ptr<const std::string>> sv)
    : Category()
{
    for (auto iterator = sv.cbegin(); iterator != sv.cend(); iterator++) {
        m_category->push_back(*iterator);
    }
}

groove_data::Category::Category(const std::vector<std::string> &sv)
    : Category()
{
    for (auto iterator = sv.cbegin(); iterator != sv.cend(); iterator++) {
        m_category->push_back(std::make_shared<const std::string>(*iterator));
    }
}

groove_data::Category::Category(std::initializer_list<std::string> initalizer_list)
    : Category()
{
    for (auto iterator = initalizer_list.begin(); iterator != initalizer_list.end(); iterator++) {
        m_category->push_back(std::make_shared<const std::string>(*iterator));
    }
}

groove_data::Category::Category(const Category &other)
    : m_category(other.m_category)
{
}

bool
groove_data::Category::isEmpty() const
{
    return m_category->empty();
}

std::shared_ptr<const std::string>
groove_data::Category::at(int i) const
{
    return m_category->at(i);
}

std::vector<std::shared_ptr<const std::string>>::const_iterator
groove_data::Category::cbegin() const
{
    return m_category->cbegin();
}

std::vector<std::shared_ptr<const std::string>>::const_iterator
groove_data::Category::cend() const
{
    return m_category->cend();
}

int groove_data::Category::size() const
{
    return m_category->size();
}

std::string
groove_data::Category::toString(std::string_view separator) const
{
    auto iterator = m_category->cbegin();
    if (iterator == m_category->cend())
        return {};
    std::string s(*(*iterator));
    for (; iterator != m_category->cend(); iterator++) {
        absl::StrAppend(&s, separator);
        auto part = *iterator;
        absl::StrAppend(&s, part->c_str());
    }
    return s;
}

bool
groove_data::Category::operator<(const Category &other) const
{
    auto literator = m_category->cbegin();
    auto riterator = other.m_category->cbegin();

    for (; literator != m_category->cend(); literator++, riterator++) {
        if (riterator == other.m_category->cend())
            return false;
        auto lstr = *literator;
        auto rstr = *riterator;
        auto cmp = lstr->compare(std::string_view(rstr->data(), rstr->size()));
        if (cmp < 0)
            return true;
        if (cmp > 0)
            return false;
    }

    if (riterator != other.m_category->cend())
        return true;
    return false;
}

bool
groove_data::Category::operator<=(const Category &other) const
{
    auto literator = m_category->cbegin();
    auto riterator = other.m_category->cbegin();

    for (; literator != m_category->cend(); literator++, riterator++) {
        if (riterator == other.m_category->cend())
            return false;
        auto lstr = *literator;
        auto rstr = *riterator;
        auto cmp = lstr->compare(std::string_view(rstr->data(), rstr->size()));
        if (cmp < 0)
            return true;
        if (cmp > 0)
            return false;
    }

    return true;
}

bool
groove_data::Category::operator>(const Category &other) const
{
    auto literator = m_category->cbegin();
    auto riterator = other.m_category->cbegin();

    for (; literator != m_category->cend(); literator++, riterator++) {
        if (riterator == other.m_category->cend())
            return true;
        auto lstr = *literator;
        auto rstr = *riterator;
        auto cmp = lstr->compare(std::string_view(rstr->data(), rstr->size()));
        if (cmp < 0)
            return false;
        if (cmp > 0)
            return true;
    }

    return false;
}

bool
groove_data::Category::operator>=(const Category &other) const
{
    auto literator = m_category->cbegin();
    auto riterator = other.m_category->cbegin();

    for (; literator != m_category->cend(); literator++, riterator++) {
        if (riterator == other.m_category->cend())
            return true;
        auto lstr = *literator;
        auto rstr = *riterator;
        auto cmp = lstr->compare(std::string_view(rstr->data(), rstr->size()));
        if (cmp < 0)
            return false;
        if (cmp > 0)
            return true;
    }

    if (riterator == other.m_category->cend())
        return true;
    return false;
}

bool groove_data::Category::operator==(const Category &other) const
{
    if (m_category->size() != other.m_category->size())
        return false;
    for (int i = 0; i < m_category->size(); i++) {
        if (*(m_category->at(i)) != *(other.m_category->at(i)))
            return false;
    }
    return true;
}

bool groove_data::Category::operator!=(const Category &other) const
{
    return !(*this == other);
}

int groove_data::Category::compare(const Category &other) const
{
    auto literator = m_category->cbegin();
    auto riterator = other.m_category->cbegin();

    for (; literator != m_category->cend(); literator++, riterator++) {
        if (riterator == other.m_category->cend())
            return 1;
        auto lstr = *literator;
        auto rstr = *riterator;
        auto cmp = lstr->compare(std::string_view(rstr->data(), rstr->size()));
        if (cmp < 0)
            return -1;
        if (cmp > 0)
            return 1;
    }

    if (riterator == other.m_category->cend())
        return 0;
    return -1;

}
