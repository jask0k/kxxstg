#include <string.h>
#include <nbug/al/afile.h>
#include <nbug/core/debug.h>

namespace e
{
	ABlock::ABlock()
	{
		data = 0;
	}

	ABlock::~ABlock()
	{
		free(data);
	}

	bool ABlock::Load(const Path & _path)
	{
		free(data);
		data = 0;

		SoundFileReader * reader = SoundFileReader::Create(_path);
		AInfo info;
		if(!reader || ! reader->GetInfo(info))
		{
			delete reader;
			return false;
		}

		channel_count = info.channel_count;
		frame_count = info.frame_count;

		if(frame_count == 0)
		{
			frame_count = 1024 * 10; // 10k
			data = (float *) malloc(frame_count * bytes_per_frame());
			uint32 pos = 0;

			reader->Rewind();
			while(true)
			{
				float buf[1024];
				int n = reader->Read(buf, 1024 / channel_count, channel_count);
				if(n <= 0)
				{
					break;
				}

				int new_frame_count = pos + n;
				if(new_frame_count > frame_count)
				{
					frame_count = new_frame_count + (new_frame_count >> 1) + 1;
					data = (float *) realloc(data, frame_count * bytes_per_frame());
				}
				memcpy(data + pos*channel_count, buf, n * bytes_per_frame());
				pos+= n;
			}
			frame_count = pos;
		}
		else
		{
			data = (float *) malloc(frame_count * bytes_per_frame());
			uint32 n = reader->Read(data, frame_count, channel_count);
			if(n > 0)
			{
				if(n < frame_count)
				{
					frame_count = n;
				}
			}
			else
			{
				frame_count = 0;
			}
		}

		delete reader;

		if(frame_count > 0)
		{
			return true;
		}
		else
		{
			free(data);
			data = 0;
			return false;
		}
	}

	bool ABlock::Save(const Path & _path)
	{
		if(data == 0)
		{
			return false;
		}

		FileRef f = FS::OpenFile(_path, true);
		if(!f)
		{
			return false;
		}

		uint16 format = 3;
		uint16 channels = this->channel_count;
		uint32 frameRate = 44100;
		uint16 bitsPerSample = sizeof(float)*8;
		uint16 blockAlign = channels * sizeof(float);
		uint32 bytesPerSecond = blockAlign * frameRate;
		uint32 cbSize = 0;
		uint32 sub_chunk_1_size = 18;
		uint32 sub_chunk_2_size = this->byte_count();
		uint32 chunk_size = 4 + 8 + sub_chunk_1_size + 8 + sub_chunk_2_size;

		if(!f->Write("RIFF", 4)
			|| !f->Write(&chunk_size, 4)
			|| !f->Write("WAVE", 4))
		{
			return false;
		}

		if(!f->Write("fmt ", 4)
			|| !f->Write(&sub_chunk_1_size, 4)
			|| !f->Write(&format, 2)
			|| !f->Write(&channels, 2)
			|| !f->Write(&frameRate, 4)
			|| !f->Write(&bytesPerSecond, 4)
			|| !f->Write(&blockAlign, 2)
			|| !f->Write(&bitsPerSample, 2)
			|| !f->Write(&cbSize, 2)
			)
		{
			return false;
		}

		if(!f->Write("data", 4)
			|| !f->Write(&sub_chunk_2_size, 4)
			|| !f->Write(data, sub_chunk_2_size))
		{
			return false;
		}

		return true;
	}

	extern SoundFileReader * create_oga_reader(const Path & _path);
	extern SoundFileReader * create_midi_reader(const Path & _path);

	SoundFileReader * SoundFileReader::Create(const Path & _path) throw(const char *)
	{
		string ext = _path.GetExtension();
		if(ext.icompare(L"mid") == 0 ||  ext.icompare(L"midi") == 0 || ext.icompare(L"rmi") == 0)
		{
			return create_midi_reader(_path);
		}
		else if(ext.icompare(L"ogg") == 0 || ext.icompare(L"oga") == 0)
		{
			return create_oga_reader(_path);
		}
		else
		{
			return enew WaveFileReader(_path);
		}
	}

