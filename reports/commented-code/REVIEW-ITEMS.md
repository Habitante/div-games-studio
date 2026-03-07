# Review Items -- Commented Code Audit

25 blocks across 14 files that need human decision before removal.

---

## Group A: `judascfg_device == DEV_NOSOUND` shims (7 blocks)

Mike replaced Judas sound library device checks with hardcoded `false`/`true` across
multiple files. Audio-dependent error paths are now unreachable. All need replacing
with an `OSDEP_AudioAvailable()` check or equivalent SDL2_mixer guard.

### 1. divhandl.c:1110-1114 -- Sound recording device check (case 1)
**Category:** MIKE_REMOVED
**Summary:** `judascfg_device != DEV_SBPRO/SB16` replaced with `false`, making the "wrong sound card" error dialog unreachable. The recording path proceeds unconditionally.
**Current code:**
```c
        } else if( false /*judascfg_device != DEV_SBPRO &&
                   judascfg_device != DEV_SB16 */ ) {
          v_texto=(char *)texto[575]; dialogo(err0);
          return;
        }
```
**Decision needed:** Remove the dead `false` branch entirely, or replace with SDL2_mixer capability check for recording support.

---

### 2. divhandl.c:1119-1122 -- Sound buffer allocation check (case 1)
**Category:** MIKE_REMOVED
**Summary:** `aligned[0]!=NULL && aligned[1]!=NULL` replaced with `true`, and `judas_stopsample(0)` / `sbmalloc()` / `sbfree()` commented out. The else-branch (out-of-memory error) is now unreachable.
**Current code:**
```c
//        judas_stopsample(0);
//        sbmalloc();

        if(true /*aligned[0]!=NULL && aligned[1]!=NULL */)
			dialogo(RecSound0);
        else { v_texto=(char *)texto[45];dialogo(err0); }
//        sbfree();
```
**Decision needed:** Remove the `true` shim and dead else-branch. The sound recording system needs a real SDL2_mixer implementation or an honest "not implemented" message.

---

### 3. divhandl.c:1192-1202 -- Song open device check (case 8)
**Category:** MIKE_REMOVED
**Summary:** `judascfg_device == DEV_NOSOUND` replaced with `false`, making the "no sound card" error dialog for song loading unreachable.
**Current code:**
```c
      case 8:
        if ( false /*judascfg_device == DEV_NOSOUND*/ ) {
          if ( SoundError ) {
            v_texto=(char *)texto[549]; dialogo(errhlp0);
            if (v_aceptar) help(2008);
          } else {
            v_texto=(char *)texto[548]; dialogo(errhlp0);
            if (v_aceptar) help(2009);
          }
          return;
        }

        OpenSong();
```
**Decision needed:** Remove the dead `false` branch, or replace with SDL2_mixer audio-available check so users get a proper error when audio init failed.

---

### 4. divbrow.c:1521 -- Browser sound preview device check
**Category:** MIKE_REMOVED
**Summary:** `judascfg_device == DEV_NOSOUND` replaced with `false` in the file browser. The sound preview button is now always enabled even when audio is unavailable.
**Current code:**
```c
  if(v_thumb==7 && false /*judascfg_device == DEV_NOSOUND*/) opc_pru=0;
```
**Decision needed:** Replace `false` with SDL2_mixer audio-available check so the preview button is disabled when audio init failed.

---

### 5. divmixer.c:168 -- `set_mixer()` no-sound guard
**Category:** MIKE_REMOVED
**Summary:** Early-return guard `if(judascfg_device==DEV_NOSOUND) return;` commented out. The mixer now runs unconditionally.
**Current code:**
```c
void set_mixer(void) {
  int  fx, cd, ma;

//  if(judascfg_device==DEV_NOSOUND) return;

  fx=Setupfile.vol_fx; if(Setupfile.mut_fx) fx=0;
  cd=Setupfile.vol_cd; if(Setupfile.mut_cd) cd=0;
  ma=Setupfile.vol_ma; if(Setupfile.mut_ma) ma=0;
  InitMixer(0, 0, ma, fx, cd);
}
```
**Decision needed:** Safe to remove -- `InitMixer()` calls `Mix_VolumeMusic`/`Mix_Volume` which are no-ops if audio is not open. Delete the commented line.

