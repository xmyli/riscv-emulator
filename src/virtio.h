#ifndef VIRTIO_H
#define VIRTIO_H

#include "device.h"

#include <vector>

#define VIRTIO_IRQ 1

#define VRING_DESC_SIZE 16
#define DESC_NUM 8

#define VIRTIO_BASE 0x10001000
#define VIRTIO_SIZE 0x1000
#define VIRTIO_MAGIC (VIRTIO_BASE + 0x000)
#define VIRTIO_VERSION (VIRTIO_BASE + 0x004)
#define VIRTIO_DEVICE_ID (VIRTIO_BASE + 0x008)
#define VIRTIO_VENDOR_ID (VIRTIO_BASE + 0x00c)
#define VIRTIO_DEVICE_FEATURES (VIRTIO_BASE + 0x010)
#define VIRTIO_DRIVER_FEATURES (VIRTIO_BASE + 0x020)
#define VIRTIO_GUEST_PAGE_SIZE (VIRTIO_BASE + 0x028)
#define VIRTIO_QUEUE_SEL (VIRTIO_BASE + 0x030)
#define VIRTIO_QUEUE_NUM_MAX (VIRTIO_BASE + 0x034)
#define VIRTIO_QUEUE_NUM (VIRTIO_BASE + 0x038)
#define VIRTIO_QUEUE_PFN (VIRTIO_BASE + 0x040)
#define VIRTIO_QUEUE_NOTIFY (VIRTIO_BASE + 0x050)
#define VIRTIO_STATUS (VIRTIO_BASE + 0x070)

class Virtio : public Device {
private:
    uint64_t id;
    uint32_t driver_features;
    uint32_t page_size;
    uint32_t queue_sel;
    uint32_t queue_num;
    uint32_t queue_pfn;
    uint32_t queue_notify;
    uint32_t status;
    std::vector<uint8_t> disk;

public:
    Virtio(std::vector<uint8_t> disk_image);
    std::pair<uint64_t, std::optional<Exception>> load(uint64_t addr, int nBytes);
    std::optional<Exception> store(uint64_t addr, int nBytes, uint64_t value);
    bool is_interrupting();
    uint64_t get_new_id();
    uint64_t desc_addr();
    uint64_t read_disk(uint64_t addr);
    void write_disk(uint64_t addr, uint64_t value);
};

#endif
