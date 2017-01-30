

void checkIfVgaRamEmpty() {
    int i;
    int vgaram_empty = 1;
    for(i = 0; i < VGARAM_SIZE; i++)
        if(m.vgaRam[i])
            vgaram_empty = 0;
    log_debug("vgaram_empty : %s\n", vgaram_empty? "true" : "false");
}

void stackDump() {
    log_debug("is_little_endian()=%d\n",m.isLittle);
    log_debug("sizeof(dd)=%zu\n",sizeof(dd));
    log_debug("sizeof(dd *)=%zu\n",sizeof(dd *));
    log_debug("sizeof(dw)=%zu\n",sizeof(dw));
    log_debug("sizeof(db)=%zu\n",sizeof(db));
    log_debug("sizeof(mem)=%zu\n",sizeof(m));
    log_debug("eax: %x\n",m.eax);
    hexDump(&m.eax,sizeof(dd));
    log_debug("ebx: %x\n",m.ebx);
    log_debug("ecx: %x\n",m.ecx);
    log_debug("edx: %x\n",m.edx);
    log_debug("ebp: %x\n",m.ebp);
    log_debug("cs: %d -> %p\n",m.cs,(void *) realAddress(0,cs));
    log_debug("ds: %d -> %p\n",m.ds,(void *) realAddress(0,ds));
    log_debug("esi: %x\n",m.esi);
    log_debug("ds:esi %p\n",(void *) realAddress(m.esi,ds));
    log_debug("es: %d -> %p\n",m.es,(void *) realAddress(0,es));
    hexDump(&m.es,sizeof(dd));
    log_debug("edi: %x\n",m.edi);
    log_debug("es:edi %p\n",(void *) realAddress(m.edi,es));
    hexDump((void *) realAddress(m.edi,es),50);
    log_debug("fs: %d -> %p\n",m.fs,(void *) realAddress(0,fs));
    log_debug("gs: %d -> %p\n",m.gs,(void *) realAddress(0,gs));
    log_debug("adress heap: %p\n",(void *) &m.heap);
    log_debug("adress vgaRam: %p\n",(void *) &m.vgaRam);
    log_debug("first pixels vgaRam: %x\n",*m.vgaRam);
    log_debug("flags: ZF = %d\n",m.ZF);
    log_debug("top stack=%d\n",m.stackPointer);
    checkIfVgaRamEmpty();
}



// thanks to paxdiablo http://stackoverflow.com/users/14860/paxdiablo for the hexDump function
void hexDump (void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    log_debug ("hexDump %p:\n", addr);
    
    if (len == 0) {
        log_debug("  ZERO LENGTH\n");
        return;
    }
    if (len < 0) {
        log_debug("  NEGATIVE LENGTH: %i\n",len);
        return;
    }
    
    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).
        
        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                log_debug ("  %s\n", buff);
            
            // Output the offset.
            log_debug ("  %04x ", i);
        }
        
        // Now the hex code for the specific character.
        log_debug (" %02x", pc[i]);
        
        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }
    
    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        log_debug ("   ");
        i++;
    }

    // And print the final ASCII bit.
    log_debug ("  %s\n", buff);
}

void asm2C_OUT(int16_t address, int data) {
    static int indexPalette = 0;
    switch(address) {
        case 0x3c8:
            indexPalette=data;
            break;
        case 0x3c9:
            if (indexPalette<768) {
                m.vgaPalette[indexPalette]=data;
                indexPalette++;
            } else {
                log_error("error: indexPalette>767 %d\n",indexPalette);
            }
            break;
        default:
            log_error("unknown OUT %d,%d\n",address, data);
            break;
    }
}

int8_t asm2C_IN(int16_t address) {
    static _Bool vblTick = 1;
    switch(address) {
        case 0x3DA:
            if (vblTick) {
                vblTick = 0;
                return 0;
            } else {
                vblTick = 1;
                m.jumpToBackGround = 1;
                return 8;
            }
            break;
        default:
            log_error("Unknown IN %d\n",address);
            return 0;
    }
}

/**
 * is_little_endian:
 *
 * Checks if the system is little endian or big-endian.
 *
 * Returns: greater than 0 if little-endian,
 * otherwise big-endian.
 **/
_Bool is_little_endian()
{
#if defined(__x86_64) || defined(__i386) || defined(_M_IX86) || defined(_M_X64)
    return 1;
#elif defined(MSB_FIRST)
    return 0;
#else
    union
    {
        uint16_t x;
        uint8_t y[2];
    } u;
    
    u.x = 1;
    return u.y[0];
#endif
}

void asm2C_init() {
    m.isLittle=is_little_endian();
    log_debug("asm2C_init is_little_endian:%d\n",m.isLittle);
}

