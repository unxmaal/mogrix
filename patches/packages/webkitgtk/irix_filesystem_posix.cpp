/* IRIX POSIX filesystem implementations for WebKitGTK
 *
 * GCC 9 libstdc++ has <filesystem> headers but no libstdc++fs.a for IRIX.
 * This provides POSIX implementations for all std::filesystem-dependent
 * functions in WTF::FileSystemImpl.
 *
 * #included in FileSystem.cpp as an #else block when HAVE(STD_FILESYSTEM) is 0.
 * Already inside namespace WTF::FileSystemImpl.
 */

#include <sys/statvfs.h>
#include <dirent.h>
#include <utime.h>
#include <ftw.h>
#include <cstring>

/* --- Functions also in FILESYSTEM_POSIX_FAST_PATH (FileSystemPOSIX.cpp) --- */

bool fileExists(const String& path)
{
    return access(fileSystemRepresentation(path).data(), F_OK) != -1;
}

bool deleteFile(const String& path)
{
    bool unlinked = !unlink(fileSystemRepresentation(path).data());
    if (!unlinked && errno != ENOENT)
        LOG_ERROR("File failed to delete. Error message: %s", safeStrerror(errno).data());
    return unlinked;
}

bool makeAllDirectories(const String& path)
{
    auto fullPath = fileSystemRepresentation(path);
    int length = fullPath.length();
    if (!length)
        return false;

    if (!access(fullPath.data(), F_OK))
        return true;

    char* p = fullPath.mutableData() + 1;
    if (p[length - 1] == '/')
        p[length - 1] = '\0';
    for (; *p; ++p) {
        if (*p == '/') {
            *p = '\0';
            if (access(fullPath.data(), F_OK)) {
                if (mkdir(fullPath.data(), S_IRWXU))
                    return false;
            }
            *p = '/';
        }
    }
    if (access(fullPath.data(), F_OK)) {
        if (mkdir(fullPath.data(), S_IRWXU))
            return false;
    }

    return true;
}

String pathByAppendingComponent(StringView path, StringView component)
{
    if (path.endsWith('/'))
        return makeString(path, component);
    return makeString(path, '/', component);
}

String pathByAppendingComponents(StringView path, const Vector<StringView>& components)
{
    StringBuilder builder;
    builder.append(path);
    bool isFirstComponent = true;
    for (auto& component : components) {
        if (isFirstComponent) {
            isFirstComponent = false;
            if (path.endsWith('/')) {
                builder.append(component);
                continue;
            }
        }
        builder.append('/', component);
    }
    return builder.toString();
}

/* --- Functions that only had std::filesystem implementations --- */

bool deleteEmptyDirectory(const String& path)
{
    auto fsRep = fileSystemRepresentation(path);
    if (fsRep.isNull())
        return false;
    return rmdir(fsRep.data()) == 0;
}

static int nftwRemoveCallback(const char* fpath, const struct stat*, int, struct FTW*)
{
    return remove(fpath);
}

bool deleteNonEmptyDirectory(const String& path)
{
    auto fsRep = fileSystemRepresentation(path);
    if (fsRep.isNull())
        return false;
    return nftw(fsRep.data(), nftwRemoveCallback, 64, FTW_DEPTH | FTW_PHYS) == 0;
}

bool moveFile(const String& oldPath, const String& newPath)
{
    auto fsOldPath = fileSystemRepresentation(oldPath);
    auto fsNewPath = fileSystemRepresentation(newPath);
    if (fsOldPath.isNull() || fsNewPath.isNull())
        return false;

    if (rename(fsOldPath.data(), fsNewPath.data()) == 0)
        return true;

    // Fall back to copy + delete for cross-device moves
    if (errno == EXDEV) {
        if (!hardLinkOrCopyFile(oldPath, newPath))
            return false;
        unlink(fsOldPath.data());
        return true;
    }
    return false;
}

std::optional<uint64_t> fileSize(const String& path)
{
    auto fsRep = fileSystemRepresentation(path);
    if (fsRep.isNull())
        return std::nullopt;
    struct stat st;
    if (stat(fsRep.data(), &st) != 0)
        return std::nullopt;
    return static_cast<uint64_t>(st.st_size);
}

std::optional<uint64_t> directorySize(const String& path)
{
    auto fsRep = fileSystemRepresentation(path);
    if (fsRep.isNull())
        return std::nullopt;

    struct stat st;
    if (stat(fsRep.data(), &st) != 0 || !S_ISDIR(st.st_mode))
        return std::nullopt;

    uint64_t total = 0;
    std::function<bool(const char*)> walk = [&](const char* dirPath) -> bool {
        DIR* d = opendir(dirPath);
        if (!d)
            return false;
        struct dirent* entry;
        while ((entry = readdir(d)) != nullptr) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            char childPath[PATH_MAX];
            snprintf(childPath, sizeof(childPath), "%s/%s", dirPath, entry->d_name);
            struct stat childSt;
            if (lstat(childPath, &childSt) != 0) {
                closedir(d);
                return false;
            }
            if (S_ISREG(childSt.st_mode))
                total += childSt.st_size;
            else if (S_ISDIR(childSt.st_mode)) {
                if (!walk(childPath)) {
                    closedir(d);
                    return false;
                }
            }
        }
        closedir(d);
        return true;
    };
    if (!walk(fsRep.data()))
        return std::nullopt;
    return total;
}

