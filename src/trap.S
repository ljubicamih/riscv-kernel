.global supervisorTrap
.align 4
supervisorTrap:
    # sscratch drzi systemStackTop tekuce niti
    # swap da dobijemo jedan slobodan registar: sscratch <-> t6.
    csrrw t6, sscratch, t6      # t6 = systemStackTop, sscratch = stari t6

    # cuva t5 privremeno na sistemski stek da imamo dva slobodna (t5, t6)
    sd t5, -8(t6)

    # t5 = running (TCB tekuce niti)
    la t5, _ZN3TCB7runningE
    ld t5, 0(t5)

    # registri se cuvaju u context tekuce niti
    sd ra,   0(t5)
    sd t0,  16(t5)
    sd t1,  24(t5)
    sd t2,  32(t5)
    sd t3,  40(t5)
    sd t4,  48(t5)
    sd a0,  72(t5)
    sd a1,  80(t5)
    sd a2,  88(t5)
    sd a3,  96(t5)
    sd a4, 104(t5)
    sd a5, 112(t5)
    sd a6, 120(t5)
    sd a7, 128(t5)
    sd s0, 136(t5)
    sd s1, 144(t5)
    sd s2, 152(t5)
    sd s3, 160(t5)
    sd s4, 168(t5)
    sd s5, 176(t5)
    sd s6, 184(t5)
    sd s7, 192(t5)
    sd s8, 200(t5)
    sd s9, 208(t5)
    sd s10,216(t5)
    sd s11,224(t5)

    # t6 (=64): pravi t6 je u sscratch (zamenjen na pocetku)
    csrr t0, sscratch
    sd t0, 64(t5)               # context.t6 = originalni t6



    ld t0, -8(t6)
    sd t0, 56(t5)              # context.t5 = originalni t5

    # sp (=8): nitin pravi sp (jos nije menjan)
    sd sp, 8(t5)

    # vraca sscratch na systemStackTop (da sledeci trap ove niti opet radi)
    csrw sscratch, t6

    # sepc, sstatus
    csrr t0, sepc
    sd t0, 232(t5)
    csrr t0, sstatus
    sd t0, 240(t5)

    # prelazi na sistemski stek i zove handleTrap
    mv sp, t6                  # sp = systemStackTop
    addi sp, sp, -32

    call handleTrap

    # handleTrap je vec azurirao TCB::running i upisao povratnu vrednost
    # u running->context.a0
    la t5, _ZN3TCB7runningE
    ld t5, 0(t5)

restore:
    # t5 = running (tekuca ili nova nit)
    ld t0, 232(t5)
    csrw sepc, t0
    ld t0, 240(t5)
    csrw sstatus, t0
    ld t0, 280(t5)
    csrw sscratch, t0          # systemStackTop nove niti u sscratch

    ld ra,   0(t5)
    ld t0,  16(t5)
    ld t1,  24(t5)
    ld t2,  32(t5)
    ld t3,  40(t5)
    ld t4,  48(t5)
    ld a0,  72(t5)
    ld a1,  80(t5)
    ld a2,  88(t5)
    ld a3,  96(t5)
    ld a4, 104(t5)
    ld a5, 112(t5)
    ld a6, 120(t5)
    ld a7, 128(t5)
    ld s0, 136(t5)
    ld s1, 144(t5)
    ld s2, 152(t5)
    ld s3, 160(t5)
    ld s4, 168(t5)
    ld s5, 176(t5)
    ld s6, 184(t5)
    ld s7, 192(t5)
    ld s8, 200(t5)
    ld s9, 208(t5)
    ld s10,216(t5)
    ld s11,224(t5)
    ld sp,   8(t5)
    ld t6,  64(t5)
    ld t5,  56(t5)            # t5 poslednji (baza)
    sret