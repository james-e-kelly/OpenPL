using System;
using System.Text;
using System.IO;

public class WAVFormatException : Exception
{
    public WAVFormatException(string message) : base(message) { }
}

public class WAV
{
	static readonly byte[] riffID;
	static readonly byte[] wavID;
	static readonly byte[] fmtID;
	static readonly byte[] junkID;
	static readonly byte[] dataID;

	static WAV()
	{
		riffID = Encoding.ASCII.GetBytes("RIFF");
		wavID = Encoding.ASCII.GetBytes("WAVE");
		fmtID = Encoding.ASCII.GetBytes("fmt ");
		junkID = Encoding.ASCII.GetBytes("JUNK");
		dataID = Encoding.ASCII.GetBytes("data");
	}

	static bool CheckData(byte[] a, byte[] b, bool throwException)
	{
		for (int i = 0; i < a.Length; i++)
			if (a[i] != b[i])
			{
				if (throwException)
					throw new WAVFormatException("Incorrect/unexpected ID");
				else
					return false;
			}
		return true;
	}

	public static float[] Read(string path, out int channels, out int sampleRate)
	{
		float[] ret;
		using (FileStream fs = new FileStream(path, FileMode.Open))
		using (BinaryReader br = new BinaryReader(fs))
		{
			if (br.BaseStream.Length < 44)
				throw new WAVFormatException("Header is too short");

			// RIFF chunk descriptor

			// ChunkID
			CheckData(riffID, br.ReadBytes(4), true);

			// ChunkSize
			var totalLength = br.ReadInt32();
			if (totalLength < 36)
				throw new WAVFormatException("Invalid length");

			// Format
            CheckData(wavID, br.ReadBytes(4), true);

			// Subchunk 1. Can be "JUNK" or "fmt "

			//Subchunk1ID
			byte[] subchunk1ID = br.ReadBytes(4);

			// if JUNK chunk
			if (CheckData(junkID, subchunk1ID, false))
            {
				var chunkSize = br.ReadInt32();
				br.ReadBytes(chunkSize);    // Read and skip the rest of this data
				subchunk1ID = br.ReadBytes(4);
			}
			// else fmt chunk
			else if (!CheckData(fmtID, subchunk1ID, false))
            {
				throw new WAVFormatException("Unkown chunk id");
			}

			// once moving past the junk header, can assume everything is back to normal

			// Subchunk1Size
			var headerLength = br.ReadInt32();

			if (headerLength < 16)
				throw new WAVFormatException("Invalid header length");

			var encoding = br.ReadUInt16();

			if (encoding != 1)
            {
				UnityEngine.Debug.Log(encoding);
				throw new WAVFormatException("Unsupported encoding (PCM only)");
			}
				

			channels = br.ReadUInt16();

			sampleRate = br.ReadInt32();

			/*var bitrate =*/
			br.ReadInt32();

			var blockAlign = br.ReadUInt16();

			var bitsPerSample = br.ReadUInt16();

			bool use16bit;

			if (bitsPerSample == 8)
				use16bit = false;
			else if (bitsPerSample == 16)
				use16bit = true;
			else
				throw new WAVFormatException("Unsupported sample size (8 or 16 bits only)");

			if (channels * bitsPerSample != blockAlign * 8)
				throw new WAVFormatException("Incorrect block align");

			headerLength -= 16;
			while (headerLength-- > 0)
				br.ReadByte();

			while (!CheckData(dataID, br.ReadBytes(4), false))
				for (int i = br.ReadInt32(); i > 0; i--)
					br.ReadByte();

			var dataLength = br.ReadInt32();
			if (br.BaseStream.Length - br.BaseStream.Position < dataLength)
				throw new WAVFormatException("Invalid data length");

			ret = new float[use16bit ? dataLength / 2 : dataLength];
			for (int i = 0; i < ret.Length; i++)
				ret[i] = use16bit ?
					br.ReadInt16() / (float)short.MaxValue :
					(br.ReadByte() - 127f) / sbyte.MaxValue;
		}
		return ret;
	}
}