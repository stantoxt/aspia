//
// PROJECT:         Aspia Remote Desktop
// FILE:            crypto/encryptor_sodium.cc
// LICENSE:         See top-level directory
// PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
//

#include "crypto/encryptor_sodium.h"
#include "base/logging.h"

namespace aspia {

EncryptorSodium::EncryptorSodium(Mode mode, SecureBuffer public_key, SecureBuffer secret_key) :
    local_public_key_(std::move(public_key)),
    local_secret_key_(std::move(secret_key)),
    decrypt_key_(crypto_kx_SESSIONKEYBYTES),
    encrypt_key_(crypto_kx_SESSIONKEYBYTES),
    nonce_(crypto_secretbox_NONCEBYTES),
    mode_(mode)
{
    // Generate nonce.
    randombytes_buf(nonce_.data(), nonce_.size());
}

EncryptorSodium::~EncryptorSodium()
{
    // Nothing
}

// static
std::unique_ptr<EncryptorSodium> EncryptorSodium::Create(Mode mode)
{
    if (sodium_init() == -1)
    {
        LOG(ERROR) << "sodium_init() failed";
        return nullptr;
    }

    SecureBuffer public_key(crypto_kx_PUBLICKEYBYTES);
    SecureBuffer secret_key(crypto_kx_SECRETKEYBYTES);

    if (crypto_kx_keypair(public_key.data(), secret_key.data()) != 0)
    {
        LOG(ERROR) << "crypto_kx_keypair() failed";
        return nullptr;
    }

    return std::unique_ptr<EncryptorSodium>(
        new EncryptorSodium(mode,
                            std::move(public_key),
                            std::move(secret_key)));
}

bool EncryptorSodium::SetRemotePublicKey(const IOBuffer& public_key)
{
    if (public_key.IsEmpty())
    {
        LOG(ERROR) << "Empty public key";
        return false;
    }

    if (public_key.size() != crypto_kx_PUBLICKEYBYTES)
    {
        LOG(ERROR) << "Wrong public key size";
        return false;
    }

    if (mode_ == Mode::SERVER)
    {
        if (crypto_kx_server_session_keys(decrypt_key_.data(),
                                          encrypt_key_.data(),
                                          local_public_key_.data(),
                                          local_secret_key_.data(),
                                          public_key.data()) != 0)
        {
            LOG(ERROR) << "crypto_kx_server_session_keys() failed";
            return false;
        }
    }
    else
    {
        DCHECK(mode_ == Mode::CLIENT);

        if (crypto_kx_client_session_keys(decrypt_key_.data(),
                                          encrypt_key_.data(),
                                          local_public_key_.data(),
                                          local_secret_key_.data(),
                                          public_key.data()) != 0)
        {
            LOG(ERROR) << "crypto_kx_client_session_keys() failed";
            return false;
        }
    }

    sodium_memzero(public_key.data(), public_key.size());

    return true;
}

IOBuffer EncryptorSodium::GetLocalPublicKey()
{
    if (local_public_key_.IsEmpty())
        return IOBuffer();

    IOBuffer public_key(local_public_key_.size());
    memcpy(public_key.data(), local_public_key_.data(), public_key.size());

    return public_key;
}

IOBuffer EncryptorSodium::Encrypt(const IOBuffer& source_buffer)
{
    DCHECK_EQ(nonce_.size(), crypto_secretbox_NONCEBYTES);

    sodium_increment(nonce_.data(), crypto_secretbox_NONCEBYTES);

    IOBuffer encrypted_buffer(source_buffer.size() +
                              crypto_secretbox_NONCEBYTES +
                              crypto_secretbox_MACBYTES);

    memcpy(encrypted_buffer.data(), nonce_.data(), crypto_secretbox_NONCEBYTES);

    // Encrypt message.
    if (crypto_secretbox_easy(encrypted_buffer.data() + crypto_secretbox_NONCEBYTES,
                              source_buffer.data(),
                              source_buffer.size(),
                              nonce_.data(),
                              encrypt_key_.data()) != 0)
    {
        LOG(ERROR) << "crypto_secretbox_easy() failed";
        return IOBuffer();
    }

    return encrypted_buffer;
}

IOBuffer EncryptorSodium::Decrypt(const IOBuffer& source_buffer)
{
    IOBuffer decrypted_buffer(source_buffer.size() -
                              crypto_secretbox_NONCEBYTES -
                              crypto_secretbox_MACBYTES);

    // Decrypt message.
    if (crypto_secretbox_open_easy(decrypted_buffer.data(),
                                   source_buffer.data() + crypto_secretbox_NONCEBYTES,
                                   source_buffer.size() - crypto_secretbox_NONCEBYTES,
                                   source_buffer.data(),
                                   decrypt_key_.data()) != 0)
    {
        LOG(ERROR) << "crypto_secretbox_open_easy() failed";
        return IOBuffer();
    }

    return decrypted_buffer;
}

} // namespace aspia
