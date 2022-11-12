#include "big_brother.h"
#include "fat_volume.h"
#include "fat_table.h"
#include "fat_util.h"
#include <stdio.h>
#include <string.h>

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
    u32 bb_dir_start_cluster = 2; //2 no 0, ver next_free
    fat_volume vol = get_fat_volume();
    u32 max_clusters = vol->table->num_data_clusters; //funciona para no iterar de mas

    while ( bb_dir_start_cluster < max_clusters){ //faltaria checkear si es el dir correcto
        if (le32_to_cpu(((const le32 *)vol->table->fat_map)[bb_dir_start_cluster]) != FAT_CLUSTER_BAD_SECTOR) {
            u32 bytes_per_cluster = fat_table_bytes_per_cluster(vol->table);
            off_t offset = fat_table_cluster_offset(vol->table, bb_dir_start_cluster);
            u8 *buf = alloca(bytes_per_cluster);
            full_pread(vol->table->fd, buf, bytes_per_cluster, offset);

            if(bb_is_log_file_dentry((fat_dir_entry) buf)) {
                break;
            }   
        }
        bb_dir_start_cluster++;
    }
    if (bb_dir_start_cluster >= max_clusters) {
        bb_dir_start_cluster = 0;
    }

    return bb_dir_start_cluster;
}


/* Creates the /bb directory as an orphan and adds it to the file tree as 
 * child of root dir.
 */
// static int bb_create_new_orphan_dir() {
//     errno = 0;
//     // ****MOST IMPORTANT PART, DO NOT SAVE DIR ENTRY TO PARENT ****

//     return -errno;
// }


int bb_init_log_dir(u32 start_cluster) {
    errno = 0;
    fat_volume vol = NULL;
    fat_tree_node root_node = NULL;



    vol = get_fat_volume();
    start_cluster = fat_table_get_next_free_cluster(vol->table);
    // Create a new file from scratch, instead of using a direntry like normally done.
    fat_file loaded_bb_dir = fat_file_init_orphan_dir(BB_DIRNAME, vol->table, start_cluster);
    start_cluster = FAT_CLUSTER_BAD_SECTOR;

    // Add directory to file tree. It's entries will be like any other dir.
    root_node = fat_tree_node_search(vol->file_tree, "/");
    vol->file_tree = fat_tree_insert(vol->file_tree, root_node, loaded_bb_dir);

    return -errno;
}
