.text

.global enable_paging
.global load_CR3

.align 4
enable_paging:
    # set PGE and PSE in CR4
    movl  %cr4, %eax
    orl   $0x00000080, %eax
    orl   $0x00000010, %eax
    movl  %eax, %cr4

    # set PG and PE in CR0
    movl  %cr0, %eax
    orl   $0x80000000, %eax
    orl   $0x00000001, %eax
    movl  %eax, %cr0

    ret

load_CR3:
    movl  4(%esp), %eax
    movl  %cr3, %ecx
    andl  $0x00000FFF, %ecx
    orl   %ecx, %eax
    movl  %eax, %cr3
    ret
