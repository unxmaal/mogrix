/* IRIX device driver stub — maps to dummy device.
 * IRIX 6.5 has no TUN/TAP device. Users can configure
 * DeviceType = dummy or use raw_socket mode. */
#include "../system.h"
#include "../device.h"
#include "../logger.h"
#include "../xalloc.h"

int device_fd = -1;
char *device = NULL;
char *iface = NULL;
static const char *device_info = "IRIX dummy device";
static uint64_t device_total_in = 0;
static uint64_t device_total_out = 0;

static bool setup_device(void) {
    device = xstrdup("irix-dummy");
    iface = xstrdup("irix-dummy");
    logger(LOG_INFO, "%s (%s) is a %s", device, iface, device_info);
    return true;
}

static void close_device(void) {
    free(device); device = NULL;
    free(iface); iface = NULL;
}

static bool read_packet(vpn_packet_t *packet) {
    (void)packet;
    return false;
}

static bool write_packet(vpn_packet_t *packet) {
    device_total_out += packet->len;
    return true;
}

static void dump_device_stats(void) {
    logger(LOG_DEBUG, "Statistics for %s %s:", device_info, device);
    logger(LOG_DEBUG, " total bytes in:  %10"PRIu64, device_total_in);
    logger(LOG_DEBUG, " total bytes out: %10"PRIu64, device_total_out);
}

const devops_t os_devops = {
    .setup = setup_device,
    .close = close_device,
    .read = read_packet,
    .write = write_packet,
    .dump_stats = dump_device_stats,
};
