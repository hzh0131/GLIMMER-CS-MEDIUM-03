#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

ino_t get_inode(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) == -1) {
        perror("stat failed");
        exit(EXIT_FAILURE);
    }
    return statbuf.st_ino;
}


/*
 * 在父目录中寻找当前目录的名字
 */
char *find_name(ino_t current_inode)
{
    DIR *dir;
    struct dirent *entry;

    dir = opendir("..");

    if (dir == NULL) {
        perror("opendir failed");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) {

        /*
         * entry->d_ino 就是该目录项对应的 Inode
         */
        if (entry->d_ino == current_inode) {

            char *name = malloc(strlen(entry->d_name) + 1);

            if (name == NULL) {
                perror("malloc failed");
                closedir(dir);
                exit(EXIT_FAILURE);
            }

            strcpy(name, entry->d_name);

            closedir(dir);

            return name;
        }
    }

    closedir(dir);

    return NULL;
}


int main(void)
{
    char **names = NULL;
    int count = 0;

    while (1) {

        /*
         * 当前目录的 Inode
         */
        ino_t current_inode = get_inode(".");

        /*
         * 父目录的 Inode
         */
        ino_t parent_inode = get_inode("..");

        /*
         * 如果两者相同，说明已经到达根目录
         */
        if (current_inode == parent_inode) {
            break;
        }

        /*
         * 去父目录中寻找：
         * 哪个目录项指向 current_inode？
         */
        char *name = find_name(current_inode);

        if (name == NULL) {
            fprintf(stderr, "cannot find directory name\n");
            exit(EXIT_FAILURE);
        }

        /*
         * 保存这个目录名
         */
        names = realloc(names, (count + 1) * sizeof(char *));

        if (names == NULL) {
            perror("realloc failed");
            exit(EXIT_FAILURE);
        }

        names[count] = name;
        count++;

        /*
         * 进入父目录
         */
        if (chdir("..") == -1) {
            perror("chdir failed");
            exit(EXIT_FAILURE);
        }
    }

    /*
     * 逆序输出
     *
     * 假设原路径：
     * /home/user/test
     *
     * 我们逆推得到：
     * test
     * user
     * home
     *
     * 所以需要倒过来输出。
     */
    printf("/");

    for (int i = count - 1; i >= 0; i--) {

        printf("%s", names[i]);

        if (i != 0)
            printf("/");

        free(names[i]);
    }

    printf("\n");

    free(names);

    return 0;
}
