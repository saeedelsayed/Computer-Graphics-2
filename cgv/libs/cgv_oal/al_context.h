#pragma once

#include <list>
#include <map>
#include <string>

#include <cgv/math/fvec.h>
#include <cgv/math/fmat.h>
#include <cgv/type/info/type_id.h>

#include "lib_begin.h"

struct ALCdevice;
struct ALCcontext;
using ALuint = unsigned int;

namespace cgv {
namespace audio {

/// simple descriptor for sounds that can be played back in OAL
struct OALSoundFormat
{
	enum Type {
		SFT_UINT8,
		SFT_INT16
	} value_type;
	enum Channels {
		SFC_MONO = 1,
		SFC_STEREO = 2
	} nr_channels;
	int sampling_rate;
	int nr_frames;
};

/**
 * @brief      This class provides easy sample loading, device enumeration and
 *             error retrieval.
 */
class CGV_API OALContext
{
public:
	/** @brief  Enumerate audio output devices available on system
	  * @return vector of device names which can be passed to constructor
	  * @see    explicit OALContext(std::string) */
	static std::vector<std::string> enumerate_devices();
	/// returns name of system's default device
	static std::string get_default_device_name();
	/** @brief     Construct context, connect to output device and make context current
	  * @param[in] device  The OpenAL device string identifier
	  * @see       enumerate_devices() */
	explicit OALContext(const std::string& device_name);
	OALContext(const OALContext& other) = delete;
	OALContext(OALContext&& other) = default;
	OALContext& operator=(const OALContext& other) = delete;
	OALContext& operator=(OALContext&& other) = default;
	/// detach context from output device and destruct
	~OALContext();
	/// return device name of contexts device
	std::string get_device_name() const;
	/// in case of multiple contexts, this makes this context current
	void make_current();
	/// decode sound file that is already in memory data with data_length bytes to a newly constructed memory buffer
	static bool decode_sound_file(const void* data, size_t data_length, OALSoundFormat& format, std::vector<int16_t>& memory_buffer);
	/// load and decode sound file to newly constructed memory buffer 
	static bool load_sound_file(const std::string& filepath, OALSoundFormat& format, std::vector<int16_t>& memory_buffer);
	/// create a named sound buffer from a format descriptor and a data buffer of given length in bytes
	void create_buffer(const std::string& symbolic_name, const OALSoundFormat& format, const void* data, size_t data_length);
	/**
	 * @brief      Loads a sound into the internal buffer list.
	 *
	 * @param[in]  filepath  The filepath to the sample file
	 * @param[in]  symbolic_name Sound is named by file stem or if given this parameter
	 *
	 * @remark     The supported filetypes are .WAV, .AIFF, .FLAC, etc. For
	 *             detailed information visit http://www.mega-nerd.com/libsndfile/
	 */
	void load_sample(std::string filepath, std::string symbolic_name = "");
	
	/**
	 * @brief      Loads a sound into the internal buffer list.
	 *
	 * @param[in]  symbolic_name  The symbolic name used as key for later
	 *                            retrieval
	 * @param[in]  data           A pointer to the first sample of the audio data
	 * @param[in]  data_length    The length of the audio data buffer in bytes
	 *
	 * @remark     This function is implemented via virtual IO on top of the
	 *             supplied buffer. Therefore the supplied buffer needs to be an
	 *             in memory copy of a ordinary audio file (for example contain
	 *             the necessary .WAV headers etc.).
	 *
	 * @remark     Each file will be transformed into a buffer ID with the
	 *             filename stem as symbolic key. For example: ./sounds/Wind.wav
	 *             will have the key "Wind".
	 */
	void load_sample(std::string symbolic_name, const void* data, size_t data_length);

	/**
	 * @brief      Loads all audio files in a folder into the internal buffer
	 *             list.
	 *
	 * @param[in]  folder     The path to the folder
	 * @param[in]  recursive  True if subfolders should be traversed as well,
	 *                        False otherwise.
	 *
	 * @remark     This function is a convenience wrapper around the tedious
	 *             process of reading all files in a folder and uploading each to
	 *             a buffer.
	 *
	 * @see        load_sample(std::string)
	 */
	void load_samples(std::string folder, bool recursive = false);

