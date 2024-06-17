
#include <groove_data/data_types.h>
#include <tempo_utils/log_stream.h>

tempo_utils::LogMessage&&
groove_data::operator<<(tempo_utils::LogMessage &&message, CategoryRange range)
{
    message.m_buffer << "CategoryRange(";

    if (range.start.isEmpty()) {
        message.m_buffer << "start: {}";
    } else {
        message.m_buffer << "start: " << range.start.getValue().toString();
    }
    if (range.end.isEmpty()) {
        message.m_buffer << "end: {}";
    } else {
        message.m_buffer << "end: " << range.end.getValue().toString();
    }

    message.m_buffer << ", start_exclusive: " << range.start_exclusive
                     << ", end_exclusive: " << range.end_exclusive
                     << ")";
    return std::move(message);
}

tempo_utils::LogMessage&&
groove_data::operator<<(tempo_utils::LogMessage &&message, DoubleRange range)
{
    message.m_buffer << "DoubleRange(";

    if (range.start.isEmpty()) {
        message.m_buffer << "start: {}";
    } else {
        message.m_buffer << "start: " << range.start.getValue();
    }
    if (range.end.isEmpty()) {
        message.m_buffer << "end: {}";
    } else {
        message.m_buffer << "end: " << range.end.getValue();
    }

    message.m_buffer << ", start_exclusive: " << range.start_exclusive
                     << ", end_exclusive: " << range.end_exclusive
                     << ")";
    return std::move(message);
}

tempo_utils::LogMessage&&
groove_data::operator<<(tempo_utils::LogMessage &&message, Int64Range range)
{
    message.m_buffer << "Int64Range(";

    if (range.start.isEmpty()) {
        message.m_buffer << "start: {}";
    } else {
        message.m_buffer << "start: " << range.start.getValue();
    }
    if (range.end.isEmpty()) {
        message.m_buffer << "end: {}";
    } else {
        message.m_buffer << "end: " << range.end.getValue();
    }

    message.m_buffer << ", start_exclusive: " << range.start_exclusive
                     << ", end_exclusive: " << range.end_exclusive
                     << ")";
    return std::move(message);
}

tempo_utils::LogMessage&&
groove_data::operator<<(tempo_utils::LogMessage &&message, groove_data::DataVectorType t)
{
    switch (t) {
        case DataVectorType::VECTOR_TYPE_DOUBLE_DOUBLE:
            message.m_buffer << "DOUBLE_DOUBLE";
            break;
        case DataVectorType::VECTOR_TYPE_DOUBLE_INT64:
            message.m_buffer << "DOUBLE_INT64";
            break;
        case DataVectorType::VECTOR_TYPE_DOUBLE_STRING:
            message.m_buffer << "DOUBLE_STRING";
            break;
        case DataVectorType::VECTOR_TYPE_INT64_DOUBLE:
            message.m_buffer << "INT64_DOUBLE";
            break;
        case DataVectorType::VECTOR_TYPE_INT64_INT64:
            message.m_buffer << "INT64_INT64";
            break;
        case DataVectorType::VECTOR_TYPE_INT64_STRING:
            message.m_buffer << "INT64_STRING";
            break;
        case DataVectorType::VECTOR_TYPE_CATEGORY_DOUBLE:
            message.m_buffer << "CATEGORY_DOUBLE";
            break;
        case DataVectorType::VECTOR_TYPE_CATEGORY_INT64:
            message.m_buffer << "CATEGORY_INT64";
            break;
        case DataVectorType::VECTOR_TYPE_CATEGORY_STRING:
            message.m_buffer << "CATEGORY_STRING";
            break;
        default:
            message.m_buffer << "UNKNOWN";
            break;
    }
    return std::move(message);
}