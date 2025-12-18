#include "propertydelegatearray.h"
#include "propertydelegatearrayitem.h"
#include "PropertyArray/propertyarray.h"
#include "QtnProperty/Delegates/PropertyDelegateFactory.h"
#include "QtnProperty/Delegates/Utils/PropertyEditorAux.h"
#include "QtnProperty/Delegates/Utils/PropertyEditorHandler.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QToolButton>

void regArrayDelegate()
{
    QtnPropertyDelegateFactory::staticInstance().registerDelegateDefault(
        &QtnPropertyArrayBase::staticMetaObject,
        &qtnCreateDelegate<QtnPropertyDelegateArray, QtnPropertyArrayBase>,
        "ArrayDelegate");

    // delegate for array items (QtnPropertyQStringCallback) with remove/move buttons
    QtnPropertyDelegateArrayItem::Register(QtnPropertyDelegateFactory::staticInstance());
}

class ArrayEditorWidget : public QWidget
{
public:
    ArrayEditorWidget(QWidget* parent) : QWidget(parent)
    {
        auto layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(2);

        label = new QLabel(this);
        layout->addWidget(label);
        
        layout->addStretch();

        auto btnAdd = new QToolButton(this);
        btnAdd->setText("+");
        btnAdd->setAutoRaise(true);
        layout->addWidget(btnAdd);
        this->btnAdd = btnAdd;

        auto btnRemove = new QToolButton(this);
        btnRemove->setText("-");
        btnRemove->setAutoRaise(true);
        layout->addWidget(btnRemove);
        this->btnRemove = btnRemove;
    }

    QLabel* label;
    QToolButton* btnAdd;
    QToolButton* btnRemove;
};

class QtnPropertyArrayEditorHandler
    : public QtnPropertyEditorHandler<QtnPropertyArrayBase, ArrayEditorWidget>
{
public:
    QtnPropertyArrayEditorHandler(
        QtnPropertyDelegate* delegate, ArrayEditorWidget& editor)
        : QtnPropertyEditorHandlerType(delegate, editor)
    {
        updateEditor();

        QObject::connect(editor.btnAdd, &QToolButton::clicked, [this, delegate](){
            property().addElement("New Element", delegate->editReason());
        });

        QObject::connect(editor.btnRemove, &QToolButton::clicked, [this, delegate](){
             int size = (int)property().value().m_vecData.size();
             if (size > 0)
                property().removeElement(size - 1, delegate->editReason());
        });
    }

    void updateEditor() override
    {
        QString t = QString::number(property().value().m_vecData.size()) + ":";
        for (QString s : property().value().m_vecData)
        {
            t += s + ";";
        }
        // int size = (int)property().value().m_vecData.size();
        editor().label->setText(t/*QString("[%1 items]").arg(size)*/);
    }
};


QtnPropertyDelegateArray::QtnPropertyDelegateArray(QtnPropertyArrayBase &owner)
    : QtnPropertyDelegateTypedEx<QtnPropertyArrayBase>(owner)
{
    updateSubProperties();
    m_connection = QObject::connect(&owner, &QtnPropertyBase::propertyDidChange, 
        [this](QtnPropertyChangeReason reason){
            // IMPORTANT:
            // m_subProperties stores QScopedPointer<QtnPropertyBase>, so clear() destroys items.
            // Rebuilding sub-properties on every Value change is unsafe because editors may still
            // reference old sub-properties during commitData/updateEditor (re-entrancy) -> crash.
            // Rebuild only when children set changed (add/remove/reindex).
            if (reason & QtnPropertyChangeReasonChildren) {
                updateSubProperties();
            }
        });
}

QtnPropertyDelegateArray::~QtnPropertyDelegateArray()
{
    QObject::disconnect(m_connection);
}

void QtnPropertyDelegateArray::updateSubProperties()
{
    m_subProperties.clear();

    PropertyArray v = owner().value();
    for (size_t i = 0; i < v.m_vecData.size(); ++i)
    {
        addSubProperty(owner().createItemProperty(i));
    }
}

void QtnPropertyDelegateArray::applyAttributesImpl(const QtnPropertyDelegateInfo &info)
{
    Q_UNUSED(info);
}

void QtnPropertyDelegateArray::drawValueImpl(QStylePainter &painter, const QRect &rect) const
{
    if (stateProperty()->isMultiValue())
    {
        QtnPropertyDelegateTypedEx::drawValueImpl(painter, rect);
        return;
    }

    QString str;
    propertyValueToStrImpl(str);
    
    QtnPropertyDelegateTyped<QtnPropertyArrayBase>::drawValueImpl(
            painter, rect);
}

QWidget *QtnPropertyDelegateArray::createValueEditorImpl(QWidget *parent, const QRect &rect, QtnInplaceInfo *inplaceInfo)
{
    auto editor = new ArrayEditorWidget(parent);
    editor->setGeometry(rect);

    new QtnPropertyArrayEditorHandler(this, *editor);

    return editor;
}

bool QtnPropertyDelegateArray::propertyValueToStrImpl(QString &strValue) const
{
    QString t = QString::number(owner().value().m_vecData.size()) + ":";
    for (QString s : owner().value().m_vecData)
    {
        t += s + ";";
    }
    // size_t size = owner().value().m_vecData.size();
    strValue = t;//QString("[%1 items]").arg(size);
    return true;
}
