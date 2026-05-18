#ifndef GUIUTIL_H
#define GUIUTIL_H

#include <QWidget>
#include <QComboBox>
#include <QStringList>
#include <QString>

class GUIUtil {
    public:
        GUIUtil();

        /**
         * @brief applyWidgetFade - apply fade animation effect
         * to desired widget
         * @param widget
         * @param duration
         */
        void applyWidgetFade(QWidget &widget, int duration);

        /**
         * @brief populateCBox - populates given combo box
         * with list of strings and sets current selected
         * option
         * @param cbox
         * @param list
         * @param selected
         */
        void populateCBox(QComboBox &cbox, QStringList &list,
                            QString &selected);
};

#endif // GUIUTIL_H
