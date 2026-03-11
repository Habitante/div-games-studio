;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
;	Nucleo en ensamblador de las rutinas de pol죊onos
;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

	p386
	.model flat
	jumps

;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
;	Definici줻 de constantes
;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

vga equ 0a0000h

;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
;	Definici줻 de macros
;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

in_asm	macro

        push ebp
        mov ebp,esp
        push esi
        push edi
        push eax
        push ebx
        push ecx
        push edx

	endm

out_asm macro

        pop edx
        pop ecx
        pop ebx
        pop eax
        pop edi
        pop esi
        pop ebp

	endm

;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
;	Variables externas
;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

	.data

;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
;	Inicio del programa
;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

	.code

        public my_call_
        public nucleo8_8_
        public nucleo8_16_
        public nucleo8_32_
        public nucleo8_64_
        public nucleo8_128_
        public nucleo8_256_
        public mask_nucleo8_8_
        public mask_nucleo8_16_
        public mask_nucleo8_32_
        public mask_nucleo8_64_
        public mask_nucleo8_128_
        public mask_nucleo8_256_
        public _direccion_textura
        public _clock1
        public _clock2

;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
;	Simula un "call" de ensamblador desde C
;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

my_call_	proc

  jmp eax

my_call_	endp

;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
;  Nucleo de texturas para 16 bits por pixel y 8 pixels de ancho/alto
;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

nucleo8_8_ proc

        in_asm

        shl eax,13
        mov esi,ebx

        shl edx,10
        mov edi,[ebp+12]

        shld edx,eax,3  ;eax colocado

        shl eax,3       ;edx colocado
        mov ebp,[ebp+8] ;ebp = ancho

        shl ecx,13
        mov ebx,11100000000000000000000000000111b

        shl esi,10
        shl ebp,1
        add edi,ebp
        shr ebp,1

        shld esi,ecx,3  ;ecx colocado

        shl ecx,3       ;esi colocado
        mov variable_auxiliar,esi

        xchg eax,ecx

        and ebx,edx
        mov esi,_direccion_textura

        rol ebx,3
        mov variable_auxiliar2,eax
        xor eax,eax

        neg ebp         ;ebp = -ancho

;        push eax
;        push edx
;        db 0fh,31h
;        mov _clock1,eax
;        pop edx
;        pop eax
bucle8_8_:
        add ecx,variable_auxiliar2
        mov ax,[esi+ebx*2]

        adc edx,variable_auxiliar
        mov ebx,11100000000000000000000000000111b

        and ebx,edx
        mov [edi+ebp*2],ax

        rol ebx,3

        inc ebp
        jnz bucle8_8_

;        push eax
;        push edx
;        db 0fh,31h
;        mov _clock2,eax
;        pop edx
;        pop eax

        out_asm
        pop eax
        mov [esp+4],eax
        pop eax
        ret

nucleo8_8_ endp

;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
;  Nucleo de texturas para 16 bits por pixel y 16 pixels de ancho/alto
;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

nucleo8_16_ proc

        in_asm

        shl eax,12
        mov esi,ebx

        shl edx,8
        mov edi,[ebp+12]

        shld edx,eax,4  ;eax colocado

        shl eax,4       ;edx colocado
        mov ebp,[ebp+8] ;ebp = ancho

        shl ecx,12
        mov ebx,11110000000000000000000000001111b

        shl esi,8
        shl ebp,1
        add edi,ebp
        shr ebp,1

        shld esi,ecx,4  ;ecx colocado

        shl ecx,4       ;esi colocado
        mov variable_auxiliar,esi

        xchg eax,ecx

        and ebx,edx
        mov esi,_direccion_textura

        rol ebx,4
        mov variable_auxiliar2,eax
        xor eax,eax

        neg ebp         ;ebp = -ancho

;        push eax
;        push edx
;        db 0fh,31h
;        mov _clock1,eax
;        pop edx
;        pop eax
bucle8_16_:
        add ecx,variable_auxiliar2
        mov ax,[esi+ebx*2]

        adc edx,variable_auxiliar
        mov ebx,11110000000000000000000000001111b

        and ebx,edx
        mov [edi+ebp*2],ax

        rol ebx,4

        inc ebp
        jnz bucle8_16_

