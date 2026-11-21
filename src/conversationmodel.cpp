#include "conversationmodel.h"

ConversationModel::ConversationModel(QObject* parent)
    : QObject(parent)
    , m_round(0)
    , m_status("klar")
{
}

void ConversationModel::addChunk(const QString& text)
{
    m_lastText += text;
    emit changed();
}

void ConversationModel::startRound()
{
    ++m_round;
    m_lastText.clear();
    m_status = "kører";
    emit changed();
}

void ConversationModel::endRound()
{
    m_status = "klar";
    emit changed();
}

QString ConversationModel::lastText() const
{
    return m_lastText;
}

int ConversationModel::round() const
{
    return m_round;
}

QString ConversationModel::status() const
{
    return m_status;
}
