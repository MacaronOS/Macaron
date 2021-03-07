#include "vfs.test.hpp"
#include "../../fs/vfs/vfs.hpp"
#include "../../monitor.hpp"
#include "../../algo/String.hpp"
#include "../../posix.hpp"

namespace kernel::tests {

bool TestVFS::test()
{
    auto& vfs = fs::VFS::the();

    // preapre a testing file
    auto fd_res = vfs.open("/ext2/test_file.txt", 11, O_CREAT);
    if (!fd_res) {
        FAILM("unexpected error after opening a testing file");
    }
    // clear a testing file
    auto truncate_res = vfs.truncate(fd_res.result(), 0);
    if (!truncate_res) {
        FAILM("unexpected error after trunctating a testing file");
    }

    String example_input = "example";

    // write some numbmer of characters in a tesing file
    auto write_res = vfs.write(fd_res.result(), example_input.cstr(), example_input.size());
    if (!write_res) {
        FAILM("unexpected error after writing to a testing file");
    }
    if (write_res.result() != example_input.size()) {
        FAILM("error: written incorrect number of characters to a testing file");
    }

    // reset offseet
    vfs.lseek(fd_res.result(), 0, SEEK_SET);

    // read those characters back to res_buffer, make sure they are correct
    char res_buffer[32];
    auto read_res = vfs.read(fd_res.result(), &res_buffer, example_input.size());
    if (!read_res) {
        FAILM("unexpected error after reading from a testing file")
    }
    if (read_res.result() != example_input.size()) {
        FAILM("error: read incorrect number of characters from a testing file");
    }
    res_buffer[example_input.size()] = '\0';

    if (!(example_input == String(res_buffer))) {
        FAILM("error: read incorrect characters from a testing file");
    }

    vfs.close(fd_res.result());

    return true;
}
}