;        push eax
;        push edx
;        db 0fh,31h
;        mov _clock2,eax
;        pop edx
;        pop eax

        out_asm
        pop eax
        mov [esp+4],eax
        pop eax
        ret

nucleo8_16_ endp

;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
;  Nucleo de texturas para 16 bits por pixel y 32 pixels de ancho/alto
;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

nucleo8_32_ proc

        in_asm

        shl eax,11
        mov esi,ebx

        shl edx,6
        mov edi,[ebp+12]

        shld edx,eax,5  ;eax colocado

        shl eax,5       ;edx colocado
        mov ebp,[ebp+8] ;ebp = ancho

        shl ecx,11
        mov ebx,11111000000000000000000000011111b

        shl esi,6
        shl ebp,1
        add edi,ebp
        shr ebp,1

        shld esi,ecx,5  ;ecx colocado

        shl ecx,5       ;esi colocado
        mov variable_auxiliar,esi

        xchg eax,ecx

        and ebx,edx
        mov esi,_direccion_textura

        rol ebx,5
        mov variable_auxiliar2,eax
        xor eax,eax

        neg ebp         ;ebp = -ancho

;        push eax
;        push edx
;        db 0fh,31h
;        mov _clock1,eax
;        pop edx
;        pop eax
bucle8_32_:
        add ecx,variable_auxiliar2
        mov ax,[esi+ebx*2]

        adc edx,variable_auxiliar
        mov ebx,11111000000000000000000000011111b

        and ebx,edx
        mov [edi+ebp*2],ax

        rol ebx,5

        inc ebp
        jnz bucle8_32_

;        push eax
;        push edx
;        db 0fh,31h
;        mov _clock2,eax
;        pop edx
;        pop eax

        out_asm
        pop eax
        mov [esp+4],eax
        pop eax
        ret

nucleo8_32_ endp

;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
;  Nucleo de texturas para 16 bits por pixel y 64 pixels de ancho/alto
;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

nucleo8_64_ proc

        in_asm

        shl eax,10
        mov esi,ebx

        shl edx,4
        mov edi,[ebp+12]

        shld edx,eax,6  ;eax colocado

        shl eax,6       ;edx colocado
        mov ebp,[ebp+8] ;ebp = ancho

        shl ecx,10
        mov ebx,11111100000000000000000000111111b

        shl esi,4
        shl ebp,1
        add edi,ebp
        shr ebp,1

        shld esi,ecx,6  ;ecx colocado

        shl ecx,6       ;esi colocado
        mov variable_auxiliar,esi

        xchg eax,ecx

        and ebx,edx
        mov esi,_direccion_textura

        rol ebx,6
        mov variable_auxiliar2,eax
        xor eax,eax

        neg ebp         ;ebp = -ancho

;        push eax
;        push edx
;        db 0fh,31h
;        mov _clock1,eax
;        pop edx
;        pop eax
bucle8_64_:
        add ecx,variable_auxiliar2
        mov ax,[esi+ebx*2]

        adc edx,variable_auxiliar
        mov ebx,11111100000000000000000000111111b

        and ebx,edx
        mov [edi+ebp*2],ax

        rol ebx,6

        inc ebp
        jnz bucle8_64_

;        push eax
;        push edx
;        db 0fh,31h
;        mov _clock2,eax
;        pop edx
;        pop eax

        out_asm
        pop eax
        mov [esp+4],eax
        pop eax
        ret

nucleo8_64_ endp

;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
;  Nucleo de texturas para 16 bits por pixel y 128 pixels de ancho/alto
;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

