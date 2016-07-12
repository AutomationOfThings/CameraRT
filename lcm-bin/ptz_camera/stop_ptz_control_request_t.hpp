/** THIS IS AN AUTOMATICALLY GENERATED FILE.  DO NOT MODIFY
 * BY HAND!!
 *
 * Generated by lcm-gen
 **/

#include <lcm/lcm_coretypes.h>

#ifndef __ptz_camera_stop_ptz_control_request_t_hpp__
#define __ptz_camera_stop_ptz_control_request_t_hpp__

#include <string>

namespace ptz_camera
{

class stop_ptz_control_request_t
{
    public:
        std::string ip_address;

        int8_t     operation_type;

    public:
        // If you're using C++11 and are getting compiler errors saying things like
        // ‘constexpr’ needed for in-class initialization of static data member
        // then re-run lcm-gen with '--cpp-std=c++11' to generate code that is
        // compliant with C++11
        static const int8_t   ALL = 1;
        // If you're using C++11 and are getting compiler errors saying things like
        // ‘constexpr’ needed for in-class initialization of static data member
        // then re-run lcm-gen with '--cpp-std=c++11' to generate code that is
        // compliant with C++11
        static const int8_t   PAN = 2;
        // If you're using C++11 and are getting compiler errors saying things like
        // ‘constexpr’ needed for in-class initialization of static data member
        // then re-run lcm-gen with '--cpp-std=c++11' to generate code that is
        // compliant with C++11
        static const int8_t   TILT = 3;
        // If you're using C++11 and are getting compiler errors saying things like
        // ‘constexpr’ needed for in-class initialization of static data member
        // then re-run lcm-gen with '--cpp-std=c++11' to generate code that is
        // compliant with C++11
        static const int8_t   ZOOM = 4;

    public:
        /**
         * Encode a message into binary form.
         *
         * @param buf The output buffer.
         * @param offset Encoding starts at thie byte offset into @p buf.
         * @param maxlen Maximum number of bytes to write.  This should generally be
         *  equal to getEncodedSize().
         * @return The number of bytes encoded, or <0 on error.
         */
        inline int encode(void *buf, int offset, int maxlen) const;

        /**
         * Check how many bytes are required to encode this message.
         */
        inline int getEncodedSize() const;

        /**
         * Decode a message from binary form into this instance.
         *
         * @param buf The buffer containing the encoded message.
         * @param offset The byte offset into @p buf where the encoded message starts.
         * @param maxlen The maximum number of bytes to reqad while decoding.
         * @return The number of bytes decoded, or <0 if an error occured.
         */
        inline int decode(const void *buf, int offset, int maxlen);

        /**
         * Retrieve the 64-bit fingerprint identifying the structure of the message.
         * Note that the fingerprint is the same for all instances of the same
         * message type, and is a fingerprint on the message type definition, not on
         * the message contents.
         */
        inline static int64_t getHash();

        /**
         * Returns "stop_ptz_control_request_t"
         */
        inline static const char* getTypeName();

        // LCM support functions. Users should not call these
        inline int _encodeNoHash(void *buf, int offset, int maxlen) const;
        inline int _getEncodedSizeNoHash() const;
        inline int _decodeNoHash(const void *buf, int offset, int maxlen);
        inline static uint64_t _computeHash(const __lcm_hash_ptr *p);
};

int stop_ptz_control_request_t::encode(void *buf, int offset, int maxlen) const
{
    int pos = 0, tlen;
    int64_t hash = (int64_t)getHash();

    tlen = __int64_t_encode_array(buf, offset + pos, maxlen - pos, &hash, 1);
    if(tlen < 0) return tlen; else pos += tlen;

    tlen = this->_encodeNoHash(buf, offset + pos, maxlen - pos);
    if (tlen < 0) return tlen; else pos += tlen;

    return pos;
}

int stop_ptz_control_request_t::decode(const void *buf, int offset, int maxlen)
{
    int pos = 0, thislen;

    int64_t msg_hash;
    thislen = __int64_t_decode_array(buf, offset + pos, maxlen - pos, &msg_hash, 1);
    if (thislen < 0) return thislen; else pos += thislen;
    if (msg_hash != getHash()) return -1;

    thislen = this->_decodeNoHash(buf, offset + pos, maxlen - pos);
    if (thislen < 0) return thislen; else pos += thislen;

    return pos;
}

int stop_ptz_control_request_t::getEncodedSize() const
{
    return 8 + _getEncodedSizeNoHash();
}

int64_t stop_ptz_control_request_t::getHash()
{
    static int64_t hash = _computeHash(NULL);
    return hash;
}

const char* stop_ptz_control_request_t::getTypeName()
{
    return "stop_ptz_control_request_t";
}

int stop_ptz_control_request_t::_encodeNoHash(void *buf, int offset, int maxlen) const
{
    int pos = 0, tlen;

    char* ip_address_cstr = (char*) this->ip_address.c_str();
    tlen = __string_encode_array(buf, offset + pos, maxlen - pos, &ip_address_cstr, 1);
    if(tlen < 0) return tlen; else pos += tlen;

    tlen = __int8_t_encode_array(buf, offset + pos, maxlen - pos, &this->operation_type, 1);
    if(tlen < 0) return tlen; else pos += tlen;

    return pos;
}

int stop_ptz_control_request_t::_decodeNoHash(const void *buf, int offset, int maxlen)
{
    int pos = 0, tlen;

    int32_t __ip_address_len__;
    tlen = __int32_t_decode_array(buf, offset + pos, maxlen - pos, &__ip_address_len__, 1);
    if(tlen < 0) return tlen; else pos += tlen;
    if(__ip_address_len__ > maxlen - pos) return -1;
    this->ip_address.assign(((const char*)buf) + offset + pos, __ip_address_len__ - 1);
    pos += __ip_address_len__;

    tlen = __int8_t_decode_array(buf, offset + pos, maxlen - pos, &this->operation_type, 1);
    if(tlen < 0) return tlen; else pos += tlen;

    return pos;
}

int stop_ptz_control_request_t::_getEncodedSizeNoHash() const
{
    int enc_size = 0;
    enc_size += this->ip_address.size() + 4 + 1;
    enc_size += __int8_t_encoded_array_size(NULL, 1);
    return enc_size;
}

uint64_t stop_ptz_control_request_t::_computeHash(const __lcm_hash_ptr *)
{
    uint64_t hash = 0xc4dd99a934cf2ba7LL;
    return (hash<<1) + ((hash>>63)&1);
}

}

#endif