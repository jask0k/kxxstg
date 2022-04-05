#ifndef E_AL_LISTENER_H
#define E_AL_LISTENER_H

#include <nbug/al/aplayer.h>
#include <nbug/core/file.h>

namespace e
{

	//  (channel_0)  (channel_1)
	// [ sample_0 ] [ sample_1 ]   <- frame 0
	// [ sample_2 ] [ sample_3 ]   <- frame 1
	// [ sample_4 ] [ sample_5 ]   <- frame 2
	// [ sample_6 ] [ sample_7 ]   <- frame 3


	static const int NB_READ_END = -1;
	static const int NB_READ_ERR = -2;

	class SoundFileReader
	{
	public:
		virtual ~SoundFileReader(){};
		virtual bool GetInfo(AInfo & _info) = 0;
		virtual bool Rewind();
		virtual int Read(float * _buf, int _read_frames, int _channels) = 0;
		static SoundFileReader * Create(const Path & _path) throw(const char *);
	};

	class WaveFileReader : public SoundFileReader
	{
	private:
//		Path path;
		FileRef file;
		bool init_failed;
		bool is_float;
		int channel_count;
		int bit_width;
		int block_align;
		int frame_count;
		int frame_read;
		string title;
		uint64 dataBeginPos;
	//	bool Init() override;
	public:
		WaveFileReader(const Path & _path);
		~WaveFileReader();
		bool GetInfo(AInfo & _info) override;
		bool Rewind() override;
		int Read(float * _buf, int _read_frames, int _channels) override;
	};

	struct ABlock
	{
		float * data; // use malloc and free
		int 	channel_count;
		int     frame_count;

		int byte_count() const
		{ return channel_count * frame_count * sizeof(float); }

		int float_count() const
		{ return channel_count * frame_count; }

		int bytes_per_frame() const
		{ return sizeof(float) * channel_count; }

		ABlock();
		~ABlock();
		bool Load(const Path & _path);
		bool Save(const Path & _path);
	};

}

#endif
