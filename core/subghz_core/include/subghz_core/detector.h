#ifndef SUBGHZ_CORE_DETECTOR_H
#define SUBGHZ_CORE_DETECTOR_H

#include "subghz_core/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct subghz_detector subghz_detector_t;

subghz_detector_t* subghz_detector_create(void);
void subghz_detector_destroy(subghz_detector_t* detector);

bool subghz_detector_identify(
    subghz_detector_t* detector,
    const subghz_input_t* input,
    subghz_identification_t* out_ident);

#ifdef __cplusplus
}
#endif

#endif
