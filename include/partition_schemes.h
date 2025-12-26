#pragma once

#include <cstddef>
#include <cstdint>

struct PartitionBlob {
    const uint8_t *data;
    size_t size;
};

#if defined(PART_08MB)
extern const uint8_t def_part[];
extern const size_t def_part_size;
extern const uint8_t doom[];
extern const size_t doom_size;
extern const uint8_t uiflow2[];
extern const size_t uiflow2_size;
extern const uint8_t gamestation[];
extern const size_t gamestation_size;
#elif defined(PART_16MB)
extern const uint8_t def_part[];
extern const size_t def_part_size;
extern const uint8_t uiFlow1[];
extern const size_t uiFlow1_size;
#endif

#if defined(HEADLESS)
extern const uint8_t def_part_headless[];
extern const size_t def_part_headless_size;
extern const uint8_t def_part8_headless[];
extern const size_t def_part8_headless_size;
extern const uint8_t def_part16_headless[];
extern const size_t def_part16_headless_size;
#endif
