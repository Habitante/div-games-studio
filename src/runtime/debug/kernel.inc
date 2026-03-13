
//-----------------------------------------------------------------------------
// Nucleo de la ejecución de los programas en DIV
//-----------------------------------------------------------------------------

case lnop:
break;
case lcar:
stack[++sp] = mem[ip++];
break;
case lasi:
stack[sp - 1] = mem[stack[sp - 1]] = stack[sp];
sp--;
break;
case lori:
stack[sp - 1] |= stack[sp];
sp--;
break;
case lxor:
stack[sp - 1] ^= stack[sp];
sp--;
break;
case land:
stack[sp - 1] &= stack[sp];
sp--;
break;
case ligu:
stack[sp - 1] = stack[sp - 1] == stack[sp];
sp--;
break;
case ldis:
stack[sp - 1] = stack[sp - 1] != stack[sp];
sp--;
break;
case lmay:
stack[sp - 1] = stack[sp - 1] > stack[sp];
sp--;
break;
case lmen:
stack[sp - 1] = stack[sp - 1] < stack[sp];
sp--;
break;
case lmei:
stack[sp - 1] = stack[sp - 1] <= stack[sp];
sp--;
break;
case lmai:
stack[sp - 1] = stack[sp - 1] >= stack[sp];
sp--;
break;
case ladd:
stack[sp - 1] += stack[sp];
sp--;
break;
case lsub:
stack[sp - 1] -= stack[sp];
sp--;
break;
case lmul:
stack[sp - 1] *= stack[sp];
sp--;
break;
case ldiv:
if (stack[sp] == 0) {
  stack[--sp] = 0;
#ifdef DEBUG
  v_function = -2;
  e(145);
  if (call_to_debug) {
    process_stoped = id;
    return;
  }
#endif
} else {
  stack[sp - 1] /= stack[sp];
  sp--;
}
break;
case lmod:
if (stack[sp] == 0) {
  stack[--sp] = 0;
#ifdef DEBUG
  v_function = -2;
  e(145);
  if (call_to_debug) {
    process_stoped = id;
    return;
  }
#endif
} else {
  stack[sp - 1] %= stack[sp];
  sp--;
}
break;
case lneg:
stack[sp] = -stack[sp];
break;
case lptr:
stack[sp] = mem[stack[sp]];
break;
case lnot:
stack[sp] ^= -1;
break;
case laid:
stack[sp] += id;
break;
case lcid:
stack[++sp] = id;
break;
case lrng:
#ifdef DEBUG
if (stack[sp] < 0 || stack[sp] > mem[ip]) {
  v_function = -2;
  e(140);
  if (call_to_debug) {
    ip++;
    process_stoped = id;
    return;
  }
}
ip++;
#else
ip++;
#endif
break;
case ljmp:
ip = mem[ip];
#ifdef DEBUG
if (frame_clock > max_reloj) {
  v_function = -2;
  e(142);
  max_reloj = max_process_time + frame_clock;
  if (call_to_debug) {
    process_stoped = id;
    return;
  }
}
#endif
break;
case ljpf:
if (stack[sp--] & 1)
  ip++;
else
  ip = mem[ip];
#ifdef DEBUG
if (frame_clock > max_reloj) {
  v_function = -2;
  e(142);
  max_reloj = max_process_time + frame_clock;
  if (call_to_debug) {
    process_stoped = id;
    return;
  }
}
#endif
break;
case lfun:
function();
#ifdef DEBUG
if (call_to_debug) {
  process_stoped = id;
  return;
}
#endif
break;
case lcal:
#ifdef DEBUG
process_exec(id, get_ticks() - profile_clock);
profile_clock = get_ticks();
#endif
mem[id + _IP] = ip + 1;
id2 = id;
if (sp > EVAL_STACK_SIZE)
  exer(3);
process_count++;
ip = mem[ip];
id = id_start;

//printf("Processes: %d  id:%d p*len:%d id_end: %d\n",process_count,id_end-id,(id_start)+((process_count-2)*iloc_len),id_end);


if ((id_start + ((process_count - 2) * iloc_len)) == id_end)
  id = id_end + iloc_len;


//if((processos-1)*iloc_len>id_end)


while (mem[id + _Status] && id <= id_end)
  id += iloc_len;
#ifdef LLPROC
insert_process(id);
#endif
if (id > id_end) {
  if (id > imem_max - iloc_len)
    exer(2);
  id_end = id;
  if (id_end > id_max)
    id_max = id_end;
}
memcpy(&mem[id], &mem[iloc], iloc_pub_len << 2);
mem[id + _Id] = id;
if ((mem[id + _BigBro] = mem[id2 + _Son]))
  mem[mem[id + _BigBro] + _SmallBro] = id;
mem[id2 + _Son] = id;
mem[id + _Father] = mem[id + _Caller] = id2;
if (mem[ip + 2] == lnop)
  mem[id + _FCount] = mem[id2 + _FCount] + 1; // Función
