#ifndef __VOLCADOS_HPP_
#define __VOLCADOS_HPP_
extern long color_cls;

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Hace un volcado lineal
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

//-----------------------------------------------------------------------------
//  Realiza el volcado en 8 bpps
//-----------------------------------------------------------------------------

void volcado_8(char *destino, char *fuente, long longitud, long dir_lux);
#pragma aux volcado_8= \
        "add edi,ecx" \
        "xor eax,eax" \
        "neg ecx" \
        "dec edi" \
"bucle_col:" \
        "mov ax,[esi]" \
        "add esi,2" \
        "mov al,[ebx+eax]" \
        "inc ecx" \
        "mov [edi+ecx],al" \
        "jnz bucle_col" \
        parm [edi] [esi] [ecx] [ebx]\
        modify [eax] \

//-----------------------------------------------------------------------------
//  Realiza el volcado en 16 bpps
//-----------------------------------------------------------------------------

void volcado_16(char *, char *, int);
#pragma aux volcado_16= \
        "shr ecx,2" \
        "rep movsd" \
        parm [edi] [esi] [ecx] \
        modify [eax] \
/*
void volcado_16(char *, char *, int);
#pragma aux volcado_16= \
        "push ebx" \
        "mov ebx,ecx" \
        "shr ebx,8" \
"bucle_volcado:" \
        "mov ecx,64" \
        "rep movsd" \
        "dec ebx" \
        "jnz bucle_volcado" \
        "pop ebx" \
        parm [edi] [esi] [ecx] \
        modify [eax] \
*/
//-----------------------------------------------------------------------------
//  Realiza el volcado en 32 bpps
//-----------------------------------------------------------------------------

void volcado_32(char *destino, char *fuente, long longitud);
#pragma aux volcado_32= \
        "push ebx" \
        "push ebp" \
        "add edi,ecx" \
        "shr ecx,1" \
        "cmp ecx,0" \
        "je bucle_fin" \
        "sub edi,2" \
        "neg ecx" \
"bucle_col:" \
        "xor eax,eax" \
        "mov ebx,[esi]" \
        "mov ax,bx" \
        "shr ah,2" \
        "add esi,4" \
        "shr ebx,8" \
        "and ebx,0f800h" \
        "shr eax,3" \
        "add eax,ebx" \
        "inc ecx" \
        "mov [edi+ecx*2],ax" \
        "jnz bucle_col" \
"bucle_fin:" \
        "pop ebp" \
        "pop ebx" \
        parm [edi] [esi] [ecx]\
        modify [eax] \

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Hace un volcado lineal y borra la pantalla
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

//-----------------------------------------------------------------------------
//  Realiza el volcado en 8 bpps con borrado de copia
//-----------------------------------------------------------------------------

void volcado_cls_8(char *destino, char *fuente, long longitud, long dir_lux);
#pragma aux volcado_cls_8= \
        "add edi,ecx" \
        "xor eax,eax" \
        "neg ecx" \
        "dec edi" \
        "mov edx,dword ptr[color_cls]" \
"bucle_col:" \
        "mov ax,[esi]" \
        "add esi,2" \
        "mov al,[ebx+eax]" \
        "inc ecx" \
        "mov [esi-2],dx" \
        "mov [edi+ecx],al" \
        "jnz bucle_col" \
        parm [edi] [esi] [ecx] [ebx]\
        modify [eax edx]\

//-----------------------------------------------------------------------------
//  Realiza el volcado en 16 bpps
//-----------------------------------------------------------------------------

void volcado_cls_16(char *, char *, int);
#pragma aux volcado_cls_16= \
        "push ebx" \
        "mov eax,dword ptr[color_cls]" \
        "shl eax,16" \
        "mov ax,word ptr[color_cls]" \
        "mov ebx,ecx" \
        "shr ebx,8" \
"bucle_volcado:" \
        "push esi" \
        "mov ecx,64" \
        "rep movsd" \
        "mov esi,edi" \
        "pop edi" \
        "mov ecx,64" \
        "rep stosd" \
        "xchg esi,edi" \
        "dec ebx" \
        "jnz bucle_volcado" \
        "pop ebx" \
        parm [edi] [esi] [ecx] \
        modify [eax] \

//-----------------------------------------------------------------------------
//  Realiza el volcado en 32 bpps
//-----------------------------------------------------------------------------

void volcado_cls_32(char *destino, char *fuente, long longitud);
#pragma aux volcado_cls_32= \
        "push ebx" \
        "push ebp" \
        "add edi,ecx" \
        "shr ecx,1" \
        "cmp ecx,0" \
        "je bucle_fin" \
        "sub edi,2" \
        "mov ebp,dword ptr[color_cls]" \
        "neg ecx" \
"bucle_col:" \
        "xor eax,eax" \
        "mov ebx,[esi]" \
        "mov ax,bx" \
        "mov [esi],ebp" \
        "shr ah,2" \
        "add esi,4" \
        "shr ebx,8" \
        "and ebx,0f800h" \
        "shr eax,3" \
        "add eax,ebx" \
        "inc ecx" \
        "mov [edi+ecx*2],ax" \
        "jnz bucle_col" \
"bucle_fin:" \
        "pop ebp" \
        "pop ebx" \
        parm [edi] [esi] [ecx]\
        modify [eax] \

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
#endif
