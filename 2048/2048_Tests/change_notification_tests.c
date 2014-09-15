#include <ansi_c.h>
#include "../../CVI_Test/CVI_Test.h"
#include "../../2048/2048/change_notification.h"

static int wasNotified;
static ListenerList listeners;
static ChangeData changeData;

/// REGION START Tests

void TESTEXPORT GetsSingleNotification(TestContext *context) {
    ChangeHandlerAdd(&listeners, changeData);
    ChangeHandlerNotifyListeners(listeners);

    ASSERT_TRUE(wasNotified, "No change notification!");
}

void TESTEXPORT RemovedGetsNoNotifications(TestContext *context) {
    ChangeHandlerAdd(&listeners, changeData);
    ChangeHandlerRemove(&listeners, changeData);
    ChangeHandlerNotifyListeners(listeners);

    ASSERT_FALSE(wasNotified, "Was notified!");
}

void TESTEXPORT GetsMultipleNotifications(TestContext *context) {
    ChangeHandlerAdd(&listeners, changeData);
    ChangeHandlerAdd(&listeners, changeData);
    ChangeHandlerNotifyListeners(listeners);

    ASSERT_INT_EQUAL(2, wasNotified, "Not notified twice!");
}

void TESTEXPORT RemovedOnceGetsOneNotification(TestContext *context) {
    ChangeHandlerAdd(&listeners, changeData);
    ChangeHandlerAdd(&listeners, changeData);
    ChangeHandlerRemove(&listeners, changeData);
    ChangeHandlerNotifyListeners(listeners);

    ASSERT_INT_EQUAL(1, wasNotified, "Was not notified once!");
}

void TESTEXPORT ClearedGetsNoNotifications(TestContext *context) {
    ChangeHandlerAdd(&listeners, changeData);
    ChangeHandlerAdd(&listeners, changeData);
    ChangeHandlerAdd(&listeners, changeData);
    ChangeHandlerClear(&listeners, 0);

    ChangeHandlerNotifyListeners(listeners);
    ASSERT_FALSE(wasNotified, "Was notified!");
}
/// REGION END

static void HandleNotification(void *target, void *data) {
    wasNotified++;
}

static void InitNotificationTest(TestContext *context) {
    listeners = 0;
    wasNotified = 0;
    changeData.data = 0;
    changeData.target = 0;
    changeData.handler = HandleNotification;
}

static void CleanupNotificationTest(TestContext *context) {
    ChangeHandlerRemove(&listeners, changeData);
    memset(&changeData, 0, sizeof(changeData));
}

BEGIN_MODULE_TEST(change_notification)
    ADD_TEST(GetsSingleNotification, InitNotificationTest, CleanupNotificationTest)
    ADD_TEST(GetsMultipleNotifications, InitNotificationTest, CleanupNotificationTest)
    ADD_TEST(RemovedGetsNoNotifications, InitNotificationTest, CleanupNotificationTest)
    ADD_TEST(ClearedGetsNoNotifications, InitNotificationTest, CleanupNotificationTest)
    ADD_TEST(RemovedOnceGetsOneNotification, InitNotificationTest, CleanupNotificationTest)
END_MODULE_TEST
