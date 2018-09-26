//
// Created by Sognus on 26.09.2018.
//

#ifndef KIV_ZOS_MFT_ITEM_H
#define KIV_ZOS_MFT_ITEM_H

#include "mft_fragment.h"
#include "bool.h"

typedef struct mft_item {
    int32_t uid;                                        //UID polozky, pokud UID = UID_ITEM_FREE, je polozka volna
    struct bool isDirectory;                                   //soubor, nebo adresar
    int8_t item_order;                                  //poradi v MFT pri vice souborech, jinak 1
    int8_t item_order_total;                            //celkovy pocet polozek v MFT
    char item_name[12];                                 //8+3 + /0 C/C++ ukoncovaci string znak
    int32_t item_size;                                  //velikost souboru v bytech
    mft_fragment fragments[]; //fragmenty souboru
} mft_item;

#endif //KIV_ZOS_MFT_ITEM_H
