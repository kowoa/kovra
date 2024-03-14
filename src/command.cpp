#include "command.hpp"
#include "device.hpp"

namespace kovra {
CommandEncoder::CommandEncoder(std::shared_ptr<Device> device)
    : device{device}, cmd{device->create_command_buffer()} {}
} // namespace kovra
