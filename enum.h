#ifndef ENUM_H
#define ENUM_H

/* Status will be used in fn. return type */
typedef enum
{
    e_success,
    e_failure
} Status;

typedef enum
{
    e_help,
    e_view,
    e_edit
} OperationType;

#endif