using System;
using System.Runtime.InteropServices;

namespace IntelliPayTerminalWrapper
{
    public static class Serial
    {
        public enum ByteSizeType
        {
            FiveBits = 5,
            SixBites = 6,
            SevenBits = 7,
            EightBits = 8
        };

        public enum ParityType
        {
            ParityNone = 0,
            ParityOdd = 1,
            ParityEven = 2,
            ParityMark = 3,
            ParitySpace = 4
        };

        public enum FlowControlType
        {
            FlowControlNone = 0,
            FlowControlSoftware,
            FlowControlHardware
        };

        public enum StopBitsType
        {
            StopBitsOne = 1,
            StopBitsTwo = 2,
            StopBitsOnePointFive
        }
    }
    
    public class Wrapper
    {
        [DllImport("IntelliPayTerminal.dll", SetLastError = true, CharSet = CharSet.Auto, EntryPoint = "init", 
            CallingConvention = CallingConvention.Cdecl)] 
        public static extern bool init(string port, string terminal_type = "v400c-plus", int baud_rate = 9600, 
            Serial.ByteSizeType byte_size = Serial.ByteSizeType.EightBits, 
            Serial.StopBitsType stop_bit = Serial.StopBitsType.StopBitsOne, 
            Serial.FlowControlType xonxoff = Serial.FlowControlType.FlowControlNone, bool rtscts = false, int timeout = 0);
 
        
        [DllImport("IntelliPayTerminal.dll", SetLastError = true, CharSet = CharSet.Auto, EntryPoint = "process",
            CallingConvention = CallingConvention.Cdecl)] 
        public static extern string process(string action, float amount, string opts = "{}");

        [DllImport("IntelliPayTerminal.dll", SetLastError = true, CharSet = CharSet.Auto, EntryPoint = "sale", 
            CallingConvention = CallingConvention.Cdecl)]
        public static extern string sale(float amount, string opts = "{}");

        [DllImport("IntelliPayTerminal.dll", SetLastError = true, CharSet = CharSet.Auto, EntryPoint = "refund",
            CallingConvention = CallingConvention.Cdecl)]
        public static extern string refund(float amount, string opts = "{}");
    }
}