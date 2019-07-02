        PUBLIC mydelay
        EXTERN SystemCoreClock

        SECTION .text : CODE (2)
        THUMB

mydelay:
        ldr r2, =SystemCoreClock
        ldr r2, [r2]
        mov r3, #10000
        udiv r2, r2, r3
loopr0: cbz r0, endr0
        sub r0, r0, #1
        mov r1, r2
loopr1: cbz r1, endr1
        sub r1, r1, #1
        nop
        nop
        nop
        nop
        nop
        nop
        b loopr1
endr1:  b loopr0
endr0:  bx lr

        END
