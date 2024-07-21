#include "pch.h"
#include "kmd-ext.h"
#include "kmd.h"

WINDBG_EXTENSION_APIS ExtensionApis;


////////////////////////////////////////////////////////////////////////////////
//
// Function   : DebugExtensionInitiaize
// Description: Initializes the debug extension.
//
// Inputs     : Version - Pointer to ULONG variable to store the debug extension version.
//              Flags   - Pointer to ULONG variable to store the debug extension flags.
// Outputs    : HRESULT - Indicates the result of the function.
extern "C" HRESULT CALLBACK
DebugExtensionInitiaize(PULONG Version, PULONG Flags) {
	*Version = DEBUG_EXTENSION_VERSION(EXT_MAJOR_VER, EXT_MINOR_VER);
	*Flags = 0;
	return S_OK;
}


////////////////////////////////////////////////////////////////////////////////
//
// Function   : DebugExtensionNotify
// Description: Handles debug extension notifications.
//
// Inputs     : Notify   - The type of notification.
//              Argument - The additional argument for the notification.
// Outputs    : None.
extern "C" void CALLBACK
DebugExtensionNotify(ULONG Notify, ULONG64 Argument) {
	UNREFERENCED_PARAMETER(Argument);
	switch (Notify) {
	case DEBUG_NOTIFY_SESSION_ACTIVE:
		break;
	case DEBUG_NOTIFY_SESSION_INACTIVE:
		break;
	case DEBUG_NOTIFY_SESSION_ACCESSIBLE:
		break;
	case DEBUG_NOTIFY_SESSION_INACCESSIBLE:
		break;
	}
	return;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function   : DebugExtensionUninitialize
// Description: Uninitializes the debug extension.
//
// Inputs     : None.
// Outputs    : None.
extern "C" void CALLBACK
DebugExtensionUninitialize(void) {
	return;
}

struct lbr_ioctl_request lbr_req;
struct bts_ioctl_request bts_req;
bool lbr_enable = 0, bts_enable = 0;

////////////////////////////////////////////////////////////////////////////////
//
// Function   : EnableLBR
// Description: Enables Last Branch Recording (LBR) for a specified process.
//
// Inputs     : pid - Optional parameter indicating the process ID. Default value is 0.
// Outputs    : lbr_ioctl_request - A structure containing the LBR configuration.
extern "C" struct lbr_ioctl_request CALLBACK
EnableLBR(unsigned int pid = 0) {
	if (lbr_enable == 1) {
		return lbr_req;
	}
	lbr_enable = 1;
	lbr_req = enable_lbr(pid);
	dprintf("LIBIHT-WINDBG: enable lbr for pid : %d\n", lbr_req.lbr_config.pid);
	return lbr_req;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function   : DisableLBR
// Description: Disables Last Branch Recording (LBR) for the current process.
//
// Inputs     : None.
// Outputs    : None.
extern "C" void CALLBACK
DisableLBR() {
	if (lbr_enable == 0) {
		return;
	}
	lbr_enable = 0;
	disable_lbr(lbr_req);
	dprintf("LIBIHT-WINDBG: disable lbr for pid : %d\n", lbr_req.lbr_config.pid);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function   : DumpLBR
// Description: Dumps the Last Branch Record (LBR) information for the current process.
//
// Inputs     : None.
// Outputs    : None.
extern "C" void CALLBACK
DumpLBR() {
	if (lbr_enable == 0) {
		return;
	}
	dprintf("LIBIHT-WINDBG: dump lbr for pid : %d\n", lbr_req.lbr_config.pid);
	dump_lbr(lbr_req);
	unsigned long long lbr_tos = lbr_req.buffer->lbr_tos;
	for (int i = 0; i < (int)lbr_tos; i++) {
		dprintf("MSR_LBR_NHM_FROM[ %d ]: %llx\n", i, lbr_req.buffer->entries[i].from);
		dprintf("MSR_LBR_NHM_FROM[ %d ]: %llx\n", i, lbr_req.buffer->entries[i].to);
	}
}

////////////////////////////////////////////////////////////////////////////////
//
// Function   : EnableBTS
// Description: Enables Branch Trace Store (BTS) for a specified process.
//
// Inputs     : pid - Optional parameter indicating the process ID. Default value is 0.
// Outputs    : bts_ioctl_request - A structure containing the BTS configuration.
extern "C" struct bts_ioctl_request CALLBACK
EnableBTS(unsigned int pid = 0) {
	if (bts_enable == 1) {
		return bts_req;
	}
	bts_enable = 1;
	bts_req = enable_bts(pid);
	dprintf("LIBIHT-WINDBG: enable bts for pid : %d\n", bts_req.bts_config.pid);
	return bts_req;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function   : DisableBTS
// Description: Disables Branch Trace Store (BTS) for the current process.
//
// Inputs     : None.
// Outputs    : None.
extern "C" void CALLBACK
DisableBTS() {
	if (bts_enable == 0) {
		return;
	}
	bts_enable = 0;
	disable_bts(bts_req);
	dprintf("LIBIHT-WINDBG: disable bts for pid : %d\n", bts_req.bts_config.pid);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function   : DumpBTS
// Description: Dumps the Branch Trace Store (BTS) information for the current process.
//
// Inputs     : None.
// Outputs    : None.
extern "C" void CALLBACK
DumpBTS() {
	if (bts_enable == 0) {
		return;
	}
	dprintf("LIBIHT-WINDBG: dump lbr for pid : %d\n", bts_req.bts_config.pid);
	dump_bts(bts_req);
	int bts_tos = 32;
	dprintf("%d\n", bts_tos);
	for (int i = 0; i < bts_tos; i++) {
		dprintf("0x%llx 0x%llx %llu\n", bts_req.buffer->bts_buffer_base[i].from, bts_req.buffer->bts_buffer_base[i].to, bts_req.buffer->bts_buffer_base[i].misc);
	}
	dprintf("\n");
}