---

### 6. divmixer.c:182-196 -- `set_init_mixer()` no-sound guard + hardware mixer query
**Category:** MIKE_REMOVED
**Summary:** Both the `DEV_NOSOUND` guard and the SB Pro/SB16 hardware mixer query path (MIX_GetVolume to read hardware levels) are commented out. The `else` branch using saved Setupfile values is correct for SDL2.
**Current code:**
```c
void set_init_mixer(void) {
  int  fx, cd, ma;
  byte fx_l, fx_r;
  byte cd_l, cd_r;
  byte ma_l, ma_r;

//  if(judascfg_device==DEV_NOSOUND) return;

/*  if( judascfg_device == DEV_SBPRO ||
      judascfg_device == DEV_SB16  )
  {
    MIX_GetVolume(MIX_VOICE_VOL,  &fx_l, &fx_r);
    MIX_GetVolume(MIX_CD_VOL,     &cd_l, &cd_r);
    MIX_GetVolume(MIX_MASTER_VOL, &ma_l, &ma_r);
    fx=Setupfile.vol_fx=((fx_l>fx_r) ? fx_l : fx_r); if (Setupfile.mut_fx) fx=0;
    cd=Setupfile.vol_cd=((cd_l>cd_r) ? cd_l : cd_r); if (Setupfile.mut_cd) cd=0;
    ma=Setupfile.vol_ma=((ma_l>ma_r) ? ma_l : ma_r); if (Setupfile.mut_ma) ma=0;
    InitMixer(judascfg_device, judascfg_port, ma, fx, cd);
  }
  else
  */
  {
    fx=Setupfile.vol_fx; if(Setupfile.mut_fx) fx=0;
    cd=Setupfile.vol_cd; if(Setupfile.mut_cd) cd=0;
    ma=Setupfile.vol_ma; if(Setupfile.mut_ma) ma=0;
    InitMixer(0, 0, ma, fx, cd);
  }
}
```
**Decision needed:** Remove the commented guard, the entire `/* if(judascfg_device...) */` block, and the now-unused variables `fx_l`, `fx_r`, `cd_l`, `cd_r`, `ma_l`, `ma_r`.

---

### 7. divdsktp.c:514-516 -- Desktop song restore device check
**Category:** MIKE_REMOVED
**Summary:** `judascfg_device != DEV_NOSOUND` guard around `OpenDesktopSong()` commented out. Song restore now happens unconditionally (which is fine for SDL2, but the function body is stubbed -- see item 12).
**Current code:**
```c
                        case    107: //mod
                                fread(SongName,1,14,desktop);
                                fread(SongPathName,1,256,desktop);
                                if ((f=fopen(SongPathName,"rb"))!=NULL) {
                                  fclose(f);
                                 //if ( judascfg_device != DEV_NOSOUND ) {
                                    OpenDesktopSong();
                                 // }
                                }
                                break;
```
**Decision needed:** Remove the commented guard lines. The real problem is that `OpenDesktopSong()` is stubbed (see item 12).

---

## Group B: PCM session save/load disabled (2 blocks)

### 8. divdsktp.c:181-202 -- PCM session save (case 105 in DownLoad_Desktop)
**Category:** MIKE_ADDED_DEAD (the `#ifdef NOTYET` wrapper)
**Summary:** Entire case 105 for saving PCM sound windows to the desktop session file is wrapped in `#ifdef NOTYET`. PCM windows are silently skipped during session save.
**Current code:**
```c
#ifdef NOTYET
                                 case    105: //pcm
                                        mypcminfo=(pcminfo *)ventana[x].aux;
                                        SaveDesktopSound(mypcminfo, desktop);
/*
                                        if(mypcminfo->pathname[0]!=0) // OJO !!!
                                        {
                                          n=fwrite(mypcminfo->name,1,14,desktop);
                                          n=fwrite(mypcminfo->pathname,1,256,desktop);
                                        }
*/
/*
                                        n=fwrite(mypcminfo, 1, sizeof(pcminfo), desktop);
                                        n=fwrite(mypcminfo->SoundData, 2, mypcminfo->SoundSize, desktop);
*/
										free(mypcminfo->SoundData);
										Mix_FreeChunk(mypcminfo->SI);
                                        break;
#endif
```
**Decision needed:** Port `SaveDesktopSound` to SDL2_mixer or accept that PCM session persistence is broken. Low priority since desktop save/restore is rarely used.

