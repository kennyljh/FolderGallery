#ifndef GUIUTIL_H
#define GUIUTIL_H

#include <QWidget>

class GUIUtil {
    public:
        GUIUtil();

        void applyWidgetFade(QWidget *widget, int duration);
};

#endif // GUIUTIL_H
