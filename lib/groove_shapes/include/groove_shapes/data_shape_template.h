//#ifndef DATA_SHAPE_TEMPLATE_H
//#define DATA_SHAPE_TEMPLATE_H
//
//#include <QString>
//#include <QVector>
//#include <tempo_utils/option_template.h>
//
//#include "base_data_shape.h"
//
///**
// *
// * @tparam C the container type for a set of records
// * @tparam B the bound type for specifying the set of records to retrieve
// * @tparam I the iterator type
// * @tparam K the key type
// * @tparam V the value type
// */
//template<class C, class B, class I, typename K, typename V>
//class DataShape {
//
//public:
//    DataShape();
//    virtual ~DataShape();
//
//    virtual QStringList listShapeIds() const = 0;
//    virtual int countShapes() const = 0;
//
//    virtual C getValues(const QString &shapeId,
//                        const B &lower,
//                        const B &upper,
//                        I *iter,
//                        int limit) = 0;
//    virtual Option<V> getValue(const QString &shapeId, const K &key, const Option<V> &value) = 0;
//    virtual bool hasValue(const QString &shapeId, const K &key, const Option<V> &value) = 0;
//};
//
//template <class C, class B, class I, typename K, typename V>
//DataShape<C,B,I,K,V>::DataShape()
//{
//}
//
//template <class C, class B, class I, typename K, typename V>
//DataShape<C,B,I,K,V>::~DataShape()
//{
//}
//
//#endif // DATA_SHAPE_TEMPLATE_H
