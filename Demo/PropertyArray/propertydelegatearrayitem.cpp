#include "propertydelegatearrayitem.h"

#include "PropertyArray/propertyarray.h"
#include "QtnProperty/Delegates/PropertyDelegateFactory.h"
#include "QtnProperty/PropertyView.h"

#include <QApplication>
#include <QMouseEvent>
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
    // Reserve space for drag handle + 3 buttons: [drag][up][down][remove]
    const int h = valueRect.height();
    const int buttonsCount = 4;
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

    // ---- drag handle (left-most button) ----
    {
        QtnSubItem dragItem(buttonsRect);
        dragItem.rect.setWidth(h);
        // left-most within buttonsRect
        dragItem.rect.setLeft(buttonsRect.left());
        dragItem.rect.setRight(dragItem.rect.left() + h - 1);
        dragItem.trackState();
        dragItem.setTextAsTooltip(QtnPropertyView::tr("Перетащить для сортировки"));

        dragItem.drawHandler = [this](QtnDrawContext& ctx, const QtnSubItem& it) {
            // Simple "grip" glyph. If you prefer an icon, we can switch to a style pixmap.
            drawToolButton(this, ctx, it, QIcon(), QString::fromUtf8("\xE2\x89\xA1"), true); // "≡"
        };

        dragItem.eventHandler = [this, arrayProp](QtnEventContext& ev,
                                 const QtnSubItem&, QtnPropertyToEdit*) -> bool {
            if (!arrayProp)
                return false;

            switch (ev.eventType())
            {
                case QEvent::MouseButtonPress:
                case QEvent::MouseButtonDblClick:
                {
                    auto me = ev.eventAs<QMouseEvent>();
                    if (me->button() != Qt::LeftButton)
                        return false;
                    m_dragStartPos = me->pos();
                    m_dragging = false;
                    return true;
                }

                case QEvent::MouseMove:
                {
                    auto me = ev.eventAs<QMouseEvent>();
                    if (!(me->buttons() & Qt::LeftButton))
                        return false;

                    if (!m_dragging)
                    {
                        if ((me->pos() - m_dragStartPos).manhattanLength() <
                            QApplication::startDragDistance())
                        {
                            return true;
                        }
                        m_dragging = true;
                        ev.widget->viewport()->setCursor(Qt::ClosedHandCursor);
                    }
                    return true;
                }

                case QtnSubItemEvent::ReleaseMouse:
                {
                    if (!m_dragging)
                        return true; // click without move

                    m_dragging = false;
                    ev.widget->viewport()->unsetCursor();

                    // Drop position -> find target property and compute target index.
                    auto me = ev.eventAs<QtnSubItemEvent>();
                    const QPoint pos = me ? me->pos() : QPoint();
                    QRect targetRect;
                    QtnPropertyBase* target = ev.widget->getPropertyAt(pos, &targetRect);
                    if (!target)
                        return true;

                    // Only reorder within the same array.
                    if (target != arrayProp && target->getMasterProperty() != arrayProp)
                        return true;

                    int targetIndex = -1;
                    if (target == arrayProp)
                    {
                        // dropped on the array header -> append
                        targetIndex = (int)arrayProp->value().m_vecData.size() - 1;
                        targetRect = QRect(); // treat as "after"
                    } else
                    {
                        const auto info = target->delegateInfo();
                        if (!info)
                            return true;
                        targetIndex = info->getAttribute<int>(qtnArrayIndexAttr(), -1);
                        if (targetIndex < 0)
                            return true;
                    }

                    const int size = (int)arrayProp->value().m_vecData.size();
                    if (m_index < 0 || m_index >= size)
                        return true;

                    // insert position: before/after depending on cursor in row
                    int insertPos = targetIndex;
                    if (!targetRect.isValid() || pos.y() > targetRect.center().y())
                        insertPos = targetIndex + 1;

                    if (insertPos > size)
                        insertPos = size;

                    // convert "insertPos" to resulting index after removing 'from'
                    int to = insertPos;
                    if (m_index < to)
                        to -= 1;
                    if (to < 0)
                        to = 0;
                    if (to >= size)
                        to = size - 1;

                    arrayProp->moveElement(m_index, to, editReason());
                    return true;
                }

                default:
                    break;
            }

            return false;
        };

        subItems.append(dragItem);
    }

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

