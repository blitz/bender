#include <mbi.h>
#include <mbi2.h>
#include <serial.h>
#include <util.h>

__attribute__((used, aligned(8))) const struct mbi2_header mbi2_header = {
    .magic = MBI2_HEADER_MAGIC,
    .architecture = 0,
    .header_length = sizeof(struct mbi2_header),
    .checksum = -(MBI2_HEADER_MAGIC + sizeof(struct mbi2_header)),
};

int main(uint32_t magic, struct mbi2_boot_info *mbi) {
  serial_init();

  if (magic == MBI2_MAGIC) {
    /* Success */
  } else if (magic == MBI_MAGIC) {
    printf("Multiboot1 is not supported. Please boot via Multiboot2. Bye.\n");
    return 1;
  } else {
    printf("Not loaded by Multiboot-compliant loader. Bye.\n");
    return 1;
  }

  printf("Loaded with Multiboot2 loader.\n");

  for (struct mbi2_tag *tag = mbi2_tag_first(mbi); tag != NULL;
       tag = mbi2_tag_next(tag)) {
    printf("Tag %02d: ", tag->type);

    switch (tag->type) {
    case MBI2_TAG_CMDLINE:
      printf("boot cmdline='%s'\n", (const char *)mbi2_tag_payload(tag));
      break;
    case MBI2_TAG_MEMORY:
      printf("memory map\n");
      for (struct mbi2_memory_entry *mem_entry = mbi2_memory_map_first(tag);
           mem_entry != NULL;
           mem_entry = mbi2_memory_map_next(tag, mem_entry)) {
        printf(" type=%02d addr=%llx len=%llx\n", mem_entry->type,
               mem_entry->addr, mem_entry->len);
      }
      break;
    case MBI2_TAG_MODULE:
      printf("module cmdline='%s'\n",
             ((struct mbi2_module *)mbi2_tag_payload(tag))->string);
      break;
    default:
      printf("unknown\n", tag->type);
      break;
    }
  }

  return 0;
}