#ifdef DEBUG
else
  process_level++;
#endif
break;
case lret:
#ifdef DEBUG
if (mem[id + _FCount] == 0)
  process_level--;
process_exec(id, get_ticks() - profile_clock);
profile_clock = get_ticks();
#endif
sp = mem[id + _Param] - mem[id + _NumPar];
stack[sp] = id;
id = mem[id + _Caller];
kill_process(stack[sp]);
if (!(id & 1))
  goto next_process1;
ip = mem[id + _IP];
break;
case lasp:
sp--;
break;
case lfrm:
#ifdef DEBUG
process_exec(id, get_ticks() - profile_clock);
profile_clock = get_ticks();
#endif
sp = mem[id + _Param] - mem[id + _NumPar];
mem[id + _IP] = ip;
next_frm : mem[id + _IdScan] = 0;
mem[id + _BlScan] = 0;
stack[sp] = id;
id = mem[id + _Caller];
if (!(id & 1))
  goto next_process1;
ip = mem[id + _IP];
if (mem[stack[sp] + _FCount] > 0) {          // Si era una funcion, duerme al padre.
  mem[stack[sp] + _Caller]++;                // Caller dormido
  bp = sp;                                   // Deja la stack aparcada en el lugar adecuado
  sp = mem[id + _Param] - mem[id + _NumPar]; // Y la prerara para retornar al anterior
  save_stack(id, sp, bp);
  mem[id + _Status] = 3;
  goto next_frm;
} else {
#ifdef DEBUG
  process_level--;
#endif
  mem[stack[sp] + _Caller] = 0;
  mem[stack[sp] + _Executed] = 1;
}
break;
case lcbp:
mem[id + _NumPar] = mem[ip++];
mem[id + _Param] = sp - mem[id + _NumPar] + 1;
break;
case lcpa:
mem[stack[sp--]] = stack[mem[id + _Param]++];
break;
case ltyp:
mem[id + _Bloque] = mem[ip++];
private_start = mem[6];
break;
case lpri:
memcpy(&mem[id + private_start], &mem[ip + 1], (mem[ip] - ip - 1) << 2);
private_start += (mem[ip] - ip - 1);
ip = mem[ip];
break;
case lcse:
if (stack[sp - 1] == stack[sp])
  ip++;
else
  ip = mem[ip];
sp--;
break;
case lcsr:
if (stack[sp - 2] >= stack[sp - 1] && stack[sp - 2] <= stack[sp]) {
  ip++;
} else {
  ip = mem[ip];
}
sp -= 2;
break;
case lshr:
stack[sp - 1] >>= stack[sp];
sp--;
break;
case lshl:
stack[sp - 1] <<= stack[sp];
sp--;
break;
case lipt:
stack[sp] = ++mem[stack[sp]];
break;
case lpti:
stack[sp] = mem[stack[sp]]++;
break;
case ldpt:
stack[sp] = --mem[stack[sp]];
break;
break;
case lptd:
stack[sp] = mem[stack[sp]]--;
break;
case lada:
stack[sp - 1] = mem[stack[sp - 1]] += stack[sp];
sp--;
break;
case lsua:
stack[sp - 1] = mem[stack[sp - 1]] -= stack[sp];
sp--;
break;
case lmua:
stack[sp - 1] = mem[stack[sp - 1]] *= stack[sp];
sp--;
break;
case ldia:
if (stack[sp] == 0) {
  mem[stack[sp - 1]] = 0;
  stack[--sp] = 0;
#ifdef DEBUG
  v_function = -2;
  e(145);
  if (call_to_debug) {
    process_stoped = id;
    return;
  }
#endif
} else {
  stack[sp - 1] = mem[stack[sp - 1]] /= stack[sp];
  sp--;
}
break;
case lmoa:
if (stack[sp] == 0) {
  mem[stack[sp - 1]] = 0;
  stack[--sp] = 0;
#ifdef DEBUG
  v_function = -2;
  e(145);
  if (call_to_debug) {
    process_stoped = id;
    return;
  }
#endif
} else {
  stack[sp - 1] = mem[stack[sp - 1]] %= stack[sp];
  sp--;
}
break;
case lana:
stack[sp - 1] = mem[stack[sp - 1]] &= stack[sp];
sp--;
break;
case lora:
stack[sp - 1] = mem[stack[sp - 1]] |= stack[sp];
sp--;
break;
case lxoa:
stack[sp - 1] = mem[stack[sp - 1]] ^= stack[sp];
sp--;
break;
case lsra:
stack[sp - 1] = mem[stack[sp - 1]] >>= stack[sp];
sp--;
break;
case lsla:
stack[sp - 1] = mem[stack[sp - 1]] <<= stack[sp];
sp--;
break;
case lpar:
private_start += mem[ip++];
break;
case lrtf:
#ifdef DEBUG
if (mem[id + _FCount] == 0)
  process_level--;
