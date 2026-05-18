#include "sessionmanager.h"

#include <QRandomGenerator>
#include <QString>
#include <QDebug>

SessionManager::SessionManager() {

    metadata.sessionName = "noname-session";
    metadata.threadSession = 0;
    metadata.currentCards = 0;
    metadata.cardsPerRow = 0;
    metadata.maxCards = 0;
    metadata.cardRenderStatus = false;
}

SessionManager::SessionManager(QString sessionName) {

    metadata.sessionName = sessionName;
    metadata.threadSession = 0;
    metadata.currentCards = 0;
    metadata.cardsPerRow = 0;
    metadata.maxCards = 0;
    metadata.cardRenderStatus = false;
}

void SessionManager::generateSession(QSize windowSize, int cardWidth,
                                int cardLimit, int cardBaseline){

    bool idGenerated = false;
    do {
        int sessionID = QRandomGenerator::global()->generate();
        if (sessionID != metadata.threadSession){
            metadata.threadSession = sessionID;
            idGenerated = true;
        }
    }
    while (!idGenerated);

    // account for margins of directory cards
    metadata.cardsPerRow = windowSize.width() / (cardWidth + 10);
    // account for QLabel of directory cards
    int rowsToDisplay = windowSize.height() / ((cardWidth + 30) * 1.414);
    // to give the illusion that more images are loaded beyond the window
    // height but not the entirety
    rowsToDisplay += 2;
    metadata.maxCards = metadata.cardsPerRow * rowsToDisplay;

    if (metadata.maxCards > cardLimit) metadata.maxCards = cardLimit;

    if (metadata.maxCards < cardBaseline) metadata.maxCards = cardBaseline;
}

void SessionManager::resetSession(){

    metadata.threadSession = 0;
    metadata.currentCards = 0;
    metadata.cardsPerRow = 0;
    metadata.maxCards = 0;
    metadata.cardRenderStatus = false;
}

void SessionManager::setCurrentCards(int val){
    metadata.currentCards = val;
}

void SessionManager::setRenderStatus(bool status){
    metadata.cardRenderStatus = status;
}

void SessionManager::incrementMaxCardsByRow(int row, int limit){

    int result = metadata.maxCards * row;
    if (result > limit){
        metadata.maxCards = limit;
        qDebug() << "MaxCards: " + QString::number(result) +
                    ", exceeded limit: " + QString::number(limit);
    }
    else {
        qDebug() << "Increased maxCards by: " +
                    QString::number(result - metadata.maxCards);
        metadata.maxCards = result;
    }
}

int SessionManager::getThreadSession(){
    return metadata.threadSession;
}

int SessionManager::getCurrentCards(){
    return metadata.currentCards;
}

int SessionManager::getCardsPerRow(){
    return metadata.cardsPerRow;
}

int SessionManager::getMaxCards(){
    return metadata.maxCards;
}

bool SessionManager::getCardRenderStatus(){
    return metadata.cardRenderStatus;
}

void SessionManager::start(QSize windowSize, int cardWidth,
                            int cardLimit, int cardBaseline){

    generateSession(windowSize, cardWidth, cardLimit, cardBaseline);
    qDebug() << metadata.sessionName + " session generated: " +
                QString::number(metadata.threadSession) +
                ", currentCards: " + QString::number(metadata.currentCards) +
                ", cardsPerRow: " + QString::number(metadata.cardsPerRow) +
                ", maxCards: " + QString::number(metadata.maxCards);
}

void SessionManager::reset(){
    resetSession();
}

void SessionManager::updateCurrentCards(int val){
    setCurrentCards(val);
}

void SessionManager::updateRenderStatus(bool status){
    setRenderStatus(status);
}

void SessionManager::increaseMaxCards(int rowIncrease, int cardLimit){
    incrementMaxCardsByRow(rowIncrease, cardLimit);
}