---

### 9. divdsktp.c:488-507 -- PCM session load (case 105 in UpLoad_Desktop)
**Category:** MIKE_ADDED_DEAD (the `#ifdef NOTYET` wrapper)
**Summary:** Entire case 105 for loading PCM sound windows from the desktop session file is wrapped in `#ifdef NOTYET`. PCM windows are silently skipped during session restore.
**Current code:**
```c
#ifdef NOTYET
                        case    105: //pcm
/*
                                fread(SoundName,1,14,desktop);
                                fread(SoundPathName,1,256,desktop);
                                if ((f=fopen(SoundPathName,"rb"))!=NULL) {
                                  fclose(f);
                                  OpenDesktopSound();
                                }
*/
/*
                                pcminfo_aux=(char *)malloc(sizeof(pcminfo));
                                mypcminfo=(pcminfo *)pcminfo_aux;
                                mypcminfo->SoundData=(short *)malloc(mypcminfo->SoundSize*2);
                                fread(mypcminfo, 1, sizeof(pcminfo), desktop);
                                fread(mypcminfo->SoundData, 2, mypcminfo->SoundSize, desktop);
*/
                                OpenDesktopSound(desktop);
                                break;
#endif
```
**Decision needed:** Same as item 8. If session save is ported, this must be ported to match. If session save stays broken, remove both blocks.

---

## Group C: Stubbed sound editor functions (3 blocks)

### 10. divpcm.c:7 -- Missing `#include <io.h>`
**Category:** MIKE_REMOVED
**Summary:** `#include <io.h>` was active in DOS (provides `read()` used by `JudasProgressRead()`). On POSIX/MinGW, `<unistd.h>` provides it instead.
**Current code:**
```c
//#include <io.h>
```
**Decision needed:** Add a conditional include (`#ifdef _WIN32` / `#include <io.h>` / `#else` / `#include <unistd.h>` / `#endif`) or verify that `read()` is already provided by another header in the build.

---

### 11. divpcm.c:539-545 and 598-605 -- Missing `free(pcminfo_aux)` in error paths
**Category:** MIKE_ADDED_DEAD
**Summary:** In both `OpenSound()` and `OpenSoundFile()`, when `Mix_LoadWAV` and `DIVMIX_LoadPCM` both fail, `pcminfo_aux` is allocated but never freed. The `free(pcminfo_aux)` calls were commented out by Mike.
**Current code (OpenSound, line 539-545):**
```c
  if(SI==NULL) {

//		 free(pcminfo_aux);
        //if(SI) free(SI);
        v_texto=(char *)texto[46];
        dialogo(err0);
        continue;
   }
```
**Current code (OpenSoundFile, line 598-606):**
```c
  if(SI==NULL) {

//     free(pcminfo_aux);
        //if(SI) free(SI);
      v_texto=(char *)texto[46];
      dialogo(err0);
      free(mypcminfo);
      return;
   }
```
**Decision needed:** Restore `free(pcminfo_aux)` in the `OpenSound()` error path (line 541). Note that `OpenSoundFile()` already has `free(mypcminfo)` on line 604 which frees the same pointer (since `mypcminfo=(pcminfo *)pcminfo_aux`), so its commented `free` is correctly removed. Only the `OpenSound()` path leaks.

---