nucleo8_128_ proc

        in_asm

        shl eax,9
        mov esi,ebx

        shl edx,2
        mov edi,[ebp+12]

        shld edx,eax,7  ;eax colocado

        shl eax,7       ;edx colocado
        mov ebp,[ebp+8] ;ebp = ancho

        shl ecx,9
        mov ebx,11111110000000000000000001111111b

        shl esi,2
        shl ebp,1
        add edi,ebp
        shr ebp,1

        shld esi,ecx,7  ;ecx colocado

        shl ecx,7       ;esi colocado
        mov variable_auxiliar,esi

        xchg eax,ecx

        and ebx,edx
        mov esi,_direccion_textura

        rol ebx,7
        mov variable_auxiliar2,eax
        xor eax,eax

        neg ebp         ;ebp = -ancho

;        push eax
;        push edx
;        db 0fh,31h
;        mov _clock1,eax
;        pop edx
;        pop eax
bucle8_128_:
        add ecx,variable_auxiliar2
        mov ax,[esi+ebx*2]

        adc edx,variable_auxiliar
        mov ebx,11111110000000000000000001111111b

        and ebx,edx
        mov [edi+ebp*2],ax

        rol ebx,7

        inc ebp
        jnz bucle8_128_

;        push eax
;        push edx
;        db 0fh,31h
;        mov _clock2,eax
;        pop edx
;        pop eax

        out_asm
        pop eax
        mov [esp+4],eax
        pop eax
        ret

nucleo8_128_ endp

;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
;  Nucleo de texturas para 16 bits por pixel y 256 pixels de ancho/alto
;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

nucleo8_256_ proc

        in_asm

        mov esi,ebx
        mov edi,[ebp+12]

        shl eax,8
        mov ebp,[ebp+8] ;ebp = ancho

        shld edx,eax,8  ;eax colocado

        shl eax,8       ;edx colocado
        shl ebp,1
        add edi,ebp
        shr ebp,1

        neg ebp         ;ebp = -ancho

        shl ecx,8
        mov ebx,11111111000000000000000011111111b

        shld esi,ecx,8  ;ecx colocado

        shl ecx,8       ;esi colocado
        mov variable_auxiliar,esi

        xchg eax,ecx

        and ebx,edx
        mov esi,_direccion_textura

        rol ebx,8
        mov variable_auxiliar2,eax
        xor eax,eax

;        push eax
;        push edx
;        db 0fh,31h
;        mov _clock1,eax
;        pop edx
;        pop eax
bucle8_256_:
        add ecx,variable_auxiliar2
        mov ax,[esi+ebx*2]

        adc edx,variable_auxiliar
        mov ebx,11111111000000000000000011111111b

        and ebx,edx
        mov [edi+ebp*2],ax

        rol ebx,8

        inc ebp
        jnz bucle8_256_

;        push eax
;        push edx
;        db 0fh,31h
;        mov _clock2,eax
;        pop edx
;        pop eax

        out_asm
        pop eax
        mov [esp+4],eax
        pop eax
        ret

nucleo8_256_ endp

;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
;  Nucleo de texturas para 16 bits por pixel y 8 pixels de ancho/alto (mascara)
;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

mask_nucleo8_8_ proc

        in_asm

        shl eax,13
        mov esi,ebx

        shl edx,10
        mov edi,[ebp+12]

        shld edx,eax,3  ;eax colocado

        shl eax,3       ;edx colocado
        mov ebp,[ebp+8] ;ebp = ancho

        shl ecx,13
        mov ebx,11100000000000000000000000000111b

        shl esi,10
        shl ebp,1
        add edi,ebp
        shr ebp,1

        shld esi,ecx,3  ;ecx colocado

        shl ecx,3       ;esi colocado
        mov variable_auxiliar,esi

        xchg eax,ecx

        and ebx,edx
        mov esi,_direccion_textura

        rol ebx,3
        mov variable_auxiliar2,eax
        xor eax,eax

        neg ebp         ;ebp = -ancho

mask_bucle8_8_:
        add ecx,variable_auxiliar2
        mov ax,[esi+ebx*2]

        adc edx,variable_auxiliar
        mov ebx,11100000000000000000000000000111b

        and ebx,edx

        or eax,eax
        jz mask_bucle8_8_fin_
        mov [edi+ebp*2],ax

