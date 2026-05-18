#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QString>
#include <QSize>

/**
 * @brief The SessionManager class - responsible for deciding
 * and keeping track of the number of cards(QWidget) to display
 * for a given instance of a widget's window size and the
 * card's size, only rendering as much as needed and not the
 * entirety.
 *
 * Each generated session has a unique identifier.
 */
class SessionManager{

    private:
        struct sessionMetadata{
            QString sessionName;
            int threadSession;
            int currentCards;
            int cardsPerRow;
            int maxCards;
            bool cardRenderStatus;
        };

        sessionMetadata metadata;

        void generateSession(QSize windowSize, int cardWidth,
                                int cardLimit, int cardBaseline);

        void resetSession();

        void setCurrentCards(int val);

        void setRenderStatus(bool status);

        void incrementMaxCardsByRow(int row, int limit);

    public:
        SessionManager();

        SessionManager(QString sessionName);

        int getThreadSession();

        int getCurrentCards();

        int getCardsPerRow();

        int getMaxCards();

        bool getCardRenderStatus();

        void start(QSize windowSize, int cardWidth,
                    int cardLimit, int cardBaseline);

        void reset();

        void updateCurrentCards(int val);

        void updateRenderStatus(bool status);

        void increaseMaxCards(int rowIncrease, int cardLimit);
};

#endif // SESSIONMANAGER_H
