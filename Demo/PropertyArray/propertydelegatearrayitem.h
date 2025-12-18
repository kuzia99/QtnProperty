#pragma once

#include "QtnProperty/Delegates/Core/PropertyDelegateQString.h"

class QtnPropertyDelegateFactory;

// Delegate for array item (QtnPropertyQStringCallback) that adds:
// - remove button
// - move up / move down buttons
class QtnPropertyDelegateArrayItem : public QtnPropertyDelegateQStringCallback
{
    Q_DISABLE_COPY(QtnPropertyDelegateArrayItem)

public:
    explicit QtnPropertyDelegateArrayItem(QtnPropertyQStringBase& owner);

    static void Register(QtnPropertyDelegateFactory& factory);

protected:
    void applyAttributesImpl(const QtnPropertyDelegateInfo& info) override;

    void createSubItemValuesImpl(QtnDrawContext& context, const QRect& valueRect,
        QList<QtnSubItem>& subItems) override;

private:
    int m_index = -1;

    // drag&drop reorder state (handled by "grab handle" sub-item)
    QPoint m_dragStartPos;
    bool m_dragging = false;
};