	/**
	 * @brief      Gets the buffer identifier for the symbolic name.
	 *
	 * @param[in]  sound_name  The symbolic sound name of the buffer.
	 *
	 * @return     The buffer ID as used by OpenAL.
	 * 
	 * @see        load_sample()
	 * @see        load_samples()
	 */
	ALuint get_buffer_id(std::string sound_name) const;
	/**
	 * @brief      Returns the last error from the OpenAL context
	 *
	 * @return     The error string.
	 * 
	 * @warning    This function call clears the last error in the OpenAL context.
	 * @see        is_no_error()
	 */
	std::string get_error_string();

	/**
	 * @brief      Determines if there were any errors since last call.
	 *
	 * @return     True if no errors were encountered, False otherwise.
	 *
	 * @warning    This function call clears the last error in the OpenAL context.
	 * @see        get_error_string()
	 */
	bool is_no_error();

	/**
	 * @brief      Toggles the OpenAL output mode to use HRTF.
	 *
	 * @param[in]  active  True if the output should be filtered with a head
	 *                     related transfer function, False otherwise.
	 *
	 * @remark     HRTFs improve the perceived localization of sounds in the
	 *             scene. Therefore it is advisable to enable HRTF output with
	 *             headphones. For loudspeakers however this should be disabled as
	 *             the users head already has a specific HRTF.
	 */
	void set_HRTF(bool active);

	/**
	 * @brief      Gets the native device handle for direct OpenAL calls.
	 *
	 * @return     The native OpenAL device handle.
	 */
	ALCdevice* get_native_device();

	/**
	 * @brief      Gets the native context handle for direct OpenAL calls.
	 *
	 * @return     The native OpenAL context handle.
	 */
	ALCcontext* get_native_context();

  private:
	ALCdevice* oal_device = nullptr;
	ALCcontext* oal_context = nullptr;

	//! Holds all uploaded samples
	std::map<std::string, ALuint> sample_buffers;
};

/**
 * @brief      This class models the single OpenAL listener.
 *
 * The OALListener is essentially a singleton class to use the listener model of
 * OpenAL. There are no constructors as there is only one listener in the OpenAL
 * context.
 */
class CGV_API OALListener final
{
  public:
	OALListener() = delete;
	OALListener(const OALListener& other) = delete;
	OALListener(OALListener&& other) = delete;
	OALListener& operator=(const OALListener& other) = delete;
	OALListener& operator=(OALListener&& other) = delete;
	~OALListener() = delete;

	/**
	 * @brief      Gets the listener position.
	 *
	 * @return     The current position.
	 */
	static cgv::math::fvec<float, 3> get_position();

	/**
	 * @brief      Gets the listener velocity.
	 *
	 * @return     The current velocity.
	 */
	static cgv::math::fvec<float, 3> get_velocity();

	/**
	 * @brief      Gets the listener orientation.
	 *
	 * @return     The view vector as first, the up vector as second.
	 */
	static cgv::math::fmat<float, 3, 2> get_orientation();

	/**
	 * @brief      Sets the listener position.
	 *
	 * @param[in]  pos   The new position
	 */
	static void set_position(cgv::math::fvec<float, 3> pos);

	/**
	 * @brief      Sets the listener velocity.
	 *
	 * @param[in]  vel   The new velocity
	 */
	static void set_velocity(cgv::math::fvec<float, 3> vel);

	/**
	 * @brief      Sets the listener orientation.
	 *
	 * @param[in]  at    The new view direction
	 * @param[in]  up    The new up direction
	 */
	static void set_orientation(cgv::math::fvec<float, 3> at, cgv::math::fvec<float, 3> up);
};

/**
 * @brief This enum represents different states of a sound source.
 */
enum OALSourceState {
	OALSS_INITIAL,
	OALSS_PLAYING,
	OALSS_PAUSED,
	OALSS_STOPPED
};

/**
 * @brief      This class describes a sound source in the scene.
 */
class CGV_API OALSource
{
  public:
	OALSource() = default;
	OALSource(const OALSource&) = delete;
	OALSource(OALSource&&) = default;
	OALSource& operator=(const OALSource&) = delete;
	OALSource& operator=(OALSource&&) = default;
	~OALSource();

