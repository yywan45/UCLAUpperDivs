#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>

//-------------------------------------------------------------------------//
//GLOBAL VARIABLES AND STRUCTS---------------------------------------------//
//-------------------------------------------------------------------------//

int disk_fd = 0;
int disk_size = 0;
int num_groups;
uint32_t block_size;

struct super_block {
  uint32_t s_inodes_count;
  uint32_t s_blocks_count;
  uint8_t  filler1[12];
  uint32_t s_first_data_block;
  uint32_t s_block_size;
  uint32_t s_frag_size;
  uint32_t s_blocks_per_group;
  uint32_t s_frags_per_group;
  uint32_t s_inodes_per_group;
  uint8_t  filler2[12];
  uint16_t s_magic_number;
  uint8_t  filler3[1024 - 58];
} superblock;

struct group_desc
{
  uint32_t g_block_bitmap;
  uint32_t g_inode_bitmap;
  uint32_t g_inode_table;
  uint16_t g_free_blocks_count;
  uint16_t g_free_inodes_count;
  uint16_t g_directories_count;
  uint8_t  filler[14];
};

struct group_desc* group_desc_table;

struct inode {
  uint16_t i_mode;
  uint16_t i_owner_id;
  uint32_t i_size;
  uint32_t i_access_time;
  uint32_t i_create_time;
  uint32_t i_modif_time;
  uint32_t i_delete_time;
  uint16_t i_group_id;
  uint16_t i_links_count;
  uint32_t i_num_blocks;
  uint32_t i_file_flags;
  uint32_t i_os_dependent1;
  uint32_t i_block_pointers[15];
  uint32_t i_file_version;
  uint32_t i_file_acl;
  uint32_t i_dir_acl;
  uint32_t i_frag_addr;
  uint16_t i_os_dependent2[6];
} inode;

struct directory {
  uint32_t inode;
  uint16_t entry_len;
  uint8_t  name_len;
  uint8_t  file_type;
  char     name[256];
} directory;

// Linked list of inode blocks
struct inode_block {
  int addr;
  struct inode_block *next;
  uint32_t containing_block;
};

//-------------------------------------------------------------------------//
//HELPER FUNCTIONS---------------------------------------------------------//
//-------------------------------------------------------------------------//

// Error-checking function for creat to use when creating csv files
int Creat(const char *path, mode_t mode, const char *file) {
  int error = creat(path, mode);
  if (error < 0) 
  {
    fprintf(stderr, "Unable to open output file %s.\n", file);
    exit(1);
  }
  return error;
}

// Append all blocks in indrect block to a data block linked list
// Type (1,2,3) gives single, double or triple block
// For writing to directory.csv
struct inode_block* read_inode_indirect_block(struct inode_block* tail, uint32_t block_num, int type) {
  uint32_t *block = malloc(block_size);
  pread(disk_fd, block, block_size, block_num * block_size);
  for (int i = 0; i < block_size / 4; i++) {
    if (block[i]) {
      if (type == 1) {
        struct inode_block* element = malloc(sizeof(struct inode_block));
        element->next = NULL;
        element->addr = block[i] * block_size;
        element->containing_block = block_num;
        tail->next = element;
        tail = element;
      }
      else
        tail = read_inode_indirect_block(tail, block[i], type-1);
    }
  }
  free(block);
  return tail;
}

// Write data in indirect block to file
// Type (1,2,3) gives single, double otr triple
// For writing to indirect.csv
void write_inode_indirect_block(int ofd, uint32_t block_num, int type) {
  if (type == 0) return;
  int i;
  uint32_t *block = malloc(block_size);
  pread(disk_fd, block, block_size, block_num * block_size);

  for (i = 0; i < block_size / 4; i++)
    if (block[i])
      dprintf(ofd, "%x,%u,%x\n", block_num, i, block[i]);
  for (i = 0; i < block_size / 4; i++)
      if (block[i])
        write_inode_indirect_block(ofd, block[i], type-1);
  free(block);
}

// Read the direct/indirect block
// Store the result into a linked list
struct inode_block* read_inode_blocks(int inode_num) {
  if (inode_num <= 0) return NULL;
  int i = inode_num - 1;
  int block_group = i / superblock.s_inodes_per_group;
  int block_group_offset = i % superblock.s_inodes_per_group;
  
  struct inode curr;
  uint8_t inode_bitmap;
  pread(disk_fd, &inode_bitmap, 1, group_desc_table[block_group].g_inode_bitmap * block_size + block_group_offset / 8);

