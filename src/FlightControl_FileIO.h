/* date = January 5th 2021 8:02 pm */

#ifndef FLIGHTCONTROL_FILEIO_H
#define FLIGHTCONTROL_FILEIO_H

u32 
GetFileSize(readonly u8 *path) 
{
    FILE *File;
    File = fopen(path, "r");
    ASSERT(File);
    
    fseek(File, 0L, SEEK_END);
    u32 size_bytes = ftell(File);
    
    return size_bytes;
}


#endif //FLIGHTCONTROL_FILEIO_H
