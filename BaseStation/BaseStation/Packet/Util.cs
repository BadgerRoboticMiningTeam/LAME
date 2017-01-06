using System;


namespace BaseStation.Packet
{
    static class Util
    {
        public static T Clamp<T>(this T val, T min, T max) where T : IComparable<T>
        {
            if (val.CompareTo(min) < 0)
                return min;
            else if (val.CompareTo(max) > 0)
                return max;
            else
                return val;
        }

        public static byte ComputeCRC(byte[] buffer, int startIndex = 0)
        {
            return ComputeCRC(buffer, startIndex, buffer.Length);
        }

        public static byte ComputeCRC(byte[] buffer, int startIndex, int endIndex)
        {
            if (buffer == null || startIndex < 0 || endIndex < 0 || startIndex > buffer.Length || endIndex > buffer.Length)
                throw new ArgumentException();

            if (endIndex < startIndex)
            {
                int temp = startIndex;
                startIndex = endIndex;
                endIndex = startIndex;
            }

            byte crc = 0;
            for (int i = startIndex; i < endIndex; i++)
            {
                crc ^= buffer[i];

                for (int j = 0; j < 8; j++)
                {
                    if ((crc & 0x80) != 0)
                    {
                        crc = (byte)((crc << 1) ^ 0x07);
                    }
                    else
                    {
                        crc <<= 1;
                    }
                }
            }

            return crc;
        }
    }
}
