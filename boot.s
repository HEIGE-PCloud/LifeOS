.section ".text.boot"

.global _start

_start:
    // Read the CPU ID from MPIDR_EL1
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, 2f
    // If CPU ID > 0, stop
1:  wfe
    b       1b
2:  // CPU ID == 0, then
    // set the top of stack before the code
    ldr     x1, =_start
    mov     sp, x1
    // Clear the BSS
    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
3:  cbz     w2, 4f
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 3b
    // jump to C code
4:  bl      main
    // Infinite loop
    b       1b