### 12. divpcm.c:880-919 -- `OpenDesktopSong()` entirely stubbed
**Category:** MIKE_REMOVED (wrapped in `#ifdef NOTYET`)
**Summary:** The entire function body that loaded songs via `judas_loadxm/loads3m/loadmod` is inside `#ifdef NOTYET`. The function does nothing except print a debug message. Desktop song restore is broken.
**Current code:**
```c
void OpenDesktopSong(void) {
debugprintf("TODO - divpcm.cpp OpenDesktopSong\n");

#ifdef NOTYET
  modinfo *mymodinfo;

  if((modinfo_aux=(char *)malloc(sizeof(modinfo)))==NULL)
  {
    v_texto=(char *)texto[45];
    dialogo(err0);
    return;
  }
  mymodinfo=(modinfo *)modinfo_aux;

  if(judas_channel[0].smp) judas_stopsample(0);

  judas_loadxm(SongPathName);
  if(judas_error != JUDAS_OK && judas_error == JUDAS_WRONG_FORMAT)
  {
    judas_loads3m(SongPathName);
    if(judas_error != JUDAS_OK && judas_error == JUDAS_WRONG_FORMAT)
    {
      judas_loadmod(SongPathName);
    }
  }
  if(judas_error != JUDAS_OK)
  {
    free(modinfo_aux);
    v_texto=(char *)texto[46];
    dialogo(err0);
    return;
  }

  memcpy(mymodinfo->name,SongName,14);
  memcpy(mymodinfo->pathname,SongPathName,256);

  nueva_ventana_carga((int)MOD0,ventana_aux.x,ventana_aux.y);
#endif

}
```
**Decision needed:** Port to SDL2_mixer using `Mix_LoadMUS()` (as `PlaySong()` already does), or accept that desktop song restore is broken. Low priority.

---

### 13. divpcm.c:1632-1701 -- `ChangeSoundFreq()` entirely stubbed
**Category:** MIKE_REMOVED (wrapped in `#ifdef NOTYET`)
**Summary:** Sound editor frequency conversion buttons (11025/22050/44100 Hz) do nothing. The resampling algorithm is intact inside the NOTYET block but depends on `judas_loadwav_mem` which no longer exists.
**Current code:**
```c
void ChangeSoundFreq(int freq)
{
	printf("TODO - divpcm.cpp ChangeSoundFreq\n");
#ifdef NOTYET
  pcminfo   *mypcminfo=(pcminfo *)pcminfo_aux;
  pcminfo   pcminfo_bak;
  byte      *FileBuffer;
  SoundInfo *SI=NULL;
  short     *short_ptr;
  float     paso, pos_f;
  int       length = mypcminfo->SoundSize;
  int       pos;

  if(mypcminfo->SoundData == NULL || mypcminfo->SoundFreq == freq) return;

  paso   = (float)mypcminfo->SoundFreq/(float)freq;
  pos_f  = (float)length/paso;
  length = (int)(pos_f+0.5);
  pos_f  = 0;
  if((short_ptr=(short *)malloc(length*2))==NULL)
  { ... }
  for(pos=0; pos<length; pos++)
  {
    short_ptr[pos]=mypcminfo->SoundData[(int)pos_f];
    pos_f+=paso;
  }

  pcminfo_bak.SoundFreq = freq;
  pcminfo_bak.SoundSize = length;
  pcminfo_bak.SoundData = short_ptr;

  if( (FileBuffer=SaveSoundMem(&pcminfo_bak))==NULL ) { ... }

  SI = judas_loadwav_mem(FileBuffer);
  ...
#endif
```
**Decision needed:** The resampling math is portable. The Judas-specific part (`judas_loadwav_mem`, `judas_freesample`) needs replacing with SDL2_mixer equivalents (`Mix_LoadWAV_RW` from an `SDL_RWFromMem`). Medium priority for sound editor functionality.

---

### 14. divpcm.c:1960-2022 -- `PasteNewSounds()` entirely stubbed
**Category:** MIKE_REMOVED (wrapped in `#ifdef NOTYET`)
**Summary:** Paste-from-sound-editor-to-desktop is broken. Creates PCM windows from the `DesktopSND[]` clipboard array using `judas_loadwav_mem`.
**Current code:**
```c
void PasteNewSounds(void)
{
	printf("TODO - divpcm.cpp PasteNewSounds\n");

#ifdef NOTYET
  pcminfo   *mypcminfo;
  byte      *FileBuffer;
  SoundInfo *SI=NULL;
  int       con;

  for(con=0; con<NumSND; con++)
  {
    if((pcminfo_aux=(char *)malloc(sizeof(pcminfo)))==NULL)
    { ... continue; }
    mypcminfo=(pcminfo *)pcminfo_aux;

    mypcminfo->SoundFreq = DesktopSND[con].SoundFreq;
    mypcminfo->SoundSize = DesktopSND[con].SoundSize;
    mypcminfo->SoundData = DesktopSND[con].SoundData;

    if( (FileBuffer=SaveSoundMem(mypcminfo))==NULL ) { ... continue; }

    SI = judas_loadwav_mem(FileBuffer);
    ...
    nueva_ventana((int)PCM0);
  }
#endif

}
```
**Decision needed:** Same pattern as item 13. Replace `judas_loadwav_mem` with `Mix_LoadWAV_RW(SDL_RWFromMem(...))`. Low priority.

