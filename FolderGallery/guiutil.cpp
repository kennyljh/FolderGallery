#include "guiutil.h"

#include <QWidget>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QAbstractAnimation>

GUIUtil::GUIUtil() {}

void GUIUtil::applyWidgetFade(QWidget &widget, int duration){

    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(&widget);
    widget.setGraphicsEffect(effect);

    QPropertyAnimation *fade = new QPropertyAnimation(effect, "opacity");
    fade->setDuration(duration);
    fade->setStartValue(0.0);
    fade->setEndValue(1.0);
    fade->setEasingCurve(QEasingCurve::InOutQuad);
    fade->start(QAbstractAnimation::DeleteWhenStopped);

    QAbstractAnimation::connect(fade, &QPropertyAnimation::finished,
        [&widget](){
            widget.setGraphicsEffect(nullptr);
        }
    );
}

void GUIUtil::populateCBox(QComboBox &cbox,
                            QStringList &list, QString &current){

    for (const auto &item : list) cbox.addItem(item);
    cbox.setCurrentText(current);
}














