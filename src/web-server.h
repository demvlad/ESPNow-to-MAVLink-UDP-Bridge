#ifndef _WEB_SERVER_H_
#define _WEB_SERVER_H_

void webSwerverSetup();
void webServerRun();
void loadMacFromStorage(uint8_t mac[6]);
void loadWifiFromStorage();

#endif