import sys

outfile = "pypaxpro/config.sh"
def main(argv):
    libs = []
    include_dirs = []
    for item in argv:
        if item.find(".") != -1:
            libs.append(item)
        elif item.find("/") != -1:
            include_dirs.append(item)

    unique_paths = []
    link_lib = []
    for lib in libs:
        path = lib.rpartition("/")[0]
        if path not in unique_paths:
            unique_paths.append(path)
        link_arg = "-l" + lib.rpartition("/")[-1].rpartition("lib")[-1]
        link_arg = link_arg.rpartition(".")[0]
        link_lib.append(link_arg)

    with open(outfile, 'w') as out:
        out.write("LIB_PATH=")
        for path in unique_paths:
            out.write("{}:".format(path))
        out.write("\n")

        out.write("LIB_LINK=\"")
        for lib in link_lib:
            out.write("{} ".format(lib))
        out.write("\"\n")

        out.write("INC_ARG=\"")
        for inc in include_dirs:
            out.write("-I{} ".format(inc))
        out.write("\"\n")
    print("Configuration written to {}".format(outfile))




if __name__ == "__main__":
    main(sys.argv[1:])
