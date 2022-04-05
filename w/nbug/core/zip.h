#ifndef NB_ZIP_H
#define NB_ZIP_H

namespace e
{
	//  _header: 0=none, 1=zlib, 2=gzip
	//  ---------- example -------
	//	void * out_buf;
	//  size_t out_len;
	//	try
	//	{
	//		nb_block_unzip(out_buf, out_len, in_buf, in_len);
	//	}
	//	catch (const char * _err)
	//	{
	//		exit(1);
	//	}
	//	memcpy(mybuf, out_buf, out_len);
	//	nb_zip_free(out_buf);
	//
	void nb_block_zip(void * & _out_buf, size_t & _out_len, void * _in_buf, size_t _in_len, int _header = 1) throw(const char *);
	void nb_block_unzip(void * & _out_buf, size_t & _out_len, void * _in_buf, size_t _in_len) throw(const char *);
	void nb_zip_free(void * _p);

	void nb_zip(
		void *_read_context,
		int (*_read_func)(void*, void*, unsigned int),
		void *_write_context,
		int (*_write_func)(void*, void*, unsigned int),
		int _header = 1);

	void nb_unzip(
		void *_read_context,
		int (*_read_func)(void*, void*, unsigned int),
		void *_write_context,
		int (*_write_func)(void*, void*, unsigned int));

	void nb_crc32(uint32 & _crc, uint8 _byte);

}

#endif
