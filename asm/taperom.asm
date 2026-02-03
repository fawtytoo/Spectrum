; assembly used for the tape rom

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
out ($7f),a
ld a,d
out ($7f),a

ld a,c
out ($7f),a

sa_loop:
ld a,(ix+$00)
out ($7f),a
xor c
ld c,a
inc ix
dec de
ld a,d
or e
jr nz,sa_loop

ld a,c              ; parity byte
out ($7f),a

scf
ret

defs 115,0          ; padding

; LD-BYTES ---------------------------------------------------------------------
;   a  = 0 header, 255 data
;   f  = carry flag set if loading, reset if verifying
;   de = block length
;   ix = start address

; verifying not supported

inc d
ex af,af'
dec d
di
ld hl,$053f
push hl
in a,($fe)
cp a
ret nz
ex af,af'
ret nc              ; causes "R Tape loading error"

defs 8,0            ; to cater for late entry points

call ld_start
ret

ld_start:
ld c,a
;ld a,$ff
in a,($7f)          ; request block type
xor c               ; clears carry flag
ret nz              ; wrong block type

ld_loop:            ; main loop
;ld a,$ff
in a,($7f)          ; request data
ld (ix+$00),a
xor c
ld c,a
inc ix
dec de
ld a,d
or e
jr nz,ld_loop

;ld a,$ff
in a,($7f)          ; parity byte
xor c
ret nz

scf
ret

; ------------------------------------------------------------------------------
