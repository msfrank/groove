#ifndef GROOVE_MODEL_SHARED_STRING_BUFFER_H
#define GROOVE_MODEL_SHARED_STRING_BUFFER_H

#include <string>

#include "model_types.h"

namespace groove_model {

    class SharedStringBuffer : public arrow::Buffer {
    public:
        explicit SharedStringBuffer(std::shared_ptr<const std::string> bytes);

        static std::shared_ptr<SharedStringBuffer> create(std::string_view &bytes);
        static std::shared_ptr<SharedStringBuffer> create(const std::string &bytes);
        static std::shared_ptr<SharedStringBuffer> create(std::string &&bytes);

    private:
        std::shared_ptr<const std::string> m_bytes;
    };
}

#endif // GROOVE_MODEL_SHARED_STRING_BUFFER_H