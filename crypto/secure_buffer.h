//
// PROJECT:         Aspia Remote Desktop
// FILE:            crypto/secure_buffer.h
// LICENSE:         See top-level directory
// PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
//

#ifndef _ASPIA_CRYPTO__SECURE_BUFFER_H
#define _ASPIA_CRYPTO__SECURE_BUFFER_H

#include "base/macros.h"

extern "C" {
#define SODIUM_STATIC
#include <sodium.h>
} // extern "C"

#include <memory>

namespace aspia {

class SecureBuffer
{
public:
    SecureBuffer() = default;

    SecureBuffer(size_t size) :
        data_size_(size)
    {
        data_.reset(new uint8_t[data_size_]);
        sodium_memzero(data_.get(), data_size_);
    }

    SecureBuffer(SecureBuffer&& other) :
        data_size_(other.data_size_)
    {
        data_ = std::move(other.data_);
        other.data_size_ = 0;
    }

    ~SecureBuffer()
    {
        if (data_ && data_size_)
        {
            sodium_memzero(data_.get(), data_size_);
            data_.reset();
        }
    }

    SecureBuffer& operator=(SecureBuffer&& other)
    {
        data_ = std::move(other.data_);
        data_size_ = other.data_size_;
        other.data_size_ = 0;
        return *this;
    }

    size_t size() const
    {
        return data_size_;
    }
    uint8_t* data() const
    {
        return data_.get();
    }

    bool IsEmpty() const
    {
        return data_ == nullptr || data_size_ == 0;
    }

private:
    std::unique_ptr<uint8_t[]> data_;
    size_t data_size_ = 0;

    DISALLOW_COPY_AND_ASSIGN(SecureBuffer);
};

} // namespace aspia

#endif // _ASPIA_CRYPTO__SECURE_BUFFER_H