	bool SoundFileReader::Rewind()
	{
		return false;
	}


// ==============================================================
// wav file
// ==============================================================
#pragma pack(push, 1)
	struct E_WAVE_FILE_HEADER
	{
		char magicCode[4];// 'R', 'I', 'F', 'F'
		uint32 fileLength_8;
		char formType[4];
	};

	struct E_WAVE_CHUNK_HEADER
	{
		char chunkType[4];
		uint32 chunkLength;
	};
#pragma pack(pop)

	WaveFileReader::WaveFileReader(const Path & _path)
	{
		init_failed = false;
		channel_count = 0;
		bit_width = 0;
		frame_count = 0;
		frame_read = 0;
		dataBeginPos = 0;
		file = 0;

		E_WAVE_FILE_HEADER wf;
		E_WAVE_CHUNK_HEADER chunk;
		bool isWaveFile = false;
		uint64 end_of_list_chunk;
		uint64 list_chunk_read;
		FileRef f = FS::OpenFile(_path);
		if(!f)
		{
			throw(NB_SRC_LOC "Failed to open file" );
		}

		if(!f->Read(&wf.magicCode[0], sizeof(wf.magicCode))
			|| memcmp(wf.magicCode, "RIFF", 4) != 0
			|| !f->Read(&wf.fileLength_8, sizeof(wf.fileLength_8))
			|| !f->Read(&wf.formType[0], sizeof(wf.formType))
			|| memcmp(wf.formType, "WAVE", 4) != 0
			)
		{
			throw(NB_SRC_LOC "Not RIFF PCM Wave format." );
		}

		while(f->Read(&chunk.chunkType[0], sizeof(chunk.chunkType)) 
			&& f->Read(&chunk.chunkLength, sizeof(chunk.chunkLength)))
		{
			if(memcmp(chunk.chunkType, "fmt ", 4) == 0)
			{
				uint16 format;
				uint16 channels;
				uint32 frameRate;
				uint32 bytesPerSecond;
				uint16 blockAlign;
				uint16 bitsPerSample;

				if(!f->Read(&format, 2)
					|| format != 1 && format != 3
					|| !f->Read(&channels, 2)
					|| !f->Read(&frameRate, 4)
					|| !f->Read(&bytesPerSecond, 4)
					|| !f->Read(&blockAlign, 2)
					|| !f->Read(&bitsPerSample, 2)
					|| bitsPerSample != 8 && bitsPerSample != 16 && bitsPerSample != 32
					)
				{
					throw(NB_SRC_LOC "Corrupted file or unsupported format." );
				}
				E_ASSERT(frameRate == 44100);
				this->is_float = format == 3;
				this->channel_count = channels;
				this->bit_width     = bitsPerSample;
				this->block_align   = channel_count * bit_width / 8;
				int skip = chunk.chunkLength - 16;
				if(skip > 0 && !f->Skip(skip))
				{
					throw(NB_SRC_LOC "Corrupted file." );
				}
				isWaveFile = true;
			}
			else if(memcmp(chunk.chunkType, "LIST", 4) == 0)
			{
				end_of_list_chunk = f->Tell() + chunk.chunkLength;
				if(chunk.chunkLength < 4 + 8)
				{
					if(!f->Skip(chunk.chunkLength))
					{
						throw(NB_SRC_LOC "Corrupted file." );
					}
					continue;
				}

				list_chunk_read = 0;
				while(list_chunk_read < chunk.chunkLength)
				{
					char sub_id[4];
					uint32 sub_len;
					if(!f->Read(sub_id, 4))
					{
						if(!f->Seek(end_of_list_chunk))
						{
							throw(NB_SRC_LOC "Corrupted file." );
						}
						break;
					}
					list_chunk_read+=4;
					if(memcmp(sub_id, "adtl", 4) == 0
						|| memcmp(sub_id, "INFO", 4) == 0)
					{
						continue;
					}

					if(!f->Read(&sub_len, 4)
						|| sub_len + 4 + list_chunk_read > chunk.chunkLength)
					{
						if(!f->Seek(end_of_list_chunk))
						{
							throw(NB_SRC_LOC "Corrupted file." );
						}
						break;
					}
					list_chunk_read+=4;
					if(sub_len & 0x01)
					{
						sub_len++;
					}

					if(memcmp(sub_id, "INAM", 4) == 0)
					{
						char buf[256];
						if(sub_len >= sizeof(buf) || !f->Read(buf, sub_len))
						{
							if(!f->Seek(end_of_list_chunk))
							{
								throw(NB_SRC_LOC "Corrupted file." );
							}
							break;
						}
						list_chunk_read+= sub_len;
						buf[sub_len] = 0;
						title = string(buf);
						if(!f->Seek(end_of_list_chunk))
						{
							throw(NB_SRC_LOC "Corrupted LIST chunk.");
						}
						break;
					}
					else
					{
						if(!f->Skip(sub_len))
						{
							throw(NB_SRC_LOC "Corrupted file." );
						}
					}
				}
			}
			else if(memcmp(chunk.chunkType, "data", 4) == 0)
			{
				if(isWaveFile)
				{
					uint32 bytesTotal = chunk.chunkLength;
					this->frame_count = bytesTotal / block_align;
					this->dataBeginPos = f->Tell();
					file = f;
					if(title.empty())
					{
						title = _path.GetBaseName(false);
					}
					return;
				}
			}
			else
			{
				if(!f->Skip(chunk.chunkLength))
				{
					throw(NB_SRC_LOC "Corrupted file." );
				}
			}
		}
		throw(NB_SRC_LOC "Corrupted file." );
	}