process_exec(id, get_ticks() - profile_clock);
profile_clock = get_ticks();
#endif
bp = stack[sp];
sp = mem[id + _Param] - mem[id + _NumPar];
stack[sp] = bp;
bp = id;
id = mem[id + _Caller];
kill_process(bp);
if (!(id & 1)) {
  if (id)
    update_stack(id - 1, stack[sp]); // Un return de función (tras frame)
  goto next_process1;
}
ip = mem[id + _IP];
break;
case lclo:
process_count++;
id2 = id;
id = id_start;
while (mem[id + _Status] && id <= id_end)
  id += iloc_len;
if (id > id_end) {
  if (id > imem_max - iloc_len)
    exer(2);
  id_end = id;
}
memcpy(&mem[id], &mem[id2], iloc_len << 2);
mem[id + _Id] = id;
mem[id + _IP] = ip + 1;
mem[id + _Caller] = 0;
if ((mem[id + _BigBro] = mem[id2 + _Son]))
  mem[mem[id + _BigBro] + _SmallBro] = id;
mem[id + _SmallBro] = 0;
mem[id + _Son] = 0;
mem[id2 + _Son] = id;
mem[id + _Father] = id2;
id = id2;
ip = mem[ip];
break;
case lfrf:
#ifdef DEBUG
process_exec(id, get_ticks() - profile_clock);
profile_clock = get_ticks();
#endif
if ((mem[id + _Frame] += stack[sp]) >= 100) {
  mem[id + _Frame] -= 100;
  mem[id + _Executed] = 1;
}
sp = mem[id + _Param] - mem[id + _NumPar];
mem[id + _IP] = ip;
next_frf : mem[id + _IdScan] = 0;
mem[id + _BlScan] = 0;
stack[sp] = id;
id = mem[id + _Caller];
if (!(id & 1))
  goto next_process2;
ip = mem[id + _IP];
if (mem[stack[sp] + _FCount] > 0) {          // Si es una funcion, duerme al padre.
  mem[stack[sp] + _Caller]++;                // Caller dormido
  bp = sp;                                   // Deja la stack aparcada en el lugar adecuado
  sp = mem[id + _Param] - mem[id + _NumPar]; // Y la prerara para retornar al anterior
  save_stack(id, sp, bp);
  mem[id + _Status] = 3;
  goto next_frf;
} else {
  mem[stack[sp] + _Caller] = 0;
#ifdef DEBUG
  process_level--;
#endif
}
break;
case limp:
printf("skipping dll %s\n", (char *)&mem[mem[ip++]]);
break;
case lext:
ip++;
break;
case lchk:
#ifdef DEBUG
if (stack[sp] < id_init || stack[sp] > id_max || stack[sp] != mem[stack[sp]]) {
  v_function = -2;
  e(141);
  if (call_to_debug) {
    process_stoped = id;
    return;
  }
}
#endif
break;
case ldbg:
#ifdef DEBUG
if (debug_active) {
  for (ibreakpoint = 0; ibreakpoint < max_breakpoint; ibreakpoint++)
    if (breakpoint[ibreakpoint].line > -1 && abs(breakpoint[ibreakpoint].offset) == ip - 1)
      break;
  if (ibreakpoint < max_breakpoint) { // Se llegó a un breakpoint
    mem[--ip] = breakpoint[ibreakpoint].code;
    breakpoint[ibreakpoint].line = -1;
    call_to_debug = 1;
    process_stoped = id;
    ids_old = -1; // Para que se vean los datos del proceso que tenia el break
    return;
  }
#ifndef TRACE
  deb();
  if (call_to_debug)
    return;
#endif
}
#endif
break;

case lcar2:
stack[++sp] = mem[ip++];
stack[++sp] = mem[ip++];
break;
case lcar3:
stack[++sp] = mem[ip++];
stack[++sp] = mem[ip++];
stack[++sp] = mem[ip++];
break;
case lcar4:
stack[++sp] = mem[ip++];
stack[++sp] = mem[ip++];
stack[++sp] = mem[ip++];
stack[++sp] = mem[ip++];
break;
case lasiasp:
mem[stack[sp - 1]] = stack[sp];
sp -= 2;
break;
case lcaraid:
#ifdef LLPROC
// check if setting priority or z
if (mem[ip] == _Priority || mem[ip] == _Z && checklist == 0) {
  checklist = mem[ip]; // save z or p
}
#endif

