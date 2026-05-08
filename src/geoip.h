#ifndef IPTRAF_NG_GEOIP_H
#define IPTRAF_NG_GEOIP_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>
#include <stdint.h>

struct sockaddr_storage;

bool geoip_init(const char *db_path);
void geoip_destroy(void);
bool geoip_is_available(void);
int geoip_get_country_code(struct sockaddr_storage *addr, char *code, size_t len);

#endif