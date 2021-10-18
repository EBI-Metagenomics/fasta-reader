#ifndef STATE_H
#define STATE_H

enum state
{
    STATE_BEGIN,
    STATE_DESC_BEGIN,
    STATE_DESC_CONT,
    STATE_SEQ_BEGIN,
    STATE_SEQ_NL,
    STATE_SEQ_CONT,
    STATE_NL,
    STATE_PAUSE,
    STATE_END,
    STATE_ERROR
};

#endif
