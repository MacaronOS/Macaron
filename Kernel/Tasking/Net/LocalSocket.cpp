#include "LocalSocket.hpp"

namespace Kernel::Net {

HashMap<String, LocalSocket*> LocalSocket::s_endpoint_to_socket;

}