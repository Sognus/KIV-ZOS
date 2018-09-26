//
// Created Jakub Vítek on 26.09.2018.
//

#ifndef KIV_ZOS_MFT_FRAGMENT_H
#define KIV_ZOS_MFT_FRAGMENT_H

typedef struct mft_fragment {
    int32_t fragment_start_address;     //start adresa
    int32_t fragment_count;             //pocet clusteru ve fragmentu
} mft_fragment;

#endif //KIV_ZOS_MFT_FRAGMENT_H
