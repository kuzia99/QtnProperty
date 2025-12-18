#pragma once

#include <vector>
#include <QString>

#include "QtnProperty/Auxiliary/PropertyTemplates.h"
#include "QtnProperty/Core/PropertyQString.h"
#include "QtnProperty/StructPropertyBase.h"

class PropertyArray
{
public:
	PropertyArray();

    bool operator==(const PropertyArray& other) const;
    bool operator!=(const PropertyArray& other) const;

    std::vector<QString> m_vecData;
};

Q_DECLARE_METATYPE(PropertyArray)

QDataStream& operator>>(QDataStream& stream, PropertyArray& arr);
QDataStream& operator<<(QDataStream& stream, const PropertyArray& arr);

class QtnPropertyArrayBase : public QtnStructPropertyBase<PropertyArray, QtnPropertyQStringCallback>
{
    Q_OBJECT
    QtnPropertyArrayBase(const QtnPropertyBase& other) Q_DECL_EQ_DELETE;
public:
    explicit QtnPropertyArrayBase(QObject* parent)
        : QtnStructPropertyBase<PropertyArray, QtnPropertyQStringCallback>(parent)
    {

    }

    QtnProperty* createItemProperty(size_t index);

    PropertyArray array() const
    {
        return m_array;
    }

    void setArray(PropertyArray array);

protected:
    // string conversion implementation
    bool fromStrImpl(
        const QString & str, QtnPropertyChangeReason reason) override;
    bool toStrImpl(QString& str) const override;

    P_PROPERTY_DECL_MEMBER_OPERATORS(QtnPropertyArrayBase)
private:
    PropertyArray m_array;

};

P_PROPERTY_DECL_EQ_OPERATORS(QtnPropertyArrayBase, PropertyArray)

class QtnPropertyArrayCallback
    : public QtnSinglePropertyCallback<QtnPropertyArrayBase>
{
    Q_OBJECT
    QtnPropertyArrayCallback(
        const QtnPropertyArrayCallback& other) Q_DECL_EQ_DELETE;

public:
    explicit QtnPropertyArrayCallback(QObject* parent)
        : QtnSinglePropertyCallback<QtnPropertyArrayBase>(parent)
    {

    }

    P_PROPERTY_DECL_MEMBER_OPERATORS2(
        QtnPropertyArrayCallback, QtnPropertyArrayBase)
};

class QtnPropertyArray : public
                         QtnSinglePropertyValue<QtnPropertyArrayBase>
{
    Q_OBJECT

private:
    QtnPropertyArray(const QtnPropertyArray& other) Q_DECL_EQ_DELETE;

public:
    explicit QtnPropertyArray(QObject* parent = nullptr);

    P_PROPERTY_DECL_MEMBER_OPERATORS2(QtnPropertyArray, QtnPropertyArrayBase)
};

