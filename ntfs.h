#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef KIV_ZOS_NTFS_H
#define KIV_ZOS_NTFS_H

/**
 * Vytvori fyzicky soubor reprezentujici pseudo-filesystem
 *
 * @param filename nazev vytvoreneho souboru
 * @param cluster_count pocet clusteru
 * @param cluster_size velikost jednoho clusteru v B
 */
void create_file(char filename[], int32_t cluster_count, int32_t cluster_size);

/**
 * Na zaklade vstupniho parametru overi, zda soubor s danym jmenem existuje
 *
 * @param file_name nazev/cesta souboru k overeni
 * @return existence souboru (0 - neexistuje, 1 existuje)
 */
bool file_exists(const char *file_name);

#endif //KIV_ZOS_NTFS_H
