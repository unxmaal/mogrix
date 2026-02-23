/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "IPCSemaphore.h"

#include "Decoder.h"
#include "Encoder.h"
#include "WebCoreArgumentCoders.h"
#include <wtf/UniStdExtras.h>

#if OS(LINUX)
#include <poll.h>
#include <sys/eventfd.h>
#elif defined(__sgi)
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#endif

namespace IPC {

Semaphore::Semaphore()
{
#if OS(LINUX)
    m_fd = { eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK | EFD_SEMAPHORE), UnixFileDescriptor::Adopt };
#elif defined(__sgi)
    // IRIX: emulate eventfd using a pipe.
    // m_fd = read end (for poll/wait), m_writeFd = write end (for signal).
    // Both ends are non-blocking + close-on-exec.
    int fds[2];
    if (pipe(fds) == 0) {
        fcntl(fds[0], F_SETFL, fcntl(fds[0], F_GETFL) | O_NONBLOCK);
        fcntl(fds[1], F_SETFL, fcntl(fds[1], F_GETFL) | O_NONBLOCK);
        fcntl(fds[0], F_SETFD, FD_CLOEXEC);
        fcntl(fds[1], F_SETFD, FD_CLOEXEC);
        m_fd = { fds[0], UnixFileDescriptor::Adopt };
        m_writeFd = { fds[1], UnixFileDescriptor::Adopt };
    }
#endif
}

Semaphore::Semaphore(UnixFileDescriptor&& fd)
    : m_fd(WTFMove(fd))
{ }

#ifdef __sgi
Semaphore::Semaphore(UnixFileDescriptor&& readFd, UnixFileDescriptor&& writeFd)
    : m_fd(WTFMove(readFd))
    , m_writeFd(WTFMove(writeFd))
{ }
#endif

Semaphore::Semaphore(Semaphore&&) = default;
Semaphore& Semaphore::operator=(Semaphore&&) = default;

Semaphore::~Semaphore()
{
    destroy();
}

void Semaphore::signal()
{
#if OS(LINUX)
    ASSERT_WITH_MESSAGE(m_fd.value() >= 0, "Signalling on an invalid semaphore object");

    // Matching waitImpl() and EFD_SEMAPHORE semantics, increment the semaphore value by 1.
    uint64_t value = 1;
    while (true) {
        int ret = write(m_fd.value(), &value, sizeof(uint64_t));
        if (LIKELY(ret != -1 || errno != EINTR))
            break;
    }
#elif defined(__sgi)
    ASSERT_WITH_MESSAGE(m_writeFd.value() >= 0, "Signalling on an invalid semaphore object");

    // Write a single byte to the pipe write end to wake up the waiter.
    uint8_t value = 1;
    while (true) {
        int ret = write(m_writeFd.value(), &value, sizeof(uint8_t));
        if (LIKELY(ret != -1 || errno != EINTR))
            break;
    }
#endif
}

#if OS(LINUX) || defined(__sgi)
static bool waitImpl(int fd, int timeout)
{
    struct pollfd pollfdValue { .fd = fd, .events = POLLIN, .revents = 0 };
    int ret = 0;

    // Iterate on polling while interrupts are thrown, otherwise bail out of the loop immediately.
    while (true) {
        ret = poll(&pollfdValue, 1, timeout);
        if (LIKELY(ret != -1 || errno != EINTR))
            break;
    }

    // Fail if the return value doesn't indicate the single file descriptor with only input events available.
    if (ret != 1 || !!(pollfdValue.revents ^ POLLIN))
        return false;

#if OS(LINUX)
    // There should be data for reading -- due to EFD_SEMAPHORE, it should be 1 packed into an 8-byte value.
    uint64_t value = 0;
    ret = read(fd, &value, sizeof(uint64_t));
    if (ret != sizeof(uint64_t) || value != 1)
        return false;
#else
    // Pipe-based: read a single byte to consume the signal.
    uint8_t value = 0;
    ret = read(fd, &value, sizeof(uint8_t));
    if (ret != sizeof(uint8_t))
        return false;
#endif

    return true;
}
#endif

bool Semaphore::wait()
{
#if OS(LINUX) || defined(__sgi)
    ASSERT_WITH_MESSAGE(m_fd.value() >= 0, "Waiting on an invalid semaphore object");

    return waitImpl(m_fd.value(), -1);
#else
    return false;
#endif
}

bool Semaphore::waitFor(Timeout timeout)
{
#if OS(LINUX) || defined(__sgi)
    ASSERT_WITH_MESSAGE(m_fd.value() >= 0, "Waiting on an invalid semaphore object");

    int timeoutValue = -1;
    if (!timeout.isInfinity())
        timeoutValue = int(timeout.secondsUntilDeadline().milliseconds());
    return waitImpl(m_fd.value(), timeoutValue);
#else
    return false;
#endif
}

void Semaphore::encode(Encoder& encoder) const
{
#ifdef __sgi
    // IRIX pipe-based: send both the read fd and write fd.
    // The receiver needs the read fd for wait() and the write fd for signal().
    encoder << m_fd.duplicate();
    encoder << m_writeFd.duplicate();
#else
    encoder << m_fd.duplicate();
#endif
}

std::optional<Semaphore> Semaphore::decode(Decoder& decoder)
{
#ifdef __sgi
    // IRIX pipe-based: receive both the read fd and write fd.
    std::optional<UnixFileDescriptor> readFd;
    decoder >> readFd;
    if (!readFd)
        return std::nullopt;
    std::optional<UnixFileDescriptor> writeFd;
    decoder >> writeFd;
    if (!writeFd)
        return std::nullopt;
    return std::optional<Semaphore> { std::in_place, WTFMove(*readFd), WTFMove(*writeFd) };
#else
    std::optional<UnixFileDescriptor> fd;
    decoder >> fd;
    if (!fd)
        return std::nullopt;
    return std::optional<Semaphore> { std::in_place, WTFMove(*fd) };
#endif
}

void Semaphore::destroy()
{
    m_fd = { };
#ifdef __sgi
    m_writeFd = { };
#endif
}

} // namespace IPC
