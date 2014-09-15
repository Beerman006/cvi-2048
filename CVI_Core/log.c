#include <toolbox.h>
#include "log.h"

static Log *globalLog;

// TODO: this should all really be thread safe...
struct Log {
    HashTableType userData;
    LogAssertHandler DoAssert;
};

static void HandleLogAssert(Log *log, int passed, char *fileName, int lineNumber, AssertReason reason, char *message) {
    DoAssert(passed, fileName, lineNumber, message);
}

Log *LogGetGlobal() {
    return globalLog;
}

void LogSetGlobal(Log *log) {
    LogDispose(LogGetGlobal());
    globalLog = log;
}

static Log *GetOrMakeGlobalLog() {
    Log *log = LogGetGlobal();
    if (!log) {
        log = LogCreate();
        LogSetGlobal(log);
    }
    return log;
}

void LogGlobalAssert(int passed, char *fileName, int lineNumber, AssertReason reason, char *message) {
    LogAssert(GetOrMakeGlobalLog(), passed, fileName, lineNumber, reason, message);
}

void LogSetPrivateData(Log *log, char *key, void *privateData) {
    LOG_ASSERT_REASON(log, ArgumentNullReason);
    if (!log->userData) {
        HashTableCreate(10, C_STRING_KEY, 0, sizeof(void *), &log->userData);
    }
    
    HashTableInsertItem(log->userData, key, &privateData);
}

void *LogGetPrivateData(Log *log, char *key) {
    LOG_ASSERT_REASON(log && log->userData, ArgumentNullReason);
    void *data;
    HashTableGetItem(log->userData, key, &data, sizeof(data));
    return data;
}

void LogClearPrivateData(Log *log, char *key) {
    LOG_ASSERT_REASON(log && log->userData, ArgumentNullReason);
    HashTableRemoveItem(log->userData, key, 0, 0);
}

void LogSetAssertHandler(Log *log, LogAssertHandler handler) {
    LOG_ASSERT_REASON(log, ArgumentNullReason);
    log->DoAssert = handler;
}

Log *LogCreate() {
    Log *log = calloc(1, sizeof(Log));
    log->DoAssert = HandleLogAssert;
    return log;
}

void LogDispose(Log *log) {
    if (!log) {
        return;
    }
    if (log == LogGetGlobal()) {
        globalLog = 0;
    }
    HashTableDispose(log->userData);
    log->userData = 0;
    free(log);
}

void LogAssert(Log *log, int passed, char *fileName, int lineNumber, AssertReason reason, char *message) {
    log->DoAssert(log, passed, fileName, lineNumber, reason, message);
}
