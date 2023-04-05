// PHZ
// 2018-5-16

#if defined(WIN32) || defined(_WIN32)
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

#include "rtp.h"
#include "MetadataSource.h"
#include <cstdio>
#include <chrono>
#if defined(__linux) || defined(__linux__)
#include <sys/time.h>
#endif

using namespace xop;
using namespace std;

MetadataSource::MetadataSource(uint32_t framerate, const std::string &track)
	: framerate_(framerate)
{
	payload_    = 107;
	media_type_ = Metadata;
	clock_rate_ = 90000;
	trackName = track;
}

MetadataSource* MetadataSource::CreateNew(uint32_t framerate, const std::string &track)
{
	return new MetadataSource(framerate, track);
}

MetadataSource::~MetadataSource()
{

}

string MetadataSource::GetMediaDescription(uint16_t port)
{
	char buf[100] = {0};
	sprintf(buf, "m=application %hu RTP/AVP 107", port); // \r\nb=AS:2000
	return string(buf);
}

string MetadataSource::GetAttribute()
{
	return string("a=rtpmap:107 VND.ONVIF.METADATA/90000");
}

bool MetadataSource::HandleFrame(MediaChannelId channel_id, AVFrame frame)
{
	uint8_t* frame_buf  = frame.buffer.get();
	uint32_t frame_size = frame.size;

	if (frame.timestamp == 0) {
		frame.timestamp = GetTimestamp();
	}

	if (frame_size <= MAX_RTP_PAYLOAD_SIZE) {
		RtpPacket rtp_pkt;
		rtp_pkt.type = frame.type;
		rtp_pkt.timestamp = frame.timestamp;
		rtp_pkt.size = frame_size + RTP_HEADER_SIZE + 4;
		rtp_pkt.last = 1;
		memcpy(rtp_pkt.data.get() + RTP_HEADER_SIZE + 4, frame_buf, frame_size);

		if (send_frame_callback_) {
			if (!send_frame_callback_(channel_id, rtp_pkt)) {
				return false;
			}
		}
	}
	else {
		while (frame_size > MAX_RTP_PAYLOAD_SIZE) {
			RtpPacket rtp_pkt;
			rtp_pkt.type = frame.type;
			rtp_pkt.timestamp = frame.timestamp;
			rtp_pkt.size = RTP_HEADER_SIZE + MAX_RTP_PAYLOAD_SIZE + 4;
			rtp_pkt.last = 0;

			memcpy(rtp_pkt.data.get() + RTP_HEADER_SIZE + 4, frame_buf, MAX_RTP_PAYLOAD_SIZE);

			if (send_frame_callback_) {
				if (!send_frame_callback_(channel_id, rtp_pkt))
					return false;
			}

			frame_buf  += MAX_RTP_PAYLOAD_SIZE;
			frame_size -= MAX_RTP_PAYLOAD_SIZE;
		}

		{
			RtpPacket rtp_pkt;
			rtp_pkt.type = frame.type;
			rtp_pkt.timestamp = frame.timestamp;
			rtp_pkt.size = RTP_HEADER_SIZE + frame_size + 4;
			rtp_pkt.last = 1;
			memcpy(rtp_pkt.data.get() + RTP_HEADER_SIZE + 4, frame_buf, frame_size);

			if (send_frame_callback_) {
				if (!send_frame_callback_(channel_id, rtp_pkt)) {
					return false;
				}
			}
		}
	}

	return true;
}

uint32_t MetadataSource::GetTimestamp()
{
	/* #if defined(__linux) || defined(__linux__)
	struct timeval tv = {0};
	gettimeofday(&tv, NULL);
	uint32_t ts = ((tv.tv_sec*1000)+((tv.tv_usec+500)/1000))*90; // 90: _clockRate/1000;
	return ts;
#else  */
	auto time_point = chrono::time_point_cast<chrono::microseconds>(chrono::steady_clock::now());
	return (uint32_t)((time_point.time_since_epoch().count() + 500) / 1000 * 90 );
	//#endif
}

