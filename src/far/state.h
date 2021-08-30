#ifndef FAR_STATE_H
#define FAR_STATE_H

enum far_state
{
    FAR_FSM_BEGIN,
    FAR_FSM_DESC_BEGIN,
    FAR_FSM_DESC_CONT,
    FAR_FSM_SEQ_BEGIN,
    FAR_FSM_SEQ_NL,
    FAR_FSM_SEQ_CONT,
    FAR_FSM_NL,
    FAR_FSM_PAUSE,
    FAR_FSM_END,
    FAR_FSM_ERROR
};

#endif
