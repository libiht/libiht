#include "../commons/lbr.h"
#include "../commons/cpu.h"
#include "../commons/types.h"
#include "../commons/debug.h"

#define LBR_STATE_TAG 'SbrL'

KIPI_BROADCAST_WORKER enable_lbr_wrap;
KIPI_BROADCAST_WORKER disable_lbr_wrap;

NTSTATUS DriverEntry(PDRIVER_OBJECT driverObject, PUNICODE_STRING regPath);
void DriverExit(PDRIVER_OBJECT driverObject);
