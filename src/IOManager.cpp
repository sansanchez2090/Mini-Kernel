#include "../include/IOManager.h"

#include <algorithm>
#include <iostream>

// Provides the textual name for each device type when logging.
namespace {
std::string deviceTypeToString(DeviceType type) {
    switch (type) {
        case DeviceType::TECLADO:
            return "TECLADO";
        case DeviceType::DISCO:
            return "DISCO";
        case DeviceType::RED:
            return "RED";
        default:
            return "DESCONOCIDO";
    }
}
}

DeviceDescriptor::DeviceDescriptor(int devId, std::string devName, DeviceType devType, int baseLatency)
    : id(devId), name(std::move(devName)), type(devType), latency(baseLatency), busy(false), currentPid(-1) {}

IOManager::IOManager()
    : nextDeviceId(1), nextRequestId(1) {
    // Prepare a default fleet so processes can immediately request IO.
    registerDefaultDevices();
}

void IOManager::registerDefaultDevices() {
    // Seeds representative keyboard/disk/network devices with baseline latencies.
    devices.emplace_back(nextDeviceId++, "Teclado PS2", DeviceType::TECLADO, 2);
    devices.emplace_back(nextDeviceId++, "Disco SATA", DeviceType::DISCO, 5);
    devices.emplace_back(nextDeviceId++, "NIC 1Gb", DeviceType::RED, 3);
}

DeviceDescriptor *IOManager::findDevice(DeviceType type) {
    // Performs a linear scan (device list is tiny) to find the matching type.
    for (auto &device : devices) {
        if (device.type == type) {
            return &device;
        }
    }
    return nullptr;
}

DeviceDescriptor *IOManager::getDeviceById(int id) {
    // Helps map request.deviceId back to its descriptor when freeing it.
    for (auto &device : devices) {
        if (device.id == id) {
            return &device;
        }
    }
    return nullptr;
}

int IOManager::requestIO(int pid, DeviceType deviceType, const std::string &description) {
    // Construct a new request capturing PID, device, description and latency budget.
    DeviceDescriptor *device = findDevice(deviceType);
    if (!device) {
        std::cerr << "[IO] No existe un dispositivo para el tipo solicitado\n";
        return -1;
    }

    IORequest request;
    request.requestId = nextRequestId++;
    request.pid = pid;
    request.deviceId = device->id;
    request.description = description;
    request.remainingTime = device->latency;

    if (device->busy) {
        // Queue the request until the device becomes available.
        pendingQueue.push_back(request);
        std::cout << "[IO] PID=" << pid << " en cola de espera para dispositivo " << device->name << "\n";
    } else {
        // Start immediately and mark the device as busy for this PID.
        device->busy = true;
        device->currentPid = pid;
        inFlight.push_back(request);
        std::cout << "[IO] PID=" << pid << " usando " << device->name << " (req " << request.requestId << ")\n";
    }

    return request.requestId;
}

void IOManager::tick() {
    // Advance every in-flight request by one time unit.
    if (!inFlight.empty()) {
        std::vector<IORequest> remaining;
        remaining.reserve(inFlight.size());

        for (auto &request : inFlight) {
            request.remainingTime--;

            if (request.remainingTime <= 0) {
                // Release the device and push a completion event for the scheduler.
                DeviceDescriptor *device = getDeviceById(request.deviceId);
                if (device) {
                    device->busy = false;
                    device->currentPid = -1;
                }
                completedEvents.push_back({request.pid, request.deviceId, request.description});
                std::cout << "[IO] PID=" << request.pid << " completo " << request.description << "\n";
            } else {
                // Keep tracking the request if it still needs time.
                remaining.push_back(request);
            }
        }

        inFlight.swap(remaining);
    }

    // Once active requests are updated, attempt to start queued ones.
    startPendingRequests();
}

void IOManager::startPendingRequests() {
    // Promote waiting requests onto any device that just became idle.
    if (pendingQueue.empty()) {
        return;
    }

    for (auto &device : devices) {
        if (device.busy) {
            continue;
        }

        for (auto it = pendingQueue.begin(); it != pendingQueue.end(); ++it) {
            if (it->deviceId == device.id) {
                device.busy = true;
                device.currentPid = it->pid;
                inFlight.push_back(*it);
                std::cout << "[IO] PID=" << it->pid << " inicia " << it->description
                          << " en dispositivo " << device.name << "\n";
                pendingQueue.erase(it);
                break;
            }
        }
    }
}

std::vector<IOEvent> IOManager::fetchCompletedEvents() {
    // Returns the accumulated completion events and clears the buffer.
    std::vector<IOEvent> ready = completedEvents;
    completedEvents.clear();
    return ready;
}

void IOManager::printDeviceTable() const {
    // Dump each device with its type, latency and busy flag for diagnostics.
    std::cout << "\n=== Tabla de Dispositivos ===\n";
    for (const auto &device : devices) {
        std::cout << "#" << device.id << " " << device.name
                  << " | tipo=" << deviceTypeToString(device.type)
                  << " | latencia=" << device.latency
                  << " | estado=" << (device.busy ? "OCUPADO" : "LIBRE");

        if (device.busy) {
            std::cout << " | pid=" << device.currentPid;
        }
        std::cout << "\n";
    }
}

void IOManager::printPendingRequests() const {
    // Report both executing and queued requests to observe pipeline pressure.
    std::cout << "\n=== Cola de E/S Pendiente ===\n";
    if (pendingQueue.empty() && inFlight.empty()) {
        std::cout << "(sin solicitudes de E/S)\n";
        return;
    }

    if (!inFlight.empty()) {
        std::cout << "-- En curso --\n";
        for (const auto &request : inFlight) {
            std::cout << "req=" << request.requestId << " pid=" << request.pid
                      << " dev=" << request.deviceId
                      << " restantes=" << request.remainingTime << "\n";
        }
    }

    if (!pendingQueue.empty()) {
        std::cout << "-- En espera --\n";
        for (const auto &request : pendingQueue) {
            std::cout << "req=" << request.requestId << " pid=" << request.pid
                      << " dev=" << request.deviceId << "\n";
        }
    }
}
