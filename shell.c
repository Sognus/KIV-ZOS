#include "shell.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ntfs.h"


/**
 * Vytvori novy shell v libovolne slozce (na zaklade UID)
 *
 * @param record boot record kontext NTFS
 * @param ntfs_filename fyzicky soubor ntfs
 * @param directory uid slozky (1 pro root)
 * @return
 */
shell *create_shell(boot_record *record, char *ntfs_filename, int32_t directory)
{
    shell *sh = malloc(sizeof(shell));

    // TODO: is UID directory

    sh->boot = record;
    sh->cwd = directory;
    sh->filename = malloc(strlen(ntfs_filename)+1);
    strcpy(sh->filename, ntfs_filename);
    read_mft_items(ntfs_filename, record, &sh->mft_array ,&sh->mft_array_size);

    return sh;
}

/**
 *  Provede parodii na vypsani struktury shell
 *
 * @param shell ukazatel na shell
 */
void print_shell(shell *shell)
{
    if(shell == NULL)
    {
        printf("Shell je NULL!\n");
        return;
    }

    printf("SHELL BOOT RECORD NULL: %d\n", (shell->boot == NULL));
    printf("SHELL CURRENT WORKING DIR: %d\n", shell->cwd);
    printf("SHELL FILENAME: %s\n", shell->filename);
    printf("SHELL MFT ITEM ARRAY NULL: %d\n", (shell->mft_array == NULL));
    printf("SHELL MFT ITEM COUNT : %d\n", shell->mft_array_size);
}

/**
 * Vytvori novy shell v rootu ntfs
 *
 * @param record
 * @param ntfs_filename
 * @return
 */
shell *create_root_shell(boot_record *record, char *ntfs_filename)
{
    // 1 = UID rootu
    return  create_shell(record, ntfs_filename, 1);
}
