/* LCM type definition class file
 * This file was automatically generated by lcm-gen
 * DO NOT MODIFY BY HAND!!!!
 */

using System;
using System.Collections.Generic;
using System.IO;
using LCM.LCM;
 
namespace PtzCamera
{
    public sealed class DiscoveryRequest_t : LCM.LCM.LCMEncodable
    {
 
        public DiscoveryRequest_t()
        {
        }
 
        public static readonly ulong LCM_FINGERPRINT;
        public static readonly ulong LCM_FINGERPRINT_BASE = 0x0000000012345678L;
 
        static DiscoveryRequest_t()
        {
            LCM_FINGERPRINT = _hashRecursive(new List<String>());
        }
 
        public static ulong _hashRecursive(List<String> classes)
        {
            if (classes.Contains("PtzCamera.DiscoveryRequest_t"))
                return 0L;
 
            classes.Add("PtzCamera.DiscoveryRequest_t");
            ulong hash = LCM_FINGERPRINT_BASE
                ;
            classes.RemoveAt(classes.Count - 1);
            return (hash<<1) + ((hash>>63)&1);
        }
 
        public void Encode(LCMDataOutputStream outs)
        {
            outs.Write((long) LCM_FINGERPRINT);
            _encodeRecursive(outs);
        }
 
        public void _encodeRecursive(LCMDataOutputStream outs)
        {
        }
 
        public DiscoveryRequest_t(byte[] data) : this(new LCMDataInputStream(data))
        {
        }
 
        public DiscoveryRequest_t(LCMDataInputStream ins)
        {
            if ((ulong) ins.ReadInt64() != LCM_FINGERPRINT)
                throw new System.IO.IOException("LCM Decode error: bad fingerprint");
 
            _decodeRecursive(ins);
        }
 
        public static PtzCamera.DiscoveryRequest_t _decodeRecursiveFactory(LCMDataInputStream ins)
        {
            PtzCamera.DiscoveryRequest_t o = new PtzCamera.DiscoveryRequest_t();
            o._decodeRecursive(ins);
            return o;
        }
 
        public void _decodeRecursive(LCMDataInputStream ins)
        {
        }
 
        public PtzCamera.DiscoveryRequest_t Copy()
        {
            PtzCamera.DiscoveryRequest_t outobj = new PtzCamera.DiscoveryRequest_t();
            return outobj;
        }
    }
}
