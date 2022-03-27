#include "DeviceManager.hpp"
#include "Graphics/BochVBE.hpp"
#include "IO/Keyboard.hpp"
#include "IO/Mouse.hpp"
#include "PTY/PTMX.hpp"
#include <Drivers/PCI/PCI.hpp>
#include <Libkernel/Logger.hpp>

namespace Kernel::Devices {

void DeviceManager::register_initial_devices()
{
    // Register PS/2 devices
    register_device(new Mouse);
    register_device(new Keyboard);
    register_device(new PTMX);

    // Register PCI devices
    auto& pci = Drivers::PCI::the();
    for (size_t device_index = 0; device_index < pci.devices().size(); device_index++) {
        auto device = pci.devices()[device_index];
        switch (device->vendor_id()) {
        case 0x1234: {
            switch (device->device_id()) {
            case 0x1111: {
                register_device(new BochVBE(device));
                break;
            }
            }
            break;
        }
        default:
            break;
        }
    }
}

}