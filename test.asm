; Test of PASM

; Signify origin and start
ORG 0x7C00
LABEL _start

; Display a character
MOV AH 0x0E
MOV AL 65
INT 0x10

; JMP to the start
MOV AX _start
JMP AX

; EXTRA: Call something, just to see if this assemble that
MOV DI 0x6969
CALL DI

; Return from ???
RET

; Pad out to make it bootloader suitable
PAD 510
DW 0xAA55

; Can it comment at the end?
