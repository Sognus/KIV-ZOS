#ifndef KIV_ZOS_PATH_LOGIC_H
#define KIV_ZOS_PATH_LOGIC_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "path_logic.h"
#include "shell.h"
#include "ntfs_logic.h"

// Oddelovac slozek
#define PATH_SEPARATOR "/"
// Velikost oddelovace slozek (pri pripadnem pouziti viceznakoveho oddelovace)
#define PATH_SEPARATOR_LENGTH 1

// Maximalni pocet casti v ceste
#define PATH_PARTS_LIMITATION 256

/**
 * Na zaklade aktualniho kontextu shell vytvori retezec
 * obsahujici textovou reprezentaci absolutni cesty pro
 * current working directory
 *
 * @param shell aktualni kontext
 * @return ukazatel na retezec
 */
char *get_current_path(shell *shell);


#endif //KIV_ZOS_PATH_LOGIC_H
