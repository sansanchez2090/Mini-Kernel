#include <FileManager.h>

#include <algorithm>
#include <iostream>
#include <utility>

FileControlBlock::FileControlBlock(int id, std::string fileName, int requestedSize, int pidOwner)
    : fileId(id), name(std::move(fileName)), sizeBytes(requestedSize), isOpen(false),
      openCount(0), ownerPid(pidOwner) {}

RootDirectory::RootDirectory() = default;

bool RootDirectory::hasFile(const std::string &name) const {
    return files.find(name) != files.end();
}

FileControlBlock *RootDirectory::createFile(int fileId, const std::string &name, int sizeBytes, int ownerPid) {
    // Reject duplicates so each filename remains unique.
    if (hasFile(name)) {
        return nullptr;
    }

    auto fcb = std::make_unique<FileControlBlock>(fileId, name, sizeBytes, ownerPid);
    FileControlBlock *ptr = fcb.get();
    files.emplace(name, std::move(fcb));
    return ptr;
}

FileControlBlock *RootDirectory::getFile(const std::string &name) {
    auto it = files.find(name);
    if (it == files.end()) {
        return nullptr;
    }
    return it->second.get();
}

const FileControlBlock *RootDirectory::getFile(const std::string &name) const {
    auto it = files.find(name);
    if (it == files.end()) {
        return nullptr;
    }
    return it->second.get();
}

std::vector<FileControlBlock> RootDirectory::snapshot() const {
    // Clone the directory into a vector so callers can iterate without locks.
    std::vector<FileControlBlock> mirror;
    mirror.reserve(files.size());
    for (const auto &entry : files) {
        mirror.push_back(*entry.second);
    }

    std::sort(mirror.begin(), mirror.end(), [](const FileControlBlock &lhs, const FileControlBlock &rhs) {
        return lhs.name < rhs.name;
    });

    return mirror;
}

// FileManager API -----------------------------------------------------------
FileManager::FileManager() : root(), nextFileId(1), nextDescriptor(100) {}

int FileManager::createFile(const std::string &name, int sizeBytes, int ownerPid) {
    // Allocate a new FCB and record ownership for auditing.
    FileControlBlock *fcb = root.createFile(nextFileId, name, sizeBytes, ownerPid);
    if (!fcb) {
        std::cerr << "[FileManager] El archivo '" << name << "' ya existe.\n";
        return -1;
    }

    std::cout << "[FileManager] Archivo creado: PID " << ownerPid << " -> '" << name << "' (" << sizeBytes
              << " bytes)." << std::endl;
    return nextFileId++;
}

int FileManager::openFile(const std::string &name) {
    // Finds the FCB and hands out a synthetic descriptor.
    FileControlBlock *fcb = root.getFile(name);
    if (!fcb) {
        std::cerr << "[FileManager] No se puede abrir '" << name << "' (no existe).\n";
        return -1;
    }

    int descriptor = nextDescriptor++;
    descriptorTable.emplace(descriptor, fcb);
    fcb->isOpen = true;
    fcb->openCount += 1;

    std::cout << "[FileManager] Archivo '" << name << "' abierto con descriptor " << descriptor << "." << std::endl;
    return descriptor;
}

void FileManager::closeFile(int descriptor) {
    // Refuses invalid descriptors to keep the table consistent.
    FileControlBlock *fcb = getFileByDescriptor(descriptor);
    if (!fcb) {
        std::cerr << "[FileManager] Descriptor " << descriptor << " invalido.\n";
        return;
    }

    descriptorTable.erase(descriptor);
    fcb->openCount = std::max(0, fcb->openCount - 1);
    if (fcb->openCount == 0) {
        fcb->isOpen = false;
    }

    std::cout << "[FileManager] Descriptor " << descriptor << " cerrado para archivo '" << fcb->name << "'."
              << std::endl;
}

void FileManager::printDirectory() const {
    // Provides a sorted listing of every file and its state.
    auto filesSnapshot = root.snapshot();

    std::cout << "\n=== Directorio Raiz ===" << std::endl;
    if (filesSnapshot.empty()) {
        std::cout << "(sin archivos)" << std::endl;
        return;
    }

    for (const auto &file : filesSnapshot) {
        std::cout << "- " << file.name << " | id=" << file.fileId << " | tamano=" << file.sizeBytes
                  << " bytes | propietario PID=" << file.ownerPid << " | estado="
                  << (file.isOpen ? "ABIERTO" : "CERRADO") << " | abiertos=" << file.openCount << std::endl;
    }
}

void FileManager::printOpenFiles() const {
    // Reports descriptor-to-file mappings for debugging leaks.
    std::cout << "\n=== Tabla de Archivos Abiertos ===" << std::endl;
    if (descriptorTable.empty()) {
        std::cout << "(sin descriptores abiertos)" << std::endl;
        return;
    }

    for (const auto &entry : descriptorTable) {
        const FileControlBlock *fcb = entry.second;
        std::cout << "descriptor=" << entry.first << " -> " << fcb->name << ", pid=" << fcb->ownerPid << std::endl;
    }
}

FileControlBlock *FileManager::getFileByDescriptor(int descriptor) {
    auto it = descriptorTable.find(descriptor);
    if (it == descriptorTable.end()) {
        return nullptr;
    }
    return it->second;
}
