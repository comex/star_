void CleanAndInvalidateCPUDataCache(void *buffer, int bufferLen) {
    char *p = buffer;
    while(bufferLen > 0) {
        asm("mcr p15, 0, %0, c7, c10, 1" :: "r"(p));
        p += 0x10; // should be higher
        bufferLen -= 0x10;
    }
}
