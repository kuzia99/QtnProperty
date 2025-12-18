#include "propertydelegatearray.h"
#include "PropertyArray/propertyarray.h"
#include "QtnProperty/Delegates/PropertyDelegateFactory.h"
#include "QtnProperty/Delegates/Utils/PropertyEditorAux.h"
#include "QtnProperty/Delegates/Utils/PropertyEditorHandler.h"

void regArrayDelegate()
{
    QtnPropertyDelegateFactory::staticInstance().registerDelegateDefault(
        &QtnPropertyArrayBase::staticMetaObject,
        &qtnCreateDelegate<QtnPropertyDelegateArray, QtnPropertyArrayBase>,
        "ArrayDelegate");
}

class QtnPropertyArrayLineEditBttnHandler
    : public QtnPropertyEditorBttnHandler<QtnPropertyArrayBase,
          QtnLineEditBttn>
{
public:
    QtnPropertyArrayLineEditBttnHandler(
        QtnPropertyDelegate* delegate, QtnLineEditBttn& editor);

protected:
    virtual void onToolButtonClick() override;
    virtual void updateEditor() override;

private:
    void onToolButtonClicked(bool);
    void onEditingFinished();
};

QtnPropertyDelegateArray::QtnPropertyDelegateArray(QtnPropertyArrayBase &owner)
    : QtnPropertyDelegateTypedEx<QtnPropertyArrayBase>(owner)
{
    PropertyArray v = owner.value();

    for (size_t i = 0; i < v.m_vecData.size(); ++i)
    {
        addSubProperty(owner.createItemProperty(i));
    }
}

void QtnPropertyDelegateArray::applyAttributesImpl(const QtnPropertyDelegateInfo &info)
{
    // PropertyArray v = owner().value();

    // v.m_vecData.push_back("ddd");
    // v.m_vecData.push_back("d");
    // v.m_vecData.push_back("dd");

    // for (size_t i = 0; i < v.m_vecData.size(); ++i)
    // {
    //     addSubProperty(owner().createItemProperty(i));
    // }
}

void QtnPropertyDelegateArray::drawValueImpl(QStylePainter &painter, const QRect &rect) const
{
    if (stateProperty()->isMultiValue())
    {
        QtnPropertyDelegateTypedEx::drawValueImpl(painter, rect);
        return;
    }

    QColor value = Qt::GlobalColor::darkGreen;

    QRect textRect = rect;

    if (1)
    {
        QRect colorRect = rect;
        colorRect.setWidth(colorRect.height());
        colorRect.adjust(2, 2, -2, -2);

        painter.fillRect(colorRect,
                         painter.style()->standardPalette().color(
                             stateProperty()->isEditableByUser() ? QPalette::Active
                                                                 : QPalette::Disabled,
                             QPalette::Text));
        colorRect.adjust(1, 1, -1, -1);
        painter.fillRect(colorRect, value);

        textRect.setLeft(colorRect.right() + 3);
    }

    if (textRect.isValid())
    {
        QtnPropertyDelegateTyped<QtnPropertyArrayBase>::drawValueImpl(
            painter, textRect);
    }
}

QWidget *QtnPropertyDelegateArray::createValueEditorImpl(QWidget *parent, const QRect &rect, QtnInplaceInfo *inplaceInfo)
{
    QtnLineEditBttn* editor = new QtnLineEditBttn(parent);
    editor->setGeometry(rect);

    new QtnPropertyArrayLineEditBttnHandler(this, *editor);

    if (inplaceInfo)
    {
        editor->lineEdit->selectAll();
    }

    return editor;
}

bool QtnPropertyDelegateArray::propertyValueToStrImpl(QString &strValue) const
{
    return owner().toStr(strValue);
}

QtnPropertyArrayLineEditBttnHandler::QtnPropertyArrayLineEditBttnHandler(
    QtnPropertyDelegate *delegate, QtnLineEditBttn &editor)
    : QtnPropertyEditorHandlerType(delegate, editor)
{
    if (!stateProperty()->isEditableByUser())
    {
        editor.lineEdit->setReadOnly(true);
        editor.toolButton->setEnabled(false);
    }

    QtnPropertyArrayLineEditBttnHandler::updateEditor();
    editor.lineEdit->installEventFilter(this);
    QObject::connect(editor.toolButton, &QToolButton::clicked, this,
                     &QtnPropertyArrayLineEditBttnHandler::onToolButtonClicked);
    QObject::connect(editor.lineEdit, &QLineEdit::editingFinished, this,
                     &QtnPropertyArrayLineEditBttnHandler::onEditingFinished);
}

void QtnPropertyArrayLineEditBttnHandler::onToolButtonClick()
{
    onToolButtonClicked(false);
}

void QtnPropertyArrayLineEditBttnHandler::updateEditor()
{
    QString str;
    property().toStr(str);
    editor().setTextForProperty(stateProperty(), str);
    editor().lineEdit->selectAll();
}

void QtnPropertyArrayLineEditBttnHandler::onToolButtonClicked(bool)
{
    // auto property = &this->property();
    // volatile bool destroyed = false;
    // auto connection = QObject::connect(property, &QObject::destroyed,
    //                                    [&destroyed]() mutable { destroyed = true; });
    // reverted = true;
    // auto dialog = new QColorDialog(property->value(), editorBase());
    // auto dialogContainer = connectDialog(dialog);

    // if (dialog->exec() == QDialog::Accepted && !destroyed)
    // {
    //     property->setValue(dialog->currentColor(), delegate()->editReason());
    // }

    // if (!destroyed)
    //     QObject::disconnect(connection);

    // Q_UNUSED(dialogContainer);
}

void QtnPropertyArrayLineEditBttnHandler::onEditingFinished()
{
    if (canApply())
    {
        property().fromStr(editor().lineEdit->text(), delegate()->editReason());
    }

    applyReset();
}