mask_bucle8_8_fin_:
        rol ebx,3

        inc ebp
        jnz mask_bucle8_8_

        out_asm
        pop eax
        mov [esp+4],eax
        pop eax
        ret

mask_nucleo8_8_ endp

;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
;  Nucleo de texturas para 16 bits por pixel y 16 pixels de ancho/alto (mascara)
;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

mask_nucleo8_16_ proc

        in_asm

        shl eax,12
        mov esi,ebx

        shl edx,8
        mov edi,[ebp+12]

        shld edx,eax,4  ;eax colocado

        shl eax,4       ;edx colocado
        mov ebp,[ebp+8] ;ebp = ancho

        shl ecx,12
        mov ebx,11110000000000000000000000001111b

        shl esi,8
        shl ebp,1
        add edi,ebp
        shr ebp,1

        shld esi,ecx,4  ;ecx colocado

        shl ecx,4       ;esi colocado
        mov variable_auxiliar,esi

        xchg eax,ecx

        and ebx,edx
        mov esi,_direccion_textura

        rol ebx,4
        mov variable_auxiliar2,eax
        xor eax,eax

        neg ebp         ;ebp = -ancho

mask_bucle8_16_:
        add ecx,variable_auxiliar2
        mov ax,[esi+ebx*2]

        adc edx,variable_auxiliar
        mov ebx,11110000000000000000000000001111b

        and ebx,edx

        or eax,eax
        jz mask_bucle8_16_fin_
        mov [edi+ebp*2],ax

mask_bucle8_16_fin_:
        rol ebx,4
        inc ebp
        jnz mask_bucle8_16_

        out_asm
        pop eax
        mov [esp+4],eax
        pop eax
        ret

mask_nucleo8_16_ endp

;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
;  Nucleo de texturas para 16 bits por pixel y 32 pixels de ancho/alto (mascara)
;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

mask_nucleo8_32_ proc

        in_asm

        shl eax,11
        mov esi,ebx

        shl edx,6
        mov edi,[ebp+12]

        shld edx,eax,5  ;eax colocado

        shl eax,5       ;edx colocado
        mov ebp,[ebp+8] ;ebp = ancho

        shl ecx,11
        mov ebx,11111000000000000000000000011111b

        shl esi,6
        shl ebp,1
        add edi,ebp
        shr ebp,1

        shld esi,ecx,5  ;ecx colocado

        shl ecx,5       ;esi colocado
        mov variable_auxiliar,esi

        xchg eax,ecx

        and ebx,edx
        mov esi,_direccion_textura

        rol ebx,5
        mov variable_auxiliar2,eax
        xor eax,eax

        neg ebp         ;ebp = -ancho

mask_bucle8_32_:
        add ecx,variable_auxiliar2
        mov ax,[esi+ebx*2]

        adc edx,variable_auxiliar
        mov ebx,11111000000000000000000000011111b

        and ebx,edx

        or eax,eax
        jz mask_bucle8_32_fin_
        mov [edi+ebp*2],ax

mask_bucle8_32_fin_:
        rol ebx,5
        inc ebp
        jnz mask_bucle8_32_

        out_asm
        pop eax
        mov [esp+4],eax
        pop eax
        ret

mask_nucleo8_32_ endp

;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
;  Nucleo de texturas para 16 bits por pixel y 64 pixels de ancho/alto (mascara)
;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

mask_nucleo8_64_ proc

        in_asm

        shl eax,10
        mov esi,ebx

        shl edx,4
        mov edi,[ebp+12]

        shld edx,eax,6  ;eax colocado

        shl eax,6       ;edx colocado
        mov ebp,[ebp+8] ;ebp = ancho

        shl ecx,10
        mov ebx,11111100000000000000000000111111b

        shl esi,4
        shl ebp,1
        add edi,ebp
        shr ebp,1

        shld esi,ecx,6  ;ecx colocado

        shl ecx,6       ;esi colocado
        mov variable_auxiliar,esi

        xchg eax,ecx

        and ebx,edx
        mov esi,_direccion_textura

        rol ebx,6
        mov variable_auxiliar2,eax
        xor eax,eax

        neg ebp         ;ebp = -ancho

