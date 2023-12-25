#include "virtio.h"

#include <iostream>

Virtio::Virtio(std::vector<uint8_t> disk_image) : id{0},
                                                  driver_features{0},
                                                  page_size{0},
                                                  queue_sel{0},
                                                  queue_num{0},
                                                  queue_pfn{0},
                                                  queue_notify{UINT32_MAX},
                                                  status{0},
                                                  disk{disk_image} {
}

std::pair<uint64_t, std::optional<Exception>> Virtio::load(uint64_t addr, int nBytes) {
    if (nBytes == 4) {
        switch (addr) {
        case VIRTIO_MAGIC:
            return std::make_pair(0x74726976, std::nullopt);
        case VIRTIO_VERSION:
            return std::make_pair(0x1, std::nullopt);
        case VIRTIO_DEVICE_ID:
            return std::make_pair(0x2, std::nullopt);
        case VIRTIO_VENDOR_ID:
            return std::make_pair(0x554d4551, std::nullopt);
        case VIRTIO_DEVICE_FEATURES:
            return std::make_pair(0, std::nullopt);
        case VIRTIO_DRIVER_FEATURES:
            return std::make_pair(driver_features, std::nullopt);
        case VIRTIO_QUEUE_NUM_MAX:
            return std::make_pair(8, std::nullopt);
        case VIRTIO_QUEUE_PFN:
            return std::make_pair(queue_pfn, std::nullopt);
        case VIRTIO_STATUS:
            return std::make_pair(status, std::nullopt);
        default:
            return std::make_pair(0, std::nullopt);
            ;
        }
    }
    return std::make_pair(0, Exception(ExceptionType::LoadAccessFault));
}

std::optional<Exception> Virtio::store(uint64_t addr, int nBytes, uint64_t value) {
    if (nBytes == 4) {
        switch (addr) {
        case VIRTIO_DEVICE_FEATURES:
            driver_features = value;
            return std::nullopt;
        case VIRTIO_GUEST_PAGE_SIZE:
            page_size = value;
            return std::nullopt;
        case VIRTIO_QUEUE_SEL:
            queue_sel = value;
            return std::nullopt;
        case VIRTIO_QUEUE_NUM:
            queue_num = value;
            return std::nullopt;
        case VIRTIO_QUEUE_PFN:
            queue_pfn = value;
            return std::nullopt;
        case VIRTIO_QUEUE_NOTIFY:
            queue_notify = value;
            return std::nullopt;
        case VIRTIO_STATUS:
            status = value;
            return std::nullopt;
        default:
            return std::nullopt;
        }
    }
    return Exception(ExceptionType::StoreAMOAccessFault);
}

bool Virtio::is_interrupting() {
    if (queue_notify != UINT32_MAX) {
        queue_notify = UINT32_MAX;
        return true;
    }
    return false;
}

uint64_t Virtio::get_new_id() {
    id += 1;
    return id;
}

uint64_t Virtio::desc_addr() {
    return (uint64_t)queue_pfn * (uint64_t)page_size;
}

uint64_t Virtio::read_disk(uint64_t addr) {
    return disk[addr];
}

void Virtio::write_disk(uint64_t addr, uint64_t value) {
    disk[addr] = value;
}
