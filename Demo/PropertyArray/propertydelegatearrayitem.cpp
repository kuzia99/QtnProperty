#include "propertydelegatearrayitem.h"

#include "PropertyArray/propertyarray.h"
#include "QtnProperty/Delegates/PropertyDelegateFactory.h"
#include "QtnProperty/PropertyView.h"

#include <QStyle>
#include <functional>

static QByteArray qtnArrayIndexAttr()
{
    return QByteArrayLiteral("arrayIndex");
}

static QByteArray qtnArrayItemDelegateName()
{
    return QByteArrayLiteral("ArrayItem");
}

QtnPropertyDelegateArrayItem::QtnPropertyDelegateArrayItem(QtnPropertyQStringBase& owner)
    : QtnPropertyDelegateQStringCallback(owner)
{
}

void QtnPropertyDelegateArrayItem::Register(QtnPropertyDelegateFactory& factory)
{
    // Register on QStringBase (works for QStringCallback items too),
    // and select it via delegateInfo.name == "ArrayItem".
    factory.registerDelegate(&QtnPropertyQStringBase::staticMetaObject,
        &qtnCreateDelegate<QtnPropertyDelegateArrayItem, QtnPropertyQStringBase>,
        qtnArrayItemDelegateName());
}

void QtnPropertyDelegateArrayItem::applyAttributesImpl(const QtnPropertyDelegateInfo& info)
{
    QtnPropertyDelegateQStringCallback::applyAttributesImpl(info);
    info.loadAttribute(qtnArrayIndexAttr(), m_index);
}

static void drawToolButton(QtnPropertyDelegateArrayItem* delegate,
    const QtnDrawContext& context, const QtnSubItem& item, const QIcon& icon,
    const QString& text, bool enabled)
{
    auto style = context.style();

    QStyleOptionToolButton option;
    context.initStyleOption(option);

    // Compute state locally (can't call protected QtnPropertyDelegate::state() here).
    QStyle::State st = QStyle::State_Active;
    if (enabled && delegate->stateProperty()->isEditableByUser())
        st |= QStyle::State_Enabled;
    if (context.isActive)
    {
        st |= QStyle::State_Selected;
        st |= QStyle::State_HasFocus;
    }
    if (enabled)
    {
        if (item.state() == QtnSubItemStateUnderCursor)
            st |= QStyle::State_MouseOver;
        else if (item.state() == QtnSubItemStatePushed)
            st |= QStyle::State_Sunken;
    }
    option.state = st;

    option.state &= ~QStyle::State_HasFocus;
    if (0 == (option.state & QStyle::State_Sunken))
        option.state |= QStyle::State_Raised;

    if (style->inherits("QWindowsVistaStyle"))
        option.styleObject = nullptr;

#ifdef Q_OS_MAC
    option.state &= ~QStyle::State_MouseOver;
#endif

    option.features = QStyleOptionToolButton::None;
    option.subControls = QStyle::SC_ToolButton;
    option.activeSubControls = QStyle::SC_ToolButton;
    option.toolButtonStyle = Qt::ToolButtonIconOnly;
    option.rect = item.rect;
    option.arrowType = Qt::NoArrow;
    if (!icon.availableSizes().empty())
    {
        option.icon = icon;
        option.iconSize = icon.actualSize(item.rect.size());
    } else
    {
        option.text = text.isEmpty() ? QStringLiteral("*") : text;
    }

    style->drawComplexControl(
        QStyle::CC_ToolButton, &option, context.painter, context.widget);
}

void QtnPropertyDelegateArrayItem::createSubItemValuesImpl(
    QtnDrawContext& context, const QRect& valueRect, QList<QtnSubItem>& subItems)
{
    // Reserve space for 3 buttons: [up][down][remove]
    const int h = valueRect.height();
    const int buttonsCount = 3;
    const int buttonsWidth = h * buttonsCount;

    QRect valueOnlyRect = valueRect;
    if (valueOnlyRect.width() > buttonsWidth)
        valueOnlyRect.setRight(valueOnlyRect.right() - buttonsWidth);

    // Default value sub-item (text + inplace editor)
    QtnPropertyDelegateWithValue::createSubItemValuesImpl(context, valueOnlyRect, subItems);

    auto master = property()->getMasterProperty();
    auto arrayProp = qobject_cast<QtnPropertyArrayBase*>(master);
    const int size = arrayProp ? (int)arrayProp->value().m_vecData.size() : 0;

    const bool canMoveUp = arrayProp && (m_index > 0) && (m_index < size);
    const bool canMoveDown = arrayProp && (m_index >= 0) && (m_index + 1 < size);
    const bool canRemove = arrayProp && (m_index >= 0) && (m_index < size);

    QRect buttonsRect = valueRect;
    buttonsRect.setLeft(valueOnlyRect.right() + 1);
    if (!buttonsRect.isValid())
        return;

    auto addBtn = [&](int orderFromRight, const QString& tooltip, const QIcon& icon,
                      bool enabled, std::function<void()> action)
    {
        QtnSubItem item(buttonsRect);
        item.rect.setWidth(h);
        item.rect.setLeft(buttonsRect.right() - h * (orderFromRight + 1) + 1);
        item.rect.setRight(item.rect.left() + h - 1);
        item.trackState();
        item.setTextAsTooltip(tooltip);

        item.drawHandler = [this, icon, enabled](QtnDrawContext& ctx, const QtnSubItem& it) {
            drawToolButton(this, ctx, it, icon, QString(), enabled);
        };

        item.eventHandler = [this, enabled, action](QtnEventContext& ev,
                             const QtnSubItem&, QtnPropertyToEdit* toEdit) -> bool {
            if (!enabled)
                return false;

            if (ev.eventType() == QtnSubItemEvent::ReleaseMouse)
            {
                // Wrap into propertyToEdit to cooperate with PropertyView editing lifecycle.
                toEdit->setup(this->stateProperty(), [action]() -> QWidget* {
                    action();
                    return nullptr;
                });
                return true;
            }
            return false;
        };

        subItems.append(item);
    };

    // Icons: use standard style pixmaps
    auto style = context.style();
    const QIcon iconUp = style->standardIcon(QStyle::SP_ArrowUp, nullptr, context.widget);
    const QIcon iconDown = style->standardIcon(QStyle::SP_ArrowDown, nullptr, context.widget);
    const QIcon iconRemove = style->standardIcon(QStyle::SP_DialogCloseButton, nullptr, context.widget);

    addBtn(2, QtnPropertyView::tr("Переместить вверх"), iconUp, canMoveUp, [arrayProp, idx = m_index, reason = editReason()]() {
        if (arrayProp)
            arrayProp->moveElement(idx, idx - 1, reason);
    });

    addBtn(1, QtnPropertyView::tr("Переместить вниз"), iconDown, canMoveDown, [arrayProp, idx = m_index, reason = editReason()]() {
        if (arrayProp)
            arrayProp->moveElement(idx, idx + 1, reason);
    });

    addBtn(0, QtnPropertyView::tr("Удалить"), iconRemove, canRemove, [arrayProp, idx = m_index, reason = editReason()]() {
        if (arrayProp)
            arrayProp->removeElement(idx, reason);
    });
}