std::optional<uint64_t> volumeFreeSpace(const String& path)
{
    auto fsRep = fileSystemRepresentation(path);
    if (fsRep.isNull())
        return std::nullopt;
    struct statvfs svfs;
    if (statvfs(fsRep.data(), &svfs) != 0)
        return std::nullopt;
    return static_cast<uint64_t>(svfs.f_bavail) * svfs.f_frsize;
}

std::optional<uint64_t> volumeCapacity(const String& path)
{
    auto fsRep = fileSystemRepresentation(path);
    if (fsRep.isNull())
        return std::nullopt;
    struct statvfs svfs;
    if (statvfs(fsRep.data(), &svfs) != 0)
        return std::nullopt;
    return static_cast<uint64_t>(svfs.f_blocks) * svfs.f_frsize;
}

bool createSymbolicLink(const String& targetPath, const String& symbolicLinkPath)
{
    auto fsTarget = fileSystemRepresentation(targetPath);
    auto fsLink = fileSystemRepresentation(symbolicLinkPath);
    if (fsTarget.isNull() || fsLink.isNull())
        return false;
    return symlink(fsTarget.data(), fsLink.data()) == 0;
}

bool hardLink(const String& targetPath, const String& linkPath)
{
    auto fsTarget = fileSystemRepresentation(targetPath);
    auto fsLink = fileSystemRepresentation(linkPath);
    if (fsTarget.isNull() || fsLink.isNull())
        return false;
    return link(fsTarget.data(), fsLink.data()) == 0;
}

bool hardLinkOrCopyFile(const String& targetPath, const String& linkPath)
{
    auto fsTarget = fileSystemRepresentation(targetPath);
    auto fsLink = fileSystemRepresentation(linkPath);
    if (fsTarget.isNull() || fsLink.isNull())
        return false;

    if (link(fsTarget.data(), fsLink.data()) == 0)
        return true;

    // Fall back to byte copy
    auto sourceHandle = openFile(targetPath, FileOpenMode::Read);
    if (!isHandleValid(sourceHandle))
        return false;

    auto destHandle = openFile(linkPath, FileOpenMode::Truncate);
    if (!isHandleValid(destHandle)) {
        closeFile(sourceHandle);
        return false;
    }

    char buf[8192];
    int bytesRead;
    bool success = true;
    while ((bytesRead = readFromFile(sourceHandle, buf, sizeof(buf))) > 0) {
        if (writeToFile(destHandle, buf, bytesRead) != bytesRead) {
            success = false;
            break;
        }
    }
    closeFile(sourceHandle);
    closeFile(destHandle);
    return success;
}

std::optional<uint64_t> hardLinkCount(const String& path)
{
    auto fsRep = fileSystemRepresentation(path);
    if (fsRep.isNull())
        return std::nullopt;
    struct stat st;
    if (stat(fsRep.data(), &st) != 0)
        return std::nullopt;
    return static_cast<uint64_t>(st.st_nlink);
}

std::optional<WallTime> fileModificationTime(const String& path)
{
    auto fsRep = fileSystemRepresentation(path);
    if (fsRep.isNull())
        return std::nullopt;
    struct stat st;
    if (stat(fsRep.data(), &st) != 0)
        return std::nullopt;
    return WallTime::fromRawSeconds(static_cast<double>(st.st_mtime));
}

bool updateFileModificationTime(const String& path)
{
    auto fsRep = fileSystemRepresentation(path);
    if (fsRep.isNull())
        return false;
    return utime(fsRep.data(), nullptr) == 0;
}

bool isHiddenFile(const String& path)
{
    auto fsRep = fileSystemRepresentation(path);
    if (fsRep.isNull())
        return false;
    const char* name = strrchr(fsRep.data(), '/');
    name = name ? name + 1 : fsRep.data();
    return name[0] == '.';
}

Vector<String> listDirectory(const String& path)
{
    Vector<String> entries;
    auto fsRep = fileSystemRepresentation(path);
    if (fsRep.isNull())
        return entries;
    DIR* d = opendir(fsRep.data());
    if (!d)
        return entries;
    struct dirent* entry;
    while ((entry = readdir(d)) != nullptr) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        entries.append(String::fromUTF8(entry->d_name));
    }
    closedir(d);
    return entries;
}

String parentPath(const String& path)
{
    auto fsRep = fileSystemRepresentation(path);
    if (fsRep.isNull())
        return { };
    const char* lastSlash = strrchr(fsRep.data(), '/');
    if (!lastSlash || lastSlash == fsRep.data())
        return String::fromUTF8("/");
    return String::fromUTF8(fsRep.data(), lastSlash - fsRep.data());
}

std::optional<FileType> fileType(const String& path)
{
    auto fsRep = fileSystemRepresentation(path);
    if (fsRep.isNull())
        return std::nullopt;
    struct stat st;
    if (lstat(fsRep.data(), &st) != 0)
        return std::nullopt;
    if (S_ISREG(st.st_mode))
        return FileType::Regular;
    if (S_ISDIR(st.st_mode))
        return FileType::Directory;
    if (S_ISLNK(st.st_mode))
        return FileType::SymbolicLink;
    return std::nullopt;
}

#if !PLATFORM(COCOA) && !PLATFORM(PLAYSTATION)
String createTemporaryDirectory()
{
    const char* tmpDir = getenv("TMPDIR");
    if (!tmpDir)
        tmpDir = "/tmp";

    char tmpl[PATH_MAX];
    snprintf(tmpl, sizeof(tmpl), "%s/WebKit-XXXXXXXX", tmpDir);
    char* result = mkdtemp(tmpl);
    if (!result)
        return String();
    return stringFromFileSystemRepresentation(result);
}
#endif
