
#include <globals.h>
#if PM_TARGET_APP // PM_TARGET_APP build
// PM_TARGET_APP: entry point for OTA applications (OTA = Over The Air - 3rd party installed apps) 
// PM_TARGET_APP: build an external app with the SDK app.mk into a .app.elf
// (see lib/PocketMage_SDK/examples/hello_app/).

void APP_INIT() {
}

void processKB_APP() {
}
void einkHandler_APP() {
}
#endif