stack[++sp] = mem[ip++] + id;
break;
case lcarptr:
stack[++sp] = mem[mem[ip++]];
break;
case laidptr:
stack[sp] = mem[stack[sp] + id];
break;
case lcaraidptr:
stack[++sp] = mem[mem[ip++] + id];
break;
case lcaraidcpa:
mem[mem[ip++] + id] = stack[mem[id + _Param]++];
break;
case laddptr:
stack[sp - 1] = mem[stack[sp - 1] + stack[sp]];
sp--;
break;
case lfunasp:
function();
sp--;
#ifdef DEBUG
if (call_to_debug) {
  process_stoped = id;
  return;
}
#endif
break;
case lcaradd:
stack[sp] += mem[ip++];
break;
case lcaraddptr:
stack[sp] = mem[stack[sp] + mem[ip++]];
break;
case lcarmul:
stack[sp] *= mem[ip++];
break;
case lcarmuladd:
stack[sp - 1] += stack[sp] * mem[ip++];
sp--;
break;
case lcarasiasp:
#ifdef LLPROC
if (checklist > 0 && mem[stack[sp]] != mem[ip] && id > 0) {
  dtemp = mem[stack[sp]];
  printf("id: %d old val: %d\n", id, dtemp);
  mem[stack[sp]] = mem[ip++];
  sp--;
  dirty(id);
} else
#endif
  mem[stack[sp]] = mem[ip++];
sp--;
break;
case lcarsub:
stack[sp] -= mem[ip++];
break;
case lcardiv:
stack[sp] /= mem[ip++];
break; // No hay nunca "cardiv 0"

case lptrchr:
stack[sp - 1] = (unsigned int)memb[stack[sp - 1] * 4 + stack[sp]];
sp--;
break;
case lasichr:
stack[sp - 2] = (unsigned int)(memb[stack[sp - 2] * 4 + stack[sp - 1]] = (byte)stack[sp]);
sp -= 2;
break;
case liptchr:
stack[sp - 1] = (unsigned int)++memb[stack[sp - 1] * 4 + stack[sp]];
sp--;
break;
case lptichr:
stack[sp - 1] = (unsigned int)memb[stack[sp - 1] * 4 + stack[sp]]++;
sp--;
break;
case ldptchr:
stack[sp - 1] = (unsigned int)--memb[stack[sp - 1] * 4 + stack[sp]];
sp--;
break;
case lptdchr:
stack[sp - 1] = (unsigned int)memb[stack[sp - 1] * 4 + stack[sp]]--;
sp--;
break;
case ladachr:
stack[sp - 2] = (unsigned int)(memb[stack[sp - 2] * 4 + stack[sp - 1]] += (byte)stack[sp]);
sp -= 2;
break;
case lsuachr:
stack[sp - 2] = (unsigned int)(memb[stack[sp - 2] * 4 + stack[sp - 1]] -= (byte)stack[sp]);
sp -= 2;
break;
case lmuachr:
stack[sp - 2] = (unsigned int)(memb[stack[sp - 2] * 4 + stack[sp - 1]] *= (byte)stack[sp]);
sp -= 2;
break;
case ldiachr:
if (stack[sp] == 0) {
  memb[stack[sp - 2] * 4 + stack[sp - 1]] = 0;
  sp -= 2;
  stack[sp] = 0;
#ifdef DEBUG
  v_function = -2;
  e(145);
  if (call_to_debug) {
    process_stoped = id;
    return;
  }
#endif
} else {
  stack[sp - 2] = (unsigned int)(memb[stack[sp - 2] * 4 + stack[sp - 1]] /= (byte)stack[sp]);
  sp -= 2;
}
break;
case lmoachr:
if (stack[sp] == 0) {
  memb[stack[sp - 2] * 4 + stack[sp - 1]] = 0;
  sp -= 2;
  stack[sp] = 0;
#ifdef DEBUG
  v_function = -2;
  e(145);
  if (call_to_debug) {
    process_stoped = id;
    return;
  }
#endif
} else {
  stack[sp - 2] = (unsigned int)(memb[stack[sp - 2] * 4 + stack[sp - 1]] %= (byte)stack[sp]);
  sp -= 2;
}
break;
case lanachr:
stack[sp - 2] = (unsigned int)(memb[stack[sp - 2] * 4 + stack[sp - 1]] &= (byte)stack[sp]);
sp -= 2;
break;
case lorachr:
stack[sp - 2] = (unsigned int)(memb[stack[sp - 2] * 4 + stack[sp - 1]] |= (byte)stack[sp]);
sp -= 2;
break;
case lxoachr:
stack[sp - 2] = (unsigned int)(memb[stack[sp - 2] * 4 + stack[sp - 1]] ^= (byte)stack[sp]);
sp -= 2;
break;
case lsrachr:
stack[sp - 2] = (unsigned int)(memb[stack[sp - 2] * 4 + stack[sp - 1]] >>= (byte)stack[sp]);
sp -= 2;
break;
case lslachr:
stack[sp - 2] = (unsigned int)(memb[stack[sp - 2] * 4 + stack[sp - 1]] <<= (byte)stack[sp]);
sp -= 2;
break;
case lcpachr:
if ((unsigned)stack[mem[id + _Param]] < 256) {
  memb[stack[sp - 1] * 4 + stack[sp]] = (byte)stack[mem[id + _Param]];
} else {
  if (stack[mem[id + _Param]] < imem_max + 258 * 4)
    memb[stack[sp - 1] * 4 + stack[sp]] = memb[stack[mem[id + _Param]] * 4];
  else
    memb[stack[sp - 1] * 4 + stack[sp]] = (byte)stack[mem[id + _Param]];
}
sp -= 2;
mem[id + _Param]++;
break;

