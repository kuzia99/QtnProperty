#include "propertyarray.h"
#include "QtnProperty/Auxiliary/PropertyDelegateInfo.h"

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

    // Use custom delegate for array items to provide per-item controls.
    QtnPropertyDelegateInfo info;
    info.name = "ArrayItem";
    info.attributes["arrayIndex"] = int(index);
    result->setDelegateInfo(info);
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

void QtnPropertyArrayBase::addElement(
    const QString& value, QtnPropertyChangeReason reason)
{
    PropertyArray arr = this->value();
    arr.m_vecData.push_back(value);
    // Emitting Children change reason to force view update children
    setValue(arr, reason | QtnPropertyChangeReasonValue | QtnPropertyChangeReasonChildren);
}

void QtnPropertyArrayBase::removeElement(
    int index, QtnPropertyChangeReason reason)
{
    PropertyArray arr = this->value();
    if (index < 0 || index >= (int)arr.m_vecData.size())
        return;

    arr.m_vecData.erase(arr.m_vecData.begin() + index);
    setValue(arr, reason | QtnPropertyChangeReasonValue | QtnPropertyChangeReasonChildren);
}

void QtnPropertyArrayBase::moveElement(
    int from, int to, QtnPropertyChangeReason reason)
{
    PropertyArray arr = this->value();
    const int size = (int)arr.m_vecData.size();
    if (from < 0 || from >= size || to < 0 || to >= size || from == to)
        return;

    const QString value = arr.m_vecData.at(from);
    arr.m_vecData.erase(arr.m_vecData.begin() + from);
    arr.m_vecData.insert(arr.m_vecData.begin() + to, value);

    setValue(arr, reason | QtnPropertyChangeReasonValue | QtnPropertyChangeReasonChildren);
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
