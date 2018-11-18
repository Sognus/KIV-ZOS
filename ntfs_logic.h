#include "shell.h"

#ifndef KIV_ZOS_NTFS_LOGIC_H
#define KIV_ZOS_NTFS_LOGIC_H

/**
 * Vrati jestli se jedna o slozku
 *
 * @param shell kontext
 * @param uid identifikator mft itemu
 * @return 0 = ne, 1 = ano, -1 = chyba
 */
int is_folder(shell *shell, int32_t uid);

/**
 * Vrati jestli se jedna o soubor
 *
 * @param shell kontext
 * @param uid identifikator mft itemu
 * @return 0 = ne, 1 = ano, -1 = chyba
 */
int is_file(shell *shell, int32_t uid);

/**
 * Vrati zda se jedna o symlink
 *
 * @param shell kontext
 * @param uid identifikator mft itemu
 * @return 0 = ne, 1 = ano, -1 = chyba
 */
int is_symlink(shell *shell, int32_t uid);

/**
 * Vrati mft item na zaklade uid
 *
 * @param shell kontext
 * @param uid identifikator mft itemu
 * @return null když neexistuje, struct když existuje
 */
mft_item *find_mft_item_by_uid(shell *shell, int32_t uid);


/**
 * Vrati 1 pokud ma current working directory soubor/slozku/symlink s danym jmenem
 * Vrati 0 pokud ^ ne
 *
 * Vrati -1 pokud nastane chyba
 *
 * @return viz popis
 */
int cwd_has_item_name(shell *shell, char item_name[13]);

/**
 * Vytvori novou slozku ve FS na zaklade current working directory
 *
 * @param shell
 * @param folder_name
 * @return
 */
int create_folder(shell *shell, char folder_name[12]);

/**
 * Vrati identifikatory souboru/slozek/symlinku ktere jsou ve slozce current directory
 *
 * @param shell kontext
 * @param uid uid slozky
 */
int get_folder_members_count(shell *shell, int32_t uid);

/**
 * Upraví ukazetele out proměnný tak, aby jejich obsah bylo pole vsech uid v dane slozce
 * out_count pak predstavuje pocet polozek v danem poli
 *
 * @param shell kontext
 * @param uid uid prohledavane slozky
 * @param out_array ukazatel na ukazatel pole kam se budou ukladat polozky pole
 * @param out_count
 */
void get_folder_members(shell *shell, int32_t uid, int **out_array, int *out_count);

#endif //KIV_ZOS_NTFS_LOGIC_H
