#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// Describes the metadata tracked for each synthetic file entry.
struct FileControlBlock {
    int fileId;
    std::string name;
    int sizeBytes;
    bool isOpen;
    int openCount;
    int ownerPid;

    FileControlBlock(int id, std::string fileName, int requestedSize, int pidOwner);
};

// Minimal in-memory directory that indexes files by name.
class RootDirectory {
public:
    RootDirectory();

    bool hasFile(const std::string &name) const;
    FileControlBlock *createFile(int fileId, const std::string &name, int sizeBytes, int ownerPid);
    FileControlBlock *getFile(const std::string &name);
    const FileControlBlock *getFile(const std::string &name) const;
    // Materializes a sorted snapshot so callers can iterate safely.
    std::vector<FileControlBlock> snapshot() const;

private:
    std::unordered_map<std::string, std::unique_ptr<FileControlBlock>> files;
};

// Provides creation/open/close bookkeeping for simulated files.
class FileManager {
public:
    FileManager();

    int createFile(const std::string &name, int sizeBytes, int ownerPid);
    int openFile(const std::string &name);
    void closeFile(int descriptor);

    // Prints persistent directory metadata for diagnostics.
    void printDirectory() const;
    // Lists every open descriptor mapped to its FCB.
    void printOpenFiles() const;

private:
    RootDirectory root;
    int nextFileId;
    int nextDescriptor;
    std::unordered_map<int, FileControlBlock *> descriptorTable;

    FileControlBlock *getFileByDescriptor(int descriptor);
};

#endif
