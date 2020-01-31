#ifndef LPM_FILE_H
#define LPM_FILE_H

#include "lpm_structs.h"
#include "lpm_error.h"

#include <stdint.h>
#include <stdbool.h>

struct LPM_File;

typedef struct LPM_FileFxns
{
    void (*write)(struct LPM_File * f, const LPM_Buf * buf, size_t offset);
    void (*read) (struct LPM_File * f, LPM_Buf * buf, size_t offset);
    void (*clear)(struct LPM_File * f);
} LPM_FileFxns;

typedef struct LPM_File
{
    const LPM_FileFxns * fxns;
    size_t maxSize;
    error_t error;
} LPM_File;


inline void LPM_File_write(LPM_File * f, const LPM_Buf * buf, size_t offset)
{
    (*(f->fxns->write))(f, buf, offset);
}

inline void LPM_File_read(LPM_File * f, LPM_Buf * buf, size_t offset)
{
    (*(f->fxns->read))(f, buf, offset);
}

inline void LPM_File_clear(LPM_File * f)
{
    (*(f->fxns->clear))(f);
}

inline bool LPM_File_errorOccured(LPM_File * f)
{
    return LPM_ErrorOccured(f->error);
}

inline size_t LPM_File_maxSize(LPM_File * f)
{
    return f->maxSize;
}

#endif // LPM_FILE_H