  if (inode_bitmap & (1 << (block_group_offset % 8)))
    pread(disk_fd, &curr, sizeof(struct inode), group_desc_table[block_group].g_inode_table * block_size + sizeof(struct inode) * block_group_offset);

  struct inode_block *head = NULL;
  struct inode_block *tail = NULL;

  int j;
  for (j = 0; j < 12 && curr.i_block_pointers[j]; j++) {
    struct inode_block* element = malloc(sizeof(struct inode_block));
    element->next = NULL;
    element->addr = curr.i_block_pointers[j] * block_size;
    element->containing_block = -1;
    if (tail == NULL) head = element;
    else tail->next = element;
    tail = element;
  }
  
  if (j == 12)
    for (; j <= 14; j++)
      if (curr.i_block_pointers[j])
        tail = read_inode_indirect_block(tail, curr.i_block_pointers[j], j-12+1);
  return head;
}

//-------------------------------------------------------------------------//
//WRITING INTO CSV FILES---------------------------------------------------//
//-------------------------------------------------------------------------//

void write_super() {
  if (pread(disk_fd, &superblock, 1024, 1024) < 1024) {
    fprintf(stderr, "Error in reading superblock");
    exit(1);
  }
  
  block_size = 1024 << superblock.s_block_size;
  int32_t fragment_shift = (int32_t) superblock.s_frag_size;
  uint32_t fragment_size = 0;
  if (fragment_shift > 0)
    fragment_size = 1024 << fragment_shift;
  else
    fragment_size = 1024 >> -fragment_shift;

  // if (superblock.s_magic_number) {
  //   fprintf(stderr, "Superblock - invalid magic: %x", superblock.s_magic_number);
  //   exit(1);
  // }

  if (!((block_size != 0) && ((block_size & (~block_size + 1)) == block_size))
    || block_size < 512 || block_size > 64000) {
    fprintf(stderr, "Superblock - invalid block size: %d", block_size);
    exit(1);
  }

  if (superblock.s_blocks_count > disk_size) {
    fprintf(stderr, "Superblock - invalid block count %d > image size %d",
      superblock.s_blocks_count, disk_size);
    exit(1);
  }
  if (superblock.s_first_data_block > disk_size) {
    fprintf(stderr, "Superblock - invalid first block %d > image size %d",
      superblock.s_blocks_count, disk_size);
    exit(1);
  }

  if (superblock.s_blocks_count % superblock.s_blocks_per_group != 0) {
    fprintf(stderr, "Superblock - %d blocks, %d blocks/group",
      superblock.s_blocks_count, superblock.s_blocks_per_group);
    exit(1);
  }

  if (superblock.s_inodes_count % superblock.s_inodes_per_group != 0) {
    fprintf(stderr, "Superblock - %d inodes, %d inodes/group",
      superblock.s_inodes_count, superblock.s_inodes_per_group);
    exit(1);
  }

  int ofd = Creat("super.csv", 0666, "super.csv");
  dprintf(ofd, "%x,%d,%d,%d,%d,%d,%d,%d,%d\n", superblock.s_magic_number, superblock.s_inodes_count, superblock.s_blocks_count, 
    block_size, fragment_size, superblock.s_blocks_per_group, 
    superblock.s_inodes_per_group, superblock.s_frags_per_group, superblock.s_first_data_block);
}

void write_group() {
  int start = superblock.s_first_data_block + 1;
  num_groups = superblock.s_blocks_count / superblock.s_blocks_per_group;
  if (superblock.s_blocks_count % superblock.s_blocks_per_group)
    num_groups++;

  group_desc_table = malloc(sizeof(struct group_desc) * num_groups);
  int size = sizeof(struct group_desc) * num_groups;

  if (pread(disk_fd, group_desc_table, size, start * block_size) < size) {
    fprintf(stderr, "Error in reading group_desc_table");
    exit(1);
  }

  int ofd = Creat("group.csv", 0666, "group.csv");
  for (int i = 0; i < num_groups; i++) {
    dprintf(ofd, "%d,%d,%d,%d,%x,%x,%x\n", superblock.s_blocks_per_group, group_desc_table[i].g_free_blocks_count, group_desc_table[i].g_free_inodes_count,
      group_desc_table[i].g_directories_count, group_desc_table[i].g_inode_bitmap, group_desc_table[i].g_block_bitmap, 
      group_desc_table[i].g_inode_table);
  }
}