// Un puntero a cadena menor a 256 se trata siempre como "%c"

case lstrcpy:
#ifdef DEBUG
if ((mem[stack[sp - 1] - 1] & 0xFFF00000) != 0xDAD00000) {
  sp--;
  v_function = -2;
  e(164);
  if (call_to_debug) {
    process_stoped = id;
    return;
  }
  break;
}
if ((unsigned)stack[sp] > 255)
  if ((mem[stack[sp - 1] - 1] & 0xFFFFF) + 1 < strlen((char *)&mem[stack[sp]])) {
    sp--;
    v_function = -2;
    e(140);
    if (call_to_debug) {
      process_stoped = id;
      return;
    }
    break;
  }
#endif
if ((unsigned)stack[sp] > 255)
  memmove((char *)&mem[stack[sp - 1]], (char *)&mem[stack[sp]],
          strlen((char *)&mem[stack[sp]]) + 1);
else {
  ((char *)&mem[stack[sp - 1]])[0] = (char)stack[sp];
  ((char *)&mem[stack[sp - 1]])[1] = 0;
}
sp--;
break;

case lstrfix:
if (stack[sp - 1] >= (oo = strlen((char *)&memb[stack[sp - 2] * 4]))) {
  for (; oo < stack[sp - 1]; oo++) {
    memb[stack[sp - 2] * 4 + oo] = ' ';
  }
  memb[stack[sp - 2] * 4 + oo + 1] = 0;
}
break;

case lstrcat:
#ifdef DEBUG
if ((mem[stack[sp - 1] - 1] & 0xFFF00000) != 0xDAD00000) {
  sp--;
  v_function = -2;
  e(164);
  if (call_to_debug) {
    process_stoped = id;
    return;
  }
  break;
}
if ((unsigned)stack[sp] > 255)
  oo = strlen((char *)&mem[stack[sp]]);
else
  oo = 1;
if ((mem[stack[sp - 1] - 1] & 0xFFFFF) + 1 < strlen((char *)&mem[stack[sp - 1]]) + oo) {
  sp--;
  v_function = -2;
  e(140);
  if (call_to_debug) {
    process_stoped = id;
    return;
  }
  break;
}
#endif
if ((unsigned)stack[sp] > 255) {
  char *dst = (char *)&mem[stack[sp - 1]];
  int dlen = strlen(dst);
  memmove(dst + dlen, (char *)&mem[stack[sp]], strlen((char *)&mem[stack[sp]]) + 1);
} else {
  char *dst = (char *)&mem[stack[sp - 1]];
  int dlen = strlen(dst);
  dst[dlen] = (char)stack[sp];
  dst[dlen + 1] = 0;
}
sp--;
break;

case lstradd: // Strcat "en el aire" (ojo, el aire tiene tambien 0xDAD00402)
#ifdef DEBUG
if ((unsigned)stack[sp - 1] > 255)
  oo = strlen((char *)&mem[stack[sp - 1]]);
else
  oo = 1;
if ((unsigned)stack[sp] > 255)
  oo += strlen((char *)&mem[stack[sp]]);
else
  oo += 1;
if (oo >= 1028) {
  sp--;
  v_function = -2;
  e(140);
  if (call_to_debug) {
    process_stoped = id;
    return;
  }
  break;
}
#endif
{
  char _tmp[1030];
  int _tlen = 0;
  if ((unsigned)stack[sp - 1] > 255) {
    _tlen = strlen((char *)&mem[stack[sp - 1]]);
    memcpy(_tmp, (char *)&mem[stack[sp - 1]], _tlen);
  } else {
    _tmp[0] = (char)stack[sp - 1];
    _tlen = 1;
  }
  if ((unsigned)stack[sp] > 255) {
    int slen = strlen((char *)&mem[stack[sp]]);
    memcpy(_tmp + _tlen, (char *)&mem[stack[sp]], slen + 1);
    _tlen += slen;
  } else {
    _tmp[_tlen++] = (char)stack[sp];
    _tmp[_tlen] = 0;
  }
  memcpy((char *)&mem[nullstring[nstring]], _tmp, _tlen + 1);
}
stack[--sp] = nullstring[nstring];
nstring = ((nstring + 1) & 3);
break;

