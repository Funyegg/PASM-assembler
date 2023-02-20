; Set origin and start
ORG 0x7C00
MOV DI _start
; JMP to start
JMP DI

; Char printing function
LABEL _outchar
MOV AH 0x0E
INT 0x10
RET

; Actual start
LABEL _start
; Prepare to call the outchar function
MOV DI _outchar

; Print 2 chars
MOV AL 66
CALL DI
INC AL
CALL DI

; Loop forecer
LABEL _infloop
MOV DI _infloop
JMP DI

PAD 510
DW 0xAA55
