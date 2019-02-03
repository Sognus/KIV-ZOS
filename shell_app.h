#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "shell.h"
#include "ntfs.h"
#include "path_logic.h"
#include "usefull_functions.h"

#ifndef KIV_ZOS_SHELL_APP_H
#define KIV_ZOS_SHELL_APP_H

/**
 * Hlavni procedura virtualizace shellu
 *
 * @param filename
 * @return
 */
int shell_app_main(char *filename);

// Prikaz: ls
// TODO: symlinky
int command_ls(shell *shell);

// Parser prikazu
int commands(shell *shell, char *command);

// Prikaz: cd <cesta>
// TODO: relativni cesty
int command_cd(shell *shell, char *command);

// Prikaz: pwd
int command_pwd(shell *shell);

// Prikaz: mkdir
int command_mkdir(shell *shell, char *command);

#endif //KIV_ZOS_SHELL_APP_H
