#include "big_brother.h"
#include "fat_volume.h"
#include "fat_table.h"
#include "fat_util.h"
#include <stdio.h>
#include <string.h>


char *censored [] = {
    "communists", "Proletarians", "ChAins", "PERSIANAMERICANA", "asdsadad", NULL, 
};

int bb_is_log_file_dentry(fat_dir_entry dir_entry) {
    return strncmp(LOG_FILE_BASENAME, (char *)(dir_entry->base_name), 3) == 0 &&
           strncmp(LOG_FILE_EXTENSION, (char *)(dir_entry->extension), 3) == 0;
}

int bb_is_log_filepath(char *filepath) {
    return strncmp(BB_LOG_FILE, filepath, 8) == 0;
}

int bb_is_log_dirpath(char *filepath) {
    return strncmp(BB_DIRNAME, filepath, 15) == 0;
}

/* Searches for a cluster that could correspond to the bb directory and returns
 * its index. If the cluster is not found, returns 0.
 */
u32 search_bb_orphan_dir_cluster() {
    u32 bb_dir_start_cluster = 2;
    fat_volume vol = get_fat_volume();
    u32 max_clusters = 10000;


    while ( bb_dir_start_cluster < max_clusters){ 
        if (fat_table_cluster_is_bad_sector(le32_to_cpu(((const le32 *)vol->table->fat_map)[bb_dir_start_cluster]))) {  //chequeo si esta bad
            DEBUG("TENGO BAD SECTOR");
            DEBUG("BAD SECTOR %u", bb_dir_start_cluster);
            
            // Se castean los contenidos del cluster encontrado como una dir_entry
            u32 bytes_per_cluster = fat_table_bytes_per_cluster(vol->table);
            off_t offset = fat_table_cluster_offset(vol->table, bb_dir_start_cluster);
            u8 *buf = alloca(bytes_per_cluster);     
            full_pread(vol->table->fd, buf, bytes_per_cluster, offset);       
            fat_dir_entry dentry = (fat_dir_entry) buf;

            if(bb_is_log_file_dentry(dentry)) {
                DEBUG("TIENE NOMBRECITO EN CLUSTER %u", bb_dir_start_cluster);
                DEBUG("NOMBRE: %s",dentry->base_name );
                break;
            }   
        }
        bb_dir_start_cluster++;      
    }

    // Si se iteró de más, se busca un cluster libre
    if (bb_dir_start_cluster >= max_clusters) {
        DEBUG("SE FUE LEJOS EL BB");
        bb_dir_start_cluster = 0;
    }
    return bb_dir_start_cluster;
}


int bb_init_log_dir(u32 start_cluster) {
    errno = 0;
    fat_volume vol = NULL;
    fat_tree_node root_node = NULL;
    int isInit = 1;

    vol = get_fat_volume();

    // Caso donde no se encontró el cluster del directorio
    if(start_cluster == 0) {
        start_cluster = fat_table_get_next_free_cluster(vol->table);
        DEBUG("%u TOMO ESTE CLUSTER LIBRE", start_cluster);
        isInit = 0;
    }
    // Create a new file from scratch, instead of using a direntry like normal
    fat_file loaded_bb_dir =
        fat_file_init_orphan_dir(BB_DIRNAME, vol->table, start_cluster);
    fat_table_set_next_cluster(vol->table, start_cluster,
                               FAT_CLUSTER_BAD_SECTOR);

    // Add directory to file tree. It's entries will be like any other dir.
    root_node = fat_tree_node_search(vol->file_tree, "/");
    vol->file_tree = fat_tree_insert(vol->file_tree, root_node, loaded_bb_dir);
    if(isInit == 1) {
        root_node = fat_tree_node_search(vol->file_tree,BB_DIRNAME);
        GList *children = fat_file_read_children(loaded_bb_dir);
        fat_tree_insert(vol->file_tree,root_node,g_list_nth_data(children,0));
    }

    return -errno;
}


GSList *words_searcher(const char *buf) {
    GSList *words_found = NULL;
    for (unsigned int i = 0; censored[i] != NULL; i++) {
        //busca la palabra ignorando los cases 
        bool res = strcasestr(buf, censored[i]);
        if (res) {
            words_found = g_slist_prepend(words_found, censored[i]);
        }
    }
    return words_found;
}