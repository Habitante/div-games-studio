#ifdef __DOS__

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void *allocate_dos_mem(uint Tam) {
  union REGS r;

  //---------------------------------------------------------------------------
  // Allocate Tam / 16 paragraphs
  // WARNING: The PMODEW.DOC document specifies the parameters
  // of function 100h incorrectly.
  // To call a real-mode interrupt, we must call
  // interrupt 0x31
  //---------------------------------------------------------------------------

  SetMem(&r, sizeof(r), 0);

  r.x.eax = 0x0100;          // DOS Allocate Memory Block function
  r.x.ebx = (Tam + 15) >> 4; // Paragraphs to allocate
  int386(0x31, &r, &r);

  // Check for error

  if (r.x.cflag)
    return ((uint)_NULL);

  // Return pointer to the allocated memory area, but first we must
  // convert the real-mode address to protected mode by
  // multiplying by 16

  return (void *)((r.x.eax & 0xFFFF) << 4);
}

#endif
