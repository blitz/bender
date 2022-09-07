configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.c.in ${CMAKE_CURRENT_BINARY_DIR}/version.c)
set(version_file "${CMAKE_CURRENT_BINARY_DIR}/version.c")

add_compile_options(
  -minline-all-stringops -m32 -march=pentium -mtune=core2
  -ffreestanding -nostdlib -Wno-multichar -Wall
  -ffunction-sections -fomit-frame-pointer -mregparm=3
  )

add_link_options(
  -m32 -static -nostdlib -Wl,-T,${CMAKE_CURRENT_SOURCE_DIR}/morbo.ld
  )

add_library(stand STATIC
  elf.c
  mbi.c
  mbi2.c
  pci.c
  pci_db.c
  printf.c
  reboot.c
  serial.c
  start.S
  util.c
  ${version_file}

  # libc stuff
  memcpy.c
  memcmp.c
  memset.c
  strlen.c

  # FreeBSD libc stuff
  strncmp.c
  strncpy.c
  strtok.c
  strtoull.c
  qdivrem.c
  )

target_include_directories(stand PUBLIC include/)

add_executable(bender bender.c)
target_link_libraries(bender PRIVATE stand)

add_executable(basicperf basicperf.c)
target_link_libraries(basicperf PRIVATE stand)

add_executable(amy amy.c)
target_link_libraries(amy PRIVATE stand)

add_executable(mbi2_example mbi2_example.c)
target_link_libraries(mbi2_example PRIVATE stand)

install(
  TARGETS bender basicperf mbi2_example
  DESTINATION share/bender
  PERMISSIONS OWNER_READ
  )
