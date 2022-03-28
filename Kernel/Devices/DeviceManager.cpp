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
    register_device(mouse);
    register_device(keyboard);
    register_device(ptmx);
    

    // Register PCI devices
    auto& pci = Drivers::PCI::the();
    for (size_t device_index = 0; device_index < pci.devices().size(); device_index++) {
        auto device = pci.devices()[device_index];
        switch (device->vendor_id()) {
        case 0x1234: {
            switch (device->device_id()) {
            case 0x1111: {
                bga.set_pci_device(device);
                register_device(bga);
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