void write_bitmap() {
  int ofd = Creat("bitmap.csv", 0666, "bitmap.csv");
  char output_buffer[4096] = {0};
  int i, j, k, s;
  int parent_inode_num = 1, inode_max = 0, block_max = 0, block_idx = 1;
  
  uint8_t *i_bitmap_block = (uint8_t *) malloc(sizeof(uint8_t) * block_size);
  uint8_t *b_bitmap_block = (uint8_t *) malloc(sizeof(uint8_t) * block_size);

  for (i = 0; i < num_groups; i++) {
    block_max += superblock.s_blocks_per_group;
    inode_max += superblock.s_inodes_per_group;
    if (i == num_groups - 1) {
      block_max = superblock.s_blocks_count;
      inode_max = superblock.s_inodes_count;
    }

    pread(disk_fd, b_bitmap_block, block_size, group_desc_table[i].g_block_bitmap * block_size);
    for (j = 0; j < block_size; j++) {
      for (k = 0; k < 8; k++) {
        if (block_idx > block_max) break;
        if ((b_bitmap_block[j] & (1 << k)) == 0) {
          // dprintf(ofd, "%x,%d\n", group_desc_table[i].g_block_bitmap, block_idx);
          s = sprintf(output_buffer, "%x,%d\n", group_desc_table[i].g_block_bitmap, block_idx);
          write(ofd, output_buffer, s);
        }
        block_idx++;
      }
    }

    pread(disk_fd, i_bitmap_block, block_size, group_desc_table[i].g_inode_bitmap * block_size);
    for (j = 0; j < block_size; j++) {
      for (k = 0; k < 8; k++) {
        if (parent_inode_num > inode_max) break;
        if ((i_bitmap_block[j] & (1 << k)) == 0) {
          s = sprintf(output_buffer, "%x,%d\n", group_desc_table[i].g_inode_bitmap, parent_inode_num);
          write(ofd, output_buffer, s);
        }
        parent_inode_num++;
      }
    }
  }
  free(i_bitmap_block);
  free(b_bitmap_block);
}

void write_inode() {
  int ofd = Creat("inode.csv", 0666, "inode.csv");
  int i, j, k, m;
  int parent_inode_num = 1, inode_max = 0;
  
  uint8_t *i_bitmap_block = (uint8_t *) malloc(sizeof(uint8_t) * block_size);

  for (i = 0; i < num_groups; i++) {
    inode_max += superblock.s_inodes_per_group;
    if (i == num_groups - 1) {
      inode_max = superblock.s_inodes_count;
    } 

    pread(disk_fd, i_bitmap_block, block_size, group_desc_table[i].g_inode_bitmap * block_size);
    for (j = 0; j < block_size; j++) {
      for (k = 0; k < 8; k++) {
        if (parent_inode_num > inode_max) break;
        if ((i_bitmap_block[j] & (1 << k)) != 0) {
          pread(disk_fd, &inode, sizeof(struct inode), group_desc_table[i].g_inode_table * block_size + sizeof(struct inode) * (j * 8 + k));
          char file_type = '?';
          if (inode.i_mode & 0x8000)
            file_type = 'f';
          else if (inode.i_mode & 0x4000)
            file_type = 'd';
          else if (inode.i_mode & 0xA000)
            file_type = 's';

          uint32_t owner_id = (inode.i_os_dependent2[2] << 16) + inode.i_owner_id;
          uint32_t group_id = (inode.i_os_dependent2[3] << 16) + inode.i_group_id;
          uint32_t i_num_blocks = inode.i_num_blocks / (2 << superblock.s_block_size);

          dprintf(ofd, "%u,%c,%o,%u,%u,%u,%x,%x,%x,%u,%u", 
            parent_inode_num, file_type, inode.i_mode, owner_id, group_id, 
            inode.i_links_count, inode.i_create_time, inode.i_modif_time, inode.i_access_time,
            inode.i_size, i_num_blocks);

          for (m = 0; m < 15; m++)
            dprintf(ofd, ",%x", inode.i_block_pointers[m]);
          dprintf(ofd, "\n");
        }
        parent_inode_num++;
      }
    }
  }
  free(i_bitmap_block);
}

