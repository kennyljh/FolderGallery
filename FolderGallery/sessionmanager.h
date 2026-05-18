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

        /**
         * @brief SessionManager - constructor, provided sessionName
         * will be displayed in future sessions for identification
         * @param sessionName
         */
        SessionManager(QString sessionName);

        int getThreadSession();

        int getCurrentCards();

        int getCardsPerRow();

        int getMaxCards();

        bool getCardRenderStatus();

        /**
         * @brief start - create a fresh session with updated
         * metadata and identifier
         * @param windowSize
         * @param cardWidth
         * @param cardLimit
         * @param cardBaseline
         */
        void start(QSize windowSize, int cardWidth,
                    int cardLimit, int cardBaseline);

        /**
         * @brief reset - returns metadata values to
         * default
         */
        void reset();

        /**
         * @brief updateCurrentCards - sets currentCards
         * by the given amount
         * @param val
         */
        void updateCurrentCards(int val);

        /**
         * @brief updateRenderStatus - change render
         * status of metadata
         * @param status
         */
        void updateRenderStatus(bool status);

        /**
         * @brief increaseMaxCards - increase the maxCards
         * count by the number of rounds. If result exceeds
         * limit, maxCards becomes the limit.
         * @param rowIncrease
         * @param cardLimit
         */
        void increaseMaxCards(int rowIncrease, int cardLimit);
};

#endif // SESSIONMANAGER_H
