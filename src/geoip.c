#include "geoip.h"

#include <maxminddb.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

static MMDB_s mmdb;
static bool available = false;

bool geoip_init(const char *db_path)
{
    struct stat st;

    if (stat(db_path, &st) != 0)
        return false;

    if (!S_ISREG(st.st_mode))
        return false;

    int status = MMDB_open(db_path, MMDB_MODE_MMAP, &mmdb);
    if (status != MMDB_SUCCESS) {
        available = false;
        return false;
    }

    available = true;
    return true;
}

void geoip_destroy(void)
{
    if (available) {
        MMDB_close(&mmdb);
        available = false;
    }
}

bool geoip_is_available(void)
{
    return available;
}

int geoip_get_country_code(struct sockaddr_storage *addr, char *code, size_t len)
{
    if (!available || !addr || !code || len < 3)
        return -1;

    int mmdb_error;

    MMDB_lookup_result_s result = MMDB_lookup_sockaddr(&mmdb, (struct sockaddr *)addr, &mmdb_error);
    if (mmdb_error != MMDB_SUCCESS)
        return -1;

    if (!result.found_entry)
        return -1;

    MMDB_entry_data_s entry_data;
    int status = MMDB_get_value(&result.entry, &entry_data, "country", "iso_code", NULL);
    if (status != MMDB_SUCCESS)
        return -1;

    if (!entry_data.has_data)
        return -1;

    if (entry_data.type != MMDB_DATA_TYPE_UTF8_STRING)
        return -1;

    size_t copy_len = entry_data.data_size;
    if (copy_len >= len)
        copy_len = len - 1;

    memcpy(code, entry_data.utf8_string, copy_len);
    code[copy_len] = '\0';

    return 0;
}