void asm2C_INT(int a) {
    static FILE * file;
    int i;
    db ah=READDBh(eax);
    db al=READDBl(eax);
    dw ax=READDW(eax);
    dw bx=READDW(ebx);
    dw cx=READDW(ecx);
    dw dx=READDW(edx);
    m.CF = 0;
    log_debug("asm2C_INT ah=%x al=%x ax=%x bx=%x cx=%x dx=%x\n",ah,al,ax,bx,cx,dx);
    
    switch(a) {
        case 0x10:
        {
            switch(ax)
            {
                case 0x03: {
                    log_debug("Switch to text mode\n");
                    return;
                }
                case 0x13: {
                    log_debug("Switch to VGA\n");
                    stackDump();
                    return;
                }
            }
            break;
        }
        case 0x21:
            switch(ah)
        {
            case 0x9:
            {
                char * s=(char *) realAddress(m.edx,ds);
                for (i=0;s[i]!='$';i++) {
                    printf("%c", s[i]);
                }
                return;
            }
            case 0x2c:
            {
                //MOV(8,8,READDBh(edx),(db)2);
                // TOFIX
                m.edx=0x200;
                return;
            }
            case 0x3d:
            {
                char fileName[1000];
                if (m.path!=NULL) {
                    sprintf(fileName,"%s/%s",m.path,(const char *) realAddress(m.edx, ds));
                } else {
                    sprintf(fileName,"%s",(const char *) realAddress(m.edx, ds));
                }
                file=fopen(fileName, "rb"); //TOFIX, multiple files support
                log_debug("Opening file %s -> %p\n",fileName,(void *) file);
                if (file!=NULL) {
                    m.eax=1; //TOFIX
                } else {
                    m.CF = 1;
                    log_error("Error opening file %s\n",fileName);
                }
                /*
                // [Index]AH = 3Dh - "OPEN" - OPEN EXISTING FILE
            Entry:

                AL = access and sharing modes
            DS:DX -> ASCIZ filename
            Return:

                CF clear if successful, AX = file handle
                    CF set on error AX = error code (01h,02h,03h,04h,05h,0Ch,56h)
                    */
                // TODO
                return;
            }
            case 0x3e:
            {
              // bx: file handle to close
              //TOFIX
              log_debug("Closing file. bx:%d\n",bx);
              if (fclose(file))  {
                    m.CF = 1;
                      perror("Error");
                      log_error("Error closing file ? bx:%d %p\n",bx,(void *) file);
                }

                file=NULL;
              return;
            }
            case 0x3f:
            {
                /*
                [Index]AH = 3Fh - "READ" - READ FROM FILE OR DEVICE

            Entry:

                BX = file handle
                CX = number of bytes to read
            DS:DX -> buffer for data
                Return:

                CF clear if successful - AX = number of bytes actually read (0 if at EOF before call)
                    CF set on error AX = error code (05h,06h)
                    */
                //char grosbuff[100000];
                void * buffer=(db *) realAddress(m.edx, ds);
                //buffer=grosbuff;
                log_debug("Reading ecx=%d cx=%d eds=%x edx=%x -> %p file: %p\n",m.ecx,cx,m.ds,m.edx,buffer,(void *)  file);

                if (feof(file)) {
                    log_debug("feof(file)\n");
                  m.eax=0;
                } else {
                    size_t r=fread (buffer,1,cx,file);
                  if (r!=cx) {
                      perror("Error");
                      log_error("r!=cx cx:%d R:%zu \n",cx,r);
                    if(!feof(file)) {
                      log_error("Error reading ? %d %zu %p\n",cx,r,(void *) file);
                      m.CF = 1;
                    }
                  } else {
                      log_debug("Reading OK %p\n",(void *) file);
                  }
                  m.eax=r;
                }
                /*
                if (ax!=cx) {
                    log_debug("Error reading ? %d %d\n",ax,cx);
                    m.CF = 1;

                }
                 */
                return;
            }
                // [Index]AH=42h - "LSEEK" - SET CURRENT FILE POSITION
            case 0x42:
            {
                /*

                 AH=42h - "LSEEK" - SET CURRENT FILE POSITION

                 Entry:

                 AL = origin of move 00h start of file 01h current file position 02h end of file
                 BX = file handle
                 CX:DX = offset from origin of new file position

                 */
                int seek = 0;
                switch(ah) {
                    case 0x0:
                        seek = SEEK_SET;
                        break;
                    case 0x1:
                        seek = SEEK_CUR;
                        break;
                    case 0x2:
                        seek = SEEK_END;
                        break;
                }
                long int offset=(cx<<16)+dx;
                log_debug("Seeking to offset %ld %d\n",offset,seek);
                if (fseek(file,offset,seek)!=0) {
                    log_error("Error seeking\n");
                }
                return;
            }

            case 0x4c:
            {
                stackDump();
                m.jumpToBackGround = 1;
                m.executionFinished = 1;
                m.exitCode = al;
                return;
            }
            default:
                break;
        }
        case 0x31:
            switch(ax)
        {
            case 0x0:
            {
                /*
                ;2.0 - Function 0000h - Allocate Descriptors:
                ;--------------------------------------------
                ;  Allocates one or more descriptors in the client's descriptor table. The
                ;descriptor(s) allocated must be initialized by the application with other
                ;function calls.
                ;In:
                ;  AX     = 0000h
                ;  CX     = number of descriptors to allocate
                ;Out:
                ;  if successful:
                ;    carry flag clear
                ;    AX     = base selector
                 */
                log_debug("Function 0000h - Allocate %d Descriptors\n",cx);
                if (m.selectorsPointer+cx>=NB_SELECTORS) {
                    m.CF = 1;
                    log_error("Not enough free selectors (increase NB_SELECTORS)\n");
                    return;
                } else {
                    m.eax = m.selectorsPointer;
                    m.selectorsPointer+=cx;
                    log_debug("Return %x\n",m.eax);
                }
                return;
            }
            case 0x02:
            {
                /*
                This function Converts a real mode segment into a protected mode descriptor.
                 BX =    real mode segment
                 Out:
                 if successful:
                 carry flag clear
                 AX =  selector
                 if failed:
                 carry flag set
                */
                log_debug("Function 0002h - Converts a real mode segment into a protected mode descriptor real mode segment: %d\n",m.ebx);
                if (m.selectorsPointer+1>=NB_SELECTORS) {
                    m.CF = 1;
                    log_error("Not enough free selectors (increase NB_SELECTORS)\n");
                    return;
                }
                // TOFIX ?
                // always return vga adress.
                m.selectors[m.selectorsPointer]=offsetof(struct Mem,vgaRam); // bx;
                m.eax=m.selectorsPointer;
                log_debug("Returns new selector: eax: %d\n",m.eax);
                m.selectorsPointer++;

                // Multiple calls for the same real mode segment return the same selector. The returned descriptor should never be modified or freed. <- TOFIX
                return;
            }
                /*
                ;2.5 - Function 0007h - Set Segment Base Address:
                ; Sets the 32bit linear base address field in the descriptor for the specified
                ; segment.
                ; In:   AX     = 0007h
                ; BX     = selector
                ;  CX:DX  = 32bit linear base address of segment
                */
            case 0x07:
            {
                log_debug("Function 0007h - Set Segment Base Address: ebx: %x, edx:%x ecx:%x\n",m.ebx,m.edx,m.ecx);
                if (bx>m.selectorsPointer) {
                    m.CF = 1;
                    log_error("Error: selector number doesnt exist\n");
                    return;
                }
                m.selectors[bx]=(READDW(edx)&0xffff)+(READDW(ecx)<<16);
                log_debug("Address for selector %d: %x\n",bx,m.selectors[bx]);
                return;
            }
            case 0x08:
            {
                /*
                ;2.6 - Function 0008h - Set Segment Limit:
                ;-----------------------------------------
                ;  Sets the limit field in the descriptor for the specified segment.
                ;  In:
                ;  AX     = 0008h
                ;  BX     = selector
                ;  CX:DX  = 32bit segment limit
                ;  Out:
                ;  if successful:
                ;    carry flag clear
                ;  if failed:
                ;    carry flag set
                 */

                // To implement...
                log_debug("Function 0008h - Set Segment Limit for selector %d (Ignored)\n",bx);
                return;
            }
            case 0x501:
            {
                /*
                ;2.29 - Function 0501h - Allocate Memory Block:
                ;In:  AX     = 0501h
                ;  BX:CX  = size of block in bytes (must be non-zero)
                ;Out: if successful:
                ;    carry flag clear
                ;    BX:CX  = linear address of allocated memory block
                ;    SI:DI  = memory block handle (used to resize and free block)
                */
                int32_t nbBlocks=(bx<<16)+cx;
                log_debug("Function 0501h - Allocate Memory Block: %d bytes\n",nbBlocks);

                if (m.heapPointer+nbBlocks>=HEAP_SIZE) {
                    m.CF = 1;
                    log_error("Not enough memory (increase HEAP_SIZE)\n");
                    return;
                } else {
                    dd a=offsetof(struct Mem,heap)+m.heapPointer;
                    m.heapPointer+=nbBlocks;
                    {
                        dd n=offsetof(struct Mem,heap)+m.heapPointer;
                        log_debug("New top of heap: %x\n",n);
                    }
                    m.ecx=a & 0xFFFF;
                    m.ebx=a >> 16;
                    m.edi=0; // TOFIX
                    m.esi=0; // TOFIX
                    log_debug("Return %x ebx:ecx %x:%x\n",a,m.ebx,m.ecx);
                    return;
                }
                break;
            }
            case 0x205:{
                /*
                 fo implement
                 ;2.18 - Function 0204h - Get Protected Mode Interrupt Vector:
                 ;------------------------------------------------------------
                 ;
                 ;  Returns the address of the current protected mode interrupt handler for the
                 ;specified interrupt.
                 ;
                 ;In:
                 ;  AX     = 0204h
                 ;  BL     = interrupt number
                 ;
                 ;Out:
                 ;  always successful:
                 ;    carry flag clear
                 ;    CX:EDX = selector:offset of protected mode interrupt handler

                 ;  AX     = 0204h
                 ;  BL     = interrupt number
                 ;
                 ;Out:
                 ;  always successful:
                 ;    carry flag clear
                 ;    CX:EDX = selector:offset of protected mode interrupt handler
                 */

                return;
            }
            default:
                break;
        }
            break;

        default:
            break;
    }
    m.CF = 1;
    log_error("Error DOSInt 0x%x ah:0x%x al:0x%x: not supported.\n",a,ah,al);
}
