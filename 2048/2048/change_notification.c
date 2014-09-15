#include <ansi_c.h>
#include "toolbox.h"
#include "change_notification.h"
#include "../../CVI_Core/log.h"

static int CompareChangeData(void *item1, void *item2) {
    ChangeData *data1 = (ChangeData *)item1;
    ChangeData *data2 = (ChangeData *)item2;
    
    return data1->target == data2->target && data1->handler == data2->handler;
}

static int Notify(int index, void *ptr, void *callbackData) {
    ChangeData *data = *(ChangeData **)ptr;
    LOG_ASSERT_REASON(data, ArgumentNullReason);
    
    data->handler(data->target, data->data);
    return 0;
}

static ChangeData *CopyData(ChangeData *data) {
    ChangeData *copy = calloc(1, sizeof(ChangeData));
    copy->data = data->data;
    copy->target = data->target;
    copy->handler = data->handler;
    
    return copy;
}

static void *RemoveHandlerAt(ListType *list, size_t idx) {
    LOG_ASSERT_REASON(idx, ArgumentOutOfRangeReason);
    
    ChangeData *copy;
    ListRemoveItem(*list, &copy, idx);
    void *clientData = copy->data;
    free(copy);
    
    if (!ListNumItems(*list)) {
        ListDispose(*list);
        *list = 0;
    }
    return clientData;
}

void ChangeHandlerNotifyListeners(ListenerList listeners) {
    ListType list = (ListType)listeners;
    if (list) {
        ListApplyToEach(list, 1, Notify, 0);
    }
}

void ChangeHandlerClear(ListenerList *listeners, ClearDataHandler handleClearData) {
    LOG_ASSERT_REASON(listeners, ArgumentNullReason);
    if (!*listeners) {
        return;
    }
    
    ListType *list = (ListType *)listeners;
    
    size_t idx = ListNumItems(*list);
    while(idx) {
        void *clientData = RemoveHandlerAt(list, idx);
        if (handleClearData != 0) {
            handleClearData(clientData);
        }
        idx = ListNumItems(*list);
    }
}

void ChangeHandlerAdd(ListenerList *listeners, ChangeData data) {
    LOG_ASSERT_REASON(listeners, ArgumentNullReason);
    ListType *listPtr = (ListType *)listeners;
    ChangeData *copy = CopyData(&data);
    
    if (!*listPtr){
        *listPtr = ListCreate(sizeof(copy));
    }
    
    ListInsertItem(*listPtr, &copy, END_OF_LIST);
}

void *ChangeHandlerRemove(ListenerList *listeners, ChangeData data) {
    LOG_ASSERT_REASON(listeners, ArgumentNullReason);
    
    if (!*listeners) {
        return 0;
    }
    
    ListType *list = (ListType *)listeners;
    
    size_t idx = ListFindItem(*list, &data, FRONT_OF_LIST, CompareChangeData);
    return RemoveHandlerAt(list, idx);
}