void write_directory() {
  int ofd = Creat("directory.csv", 0666, "directory.csv");
  int i, j, k;
  int parent_inode_num = 1, inode_max = 0;
  uint8_t *i_bitmap_block = (uint8_t *) malloc(sizeof(uint8_t) * block_size);

  for (i = 0; i < num_groups; i++) {
    inode_max += superblock.s_inodes_per_group;
    if (i == num_groups - 1)
      inode_max = superblock.s_inodes_count;
    
    pread(disk_fd, i_bitmap_block, block_size, group_desc_table[i].g_inode_bitmap * block_size);
    for (j = 0; j < block_size; j++) {
      for (k = 0; k < 8; k++) {
        if (parent_inode_num > inode_max) break;
        if (i_bitmap_block[j] & (1 << k)) {
          pread(disk_fd, &inode, sizeof(struct inode), group_desc_table[i].g_inode_table * block_size + sizeof(struct inode) * (j * 8 + k));
          if (inode.i_mode & 0x4000) {
            struct inode_block *head = read_inode_blocks(parent_inode_num);
            struct inode_block *temp = head;
            uint32_t entry_num = 0;

            while (temp != NULL) {
              int pos = temp->addr;
              memset(directory.name, 0, 256);
              pread(disk_fd, &directory, 8, pos);
              pread(disk_fd, &(directory.name), directory.name_len, pos + 8);
              if (directory.inode != 0) {
                // if (directory.entry_len < 8 || directory.entry_len > 1024) {
                //   fprintf(stderr, "Inode %u, block %u - bad directory: len = %u",
                //     directory.inode, block_num, directory.entry_len);
                //   exit(1);
                // }

                // if (directory.name_len > directory.entry_len) {
                //   fprintf(stderr, "Inode %u, block %u - bad directory: len = %u, namelen = %u",
                //     directory.inode, block_num, directory.entry_len, directory.name_len);
                //   exit(1);
                // }

                // if (directory.inode > s_inodes_count) {
                //   fprintf(stderr, "Inode %u, block %u - bad directory: file entry inode number = %u, number of inodes = %u",
                //     directory.inode, block_num, directory.inode, s_inodes_count);
                //   exit(1);
                // }
                dprintf(ofd, "%u,%u,%u,%u,%u,\"%s\"\n", parent_inode_num, entry_num, directory.entry_len, directory.name_len, directory.inode, directory.name);
              }
              entry_num++;

              while (directory.entry_len + pos < temp->addr + block_size) {
                pos += directory.entry_len;
                memset(directory.name, 0, 256);
                pread(disk_fd, &directory, 8, pos);
                pread(disk_fd, &(directory.name), directory.name_len, pos + 8);

                if (directory.inode != 0)
                  dprintf(ofd, "%u,%u,%u,%u,%u,\"%s\"\n", parent_inode_num, entry_num, directory.entry_len, directory.name_len, directory.inode, directory.name);
                entry_num++;
              }
              temp = temp->next;
            }
          }
        }
        parent_inode_num++;
      }
    }
  }
  free(i_bitmap_block);
}

void write_indirect() {
  int ofd = Creat("indirect.csv", 0666, "indirect.csv");
  int i, j, k, x, parent_inode_num = 1, inode_max = 0;
  uint8_t *i_bitmap_block = (uint8_t *) malloc(sizeof(uint8_t) * block_size);

  for (i = 0; i < num_groups; i++) {
    inode_max += superblock.s_inodes_per_group;
    if (i == num_groups - 1)
      inode_max = superblock.s_inodes_count;
    
    pread(disk_fd, i_bitmap_block, block_size, group_desc_table[i].g_inode_bitmap * block_size);
    for (j = 0; j < block_size; j++) {
      for (k = 0; k < 8; k++) {
        if (parent_inode_num > inode_max) break;
        if (i_bitmap_block[j] & (1 << k)) {
          pread(disk_fd, &inode, sizeof(struct inode), group_desc_table[i].g_inode_table * block_size + sizeof(struct inode) * (j * 8 + k));
          uint32_t i_num_blocks = inode.i_num_blocks / (2 << superblock.s_block_size);
          if (i_num_blocks > 10)
            for (x = 12; x <= 14; x++)
              if (inode.i_block_pointers[x])
                write_inode_indirect_block(ofd, inode.i_block_pointers[x], x-12+1);
        }
        parent_inode_num++;
      }
    }
  }
  free(i_bitmap_block);
}

//-------------------------------------------------------------------------//
//MAIN ROUTINE-------------------------------------------------------------//
//-------------------------------------------------------------------------//

int main(int argc, char **argv) {
  if (argc > 2) {
    fprintf(stderr, "Too many arguments");
    exit(1);
  }

  disk_fd = open(argv[1], O_RDONLY);
  if (disk_fd < 0) {
    fprintf(stderr, "Error opening disk image");
    exit(1);
  }

  struct stat st;
  stat(argv[1], &st);
  disk_size = st.st_size;
  // printf("File size: %d bytes\n", disk_size);
  
  write_super();
  write_group();
  write_bitmap();
  write_inode();
  write_directory();
  write_indirect();

  return 0;
}