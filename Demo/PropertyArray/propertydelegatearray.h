#pragma once
#include "QtnProperty/Delegates/Utils/PropertyDelegateMisc.h"

class QtnPropertyArrayBase;

class QtnPropertyDelegateArray
    : public QtnPropertyDelegateTypedEx<QtnPropertyArrayBase>
{
    Q_DISABLE_COPY(QtnPropertyDelegateArray)

public:
    QtnPropertyDelegateArray(QtnPropertyArrayBase& owner);

protected:
    void applyAttributesImpl(
        const QtnPropertyDelegateInfo& info) override;

    void drawValueImpl(
        QStylePainter& painter, const QRect& rect) const override;

    QWidget* createValueEditorImpl(QWidget* parent, const QRect& rect,
        QtnInplaceInfo* inplaceInfo = nullptr) override;

    bool propertyValueToStrImpl(QString& strValue) const override;
};
