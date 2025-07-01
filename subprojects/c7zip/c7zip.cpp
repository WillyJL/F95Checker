#include "c7zip.h"

#include <7zip/CPP/Common/MyInitGuid.h>
#include <7zip/CPP/Common/StdOutStream.h>

#include <7zip/CPP/7zip/UI/Common/Extract.h>
#include <7zip/CPP/7zip/UI/Common/LoadCodecs.h>
#include <7zip/CPP/7zip/UI/Console/ExtractCallbackConsole.h>

CREATE_CODECS_OBJECT

CStdOutStream* g_StdStream = &g_StdOut;
CStdOutStream* g_ErrStream = &g_StdErr;

bool c7zip_extract(const char* archive, const char* output, m_string_ptr error) {
    if(codecs->Formats.IsEmpty()) {
        if(codecs->Load() != S_OK) {
            m_string_set_cstr(error, "Could not load 7-Zip codecs");
            return false;
        }
        Codecs_AddHashArcHandler(codecs);
    }

    CObjectVector<COpenType> types;
    CIntVector excluded_formats;
    NWildcard::CCensorNode wildcard_censor;
    wildcard_censor.Add_Wildcard();

    UStringVector archive_paths;
    UStringVector archive_paths_full;
    archive_paths.Add(UString(archive));
    archive_paths_full.Add(UString(archive));

    CExtractOptions options;
    options.YesToAll = true;
    options.OverwriteMode = NExtract::NOverwriteMode::kOverwrite;
    options.OverwriteMode_Force = true;
    options.OutputDir = FString(output);

    CExtractCallbackConsole* callback = new CExtractCallbackConsole;
    bool print_progress = false;
    callback->Init(g_StdStream, g_ErrStream, g_StdStream, !print_progress);
    callback->LogLevel = 0;

    IHashCalc* hash = NULL;
    UString error_message;
    CDecompressStat stat;

    int32_t res = Extract(
        codecs,
        types,
        excluded_formats,
        archive_paths,
        archive_paths_full,
        wildcard_censor,
        options,
        callback,
        callback,
        callback,
        hash,
        error_message,
        stat);
    callback->ClosePercents();

    if(res != S_OK || !error_message.IsEmpty()) {
        if(!error_message.IsEmpty()) {
            m_string_printf(error, "%ls", error_message.Ptr());
        } else {
            m_string_printf(error, "Unknown error : code=%d", res);
        }
        return false;
    }

    return true;
}