	WaveFileReader::~WaveFileReader()
	{
	}

	bool WaveFileReader::GetInfo(AInfo & _info)
	{
		_info.channel_count = this->channel_count;
		_info.frame_count   = this->frame_count;
		_info.title         = this->title;
		return true;
	}

	bool WaveFileReader::Rewind()
	{
		frame_read = 0;
		return file->Seek(dataBeginPos);
	}

	int WaveFileReader::Read(float * _buf, int _read_frames, int _channels)
	{
		if( _channels != this->channel_count)
		{
			return NB_READ_ERR;
		}
		int frame_remain = frame_count - frame_read;
		if(frame_remain == 0)
		{
			return NB_READ_END;
		}
		int actual_frame_to_read = frame_remain < _read_frames ? frame_remain : _read_frames;
		int bytes_to_read = actual_frame_to_read * block_align;
		int	actual_frame_read;
		if(this->is_float)
		{
			int actual_bytes_read = file->ReadSome(_buf, bytes_to_read);
			actual_frame_read = actual_bytes_read / block_align;
		}
		else if(!this->is_float)
		{
			int buf_size_bytes = _read_frames * _channels * sizeof(float);
			int off = buf_size_bytes - bytes_to_read;
			uint8* buf = ((uint8*)_buf) + off;
			int actual_bytes_read = file->ReadSome(buf, bytes_to_read);
			actual_frame_read = actual_bytes_read / block_align;
			int count = actual_frame_read * this->channel_count;
			float* p1 = _buf;
			switch(bit_width)
			{
			case 32:
				{
					int32* p = (int32*)buf;
					int32* end = p+count;
					while(p < end)
					{
						*p1++ = float(*p++) / 2147483648.0f;
					}
				}
				break;
			case 16:
				{
					int16* p = (int16*)buf;
					int16* end = p+count;
					while(p < end)
					{
						*p1++ = float(*p++) / 32768.0f;
					}
				}
				break;
			case 8:
				{
					uint8* p = (uint8*)buf;
					uint8* end = p+count;
					while(p < end)
					{
						*p1++ = float(*p++) / 128.0f - 1.0f;
					}
				}
				break;
			default:
				return NB_READ_ERR;
			}
		}
		frame_read+= actual_frame_read;
		if(actual_frame_read < actual_frame_to_read)
		{
			frame_read = frame_count;
		}

		return actual_frame_read;
	}
}
