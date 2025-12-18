#include "propertyarray.h"

PropertyArray::PropertyArray() {}

bool PropertyArray::operator==(const PropertyArray &other) const
{
    return other.m_vecData == m_vecData;
}

bool PropertyArray::operator!=(const PropertyArray &other) const
{
    return !(operator==(other));
}

QDataStream& operator>>(QDataStream& stream, PropertyArray& arr)
{
    return stream;
}

QDataStream& operator<<(QDataStream& stream, const PropertyArray& arr)
{
    return stream;
}

QtnProperty* QtnPropertyArrayBase::createItemProperty(size_t index)
{
    auto result = new QtnPropertyQStringCallback(nullptr);

    result->setDisplayName("index " + QString::number(index) + " DisplayName");
    result->setName(QString::number(index) + " Name");
    result->setDescription(QString::number(index) + " description");
    result->setCallbackValueGet([index, this]() -> QString {
        PropertyArray arr = value();
        return arr.m_vecData.at(index);
    });
    result->setCallbackValueSet([index, this](QString str, QtnPropertyChangeReason reason) {
        PropertyArray arr = this->value();
        arr.m_vecData.at(index) = str;
        this->setValue(arr, reason);
    });

    return result;
}

void QtnPropertyArrayBase::setArray(PropertyArray array)
{
    if (m_array == array)
        return;

    Q_EMIT propertyWillChange(QtnPropertyChangeReasonValue, &array, 0);

    m_array = array;

    Q_EMIT propertyDidChange(QtnPropertyChangeReasonValue);

}

bool QtnPropertyArrayBase::fromStrImpl(const QString &str, QtnPropertyChangeReason reason)
{
    Q_UNUSED(reason);
    Q_UNUSED(str);
    return false;
}

bool QtnPropertyArrayBase::toStrImpl(QString &str) const
{
    str = "loil";
    Q_UNUSED(str);
    return false;
}

QtnPropertyArray::QtnPropertyArray(QObject *parent)
    : QtnSinglePropertyValue<QtnPropertyArrayBase>(parent)
{

}
