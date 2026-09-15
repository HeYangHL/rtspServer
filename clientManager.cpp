#include "clientManager.hpp"

int ClientManager::Add_Source(std::string sourceName, int (*read_video_stream)(void *opaque, uint8_t *data, bool keyFrame, uint64_t *pts), void *video_opaque, int (*read_audio_stream)(void *opaque, uint8_t *data, int len, uint64_t *pts), void *audio_opaque, uint32_t videoFrameRate, uint32_t audioSampleRate, std::string videoCodecType, std::string audioCodecType)
{
    SOURCE new_session;

	new_session.read_video_callback = read_video_stream;
	new_session.read_audio_callback = read_audio_stream;
	new_session.video_fps = videoFrameRate;
	new_session.audio_sample = audioSampleRate;
	new_session.video_format = videoCodecType;
	new_session.audio_format = audioCodecType;
	new_session.video_opaque = video_opaque;
	new_session.audio_opaque = audio_opaque;
    rtsp_source.insert(std::map<std::string, SOURCE>::value_type(sourceName, new_session));
}