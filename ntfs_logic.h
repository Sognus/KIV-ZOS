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

/**
 *  * Na zaklade aktualniho kontextu shell, najde PRVNI PRAZDNOU adresu a poradi daneho clusteru
 *
 * @param shell kontext funkce
 * @param cluster_addr ukazatel na output int jaka je adresa clusteru volneho
 * @param cluster_order ukazatel na output int kolikaty je volny cluster v bitmape
 * @return chybova hlaska, 0 = bez chyby, <-max, 0) = error, (0, max) = notice
 */
int32_t find_empty_cluster(shell *shell, int32_t *cluster_addr, int *cluster_order);

/**
 * Projde MFT tabulku, najde dalsi volne uid
 *
 * Nejprve inkrementuje nejvyssi nalezene cislo o 1, v pripade, ze je dosahnuto maximalniho
 * cisla 32bit, pak prochazi cisla od 1 do max cisla
 *
 * @param shell
 * @return
 */
int32_t get_next_uid(shell *shell);

/**
 * Porovna dve struktury na zaklade UID pro Qsort
 *
 * @param s1 struktura 1
 * @param s2 struktura 2
 * @return porovnani
 */
int compare_mft_items(const void *s1, const void *s2);

/**
 * Nalezne index volneho mft itemu, pokud neexistuje, hodi -2
 *
 * @param shell
 * @return index volneho mft itemu nebo chyba
 */
int get_free_mft_item_index(shell *shell);

/**
 * Prida fyzicky zaznam o souboru/slozce/symlinku do nadrazene slozky
 *
 * @param shell kontext
 * @param parent_uid uid nadrazene slozky
 * @param add_uid pridavane uid
 * @return chyba nebo uspech ( = 0 )
 *
 */
int parrent_add_uid(shell *shell, int32_t parent_uid, int32_t add_uid);

/**
 * Vrati pocet alokovanych clusteru k jednomu mft itemu
 *
 * @param shell
 * @param mft_item
 * @return
 */
int get_allocated_cluster_count(mft_item *mft_item);

/**
 *  Vrati adresu dalsiho prvku kam zapsat UID
 *
 * @param shell
 * @param uid
 */
int get_folder_next_member_adress(shell *shell, int32_t uid);

#endif //KIV_ZOS_NTFS_LOGIC_H
