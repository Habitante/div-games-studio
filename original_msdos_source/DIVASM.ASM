.386p
                NAME    divasm
                EXTRN   compilar_ :BYTE
                EXTRN   free_resources_ :BYTE
                EXTRN   _saved_esp :DWORD
DGROUP          GROUP   CONST,CONST2,_DATA,_BSS
_TEXT           SEGMENT PARA PUBLIC USE32 'CODE'
                ASSUME  CS:_TEXT ,DS:DGROUP,SS:DGROUP
                PUBLIC  memcpyb_,call_,comp_,comp_exit_

; //อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
; //      memcpy con movsb
; //อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

memcpyb_:       PUSH    ECX
                PUSH    ESI
                PUSH    EDI

                MOV     ECX,EBX
                MOV     ESI,EDX
                MOV     EDI,EAX
                REPNE   MOVSB    

                POP     EDI
                POP     ESI
                POP     ECX
                RET

; //อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
; //      Salta a una funciขn
; //อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

call_:          CALL    NEAR EAX
                RET

; //อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
; //      Inicia la compilaciขn
; //อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

comp_:          PUSHA
                PUSH EBP
                MOV     _saved_esp,ESP
                CALL    NEAR PTR compilar_
                RET

; //อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
; //      Finaliza la compilaciขn
; //อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

comp_exit_:     CALL    NEAR PTR free_resources_
                MOV     ESP,_saved_esp
                POP     EBP
                POPA
                RET

_TEXT           ENDS

CONST           SEGMENT DWORD PUBLIC USE32 'DATA'
CONST           ENDS

CONST2          SEGMENT DWORD PUBLIC USE32 'DATA'
CONST2          ENDS

_DATA           SEGMENT DWORD PUBLIC USE32 'DATA'
_DATA           ENDS

_BSS            SEGMENT PARA PUBLIC USE32 'BSS'
_BSS            ENDS

                END