case lstrdec: // cambio de tamaño "en el aire" (no da error, hace lo que puede)
oo = strlen((char *)&mem[stack[sp - 1]]);
if (oo < 1028) {
  memmove((char *)&mem[nullstring[nstring]], (char *)&mem[stack[sp - 1]],
          strlen((char *)&mem[stack[sp - 1]]) + 1);
  if (stack[sp] > 0) { // Quitar caracteres
    if (stack[sp] >= oo)
      memb[nullstring[nstring] * 4] = 0;
    else
      memb[nullstring[nstring] * 4 + oo - stack[sp]] = 0;
  } else if (stack[sp] < 0) {   // Añadir (?) caracteres (por homogeneidad)
    stack[sp] = oo - stack[sp]; // Nueva longitud
    if (stack[sp] > 1025)
      stack[sp] = 1025;
    for (; oo < stack[sp]; oo++) {
      memb[nullstring[nstring] * 4 + oo] = ' ';
    }
    memb[nullstring[nstring] * 4 + oo] = 0;
  }
} else
  mem[nullstring[nstring]] = 0;
stack[--sp] = nullstring[nstring];
nstring = ((nstring + 1) & 3);
break;

case lstrsub: // cambio de tamaño a un string
oo = strlen((char *)&mem[stack[sp - 1]]);
#ifdef DEBUG
if ((mem[stack[sp - 1] - 1] & 0xFFF00000) != 0xDAD00000) {
  sp--;
  v_function = -2;
  e(164);
  if (call_to_debug) {
    process_stoped = id;
    return;
  }
  break;
}
if ((mem[stack[sp - 1] - 1] & 0xFFFFF) + 1 < oo - stack[sp]) {
  sp--;
  v_function = -2;
  e(140);
  if (call_to_debug) {
    process_stoped = id;
    return;
  }
  break;
}
#endif
if (stack[sp] > 0) { // Quitar caracteres
  if (stack[sp] >= oo)
    memb[stack[sp - 1] * 4] = 0;
  else
    memb[stack[sp - 1] * 4 + oo - stack[sp]] = 0;
} else if (stack[sp] < 0) {   // Añadir (?) caracteres (por homogeneidad)
  stack[sp] = oo - stack[sp]; // Nueva longitud
  for (; oo < stack[sp]; oo++) {
    memb[stack[sp - 1] * 4 + oo] = ' ';
  }
  memb[stack[sp - 1] * 4 + oo] = 0;
}
sp--;
break;

case lstrlen:
if ((unsigned)stack[sp] > 255)
  stack[sp] = strlen((char *)&mem[stack[sp]]);
else
  stack[sp] = 1;
break;

case lstrigu:
if ((unsigned)stack[sp - 1] > 255) {
  if ((unsigned)stack[sp] > 255) {
    if (strcmp((char *)&mem[stack[sp - 1]], (char *)&mem[stack[sp]]))
      stack[sp - 1] = 0;
    else
      stack[sp - 1] = 1;
  } else {
    if (strcmp((char *)&mem[stack[sp - 1]], (char *)&stack[sp]))
      stack[sp - 1] = 0;
    else
      stack[sp - 1] = 1;
  }
} else {
  if ((unsigned)stack[sp] > 255) {
    if (strcmp((char *)&stack[sp - 1], (char *)&mem[stack[sp]]))
      stack[sp - 1] = 0;
    else
      stack[sp - 1] = 1;
  } else {
    stack[sp - 1] = (stack[sp - 1] == stack[sp]);
  }
}
sp--;
break;

case lstrdis:
if ((unsigned)stack[sp - 1] > 255) {
  if ((unsigned)stack[sp] > 255) {
    if (strcmp((char *)&mem[stack[sp - 1]], (char *)&mem[stack[sp]]))
      stack[sp - 1] = 1;
    else
      stack[sp - 1] = 0;
  } else {
    if (strcmp((char *)&mem[stack[sp - 1]], (char *)&stack[sp]))
      stack[sp - 1] = 1;
    else
      stack[sp - 1] = 0;
  }
} else {
  if ((unsigned)stack[sp] > 255) {
    if (strcmp((char *)&stack[sp - 1], (char *)&mem[stack[sp]]))
      stack[sp - 1] = 1;
    else
      stack[sp - 1] = 0;
  } else {
    stack[sp - 1] = (stack[sp - 1] != stack[sp]);
  }
}
sp--;
break;
break;

case lstrmay:
if ((unsigned)stack[sp - 1] > 255) {
  if ((unsigned)stack[sp] > 255) {
    if (strcmp((char *)&mem[stack[sp - 1]], (char *)&mem[stack[sp]]) > 0)
      stack[sp - 1] = 1;
    else
      stack[sp - 1] = 0;
  } else {
    if (strcmp((char *)&mem[stack[sp - 1]], (char *)&stack[sp]) > 0)
      stack[sp - 1] = 1;
    else
      stack[sp - 1] = 0;
  }
} else {
  if ((unsigned)stack[sp] > 255) {
    if (strcmp((char *)&stack[sp - 1], (char *)&mem[stack[sp]]) > 0)
      stack[sp - 1] = 1;
    else
      stack[sp - 1] = 0;
  } else {
    stack[sp - 1] = (stack[sp - 1] > stack[sp]);
  }
}
sp--;
break;
break;

