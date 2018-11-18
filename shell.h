
#ifndef KIV_ZOS_SHELL_H
#define KIV_ZOS_SHELL_H

#include "structure.h"

typedef struct shell
{
    boot_record *boot;      // Boot record se kterym se pracuje
    char *filename;         // Fyzicky soubor se strukturou ntfs se kterym se pracuje
    int32_t cwd;            // UID aktualni slozky, se kterou se pracuje
    mft_item *mft_array;    // Nactene MFT
    int mft_array_size;     // Pocet polozek MFT

} shell;

/**
 * Vytvori novy shell v libovolne slozce (na zaklade UID)
 *
 * @param record boot record kontext NTFS
 * @param ntfs_filename fyzicky soubor ntfs
 * @param directory uid slozky (1 pro root)
 * @return
 */
shell *create_shell(boot_record *record, char *ntfs_filename, int32_t directory);

/**
 * Vytvori novy shell v rootu ntfs
 *
 * @param record
 * @param ntfs_filename
 * @return
 */
shell *create_root_shell(boot_record *record, char *ntfs_filename);

/**
 *  Provede parodii na vypsani struktury shell
 *
 * @param shell ukazatel na shell
 */
void print_shell(shell *shell);

#endif //KIV_ZOS_SHELL_H
