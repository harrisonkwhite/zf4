#include "zf4ap.h"

int main(const int arg_cnt, const char* const* args) {
    if (arg_cnt != 3) {
        LogError("Invalid number of command-line arguments! Expected a source directory and an output directory.");
        return EXIT_FAILURE;
    }

    const char* const src_dir = args[1]; // The directory containing the assets to pack.
    const char* const output_dir = args[2]; // The directory to output the packed assets file to.

    s_asset_packer packer = {0};

    const bool packing_success = RunAssetPacker(&packer, src_dir, output_dir);

    CleanAssetPacker(&packer, packing_success);

    return packing_success ? EXIT_SUCCESS : EXIT_FAILURE;
}