case lstrmen:
if ((unsigned)stack[sp - 1] > 255) {
  if ((unsigned)stack[sp] > 255) {
    if (strcmp((char *)&mem[stack[sp - 1]], (char *)&mem[stack[sp]]) < 0)
      stack[sp - 1] = 1;
    else
      stack[sp - 1] = 0;
  } else {
    if (strcmp((char *)&mem[stack[sp - 1]], (char *)&stack[sp]) < 0)
      stack[sp - 1] = 1;
    else
      stack[sp - 1] = 0;
  }
} else {
  if ((unsigned)stack[sp] > 255) {
    if (strcmp((char *)&stack[sp - 1], (char *)&mem[stack[sp]]) < 0)
      stack[sp - 1] = 1;
    else
      stack[sp - 1] = 0;
  } else {
    stack[sp - 1] = (stack[sp - 1] < stack[sp]);
  }
}
sp--;
break;
break;

case lstrmei:
if ((unsigned)stack[sp - 1] > 255) {
  if ((unsigned)stack[sp] > 255) {
    if (strcmp((char *)&mem[stack[sp - 1]], (char *)&mem[stack[sp]]) <= 0)
      stack[sp - 1] = 1;
    else
      stack[sp - 1] = 0;
  } else {
    if (strcmp((char *)&mem[stack[sp - 1]], (char *)&stack[sp]) <= 0)
      stack[sp - 1] = 1;
    else
      stack[sp - 1] = 0;
  }
} else {
  if ((unsigned)stack[sp] > 255) {
    if (strcmp((char *)&stack[sp - 1], (char *)&mem[stack[sp]]) <= 0)
      stack[sp - 1] = 1;
    else
      stack[sp - 1] = 0;
  } else {
    stack[sp - 1] = (stack[sp - 1] <= stack[sp]);
  }
}
sp--;
break;
break;

case lstrmai:
if ((unsigned)stack[sp - 1] > 255) {
  if ((unsigned)stack[sp] > 255) {
    if (strcmp((char *)&mem[stack[sp - 1]], (char *)&mem[stack[sp]]) >= 0)
      stack[sp - 1] = 1;
    else
      stack[sp - 1] = 0;
  } else {
    if (strcmp((char *)&mem[stack[sp - 1]], (char *)&stack[sp]) >= 0)
      stack[sp - 1] = 1;
    else
      stack[sp - 1] = 0;
  }
} else {
  if ((unsigned)stack[sp] > 255) {
    if (strcmp((char *)&stack[sp - 1], (char *)&mem[stack[sp]]) >= 0)
      stack[sp - 1] = 1;
    else
      stack[sp - 1] = 0;
  } else {
    stack[sp - 1] = (stack[sp - 1] >= stack[sp]);
  }
}
sp--;
break;
break;

case lcpastr:
#ifdef DEBUG
if ((mem[stack[sp] - 1] & 0xFFF00000) != 0xDAD00000) {
  sp--;
  mem[id + _Param]++;
  v_function = -2;
  e(164);
  if (call_to_debug) {
    process_stoped = id;
    return;
  }
  break;
}
if ((unsigned)stack[mem[id + _Param]] > 255) {
  if ((mem[stack[sp] - 1] & 0xFFFFF) + 1 < strlen((char *)&mem[stack[mem[id + _Param]]])) {
    sp--;
    mem[id + _Param]++;
    v_function = -2;
    e(140);
    if (call_to_debug) {
      process_stoped = id;
      return;
    }
    break;
  }
}
#endif
if ((unsigned)stack[mem[id + _Param]] > 255)
  memmove((char *)&mem[stack[sp]], (char *)&mem[stack[mem[id + _Param]]],
          strlen((char *)&mem[stack[mem[id + _Param]]]) + 1);
else {
  ((char *)&mem[stack[sp]])[0] = (char)stack[mem[id + _Param]];
  ((char *)&mem[stack[sp]])[1] = 0;
}
sp--;
mem[id + _Param]++;
break;