mask_bucle8_64_:
        add ecx,variable_auxiliar2
        mov ax,[esi+ebx*2]

        adc edx,variable_auxiliar
        mov ebx,11111100000000000000000000111111b

        and ebx,edx

        or eax,eax
        jz mask_bucle8_64_fin_
        mov [edi+ebp*2],ax

mask_bucle8_64_fin_:
        rol ebx,6
        inc ebp
        jnz mask_bucle8_64_

        out_asm
        pop eax
        mov [esp+4],eax
        pop eax
        ret

mask_nucleo8_64_ endp

;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
;  Nucleo de texturas para 16 bits por pixel y 128 pixels de ancho/alto (mascara)
;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

mask_nucleo8_128_ proc

        in_asm

        shl eax,9
        mov esi,ebx

        shl edx,2
        mov edi,[ebp+12]

        shld edx,eax,7  ;eax colocado

        shl eax,7       ;edx colocado
        mov ebp,[ebp+8] ;ebp = ancho

        shl ecx,9
        mov ebx,11111110000000000000000001111111b

        shl esi,2
        shl ebp,1
        add edi,ebp
        shr ebp,1

        shld esi,ecx,7  ;ecx colocado

        shl ecx,7       ;esi colocado
        mov variable_auxiliar,esi

        xchg eax,ecx

        and ebx,edx
        mov esi,_direccion_textura

        rol ebx,7
        mov variable_auxiliar2,eax
        xor eax,eax

        neg ebp         ;ebp = -ancho

mask_bucle8_128_:
        add ecx,variable_auxiliar2
        mov ax,[esi+ebx*2]

        adc edx,variable_auxiliar
        mov ebx,11111110000000000000000001111111b

        and ebx,edx

        or eax,eax
        jz mask_bucle8_128_fin_
        mov [edi+ebp*2],ax

mask_bucle8_128_fin_:
        rol ebx,7
        inc ebp
        jnz mask_bucle8_128_

        out_asm
        pop eax
        mov [esp+4],eax
        pop eax
        ret

mask_nucleo8_128_ endp

;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
;  Nucleo de texturas para 16 bits por pixel y 256 pixels de ancho/alto (mascara)
;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

mask_nucleo8_256_ proc

        in_asm

        mov esi,ebx
        mov edi,[ebp+12]

        shl eax,8
        mov ebp,[ebp+8] ;ebp = ancho

        shld edx,eax,8  ;eax colocado

        shl eax,8       ;edx colocado
        shl ebp,1
        add edi,ebp
        shr ebp,1

        neg ebp         ;ebp = -ancho

        shl ecx,8
        mov ebx,11111111000000000000000011111111b

        shld esi,ecx,8  ;ecx colocado

        shl ecx,8       ;esi colocado
        mov variable_auxiliar,esi

        xchg eax,ecx

        and ebx,edx
        mov esi,_direccion_textura

        rol ebx,8
        mov variable_auxiliar2,eax
        xor eax,eax

mask_bucle8_256_:
        add ecx,variable_auxiliar2
        mov ax,[esi+ebx*2]

        adc edx,variable_auxiliar
        mov ebx,11111111000000000000000011111111b

        and ebx,edx

        or eax,eax
        jz mask_bucle8_256_fin_
        mov [edi+ebp*2],ax

mask_bucle8_256_fin_:
        rol ebx,8
        inc ebp
        jnz mask_bucle8_256_

        out_asm
        pop eax
        mov [esp+4],eax
        pop eax
        ret

mask_nucleo8_256_ endp

;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
;	Segmento de datos
;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

	.data

variable_auxiliar dd 0
variable_auxiliar2 dd 0

_clock1 dd 0
_clock2 dd 0
_direccion_textura dd 0

ajuste db 128 dup(0)

	end
