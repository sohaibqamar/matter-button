#pragma once

#define CHECK_STATUS(info, status, condition) if ((status) condition) { LOG_ERR("Invalid status while " info ": %d", status); return status; }
#define CHECK_SUCCESS(info, status) CHECK_STATUS(info, status, != 0)

#define CHECK_STATUS_VOID(info, status, condition) if ((status) condition) { LOG_ERR("Invalid status while " info ": %d", status); return; }
#define CHECK_SUCCESS_VOID(info, status) CHECK_STATUS_VOID(info, status, != 0)

#define CHECK_CHIP_STATUS(info, status, condition) if ((status) condition) { LOG_ERR("Invalid status while " info ": %s (%d)", status.AsString(), status.AsInteger()); return status.AsInteger(); }
#define CHECK_CHIP_SUCCESS(info, status) CHECK_CHIP_STATUS(info, status, != CHIP_NO_ERROR)