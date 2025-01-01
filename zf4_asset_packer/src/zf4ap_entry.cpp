#include "zf4ap.h"

int main(const int argCnt, const char* const* args) {
    if (argCnt != 3) {
        zf4_log_error("Invalid number of command-line arguments! Expected a source directory and an output directory.");
        return EXIT_FAILURE;
    }

    const char* const srcDir = args[1]; // The directory containing the assets to pack.
    const char* const outputDir = args[2]; // The directory to output the packed assets file to.

    AssetPacker packer = {};

    const bool packingSuccess = run_asset_packer(&packer, srcDir, outputDir);

    clean_asset_packer(&packer, packingSuccess);

    return packingSuccess ? EXIT_SUCCESS : EXIT_FAILURE;
}
