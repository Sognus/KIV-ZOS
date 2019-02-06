#include "shell_app.h"

int shell_app_main(char *filename)
{
    // Overeni vstupu
    if(filename == NULL || strlen(filename) < 1)
    {
        printf("Invalid input filename! Exiting");
        return -1;
    }

    // Existence souboru, neexistuje-li, bude zalozen
    if(!file_exists(filename))
    {
        format_file(filename, 128000);
    }

    boot_record *boot = read_boot_record(filename);
    shell *shell = create_shell(boot, filename, NTFS_ROOT_UID);

    // Hlavni loop aplikace - nacitani prikazu
    while(true)
    {
        char *line = NULL;  /* forces getline to allocate with malloc */
        size_t len = 0;
        ssize_t read;

        // Vypis aktualni slozky
        printf("%s$ ", get_current_path(shell));
        read = getline (&line, &len, stdin);
        line[read-1] = '\0';

        // Testovani ukonceni shellu
        if(strcmp("exit", line) == 0) {
            printf("Shell exit\n");
            break;
        }

        // Podprogram pro prikazy
        commands(shell, line);
    }

    return 0;
}

// Parser prikazu
int commands(shell *shell, char *command)
{
    // command: ls
    if(strcmp("ls", command) == 0)
    {
        return command_ls(shell);
    }

    if(strcmp("pwd", command) == 0)
    {
        return command_pwd(shell);
    }

    if(starts_with("cd", command))
    {
        return command_cd(shell, command);
    }

    if(starts_with("mkdir", command))
    {
        return command_mkdir(shell, command);
    }

    return 1;
}

// Prikaz: ls
int command_ls(shell *shell)
{
    int *uids = NULL;
    int uid_count = -1;


    get_folder_members(shell, shell->cwd, &uids, &uid_count);

    for(int i = 0; i < uid_count; i++)
    {
        // TODO: symlink vypis
        mft_item *item = find_mft_item_by_uid(shell, uids[i]);
        char *mark = item->isDirectory == 0 ? "-" : "+";
        char *name = i < 2 ? (i == 0 ? "." : "..") : item->item_name;
        printf("%s%s\n", mark, name);
    }


    return 0;
}

int command_cd(shell *shell, char *command)
{
    // presun na druhou cast retezce po mezere
    char delim[] = " ";
    char *ptr = strtok(command, delim);
    ptr = strtok(NULL, delim);

    int target_dir = path_target_uid(shell, ptr);

    // Nastavit
    if(target_dir > 0)
    {
        printf("OK\n");
        shell->cwd = target_dir;

    }
    else
    {
        printf("PATH NOT FOUND (neexistující cesta)\n");
    }

    printf("path: %s -> %d\n", ptr, target_dir);

    return 0;
}

int command_pwd(shell *shell)
{
    printf("%s\n", get_current_path(shell));
    return 0;
}

int command_mkdir(shell *shell, char *command)
{
    // presun na druhou cast retezce po mezere
    char delim[] = " ";
    char *ptr = strtok(command, delim);
    ptr = strtok(NULL, delim);

    char *full_path = malloc(sizeof(char) * strlen(ptr));
    strcpy(full_path, ptr);

    // Posledni token
    char * token, * last;
    last = token = strtok(ptr, "/");
    for (;(token = strtok(NULL, "/")) != NULL; last = token);

    // Cesta bez koncove slozky
    char *path = malloc(sizeof(char) * strlen(full_path));
    strcpy(path, full_path);
    path[strlen(full_path) - strlen(last)] = '\0';


    //printf("FULL: %s\n", full_path);
    //printf("LAST: %s\n", last);
    //printf("CESTA: %s\n", path);

    // Ulozeni aktualni slozky
    int current_uid = shell->cwd;

    if(path_exist(shell,path))
    {
        // Zmena na cilovou slozku
        shell->cwd = path_target_uid(shell, path);
        // Zjisteni zda slozka s danym jmenem existuje
        int *uids = NULL;
        int uid_count = -1;
        get_folder_members(shell, shell->cwd, &uids, &uid_count);
        int found = 0;
        for(int i = 0; i < uid_count; i++)
        {
            mft_item *item = find_mft_item_by_uid(shell, uids[i]);
            if(strcmp(item->item_name, last) == 0)
            {
                found = 1;
                break;
            }
        }

        // Pokud nebylo jmeno nalezeno, vytvor ho
        if(found == 1)
        {
            printf("EXIST (nelze založit, již existuje)\n");
        }
        else
        {
            printf("OK\n");
            create_folder(shell, last);
        }

    }
    else
    {
        printf("PATH NOT FOUND (neexistuje zadaná cesta)\n");
    }

    // Navrat na aktualni slozku
    shell->cwd = current_uid;

    //create_folder(shell, ptr);

    return 0;
}