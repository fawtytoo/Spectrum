; assembly used for the tape rom

org $0000
TAPE_DATA:          ; location where data is either read from or written to

defs 1218, 0        ; padding

; SA-BYTES ---------------------------------------------------------------------
;   a  = 0 header, 255 data
;   de = block length
;   ix = start address

call sa_start
ret

sa_start:
ld c,a

ld a,e              ; transmit block length
ld (TAPE_DATA),a
ld a,d
ld (TAPE_DATA),a

ld a,c
ld (TAPE_DATA),a

sa_loop:
ld a,(ix+$00)
ld (TAPE_DATA),a
xor c
ld c,a
inc ix
dec de
ld a,d
or e
jr nz,sa_loop

ld a,c              ; parity byte
ld (TAPE_DATA),a

scf
ret

defs 110,0          ; padding

; LD-BYTES ---------------------------------------------------------------------
;   a  = 0 header, 255 data
;   f  = carry flag set if loading, reset if verifying
;   de = block length
;   ix = start address

inc d
ex af,af'
dec d
di
nop
nop
nop
nop
ld hl,$053f
push hl
in a,($fe)
nop
nop
nop
nop
nop
nop
cp a
ret nz

call ld_start
ret

ld_start:
ex af,af'
ret nc              ; verifying not supported (R Tape loading error)
ld c,a
ld a,(TAPE_DATA)
xor c               ; clears carry flag
ret nz              ; wrong block type

ld_loop:            ; main loop
ld a,(TAPE_DATA)
ld (ix+$00),a
xor c
ld c,a
inc ix
dec de
ld a,d
or e
jr nz,ld_loop

ld a,(TAPE_DATA)
xor c
ret nz

scf
ret

; ------------------------------------------------------------------------------
