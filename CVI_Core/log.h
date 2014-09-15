#ifndef __log_H__
#define __log_H__

#ifdef __cplusplus
    extern "C" {
#endif

//==============================================================================
// Include files

#include "cvidef.h"
      
typedef enum AssertReason {
    NoReason,
    AnyReason,
    ArgumentNullReason,
    ArgumentOutOfRangeReason,
    InvalidOperationReason
} AssertReason;        
        
#ifdef _CVI_DEBUG_
#define LOG_ASSERT(passed) LogGlobalAssert(!!(passed), __FILE__, __LINE__, NoReason, 0)
#define LOG_ASSERTMSG(passed, msg) LogGlobalAssert(!!(passed), __FILE__, __LINE__, NoReason, msg)
#define LOG_ASSERT_REASON(passed, reason) LogGlobalAssert(!!(passed), __FILE__, __LINE__, reason, 0)
#define LOG_ASSERTMSG_REASON(passed, msg, reason) LogGlobalAssert(!!(passed), __FILE__, __LINE__, reason, msg)
#else
#define LOG_ASSERT(passed)
#define LOG_ASSERTMSG(passed, msg)
#define LOG_ASSERT_REASON(passed, reason)
#define LOG_ASSERTMSG_REASON(passed, msg, reason)
#endif
        
typedef struct Log Log;        
      
typedef void (*LogAssertHandler)(Log *, int, char *, int, AssertReason, char *);

Log *LogGetGlobal();
void LogSetGlobal(Log *log);
void LogGlobalAssert(int passed, char *fileName, int lineNumber, AssertReason reason, char *message);
      
Log *LogCreate();
void LogDispose(Log *log);
                
void LogSetAssertHandler(Log *log, LogAssertHandler handler);

void LogSetPrivateData(Log *log, char *key, void *privateData);
void *LogGetPrivateData(Log *log, char *key);
void LogClearPrivateData(Log *log, char *key);

void LogAssert(Log *log, int passed, char *fileName, int lineNumber, AssertReason reason, char *message);

#ifdef __cplusplus
    }
#endif

#endif  /* ndef __log_H__ */
