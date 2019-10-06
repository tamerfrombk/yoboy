#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <string>

#include "common.h"
#include "cartridge.h"

static void print_help()
{
    std::puts("yoBoy -- The GameBoy emulator.");

    std::puts("Usage: yoBoy '/path/to/rom.gb' [-h]");
    std::putchar('\n');

    std::puts("Optional arguments:");
    std::puts("-h            show this help message and exit.");
    std::putchar('\n');
}

static size_t fsize(std::FILE *file)
{
    size_t curr = std::ftell(file);
    std::fseek(file, 0, SEEK_END);

    size_t size = std::ftell(file);

    std::fseek(file, curr, SEEK_SET);

    return size;
}

static yb::Cartridge read_cartridge(const char* path)
{
    std::FILE *file = std::fopen(path, "rb");
    if (!file) {
        return std::vector<std::uint8_t>{};
    }

    size_t fileSize = fsize(file);

    std::vector<std::uint8_t> mem(fileSize, 0);

    size_t bytesRead = std::fread(mem.data(), sizeof(uint8_t), fileSize, file);
    if (bytesRead != fileSize) {
        std::fclose(file);

        return std::vector<std::uint8_t>{};
    }

    std::fclose(file);

    return mem;
}

struct Args {
    std::string cartridge_path;
    bool print_help;
};

static Args parse_args(int argc, char **argv)
{
    Args args;
    if (argc < 2) {
        return args;
    }

    if (std::strcmp("-h", argv[1]) == 0) {
        args.print_help = true;
        return args;
    }

    args.cartridge_path = argv[1];
    args.print_help = false;

    for (int i = 2; i < argc;) {
        if (std::strcmp(argv[i], "-h") == 0) {
            args.print_help = true;
            ++i;
        }
        else {
            yb::exit("Unrecognized argument %s.\n", argv[i]);
        }
    }

    return args;
}


int main(int argc, char **argv)
{
    Args args = parse_args(argc, argv);
    if (args.print_help) {
        print_help();
        return 0;
    }

    if (args.cartridge_path.empty()) {
        yb::exit("The GameBoy ROM file was not supplied.\n");
    }

    auto cartridge = read_cartridge(args.cartridge_path.c_str());
    if (cartridge.empty()) {
        yb::exit("Could not read %s.\n", args.cartridge_path.c_str());
    }
}