	/**
	 * @brief      This function links a source to a context.
	 *
	 * @param      ctx_ptr         The OpenAL context
	 * @param[in]  sound_name  Optionally a named sound whose buffer to append
	 *
	 * @return     True if initialization was successful, False otherwise.
	 *
	 * @warning    An internal reference to the given context is made. You need to
	 *             make sure, that this object does not outlive the context.
	 * @see        OALContext::load_sample()
	 * @see        OALContext::load_samples()
	 */
	bool init(OALContext& ctx, std::string sound_name = "");
	/// destruct source and restore uninitialized state
	void clear();
	/**
	 * @brief      Append buffer of named sound to source's playback list
	 *
	 * @param[in]  sound_name  The sounds symbolic name
	 *
	 * @return     True if attachment was successful, False otherwise.
	 *
	 * @warning    Failes if source is playing or paused and not stopped.
	 * 
	 * @see        OALContext::load_sample()
	 * @see        OALContext::load_samples()
	 */
	bool append_sound(std::string sound_name);
	/**
	 * @brief      Sets the source position.
	 *
	 * @param[in]  pos   The new position of the source
	 */
	void set_position(cgv::math::fvec<float, 3> pos);

	/**
	 * @brief      Sets the source velocity.
	 *
	 * @param[in]  vel   The new velocity of the source
	 */
	void set_velocity(cgv::math::fvec<float, 3> vel);

	/**
	 * @brief      Sets the source pitch.
	 *
	 * @param[in]  pitch  The new pitch of the source
	 */
	void set_pitch(float pitch);

	/**
	 * @brief      Sets the source gain.
	 *
	 * @param[in]  gain  The new gain of the source
	 */
	void set_gain(float gain);

	/**
	 * @brief      Sets the looping property of the source.
	 *
	 * @param[in]  should_loop  True if the source buffer should be looped, False
	 *                          otherwise.
	 */
	void set_looping(bool should_loop);

	/**
	 * @brief      Gets the source position.
	 *
	 * @return     The current position of the source.
	 */
	cgv::math::fvec<float, 3> get_position() const;

	/**
	 * @brief      Gets the source velocity.
	 *
	 * @return     The current velocity of the source.
	 */
	cgv::math::fvec<float, 3> get_velocity() const;

	/**
	 * @brief      Gets the source pitch.
	 *
	 * @return     The current pitch of the source.
	 */
	float get_pitch() const;

	/**
	 * @brief      Gets the source gain.
	 *
	 * @return     The current gain of the source.
	 */
	float get_gain() const;

	/**
	 * @brief      Determines if the source has the looping property.
	 *
	 * @return     True if the source is looping its buffer, False otherwise.
	 */
	bool is_looping() const;

	/**
	 * @brief      Determines if the source is playing.
	 *
	 * @return     True if the source is currently playing, False otherwise.
	 */
	bool is_playing() const;

	
	/// returns state of this source
	OALSourceState get_state() const;

	/**
	 * @brief      Commences the playback of the sound buffer.
	 */
	void play();

	/**
	 * @brief      Pauses the playback of the sound buffer without moving the
	 *             playhead.
	 */
	void pause();

	/**
	 * @brief      Explicitly toggles between pausing and playing the sound
	 *             buffer.
	 *
	 * @param[in]  should_play  True if the source should play, False if it should
	 *                          be paused.
	 *
	 * @remark     This function is provided as convenience to directly control
	 *             the source with a boolean argument.
	 */
	void play_pause(bool should_play);

	/**
	 * @brief      Stops the playback of the sound buffer and moves the playhead
	 *             to the beginning.
	 */
	void stop();

	/**
	 * @brief      Moves the playhead to the beginning of the buffer.
	 */
	void rewind();
protected:
	bool append_sound_impl(std::string sound_name);
  private:
	OALContext* ctx_ptr = nullptr;
	ALuint src_id = 0;
};

} // namespace audio
} // namespace cgv

#include <cgv/config/lib_end.h>