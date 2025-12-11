#ifndef IOMANAGER_H
#define IOMANAGER_H

#include <deque>
#include <string>
#include <vector>

// Supported device categories for the simulated IO subsystem.
enum class DeviceType {
    TECLADO,
    DISCO,
    RED
};

// Captures the static properties and runtime status of each device.
struct DeviceDescriptor {
    int id;
    std::string name;
    DeviceType type;
    int latency;
    bool busy;
    int currentPid;

    DeviceDescriptor(int devId, std::string devName, DeviceType devType, int baseLatency);
};

// Represents a queued or executing IO request tied to a PID.
struct IORequest {
    int requestId;
    int pid;
    int deviceId;
    std::string description;
    int remainingTime;
};

// Event emitted once a request finishes so the scheduler can react.
struct IOEvent {
    int pid;
    int deviceId;
    std::string description;
};

// Manages IO devices, queues, and completion notifications.
class IOManager {
public:
    IOManager();

    // Initiates an IO request for a process on the specified device.
    int requestIO(int pid, DeviceType device, const std::string &description);
    // Advances device timers and potentially completes requests.
    void tick();
    // Returns the batch of events produced since the last tick.
    std::vector<IOEvent> fetchCompletedEvents();
    void printDeviceTable() const;
    void printPendingRequests() const;

private:
    int nextDeviceId;
    int nextRequestId;
    std::vector<DeviceDescriptor> devices;
    std::vector<IORequest> inFlight;
    std::deque<IORequest> pendingQueue;
    std::vector<IOEvent> completedEvents;

    DeviceDescriptor *findDevice(DeviceType type);
    DeviceDescriptor *getDeviceById(int id);
    void startPendingRequests();
    void registerDefaultDevices();
};

#endif