---

## Group D: Runtime semantics changes (3 blocks)

### 15. runtime/f.c:2108-2113 -- `stop_sound()` lost "stop all channels" branch
**Category:** MIKE_REMOVED
**Summary:** Original DOS `stop_sound()` stopped all channels when `pila[sp]==-1` (looping from InitChannel to CHANNELS). Mike simplified it to always call `StopSound(pila[sp]-1)`, which means `stop_sound(-1)` in DIV programs now calls `StopSound(-2)` -- an out-of-bounds error.
**Current code:**
```c
void stop_sound(void) {
#ifdef MIXER
  int x;
//  if(pila[sp]==-1) {
//    for(x=0; x<CHANNELS; x++) StopSound(x);
//  } else {

    StopSound(pila[sp]-1);
//  }
#endif
  pila[sp]=0;
}
```
**Decision needed:** Restore the `pila[sp]==-1` check. This is part of the DIV language API -- user programs call `stop_sound(-1)` to stop all channels. Without the check, this is a runtime bug.

---

### 16. runtime/f.c:2508-2516 -- `fade_off()` changed from blocking to non-blocking
**Category:** MIKE_REMOVED
**Summary:** Original `fade_off()` was a blocking call that looped `while (color != target) { set_paleta(); set_dac(); }` until the fade completed. Mike converted it to non-blocking by commenting out the loop body and just setting `fading=1`. This changes semantics for any DIV program relying on `fade_off()` completing before the next statement.
**Current code:**
```c
void fade_off(void) {
  dacout_r=64; dacout_g=64; dacout_b=64; dacout_speed=8;
  if (now_dacout_r!=dacout_r || now_dacout_g!=dacout_g || now_dacout_b!=dacout_b)
  //{
  //  set_paleta(); set_dac(); //LoopSound();
  //}
  fading=1;
  pila[++sp]=0;
}
```
**Decision needed:** Is non-blocking fade intentional for SDL2? The fading system already handles gradual fade in `frame_end()`, so non-blocking may actually be correct for the SDL2 event-driven model. However, the original `while` loop is now an `if` statement, which means it only sets `fading=1` once and returns immediately. If DIV programs expect `fade_off()` to block, this changes behavior. Decide: keep non-blocking (SDL2 style) or restore blocking with `while` + frame pumping.

---