case lptrwor:
stack[sp - 1] = (unsigned int)memw[stack[sp - 1] * 2 + stack[sp]];
sp--;
break;
case lasiwor:
stack[sp - 2] = (unsigned int)(memw[stack[sp - 2] * 2 + stack[sp - 1]] = (word)stack[sp]);
sp -= 2;
break;
case liptwor:
stack[sp - 1] = (unsigned int)++memw[stack[sp - 1] * 2 + stack[sp]];
sp--;
break;
case lptiwor:
stack[sp - 1] = (unsigned int)memw[stack[sp - 1] * 2 + stack[sp]]++;
sp--;
break;
case ldptwor:
stack[sp - 1] = (unsigned int)--memw[stack[sp - 1] * 2 + stack[sp]];
sp--;
break;
case lptdwor:
stack[sp - 1] = (unsigned int)memw[stack[sp - 1] * 2 + stack[sp]]--;
sp--;
break;
case ladawor:
stack[sp - 2] = (unsigned int)(memw[stack[sp - 2] * 2 + stack[sp - 1]] += (word)stack[sp]);
sp -= 2;
break;
case lsuawor:
stack[sp - 2] = (unsigned int)(memw[stack[sp - 2] * 2 + stack[sp - 1]] -= (word)stack[sp]);
sp -= 2;
break;
case lmuawor:
stack[sp - 2] = (unsigned int)(memw[stack[sp - 2] * 2 + stack[sp - 1]] *= (word)stack[sp]);
sp -= 2;
break;
case ldiawor:
if (stack[sp] == 0) {
  memw[stack[sp - 2] * 2 + stack[sp - 1]] = 0;
  sp -= 2;
  stack[sp] = 0;
#ifdef DEBUG
  v_function = -2;
  e(145);
  if (call_to_debug) {
    process_stoped = id;
    return;
  }
#endif
} else {
  stack[sp - 2] = (unsigned int)(memw[stack[sp - 2] * 2 + stack[sp - 1]] /= (word)stack[sp]);
  sp -= 2;
}
break;
case lmoawor:
if (stack[sp] == 0) {
  memw[stack[sp - 2] * 2 + stack[sp - 1]] = 0;
  sp -= 2;
  stack[sp] = 0;
#ifdef DEBUG
  v_function = -2;
  e(145);
  if (call_to_debug) {
    process_stoped = id;
    return;
  }
#endif
} else {
  stack[sp - 2] = (unsigned int)(memw[stack[sp - 2] * 2 + stack[sp - 1]] %= (word)stack[sp]);
  sp -= 2;
}
break;
case lanawor:
stack[sp - 2] = (unsigned int)(memw[stack[sp - 2] * 2 + stack[sp - 1]] &= (word)stack[sp]);
sp -= 2;
break;
case lorawor:
stack[sp - 2] = (unsigned int)(memw[stack[sp - 2] * 2 + stack[sp - 1]] |= (word)stack[sp]);
sp -= 2;
break;
case lxoawor:
stack[sp - 2] = (unsigned int)(memw[stack[sp - 2] * 2 + stack[sp - 1]] ^= (word)stack[sp]);
sp -= 2;
break;
case lsrawor:
stack[sp - 2] = (unsigned int)(memw[stack[sp - 2] * 2 + stack[sp - 1]] >>= (word)stack[sp]);
sp -= 2;
break;
case lslawor:
stack[sp - 2] = (unsigned int)(memw[stack[sp - 2] * 2 + stack[sp - 1]] <<= (word)stack[sp]);
sp -= 2;
break;
case lcpawor:
if ((unsigned)stack[mem[id + _Param]] < 256) {
  memw[stack[sp - 1] * 2 + stack[sp]] = (word)stack[mem[id + _Param]];
} else {
  if (stack[mem[id + _Param]] < imem_max + 258 * 4)
    memw[stack[sp - 1] * 2 + stack[sp]] = memw[stack[mem[id + _Param]] * 2];
  else
    memw[stack[sp - 1] * 2 + stack[sp]] = (word)stack[mem[id + _Param]];
}
sp -= 2;
mem[id + _Param]++;
break;

case lnul:
#ifdef DEBUG
if (!stack[sp]) {
  v_function = -2;
  e(165);
  if (call_to_debug) {
    process_stoped = id;
    return;
  }
}
#endif
break;

//-----------------------------------------------------------------------------
//  Cases no utilizados (hasta el 255)
//-----------------------------------------------------------------------------

case 127:
case 128:
case 129:
case 130:
case 131:
case 132:
case 133:
case 134:
case 135:
case 136:
case 137:
case 138:
case 139:
case 140:
case 141:
case 142:
case 143:
case 144:
case 145:
case 146:
case 147:
case 148:
case 149:
case 150:
case 151:
case 152:
case 153:
case 154:
case 155:
case 156:
case 157:
case 158:
case 159:
case 160:
case 161:
case 162:
case 163:
case 164:
case 165:
case 166:
case 167:
case 168:
case 169:
case 170:
case 171:
case 172:
case 173:
case 174:
case 175:
case 176:
case 177:
case 178:
case 179:
case 180:
case 181:
case 182:
case 183:
case 184:
case 185:
case 186:
case 187:
case 188:
case 189:
case 190:
case 191:
case 192:
case 193:
case 194:
case 195:
case 196:
case 197:
case 198:
case 199:
case 200:
case 201:
case 202:
case 203:
case 204:
case 205:
case 206:
case 207:
case 208:
case 209:
case 210:
case 211:
case 212:
case 213:
case 214:
case 215:
case 216:
case 217:
case 218:
case 219:
case 220:
case 221:
case 222:
case 223:
case 224:
case 225:
case 226:
case 227:
case 228:
case 229:
case 230:
case 231:
case 232:
case 233:
case 234:
case 235:
case 236:
case 237:
case 238:
case 239:
case 240:
case 241:
case 242:
case 243:
case 244:
case 245:
case 246:
case 247:
case 248:
case 249:
case 250:
case 251:
case 252:
case 253:
case 254:
case 255:
break;
