// PHZ
// 2018-5-16

#ifndef XOP_METADATA_SOURCE_H
#define XOP_METADATA_SOURCE_H

#include "MediaSource.h"

namespace xop
{ 

class MetadataSource : public MediaSource
{
public:
	static MetadataSource* CreateNew(uint32_t framerate=25, const std::string &track = "trackID");
	~MetadataSource();

	void SetFramerate(uint32_t framerate)
	{ framerate_ = framerate; }

	uint32_t GetFramerate() const 
	{ return framerate_; }

	virtual std::string GetMediaDescription(uint16_t port); 

	virtual std::string GetAttribute(); 

	bool HandleFrame(MediaChannelId channel_id, AVFrame frame);

	static uint32_t GetTimestamp();
	
private:
	MetadataSource(uint32_t framerate, const std::string &track);

	uint32_t framerate_ = 25;
};
	
}

#endif



