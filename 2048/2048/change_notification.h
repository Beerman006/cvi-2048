#ifndef __change_notification_H__
#define __change_notification_H__

#ifdef __cplusplus
    extern "C" {
#endif

#include "cvidef.h"

typedef void * ListenerList;
typedef void (*ChangeHandler)(void *, void *);
typedef void (*ClearDataHandler)(void *);
        
typedef struct ChangeData {
    void *target;
    void *data;
    ChangeHandler handler;
} ChangeData;

void ChangeHandlerClear(ListenerList *listeners, ClearDataHandler handler);
void ChangeHandlerAdd(ListenerList *listeners, ChangeData data);
void *ChangeHandlerRemove(ListenerList *listeners, ChangeData data);

void ChangeHandlerNotifyListeners(ListenerList listeners);

#ifdef __cplusplus
    }
#endif

#endif  /* ndef __change_notification_H__ */