### 17. runtime/i.c:644 -- `free()` in `carga_pila()` is NOT a memory leak
**Category:** MIKE_ADDED_DEAD
**Summary:** The commented-out `free(stack[mem[id+_SP]])` was flagged in MEMORY.md as a memory leak. It is actually correct to leave it commented out -- `p` holds the same pointer and is freed on line 648. Uncommenting would cause a double-free.
**Current code:**
```c
void carga_pila(int id) {
  int n;
  int32_t * p;
  if (mem[id+_SP]) {
    p=stack[mem[id+_SP]];

    for (n=0;n<=p[1]-p[0];n++)
    	pila[p[0]+n]=p[n+2];

//    free(stack[mem[id+_SP]]);
    stack[mem[id+_SP]]=0;
    mem[id+_SP]=0;
    sp=p[1];
    free(p);
  } else sp=0;
}
```
**Decision needed:** Remove the commented `free()` line. Also update MEMORY.md to close the "runtime/i.c:778: free() commented out -- memory leak" landmine entry -- the line number is wrong (it's 644, not 778) and there is no leak. Mike correctly avoided a double-free.

---

## Group E: Video / display incomplete porting (3 blocks)

### 18. div.c:732 -- `test_video` dialog disabled
**Category:** MIKE_REMOVED
**Summary:** The startup video test dialog `if (test_video) dialogo(test0)` was active in DOS when launching with the `TEST` argument. Mike commented it out.
**Current code:**
```c
//  if (test_video) dialogo(test0);
```
**Decision needed:** Either restore the test dialog for debugging video setup, or remove `test_video` and the `TEST` argument handling entirely (div.c line 419-420).

---

### 19. div.c:1693 -- `check_free()` disk space warning disabled
**Category:** MIKE_REMOVED
**Summary:** The startup disk space check was active in DOS. Mike commented it out. The function body exists and works (Win32 `GetDiskFreeSpaceEx` implementation).
**Current code:**
```c
//  check_free();
```
**Decision needed:** Restore the call to warn users about low disk space, or remove both the call and the function body if disk space warnings are unwanted.

---

### 20. osd_sdl2.c:112-117 -- Video mode resize silently ignored
**Category:** INCOMPLETE_PORT
**Summary:** When `OSDEP_SetVideoMode()` is called and a window already exists, nothing happens. The commented-out code shows two approaches: destroy/recreate or resize.
**Current code:**
```c
    if(OSDEP_window != NULL) {
//	    SDL_DestroyRenderer(OSDEP_renderer);
//    	SDL_DestroyWindow(OSDEP_window);
    	// if window smaller that vibile allowed, resize
    	// SDL_SetWindowSize(OSDEP_window,
     //                   width, height);

    } else {
    	SDL_CreateWindowAndRenderer(width, height, SDL_WINDOW_RESIZABLE, &OSDEP_window, &OSDEP_renderer);
```
**Decision needed:** Implement window resizing (via `SDL_SetWindowSize` + texture recreation) for resolution changes after initial window creation, or document that resolution switching is not supported.

---

## Group F: Path resolution stubs (2 blocks)

### 21. shared/osdep.c:231 -- `_fullpath()` uses `strcpy` instead of `realpath`
**Category:** INCOMPLETE_PORT
**Summary:** `realpath()` is commented out and replaced with `strcpy()`. The function does no actual path resolution on Linux/macOS.
**Current code:**
```c
char *_fullpath(char *_FullPath,const char *_Path,size_t _SizeInBytes) {
//	realpath(_Path, _FullPath);
strcpy(_FullPath, _Path);
	return _FullPath;
}
```
**Decision needed:** Restore `realpath()` on Linux/macOS (it is available on POSIX). For Windows, this file may not even be compiled (osdepwin.c is used instead) -- verify.

---

### 22. win/osdepwin.c:233 -- `_fullpath()` uses `strcpy` instead of `realpath`
**Category:** INCOMPLETE_PORT
**Summary:** Same issue as item 21 but in the Windows platform layer. On MinGW32, `_fullpath()` is actually available as a CRT function, so this entire wrapper may be unnecessary.
**Current code:**
```c
char *_fullpath(char *_FullPath,const char *_Path,size_t _SizeInBytes) {
//	realpath(_Path, _FullPath);
strcpy(_FullPath, _Path);
	return _FullPath;
}
```
**Decision needed:** On MinGW32, check if the CRT `_fullpath()` is linked instead of this wrapper (it may be guarded by `#if !defined(__MINGW32__)`). If this wrapper is compiled, use `_fullpath` from `<stdlib.h>` (MinGW CRT) or `GetFullPathNameA` (Win32 API) instead of `strcpy`.

---

## Group G: Input -- spacebar-as-mouse-button (1 logical item, 2 blocks)

### 23. divmouse.c:76-83 and 137-144 -- Spacebar-as-mouse-click for paint editor
**Category:** MIKE_REMOVED
**Summary:** Original DOS code mapped Spacebar to pseudo mouse button `0xfffd` when in paint editor keyboard-cursor mode. This enabled drawing with keyboard arrows + spacebar. Mike commented it out in both the physical-mouse branch (line 76) and the keyboard-movement branch (line 137).
**Current code (line 76-83, physical mouse path):**
```c
		if (modo<100 && hotkey && !help_paint_active) {
/*			if (key(_SPC)) {
				if (mouse_b!=0xfffd) {
					mouse_b=0xfffd;
				}
			} else if (mouse_b==0xfffd) {
				mouse_b=0;
			}
			* */
		}
```
**Current code (line 137-144, keyboard cursor path):**
```c
/*			if (key(_SPC)) {
				if (mouse_b!=0xfffd) {
					mouse_b=0xfffd;
				}
			} else if (mouse_b==0xfffd) {
				mouse_b=0;
			}
*/
```
**Decision needed:** Restore spacebar-as-mouse-button for keyboard-driven paint editing. Likely commented out because it interfered with SDL2 text input -- may need to only activate when the paint editor is focused (not during text input).

---

## Group H: Wallpaper JPEG support (1 block)

### 24. divpalet.c:1355 -- JPEG desktop wallpaper decompression disabled
**Category:** MIKE_REMOVED
**Summary:** `case 4: descomprime_JPG()` was active in DOS. The `es_JPG()` check (line 1349) still detects JPEGs and sets `x=4`, but the decompression case is commented out. Selecting a JPG as desktop wallpaper silently fails.
**Current code:**
```c
  switch (x) {
    case 1: descomprime_MAP(temp2,temp,0); break;
    case 2: descomprime_PCX(temp2,temp,0); break;
    case 3: descomprime_BMP(temp2,temp,0); break;
//    case 4: descomprime_JPG(temp2,temp,0,lon); break;
  } swap(map_an,tap_an); swap(map_al,tap_al);
```
**Decision needed:** If JPEG support is restored elsewhere (divforma.c has `#ifdef JPGLIB` stubs), uncomment this line. Otherwise, remove the `es_JPG()` detection too so users do not see false "success" when selecting a JPG wallpaper.

---

## Group I: DOS shell / command_path (1 block)

### 25. div.c:1713-1729 -- `command_path()` body and `shell()` function
**Category:** MIKE_REMOVED
**Summary:** The entire `command_path()` body (finds `command.com` via COMSPEC/PATH) was commented out and replaced with `return NULL`. The `shell()` function calls `command_path()`, gets NULL, and shows an error. Both functions are dead-end DOS code.
**Current code:**
```c
char *command_path() {
/*    static char fullpath[128];
    char *commandpath;
    int i;

    if (commandpath=getenv("COMSPEC")) {
         strlwr(strcpy(fullpath,commandpath));
        if (strstr(fullpath,".com")) return(&fullpath);
    }

    for(i=0;i<sizeof(paths_to_check)/sizeof(paths_to_check[0]);i++) {
    _searchenv("command.com",paths_to_check[i],fullpath);
    if(fullpath[0]) return(&fullpath);
    }    return(NULL);
    */
    return NULL;
}
```
**Decision needed:** Remove both `command_path()` and `shell()` entirely. DOS Shell is already hidden from the menu (lenguaje.div). The Alt+S shortcut is already disabled (div.c line 1671). There is no use for a DOS shell in the SDL2 port.

---

## Group J: Installer (non-critical, 2 blocks noted for completeness)

### (Not numbered -- low priority) divinsta.c:476-516 -- Installer thumbnail rendering disabled
**Category:** MIKE_REMOVED
**Summary:** The entire 48-line thumbnail creation body of `crear_imagen_install()` is commented out. The installer module is entirely non-functional on SDL2 anyway.
**Decision needed:** The entire installer module (`divinsta.c`) is DOS-only dead code. Consider removing the whole file in Phase 1 rather than fixing individual functions.

---

## Summary by priority

| Priority | Items | Description |
|----------|-------|-------------|
| **HIGH** | 15 | `stop_sound(-1)` runtime bug -- breaks DIV programs |
| **HIGH** | 11 | Memory leak in `OpenSound()` error path |
| **HIGH** | 17 | Close false "memory leak" landmine in MEMORY.md |
| **MEDIUM** | 1-4, 7 | Replace `false`/`true` shims with real audio check |
| **MEDIUM** | 16 | `fade_off()` blocking semantics change |
| **MEDIUM** | 20 | Video mode resize silently ignored |
| **MEDIUM** | 13 | Sound editor frequency conversion broken |
| **LOW** | 5, 6 | Mixer guards (safe to just delete) |
| **LOW** | 8, 9, 12, 14 | Desktop session PCM/song persistence broken |
| **LOW** | 10 | Missing `<io.h>` / `<unistd.h>` include |
| **LOW** | 18, 19 | test_video dialog and check_free() |
| **LOW** | 21, 22 | `_fullpath` path resolution stubs |
| **LOW** | 23 | Spacebar-as-mouse-button for paint editor |
| **LOW** | 24 | JPEG wallpaper support |
| **LOW** | 25 | Dead `command_path()`/`shell()` functions |
